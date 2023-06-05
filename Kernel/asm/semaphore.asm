GLOBAL _semLock
GLOBAL _semUnlock


section .text

;---------------------------------------------------------------------------------------------------|
; Func semLock:                                                                                     |
;                                                                                                   |
;    args:                                                                                          |
;       -rdi: pointer to sem current value                                                          |
;       -rdx: mutex to modify sem                                                                   |
;   return: void                                                                                    |
;---------------------------------------------------------------------------------------------------|

_semLock:

.init:
    MOV rax, 1
    XCHG rax, [rdx]
    CMP rax, 0
    JNE .interrupt

    CMP QWORD [rdi], 0
    JE .interruptAndSetReg
    SUB QWORD [rdi], 1
    MOV QWORD [rdx], 0
    RET

.interrupt: 
    int 0x20
    jmp .init

.interruptAndSetReg:
    MOV QWORD [rdx], 0                ; Liberamos el mutex.
    ret


;---------------------------------------------------------------------------------------------------|
; Func semUnlock:                                                                                   |
;                                                                                                   |
;    args:                                                                                          |
;       -rdi: pointer to sem current value                                                          |
;       -rdx: mutex to modify sem                                                                   |                                                                        
;   return: void                                                                                    |
;---------------------------------------------------------------------------------------------------|
_semUnlock:

.init:
    MOV rax, 1
    XCHG rax, [rdx]
    CMP rax, 0
    JNE .interrupt
    ADD QWORD [rdi], 1                    ; Incrementamos el semaforo.
    MOV QWORD [rdx], 0                    ; Liberamos el mutex.    
    RET

.interrupt: 
    int 0x20
    jmp .init
