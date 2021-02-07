#!/usr/bin/env bash
browserify index.js \
  --noparse="pcejs-ibmpc.js" \
  --ignore-missing \
  --standalone=PCEJSIBMPC \
  > pcejs-ibmpc.umd.js