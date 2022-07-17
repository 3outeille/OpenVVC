%define ARCH_X86_64 1
%define private_prefix func

%include "x86inc.asm"

	extern printf

    section .rodata
format  db  "%d", 0xa, 0x0
    
    section .text
INIT_XMM

cglobal fg_compute_block_avg_asm, 1,2	
    mov    rdi, format
    mov    esi, 15
    xor    eax, eax
    call   printf wrt ..plt
    RET
	