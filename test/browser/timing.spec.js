'use strict';

// The browser main loop emulates the real time that passes, both while the
// page is visible (driven by animation frames) and while it is hidden
// (driven by a timer, because browsers don't run animation frames for
// hidden pages).

const { test, expect } = require('@playwright/test');
const {
  ARCHS,
  checkPrerequisites,
  openEmulator,
  hidePage,
  showPage,
} = require('./emulator');

const MEASURE_MS = 3000;

// emulated time / real time over MEASURE_MS
async function measureSpeed(page, emulator) {
  const a = await emulator.sample();
  await page.waitForTimeout(MEASURE_MS);
  const b = await emulator.sample();

  return (b.emulated - a.emulated) / (b.real - a.real);
}

for (const arch of ARCHS) {
  test.describe(arch, () => {
    test('keeps up with real time while visible', async ({ page }) => {
      checkPrerequisites(arch);
      const emulator = await openEmulator(page, arch);

      const speed = await measureSpeed(page, emulator);

      expect(speed).toBeGreaterThan(0.9);
      expect(speed).toBeLessThan(1.05);
      expect(await emulator.problems()).toEqual([]);
    });

    test('keeps running while the page is hidden', async ({ page }) => {
      checkPrerequisites(arch);
      const emulator = await openEmulator(page, arch);

      await hidePage(page);
      const hiddenSpeed = await measureSpeed(page, emulator);

      await showPage(page);
      const visibleSpeed = await measureSpeed(page, emulator);

      expect(hiddenSpeed).toBeGreaterThan(0.9);
      expect(hiddenSpeed).toBeLessThan(1.05);
      expect(visibleSpeed).toBeGreaterThan(0.9);
      expect(visibleSpeed).toBeLessThan(1.05);
      expect(await emulator.problems()).toEqual([]);
    });
  });
}
