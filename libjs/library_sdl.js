// additions and overrides to emscripten's builtin SDL library

LibrarySDL = {
  SDL_EventState: function() {},
};

var PCEJS_SDL_CreateRGBSurfaceFrom = true; // enable PCEJS custom implementation of SDL_CreateRGBSurfaceFrom
if (PCEJS_SDL_CreateRGBSurfaceFrom) {
  LibrarySDL.SDL_CreateRGBSurfaceFrom = function(pixels, width, height, depth, pitch, rmask, gmask, bmask, amask) {
    // TODO: Take into account depth and pitch parameters.

    var surface = SDL.makeSurface(width, height, 0, false, 'CreateRGBSurfaceFrom', rmask, gmask, bmask, amask);

    var surfaceData = SDL.surfaces[surface];
    var surfaceImageData = surfaceData.ctx.getImageData(0, 0, width, height);
    var surfacePixelData = surfaceImageData.data;

    // Fill pixel data to created surface.
    // Supports SDL_PIXELFORMAT_RGBA8888 and SDL_PIXELFORMAT_RGB888
    var channels = amask ? 4 : 3; // RGBA8888 or RGB888
    for (var pixelOffset = 0; pixelOffset < width*height; pixelOffset++) {
      surfacePixelData[pixelOffset*4] = {{{ makeGetValue('pixels', 'pixelOffset*channels', 'i8', null, true) }}}; // R
      surfacePixelData[pixelOffset*4+1] = {{{ makeGetValue('pixels', 'pixelOffset*channels+1', 'i8', null, true) }}}; // G
      surfacePixelData[pixelOffset*4+2] = {{{ makeGetValue('pixels', 'pixelOffset*channels+2', 'i8', null, true) }}}; // B
      surfacePixelData[pixelOffset*4+3] = amask ? {{{ makeGetValue('pixels', 'pixelOffset*channels+3', 'i8', null, true) }}} : 0xff; // A
    };

    surfaceData.ctx.putImageData(surfaceImageData, 0, 0);

    return surface;
  }
};

// Sound output (used by the sound-sdl driver, e.g. the Atari ST PSG).
//
// Browsers create an AudioContext in the 'suspended' state unless it is
// created in response to a user gesture (autoplay policy). The emulator opens
// its audio device at startup, so resume the context on the first user input.
// The context is also exposed as Module.audioContext, so that the embedding
// page can resume it from its own UI (e.g. a 'sound on' button).
LibraryManager.library.$SDL.openAudioContext = function() {
  // Initialize Web Audio API if we haven't done so yet. Note: Only initialize Web Audio context ever once on the web page,
  // since initializing multiple times fails on Chrome saying 'audio resources have been exhausted'.
  if (!SDL.audioContext) {
    if (typeof(AudioContext) !== 'undefined') SDL.audioContext = new AudioContext();
    else if (typeof(webkitAudioContext) !== 'undefined') SDL.audioContext = new webkitAudioContext();

    if (SDL.audioContext) {
      Module['audioContext'] = SDL.audioContext;
      SDL.resumeAudioContextOnUserInput();
    }
  }
};

LibraryManager.library.$SDL.resumeAudioContextOnUserInput = function() {
  var ctx = SDL.audioContext;
  var events = ['keydown', 'mousedown', 'pointerdown', 'touchend', 'click'];

  if (typeof document === 'undefined' || typeof ctx['resume'] !== 'function') return;
  if (ctx['state'] === 'running') return;

  var removeListeners = function() {
    events.forEach(function(name) {
      document.removeEventListener(name, resume, true);
    });
  };

  var resume = function() {
    if (ctx['state'] === 'running') {
      removeListeners();
      return;
    }
    ctx['resume']().then(function() {
      if (ctx['state'] === 'running') removeListeners();
    }, function() {});
  };

  events.forEach(function(name) {
    document.addEventListener(name, resume, true);
  });
};

mergeInto(LibraryManager.library, LibrarySDL);
