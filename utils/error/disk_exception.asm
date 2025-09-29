carry_on_msg db "[-] A flag carry on está setada, ocorreu um erro relacionado a inicialização dos estados dos discos" 
carry_on_len equ $ - carry_on_msg

%include "../msg/write.asm"
carry_flag_on: 
    lea si, [carry_on_msg]
    mov cx, carry_on_len
    call write_char

