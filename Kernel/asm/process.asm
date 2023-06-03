GLOBAL _setNewStack
GLOBAL _setProcess
GLOBAL _createInitStack
GLOBAL _createIdleStack

section .text

;----------------------------------------------------------------------------------------------------|
; Func setNewStack: Sets the stack for fork, changing its structure like its on a int20h istead of   |
;                 the int80h where this function was called, the return value of the iretq will      |
;                 to the userland fork syscall, so we are overriding rax with the return value of 0  |
;                 to let the user know that its in the child process created by fork                 |
;    args:                                                                                           |
;       -rdi: Pointer of the Target stack                                                            |
;   return: void                                                                                     |
;----------------------------------------------------------------------------------------------------|
_setNewStack:
    push rbp
    mov rbp, rsp
    push rax
    push rcx
    push rdx

    xor rax, rax

    mov rax, rdi
    add rax, 21*8           ; We compute the new RBP value for this process
    mov [rdi+18*8], rax     ; We override the old value of the target RSP for the new one
    mov [rdi+4*8], rax      ; We override that value on the stack

    xor rcx, rcx
    xor rdx, rdx
    xor rax, rax
    mov rcx, 14
    mov rdx, 0

.loop:
    mov rax, [rdi+rdx*8+8]
    mov [rdi+rdx*8], rax
    inc rdx
    cmp rdx, rcx
    jne .loop

    mov rax, 0
    mov [rdi+14*8], rax   ; The return value for fork

    pop rdx
    pop rcx
    pop rax
    mov rsp, rbp
    pop rbp
    ret


;---------------------------------------------------------------------------------------------------|
; Func setProcess: Prepares the current process' image to be replaced by a new process              |
;    args:                                                                                          |
;       -rdi: Pointer to the process binary                                                         |
;       -rsi: argc                                                                                  |
;       -rdx: argv                                                                                  |
;       -rcx: rsp                                                                                   |
;   return: 0 if success, -1 if error (idk how we can have an error yet)                            |
;---------------------------------------------------------------------------------------------------|
_setProcess:
    push rbp
    mov rbp, rsp
    push rdi                        ; int (*processFunc)(int, char **)
    push rsi                        ; int argc
    push rdx                        ; int argv  
    push rcx                        ; Process rsp

    mov [rcx+9*8], rdi              ; processFunc
    mov [rcx+8*8], rsi              ; argc
    mov [rcx+11*8], rdx             ; *argv[]
    xor rax, rax
    mov rax, [userSpace]
    mov [rcx+14*8], rax ; We override the return of the interrupt with the _start wrapper on userSpace

    pop rcx
    pop rdx
    pop rsi
    pop rdi
    mov rax, 0  ; Success
    mov rsp, rbp
    pop rbp
    ret

;---------------------------------------------------------------------------------------------------|
; Func copyState: Creates a stack for the first process of the system that will be put into the     |
;             scheduler                                                                             |
;    args:                                                                                          |
;       -rdi: Location of the stack                                                                 |
;   return: New rsp value                                                                           |
;---------------------------------------------------------------------------------------------------|
_createInitStack:
    push rbp
    mov rbp, rsp

    mov rax, rdi
    sub rax, 19*8                   ; The size of the complete process stack info
    mov rdi, rax
    add rax, 19*8
    mov [rdi+10*8], rax             ; Correct value of RBP for the new process

    mov QWORD [rdi+9*8], 0          ; NULL, that indicates to _start on the userSpace that is the init process
    xor rax, rax
    mov rax, [userSpace]
    mov [rdi+15*8], rax             ; We override the return of the interrupt with the _start wrapper on userSpace
    mov QWORD [rdi+16*8], 0x8       ; CS
    mov QWORD [rdi+17*8], 0x202     ; RFLAGS
    mov QWORD [rdi+18*8], rdi       ; RSP
    mov QWORD [rdi+19*8], 0x0       ; SS

    mov rax, rdi
    mov rsp, rbp
    pop rbp
    ret

_createIdleStack:
    push rbp
    mov rbp, rsp

    mov rax, rdi
    sub rax, 20*8                   ; The size of the complete process stack info
    mov rdi, rax
    add rax, 19*8
    mov [rdi+10*8], rax             ; Correct value of RBP for the new process

    mov QWORD [rdi+9*8], 1          ; 1, that indicates to the _start wrapper that is the waiter process
    xor rax, rax
    mov rax, [userSpace]
    mov [rdi+15*8], rax             ; We override the return of the interrupt with the _start wrapper on userSpace
    mov QWORD [rdi+16*8], 0x8       ; CS
    mov QWORD [rdi+17*8], 0x202     ; RFLAGS
    mov QWORD [rdi+18*8], rdi       ; RSP
    mov QWORD [rdi+19*8], 0x0       ; SS

    mov rax, rdi
    mov rsp, rbp
    pop rbp
    ret

section .rodata
	userSpace dq 0x400000            ; _start wrapper address