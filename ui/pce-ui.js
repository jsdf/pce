var o_canvas = document.getElementById('canvas')
var o_context = o_canvas.getContext('2d');
o_canvas.width = 512;
o_canvas.height = 342;

var m_canvas = document.createElement('canvas');
var m_context = m_canvas.getContext('2d');

// connect to canvas
var Module = {
  preRun: [],
  postRun: [],
  printEnabled: true,
  print: (function() {
    return function(text) {
      if (!Module.printEnabled) return;
      console.log(text);
    };
  })(),
  printErr: function(text) {
    text = Array.prototype.slice.call(arguments).join(' ');
    console.log(text);
  },
  canvas: m_canvas,
  canvasFront: o_canvas,
  setStatus: function(text) {
    if (Module.setStatus.interval) clearInterval(Module.setStatus.interval);
    var m = text.match(/([^(]+)\((\d+(\.\d+)?)\/(\d+)\)/);
    var statusElement = document.getElementById('status');
    var progressElement = document.getElementById('progress');
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
  }
};
Module.setStatus('Downloading...');

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

