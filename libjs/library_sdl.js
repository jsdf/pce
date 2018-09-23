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

mergeInto(LibraryManager.library, LibrarySDL);