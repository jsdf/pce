'use strict';

// Browser tests for the emulators: npm run test:setup once, then
// npm run test:browser (after building the emulators and UMD bundles)

const { defineConfig, devices } = require('@playwright/test');

const PORT = 8181;

module.exports = defineConfig({
  testDir: 'test/browser',
  timeout: 120000,

  // one at a time, the timing tests measure real time
  workers: 1,
  fullyParallel: false,

  // the canvas pixels don't depend on the platform
  snapshotPathTemplate: '{testDir}/__screenshots__/{arg}{ext}',

  use: {
    baseURL: `http://127.0.0.1:${PORT}`,
  },

  projects: [
    {
      name: 'chromium',
      use: { ...devices['Desktop Chrome'], deviceScaleFactor: 1 },
    },
  ],

  webServer: {
    command: `node test/browser/server.js ${PORT}`,
    url: `http://127.0.0.1:${PORT}/healthz`,
    reuseExistingServer: !process.env.CI,
  },
});
