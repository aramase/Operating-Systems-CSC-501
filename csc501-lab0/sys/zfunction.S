.globl zfunction
zfunction:
        push %ebp 
        mov %esp, %ebp
        mov 8(%ebp), %eax
        and $0xFFFFF00F, %eax
        shl $0x8, %eax
        and $0xFFFFFF00, %eax
        pop %ebp
        ret