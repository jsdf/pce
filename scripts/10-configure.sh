#!/bin/bash
set -euo pipefail
set -x

$PCEJS_CONFIGURE \
	CFLAGS="$PCEJS_CFLAGS" \
	EMSCRIPTEN="$PCEJS_EMSCRIPTEN" \
	--prefix="$PCEJS_PREFIX" \
	--disable-rc759  --disable-sim405  --disable-sim6502 --disable-sims32  --disable-simarm \
	--disable-char-termios --disable-char-tcp \
	--without-x \
	--with-sdl
