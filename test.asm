.global start
.global add_until_carry
.global print_string
.global print_line
.global halt

.extern factorial, 0xFF

.section text

start:
        NOP

        Pushi   1024
        dup
        popr    $t2
        storew  $t2, $t0, 256

        jmpal   #add_until_carry

        movi    $a0, #hello
        jmpal   #print_line

        movi    $a0, #bye
        jmpal   #print_line

        int     0XFF

;------- Functions --------;

; doubles $a0 value until carry
; @returns $v0 result after carrying
.proc add_until_carry
        movr    $v0, $a0
add_until_carry.loop:
        addr    $v0, $a0
        jmp cc  #add_until_carry.loop
        ret
.endproc

; prints null-terminated string with default 0x10 cpu interrupt
; @param $a0 address of _first char of string
.proc print_string
        xorr    $t0, $t0

print_string.loop:
        loadb   $t0, $a0
        cmpi    $t0, 0
        jmp zs  #print_string.end
        int     0x10
        inc     $t0
        jmp     #print_string.loop

print_string.end:
        ret
.endproc

; prints null-terminated string with print_string and print line break after
; @param $a0 address of _first char of string
.proc print_line
        jmpal   #print_string
        pushr   $a0
        movi    $a0, '\n'
        int     0x10
        popr    $a0
        ret
.endproc

; halts processor
.proc halt
        movi    $v0, 0xFF
        int     0x01
        ret
.endproc

.section data

    hello:      .asciiz "Hello, World!", '\n'
    bye:        .ascii  "Bye!", '\n', 0       ; same as `.asciiz "Bye!"`

    bytes:      .byte   0, 2, 0xF, 0o77, 0b1111_0101
    hwords:     .hword  100, 0xFFF, 0b1111_1111_1111_1010
    words:      .word   10, 0Xf1Abc, 0b11
    dwords:     .dword  0xfAFFfF_101, 0b111_1_111111_1, 4096
    buffer:     .space  100