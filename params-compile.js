const params = require('./params')
const fs = require('fs')

const dtos = []
const groupData = []
let id = -1
function nextId() {
    do {
        id++
    } while (params.reservedIds.includes(id))
    return id
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
                    id: nextId()
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

let code = `
#pragma once

constexpr int total_number_of_parameters = ${dtos.length};\n\n`

let nameGetterCode = `constexpr const char *getNameForParam(int id, bool fullName)\n{\n`

dtos.forEach(dto => {
    code += `constexpr int id_${fullNameToVarName(dto.fullName)} = ${dto.id};\n`
    nameGetterCode += `    if (id == ${dto.id}) return fullName ? "${dto.fullName}" : "${dto.shortName}";\n`
})

nameGetterCode += '    return fullName ? "unknown full" : "unknown";\n}\n'

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

fs.writeFileSync('params.h', code)