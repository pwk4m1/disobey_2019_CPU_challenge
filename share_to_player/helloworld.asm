movc al, 0x48
push al
movc cl, 0x45
push cl
movc bl, 4
add al, bl
push al
push al
movc al, 0x4f
push al
movc al, 0x0A
push al

movc al, 1
movc bl, 0x01
xor si, si
int
hlt

