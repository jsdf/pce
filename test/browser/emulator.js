'use strict';

// Helpers for driving an emulator test page (page.html) from Playwright

const fs = require('fs');
const path = require('path');

const ARCHS = ['macplus', 'ibmpc', 'atarist'];

const root = path.resolve(__dirname, '../..');

// lines in the emulator log that mean something is wrong
const BAD_LOG_LINES = [
  /too slow/,
  /^\*\*\*/, // pce error messages
  /Aborted|abort\(/,
  /function signature mismatch/,
  /RuntimeError/,
];

// errors caused by the example system configs, not by the emulator: the mac
// plus config has a pram file that isn't in the zip and an empty disk 0x81
const EXPECTED_LOG_LINES = [
  /^\*\*\* reading rtc file failed$/,
  /^\*\*\* loading drive 0x81 failed$/,
];

// fail with a helpful message if the build or the system files are missing
function checkPrerequisites(arch) {
  const umd = path.join(root, 'commonjs', `pcejs-${arch}`, `pcejs-${arch}.umd.js`);
  const config = path.join(__dirname, '..', '.cache', 'systems', arch, 'pce-config.cfg');

  if (!fs.existsSync(umd)) {
    throw new Error(
      `${path.relative(root, umd)} is missing: build the emulators and run ./umd.sh in commonjs/pcejs-${arch}`
    );
  }

  if (!fs.existsSync(config)) {
    throw new Error(`the ${arch} system files are missing: run npm run test:setup`);
  }
}

// open the test page for arch and wait until the emulator is running
async function openEmulator(page, arch) {
  const pageErrors = [];
  page.on('pageerror', (err) => pageErrors.push(err.message));

  await page.goto(`/${arch}/`);

  await page.waitForFunction(
    () =>
      window.emulator &&
      typeof window.emulator._pce_em_get_emulated_ms === 'function' &&
      window.emulator._pce_em_get_emulated_ms() > 0,
    null,
    { timeout: 60000 }
  );

  return {
    // wait until the emulator has emulated ms milliseconds since it started
    async waitForEmulatedMs(ms) {
      await page.waitForFunction(
        (ms) => window.emulator._pce_em_get_emulated_ms() >= ms,
        ms,
        { timeout: ms * 2 + 30000 }
      );
    },

    // emulated ms and real ms (performance.now()) at the same moment
    async sample() {
      return page.evaluate(() => ({
        emulated: window.emulator._pce_em_get_emulated_ms(),
        real: performance.now(),
      }));
    },

    // page errors and log lines that indicate a problem
    async problems() {
      const log = await page.evaluate(() => window.pceLog);
      return pageErrors.concat(
        log.filter(
          (line) =>
            BAD_LOG_LINES.some((re) => re.test(line)) &&
            !EXPECTED_LOG_LINES.some((re) => re.test(line))
        )
      );
    },
  };
}

// make the page look hidden to the emulator, the way a background tab does:
// document.hidden is true and animation frames don't run
async function hidePage(page) {
  await page.evaluate(() => {
    window.realRequestAnimationFrame = window.requestAnimationFrame;
    window.requestAnimationFrame = () => 0;
    Object.defineProperty(document, 'hidden', { configurable: true, get: () => true });
    Object.defineProperty(document, 'visibilityState', {
      configurable: true,
      get: () => 'hidden',
    });
    document.dispatchEvent(new Event('visibilitychange'));
  });
}

async function showPage(page) {
  await page.evaluate(() => {
    window.requestAnimationFrame = window.realRequestAnimationFrame;
    delete document.hidden;
    delete document.visibilityState;
    document.dispatchEvent(new Event('visibilitychange'));
  });
}

module.exports = { ARCHS, checkPrerequisites, openEmulator, hidePage, showPage };
