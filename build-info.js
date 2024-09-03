const fs = require('fs')

const version = require('./params.json')._pluginVersion

const date = new Date().toISOString();

const [datePart, timePart] = date.split('T')
const [h, min] = timePart.split(':')

fs.writeFileSync('build.h', 
`// This file is generated
#pragma once

#define BUILD_DATE "${datePart}-${h}:${min}"

#define VERSION_STRING "${version.join('.')}"
constexpr int VERSION_MAJOR = ${version[0]};
constexpr int VERSION_MINOR = ${version[1]};

constexpr void versionConvert(int *combined, int *major, int *minor) {
    const int c = (*major << 12) | *minor;
    *major = (*combined >> 12) & 0xFFF;
    *minor = *combined & 0xFFF;
    *combined = c;
}
`)