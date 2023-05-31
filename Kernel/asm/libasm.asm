GLOBAL cpuVendor
GLOBAL getCPUSpeed
GLOBAL readTimeStampCounter
GLOBAL _hours
GLOBAL _seconds
GLOBAL _minutes
GLOBAL _setPIT
GLOBAL _setFrequency
GLOBAL _getSound
GLOBAL _playSound
section .text
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

getCPUSpeed:
	push rbp
	mov rbp, rsp

	xor rax, rax
	mov ax, WORD [0x0000000000005A00 + 256] ; CPU Speed location - Source: Sysvar.asm on Bootloader/Pure64/src

	mov rsp, rbp
	pop rbp
	ret

readTimeStampCounter:
	push rbp
	mov rbp, rsp
	push rdx

	xor rax, rax
	xor rdx, rdx

	rdtsc				; We read te Time-Stamp counter	- Leaves the result in EDX:EAX

	and rdx, QWORD 0xFFFFFFFF00000000	; We override the useless data in the low order bits
	and rax, QWORD 0x00000000FFFFFFFF ; We do the same thing with the high order bits
	
	or rax, rdx			; We put the entire 64-bit number on the return-value register rax

	pop rdx
	mov rsp, rbp
	pop rbp
	ret