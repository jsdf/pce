// add a loading progress bar. not required, but good ux
var loadingStatus = PCEJSUtil.loadingStatus(
  document.querySelector('.pcejs-loading-status')
);

PCEJSMacplus({
  arguments: ['-c', 'pce-config.cfg', '-r'],
  autoloadFiles: [
    'macplus-pcex.rom',
    'mac-plus.rom',
    'hd1.qed',
    'pce-config.cfg',
  ],
  print: console.log.bind(console),
  printErr: console.warn.bind(console),
  canvas: document.querySelector('.pcejs-canvas'),
  monitorRunDependencies: function(remainingDependencies) {
    loadingStatus.update(remainingDependencies);
  },
});
