; compile with: ml /c /Cx /coff music_queueing.asm

.586
.XMM
.MODEL FLAT, C
.STACK
.DATA
.CODE

REPEAT 128
int 3
ENDM

set_dirty_and_write_sound_queue_4bytes:
    mov eax,1111111h
    mov byte ptr[eax],1
    jmp write_sound_queue_4bytes

REPEAT 1024
int 3
ENDM

clear_dirty_if_queue_empty:
    mov  esi,dword ptr [ebp-94h]
    lea  ecx,[esi+44h]
    call lock_mutex
    cmp  dword ptr [esi+6Ch],0
    jne  queue_not_empty
    mov  ecx,1111111h
    mov  byte ptr[ecx],0
queue_not_empty:
    lea  ecx,[esi+44h]
    call unlock_mutex
    jmp  queue_empty_jump_back

REPEAT 1024
int 3
ENDM

lock_mutex:
ret
REPEAT 15
int 3
ENDM
unlock_mutex:
ret
REPEAT 15
int 3
ENDM
write_sound_queue_4bytes:
ret
REPEAT 15
int 3
ENDM
queue_empty_jump_back:
ret
REPEAT 15
int 3
ENDM

END
