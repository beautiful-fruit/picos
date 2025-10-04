psect	intcode,class=CODE,space=0,reloc=2
psect intcode
_intcode:
    global _isr
    goto _isr
