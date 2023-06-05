
GLOBAL _cli
GLOBAL _sti
GLOBAL _hlt
GLOBAL _irq00Handler
GLOBAL _irq01Handler
GLOBAL _hasRegDump
GLOBAL _inforeg
GLOBAL _exception0Handler
GLOBAL _exception5Handler
GLOBAL _exception6Handler
GLOBAL _exception14Handler
GLOBAL _int20h
GLOBAL _syscallsHandler
GLOBAL _startSystem

EXTERN irqDispatcher
EXTERN exceptionDispatcher

EXTERN syscallsDispatcher
EXTERN scheduler
EXTERN timer_handler
EXTERN addKey
EXTERN getStackBase


SECTION .text

%macro pushState 0
	push rax
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

%macro popState 0
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rsi
	pop rdi
	pop rbp
	pop rdx
	pop rcx
	pop rbx
	pop rax
%endmacro

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

%macro popStateWithoutRAX 0
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rsi
	pop rdi
	pop rbp
	pop rdx
	pop rcx
	pop rbx
%endmacro


%macro endHardwareInterrupt 0
	mov al, 0x20
	out 0x20, al
%endmacro



%macro exceptionHandler 1
	
	mov [regdump + 8*0], rax
	mov [regdump + 8*1], rbx
	mov [regdump + 8*2], rcx
	mov [regdump + 8*3], rdx
	mov [regdump + 8*4], rdi
	mov [regdump + 8*5], rsi
	mov [regdump + 8*6], rbp
	
	mov [regdump + 8*8], r8
	mov [regdump + 8*9], r9
	mov [regdump + 8*10], r10
	mov [regdump + 8*11], r11
	mov [regdump + 8*12], r12
	mov [regdump + 8*13], r13
	mov [regdump + 8*14], r14
	mov [regdump + 8*15], r15

	mov rax, %1						; Check if it's a page fault (It pushes in the stack an error code)
	cmp rax, 14
	jne .continue
	mov rdx, [rsp]					; Error code number
	add rsp, 8						; Locates the stack where it should be

.continue:
	mov rax, [rsp+8+8+8]			; RSP Location
	mov [regdump + 8*7], rax		; RSP

	mov rax, [rsp+8+8]				; RFlags location
	mov [regdump + 8*16], rax		; RFlags

	mov rax, [rsp]					; RIP to the instruction that
	mov [regdump + 8*17], rax		;triggered the exception

	mov rsi, regdump				; Pointer to the vector that contains the registers info
	mov rdi, %1						; Exception ID
	call exceptionDispatcher
%endmacro

_syscallsHandler:
	pushStateWithoutRAX

	push rsp					; We need to use the rsp as a aditional parameter for all the syscalls that needs to deal with processes
	push r9
	mov r9, r8
	mov r8, rcx
	mov rcx, rdx
	mov rdx, rsi
	mov rsi, rdi
	mov rdi, rax
	call syscallsDispatcher
	add rsp, 8*2

	popStateWithoutRAX
	iretq

_hlt:
	sti
	hlt
	ret

_cli:
	cli
	ret


_sti:
	sti
	ret

;8254 Timer (Timer Tick)
_irq00Handler:
	pushState

	mov rdi, rsp
	call scheduler
	mov rsp, rax

	endHardwareInterrupt
	popState
	iretq



;Keyboard Interrupt Handler
_irq01Handler:
	pushState
	mov rax, 0
	in al, 60h

	cmp al, 0x38		; Check if it's the left-alt scancode
	jne .continue

; If it's the left-alt, we execute save the register's state
; https://wiki.osdev.org/Interrupt_Service_Routines For the location of rsp

	mov rax, [rsp+8*14]			; RAX Location

	mov [_inforeg + 8*1], rax
	mov [_inforeg + 8*2], rbx
	mov [_inforeg + 8*3], rcx
	mov [_inforeg + 8*4], rdx
	mov [_inforeg + 8*5], rdi
	mov [_inforeg + 8*6], rsi
	mov [_inforeg + 8*7], rbp
	mov rax, rsp 
	add rax, 8*15+8+8+8			; pushState + rip + cs + rflags = RSP value in stack
	mov [_inforeg + 8*8], rax	; RSP value
	mov [_inforeg + 8*9], r8
	mov [_inforeg + 8*10], r9
	mov [_inforeg + 8*11], r10
	mov [_inforeg + 8*12], r11
	mov [_inforeg + 8*13], r12
	mov [_inforeg + 8*14], r13
	mov [_inforeg + 8*15], r14
	mov [_inforeg + 8*16], r15

	mov rax, [rsp + 8*15]		; RIP location
	mov [_inforeg], rax

	mov BYTE [_hasRegDump], 1	; Flag para el inforeg

	jmp .end

.continue:
	cmp al, 0xB8		; If it's the left-alt release, skip the keyboard interrupt handler
	je .end

	mov rdi, rax
	call addKey

.end:
	endHardwareInterrupt
	popState
	iretq




;Zero Division Exception
_exception0Handler:
	exceptionHandler 0

_exception5Handler:
	exceptionHandler 5

_exception6Handler:
	exceptionHandler 6

_exception14Handler:
	exceptionHandler 14

_int20h:
	push rbp
	mov rbp, rsp

	int 20h

	mov rsp, rbp
	pop rbp
	ret

_startSystem:
	int 20h
    ret


section .rodata
	userSpace dq 0x400000

SECTION .bss
	aux resq 1
	_inforeg resq 17
	regdump resq 18
	_hasRegDump resb 1