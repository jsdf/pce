(function() {
  window.PCEJS = window.PCEJS || {};

  _.defaults(window.PCEJS, {
    containerEl: 'pcejs',
    doubleBuffer: false,
    printEnabled: false,
    touchCurrentFirst: null,
    canvasWidth: 512,
    canvasHeight: 384,
    'arguments': ['-c','roms/pce-config.cfg']
  });

  // include emscripten boilerplate html
  $(PCEJS.containerEl).html(containerHTML());

  if (PCEJS.doubleBuffer) {
    // double buffer front canvas
    var o_canvas = $('#canvas').get(0)
    var o_context = o_canvas.getContext('2d');
    o_canvas.width = PCEJS.canvasWidth;
    o_canvas.height = PCEJS.canvasHeight;

    // double buffer back canvas
    var m_canvas = document.createElement('canvas');
    var m_context = m_canvas.getContext('2d');
  } else {
    // single buffer canvas
    var t_canvas = $('#canvas').get(0)
    var t_context = t_canvas.getContext('2d');
    t_canvas.width = PCEJS.canvasWidth;
    t_canvas.height = PCEJS.canvasHeight;
  }

  // emscripten module object initialisation
  // injects config and behaviour into emscripen compiled module
  window.Module = {
    preRun: [],
    postRun: [],
    print: (function() {
      if (!PCEJS.printEnabled) return function() {};
      return function(text) {
        return console.log(text);
      };
    })(),
    printErr: (function() {
      if (!PCEJS.printEnabled) return function() {};
      return function(text) {
        return console.warn(text);
      };
    })(),
    canvas: PCEJS.doubleBuffer ? m_canvas : t_canvas,
    canvasFront: PCEJS.doubleBuffer ? o_canvas : null,
    setStatus: function(text) {
      if (Module.setStatus.interval) clearInterval(Module.setStatus.interval);
      var m = text.match(/([^(]+)\((\d+(\.\d+)?)\/(\d+)\)/);
      var statusElement = $('#status').get(0);
      var progressElement = $('#progress').get(0);
      if (m) {
        text = m[1];
        progressElement.value = parseInt(m[2])*100;
        progressElement.max = parseInt(m[4])*100;
        progressElement.hidden = false;
      } else {
        progressElement.value = null;
        progressElement.max = null;
        progressElement.hidden = true;
      }
      statusElement.innerHTML = text;
    },
    totalDependencies: 0,
    monitorRunDependencies: function(left) {
      this.totalDependencies = Math.max(this.totalDependencies, left);
      Module.setStatus(left ? 'Preparing... (' + (this.totalDependencies-left) + '/' + this.totalDependencies + ')' : 'All downloads complete.');
      if (!left) {
        $(document).trigger('dependenciesLoaded');
      }
    }
  };

  bindTouchEventHandler();

  if (PCEJS.doubleBuffer) initDoubleBuffer();

  $('#gofullscreen').click(function(){
    Module.requestFullScreen($('#pointerLock').get(0).checked,$('#resize').get(0).checked)
  });

  Module.setStatus('Downloading...');

  // helper functions
  function containerHTML() {
    return '<div class="emscripten" id="status">Downloading...</div>'+
    '<div class="emscripten">'+
      '<progress value="0" max="100" id="progress" hidden=1></progress>'+
    '</div>'+
    '<div class="emscripten_border">'+
      '<canvas class="emscripten" id="canvas" oncontextmenu="event.preventDefault()"></canvas>'+
    '</div>'+
    '<div class="emscripten" id="fullscreencontrols">'+
      '<input type="checkbox" id="resize">Resize canvas'+
      '<input type="checkbox" id="pointerLock" checked>Lock/hide mouse pointer'+
      '&nbsp;&nbsp;&nbsp;'+
      '<input type="button" value="Fullscreen" id="gofullscreen">'+
    '</div>'+
    '<textarea class="emscripten" id="output" rows="8"></textarea>';
  }

  function initDoubleBuffer() {
      // proxy events from front canvas to back canvas
      ['mousedown', 'mouseup', 'mousemove', 'mouseout'].forEach(function(event) {
        Module["canvasFront"].addEventListener(event, function(e) {
          try {
            Module["canvas"].dispatchEvent(e);
          } catch (e) {
            // console.warn('failed to dispatch '+event, e);
          }
        }, true);
      });

      function render() {
        o_context.drawImage(m_canvas, 0, 0);
        requestAnimationFrame(render);
      }

      render();
  }

  function touchHandler(event) {
    console.log(event);
      var touches = event.changedTouches,
          first = touches[0],
          type = "";
           switch(event.type)
      {
          case "touchstart": type = "mousedown"; break;
          case "touchmove":  type="mousemove"; break;        
          case "touchend":   type="mouseup"; break;
          default: return;
      }

      if (event.type == 'touchstart') {
        PCEJS.touchCurrentFirst = first;
      }

      if (event.type == 'touchmove') {
        event.preventDefault();
        return false;
      }
      if (
        event.type == 'touchmove'
        && PCEJS.touchCurrentFirst
        && !(
          Math.abs(first.screenX - PCEJS.touchCurrentFirst.screenX) > 30
          || Math.abs(first.screenY - PCEJS.touchCurrentFirst.screenY) > 30
        )
      ) {
        console.log('drag less than 10px');
        return;
      }

      console.log('proxying event of type',event.type,'to type',type)

      var simulatedEvent = document.createEvent("MouseEvent");
      simulatedEvent.initMouseEvent(
        type, true, true, window, 1, 
        first.screenX, first.screenY, 
        first.clientX, first.clientY, false, 
        false, false, false, 0/*left*/, null
      );

     first.target.dispatchEvent(simulatedEvent);
     Browser.step_func();
  }

  function bindTouchEventHandler () {
    [
    'touchstart'
    ,'touchend'
    , 'touchmove'
    ].forEach(function(event) {
      var mainCanvas = Module.canvasFront || Module.canvas;
      mainCanvas.addEventListener(event, touchHandler, true);
    });
  }

})();
