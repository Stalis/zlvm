NOP
Pushi   1024
dup
popr    $t1

movr    $t0   $t1
addr    $t0   $t1
ret     cs
jmpal   296

movi     $sc   $r0  0xFF
syscall