# Build System - Documentação

## Arquivo Principal
`Makefile` - Sistema de compilação automática

## Ferramentas Utilizadas

### Compiladores
- **GCC**: Compilação de código C
- **AS**: Assembler para código assembly
- **LD**: Linker para criar binário final

### Flags de Compilação

#### C Flags (CFLAGS)
- `-m32` - Compilação para 32 bits
- `-ffreestanding` - Ambiente independente (sem libc)
- `-nostdlib` - Sem bibliotecas padrão
- `-fno-pie` - Desabilita Position Independent Executable
- `-O2` - Otimização nível 2
- `-Wall -Wextra` - Warnings adicionais
- `-masm=intel` - Sintaxe Intel para inline assembly

#### Assembly Flags (ASFLAGS)
- `--32` - Modo 32 bits
- `-msyntax=intel` - Sintaxe Intel
- `-mnaked-reg` - Registradores sem prefixo %

#### Linker Flags (LDFLAGS)
- `-m elf_i386` - Formato ELF 32 bits
- `-T linker.ld` - Script de linker customizado

## Targets do Makefile

### all
- Target padrão
- Compila kernel.bin completo

### kernel.bin
- Binário final executável
- Linka todos os arquivos objeto

### Arquivos Objeto
- `boot.o` - Código de inicialização
- `kernel.o` - Kernel principal
- `gdt_flush.o` - Carregamento GDT
- `interrupts.o` - Handlers de interrupção
- `commands.o` - Sistema de comandos

### run
- Executa kernel no QEMU
- Dependência: kernel.bin deve existir

### clean
- Remove arquivos compilados
- Limpa diretório build

## Estrutura de Dependências
1. Diretório build é criado
2. Arquivos .c são compilados para .o
3. Arquivos .s são montados para .o
4. Todos .o são linkados em kernel.bin