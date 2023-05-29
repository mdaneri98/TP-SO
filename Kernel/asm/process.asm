GLOBAL copyState
GLOBAL setProcess
GLOBAL createInitStack
GLOBAL startSystem
GLOBAL createWaiterStack

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

    xor rcx, rcx
    xor rax, rax

    mov rax, [rdi]

    sub rax, 19*8                   ; The size of the complete process stack info
    mov [rdi], rax                  ; Update current value of the target stack
    mov rdi, rax                    ; Now we use the stack location for updating its values
    xor rax, rax

.loop:                              ; Loop to copy all the stack info of the current process
    cmp rcx, 19
    jz .end

    mov rax, [rsi+rcx*8]
    mov [rdi+rcx*8], rax

    xor rax, rax
    inc rcx
    jmp .loop

.end:
    mov rax, rdi
    add rax, 19*8
    mov [rdi+10*8], rax        ; Correct value of RBP for the new process
    mov [rdi+18*8], rdi        ; Value of the stack of the new process

    pop rdi
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
createInitStack:
    push rbp
    mov rbp, rsp

    mov rax, rdi
    sub rax, 20*8                   ; The size of the complete process stack info
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

createWaiterStack:
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

startSystem:
	int 20h
    ret

section .rodata
	userSpace dq 0x400000            ; _start wrapper address