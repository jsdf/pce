echo "converting bitcode to javascript for target: ${PCEJS_TARGET}"

# extension rom files
if [ "${PCEJS_TARGET}" == "macplus" ]; then
	cp ${PCEJS_PREFIX}/share/pce/macplus/macplus-pcex.rom ${PCEJS_PACKAGEDIR}/data/macplus/macplus-pcex.rom
elif [ "${PCEJS_TARGET}" == "ibmpc" ]; then
	cp ${PCEJS_PREFIX}/share/pce/ibmpc/ibmpc-pcex.rom ${PCEJS_PACKAGEDIR}/data/ibmpc/ibmpc-pcex.rom
fi

cp ${PCEJS_PREFIX}/bin/pce-${PCEJS_TARGET} ${PCEJS_PREFIX}/bin/pce-${PCEJS_TARGET}.bc
# EMCC_DEBUG=2 
${PCEJS_EMSDK_PATH}emcc ${PCEJS_PREFIX}/bin/pce-${PCEJS_TARGET}.bc -o ${PCEJS_PREFIX}/bin/pce-${PCEJS_TARGET}.${PCEJS_OUTPUT_FORMAT} \
	$PCEJS_EMFLAGS

mv ${PCEJS_PREFIX}/bin/pce-${PCEJS_TARGET}.${PCEJS_OUTPUT_FORMAT} ${PCEJS_PACKAGEDIR}/pce-${PCEJS_TARGET}.${PCEJS_OUTPUT_FORMAT}
