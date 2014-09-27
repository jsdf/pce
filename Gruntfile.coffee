path = require('path')
util = require('util')
child_process = require('child_process')

PCEJSBuild = require('./tasks/pcejs-build')
moduleBuild = require('./tasks/module-build')

ARCHS = [
  'macplus',
  'ibmpc',
  'atarist',
]

module.exports = (grunt) -> 
  pcejsBuild = new PCEJSBuild(grunt)
  pcejsBuild.loadConfig() # extend with saved config

  packagedir = pcejsBuild.config.packagedir

  # Project configuration.
  grunt.initConfig
    pkg: grunt.file.readJSON('package.json')

  # Main tasks
  grunt.task.registerTask 'reset', 'Reset config to default', ->
    pcejsBuild.initConfig() # discard loaded config, reset to defaults
    pcejsBuild.saveConfig()

  grunt.task.registerTask 'env', 'Print build environment variables', ->
    console.log(pcejsBuild.getEnv())

  grunt.task.registerTask 'target', 'Choose build target (macplus,ibmpc,atarist)', (target) ->
    pcejsBuild.config.target = target
    pcejsBuild.saveConfig()

  grunt.task.registerTask 'configure', 'Configure emulator build', (platform) ->
    if (platform is 'native')
      pcejsBuild.config.emscripten = false
      pcejsBuild.config.prefix = 'build-native/'
    if (platform is 'em')
      pcejsBuild.config.emscripten = true
      pcejsBuild.config.prefix = 'build/'

    pcejsBuild.run(this.async(), './scripts/10-configure.sh')

    pcejsBuild.saveConfig()

  grunt.task.registerTask 'make', 'Compile emulator source to LLVM bitcode', ->
    pcejsBuild.run(this.async(), './scripts/20-make.sh')
  
  grunt.task.registerTask 'remake', 'Recompile only changed files of emulator source to LLVM bitcode', ->
    pcejsBuild.run(this.async(), './scripts/21-remake.sh')

  grunt.task.registerTask 'clean', 'Clean source tree', (full) ->
    if full
      pcejsBuild.run(this.async(), './scripts/a0-clean.sh')
    else
      pcejsBuild.run(this.async(), 'make', ['clean'])
  grunt.task.registerTask 'afterbuild', 'Convert LLVM bitcode to JS', (target) ->
    pcejsBuild.config.target = target if target

    pcejsBuild.run(this.async(), './scripts/30-afterbuild.sh')

  grunt.registerTask 'build', 'Configure build and compile emulator to JS', (target) ->
    pcejsBuild.config.target = target if target

    if target is 'native'
      grunt.task.run.apply(grunt.task, [
        'configure:native',
        'make'
      ])
    else
      grunt.task.run.apply(grunt.task, [
        'configure:em',
        'make',
        'afterbuild:'+pcejsBuild.config.target
      ])
      
    pcejsBuild.saveConfig()

  grunt.registerTask 'rebuild', 'Build last again', (target) ->
    pcejsBuild.config.target = target if target

    if target is 'native'
      grunt.task.run.apply(grunt.task, [
        'remake'
      ])
    else if target
      grunt.task.run.apply(grunt.task, [
        'remake',
        'afterbuild:'+pcejsBuild.config.target
      ])
    else
      grunt.task.run.apply(grunt.task, [
        'remake',
        'afterbuild:macplus',
        'afterbuild:ibmpc',
        'afterbuild:atarist',
      ])

  grunt.task.registerTask 'module', 'Build commonjs module', (target) ->
    throw new Error('no target') unless target?
    done = this.async()
    moduleBuild(grunt, target, done)

  grunt.task.registerTask 'commonjs', ['module']

  grunt.registerTask 'default', ->
    if pcejsBuild.config.emscripten
      tasks = [
        'remake',
      ]
    else
      tasks = [
        'configure:em',
        'make',
      ]

    tasks
      .concat(ARCHS.map (arch) -> "afterbuild:#{arch}")
      .concat(ARCHS.map (arch) -> "module:#{arch}")

    grunt.task.run(tasks...)


  