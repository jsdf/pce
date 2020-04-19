var Module = opts || {};

// hide node/commonjs globals so emscripten doesn't get confused
var process = void 0;
var require = void 0;

var pathGetFilenameRegex = /\/([^\/]+)$/;

function pathGetFilename(path) {
  var matches = path.match(pathGetFilenameRegex);
  if (matches && matches.length) {
    return matches[1];
  } else {
    return path;
  }
}

function addAutoloader(module) {
  var loadDatafiles = function() {
    module.autoloadFiles.forEach(function(filepath) {
      module.FS_createPreloadedFile(
        '/',
        pathGetFilename(filepath),
        filepath,
        true,
        true
      );
    });
  };

  if (module.autoloadFiles) {
    module.preRun = module.preRun || [];
    module.preRun.unshift(loadDatafiles);
  }

  return module;
}

function addCustomAsyncInit(module) {
  if (module.asyncInit) {
    module.preRun = module.preRun || [];
    module.preRun.push(function waitForCustomAsyncInit() {
      module.addRunDependency('pcejsAsyncInit');

      module.asyncInit(module, function asyncInitCallback() {
        module.removeRunDependency('pcejsAsyncInit');
      });
    });
  }
}

// inject extra behaviours
addAutoloader(Module);
addCustomAsyncInit(Module);
