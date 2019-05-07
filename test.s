.entry start

.section text
.align 0x00
:start
        NOP
        Pushi   1024
        dup
        popr    $r2

        jmpal   #add_until_carry

        int     c    0XFF

:add_until_carry           ; doubles $r1 value until carry
        movr    $r1   $r2
        addr    $r1   $r2
        ret

.section data
.align 0b1011_0010
:hello .asciiz "Hello, World!"