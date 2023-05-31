GLOBAL runDivzero
GLOBAL runInvalidOpcode

GLOBAL runPageFault

section .text
runDivzero:
    mov rax, 0
    div rax
    ret

runPageFault:
    mov QWORD [rsp], 0x0
    ret

runInvalidOpcode:
    jmp 0x000000