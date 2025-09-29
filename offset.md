
No real mode, registradores chegam em até 16 bits (64 KB)

no real mode a memória pode ser de até 1MB

Para alcançar um endereço mais longe que os 64KB dos registradores, calculamos: 

Endereço físico na memória = segmento atual * 16(ou 0x10) (Desloca 4 bits) + posição atual nesse segmento (offset)
Ex:
Se temos
```asm
mov ax, 0x2000
mov bx, 0x0100 
```

endereço físico = 0x2000 * 16 + 0x0100 = 0x20000 + 0x0100 (Anda 16 bytes na memória (Equivalente a um registrador))= 0x20100

usamos * 16 para deslocar 4 bits