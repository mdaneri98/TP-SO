GLOBAL cpuVendor
GLOBAL kbFlag
GLOBAL hasKey
GLOBAL readKey
GLOBAL _hours
GLOBAL _seconds
GLOBAL _minutes
GLOBAL _setPIT
GLOBAL _setFrequency
GLOBAL _getSound
GLOBAL _playSound
GLOBAL _getregs
EXTERN scr_print
;;EXTERN writeValues
section .text

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
	
cpuVendor:
	push rbp
	mov rbp, rsp

	push rbx

	mov rax, 0
	cpuid

	mov [rdi], ebx
	mov [rdi + 4], edx
	mov [rdi + 8], ecx
	mov byte [rdi+13], 0

	mov rax, rdi

	pop rbx

	mov rsp, rbp
	pop rbp
	ret


hasKey:
	push rbp
    mov rbp, rsp
    
	mov rax, 0
	in al,0x64
	and al, 0x01

	mov rsp, rbp
    pop rbp
    ret

readKey:
    push rbp
    mov rbp, rsp
    
	in al,0x60

	mov rsp, rbp
    pop rbp
    ret


kbFlag:
    push rbp
    mov rbp, rsp
      
    mov rax,0
loop:    
	in al,0x64       
    mov cl,al
    and al,0x01       
    cmp al,0
    je loop
    
	in al,0x60
       
    mov rsp, rbp
    pop rbp
    ret

_seconds:
	mov al, 0		; seconds.
	jmp RTC

_minutes:
	mov rax, 0
	mov al, 2		; minutes.
	jmp RTC

_hours:
	mov rax, 0
	mov al, 4		; hours.
	jmp RTC

RTC:
	out 70h, al		; 70h entrada para la informacion que quiero en 71h.
	in al, 71h		; en ax se deposita lo pedido por 70h, presente en 71h.
	ret

_setPIT:			;PIT data found at https://wiki.osdev.org/Programmable_Interval_Timer
	push rbp
	mov rbp, rsp
	mov rax, 0

	mov al, 0xb6
	out 0x43, al

	mov rsp, rbp
	pop rbp
	ret

_setFrequency:
	push rbp
	mov rbp, rsp
	mov rax, rdi

	out 0x42, al

	mov rsp, rbp
	pop rbp
	ret

_getSound:
	push rbp
	mov rbp, rsp
	mov rax, 0

	in al, 0x61
	mov rdi, rax

	mov rsp, rbp
	pop rbp
	ret

_playSound:
	push rbp
	mov rbp, rsp
	push rax

	mov rax, rdi
	out 0x61, al

	pop rax
	mov rsp, rbp
	pop rbp
	ret
section .rodata
	string 			db "I got here owo", 0