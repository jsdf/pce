// @flow

'use strict';

const spawn = require('child_process').spawn;
const exec = require('child_process').execSync;
const http = require('http');
const util = require('util');
const path = require('path');
const fs = require('fs');

const port = 8080;
const host = `http://127.0.0.1:${port}`;
const serveDir = process.argv[2];

const filesMimeTypesCache = {};
function getMimeType(filepath) {
  if (!filesMimeTypesCache[filepath]) {
    switch (path.extname(filepath)) {
      case '.css':
        filesMimeTypesCache[filepath] = 'text/css';
        break;
      case '.js':
        filesMimeTypesCache[filepath] = 'application/javascript';
        break;
      case '.wasm':
        filesMimeTypesCache[filepath] = 'application/wasm';
        break;
      default:
        filesMimeTypesCache[filepath] = exec(
          `file --mime-type --brief ${filepath}`
        )
          .toString()
          .trim();
    }
  }
  return filesMimeTypesCache[filepath];
}

const server = http.createServer(function(req, res) {
  try {
    const serverpath =
      req.url[req.url.length - 1] == '/' ? req.url + 'index.html' : req.url;
    const filepath = path.join(serveDir, serverpath);
    const mimeType = getMimeType(filepath);
    console.log(`200 ${req.url} ${mimeType}`);
    res.writeHead(200, {'Content-Type': mimeType});
    res.write(fs.readFileSync(filepath));
    res.end();
  } catch (err) {
    console.log(`404 ${req.url} ${err}`);
    res.writeHead(404, {'Content-Type': 'text/plain'});
    res.write(err.stack);
    res.end();
  }
});
server.listen(port);

console.log(
  `
opening http://127.0.0.1:${port}/ in your browser

press CTRL-C to quit this program
`
);
exec(`open http://127.0.0.1:${port}/`);
