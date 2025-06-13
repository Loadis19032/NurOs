[BITS 16]
[ORG 0x7C00]

; Константы
KERNEL_LOAD_SEGMENT equ 0x1000  
KERNEL_SECTORS equ 75      
KERNEL_FINAL_ADDR equ 0x100000  


start:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00  

    mov [boot_disk], dl

    mov ah, 0x00 
    mov al, 0x03  
    int 0x10

    mov si, msg_loading
    call print_string

    call load_kernel

    call switch_to_pm

    jmp $  

load_kernel:
    mov ah, 0x02     
    mov al, KERNEL_SECTORS  
    mov ch, 0x00        
    mov cl, 0x02        
    mov dh, 0x00        
    mov dl, [boot_disk] 
    mov bx, KERNEL_LOAD_SEGMENT
    mov es, bx
    mov bx, 0x0000      ; ES:BX = 0x1000:0x0000 = 0x10000

    int 0x13            

    jc disk_error       

    cmp al, KERNEL_SECTORS
    jne disk_error

    mov si, msg_kernel_loaded
    call print_string
    ret

disk_error:
    mov si, msg_disk_error
    call print_string
    jmp $

print_string:
    mov ah, 0x0E     
.loop:
    lodsb             
    cmp al, 0        
    je .done
    int 0x10           
    jmp .loop
.done:
    ret

switch_to_pm:
    cli

    lgdt [gdt_descriptor]

    mov eax, cr0
    or eax, 0x1
    mov cr0, eax

    jmp CODE_SEG:init_pm

[BITS 32]
init_pm:
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov ebp, 0x90000
    mov esp, ebp

    call enable_a20

    call copy_kernel

    mov esi, msg_press_any_key
    call print_string_32

    mov esi, msg_jumping
    call print_string_32

    jmp KERNEL_FINAL_ADDR


enable_a20:
    call wait_8042
    mov al, 0xAD
    out 0x64, al  

    call wait_8042
    mov al, 0xD0
    out 0x64, al    

    call wait_8042_data
    in al, 0x60
    push eax

    call wait_8042
    mov al, 0xD1
    out 0x64, al  

    call wait_8042
    pop eax
    or al, 2     
    out 0x60, al

    call wait_8042
    mov al, 0xAE
    out 0x64, al    

    call wait_8042
    ret

wait_8042:
    in al, 0x64
    test al, 2
    jnz wait_8042
    ret

wait_8042_data:
    in al, 0x64
    test al, 1
    jz wait_8042_data
    ret

print_string_32:
    mov ah, 0x0F   
    mov edi, 0xB8000  
    add edi, 160*5    
.loop:
    lodsb
    cmp al, 0
    je .done
    mov [edi], ax
    add edi, 2
    jmp .loop
.done:
    ret
copy_kernel:
    mov esi, 0x10000   
    mov edi, KERNEL_FINAL_ADDR 
    mov ecx, KERNEL_SECTORS * 512 / 4 
    rep movsd  
    ret

; GDT (Global Descriptor Table)
gdt_start:
    dd 0x0
    dd 0x0

gdt_code:
    dw 0xFFFF       
    dw 0x0        
    db 0x0          
    db 10011010b  
    db 11001111b  
    db 0x0          

gdt_data:
    dw 0xFFFF       
    dw 0x0         
    db 0x0          
    db 10010010b
    db 11001111b    
    db 0x0          

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1  
    dd gdt_start               

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

boot_disk: db 0

msg_loading: db 'Loading kernel...', 13, 10, 0
msg_kernel_loaded: db 'Kernel loaded successfully!', 13, 10, 0
msg_disk_error: db 'Disk read error!', 13, 10, 0
msg_jumping: db 'Jumping to kernel...', 0
msg_press_any_key db "press any key...", 0

times 510-($-$$) db 0
dw 0xAA55