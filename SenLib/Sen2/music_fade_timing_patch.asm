; compile with: ml /c /Cx /coff music_fade_timing_patch.asm

.586
.XMM
.MODEL FLAT, C
.STACK
.DATA
.CODE

REPEAT 128
int 3
ENDM

entry_point:
    push       ebp
    mov        ebp,esp
    sub        esp,7Ch
    mov        ecx,dword ptr [ebp+8]
    push       edi
    push       esi
    jmp        thread_mainloop

REPEAT 1024
int 3
ENDM

; int do_compare(int64* ticks_last, int64* ticks_per_loop, int64* ticks_now)
; input: eax -> &ticks_now
;        edx -> &ticks_per_loop
;        ecx -> &ticks_last
; clobbers: eax, ecx
; if (ticks_last + ticks_per_loop < ticks_now)
;     return 1;
; return 0;

do_compare:
    push        ebx
    mov         ebx,eax
    mov         eax,dword ptr [ecx+4]
    push        esi
    mov         esi,dword ptr [ecx]
    add         esi,dword ptr [edx]
    mov         ecx,ebx
    adc         eax,dword ptr [edx+4]
    cmp         eax,dword ptr [ecx+4]
    jg          success
    jl          fail
    cmp         esi,dword ptr [ecx]
    jae         success
fail:
    mov         eax,1
    jmp         do_compare_end
success:
    xor         eax,eax
do_compare_end:
    pop         esi
    pop         ebx
    ret



remainder_increment:
; this should be the equivalent of
;   counter += ticks_per_loop_remainder
;   if (counter >= original_divisor) {
;       ++ticks_last
;       counter -= original_divisor
;   }
; input:
;  edx / dword ptr [ebp-40h] = ticks_per_loop_remainder
;        dword ptr [ebp-44h] = counter
;        dword ptr [ebp-38h] = original_divisor
;  eax / dword ptr [ebp-20h] = ticks_per_loop low part
;  ecx / dword ptr [ebp-1Ch] = ticks_per_loop high part
;        dword ptr [ebp-10h] = ticks_last low part
;        dword ptr [ebp-0Ch] = ticks_last high part
    mov        eax,dword ptr [ebp-44h]  ; load counter into eax
    add        eax,edx                  ; counter += ticks_per_loop_remainder
    mov        edx,dword ptr [ebp-38h]  ; edx = original_divisor
    cmp        eax,edx                  ; if counter >= original_divisor
    jb         exit_remainder_increment
    add        dword ptr [ebp-10h],1    ; ++ticks_last
    adc        dword ptr [ebp-0Ch],0
    sub        eax,edx                  ; counter -= original_divisor
exit_remainder_increment:
    mov        dword ptr [ebp-44h],eax  ; write counter back to stack
    jmp        time_pass_loop




REPEAT 1024
int 3
ENDM

; ---------------------------------------------------------------------
; main function

thread_mainloop:
    push       ebx
    mov        edi,ecx
    xor        esi,esi

    test       dword ptr [edi+10h],7FFFFFFFh
    je         early_exit

; place frequency in ticks_per_second == ebp-28h (and ebp-24h, it's a 64bit val on 32bit cpu)
    lea        eax,[ebp-28h]
    call       invoke_query_performance_frequency
    jmp        thread_mainloop_continue

REPEAT 1024
int 3
ENDM

; ---------------------------------------------------------------------

thread_mainloop_continue:
; divide by N to get the time frequency we want the inner function to run at
; this division may have a remainder
    mov eax,1000 ; divisor
    mov dword ptr [ebp-38h],eax ; remember divisor for remainder_increment
    push 0
    push eax
    mov eax,dword ptr [ebp-24h]
    mov edx,dword ptr [ebp-28h]
    push eax
    push edx
    call _alldvrm
    mov dword ptr [ebp-20h],eax ; ebp-20h = low part of ticks_per_loop == [ticks_per_loop]
    mov dword ptr [ebp-1Ch],edx ; ebp-1Ch = high part of ticks_per_loop
    mov dword ptr [ebp-40h],ecx ; ebp-40h = low part of ticks_per_loop_remainder
    ; mov dword ptr [ebp-3Ch],ebx ; there is never a high part of the remainder, so we can skip this

; multiply by N to get the time frequency when we should assume something happened and we should reset the timer
; eg. CPU went to sleep and woke back up later, or just a big lag spike where we never get a cycle -- unlikely, but hey
    mov eax,dword ptr [ebp-24h]
    mov edx,dword ptr [ebp-28h]
    push 0
    push 5 ; multiplicator, 5 seconds should be reasonable
    push eax
    push edx
    call _allmul
    mov dword ptr [ebp-30h],eax ; ebp-30h = low part of ticks_for_reset == [ticks_for_reset]
    mov dword ptr [ebp-2Ch],edx ; ebp-2Ch = high part of ticks_for_reset

; initialize ticks_last == ebp-10h / ebp-0Ch
    lea        eax,[ebp-10h]
    call       invoke_query_performance_counter

    cmp        byte ptr [edi+54h],0
    jne        early_exit
    jmp        outer_loop_init

REPEAT 1024
int 3
ENDM

; ---------------------------------------------------------------------

outer_loop_init:
; initialize remainder-counter to 0
    mov        dword ptr [ebp-44h],0

outer_loop:
; remember that we're in a fresh iteration of the loop
    xor        ebx,ebx

; initialize ticks_now == ebp-18h / ebp-14h
    lea        eax,[ebp-18h]
    call       invoke_query_performance_counter

; if over reset time passed reset ticks_last
    lea        eax,[ebp-18h] ; ticks_now
    lea        edx,[ebp-30h] ; ticks_for_reset
    lea        ecx,[ebp-10h] ; ticks_last
    call       do_compare
    test       eax,eax
    je         time_pass_loop
    mov        eax,dword ptr [ebp-18h]
    mov        dword ptr [ebp-10h],eax
    mov        eax,dword ptr [ebp-14h]
    mov        dword ptr [ebp-0Ch],eax

time_pass_loop:
    lea        eax,[ebp-18h] ; ticks_now
    lea        edx,[ebp-20h] ; ticks_per_loop
    lea        ecx,[ebp-10h] ; ticks_last
    call       do_compare
    test       eax,eax
    je         go_to_sleep_maybe
    jmp        inner_loop

exit_inner_loop:
; ticks_last += ticks_per_loop
    mov        eax,dword ptr [ebp-20h] ; ticks_per_loop
    mov        ecx,dword ptr [ebp-1Ch]
    add        dword ptr [ebp-10h],eax ; ticks_last
    adc        dword ptr [ebp-0Ch],ecx

; if ticks_per_loop_remainder != 0
    mov        edx,dword ptr [ebp-40h] ; ticks_per_loop_remainder
    test       edx,edx
    je         time_pass_loop ; no remainder, just go back to loop
    jmp        remainder_increment

go_to_sleep_maybe:
; if we have done an inner_loop this iteration, do not sleep, immediately go to next loop
; this avoids going to sleep when we're behind on the timing
    test       ebx,ebx
    jne        go_to_next_iteration

; otherwise do a short sleep so that we don't take all the cpu cycles
    push       0
    call       invoke_sleep_milliseconds ; 0071DE50h ; invoke_sleep_milliseconds
    add        esp,4

go_to_next_iteration:
    cmp        byte ptr [edi+54h],0
    je         outer_loop

early_exit:
    pop        ebx
    pop        esi
    xor        eax,eax
    pop        edi
    mov        esp,ebp
    pop        ebp
    ret

REPEAT 1024
int 3
ENDM

inner_loop:
    mov        ebx,1 ; remember that we've executed an inner_loop

    lea        ecx,[edi+38h]
    call       lock_mutex ; 0071E550h
    mov        ecx,edi
    call       process_sound_queue ; 0041E9F0h

    cmp        esi,21h
    jb         post_every_33_iterations

inner_if_condition:
    movd       xmm0,esi
    cvtdq2ps   xmm0,xmm0
    mov        edx,dword ptr [edi]
    push       ecx
    mov        ecx,edi
    divss      xmm0,dword ptr ds:[8ED254h]
    mulss      xmm0,dword ptr [edi+58h]
    movss      dword ptr [esp],xmm0
    call       dword ptr [edx+68h]
    sub        esi,21h

post_every_33_iterations:
    lea        ecx,[edi+38h]
    call       unlock_mutex ; 0071E580h
    inc        esi
    jmp        exit_inner_loop

REPEAT 1024
int 3
ENDM


invoke_query_performance_counter:
push ebp
mov  ebp,esp
push eax
call dword ptr[QueryPerformanceCounter]
pop  ebp
ret

REPEAT 4
int 3
ENDM

invoke_query_performance_frequency:
push ebp
mov  ebp,esp
push eax
call dword ptr[QueryPerformanceFrequency]
pop  ebp
ret

REPEAT 4
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
invoke_sleep_milliseconds:
ret
REPEAT 15
int 3
ENDM
process_sound_queue:
ret
REPEAT 15
int 3
ENDM
_allmul:
ret
REPEAT 15
int 3
ENDM
_alldiv:
ret
REPEAT 15
int 3
ENDM
_allrem:
ret
REPEAT 15
int 3
ENDM
_alldvrm:
ret
REPEAT 15
int 3
ENDM
QueryPerformanceCounter:
ret
REPEAT 15
int 3
ENDM
QueryPerformanceFrequency:
ret
REPEAT 15
int 3
ENDM

END
