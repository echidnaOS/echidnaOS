load_file:

; *************************************************
;     Loads a file from a FAT16 formatted drive
; *************************************************

; IN:
; es:ebx	-->		Target segment:offset
; si		-->		Must point to a FAT16 filename
; dl		-->		Drive number

; OUT:
; Carry if file not found, (does not clear the carry flag if found)
; ecx		-->		File size

; NOTES:
; This routine assumes 1000:0000 --> 1000:FFFF to be a usable buffer

push eax						; Save registers to prepare entering routine
push ebx
push edx
push esi
push fs

push ebx						; Save target offset for later
push es						; Save target segment for later

; Save current drive number

mov byte [.CurrentDrive], dl

;  Fetch metadata

xor ebx, ebx					; Load in the buffer (1000:0000)
mov ax, 0x1000
mov es, ax
xor ax, ax					; Load LBA sector 0 (boot sector)
mov cx, 1					; Load 1 sector
call read_sectors

mov al, byte [es:0x000D]
mov byte [.SectorsPerCluster], al
mov ax, word [es:0x000E]			; Save specifications in memory
mov word [.ReservedSectors], ax
mov al, byte [es:0x0010]
mov byte [.NumberOfFATs], al
mov ax, word [es:0x0011]
mov word [.RootEntries], ax
mov ax, word [es:0x0016]
mov word [.SectorsPerFAT], ax

;  Calculate the start and size of the root directory

; Start = reserved_sectors + (number_of_FATs * sectors_per_FAT)
; Size = (root_entries * 32) / bytes_per_sector

; Number of fats * sector per fat in AX

;mov ax, word [.SectorsPerFAT]
mov bl, byte [.NumberOfFATs]
xor bh, bh
mul bx

add ax, word [.ReservedSectors]			; Add reserved sectors

mov word [.StartOfRoot], ax				; Save result in memory

; Root entries * 32

mov ax, 32
mul word [.RootEntries]

xor dx, dx								; XOR DX for division
div word [.BytesPerSector]

mov word [.SizeOfRoot], ax				; Save result in memory

; Load root dir into buffer

xor ebx, ebx
mov ax, word [.StartOfRoot]				; Load from here
mov cx, word [.SizeOfRoot]				; Load this many sectors
mov dl, byte [.CurrentDrive]			; Retrieve drive
call read_sectors

; Browse root dir for a matching name to the one in SI

push si					; Save current SI
xor dx, dx				; Reset entry counter
xor bx, bx				; Reset root pointer

.browsing_loop:

cmp dx, word [.RootEntries]		; Is this the last entry?
je .not_found			; File not found
mov cx, 11				; Each entry is 11 bytes
.file_name_loop:
lodsb					; Byte from SI
mov ah, byte [es:bx]	; Byte from buffer
cmp ah, al				; Compare
jne .next_entry			; If not equal go to next entry
inc bx					; Increment BX pointer
loop .file_name_loop	; If 11 bytes
jmp .found_entry		; we found the file

.next_entry:

inc dx					; Increment DX entry counter
mov cx, dx				; Move DX in CX for multiplication

mov ax, 32				; Multiply CX * 32 to get to the next entry
mul cx
mov bx, ax				; Next entry in BX
pop si
push si					; Reset SI
mov dx, cx				; Move back CX in DX
jmp .browsing_loop		; Reenter loop

.found_entry:

pop si					; POP SI out of stack
mov cx, dx				; Move DX in CX for multiplication
mov ax, 32				; Multiply CX * 32 to get to the start of this entry
mul cx
add ax, 0x1A			; Add offset to the cluster number
mov bx, ax

mov ax, word [es:bx]
mov word [.Cluster], ax			; Save starting cluster

add bx, 2
mov eax, dword [es:bx]
mov dword [.FileSize], eax

; Let's load the FAT in the buffer now

; FAT start = Reserved Sectors
; FAT size = Sectors per FAT

xor ebx, ebx
mov ax, word [.ReservedSectors]			; Load from here
mov cx, word [.SectorsPerFAT]			; Load this many sectors
mov dl, byte [.CurrentDrive]			; Retrieve current drive
call read_sectors

; Start of data

mov bx, word [.StartOfRoot]
add bx, word [.SizeOfRoot]
mov ax, 2
xor cx, cx
mov cl, byte [.SectorsPerCluster]
mul cx
sub bx, ax
mov word [.DataStart], bx

; We'll now load the chain of clusters in the target destination

pop es						; Restore target segment
pop ebx						; Restore target offset

mov ax, 0x1000				; Now point FS to the buffer
mov fs, ax

mov ax, word [.Cluster]		; Load starting cluster

.cluster_loop:

cmp ax, 0xFFF8				; Is the last cluster?
jae .done					; If yes, we finished
xor cx, cx
mov cl, byte [.SectorsPerCluster]
mul cx
add ax, word [.DataStart]	; Add the data start offset
mov dl, byte [.CurrentDrive]	; Retrieve current drive
call read_sectors			; Read!

.move_buf:
add ebx, 512					; Move buffer up
loop .move_buf

.cluster_fetch:

push bx
mov bx, word [.Cluster]
add bx, word [.Cluster]
mov ax, word [fs:bx]		; Fetch cluster
pop bx
mov word [.Cluster], ax		; Save current cluster
jmp .cluster_loop

.not_found:

pop si					; SI out of stack
pop es
pop ebx
stc						; Set carry

.done:

pop fs					; Restore registers
pop esi
pop edx
pop ebx
pop eax
mov ecx, dword [.FileSize]
ret

.FileSize dd 0x00000000
.SectorsPerCluster db 0x00
.ReservedSectors dw 0x0000
.SectorsPerFAT dw 0x0000
.RootEntries dw 0x0000
.NumberOfFATs db 0x00
.StartOfRoot dw 0x0000
.SizeOfRoot dw 0x0000
.Cluster dw 0x0000
.BytesPerSector dw 512
.DataStart dw 0x0000
.CurrentDrive db 0x00
