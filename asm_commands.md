# Cheatsheet de Assembly x86

---

## Flags
| Flag | Significado | Template/Exemplo |
|------|-------------|-----------------|
| AF (Auxiliary Flag) | Operações BCD (Deprecated) | - |
| ZF (Zero Flag) | 1 se a última operação for zero | `cmp eax, ebx` → ajusta ZF |
| SF (Sign Flag) | 1 se resultado negativo | `sub eax, ebx` → ajusta SF |
| OF (Overflow Flag) | 1 se operação aritmética excede capacidade | `add al, 127` → pode setar OF |
| CF (Carry Flag) | 1 se overflow/borrow em operações sem sinal | `add ax, 0xFFFF` → CF = 1 |
| PF (Parity Flag) | 1 se número de bits 1 do resultado for par | `xor al, al` → PF ajusta |
| IF (Interrupt Flag) | 0 ignora interrupções | `cli` / `sti` |
| DF (Direction flag) | Controla a direção das operações de string, 1 = para frente, 0 = para trás |  `cld` → DF = 0, `std` → DF = 1 |

---

## Sessões e Segmentos
| Diretiva | Função | Exemplo |
|----------|--------|---------|
| `section .text` | Código executável | `section .text` |
| `section .data` | Dados inicializados | `section .data`<br>`msg db "Hello",0` |
| `section .bss` | Dados não inicializados | `section .bss`<br>`buffer resb 64` |
| `org` | Define endereço inicial | `org 0x7C00` |
| `bits` | Define modo da CPU | `bits 16` |

---

## Controle de Fluxo
| Comando | Condição | Template |
|---------|----------|---------|
| `jmp label` | Sempre | `jmp start` |
| `je/jz label` | ZF = 1 | `je equal_label` |
| `jne/jnz label` | ZF = 0 | `jne not_equal` |
| `jg label` | SF=OF e ZF=0 | `jg greater` |
| `jl label` | SF!=OF | `jl less` |
| `jge label` | SF=OF | `jge ge_label` |
| `jle label` | SF!=OF ou ZF=1 | `jle le_label` |
| `jc label` | CF=1 | `jc carry_label` |
| `jnc label` | CF=0 | `jnc no_carry` |
| `jo label` | OF=1 | `jo overflow_label` |
| `jno label` | OF=0 | `jno no_overflow` |
| `js label` | SF=1 | `js negative` |
| `jns label` | SF=0 | `jns positive` |
| `loop label` | ECX != 0 | `loop my_loop` |
| `loopz/loope label` | ZF=1 e ECX != 0 | `loopz zero_loop` |
| `loopnz/loopne label` | ZF=0 e ECX != 0 | `loopnz not_zero` |

---

## Manipulação de Registradores
| Comando | Função | Template |
|---------|--------|---------|
| `mov dest, src` | Copia valor | `mov eax, ebx` |
| `lea reg, [mem]` | Endereço de memória | `lea eax, [msg]` |
| `xchg reg, reg/mem` | Troca valores | `xchg eax, ebx` |
| `call label` | Chama função | `call my_function` |
| `ret` | Retorna de função | `ret` |
| `push reg/mem` | Empilha valor na stack | `push eax` |
| `pusha` | Empilha todos os registradores gerais nessa ordem: `AX, CX, DX, BX, SP, BP, SI e DI | `pusha` |
| `pop reg/mem` | Desempilha valor da stack | `pop eax` |
| `popa` | Restaura o estado de todos os registradores gerais | `popa` |
| `enter size, nesting` | Cria stack frame | `enter 32,0` |
| `leave` | Limpa stack frame | `leave` |

---

## Manipulação de Flags
| Comando | Função | Template |
|---------|--------|---------|
| `clc` | Limpa CF | `clc` |
| `stc` | Seta CF | `stc` |
| `lahf` | Carrega flags em AH | `lahf` |
| `sahf` | Salva flags de AH | `sahf` |

---

## Funções Aritméticas
| Comando | Função | Template |
|---------|--------|---------|
| `add dest, src` | Soma | `add eax, ebx` |
| `sub dest, src` | Subtração | `sub eax, 5` |
| `inc reg/mem` | Incrementa 1 | `inc eax` |
| `dec reg/mem` | Decrementa 1 | `dec ecx` |
| `imul src` | Multiplicação com sinal | `imul eax, ebx` |
| `mul src` | Multiplicação sem sinal | `mul bx` |
| `neg dest` | Inverte sinal | `neg eax` |
| `div src` | Divisão sem sinal | `div bx` |
| `idiv src` | Divisão com sinal | `idiv bx` |

---

## Operações Lógicas
| Comando | Função | Template |
|---------|--------|---------|
| `and dest, src` | E lógico | `and al, 0x0F` |
| `or dest, src` | OU lógico | `or eax, ebx` |
| `xor dest, src` | XOR lógico | `xor eax, eax` |
| `not dest` | NOT lógico | `not al` |
| `test dest, src` | AND, ajusta flags | `test eax, 1` |
| `shl dest, n` | Shift left | `shl eax, 1` |
| `shr dest, n` | Shift right | `shr eax, 1` |
| `sar dest, n` | Shift right com sinal | `sar eax, 1` |
| `sal dest, n` | Shift left com sinal | `sal eax, 1` |

---
## Operações de string

| Comando | Função | Template |
| ------------- | --------- | ------------ |
| `movsb` | Move um byte de si para di | `movsb` copia um byte da posição apontada por SI para DI e incrementa/decrementa SI/DI conforme DF |
| `movsw` | mesmo uso de movsb mas com word (2 bytes) | `movsw` |
| `movsd` | mesmo uso de movsb mas com double word (4 bytes) | `movsd` |
| `movsq` | mesmo uso de movsb mas com quad word (8 bytes) | `movsq` |
| `lodsb` | Carrega um byte apontado por si em al depois incrementa ou decrementa de acordo com o DF | `lodsb` |
| `lodsb` | Carrega um byte apontado por si em al depois incrementa ou decrementa de acordo com o DF | `lodsb` |
| `lodsw` | Mesma aplicação de lodsb com word (2 bytes) | `lodsw` |
| `lodsd` | Mesma aplicação de lodsb com double word (4 bytes) | `lodsd` |
| `stosb` | Armazena um byte de al em di e incrementa/decrementa | `stosb` |
| `stosw` | Mesma aplicação de stosb com word (2 bytes) | `stosw` |
| `stosd` | Mesma aplicação de stosb com double word (4 bytes) | `stosd` |
| `scasb` | Compara  al com di, ajusta flags e incrementa/decrementa | `scasb` |
| `scasw` | Compara  ax com di, ajusta flags e incrementa/decrementa | `scasw` |
| `scasd` | Compara  eax com di, ajusta flags e incrementa/decrementa | `scasd` |
| `cmpsb` | compara si com di, ajustas flags e incrementa/decrementa | `cmpsb` |
| `cmpsw` | Compara word | `cmpsw` |
| `cmpsd` | Compara double words | `cmpsd` |
| `rep` | Repete a instrução de string X ecx (valor dentro de ecx) vezes enquanto ecx != 0 | `rep X` copia ecx bytes |
| `repe`/`repz` | Repete enquanto ZF = 1 e ECX != 0 | `repe cmpsb` |  
| `repne`/`repnz` | Repete enquanto ZF = 0 ECX != 0| `repne scasb` |  
---
## Interrupções
| Comando | Função | Template |
|---------|--------|---------|
| `int 0xNN` | Chama interrupção | `int 0x10` |
| `iret` | Retorna da interrupção | `iret` |
| `cli` | Desativa interrupções | `cli` |
| `sti` | Ativa interrupções | `sti` |
| `hlt` | Pausa CPU até interrupção | `hlt` |
| `nop` | Não faz nada | `nop` |

---

## Comparação
| Comando | Função | Template |
|---------|--------|---------|
| `cmp dest, src` | Subtração sem alterar valor | `cmp eax, ebx` |
| `cmps` | Compara blocos de memória | `cmpsb` |
| `scas` | Compara registrador com memória | `scasb` |

---

## Diretrizes de Dados
| Diretiva | Função | Template |
|----------|--------|---------|
| `db` | Define 1 byte | `msg db "Hello",0` |
| `dw` | Define 2 bytes | `num dw 0x1234` |
| `dd` | Define 4 bytes | `val dd 0x12345678` |
| `dq` | Define 8 bytes | `qval dq 0x123456789ABCDEF0` |
| `dt` | Define 10 bytes | `tval dt 1234567890` |
| `resb N` | Reserva N bytes | `resb 16` |
| `resw N` | Reserva N palavras | `resw 4` |
| `resd N` | Reserva N double words | `resd 2` |
| `equ` | Constante simbólica | `BUF_SIZE equ 64` |
| `global` | Exporta símbolo | `global _start` |
| `extern` | Importa símbolo | `extern printf` |
| `align N` | Alinha próximo dado/código | `align 4` |

---

## Pseudo-instruções
| Comando | Função | Template |
|---------|--------|---------|
| `times N db X` | Repete X N vezes | `times 510 db 0` |
| `seg label` | Retorna o segmento do label | `seg buffer` |
---

## Expressões Úteis
| Expressão | Função | Exemplo |
|-----------|--------|---------|
| `$` | Endereço atual | `jmp $+2` |
| `$$` | Início do segmento (org) | `times 510-($-$$) db 0` |
| `$-$$` | Tamanho atual do programa | `times 510-($-$$) db 0` |
| `(var) db "string",0` | Define string com terminador nulo | `msg db "Hello",0` |
| valor | Pega o valor da variável valor; No exemplo | `mov eax, valor` |
| [ptr] | Aponta para o endereço do início da variável ptr | `mov eax, [ptr] | 
---
