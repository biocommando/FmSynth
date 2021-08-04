const params = require('./params')
const fs = require('fs')
const crypto = require('crypto')

const dtos = []
const groupData = []
let index = -1
function nextIndex() {
    return ++index
}

function iterate(obj, parent, parentShort) {
    const parentStr = parent ? parent + "/" : "";
    const parentShortStr = parentShort ? parentShort : "";
    Object.keys(obj)
        .forEach(key => {
            if (typeof obj[key] !== "string" && key[0] !== '_') {
                const fullName = parentStr + (obj[key]._fullName ? obj[key]._fullName : key)
                iterate(obj[key], fullName, parentShortStr + key)
            } else if (key[0] !== '_') {
                const fullName = parentStr + key
                dtos.push({
                    fullName,
                    shortName: parentShortStr + obj[key],
                    index: nextIndex()
                })
            } else if (key === '_group') {
                const data = { groupId: obj[key], groupName: params[obj[key]]._groupName, parent, start: dtos.length }
                groupData.push(data)
                iterate(params[obj[key]], parent, parentShort)
                data.length = dtos.length - data.start
            }
        })
}

iterate(params)

function fullNameToVarName(fn) {
    return fn.replace(/\//g, '__').replace(/ /g, '_')
}

function getSaveId(nameToHash) {
    const found = params._reservedIds.find(x => x.name === nameToHash)
    if (found)
        return found.id
    const h = crypto.createHash('sha1')
    h.update(nameToHash)
    return new Int32Array(h.digest().buffer)[0]
}

let code = `
#pragma once

constexpr int total_number_of_parameters = ${dtos.length};\n\n`

let nameGetterCode = `constexpr const char *getNameForParam(int idx, bool fullName)\n{\n`
let saveIdGetterCode = `constexpr int getSaveIdForParam(int idx)\n{\n    switch(idx)\n    {\n`
const saveIds = []

dtos.forEach(dto => {
    code += `constexpr int idx_${fullNameToVarName(dto.fullName)} = ${dto.index};\n`
    nameGetterCode += `    if (idx == ${dto.index}) return fullName ? "${dto.fullName}" : "${dto.shortName}";\n`
    const saveId = getSaveId(dto.fullName)
    if (saveIds.includes(saveId) ||
        params._reservedIds.some(x => x.id === saveId && x.name !== dto.fullName)) {
        throw `Save id conflict for param #${dto.index} '${dto.fullName}'! ` +
        `You can fix this by defining an explicit id in _reservedIds list.`
    }
    saveIds.push(saveId)
    saveIdGetterCode += `        case ${dto.index}: return ${saveId};\n`
})

nameGetterCode += '    return fullName ? "unknown full" : "unknown";\n}\n'
saveIdGetterCode += '        default: return 0;\n    }\n}\n'

code += '\n'

groupData.forEach(grp => {
    code += `constexpr int group_${fullNameToVarName(grp.parent)}_start = ${grp.start};\n`
})

code += '\n// Group lengths\n'

groupIds = {}
groupData.forEach(grp => {
    if (!groupIds[grp.groupId]) {
        groupIds[grp.groupId] = true
        code += `constexpr int group_${fullNameToVarName(grp.groupName)}_length = ${grp.length};\n`
    }
})

code += '\n'

code += nameGetterCode

code += '\n' + saveIdGetterCode

fs.writeFileSync('params.h', code)