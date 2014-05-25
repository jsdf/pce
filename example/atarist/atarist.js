var atarist = require('pcejs-atarist')
var utils = require('pcejs-util')

// add a load progress bar. not required, but good ux
var loadingStatus = utils.loadingStatus(document.querySelector('.pcejs-loading-status'))

atarist({
  'arguments': ['-c','pce-config.cfg','-r'],
  autoloadFiles: [
    'tos-1.00-us.rom',
    'pce-config.cfg',
    'fd0.st',
    'fd1.psi',
    'hd0.qed',
  ],

  print: console.log.bind(console),
  
  printErr: console.warn.bind(console),
  
  canvas: document.querySelector('.pcejs-canvas'),

  monitorRunDependencies: function (remainingDependencies) {
    loadingStatus.update(remainingDependencies)
  },
})

