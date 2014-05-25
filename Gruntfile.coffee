path = require('path')
util = require('util')
child_process = require('child_process')

PCEJSBuild = require('./tasks/pcejs-build')
moduleBuild = require('./tasks/module-build')


module.exports = (grunt) -> 
  pcejsBuild = new PCEJSBuild(grunt)
  pcejsBuild.loadConfig() # extend with saved config

  packagedir = pcejsBuild.config.packagedir

  # Project configuration.
  grunt.initConfig
    pkg: grunt.file.readJSON('package.json')
    coffee: 
      compile: 
        options: 
          join: true # concat then compile into single file
          sourceMap: false # create sourcemaps
          bare: false  # add global wrapper
        files: [
          dest: "#{packagedir}js/pce.js"
          src: "ui/pce.coffee"
        ]
    less: 
      compile: 
        files: [
            src: 'ui/*.less'
            dest: "#{packagedir}css/pce.css"
        ]
    concat:
      deps: 
        options:
          separator: ';'+grunt.util.linefeed
        files: [
          dest: "#{packagedir}js/pce-deps.js"
          # order matters
          src: [
            'zepto.min',
            'underscore-min',
          ].map (filename) -> "ui/deps/#{filename}.js"
        ]
    watch: 
      coffee:
        files: ['ui/*.coffee']
        tasks: ['coffee:compile']
        options: spawn: false
      less:
        files: ['ui/*.less']
        tasks: ['less:compile']
        options: spawn: false

  # Load plugins for less, coffeescript etc
  grunt.loadNpmTasks('grunt-contrib-coffee')
  grunt.loadNpmTasks('grunt-contrib-less')
  grunt.loadNpmTasks('grunt-contrib-concat')
  grunt.loadNpmTasks('grunt-contrib-watch')

  # Main tasks
  grunt.task.registerTask 'reset', 'Reset config to default', ->
    pcejsBuild.initConfig() # discard loaded config, reset to defaults
    pcejsBuild.saveConfig()

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

  grunt.task.registerTask 'make', 'Build emulator', ->
    pcejsBuild.run(this.async(), './scripts/20-make.sh')

  grunt.task.registerTask 'remake', 'Rebuild emulator', ->
    pcejsBuild.run(this.async(), './scripts/21-remake.sh')

  grunt.task.registerTask 'clean', 'Clean source tree', (full) ->
    if full
      pcejsBuild.run(this.async(), './scripts/a0-clean.sh')
    else
      pcejsBuild.run(this.async(), 'make', ['clean'])
  grunt.task.registerTask 'afterbuild', 'Package build for web', (target) ->
    pcejsBuild.config.target = target if target

    pcejsBuild.run(this.async(), './scripts/30-afterbuild.sh')

  grunt.registerTask 'build', 'Configure, build and package for web', (target) ->
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

  grunt.task.registerTask 'run', 'Run emulator', ->
    done = this.async()
    
    pcejsBuild.run(done, 'http-server', [packagedir])

    child_process.exec('open http://localhost:8080/')
    console.log('serving emulator at http://localhost:8080/')

  grunt.task.registerTask 'romdir', 'Set rom/config/data directory', (romdir) ->
    pcejsBuild.config.romdir = path.resolve(romdir)
    pcejsBuild.saveConfig()
    # pcejsBuild.run(this.async(), './scripts/a1-romdir.sh', [], PCEJS_ROMDIR: path.resolve(romdir))

  grunt.task.registerTask 'env', 'Print build environment variables', ->
    console.log(pcejsBuild.getEnv())
  
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
  

  # build ui
  grunt.registerTask 'ui', [
    'coffee:compile',
    'less:compile',
    'concat:deps',
  ]

  # Default task
  # grunt.registerTask('default', [])
