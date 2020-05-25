; compile with: ml /c /Cx /coff music_fade_timing_patch.asm

; we have five code blocks here:
; 0x3D bytes
; 0x2B bytes
; 0x4E bytes
; 0x7F bytes <- this needs the original function location
; 0x49 bytes

.586
.XMM
.MODEL FLAT, C
.STACK
.DATA
.CODE

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

; compiles into 0x3D bytes
do_compare:
    push        ebx
    mov         ebx,eax
    mov         eax,dword ptr [ecx+4]
    push        esi
    mov         esi,dword ptr [ecx]
    add         esi,dword ptr [edx]
    mov         ecx,dword ptr [ebx]
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
    inc        ebx      ; counter += 1
    cmp        ebx,edx  ; if counter > ticks_per_loop_remainder
    jbe        time_pass_loop_remainder_increment ; counter <= ticks_per_loop_remainder, go to loop
    add        eax,1
    adc        ecx,0
    mov        dword ptr [ebp-10h],eax
    mov        dword ptr [ebp-0Ch],ecx
time_pass_loop_remainder_increment:
    jmp        time_pass_loop




REPEAT 1024
int 3
ENDM

; ---------------------------------------------------------------------
; main function

; compiles into 0x2B bytes

thread_mainloop:
    push       ebp
    mov        ebp,esp

    sub        esp,7Ch

    mov        ecx,dword ptr [ebp+8]
    push       edi
    mov        edi,ecx
    push       esi
    xor        esi,esi
    push       ebx

    test       dword ptr [edi+10h],7FFFFFFFh
    je         early_exit

; place frequency in ticks_per_second == ebp-28h (and ebp-24h, it's a 64bit val on 32bit cpu)
    lea        eax,[ebp-28h]
    push       eax
    call       QueryPerformanceFrequency
    jmp        thread_mainloop_continue

REPEAT 1024
int 3
ENDM

; ---------------------------------------------------------------------
; compiles into 0x4E bytes

thread_mainloop_continue:
; divide by N to get the time frequency we want the inner function to run at
; this division may have a remainder
    mov eax,dword ptr [ebp-24h]
    mov edx,dword ptr [ebp-28h]
    push 0
    push 3E8h ; divisor, 1000 in this case
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
    push 5h ; multiplicator, 5 seconds should be reasonable
    push eax
    push edx
    call _allmul
    mov dword ptr [ebp-30h],eax ; ebp-30h = low part of ticks_for_reset == [ticks_for_reset]
    mov dword ptr [ebp-2Ch],edx ; ebp-2Ch = high part of ticks_for_reset

    xor        ebx,ebx  ; we designate ebx as our remainder-counter, initialize to 0

; initialize ticks_last == ebp-10h / ebp-0Ch
    lea        eax,[ebp-10h]
    push       eax
    call       QueryPerformanceCounter

    cmp        byte ptr [edi+54h],0
    jne        first_loop_exit
    jmp        outer_loop

REPEAT 1024
int 3
ENDM

; ---------------------------------------------------------------------
; compiles into 0x7F bytes

outer_loop:
    lea        ecx,[edi+38h]
    call       lock_mutex ; 0071E550h ; lock_mutex

; initialize ticks_now == ebp-18h / ebp-14h
    lea        eax,[ebp-18h]
    push       eax
    call       QueryPerformanceCounter

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
    je         go_to_sleep
    jmp        inner_loop

exit_inner_loop:
; ticks_last += ticks_per_loop
    mov        eax,dword ptr [ebp-20h]
    mov        ecx,dword ptr [ebp-1Ch]
    add        dword ptr [ebp-10h],eax
    adc        dword ptr [ebp-0Ch],ecx

; if ticks_per_loop_remainder != 0
    mov        edx,dword ptr [ebp-40h]
    test       edx,edx
    je         time_pass_loop ; no remainder, just go back to loop
    jmp        remainder_increment

go_to_sleep:
    lea        ecx,[edi+38h]
    call       unlock_mutex ; 0071E580h ; unlock_mutex
    push       0
    call       invoke_sleep_milliseconds ; 0071DE50h ; invoke_sleep_milliseconds
    add        esp,4

    cmp        byte ptr [edi+54h],0
    je         outer_loop

first_loop_exit:
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

; compiles into 0x49 bytes

inner_loop:
    mov        ecx,edi
    call       unknown_func ; 0041E9F0h

    cmp        esi,21h
    jb         post_every_33_iterations

inner_if_condition:
    movd       xmm0,esi
    cvtdq2pd   xmm0,xmm0
    mov        eax,esi
    shr        eax,1Fh
    mov        edx,dword ptr [edi]
    addsd      xmm0,mmword ptr [eax*8+8EC240h]
    push       ecx
    mov        ecx,edi
    cvtpd2ps   xmm0,xmm0
    divss      xmm0,dword ptr ds:[8ED254h]
    mulss      xmm0,dword ptr [edi+58h]
    movss      dword ptr [esp],xmm0
    call       dword ptr [edx+68h]
    sub        esi,21h

post_every_33_iterations:
    inc        esi
    jmp        exit_inner_loop

REPEAT 1024
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
unknown_func:
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


END
