GLOBAL semLock
GLOBAL semUnlock


section .text

;---------------------------------------------------------------------------------------------------|
; Func semLock:                                                                                     |
;                                                                                                   |
;    args:                                                                                          |
;       -rdi: pointer to sem current value                                                          |
;       -rdx: mutex to modify sem                                                                   |
;   return: void                                                                                    |
;---------------------------------------------------------------------------------------------------|

semLock:

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

.interruptAndSetReg:
    MOV QWORD [rdx], 0                ; Liberamos el mutex.
    int 0x20

.scheduler: 
    int 0x20


;---------------------------------------------------------------------------------------------------|
; Func semUnlock:                                                                                   |
;                                                                                                   |
;    args:                                                                                          |
;       -rdi: pointer to sem current value                                                          |                                                                        |
;   return: void                                                                                    |
;---------------------------------------------------------------------------------------------------|
semUnlock:

    MOV rax, 1
    XCHG rax, [rdx]
    CMP rax, 0
    JNE .interrupt
    ADD QWORD [rdi], 1              ; Incrementamos el semaforo.
    MOV QWORD [rdx], 0                    ; Liberamos el mutex.    
    RET

.interrupt: 
    int 0x20
