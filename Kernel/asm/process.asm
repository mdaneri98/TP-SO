GLOBAL copyState

section .text

;---------------------------------------------------------------------------------------------------|
; Func switchProcess: Switches the current rsp to the given one by parameter.                       |
;    args:                                                                                          |
;       -rdi: Location of the process stack to switch.                                              |
;   return: void                                                                                    |
;---------------------------------------------------------------------------------------------------|
switchProcess:    
    mov rsp, rdi        ; Doesnt need stack protection.
    ret

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

    sub rax, 20*8                   ; The size of the complete process stack info
    mov [rdi], QWORD rax            ; Update current value of the target stack
    mov rdi, rax                    ; Now we use the stack location for updating its values
    xor rax

.loop:                              ; Loop to copy all the stack info of the current process
    cmp rcx, 20
    jz .end

    mov rax, [rsi+rcx*8]
    mov [rdi+rcx*8], rax

    xor rax
    inc rcx
    jmp .loop

.end:
    mov [rdi+10*8], QWORD rdi+20*8   ; Correct value of RBP for the new process
    mov [rdi+19*8], QWORD rdi        ; Value of the stack of the new process

    pop rdi
    pop rcx
    pop rax
    mov rsp, rbp
    pop rbp
    ret


;---------------------------------------------------------------------------------------------------|
; Func setProcess: Prepares the current process' image to be replaced by a new process              |
;    args:                                                                                          |
;       -rdi: Pointer to the process itself                                                         |
;       -rsi: argc                                                                                  |
;       -rdx: argv                                                                                  |
;       -rcx: rsp                                                                                   |
;   return: 0 if success, -1 if error (idk how we can have an error yet                             |
;---------------------------------------------------------------------------------------------------|
%macro pushStateWithoutRAX 0
	push rbx
	push rcx
	push rdx
	push rbp
	push rdi
	push rsi
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
%endmacro
setProcess:
    push rbp
    mov rbp, rsp
    push rdi                        ; int (*processFunc)(int, char **)
    push rsi                        ; int argc
    push rdx                        ; int argv  
    push rcx                        ; Process rsp

    mov [rcx+9*8], rdi              ; processFunc
    mov [rcx+8*8], rsi              ; argc
    mov [rcx+11*8], rdx             ; *argv[]
    mov [rcx+14*8], QWORD userSpace ; We override the return of the interrupt with the _start wrapper on userSpace

    pop rcx
    pop rdx
    pop rsi
    pop rdi
    mov rax, 0  ; Success
    mov rsp, rbp
    pop rbp
    ret

section .rodata
	userSpace dq 0x400000   ; _start wrapper address