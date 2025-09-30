/* Script do Linker para o NanoOS */
/* Define como organizar o kernel na memória */

ENTRY(_start)              /* Ponto de entrada do kernel */

SECTIONS
{
    . = 1M;                /* Carrega o kernel em 1MB (endereço padrão) */
    
    /* Seção de código executável */
    .text BLOCK(4K) : ALIGN(4K)
    {
        *(.multiboot)      /* Header Multiboot DEVE vir primeiro */
        *(.text)           /* Código executável */
    }
    
    /* Seção de dados somente leitura */
    .rodata BLOCK(4K) : ALIGN(4K)
    {
        *(.rodata)         /* Strings constantes, etc. */
    }
    
    /* Seção de dados inicializados */
    .data BLOCK(4K) : ALIGN(4K)
    {
        *(.data)           /* Variáveis globais com valor inicial */
    }
    
    /* Seção de dados não inicializados */
    .bss BLOCK(4K) : ALIGN(4K)
    {
        *(COMMON)          /* Símbolos comuns */
        *(.bss)            /* Variáveis globais não inicializadas */
    }
}
