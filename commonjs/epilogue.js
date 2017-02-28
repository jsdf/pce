// expose apis

typeof FS !== 'undefined' && (Module.FS = FS);
typeof IDBFS !== 'undefined' && (Module.IDBFS = IDBFS);
typeof NODEFS !== 'undefined' && (Module.NODEFS = NODEFS);
typeof PATH !== 'undefined' && (Module.PATH = PATH);
typeof ERRNO_CODES !== 'undefined' && (Module.ERRNO_CODES = ERRNO_CODES);
typeof ERRNO_MESSAGES !== 'undefined' && (Module.ERRNO_MESSAGES = ERRNO_MESSAGES);
