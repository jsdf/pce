
# util functions

isNodeEnvironment = -> window? and not (process?.title is 'node')

browserPrefixes = ['-webkit-', '-moz-', '-ms-', '']

pathGetFilenameRegex = /\/([^\/]+)$/
pathGetFilename = (path) ->
  matches = path.match pathGetFilenameRegex
  if matches and matches.length
    matches[1]
  else
    path

# end util functions

window.PCEJS =
  #  default properties
  useDom: isNodeEnvironment()
  containerSelector: '#pcejs'
  canvas: null
  canvasId: 'canvas'
  printEnabled: false
  touchCurrentFirst: null
  canvasWidth: 512
  canvasHeight: 342
  canvasScale: 1
  canvasOffset: 0
  zoomControls: false
  dataFiles: [
    # 'data/hd1.qed',
    # 'data/mac-plus.rom',
    # 'data/macplus-pcex.rom',
    # 'data/pce-config.cfg',
  ]
  argv: ['-c','pce-config.cfg','-r']

  init: (@config = {}) ->
    _.extend(this, @config) # allow configuration override

    @preInit()
    @moduleInit()
    @postInit()

  preInit: ->
    if @useDom
      @container = $(@containerSelector)
      # include emscripten boilerplate html
      @container.html(@containerHTML())

    # single buffer canvas
    @canvas ||= document.getElementById(@canvasId)
    @context = @canvas.getContext('2d')

    @layoutCanvas()

  moduleInit: ->
    config = this

    if @useDom
      statusElement = $('#'+@canvasId+'-status').get(0)
      progressElement = $('#'+@canvasId+'-progress').get(0)

    # emscripten module object initialisation
    # injects config and behaviour into emscripen compiled module
    @module =
      'arguments': @argv
      preRun: [
        @mountPersistentFS.bind(this),
        @loadDataFiles.bind(this)
      ]
      postRun: []
      print: if (!@printEnabled) then (->) else console.log.bind(console)
      printErr: if (!@printEnabled) then (->) else ((text) -> console.warn(text))
      canvas: @canvas
      canvasFront: if @doubleBuffer then @canvas else null
      totalDependencies: 0
      setStatus: (text) ->
        if config.useDom
          if (@setStatus.interval) then clearInterval(@setStatus.interval)
          m = text.match(/([^(]+)\((\d+(\.\d+)?)\/(\d+)\)/)
          if (m)
            text = m[1]
            progressElement.value = parseInt(m[2])*100
            progressElement.max = parseInt(m[4])*100
            progressElement.hidden = false
          else
            progressElement.value = null
            progressElement.max = null
            progressElement.hidden = true

          statusElement.innerHTML = text
        else
          console.log text

      monitorRunDependencies: (remainingDependencies) ->
        @totalDependencies = Math.max(@totalDependencies, remainingDependencies)

        @setStatus(if remainingDependencies then 'Preparing... (' + (@totalDependencies-remainingDependencies) + '/' + @totalDependencies + ')' else 'All downloads complete.')
        if @useDom
          $(document).trigger('dependenciesLoaded') unless remainingDependencies

  postInit: ->
    if @useDom
      @bindTouchEventHandlers()

    @module.setStatus('Downloading...')

    if @useDom
      @addZoomControls() if @zoomControls
      @addAboutLink()

    @addErrorHandler()

    window.Module = @module

    @fsProxy = (methodName) =>
      if FS?[methodName]?
        FS[methodName]
      else if @module["FS_#{methodName}"]?
        @module["FS_#{methodName}"]
      else
        -> console.warn("couldn't resolve method FS.#{methodName}")

  loadDataFiles: ->
    _.each @dataFiles, (filepath) =>
      @fsProxy('createPreloadedFile')('/', pathGetFilename(filepath), filepath, true, true)

  mountPersistentFS: ->
    @fsProxy('mkdir')('/data')
    @fsProxy('mount')((IDBFS if IDBFS?), {}, '/data')
    @fsProxy('syncfs')(true, (err) -> console.error(err) if err)

  syncPersistentFS: ->
    @fsProxy('syncfs')((err) -> console.error(err) if err)

  touchToMouseEvent: (event) ->
    firstTouch = event.changedTouches[0]
    mouseEventType = ""
    switch event.type
      when "touchstart" then mouseEventType = "mousedown"
      when "touchmove"  then mouseEventType = "mousemove"        
      when "touchend"   then mouseEventType = "mouseup"
      else return

    if (event.type == 'touchstart') 
      @touchCurrentFirst = firstTouch

    if (event.type == 'touchmove') 
      event.preventDefault()
      return false

    if ( event.type == 'touchmove' && @touchCurrentFirst && !( Math.abs(firstTouch.screenX - @touchCurrentFirst.screenX) > 30 || Math.abs(firstTouch.screenY - @touchCurrentFirst.screenY) > 30)) 
      # drag less than 10px
      return
    
    # proxying event of event.type to mouseEventType

    simulatedEvent = document.createEvent("MouseEvent")
    simulatedEvent.initMouseEvent(
      mouseEventType, true, true, window, 1, 
      firstTouch.screenX, firstTouch.screenY, 
      firstTouch.clientX, firstTouch.clientY, false, 
      false, false, false, 0, null
    )

    firstTouch.target.dispatchEvent(simulatedEvent)

    Browser.step_func() if typeof Browser != 'undefined' # emscripten runtime method

  layoutCanvas:  ->
    @canvas.width = @canvasWidth if @canvasWidth
    @canvas.height = @canvasHeight if @canvasHeight
    @container.find('.emscripten_border').height((@canvas.height)*@canvasScale) if @canvasHeight
    # $(@canvas).css(_.reduce(browserPrefixes, (css, prefix) =>
    #   css["#{prefix}transform"] = "scale(#{@canvasScale})"
    #   css["#{prefix}transform-origin"] = "center top"
    #   css
    # , {}))
    
  bindTouchEventHandlers: ->
    _.each ['touchstart','touchend', 'touchmove'], (event) =>
      mainCanvas = @module.canvasFront || @module.canvas
      mainCanvas.addEventListener(event, (-> @touchToMouseEvent), true)

  addErrorHandler: ->
    window.onerror = -> Browser.step_run() if typeof Browser != 'undefined' and Browser.step_run

  addAboutLink: ->
    @container
      .append $('<small><a href="https://github.com/jsdf/pce/blob/pcejs/README.md">about pce.js emulator</a></small>')

  addZoomControls: ->
    @container
      .append $('<div class="zoom-controls"></div>')
        .append($('<label>Scale:</label>'))
        .append _.map [1,1.5,2], (zoom) ->
          $('<input />',  {'type': 'button', 'value': zoom+"x", 'data-scale': zoom}).get(0)

    $(document).on 'click', '.zoom-controls input[type=button]', (event) =>
      @canvasScale = parseFloat($(event.target).attr('data-scale'),10)
      @layoutCanvas()

  containerHTML: -> """
    <div class="emscripten" id="#{@canvasId}-status">Downloading...</div>
    <div class="emscripten">
      <progress value="0" max="100" id="#{@canvasId}-progress" hidden=1></progress>
    </div>
    <div class="emscripten_border">
      <canvas class="emscripten" id="#{@canvasId}" oncontextmenu="event.preventDefault()"></canvas>
    </div>
    """
