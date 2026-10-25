'use strict';

// Each emulator boots the example system to its desktop or prompt.
// The screen is compared after a fixed amount of emulated time, so that it
// doesn't depend on how fast the host is. The expected screens are in
// __screenshots__/; update them with npm run test:browser -- --update-snapshots

const { test, expect } = require('@playwright/test');
const { ARCHS, checkPrerequisites, openEmulator } = require('./emulator');

const SCREENS = {
  macplus: 'the Finder',
  ibmpc: 'the DOS prompt',
  atarist: 'the GEM desktop',
};

// emulated time after which the system has finished booting
const BOOT_MS = {
  macplus: 30000,
  ibmpc: 20000,
  atarist: 30000,
};

for (const arch of ARCHS) {
  test(`${arch} boots to ${SCREENS[arch]}`, async ({ page }) => {
    checkPrerequisites(arch);

    const emulator = await openEmulator(page, arch);
    await emulator.waitForEmulatedMs(BOOT_MS[arch]);

    // retries until the screen matches, e.g. the blinking dos cursor
    await expect(page.locator('canvas')).toHaveScreenshot(`${arch}-booted.png`, {
      timeout: 90000,
    });

    expect(await emulator.problems()).toEqual([]);
  });
}
