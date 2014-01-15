path = require('path')
util = require('util')
child_process = require('child_process')

PCEJSBuild = require('./pcejs_build')

module.exports = (grunt) ->
  pcejs = new PCEJSBuild(grunt)
  pcejs.loadConfig() # extend with saved config

  packagedir = pcejs.config.packagedir

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
    pcejs.initConfig() # discard loaded config, reset to defaults
    pcejs.saveConfig()

  grunt.task.registerTask 'target', 'Choose build target (macplus,ibmpc,atarist)', (target) ->
    pcejs.config.target = target
    pcejs.saveConfig()

  grunt.task.registerTask 'configure', 'Configure emulator build', (arch) ->
    if (arch is 'native')
      pcejs.config.emscripten = false
      pcejs.config.prefix = 'build-native/'
    if (arch is 'em')
      pcejs.config.emscripten = true
      pcejs.config.prefix = 'build/'

    pcejs.run(this.async(), './scripts/10-configure.sh')

    pcejs.saveConfig()

  grunt.task.registerTask 'make', 'Build emulator', ->
    pcejs.run(this.async(), './scripts/20-make.sh')

  grunt.task.registerTask 'remake', 'Rebuild emulator', ->
    pcejs.run(this.async(), './scripts/21-remake.sh')

  grunt.task.registerTask 'clean', 'Clean source tree', (full) ->
    if full
      pcejs.run(this.async(), './scripts/a0-clean.sh')
    else
      pcejs.run(this.async(), 'make', ['clean'])
  grunt.task.registerTask 'afterbuild', 'Package build for web', (target) ->
    pcejs.config.target = target if target

    pcejs.run(this.async(), './scripts/30-afterbuild.sh')

  grunt.registerTask 'build', 'Configure, build and package for web', (target) ->
    pcejs.config.target = target if target

    if target is 'native'
      grunt.task.run.apply(grunt.task, [
        'configure:native',
        'make'
      ])
    else
      grunt.task.run.apply(grunt.task, [
        'configure:em',
        'make',
        'afterbuild:'+pcejs.config.target
      ])
      
    pcejs.saveConfig()

  grunt.task.registerTask 'run', 'Run emulator', ->
    done = this.async()
    
    pcejs.run(done, 'http-server', [packagedir])

    child_process.exec('open http://localhost:8080/')
    console.log('serving emulator at http://localhost:8080/')

  grunt.task.registerTask 'romdir', 'Set rom/config/data directory', (romdir) ->
    pcejs.config.romdir = path.resolve(romdir)
    pcejs.saveConfig()
    # pcejs.run(this.async(), './scripts/a1-romdir.sh', [], PCEJS_ROMDIR: path.resolve(romdir))

  grunt.task.registerTask 'env', 'Print build environment variables', ->
    console.log(pcejs.getEnv())
  
  grunt.registerTask 'rebuild', 'Build last again', (target) ->
    pcejs.config.target = target if target

    if target is 'native'
      grunt.task.run.apply(grunt.task, [
        'remake'
      ])
    else
      grunt.task.run.apply(grunt.task, [
        'remake',
        'afterbuild:'+pcejs.config.target
      ])

  # build ui
  grunt.registerTask 'ui', [
    'coffee:compile',
    'less:compile',
    'concat:deps',
  ]

  # Default task
  # grunt.registerTask('default', [])
