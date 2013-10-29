echo "converting bitcode to javascript with datafiles for target: ${PCEJS_TARGET}"

cp ${PCEJS_PREFIX}/bin/pce-${PCEJS_TARGET} ${PCEJS_PREFIX}/bin/pce-${PCEJS_TARGET}.bc
emcc [-Ox] ${PCEJS_PREFIX}/bin/pce-${PCEJS_TARGET}.bc -o ${PCEJS_PREFIX}/bin/pce-${PCEJS_TARGET}.${PCEJS_OUTPUT_FORMAT} \
	$PCEJS_EMFLAGS \
	--preload-file roms

mv ${PCEJS_PREFIX}/bin/pce-${PCEJS_TARGET}.${PCEJS_OUTPUT_FORMAT} ${PCEJS_PACKAGEDIR}/pce-${PCEJS_TARGET}.${PCEJS_OUTPUT_FORMAT}
mv ${PCEJS_PREFIX}/bin/pce-${PCEJS_TARGET}.data ${PCEJS_PACKAGEDIR}/pce-${PCEJS_TARGET}.data

# extension rom files
if [ "${PCEJS_TARGET}" == "macplus" ]; then
	cp ${PCEJS_PREFIX}/share/pce/macplus/macplus-pcex.rom roms/macplus-pcex.rom
elif [ "${PCEJS_TARGET}" == "ibmpc" ]; then
	cp ${PCEJS_PREFIX}/share/pce/ibmpc/ibmpc-pcex.rom roms/ibmpc-pcex.rom
fi