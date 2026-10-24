### pcejs-atarist
an atari st emulator for the browser

#### getting started
given a commonjs module
```bash
npm init
```
install dependencies
```bash
npm install --save pcejs-atarist pcejs-util
```

add some js `index.js`
```js
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
curl -O https://jamesfriend.com.au/pce-js/dist/atarist-system.zip
unzip atarist-system.zip
```

#### sound
The YM2149 PSG sound chip is emulated, and its output is played through the
Web Audio API. By default the browser build uses the `sdl` sound driver, even
if `pce-config.cfg` has no `psg` section. To configure it, add a section like
this to `pce-config.cfg`:
```
psg {
	driver = "sdl"          # "null" disables sound
	sample_rate = 48000
	highpass = 1
	lowpass = 8000          # 0 disables the lowpass filter
}
```

Browsers don't allow a page to start audio before the user has interacted with
it, so sound starts after the first click or key press on the page. The Web
Audio context is available as `Module.audioContext` (the object returned by
`atarist(...)`), e.g. if you want to resume it from your own "sound on" button:
```js
var emulator = atarist({ /* ... */ })
button.onclick = function () { emulator.audioContext && emulator.audioContext.resume() }
```

bundle it with browserify
```bash
npm install -g browserify@4.x
browserify index.js \
  --noparse="node_modules/pcejs-atarist/lib/pcejs-atarist.js" \
  > bundle.js
```

serve it up
```bash
npm install -g http-server
open http://localhost:8080
http-server .
```

done

