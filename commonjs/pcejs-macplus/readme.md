### pcejs-macplus
a classic mac emulator for the browser

#### getting started
given a commonjs module
```bash
npm init
```
install dependencies
```bash
npm install --save pcejs-macplus pcejs-util
```
# example using a js bundler

add some js `index.js`
```js
var macplus = require('pcejs-macplus')
var utils = require('pcejs-util')

// add a loading progress bar. not required, but good ux
var loadingStatus = utils.loadingStatus(document.querySelector('.pcejs-loading-status'))

macplus({
  'arguments': ['-c','pce-config.cfg','-r'],
  autoloadFiles: [
    'macplus-pcex.rom',
    'mac-plus.rom',
    'hd1.qed',
    'pce-config.cfg',
  ],
  print: console.log.bind(console),
  printErr: console.warn.bind(console),
  canvas: document.querySelector('.pcejs-canvas'),
  monitorRunDependencies: function (remainingDependencies) {
    loadingStatus.update(remainingDependencies)
  },
})
```

add some html `index.html`
```html
<!DOCTYPE html>
<html>
  <head>
    <style type="text/css">
    .pcejs {
      margin-left: auto;
      margin-right: auto;
      text-align: center;
      font-family: sans-serif;
      /* the canvas *must not* have any border or padding, or mouse coords will be wrong */
      border: 0px none;
      padding: 0;
    }
    .pcejs-container { margin-top: 32px }
    /* macplus has mouse integration, so we can hide the host mouse */
    .pcejs-canvas { cursor: none }
    </style>
  </head>
  <body>
    <div class="pcejs pcejs-container">
      <div class="pcejs pcejs-loading-status">Downloading...</div>
      <div class="pcejs">
        <canvas class="pcejs pcejs-canvas" oncontextmenu="event.preventDefault()"></canvas>
      </div>
    </div>
    <script type="text/javascript" src="bundle.js"></script>
  </body>
</html>
```

add a mac os disk image, rom, and pce-config.cfg config file. 
```bash
curl -O https://jamesfriend.com.au/pce-js/dist/macplus-system.zip
unzip macplus-system.zip
```

the pce-config.cfg config file should reference the disk images you've loaded (in the `autoloadFiles` array). in this example the disk image is called `hd1.qed`, but you can replace it with your own hard drive and floppy disk images in .img, .dsk or .qed format. if you change the disk image, make sure update both pce-config.cfg and the `autoloadFiles` array.

copy the pce rom patch file and wasm file from the npm package to your project
```bash
cp node_modules/pcejs-macplus/macplus-pcex.rom ./macplus-pcex.rom
cp node_modules/pcejs-macplus/pce-macplus.wasm ./pce-macplus.wasm
```

bundle it with browserify
```bash
npm install -g browserify
browserify index.js \
  --ignore-missing \
  > bundle.js
```


serve it up
```bash
npm install -g http-server
http-server .
```

open http://localhost:8080 to see the emulator running in the browser



