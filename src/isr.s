psect	intcode,class=CODE,space=0,reloc=2
psect intcode
_intcode:
    movlw 0x00
    global _isr
    goto _isr
