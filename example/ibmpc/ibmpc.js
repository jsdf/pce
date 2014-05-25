var ibmpc = require('pcejs-ibmpc')
var utils = require('pcejs-util')

// add a load progress bar. not required, but good ux
var loadingStatus = utils.loadingStatus(document.querySelector('.pcejs-loading-status'))

ibmpc({
  'arguments': ['-c','pce-config.cfg','-r', '-g','vga'],

  autoloadFiles: [
    'basic-1.10.rom',
    'ibm-pc-1982.rom',
    'ibm-xt-1982.rom',
    'ibmpc-pcex.rom',
    'ibmvga.rom',
    'hd0.qed',
    'pce-config.cfg',
  ],

  print: console.log.bind(console),
  
  printErr: console.warn.bind(console),
  
  canvas: document.querySelector('.pcejs-canvas'),

  monitorRunDependencies: function (remainingDependencies) {
    loadingStatus.update(remainingDependencies)
  },
})

