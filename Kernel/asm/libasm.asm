GLOBAL cpuVendor
GLOBAL _hours
GLOBAL _seconds
GLOBAL _minutes
GLOBAL _setPIT
GLOBAL _setFrequency
GLOBAL _getSound
GLOBAL _playSound
GLOBAL _picMasterMask
GLOBAL _picSlaveMask
GLOBAL _getCPUCristalSpeed
GLOBAL _getTSCNumerator
GLOBAL _getTSCDenominator
GLOBAL _readTimeStampCounter
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
	mov al, 0		; Seconds.
	jmp RTC

_minutes:
	mov rax, 0
	mov al, 2		; Minutes.
	jmp RTC

_hours:
	mov rax, 0
	mov al, 4		; Hours.
	jmp RTC

RTC:
	out 70h, al		; 70h port to indicate to the RTC what info I'm looking for
	in al, 71h		; In the 71h port we find the result requested in the 70h port
	ret

_setPIT:			; PIT data found at https://wiki.osdev.org/Programmable_Interval_Timer
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

_picMasterMask:
	push rbp
    mov rbp, rsp
    mov ax, di
    out	21h,al
    pop rbp
    retn

_picSlaveMask:
	push    rbp
    mov     rbp, rsp
    mov     ax, di  ; ax = mascara de 16 bits
    out		0A1h,al
    pop     rbp
    retn

;-------------------------------------------------------------------------------------------------------------------|
; _getCPUCristalSpeed: Uses the cpuid instruction to get the CPU Cristal Clock Speed, needed to calculate the TSC	|
;					frequency. First it enables the 0x15 level of cpuid, just in case it's not allowed by the system|
; returns: CPU Cristal Speed (uint64_t)																				|
;-------------------------------------------------------------------------------------------------------------------|
_getCPUCristalSpeed:
	push rbp
	mov rbp, rsp
	push rbx
	push rcx
	push rdx

; This instructions should enable the level 0x15 of the cpuid instruction
	xor rax, rax
	xor rbx, rbx
	xor rcx, rcx
	mov ecx, MISC_ENABLE
	rdmsr
	mov ecx, MISC_ENABLE_MASK
	and eax, ecx
	mov ecx, MISC_ENABLE
	wrmsr


	xor rax, rax
	xor rbx, rbx
	xor rcx, rcx
	xor rdx, rdx

	mov eax, 0x15
	cpuid			; Result stored in ECX
	
	xor rax, rax
	mov rax, rcx

	pop rdx
	pop rcx
	pop rdx
	mov rsp, rbp
	pop rbp
	ret

;-------------------------------------------------------------------------------------------------------------------|
; _getTSCNumerator: Uses the cpuid instruction to get the TSC numerator, needed to calculate the TSC frequency		|
; returns: Time Stamp Counter Numerator (uint64_t)																	|
;-------------------------------------------------------------------------------------------------------------------|
_getTSCNumerator:
	push rbp
	mov rbp, rsp
	push rbx
	push rcx
	push rdx

	xor rax, rax
	xor rbx, rbx
	xor rcx, rcx
	xor rdx, rdx

	mov eax, 0x15
	cpuid			; Result stored in EBX

	xor rax, rax
	mov rax, rbx

	pop rdx
	pop rcx
	pop rdx
	mov rsp, rbp
	pop rbp
	ret

;-------------------------------------------------------------------------------------------------------------------|
; _getTSCDenominator: Uses the cpuid instruction to get the TSC denominator, needed to calculate the TSC frequency	|
; returns: Time Stamp Counter Denominator (uint64_t)																|
;-------------------------------------------------------------------------------------------------------------------|
_getTSCDenominator:
	push rbp
	mov rbp, rsp
	push rbx
	push rcx
	push rdx

	xor rax, rax
	xor rbx, rbx
	xor rcx, rcx
	xor rdx, rdx

	mov eax, 0x15
	cpuid			; Result already stored in EAX/RAX

	pop rdx
	pop rcx
	pop rdx
	mov rsp, rbp
	pop rbp
	ret

_readTimeStampCounter:
	push rbp
	mov rbp, rsp
	push rdx
	push rcx

	xor rax, rax
	xor rdx, rdx

	rdtsc				; We read te Time-Stamp counter	- Leaves the result in EDX:EAX

	shl rdx, 32			; We shift the 32 lower bits that contains the high-value of the TSC
	or rax, rdx			; We put the entire 64-bit number on the return-value register rax

	pop rcx
	pop rdx
	mov rsp, rbp
	pop rbp
	ret

section .data
	lowerMask equ 0x00000000FFFFFFFF
	higherMask equ 0xFFFFFFFF00000000

	MISC_ENABLE equ 0x1A0				; Memory Location of the LCMV bit, needed to compute the TSC frequency
	MISC_ENABLE_MASK equ 0xFFBFFFFF		; Mask that we are going to use to enable the 0x15 cpuid level