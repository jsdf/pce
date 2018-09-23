### pcejs-ibmpc
a classic pc emulator for the browser

#### getting started
given a commonjs module
```bash
npm init
```
install dependencies
```bash
npm install --save pcejs-ibmpc pcejs-util
```

add some js `index.js`
```js
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

add some files
```bash
curl -O https://jamesfriend.com.au/pce-js/dist/ibmpc-system.zip
unzip ibmpc-system.zip
```

grab the extension rom file from the npm package
```bash
cp node_modules/pcejs-ibmpc/ibmpc-pcex.rom ./ibmpc-pcex.rom
```

bundle it with browserify
```bash
npm install -g browserify@4.x
browserify index.js \
  --noparse="node_modules/pcejs-ibmpc/lib/pcejs-ibmpc.js" \
  > bundle.js
```

serve it up
```bash
npm install -g http-server
open http://localhost:8080
http-server .
```

done
