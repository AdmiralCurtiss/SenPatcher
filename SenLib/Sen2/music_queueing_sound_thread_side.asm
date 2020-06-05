; compile with: ml /c /Cx /coff music_queueing_sound_thread_side.asm

.586
.XMM
.MODEL FLAT, C
.STACK
.DATA
.CODE

REPEAT 128
int 3
ENDM

inject_entry_point:
    test dword ptr[esi+10h],7FFFFFFFh  ; not entirely sure what this is checking but all the audio code does this before accessing any data, so...
    jne  enqueue_check
    mov  ecx,esi
    call dword ptr[eax+34h]
    jmp  back_to_function

REPEAT 1024
int 3
ENDM

enqueue_check:
    push edi
    lea  ecx,[esi+40h]
    call lock_mutex                 ; lock the bgm state mutex
    mov  eax,dword ptr[esi+14h]     ; eax now points at FSoundChannelController ptr array
    mov  ecx,dword ptr[esp+1Ch]     ; ecx now contains the channel number we were instructed to enqueue into
    mov  edi,dword ptr[eax+ecx*4h]  ; edi now points at FSoundChannelController of the bgm to enqueue

    mov  eax,dword ptr[edi]         ; call virtual function to check if channel is active
    mov  ecx,edi
    mov  eax,dword ptr[eax+10h]
    call eax                        ; al now contains whether the channel is active
    test al,al
    je   exit_continue              ; if channel is not active, continue the enqueue

    mov  eax,dword ptr[edi+8h]      ; go to FSoundData of the current FSoundChannelController
    test eax,eax                    ; nullpointer check
    je   exit_continue              ; if no FSoundData continue the enqueue
    mov  eax,dword ptr[eax+4Ch]     ; eax now contains the id of the track currently playing
    mov  ecx,dword ptr[ebp-134h]    ; ecx now contains the id of the track we want to enqueue
    cmp  eax,ecx
    jne  exit_continue              ; if tracks mismatch we always want to enqueue

    ; if we reach here the track playing and the track to enqueue are the same
    ; see if it's currently fading out, because if so we want to enqueue anyway
    mov  al,byte ptr[edi+38h]       ; al is now 1 when fade is active, 0 if not
    test al,al
    jne  exit_continue              ; if fading we do want to enqueue

    ; if we reach here we want to skip the enqueueing
    pop  edi
    add  esp,1Ch                    ; fixup stack as if the function was called
    jmp  exit_skip                  ; and leave

exit_continue:
    pop  edi
    mov  eax,dword ptr[esi]         ; call virtual function to enqueue the track, parameters have already been set up before the injected code
    mov  ecx,esi
    call dword ptr[eax+34h]

exit_skip:
    lea  ecx,[esi+40h]
    call unlock_mutex
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
back_to_function:
ret
REPEAT 15
int 3
ENDM

END
