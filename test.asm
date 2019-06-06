.global start
.global add_until_carry
.global print_string
.global print_line
.global halt

.extern factorial, 0xFF

.section text

start:
        NOP             ; 0x08

        Pushi   0x1024  ; 0x10
        dup         ; 0x18
        popr    $t2 ; 0x20
        storew  $t2, $t0, 5192   ; 0x28
        movr    $a0, $t2    ; 0x30

        jmpal   #add_until_carry   ; 0x38

        movi    $a0, #hello ; 0x40
        jmpal   #print_line ; 0x48

        movi    $a0, #bye   ; 0x50
        jmpal   #print_line ; 0x58

        jmp     #halt   ; 0x60

;------- Functions --------;

; doubles $a0 value until carry
; @returns $v0 result after carrying
.proc add_until_carry
        movr    $v0, $a0        ; 0x68
loop:
        addr    $v0, $a0        ; 0x70
        jmp cc  #add_until_carry.loop ; 0x78
        ret     ;0x80
.endproc

; prints null-terminated string with default 0x10 cpu interrupt
; @param $a0 address of _first char of string
.proc print_string
        movr    $t0, $a0    ; 0x88
        xorr    $a0, $a0    ; 0x90

loop:
        loadb   $a0, $t0    ; 0x98
        cmpi    $a0, 0      ; 0xA0
        jmp zs  #print_string.end   ;0xA8
        int     0x02        ; 0xB0
        inc     $t0         ; 0xB8
        jmp     #print_string.loop  ;0xC0

end:
        ret     ; 0xC8
.endproc

; prints null-terminated string with print_string and print line break after
; @param $a0 address of _first char of string
.proc print_line
        pushr   $lp ; 0xD0
        jmpal   #print_string ;0xD8
        popr    $lp     ; 0xE0

        pushr   $a0     ; 0xE8
        movi    $a0, '\n'   ; 0xF0
        int     0x02    ; 0xF8
        popr    $a0     ; 0x100
        ret             ; 0x108 LOADH????
.endproc

; halts processor
.proc halt
        movi    $v0, 0xFF
        int     0xFF
.endproc

.section data

    hello:      .asciiz "Hello, World!", '\n'
    bye:        .ascii  "Bye!", '\n', 0       ; same as `.asciiz "Bye!", '\n'`

    bytes:      .byte   0, 2, 0xF, 0o77, 0b1111_0101
    hwords:     .hword  100, 0xFFF, 0b1111_1111_1111_1010
    words:      .word   10, 0Xf1Abc, 0b11
    dwords:     .dword  0xfAFFfF_101, 0b111_1_111111_1, 4096
    buffer:     .space  100