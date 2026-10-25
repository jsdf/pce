// the directory of the umd bundle this module is loaded from, determined
// while the script is executing. only when the script is a pcejs umd bundle:
// when the module is bundled into an app with browserify or webpack, the
// current script is the app bundle, which usually isn't next to the .wasm
var pcejsScriptDirectory = (function() {
  var src =
    typeof document !== 'undefined' &&
    document.currentScript &&
    document.currentScript.src
      ? document.currentScript.src.replace(/[?#].*$/, '')
      : '';

  return /\/pcejs-[a-z]+\.umd(\.min)?\.js$/.test(src)
    ? src.replace(/[^\/]*$/, '')
    : '';
})();
