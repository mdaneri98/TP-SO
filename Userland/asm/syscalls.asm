GLOBAL _write
GLOBAL _read
GLOBAL _clock
GLOBAL _clear
GLOBAL _beep
GLOBAL _sleep
GLOBAL _getScreenH
GLOBAL _getScreenW
GLOBAL _gameRead
GLOBAL _getPtrToPixel
GLOBAL _drawLine
GLOBAL _getPenX
GLOBAL _getPenY
GLOBAL _changeFont
GLOBAL _memoryDump
GLOBAL _getRegs
GLOBAL _idle
GLOBAL _sysKill
GLOBAL _sysPs
GLOBAL _sysFork
GLOBAL _sysExecve
GLOBAL _sysPriority
GLOBAL _sysChangeState
GLOBAL _wait
GLOBAL _sysExit
GLOBAL _sysSemOpen
GLOBAL _sysSemUp
GLOBAL _sysSemDown
GLOBAL _sysSemClose
GLOBAL _getPid
GLOBAL _sysMalloc
GLOBAL _sysFree
GLOBAL _sysRealloc
GLOBAL _pipe
GLOBAL _dup2
GLOBAL _close
GLOBAL _yield
GLOBAL _setToBackground
GLOBAL _sysExit
GLOBAL _sysBlock
GLOBAL _sysUnblock
GLOBAL _getFreeMemory
GLOBAL _getUsedMemory
GLOBAL _setToForeground


_setToForeground:
    mov rax, 35
    int 80h
    ret
    
_sysUnblock:
    mov rax, 39
    int 80h
    ret

_sysBlock:
    mov rax, 16
    int 80h
    ret

_sysExit:
    mov rax, 25
    int 80h
    ret

_setToBackground:
    mov rax, 36
    int 80h
    ret

_yield:
    mov rax, 38
    int 80h
    ret
    
_pipe:
    mov rax, 30
    int 80h
    ret

_dup2:
    mov rax, 37
    int 80h
    ret

_close
    mov rax, 31
    int 80h
    ret

_sysMalloc:
    mov rax, 32
    int 80h
    ret

_sysFree:
    mov rax, 33
    int 80h
    ret

_sysRealloc:
    mov rax, 34
    int 80h
    ret

_sysSemOpen:
    mov rax, 26
    int 80h
    ret

_sysSemUp:
    mov rax, 27
    int 80h
    ret

_sysSemDown:
    mov rax, 28
    int 80h
    ret

_sysSemClose:
    mov rax, 29
    int 80h
    ret

_sysChangeState:
    mov rax, 23
    int 80h
    ret

_wait:
    mov rax, 0x18
    int 80h
    ret

_sysKill:
    mov rax, 17
    int 80h
    ret

_sysFork:
    mov rax, 19
    int 80h
    ret

_sysPriority:
    mov rax, 22
    int 80h
    ret

_sysExecve:
    mov rax, 18
    int 80h
    ret

_sysPs:
    mov rax, 21
    int 80h
    ret

_read:
    mov rax, 0x0
    int 80h
    ret

_write:
    mov rax, 0x1
    int 80h
    ret

_clear:
    mov rax, 0x2
    int 80h
    ret

_beep:
    mov rax, 0x3
    int 80h
    ret

_sleep:
    mov rax, 0x4
    int 80h
    ret

_clock:
    mov rax, 0x5
    int 80h
    ret

_getScreenH:
    mov rax, 0x6
    int 80h
    ret

_getScreenW:
    mov rax, 0x7
    int 80h
    ret

_getPtrToPixel:
    mov rax, 0x8
    int 80h
    ret

_gameRead:
    mov rax, 0x9
    int 80h
    ret


_drawLine:
    mov rax, 0xA
    int 80h
    ret

_getPenX:
    mov rax, 0xB
    int 80h
    ret

_getPenY:
    mov rax, 0xC
    int 80h
    ret

_changeFont:
    mov rax, 0xD
    int 80h
    ret

_memoryDump:
    mov rax, 0xE
    int 80h
    ret

_getRegs:
    mov rax, 0xF
    int 80h
    ret
    
_idle:
    mov rax, 0x14
    int 80h
    ret

_getPid:
    mov rax, 40
    int 80h
    ret

_getFreeMemory:
    mov rax, 41
    int 80h
    ret

_getUsedMemory:
    mov rax, 42
    int 80h
    ret