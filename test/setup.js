#!/usr/bin/env node
'use strict';

// Set up the browser tests (npm run test:setup): download the system files
// (ROMs, disk images and configs) into test/.cache/systems/<arch>/ and
// install the browser that Playwright runs the tests in.

const fs = require('fs');
const path = require('path');
const { execFileSync } = require('child_process');

const SYSTEMS_URL = 'https://jamesfriend.com.au/pce-js/dist/';
const ARCHS = ['macplus', 'ibmpc', 'atarist'];

const cacheDir = path.resolve(__dirname, '.cache');
const systemsDir = path.join(cacheDir, 'systems');

async function downloadSystem(arch) {
  const dir = path.join(systemsDir, arch);
  const zip = path.join(cacheDir, `${arch}-system.zip`);

  if (fs.existsSync(path.join(dir, 'pce-config.cfg'))) {
    console.log(`${arch}: system files already in ${path.relative(process.cwd(), dir)}`);
    return;
  }

  console.log(`${arch}: downloading ${SYSTEMS_URL}${arch}-system.zip`);
  const res = await fetch(`${SYSTEMS_URL}${arch}-system.zip`);
  if (!res.ok) {
    throw new Error(`downloading ${arch}-system.zip failed: HTTP ${res.status}`);
  }
  fs.writeFileSync(zip, Buffer.from(await res.arrayBuffer()));

  fs.mkdirSync(dir, { recursive: true });
  execFileSync('unzip', ['-q', '-o', zip, '-d', dir], { stdio: 'inherit' });
  fs.unlinkSync(zip);
}

async function main() {
  fs.mkdirSync(systemsDir, { recursive: true });

  for (const arch of ARCHS) {
    await downloadSystem(arch);
  }

  console.log('installing the Playwright browser (chromium)');
  execFileSync('npx', ['playwright', 'install', 'chromium'], { stdio: 'inherit' });
}

main().catch((err) => {
  console.error(err.message);
  process.exit(1);
});
