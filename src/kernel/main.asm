org 0x0
bits 16

%define ENDL 0x0D, 0x0A

start:
	; print success message
	mov si, msg_success
	call puts

.halt:
	cli
	hlt

;
; Prints a string to the screen
; Params:
;   - ds:si points to string
;
puts:
	push si
	push ax
	push bx

.loop:
	lodsb               ; loads next character in al
	or al, al           ; next char == 0
	jz .done

	mov ah, 0x0E        ; bios interrupt
	mov bh, 0           ; page number = 0
	int 0x10

	jmp .loop

.done:
	pop bx
	pop ax
	pop si

	ret

msg_success: db 'Kernel loaded successfully!', ENDL, 0