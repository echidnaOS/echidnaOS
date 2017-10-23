
asm (
    "jmp 1f\n\t"
    ".ascii \"ECH_EXEC\"\n\t"
    ".align 16\n\t"
    "1:\n\t"
    "pushl $0x1010\n\t"
    "movl $0x1000, %eax\n\t"
    "pushl (%eax)\n\t"
    "call main\n\t"
    "addl $8, %esp\n\t"
    "movl %eax, %ecx\n\t"
    "movl $0x00, %eax\n\t"
    "int $0x80\n\t"
);

void _start(void) {}
