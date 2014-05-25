var xtend = require('xtend')
var pcejs = require('./lib/pcejs-macplus')

var configOpts = {
  'arguments': ['-c','pce-config.cfg','-r'],
}

module.exports = function (opts) {
  // inject dependencies and config
  var config = xtend(configOpts, opts)
  var deps = {
    extend: xtend,
  }
  return pcejs(deps, config)
}