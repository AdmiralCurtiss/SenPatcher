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

; we want to return something along the lines of
; is_playing_bgm(requested_bgm_path) && !bgm_is_fading() && !dirty
; in eax
; al == 0 -> we want to enqueue the track
; al == 1 -> we don't want to enqueue the track

check_if_should_enqueue:
    push eax                   ; this is for the 'call eax' in the check is_playing_bgm() block!
    push dword ptr[ebp+18h]    ; see above

    ; load dirty flag and remember for check later
    ; we do this so weirdly to prevent a mutex deadlock and I don't think exists but can't actually disprove
    ; since this code should only be executed in the main thread, and the dirty flag should only be set by
    ; the main thread (and cleared by the sound thread), this can at worst give a stale dirty == 1, never a stale dirty == 0
    lea  ecx,[esi+44h]         ; lock on sound queue as that's the mutex the dirty flag sits behind
    call lock_mutex
    mov  eax,1111111h
    mov  al,byte ptr[eax]
    mov  edi,eax
    lea  ecx,[esi+44h]
    call unlock_mutex

    ; in theory we could already bail here if dirty == 1, but I'm not sure if that call eax below has side-effects, so let's not

    ; lock on bgm state
    ; the call eax below does this too, but we want to keep it a bit longer so we can
    ; check for the fading state without a chance that another thread switches bgm
    lea  ecx,[esi+40h]
    call lock_mutex

    ; check is_playing_bgm()
    mov  eax,dword ptr[esi]
    mov  eax,dword ptr[eax+94h]
    mov  ecx,esi
    call eax         ; al is now 1 when bgm is playing, 0 if not
    test al,al
    je   check_done  ; if is_playing_bgm() == 0 we're already done; this returns 0

    ; check dirty flag
    mov  eax,edi
    test al,al                  ; al is now 1 when dirty, 0 if not
    jne  check_done_return_0    ; if dirty return 0

    ; check bgm_is_fading()
    mov  eax,dword ptr[esi+14h]
    mov  eax,dword ptr[eax]     ; eax is now pointing at the bgm sound channel
    mov  al,byte ptr[eax+38h]   ; al is now 1 when fade is active, 0 if not
    test al,al
    jne  check_done_return_0    ; if it's fading out return 0
    inc  eax
    jmp  check_done             ; otherwise return 1

check_done_return_0:
    xor  eax,eax
check_done:
    mov  edi,eax
    lea  ecx,[esi+40h]
    call unlock_mutex
    mov  eax,edi
    mov  ecx,esi               ; restore ecx (probably unnecessary)
    mov  edi,dword ptr[esi]    ; restore edi
    jmp  back_to_function

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
back_to_function:
ret
REPEAT 15
int 3
ENDM

END
