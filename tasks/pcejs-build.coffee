util = require('util')
path = require('path')
child_process = require('child_process')
_ = require('lodash')

# utils
spawnRun = (done, command, args, opts) ->
  process = child_process.spawn(command, args, opts)

  process.stdout.setEncoding('utf8')
  process.stdout.on('data', (data) -> util.print(data))

  process.stderr.setEncoding('utf8')
  process.stderr.on('data', (data) -> util.error(data))

  process.on('close', (code) -> done(code))

class PCEJSBuild
  archs: [
    'macplus',
    'ibmpc',
    'atarist',
  ]
  defaultConfig:
    target: 'macplus'
    emscripten: true
    asmjs: true
    optlvl: 'O2'
    dbglvl: null
    memory: 256
    prefix: 'build/'
    packagedir: 'dist/'
    outputformat: 'js'
    emsdkpath: ''
    romdir: ''

  configFilePath: './pcejs-build-config.json'
  
  constructor: (@grunt) -> this.initConfig()

  initConfig: (@config = _.clone(@defaultConfig)) ->

  extendConfig: (extraConfig) -> _.extend(@config, extraConfig)

  # extend config from saved
  loadConfig: (path = @configFilePath) ->
    try
      this.extendConfig(@grunt.file.readJSON(path))
    catch e
      console.warn(path+' file not found')

  saveConfig: (path = @configFilePath) ->
    try
      @grunt.file.write(path, JSON.stringify(@config, undefined, 2))
    catch e
      console.error(path+' could not save')

  getEmscriptenFlags: ->
    flags = []
    # flags.push('-s VERBOSE=1')
    # flags.push('-s OUTLINING_LIMIT=16000')
    flags.push('-s PRECISE_I64_MATH=0') if @config.imprecise64 is true
    flags.push('-s TOTAL_MEMORY=' + @config.memory*1024*1024) if @config.memory?
    flags.push('-s ASM_JS=1') if @config.asmjs is true
    flags.push('-s OUTLINING_LIMIT='+@config.outlining) if @config.outlining
    if @config.exportfuncs
      flags.push('-s EXPORTED_FUNCTIONS='+JSON.stringify(@config.exportfuncs)+'')
    flags.push('-'+@config.optlvl) if @config.optlvl?
    flags.push('-g'+(@config.dbglvl if _.isNumber(@config.dbglvl))) if @config.dbglvl?
    flags.join(' ')

  getEnv: ->
    env =
      PCEJS_PREFIX: path.resolve(@config.prefix) 
      PCEJS_PACKAGEDIR: path.resolve(@config.packagedir) 
      PCEJS_TARGET: @config.target
      PCEJS_OUTPUT_FORMAT: @config.outputformat
      PCEJS_ROMDIR: @config.romdir

    if @config.emscripten
      emflags = this.getEmscriptenFlags()

      env.PCEJS_EMSDK_PATH = @config.emsdkpath
      env.PCEJS_EMSCRIPTEN = "yes"
      env.PCEJS_EMFLAGS = emflags
      env.PCEJS_CFLAGS = "-Qunused-arguments -include src/include/pcedeps.h #{emflags}"
      env.PCEJS_CONFIGURE = "#{@config.emsdkpath}emconfigure ./configure"
      
    else
      env.PCEJS_CFLAGS = "-I/usr/local/opt/emscripten/system/include/emscripten/"
      env.PCEJS_CONFIGURE = "./configure"

    return env

  configEnv: (baseEnv) -> _.extend(_.clone(baseEnv), this.getEnv())

  run: (done, command, args = [], env = {}) ->
    spawnRun done, command, args, 
      cwd: process.env.PWD
      env: _.extend(this.configEnv(process.env), env)

module.exports = PCEJSBuild
