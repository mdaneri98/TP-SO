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

    CMP [rdi], 0
    JE .bloquea
    DEC [rdi]
    MOV [rdx], 0
    RET

.interrupt: 
    int 0x20

.interruptAndSetReg:
    MOV [rdx], 0                ; Liberamos el mutex.
    int 0x20

.scheduler: 
    int 0x20


;---------------------------------------------------------------------------------------------------|
; Func semUnlock:                                                                                   |
;                                                                                                   |
;    args:                                                                                          |
;       -rdi: pointer to sem current value                                                          |
;       -rdx: mutex to modify sem                                                                   |                                                                        
;   return: void                                                                                    |
;---------------------------------------------------------------------------------------------------|
semUnlock:

    MOV rax, 1
    XCHG rax, [rdx]
    CMP rax, 0
    JNE .interrupt
    ADD [rdi]                   ; Incrementamos el semaforo.
    MOV [rdx], 0                ; Liberamos el mutex.    
    RET