GLOBAL copyState

section .text

;---------------------------------------------------------------------------------------------------|
; Func copyState: Copies the current process state into a new stack, preserving rsp and rbp         |
;             of the target stack, isolating the new process from its caller                        |
;    args:                                                                                          |
;       -rdi: Location of the pointer of the Target stack                                           |
;       -rsi: Pointer of the Source stack                                                           |
;   return: void                                                                                    |
;---------------------------------------------------------------------------------------------------|
copyState:
    push rbp
    mov rbp, rsp
    push rax
    push rcx
    push rdi

    xor rcx
    xor rax

    mov rax, [rdi]

    sub rax, 21*8                   ; The size of the complete process stack info
    mov [rdi], QWORD rax            ; Update current value of the target stack
    mov rdi, rax                    ; Now we use the stack location for updating its values
    xor rax

.loop:                              ; Loop to copy all the stack info of the current process
    cmp rcx, 21
    jz .end

    mov rax, [rsi+rcx*8]
    mov [rdi+rcx*8], rax

    xor rax
    inc rcx
    jmp .loop

.end:
    mov [rdi+10*8], QWORD rdi-21*8   ; Correct value of RBP for the new process
    mov [rdi+19*8], QWORD rdi        ; Value of the stack of the new process

    pop rdi
    pop rcx
    pop rax
    mov rsp, rbp
    pop rbp
    ret