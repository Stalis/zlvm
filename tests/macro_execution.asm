.macro halt_with_code code
    movi $v0, code
    int 0xFF
.endmacro

halt_with_code 0xFF
