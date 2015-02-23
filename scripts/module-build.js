#!/usr/bin/env node

var fs = require('fs')
var path = require('path')

var moduleWrapperStart = "module.exports = function(deps, opts) {\n"
var moduleWrapperEnd = "\nreturn Module;\n}"

var target = process.argv[2]

var prelude = fs.readFileSync(path.resolve('commonjs/prelude.js'), {encoding: 'utf8'})
var epilogue = fs.readFileSync(path.resolve('commonjs/epilogue.js'), {encoding: 'utf8'})
var outputDirPath = path.join(path.resolve('commonjs/'), "pcejs-" + target + "/lib/")

try { fs.mkdirSync(outputDirPath) } catch (err) { if (err.code !== 'EEXIST') throw err }

var outStream = fs.createWriteStream(path.join(outputDirPath, "pcejs-" + target + ".js"), {encoding: 'utf8'})

outStream.on('close', function() {
  console.log("wrote commonjs/pcejs-" + target + "/lib/pcejs-" + target + ".js")
  process.exit(0)
})

outStream.write(moduleWrapperStart)
outStream.write(prelude)

console.log("reading dist/pce-" + target + ".js")
fs.createReadStream(path.resolve("dist/pce-" + target + ".js"), {encoding: 'utf8'})
  .on('end', function() {
    console.log("end dist/pce-" + target + ".js")
    outStream.write(epilogue)
    outStream.end(moduleWrapperEnd)
  })
  .pipe(outStream, {end: false})
