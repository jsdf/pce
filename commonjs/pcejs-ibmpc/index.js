var xtend = require('xtend')
var pcejs = require('./lib/pcejs-ibmpc')

var configOpts = {
  'arguments': ['-c','pce-config.cfg','-r', '-g','vga'],
}

module.exports = function (opts) {
  // inject dependencies and config
  var config = xtend(configOpts, opts)
  var deps = {
    extend: xtend,
  }
  return pcejs(deps, config)
}