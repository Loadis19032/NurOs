[BITS 32]

global _start
extern kmain

jmp start

start:
    mov edi, 0xB8000
    mov esi, debug_msg
    mov ah, 0x0A   

    
.debug_loop:
    lodsb
    cmp al, 0
    je .debug_done
    mov [edi], ax
    add edi, 2
    jmp .debug_loop
    
.debug_done:
    mov esp, 0x90000 

    mov ax, 0x10   
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    push 0
    popf

    call kmain
    cli
.hang:
    hlt
    jmp .hang

debug_msg: db 'KERNEL ENTRY OK!', 0