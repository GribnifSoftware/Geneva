EXTRAS = ($8A-1)
DEBUG  = 0
TEST   = 0

XBRA_ID = $476E7661 ; 'Gnva'

.import _BasPag, curr_bp
.export new_exec
.export old_exec
.import initialize
.import is_auto
.export old_stack
        .ascii  'XBRA'
        dc.l    XBRA_ID
old_exec:
        ds.l    1
new_exec:
        move.l  4(sp), a3
;        move.l  a3, my_bp
        move.l  old_stack(pc), a7
        pea     $100.w
        pea     (a3)
        move.l  #$4a0000, -(sp) ; Mshrink
        trap    #1
        lea     12(sp), sp
        move.l  (a3), a0
        lea     $100(a0), a0
        move.l  a0, 4(a3)       ; set hitpa
        bsr     initialize
;        pea     0.w
;        move    #$20, -(sp)
;        trap    #1
;        move.l  d0, 2(sp)
;        move.l  old_exec(pc), $4fe.w
;        trap    #1
;        move.l  my_bp(pc), (sp)
;        move.l  old_exec(pc), a0
;        jsr     (a0)
        move.l  44(a3), -(sp)
        pea     empty(pc)
        pea     0.w
        move.l  #$004b0005, -(sp)
        trap    #1
        lea     16(sp), sp
        tst.l   d0
        blt     warmboot
        move.l  d0, a0
        move.l  old_exec(pc), 8(a0)
        move.l  44(a3), -(sp)
        pea     (a0)
        pea     0.w
        move.l  #$004b0004, -(sp)
        trap    #1
        bra     warmboot

.export my_midi, old_midi, midi_io
midi_io: ds.l   1
old_midi:ds.l   1
my_midi:                        ; 005: read MIDI data at IPL 7
        lea     $FFFFFC04.w, a1
        move.l  xkbdvb(pc), a2
        move.l  midi_io(pc), a0
        move.b  (a1), d2
        btst    #7, d2
        beq.b   mid1
        btst    #5, d2
        bne.b   miderr
        btst    #0, d2
        beq.b   mid1
        move.l  (a2), a2
        move    sr, -(sp)
        ori     #$700, sr
        move.b  2(a1), d0
        jsr     (a2)
        move    (sp)+, sr
mid1:   rts
miderr: move.b  2(a1), d0
        move.l  8(a2), a2
        jmp     (a2)

.export old_kbd
.export my_kbd
        .ascii  'XBRA'
        dc.l    XBRA_ID
old_kbd:
        ds.l    1
nkreg reg a0-a1/d0-d1           ; my_200 depends on this
my_kbd:
        move.l  kbio(pc), a0
        move    8(a0), -(sp)
        move.l  old_kbd(pc), a0
        jsr     (a0)
        move    (sp)+, d1
setkey: movea.l kbio(pc), a0
        moveq   #0, d0
        move    8(a0), d0
        cmp     d1, d0
        beq.b   tk8
        lsr     #2, d0          ; 004: was asr
        cmp     kbsize(pc), d0  ; d0 can never be > 65535/4
        bge.b   tk8             ; 004: get out if past end of kbbuf
        move.l  kbbuf(pc), a1
        movea.l kbshift(pc), a0
        move.b  (a0), d1
        andi.b  #$f, d1
        move.b  d1, 0(a1,d0)
tk8:    rts

.export old_200
.export my_200
        .ascii  'XBRA'
        dc.l    XBRA_ID
old_200:
        ds.l    1
my_200: movem.l #nkreg, -(sp)
        move    4*4(sp), d0     ; 004: get rte sr
        ori     #$2000, d0      ; 004: make sure S=1
        move.l  kbio(pc), a0
        move    8(a0), d1       ; 004: used to push
        tst.w   $59e.w
        beq.b   tk3
        move    #$114, -(sp)    ; exception frame format 0, vector 114
tk3:    pea     nks2(pc)
        move    d0, -(sp)
nks49:  move.l  old_200(pc), a0
        jmp     (a0)
nks2:   bsr.b   setkey          ; 004: d1 already has pointer
        movem.l (sp)+, #nkreg
        rte

.export set_t2
set_t2: move.l  $88.w, old_t2
        move.l  #my_t2, $88.w
        tst.b   is_auto
        beq.b   st20
        ; look in the ROM for a specific sequence of code
        moveq   #-1, d0         ; look at most $10000 times
        move.l  $4f2.w, a0
        move.l  8(a0), a0
st21:   cmpi.l  #$504f4e75, (a0); *** addq #8, a7;  rts
        beq.b   st23
        addq    #2, a0
        dbra    d0, st21
        bra.b   st20            ; not found, this must not be a real Atari TOS
st23:   pea     st22(pc)        ; my return
        pea     2(a0)           ; new vector address (rts in ROMs)
        move.l  #$050101, -(sp) ; Setexec
        tst     $59e.w
        beq.b   st24
        move.w  #$00b4, -(sp)   ; fake format word
st24:   pea     (a0)            ; address of rts in ROM
        move    sr, -(sp)
        move.l  $b4.w, a1       ; and jump through
        jmp     (a1)
st22:   move.l  d0, old_crit
        bra.b   st25
st20:   move.l  $404.w, old_crit
st25:   move.l  #critical, $404.w
        pea     (a2)
        bsr.b   move_linea
        lea     9*4(a2), a2
        move.l  (a2), old_la9
        move.l  #my_la9, (a2)+
        move.l  (a2), old_laa
        move.l  #my_laa, (a2)
        move.l  (sp)+, a2
        rts

move_linea:
        dc.w    $a000
        move.l  a0, la_base
        move.l  a1, la_font
        move.l  $4f2.w, a0
        cmp.l   $8(a0), a2
        blt.b   mlok
        cmp.l   $14(a0), a2
        bge.b   mlok
        lea     16*4(a2), a0
        lea     my_latbl+16*4(pc), a2
        moveq   #14, d0
ml1:    move.l  -(a0), -(a2)
        dbra    d0, ml1
        subq    #4, a2
        move.l  $28.w, old_la
        move.l  #my_lavec, $28.w
mlok:   rts

.export undo_linea
undo_linea:
        dc.w    $a000
        cmpa.l  #my_latbl, a2
        beq.b   ula2
        move.l  $28.w, d0
        cmpi.l  #my_lavec, d0
        bne.b   ula1
ula2:   move.l  old_la(pc), $28.w
        rts
ula1:   lea     9*4(a2), a2
        move.l  old_la9(pc), (a2)+
        move.l  old_laa(pc), (a2)
        rts

        .ascii  'XBRA'
        dc.l    XBRA_ID
old_la:
        ds.l    1
my_lavec:
        MOVEA.L  2(A7),A1
        MOVE.W   (A1)+,D2
        MOVE.L   A1,2(A7)
        SUBI.W   #$A000,D2
        cmpi     #$f, d2
        BGT.B    labad
        ADD.W    D2,D2
        ADD.W    D2,D2
        MOVEA.L  my_latbl(PC,D2.W),A1
        MOVEM.L  D3-D7/A3-A6,-(A7)
        JSR      (A1)
        MOVEM.L  (A7)+,D3-D7/A3-A6
labad:  rte

my_la0: move.l  la_base(pc), a0
        move.l  a0, d0
        move.l  la_font(pc), a1
        lea     my_latbl(pc), a2
        rts

my_latbl:
        dc.l    my_la0
        ds.l    14

;;my_t1r:
;;      clr.b   255(a6)
;;      move.l  248(a6), -(sp)
;;      move.l  244(a6), a6
;;      rts

.export do_frk, frk_mode, frk_id
do_frk: ds.b    1                       ; must be in this order
frk_mode:
        ds.b    1                       ;
ofrk_ret:
        ds.l    1
frk_id: ds.w    1
frk_ret:tst     d0
        bne.b   frk1
        movem.l d0-d2/a0-a1, -(sp)
        st      do_frk
frk3:   tst.b   do_frk
        beq.b   frk2
        jsr     Syield
        bra.b   frk3
frk2:   movem.l (sp)+, d0-d2/a0-a1
frk1:   move.l  ofrk_ret(pc), -(sp)
        rts

.import mint_preem
fren:   addq    #2, a0
path:   tst.b   mint_preem
        bne     t1x
        move.l  (a0), pthptr
        move.l  2+16(sp), pthret
        move.l  #mypthret, 2+16(sp)
        bra     t1x

t111:   move.b  preempt(pc), d0
        bne.b   t1ok
        lea     t1_fatal(pc), a0
        bsr     fatal
        moveq   #-32, d0
        rte

t1x2:   subi    #$113-$31, d0           ; 006: Intercept Pfork & Pvfork
        beq.b   pfrk
        subq    #$11b-$113, d0
        bne.b   t1x
        moveq   #1, d0                  ; signifies Pvfork
pfrk:   tst.b   mint_preem
        beq.b   t1x
        move.b  d0, frk_mode
        jsr     Pgetpid
        move    d0, frk_id
        move.l  2+16(sp), ofrk_ret
        move.l  #frk_ret, 2+16(sp)
        bra.b   t1x

.export in_t1
.import no_set
.export old_t1
.export my_t1
        .ascii  'XBRA'
        dc.l    XBRA_ID
old_t1:
        ds.l    1
my_t1:
        tst.b   in_t1
        bne.b   t111
t1ok:   movem.l a0-a1/d1-d2, -(sp)
        move.l  usp, a0
        btst    #5, 16(sp)
        beq.b   t1_1
        lea     6+16(sp), a0
        adda    frame_off(pc), a0
t1_1:   move    (a0)+, d0
        beq.b   t1_term0
        subi    #$31, d0                ; 004
        blt.b   t1x                     ;
        cmpi    #($56-$31), d0          ;
        bhi.b   t1x2                    ;
        add     d0, d0                  ;
        lea     t1atbl(pc), a1          ;
        move    0(a1,d0), d0            ;
        jmp     t1x(pc,d0)              ;

t1x:    movem.l (sp)+, a0-a1/d1-d2
        move.l  old_t1(pc), -(sp)
;        clr.b   in_t1
        rts

.import set_oldapp, last_parent, pfork

t1_termp:
        move    4(a0), -(sp)
        bra.b   t1t
t1_term0:
        clr     -(sp)
        bra.b   t1t
t1_term:move    (a0), -(sp)
t1t:    st      in_t1
        tst.b   mint_preem
        bne.b   t1tx
;       jsr     get_curapp
;       tst     d0
;       beq.b   t1tx
        move    (sp)+, d0
        move.l  a7, a0
        lea     temp, a7
        pea     (a0)
        clr.l   last_parent     ; 004
        jsr     terminate
        move.l  (sp), sp
;       jsr     set_oldapp
        sf      in_t1
        bra.b   t1x
t1tx:   addq    #2, sp
        sf      in_t1
        bra.b   t1x

;;.export       PD_bp
;;PD_bp:        ds.l    1

.import memory
mxall:  andi    #$FF03, 4(a0)   ; 007
        tst.b   fix_mxall       ;
        bne.b   mxa1            ;
        moveq   #0, d0
        bra.b   memtest
mxa1:   move    4(a0), d0       ;
        subq    #1, d0          ; asking for TT ram
        bne.b   mxa2            ;
        moveq   #0, d0          ; if so, return 0
        bra.b   t1xr            ;
mxa2:   move    #72, -2(a0)     ; else make it a Malloc
mall:   moveq   #1, d0
        bra.b   memtest
mfre:   moveq   #2, d0
        bra.b   memtest
mshr:   moveq   #3, d0
        addq    #2, a0
memtest:move.b  sleeping(pc), d1
        or.b    no_set, d1
        bne     t1x
;;      move.l  bp_addr(pc), a1         ; 005: test for PD  006: removed
;;      move.l  (a1), a1
;;      cmpa.l  PD_bp(pc), a1
;;      beq.s   t1x                     ; 005a: was bne!
        move    d0, -(sp)
        pea     (a0)
        jsr     get_curapp
        tst     d0
        beq.b   mt1
        move.l  (sp)+, a0
        move    (sp)+, d0
        st      in_t1
        jsr     memory
        move.l  d0, -(sp)
        jsr     set_oldapp
        sf      in_t1
        move.l  (sp)+, d0
        move.l  d0, d1
        subq.l  #1, d1
        beq     t1x
t1xr:   movem.l (sp)+, a0-a1/d1-d2
        rte
mt1:    move.l  (sp)+, a0
        move    (sp)+, d0
        bra     t1x

t1_8:   cmpi    #100, d0        ; whole routine in 007
        bne     t1x
        pea     (a0)
        jsr     get_curapp
        move.l  (sp)+, a0
        tst     d0
        beq.b   t1_3x
        move.l  sp, a1
        lea     temp, sp
        pea     (a1)
        moveq   #0, d0
        moveq   #1, d1
        moveq   #1, d2
        move.l  4(a0), a1
        move.l  (a0), a0
        jsr     _shel_write
        move.l  (sp), sp
        jsr     set_oldapp
        bra.b   t1xr

.import sh_exec
t1_2:                           ; user Pexec
        tst.b   no_set
        beq.b   t1_9
        clr.b   no_set
        bra     t1x
t1_9:   move    (a0)+, d0
        bne     t1_8            ; 007
t1_3:
        st      in_t1
        movem.l a1-a2/d1-d2, -(sp)
        pea     (a0)
        jsr     get_curapp
        move.l  (sp)+, a0
        tst     d0
        beq.b   t1_3x
        move.l  sp, a1
        lea     temp, sp
        pea     (a1)
        move.l  8(a0), -(sp)
        move.l  4(a0), a1
        move.l  (a0), a0
        jsr     sh_exec
        move.l  4(sp), sp
        jsr     set_oldapp
t1_3x:  movem.l (sp)+, a1-a2/d1-d2
        sf      in_t1
        bra     t1x

.import pathtest
mypthret:
        pea     0.w
        movem.l a0-a2/a6/d0-d2, -(sp)
        tst.l   d0
        blt.b   mpr1
        jsr     get_curapp
        tst     d0
        beq.b   mpr1
        move.l  sp, a6
        lea     temp, sp
        move.l  pthptr(pc), a0
        jsr     pathtest
        move.l  a6, sp
        jsr     set_oldapp
mpr1:   move.l  pthret(pc), 7*4(sp)
        movem.l (sp)+, a0-a2/a6/d0-d2
        rts

.export get_t2
get_t2: move.l  $88.w, t2_term
        rts

.export old_t13_term
.export term_t13
        .ascii  'XBRA'
        dc.l    XBRA_ID
old_t13_term:
        ds.l    1
term_t13:
        btst    #5, (sp)
nop_me: bne.b   t13t_1
t13tx:  move.l  old_t13_term(pc), a0
        jmp     (a0)
t13t_1: lea     6(sp), a0
        adda    frame_off(pc), a0
        cmpi.l  #$50101, (a0)+
        bne.b   t13tx
        move.l  $404.w, d0
        move.l  (a0), d1
        blt.b   t13tr
        move    #$4E71, nop_me          ; change to "nop"
        move.l  $88.w, old_t2
        move.l  t2_term(pc), $88.w
        move    #$6000, my_t2           ; change to "bra out"
        move    #out-my_t2-2, my_t2+2
        move    #$4E71, vterm           ; 004
        move.l  d1, $404.w
t13tr:  rte

.export old_t13
.export my_t13
        .ascii  'XBRA'
        dc.l    XBRA_ID
old_t13:
        ds.l    1
my_t13:
.IFEQ DEBUG
        btst    #5, (sp)
        bne.b   t13_1
t13x:   move.l  old_t13(pc), a0
        jmp     (a0)
t13_1:  lea     6(sp), a0
        adda    frame_off(pc), a0
        cmpi.l  #$50101, (a0)+
        bne.b   t13x
        move.l  $404.w, d0
        move.l  (a0), d1
        blt.b   t13r
        move.l  d1, $404.w
t13r:   rte
.ELSE
/***    for testing: prevents Setexec from working ***/
        move.l  usp, a0
        btst    #5, (sp)
        beq.b   t13_0
        lea     6(sp), a0
t13_0:  adda    frame_off(pc), a0
        cmpi.l  #$50101, (a0)
        bne.b   t13_2
        move.l  $404.w, d0
        move.l  4(a0), d1
        blt.b   t13r
        move.l  d1, $404.w
t13r:   rte
t13_2:  cmpi    #$5, (a0)
        bne.b   t13x
        move.w  2(a0), a0
        adda.l  a0, a0
        adda.l  a0, a0
        move.l  (a0), d0
        rte
t13x:   move.l  old_t13(pc), a0
        jmp     (a0)
.ENDIF

evregs reg a0-a1/d0-d2
.import g_mx
.import g_my
.import g_mb
.export my_butv
.export old_butv
.import clicked
        .ascii  'XBRA'
        dc.l    XBRA_ID
old_butv:
        ds.l    1
my_butv:
        movem.l #evregs, -(sp)
        bsr     clicked
        movem.l (sp)+, #evregs
;        move.l  old_butv(pc), -(sp)
        rts

.export my_timv
.export old_timv
.import ticked
        .ascii  'XBRA'
        dc.l    XBRA_ID
old_timv:
        ds.l    1
my_timv:
        movem.l #evregs, -(sp)
        bsr     ticked
        movem.l (sp)+, #evregs
        move.l  old_timv(pc), -(sp)
        beq.b   mtv1    ; vex_timv has not returned old value yet
        rts
mtv1:   addq    #4, sp
        rts

.import recorder, record_event, player

.export my_motv, null_mouse
.export old_motv
        .ascii  'XBRA'
        dc.l    XBRA_ID
old_motv:
        ds.l    1
my_motv:
move_mouse:
        move    d0, g_mx
        move    d1, g_my
        move.l  old_motv(pc), -(sp)
        beq.b   mtv1    ; vex_motv has not returned old value yet
null_mouse:
        rts

trr2:   movem.l (sp)+, d0-d2/a0-a1
        bra.b   trr3

.import track_vdi
track_ret:
        tst.b   preempt
        beq.b   trr1
        movem.l d0-d2/a0-a1, -(sp)
        jsr     get_curapp
        tst     d0
        beq.b   trr2
        movem.l (sp)+, d0-d2/a0-a1
trr1:   lea     temp, a0
        movem.l d0-d1/a1/a7, -(a0)
        move.l  a0, sp
        move.l  vtr_ptr(pc), a0
        move    12(a0), d0
        beq.b   trr
        move    vtr_mode(pc), d1
        jsr     track_vdi
trr:    jsr     set_oldapp
        movem.l (sp), d0-d1/a1/a7
trr3:   movem.l (sp)+, a0-a1/d2 ; vmouse uses this too
        addq    #4, sp          ; don't chain
        rte

.export retrap_vdi
vrt:    dc.l    my_vdi
retrap_vdi:
        move.b  preempt(pc), d0
        or.b    sleeping(pc), d0
        or.b    is_PD(pc), d0
        bne.b   rtvx
        move.l  $88.w, a0
rtv3:   cmpi.l  #'XBRA', -12(a0)
        bne.b   rtv1
        cmpi.l  #'SMon', -8(a0)         ; if it's Sysmon
        bne.b   rtv1
        move.l  -4(a0), a0              ; continue chaining
        bra.b   rtv3
rtv1:   move.l  vrt(pc), a1
        cmp.l   a1, a0                  ; if it's already correct
        beq.b   rtvx                    ;   get out
rtv2:   cmp.l   #my_vdi1, a1            ; if it's at the last possible place
        beq.b   rtvx                    ;   get out
        move    sr, d0
        ori     #$700, sr
        move    #$4E75, vdirt-my_vdi(a1); set to rts
        sub     #my_vdi2-my_vdi1, a1    ; find previous
        move.l  a0, -4(a1)              ; set XBRA
        move.l  a1, $88.w               ; and into trap
        move.l  a1, vrt
        move    d0, sr
rtvx:   rts

vopw:   move    #$100, d2
        bra.b   dov
vclsw:  move    #$101, d2
        bra.b   dov
vvopw:  moveq   #0, d2
        bra.b   dov
vvclsw: moveq   #1, d2

dov:    move    d2, vtr_mode
        move.l  a0, vtr_ptr
        move.l  12(sp), a0              ; get old vector addr
        tst.w   $59e.w
        beq.b   dv1
        move.w  #$0088, -(sp)           ; format word
dv1:    pea     track_ret(pc)
        move    sr, -(sp)
        jmp     (a0)

        .ascii  'XBRA'
        dc.l    XBRA_ID
oldvdi1:ds.l    1
my_vdi1:move.l  oldvdi1(pc), -(sp)
        bra.b   vdistart

        .ascii  'XBRA'
        dc.l    XBRA_ID
oldvdi2:ds.l    1
my_vdi2:move.l  oldvdi2(pc), -(sp)
        bra.b   vdistart

        .ascii  'XBRA'
        dc.l    XBRA_ID
oldvdi3:ds.l    1
        move.l  oldvdi3(pc), -(sp)
        bra.b   vdistart

        .ascii  'XBRA'
        dc.l    XBRA_ID
oldvdi4:ds.l    1
        move.l  oldvdi4(pc), -(sp)
        bra.b   vdistart

.export my_vdi
.export old_vdi
        .ascii  'XBRA'          ; 004: VDI is installed separately now
        dc.l    XBRA_ID
old_vdi:ds.l    1
my_vdi:
        move.l  old_vdi(pc), -(sp)
vdirt:  nop                     ; gets changed to rts if retrapped
vdistart:
        cmp.w   #$73, d0
vterm:  beq.b   vok             ; term_t13 depends on this
vout:   rts
vok:    tst.b   sleeping
        bne.b   vout
        movem.l a0-a1/d2, -(sp)
        move.l  d1, a0
        move.l  (a0), a0
        move    (a0), d2
        cmpi    #129, d2
        bhi.b   vdicont
        lea     vditbl1(pc), a1
        move.b  0(a1,d2), d2
        move.w  vditbl2-vditbl1(a1,d2.w), d2
        jmp     vdicont(pc,d2.w)
/*****  subq    #1, d2
        beq.b   vopw
        subq    #1, d2
        beq.b   vclsw
        subi    #14-2, d2
        beq.b   vscolor
        subi    #32-14, d2
        beq.b   vswr
        subi    #100-32, d2
        beq.b   vvopw
        subq    #1, d2
        beq.b   vvclsw
        sub     #120-101, d2
        beq.b   unldf
        subq    #122-120, d2
        beq.b   vshowc
        subq    #123-122, d2
        beq.b   vhidec
        subq    #129-123, d2
        beq.b   vsclip ********/
vdicont:
vdic1:  movem.l (sp)+, a0-a1/d2
        rts                     ; return is already on stack

.import wr_mode
vswr:   move    12(a0), d2      ; get handle
        cmp     vdi_hand, d2    ; is it mine?
        bne.b   vdicont
        move.l  d1, a0
        move.l  4(a0), a0
        move    (a0), wr_mode
        bra.b   vdicont

.import clip_arr, clip_on, vdi_hand, have_fonts, txt_point, gr_font
vsclip: move    12(a0), d2      ; get handle
        cmp     vdi_hand, d2    ; is it mine?
        bne.b   vdicont
        bra.b   _vscl

unldf:  move    12(a0), d2      ; get handle
        cmpi    #11, d2         ; is a screen?
        bge.b   vdicont
        clr.b   have_fonts
        st      txt_point
        st      gr_font
        bra.b   vdicont

.import mouse_onoff
.import my_mouse
vshowc: moveq   #-1, d2         ; show
        move.l  d1, a0
        move.l  4(a0), a0
        tst     (a0)
        bne.b   vmouse
        moveq   #0, d2          ; reset
        bra.b   vmouse
vhidec: moveq   #1, d2          ; hide
vmouse: tst.b   in_t2
        bne     vdicont
        tst.b   is_PD
        bne     vdicont
        subq.b  #1, my_mouse
        bge     vdicont
        clr.b   my_mouse
        movem.l d0-d1, -(sp)
        move    d2, d0
        jsr     mouse_onoff
        tst     d0
        movem.l (sp)+, d0-d1
        bne.b   mshook          ; from tst
        bra     trr3

_vscl:  pea     (a1)
        move.l  d1, a0
        move.l  8(a0), a0
        lea     clip_arr, a1
        cmpm.l  (a1)+, (a0)+    ; is this the same as what's in clip_arr?
        bne.b   vsbad
        cmpm.l  (a1)+, (a0)+
        beq.b   vsout
vsbad:  move    #-1, clip_on    ; no, reset clip_on so that next _vs_clip
vsout:  move.l  (sp)+, a1       ; will definitely redo vs_clip
        bra     vdicont

.import usrcolor
vqcolor:moveq   #0, d2          ; getset
        move.l  d1, a1
        move.l  12(a1), a0      ; intout
        move    #1, (a0)+       ; intout[0] = 1
        bra.b   vsc

vscolor:moveq   #1, d2          ; getset
        move.l  d1, a1
        move.l  4(a1), a0
        addq    #2, a0          ; &intin[1]
vsc:    movem.l d0-d1/a2, -(sp)
        move.l  4(a1), a2
        move    (a2), d1        ; intin[0]
        move.l  (a1), a1        ; contrl
        move    12(a1), d0      ; vdi hand
        jsr     usrcolor
        tst     d0
        movem.l (sp)+, d0-d1/a2
        beq     vdicont         ; from tst: allow color to be set
        bra     trr3

mshook: move.l  4+4+4(sp), a0
        tst.w   $59e.w
        beq.b   msh1
        move    #$88, -(sp)     ; exception frame format 0, vector 88
msh1:   pea     mshret(pc)
        move    sr, -(sp)
        st      in_vdi
        jmp     (a0)
mshret: sf      in_vdi
        bra     trr3

vditbl2:dc.w    0, vopw-vdicont, vclsw-vdicont, vscolor-vdicont
        dc.w    vswr-vdicont, vvopw-vdicont, vvclsw-vdicont
        dc.w    unldf-vdicont, vshowc-vdicont, vhidec-vdicont
        dc.w    vsclip-vdicont, vqcolor-vdicont
vditbl1:dc.b    0, 2, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0
        dc.b    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,22, 0, 0, 0, 0, 0
        dc.b    8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        dc.b    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        dc.b    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        dc.b    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        dc.b    0, 0, 0, 0,10,12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        dc.b    0, 0, 0, 0, 0, 0, 0, 0,14, 0,16,18, 0, 0, 0, 0
        dc.b    0,20
.even

.import Linea

oldla92:ds.l    1
la9min: move.l  oldla92(pc), -(sp)
        bra.b   do_l91
oldla91:ds.l    1
        move.l  oldla91(pc), -(sp)
        bra.b   do_l91
old_la9:ds.l    1
my_la9: move.l  old_la9(pc), -(sp)
lanop:  nop
do_l91: moveq   #-1, d0         ; show
        move.l  Linea, a0
        move.l  8(a0), a0       ; get intin
        tst     (a0)
        bne.b   my_la1
        moveq   #0, d0          ; reset
        bra.b   my_la1
oldlaa2:ds.l    1
laamin: move.l  oldlaa2(pc), -(sp)
        bra.b   do_la1
oldlaa1:ds.l    1
        move.l  oldlaa1(pc), -(sp)
        bra.b   do_la1
old_laa:ds.l    1
my_laa: move.l  old_laa(pc), -(sp)
        nop
do_la1: moveq   #1, d0          ; hide
my_la1: move.b  sleeping(pc), d1
        or.b    in_vdi(pc), d1
        or.b    is_PD(pc), d1
        bne.b   my_laok
        subq.b  #1, my_mouse
        bge     my_laok
        clr.b   my_mouse
        jsr     mouse_onoff
        tst     d0
        bne.b   my_laok
        addq    #4, sp          ; don't allow it to happen
my_laok:rts

la9ptr: dc.l    my_la9
laaptr: dc.l    my_laa

.export retrap_la
retrap_la:
        pea     (a2)
        bsr     move_linea
        lea     9*4(a2), a2
        lea     la9ptr(pc), a1
        lea     la9min(pc), a0
        bsr.b   rtla
        lea     4(a2), a2
        lea     laaptr(pc), a1
        lea     laamin(pc), a0
        bsr.b   rtla
        move.l  (sp)+, a2
        rts

rtla:   move.l  (a1), d0
        cmp.l   (a2), d0                ; if it's already correct
        beq.b   rtlx                    ;   get out
        cmp.l   d0, a0                  ; if it's at the last possible place
        beq.b   rtlx                    ;   get out
        move    sr, d1
        ori     #$700, sr
        move.l  d0, a0
        move    #$4E75, lanop-my_la9(a0); set to rts
        sub     #oldla91-oldla92, a0    ; find previous
        move.l  (a2), -4(a0)            ; set old
        move.l  a0, (a2)                ; and into trap
        move.l  a0, (a1)                ; and into ptr
        move    d1, sr
rtlx:   rts

apyld:  dc.l    apyfc, 0, 0, apyinto, 0, 0      ; 007
apyfc:  dc.w    $11                             ; 007
apyinto:ds.w    1                               ; 007

mt21:   cmp.w   #$C9, d0
        bne     out             ; 004: was going to vdi handler
        lea     apyld(pc), a0   ; 007: do appl_yield
        move.l  a0, d1
        bra.b   t2apy
;;      moveq   #0, d0
;;      rte

.import Syield

ill:    ;;tst.b has_mint
        ;;beq.b ill2
;;ill3: movem.l d0-d2/a0-a1,-(sp)
;;      bsr     Syield
;;      movem.l (sp)+, d0-d2/a0-a1
;;      tst.b   in_t2
;;      bne.b   ill3
;;      bra.b   t2_c
ill2:   lea     t2_fatal(pc), a0
        bsr     fatal
        moveq   #0, d0
        rte

.export my_t2
.export old_t2
        .ascii  'XBRA'
        dc.l    XBRA_ID
old_t2: ds.l    1

t2r reg d0-d7/a0-a5

my_t2:
        cmp.w   #$C8, d0
        bne.b   mt21
t2apy:  tst.b   preempt
        bne.b   t2rept
        tst.b   in_t2           ; remove me someday
        bne.b   ill
t2rept: pea     (a6)
        move.l  usp, a6
        move.l  (a7), -(a6)
        movem.l #t2r, -(a6)
        move.l  a6, usp
        move.l  (sp)+, a6
.ifne TEST
        move.l  d1, tst_d1
.endif
        move.l  d1, a5
        move.l  (a5), a1
        move    (a1), d0
        move    d0, d2
        move.l  8(a5), a1
        move.l  4(a5), a2
        moveq   #1, d3                  ; no_entry flag for dispatch
        cmpi    #10, d0
        blo.b   badno
        cmpi    #130, d0
        bhi.b   badno
        lea     tbl(pc), a2
        add     d0, d0
        move    -10*2(a2,d0), d1
        lea     t2atbl(pc), a2
        add     d0, d0
        move.l  -10*4(a2,d0), a2
        jmp     2(pc,d1)
out2:   bra.b   xbad
        pea     (a6)
        move.l  usp, a6
        movem.l (a6)+, #t2r
        move.l  (a6)+, (sp)
        move.l  a6, usp
        move.l  (sp)+, a6
out:    move.l  old_t2(pc), -(sp)
        rts

.import bad_call
.import spf
badno:  subi    #$7100, d0
        bcs.b   xbad
        cmpi    #EXTRAS, d0
        bhi.b   xbad
        add     d0, d0
        lea     xtbl(pc), a2
        move    0(a2,d0), d1
        lea     t2xtbl(pc), a2
        add     d0, d0
        move.l  0(a2,d0), a2
        jmp     2(pc,d1)
xbad:   move    d2, d0
        lea     bad_call, a0
        lea     spf_alert, a2
        bra     wcl

.import spf_alert
.import dispatch, wind_app
.import magic_bp, get_curapp
.bss
        ds.b    2000
t2stack:
.text

.import app0, set_proc

set_stack:
        st.b    in_t2
        pea     (a0)                    ; also a0
        move    d3, -(sp)               ; save d3 temporarily
        pea     (a1)                    ; also a1
        move.b  preempt(pc), d3
        beq.b   ss7
        movem.l d0-d2/a0-a1, -(sp)
sss2:   tas     havet2st
        beq.b   sss1
        addq.b  #1, wait_curapp
        jsr     Syield
        subq.b  #1, wait_curapp
        bra.b   sss2
sss1:   jsr     get_curapp
        movem.l (sp)+, d0-d2/a0-a1
ss7:    move.l  curstack(pc), a0
        lea     10(sp), a1              ; stack on entry
        move.l  (a1)+, -(a0)            ; rts addr
        move.w  (a1)+, -(a0)            ; sr
        move.l  (a1)+, -(a0)            ; rte addr
        tst     $59e.w
        beq.b   ss2
        move    (a1)+, -(a0)            ; format word
ss2:    move.l  a1, -(a0)               ; save it
        move.l  usp, a1
        move.l  a1, -(a0)               ; save usp
        move.l  a5, -(a0)               ; save parmblk ptr
        move.l  bp_addr(pc), a1
        move.l  (a1), a5
        ;; don't change what is on (a0) without altering dispatch!
        move.l  a5, curr_bp
        move.b  preempt(pc), d3
        bne.b   ss3
        move.l  a5, -(a0)               ; save context
        move.l  #magic_bp, (a1)
        movem.l a0/d0-d2, -(sp)
        move.l  app0, a0
        jsr     set_proc
        movem.l (sp)+, a0/d0-d2
ss3:    move.l  (sp)+, a1               ; restore a1
        move    (sp)+, -(a0)            ; save d3
        move.w  #1, -(a0)               ; default return=1 (for shel_write)
        move.l  a0, a5                  ; save curstack ptr
        move.l  (sp)+, a0               ; restore a0
        move.l  #t2stack, sp                  ; now set stack
        move.l  curapp, wind_app        ; used for getting old window handles
        move.l  a2, last_func           ; 004
        jsr     (a2)                    ; call function
        clr.l   last_func               ; 005
        clr.l   wind_app                ; so internal window calls work right
        move.l  curapp, d1              ; skip if curapp is null
        beq.b   ss5
        move    d0, (a5)                ; save return on curstack for later
        move    2(a5), d0               ; get what used to be d3
        move.l  a5, d1                  ; current stack
ss5:    move.b  preempt(pc), d3
        beq.b   ss8
        move.l  12(a5), sp              ; restore ssp
        clr.b   havet2st
        bclr    #0, block_app           ; 004: test no_evnt & block_app
        bne.b   ss8
        tst     d0
        bne.b   sspne
ss8:    jsr     dispatch                ; call dispatcher
        move.l  d0, a0                  ; set stack
ss9:    move    (a0), d0                ; restore function return
        addq    #4, a0                  ; skip "d3"
        move.b  preempt(pc), d3
        bne.b   ss6
        move.l  bp_addr(pc), a1
        move.l  (a0)+, (a1)             ; set context
ss6:    move.l  (a0)+, a5               ; restore parmblk ptr
        move.l  (a0)+, a1               ; restore usp
        move.l  a1, usp
        move.l  (a0)+, sp               ; restore ssp
        tst     $59e.w
        beq.b   ss4
        move.w  (a0)+, -(sp)            ; format
ss4:    move.l  (a0)+, -(sp)            ; rte addr
        move.w  (a0)+, -(sp)            ; sr
        move.l  (a0), -(sp)             ; rts addr
        move.l  a5, a1
        move.l  12(a1), a0              ; intout
        sf      in_t2
        rts

sspne:  move.l  d1, a0
        bra.b   ss9

.import multi
mesag:  dc.w    $10
kbd:    dc.w    $01
btn:    dc.w    $02, 0, 0, 0
mse:    dc.w    $04, 0, 0, 0,  0, 0, 0, 0, 0
timr:   dc.w    $20, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0

.import nobuf
evnt_ke:
        moveq   #0, d3          ; no_entry flag for dispatch
        lea     kbd(pc), a1
        lea     nobuf, a0
        bsr     set_stack
        move    emult_out+10(pc), (a0)
        bra     out3

evnt_bu:
        moveq   #0, d3          ; no_entry flag for dispatch
        lea     btn(pc), a0
        move.l  (a1)+, 2(a0)
        move    (a1), 6(a0)
        move.l  a0, a1
        lea     nobuf, a0
        bsr     set_stack
        lea     emult_out+2(pc), a1
        move    10(a1), (a0)+
evmo:   move.l  (a1)+, (a0)+
        move.l  (a1), (a0)
        bra     out3

evnt_mo:
        moveq   #0, d3          ; no_entry flag for dispatch
        lea     mse(pc), a0
        move.l  (a1)+, 8(a0)
        move.l  (a1)+, 12(a0)
        move    (a1), 16(a0)
        move.l  a0, a1
        lea     nobuf, a0
        bsr     set_stack
        lea     emult_out+2(pc), a1
        move    #1, (a0)+
        bra.b   evmo

evnt_me:
        moveq   #0, d3          ; no_entry flag for dispatch
        lea     mesag(pc), a1
;;        move.l  16(a5), a0
;;        move.l  (a0), a0
        lea     nobuf, a0
        bsr     set_stack
        move    #1, (a0)
        bra.b   evme

evnt_ti:
        moveq   #0, d3          ; no_entry flag for dispatch
        lea     timr(pc), a0
        move.l  (a1), 28(a0)
        move.l  a0, a1
        lea     nobuf, a0
        bsr     set_stack
        move    #1, (a0)
        bra.b   out3

.import lock_curapp, repeat_func, last_func

evnt_mu:
;        move.l  16(a5), a0
;        move.l  (a0), a0
        lea     nobuf, a0
        moveq   #0, d3          ; no_entry flag for dispatch
        bsr     set_stack               ; a1 stays put
        lea     emult_out(pc), a1
        move.l  (a1)+, (a0)+
        move.l  (a1)+, (a0)+
        move.l  (a1)+, (a0)+
        move    (a1), (a0)
        move    -12(a0), d0
        andi    #$4010, d0      ; 004: is it MU_MESAG|X_MU_DIALOG?
        beq.b   out3            ;      no, skip msgbuf copy
evme:   move.l  16(a5), a0      ; 004: use nobuf instead of app's buf
        move.l  (a0), a0        ;      so memory protection works
        lea     nobuf, a1
        move.l  (a1)+, (a0)+
        move.l  (a1)+, (a0)+
        move.l  (a1)+, (a0)+
        move.l  (a1), (a0)
out3:   clr     lock_curapp     ; 005 word
        pea     (a6)
        move.l  usp, a6
        movem.l (a6)+, #t2r
        move.l  (a6)+, (sp)
        move.l  a6, usp
        move.l  (sp)+, a6
;;      move.b  repeat_func, d0
;;      bne.b   axc2
        bclr.b  #0, repeat_func ; 004
        bne     t2rept          ; 004
axc1:   moveq   #0, d0
        rte

.import evnt_dclick
evnt_dc:movem.w (a1), d0-d1
        bra     wcl

.import icheck_menu
.import menu_popup
.import menu_attach
.import x_form_filename
.import x_appl_trecord
xtrec:
menu_ic:
mnu_pop:
mnu_att:
xfofi:
mnu_ch: movem.w (a1), d0-d1
        move.l  16(a5), a1
        move.l  (a1)+, a0
        move.l  (a1), a1
        bra     wcl

trec:   moveq   #0, d1
        move    (a1), d0
        suba.l  a1, a1
        bra.b   obo

.import ienable_menu
.import tnormal_menu
.import text_menu
.import _menu_bar
.import menu_settings
.import x_shel_put
mnu_set:
menu_ba:
xshpu:
        move    (a1), d0
        bra.b   obo

.import _menu_register

.import offset_objc
objc_o:
aps:    move    (a1)+, d0
        move.l  12(a5), a1
        addq    #2, a1          ; &intout[1]
obo:    move.l  16(a5), a0      ; addrin[0]
        move.l  (a0), a0
        bra     wcl

.import objc_add
.import objc_delete
.import obj_draw        ; uses a1
.import _objc_find      ; uses a1
.import objc_order
.import _objc_change    ; uses a1
.import objc_edit
.import __xobjc_edit
xobj_ed:move    8(a1), d1
        swap    d1
objc_ed:
obed:   move    (a1)+, -(sp)
        move    (a1)+, d1
        move    (a1)+, d0
        move    (a1), d2
        move.l  16(a5), a0
        move.l  (a0), a0
        move.l  12(a5), a1
        addq    #2, a1
        move    d0, (a1)                ; set old value into return
        move    (sp)+, d0
        bra     wcl

.import _form_do

.import _form_dial
fodi:   move    (a1)+, d0
        move.l  a1, a0
        addq    #8, a1
        bra     wcl

.import x_malloc
.import lrealloc
.import lshrink
xmshr:
xreal:
xmal:   move.l  (a1), d0
        bra.b   obf

.import _form_alert
.import x_wind_tree
.import x_form_error
.import _graf_mouse
.import x_scrp_get
.import lfree
grfm:   moveq   #1, d1          ; set4app
form_al:
wintr:
xfoer:
xscrp:
frd:    move    (a1), d0
xmfr:
obf:    move.l  16(a5), a0
        move.l  (a0), a0
        bra     wcl

.import form_error

.import _form_center
xfoce:  moveq   #1, d0
        bra.b   foce2
form_ce:moveq   #0, d0
foce2:  move.l  12(a5), a1
        addq    #2, a1
        bra.b   obf

.import __form_keybd
form_ke:movem.w (a1)+, d1-d2
        move    (a1), -(sp)
        move.l  16(a5), a0
        move.l  (a0), d0
        move.l  12(a5), a1
        lea     2(a1), a0
        move    (sp)+, (a0)
        addq    #4, a1
        bra.b   wcl

.import form_button
form_bu:move.l  16(a5), a0
        move.l  (a0), a0
        movem.w (a1), d0-d1
        bra.b   fbu

.import calc_wind
xwcalc: movem.w (a1)+, d0-d2
        bra.b   xwc

caw:    movem.w (a1)+, d0-d1
        clr     d2
xwc:    move.l  12(a5), a0      ; &intout[1]
        addq    #2, a0
        bra.b   wcl
.import create_window
xwind_cr:
        movem.w (a1)+, d0-d1
        bra.b   wop
wind_cr:
        move    (a1)+, d0
        clr     d1
wop:    move.l  a1, a0
wcl:    bsr     set_stack
        move    d0, (a0)
        bra     out3
.import open_window
.import close_window
wind_cl:
wdl:    move    (a1), d0
        bra.b   wcl
.import delete_window
.import get_window
.import x_appl_sleep
wind_ge:
        move.l  12(a5), a0
        addq    #2, a0          ; &intout[1]
        move    4(a1), (a0)     ; for WF_DCOLOR/COLOR
xapsl:
wfi:    movem.w (a1), d0-d1
        bra.b   wcl

.import set_window
.import _find_wind
.import update
.import _wind_update
.import _wind_new

.import _appl_getinfo

apgi:   move    (a1), d0
        bra.b   mks

.import _graf_dragbox
grf_dr: clr     d0              ; mode: don't blit
mks:    move.l  12(a5), a0
        addq    #2, a0          ; &intout[1]
        bra.b   wcl

.import rubberbox_graf
.import _objc_sysvar
obsys:
grf_ru: move.l  a1, a0
fbu:    move.l  12(a5), a1
        addq    #2, a1          ; &intout[1]
        bra     wcl

.import _mks_graf

rnull:
        moveq   #1, d0
        rts

.import graf_slidebox
.import graf_watchbox
.import menu_istart
grf_wa:
        addq    #2, a1
grf_sl:
mnu_ist:
xtplay:
grwa:   movem.w (a1), d0-d2
        bra.b   grsl

.import _graf_handle
grf_ha: move.l  12(a5), a0
        addq    #2, a0
        bra     wcl

.import _appl_read
ap_read:
        movem.w (a1), d0-d1
        clr     d3              ; clear no_entry flag
        move.l  16(a5), a0      ; this won't work with memory prot
        move.l  (a0), a0        ;
        bra     wcl

.import scrp_read
.import scrp_write
.import uappl_write
.import x_shel_get
.import x_appl_flags
.import _x_appl_font
.import x_appl_tplay
tplay:
        clr     d2              ; 004: clear mode
;;      bra.b   oba             ; 005: removed. Not needed because of 004 comment below
ap_write:
;;      clr     d2              ;; 004: removed ; don't consider AES40 flag
xshge:
xapf:
oba:    movem.w (a1), d0-d1
grsl:   move.l  16(a5), a0      ; addrin[0]
        move.l  (a0), a0
        bra     wcl

.import app_find
.import app_init
ap_init:/* hack to fix up cleared-out parent pointer for DA's */
;        move.l bp_addr(pc), a1
;        move.l (a1), a1
        move.l  4(a5), a0
        bra     wcl

.import appl_exit
.import _appl_yield, dispatch2, Pgetpid, wait_id
/**apyd:        move.b  preempt(pc), d0         005: use direct method: _appl_yield changed, jump table goes to wcl
        beq     wcl
        jsr     Pgetpid
        jsr     wait_id
        tst     d0
        beq     wcl
        jsr     dispatch2
        clr     lock_curapp     ; 005 word
        bra     wcl     **/

.import _appl_search

.import _rsrc_rcfix
.import _rsrc_load
rsrc_lo:
        move.l  4(a5), a1
        lea     10(a1), a1              ; &global[5]
        bra     obo
.import _rsrc_free
.import _rsrc_gaddr
rsrc_ga:
rsrs:   movem.w (a1), d0-d1
        move.l  20(a5), a1
rsrc_fr:
rsr:    move.l  4(a5), a0
        lea     10(a0), a0              ; &global[5]
        bra     wcl

.import _shel_write, swrite_wait
shel_wr:
;;      move    (a1), d0                005: modified swrite_wait instead
;;      jsr     swrite_wait             ; 004: wait for other writes to finish
;;      move.l  8(a5), a1               ; re-fetch intin
        movem.w (a1), d0-d2
.import _rsrc_saddr
.import obfix
.import shel_read
.import shel_envrn
shel_re:
shel_env:
        move.l  16(a5), a1
        move.l  (a1)+, a0
        move.l  (a1), a1
        bra     wcl

.import shel_get
shel_ge:
shp:    move    (a1), d0
shf:    move.l  16(a5), a1
        move.l  (a1), a0
        bra     wcl

.import shel_put
.import shel_find
.import _fsel_exinput

.import _x_fsel_input
xfsin:  movem.w (a1), d0-d2
        move.l  12(a5), a1
        addq    #2, a1
        move.l  16(a5), a0
        bra     wcl

.import _fsel_input
fsel:
fsl:    move.l  12(a5), d0
        addq.l  #2, d0
        move.l  16(a5), a0
        move.l  (a0)+, -(sp)
        move.l  (a0)+, a1
        move.l  (a0)+, d1
        move.l  (sp)+, a0
        bra     wcl

.import _spf
.import sscnf_
xspf:   move.l  16(a5), a0
        bra     wcl

.import x_wdial_draw
.import x_wdial_change
.import __x_appl_term
xaptrm:
xwdd:   movem.w (a1), d0-d2
        bra     wcl

.import x_settings

.import _form_mouse
form_mo:movem.w (a1)+, d0-d2
        move.l  12(a5), a0      ; intout[1]
        addq    #2, a0
        move.l  a0, -(sp)
        move.l  (a1)+, (a0)+
        move    (a1), (a0)
        move.l  16(a5), a1
        move.l  (a1), a0
        move.l  (sp)+, a1
        bra     wcl

.import x_graf_blit
.import _x_help
xhelp:
        move    (a1), d0
grf_b:  move.l  16(a5), a1
        move.l  (a1)+, a0
        move.l  (a1), a1
        bra     wcl

.import growbox
growb:  move.l  a1, a0
        addq    #8, a1
        bra     wcl

.import shrinkbox
shrnkb: move.l  a1, a0
        addq    #8, a0
        bra     wcl

.import movebox
mvbx:   move.l  a1, a0
        bra     wcl

.import _x_graf_rubberbox
xgrru:  move.l  a1, d0
        bra.b   grf_b

.import xfix_cicon
xfixc:  move.l  (a1)+, d0
        movem.w (a1), d1-d2
        bra.b   grf_b

.export curstack
.import curapp, x_set_curapp /* hidden function */
.export emult_out, t2atbl, t2xtbl
t2atbl: dc.l    app_init,    _appl_read,    uappl_write,    app_find,       x_appl_tplay
        dc.l    x_appl_trecord, 0, _appl_yield, _appl_search, appl_exit, multi
        dc.l    multi,        multi,        multi,              multi
        dc.l    multi,              evnt_dclick,        0, 0, 0
        dc.l    _menu_bar,          icheck_menu, ienable_menu, tnormal_menu
        dc.l    text_menu,         _menu_register, menu_popup,   menu_attach,  menu_istart, menu_settings, objc_add, objc_delete
        dc.l    obj_draw,       _objc_find, offset_objc,       objc_order
        dc.l    objc_edit,    _objc_change, _objc_sysvar, 0, _form_do, _form_dial
        dc.l    _form_alert,  form_error, _form_center, __form_keybd
        dc.l    form_button,  0, 0, 0, 0, 0, 0, 0, 0, 0
        dc.l    0,       0, 0, 0, rubberbox_graf,    _graf_dragbox
        dc.l    movebox,    growbox,    shrinkbox,    graf_watchbox, graf_slidebox
        dc.l    _graf_handle, _graf_mouse, _mks_graf,     scrp_read
        dc.l    scrp_write, 0, 0, 0, 0, 0,       0, 0, 0, _fsel_input, _fsel_exinput
        dc.l    0, 0, 0, 0, 0, 0, 0, 0, create_window, open_window, close_window
        dc.l    delete_window, get_window,   set_window, _find_wind, _wind_update
        dc.l    calc_wind, _wind_new, _rsrc_load,   _rsrc_free,   _rsrc_gaddr
        dc.l    _rsrc_saddr,      obfix,    _rsrc_rcfix, 0, 0, 0, 0, shel_read
        dc.l    _shel_write,  shel_get,     shel_put, shel_find, shel_envrn
        dc.l    0, 0, 0, 0, _appl_getinfo /* 130 */
tbl:    dc.w    ap_init-out2, ap_read-out2, ap_write-out2, obo-out2/*13*/, tplay-out2
        dc.w    trec-out2,      0, wcl-out2,   aps-out2,     wcl-out2,  evnt_ke-out2/*20*/      ; 005: appl_yield->wcl
        dc.w    evnt_bu-out2, evnt_mo-out2, evnt_me-out2/*23*/, evnt_ti-out2
        dc.w    evnt_mu-out2/*25*/, evnt_dc-out2/*26*/, 0, 0, 0
        dc.w    menu_ba-out2/*30*/, menu_ic-out2, mnu_ch-out2, mnu_ch-out2
        dc.w    mnu_ch-out2/*34*/, frd-out2, mnu_pop-out2, mnu_att-out2, mnu_ist-out2, mnu_set-out2,  oba-out2, frd-out2
        dc.w    obo-out2/*42*/, obf-out2,   objc_o-out2/*44*/, oba-out2
        dc.w    objc_ed-out2, obo-out2, obsys-out2, 0, frd-out2, fodi-out2/*51*/
        dc.w    form_al-out2, wdl-out2,   form_ce-out2, form_ke-out2
        dc.w    form_bu-out2, 0, 0, 0, 0, 0, 0, 0, 0, 0
        dc.w    /*66*/0, 0, 0, 0, grf_ru-out2/*70*/, grf_dr-out2/*71*/
        dc.w    mvbx-out2, growb-out2, shrnkb-out2, grf_wa-out2,   grf_sl-out2/*76*/
        dc.w    grf_ha-out2,  grfm-out2,   mks-out2/*79*/, grsl-out2
        dc.w    grsl-out2,  0, 0, 0, 0, /*86*/0, 0, 0, 0, fsel-out2,   fsel-out2
        dc.w    0, 0, 0, 0, 0, 0, 0, 0, wind_cr-out2,  wop-out2,    wind_cl-out2
        dc.w    wdl-out2,      wind_ge-out2, wop-out2,   wfi-out2, wdl-out2
        dc.w    caw-out2, wcl-out2,   rsrc_lo-out2, rsrc_fr-out2, rsrc_ga-out2
        dc.w    /*113*/rsrs-out2, frd-out2, rsrc_lo-out2, 0, 0, 0, 0, shel_re-out2
        dc.w    shel_wr-out2, shel_ge-out2, shp-out2, shf-out2, shel_env-out2
        dc.w    0, 0, 0, 0, apgi-out2 /*130*/
t2xtbl: dc.l    x_appl_flags, _x_appl_font, x_appl_sleep, __x_appl_term
        dc.l    x_appl_tplay, x_appl_trecord, x_set_curapp
        ds.l    9
        dc.l    _form_center, x_form_error, x_form_filename, _form_mouse/* $10 */
        ds.l    12
        dc.l    _x_fsel_input                                           /* $20 */
        ds.l    15
        dc.l    x_graf_blit, _x_graf_rubberbox, xfix_cicon              /* $30 */
        ds.l    13
        dc.l    __xobjc_edit                                            /* $40 */
        ds.l    15
        dc.l    x_wdial_draw, x_wdial_change, x_wind_tree, create_window/* $50 */
        dc.l    calc_wind
        ds.l    11
        dc.l    x_scrp_get                                              /* $60 */
        ds.l    14
        dc.l    in_t2
        ds.l    16                                                      /* $70 */
        dc.l    x_settings,  x_shel_get, x_shel_put, _spf               /* $80 */
        dc.l    sscnf_,    _x_help,    x_malloc,  lfree
        dc.l    lshrink,   lrealloc
xtbl:   dc.w    xapf-xbad,    oba-xbad,     xapsl-xbad,   xaptrm-xbad
        dc.w    xtplay-xbad, xtrec-xbad, 0/*x_set_curapp*/
        ds.w    9
        dc.w    xfoce-xbad,   xfoer-xbad,   xfofi-xbad,      form_mo-xbad
        ds.w    12
        dc.w    xfsin-xbad
        ds.w    15
        dc.w    grf_b-xbad,  xgrru-xbad,        xfixc-xbad
        ds.w    13
        dc.w    xobj_ed-xbad
        ds.w    15
        dc.w    xwdd-xbad,    xwdd-xbad,      wintr-xbad,  xwind_cr-xbad
        dc.w    xwcalc-xbad
        ds.w    11
        dc.w    xscrp-xbad
        ds.w    15+16
        dc.w    mnu_ch-xbad, xshge-xbad, xshpu-xbad, xspf-xbad
        dc.w    xspf-xbad, xhelp-xbad, xmal-xbad, xmfr-xbad
        dc.w    xmshr-xbad, xreal-xbad

t1atbl: dc.w    t1_termp-t1x, 0, 0, 0, 0, 0, 0, 0, path-t1x, path-t1x
        dc.w    0, path-t1x, 0, 0, 0, 0, path-t1x, 0, 0, mxall-t1x
        dc.w    0, 0, 0, mall-t1x, mfre-t1x, mshr-t1x, t1_2-t1x
        dc.w    t1_term-t1x, 0, 0, 0, 0, 0, 0, 0, 0, 0, fren-t1x

.import crit_error
.export old_crit

cregs reg a1-a2/d1-d2
        .ascii  'XBRA'
        dc.l    XBRA_ID
old_crit:
        ds.l    1
critical:
        st      in_t1
        movem.l #cregs, -(sp)
        move    4+16(sp), d0
        move    6+16(sp), d1
        move.l  sp, a0
        lea     temp, sp
        pea     (a0)
        jsr     crit_error
        move.l  (sp), sp
        subq    #1, d0
        bne.b   crit3
        move.l  #$10000, d0
        bra.b   crit4
crit3:  move    4+16(sp), d0
        ext.l   d0
crit4:  movem.l (sp)+, #cregs
        sf      in_t1
        rts

.export setexc
setexc:
        movem.l a2-a5/d3-d4, -(sp)
        move.w  d0, a5
        move.w  d1, d3
        move.l  a0, a3
        move.l  a1, a4
        pea     1.w
        move    #$20, -(sp)
        trap    #1
        addq    #6, sp
        tst     d0
        sne     d4
        bne.b   se0
        pea     0.w
        move    #$20, -(sp)
        trap    #1
        move.l  d0, 2(sp)
se0:    move.l  a4, d0
        bne.b   se1
        move.l  a5, a2          ; 004: used to set too early
ne1:    move.l  (a2), a0
        move.l  a0, d0
        beq.b   force
        cmpi.l  #'XBRA', -12(a0)
        bne.b   force
        cmpi.l  #XBRA_ID, -8(a0)
        beq.b   ne2
        lea     -4(a0), a2
        bra.b   ne1
ne2:    move.l  -4(a0), (a2)
        st      d3
        bra.b   sx
force:  tst     d3
        beq.b   sx
        move.l  a3, (a5)
        st      d3
        bra.b   sx
se1:    move.l  (a5), (a4)
        move.l  a3, (a5)
sx:     tst.b   d4
        bne.b   sx2
        trap    #1
        addq    #6, sp
sx2:    move    d3, d0
        movem.l (sp)+, a2-a5/d3-d4
        rts

.export warmboot
warmboot:
        pea     0.w
        move    #$20, -(sp)
        trap    #1
        movea.l $4.w, a0
        jmp     (a0)

.export breakpoint
breakpoint:
        lea     bp_fatal(pc), a0
        pea     (a2)
        bsr     fatal
        move.l  (sp)+, a2
        rts

.export userdef
usrreg reg d3-d7/a2-a6
userdef:movem.l #usrreg, -(sp)
        pea     0.w             ; Super stack
        pea     (a1)            ; push PARMBLK ptr
objh:   pea     usrret(pc)      ; push return address
        pea     (a0)            ; push function addr
        move.b  preempt(pc), d0
        beq.b   ojh2
        pea     1.w
        move    #32, -(sp)
        trap    #1              ; Super( inquire )
        addq    #6, sp
        tst     d0
        bne.b   ojh1
        pea     0.w
        move    #32, -(sp)
        trap    #1              ; Super(0L)
        addq    #6, sp
        move.l  d0, 12(sp)      ; save old stack
        rts
ojh2:   move.l  usp, a6
        movem.l (a6)+, #t2r
        move.l  (a6)+, a6
ojh1:   rts                     ; call function
usrret: addq    #4, sp          ; skip PARMBLK ptr
        move.l  (sp)+, d1       ; test old stack
        beq.b   usr1
        move    d0, d3          ; save return
        move.l  d1, -(sp)
        move    #32, -(sp)
        trap    #1
        addq    #6, sp
        move    d3, d0
usr1:   movem.l (sp)+, #usrreg
        rts

.export objhand
objhand:movem.l #usrreg, -(sp)
        pea     0.w             ; Super old stack
        move    d1, -(sp)
        move    d0, -(sp)
        bra.b   objh

.import x_appl_term
.import sw_dflt, shel_path, run_wait

root:   dc.b    "\", 0

.export my_pexec
.import stand_handles, sigchild, has_mint
my_pexec:
        move.l  24(a0), a1
        add.l   28(a0), a1
        move.l  sp, -(a1)
        movem.l a5/a2/d6, -(a1)
        move.l  a1, sp
        move.l  a0, a5
        pea     $100.w
        pea     (a5)
        move.l  #$004a0000, -(sp) ; Mshrink
        trap    #1
        lea     12(sp), sp
        move.l  (a5), a0
        lea     $100(a0), a0
        move.l  a0, 4(a5)       ; set hitpa
        move.l  bp_addr(pc), a0
        move.l  (a0), $24(a5)   ; set parent pointer

        tst.b   has_mint
        beq.b   _es9
        lea     sw_dflt, a0
        jsr     shel_path
        bra.b   _es7
_es9:   move.l  (a0), -(sp)     ; save old bp
        move.l  a5, (a0)
        jsr     stand_handles
_es7:   move.l  44(a5), -(sp)           ; environ
        move.l  132(a5), -(sp)          ; tail
        move.l  128(a5), -(sp)          ; path
        st      no_set
        move.b  preempt(pc), d1
        beq.b   _es1
        move.l  128+18(a5), d1
        ble.b   _es5
        moveq   #3, d0
        jsr     Psetlimit
_es5:   move.l  #$004b0064, -(sp); Pexec(100
        trap    #1
        lea     16(sp), sp
        move.l  128+14(a5), a0  ; *mint_id
        move    d0, (a0)        ; = pid
        blt.b   _es8            ; get out if bad exec
        clr     lock_curapp     ; moved here for 004    , 005 word
        move    128+22(a5), d1  ; Prenice value
        beq.b   _es4
        jsr     Prenice
        moveq   #0, d0
        moveq   #3, d0
        jsr     Psetlimit       ; reset Psetlimit
_es4:   moveq   #0, d0
_es8:   movem.l (sp)+, a5/a2/d6
        move.l  (sp), sp
        rts

_es1:   move.l  #$004b0000, -(sp)       ; Pexec
        trap    #1
        lea     16(sp), sp
        move.l  bp_addr(pc), a0
        move.l  (sp)+, (a0)

_es6:   move.l  d0, d6
        pea     root(pc)
        move    #59, -(sp)              ; Dsetpath
        trap    #1
        addq    #6, sp
        clr     d2                      ; no alert
        move    d6, d1
        move    136(a5), d0
        lea     -50(sp), a0
        move.l  a0, usp
        jsr     x_appl_term
        lea     es_fatal(pc), a0
        bsr     fatal

.export run_acc
run_acc:
        move.l  4(sp), a0       ; 004
        bsr.b   _exec
        move.b  preempt(pc), d2
        bne.b   ra1
        lea     -50(sp), a0
        move.l  a0, usp
        clr     d2              ; no alert
        jsr     x_appl_term
        lea     ra_fatal(pc), a0
        bsr     fatal
        clr     -(sp)
        trap    #1
ra1:    move.l  a0, a7          ; 004
        rts

.export start_magic
.import magic_shell, Psetlimit, Prenice, Pnice
start_magic:
        movem.l d3/a2/a4/a5, sm_save
        bsr.b   _exec
        movem.l sm_save(pc), d3/a2/a4/a5
        move.l  a0, sp
        rts

_exec:  move.l  24(a0), a1              ; BSS base
        add.l   28(a0), a1              ; + BSS len
        move.l  (sp)+, -(a1)            ; save return for rts later
        move.l  sp, -(a1)               ; save old stack
        movem.l d3/a2/a4/a5/d6, -(a1)
        move.l  a1, sp
        move.l  a0, a5
        st      no_set
        pea     $100.w
        pea     (a5)
        move.l  #$004a0000, -(sp) ; Mshrink
        trap    #1
        lea     12(sp), sp
        move.l  (a5), a0
        lea     $100(a0), a0
        move.l  a0, 4(a5)       ; set hitpa

        move.l  132(a5), a4
        move.l  8(a4), old_text
        move.l  138(a5), 8(a4)  ; text start
        move.l  bp_addr(pc), a0
        move.l  (a0), $24(a5)   ; set parent pointer
        move.b  preempt(pc), d0
        bne.b   ex5
        move.l  (a0), -(sp)     ; save old bp ptr
        move.l  a5, (a0)        ; set to child
        jsr     stand_handles
ex5:    pea     0.w
        pea     (a4)
        pea     0.w
        move.b  preempt(pc), d0
        beq.b   ex1

        move.l  128+18(a5), d1
        ble.b   ex3
        moveq   #3, d0
        jsr     Psetlimit
ex3:    move.l  128(a5), (sp)   ; pass filename
        move.l  #$004b0068, -(sp); Pexec(104
        trap    #1
        lea     16(sp), sp
        move.l  128+14(a5), a0  ; *mint_id
        move    d0, (a0)        ; = pid
        clr     lock_curapp     ; 005: moved down one, word
        move    d0, d3          ; save pid
        moveq   #0, d1
        moveq   #3, d0
        jsr     Psetlimit       ; reset Psetlimit
        move    d3, d0          ; d0 for Prenice
        blt.b   ex2
        move    128+22(a5), d1  ; Prenice value
        blt.b   ex4
        jsr     Prenice
ex4:    move    d3, d0
        bra.b   ex2             ; d0 contains return

ex1:    move.l  #$004b0004, -(sp)
        trap    #1              ; Pexec app
        lea     16(sp), sp
        move.l  bp_addr(pc), a0 ; restore bp ptr
        move.l  (sp)+, (a0)

ex2:    move    d0, d3          ; save return
        st      no_set
        move.l  44(a4), -(sp)
        move    #$49, -(sp)     ; Mfree environ
        trap    #1
        addq    #6, sp
        pea     (a4)
        move    #$49, -(sp)     ; Mfree basepage
        trap    #1
        addq    #6, sp
        sf      no_set
        pea     root(pc)
        move    #59, -(sp)      ; Dsetpath
        trap    #1
        addq    #6, sp
        move    136(a5), d0     ; return apid
        move    d3, d1          ; return value
        movem.l (sp)+, d3/a2/a4/a5/d6
        move.l  (sp)+, a0       ; old stack
        rts

.export acc_start, magic_init, pdisp_init

acc_start:
        jsr     stand_handles
        move.l  4(sp), a0
        move.l  old_text, a1
        move.l  a1, 8(a0)
        clr.l   $24(a0)
        suba.l  a7, a7
        jmp     (a1)

magic_init:
        lea     temp2, a7
        jsr     magic_shell
        clr     -(sp)
        trap    #1

.import preempt_loop, pdisp_temp
pdisp_init:
        move.l  pdisp_temp, a7
        jsr     preempt_loop
        move    d0, -(sp)
        move    #$4c, -(sp)             ; Pterm
        trap    #1

.export old_term, otrts
.export my_term
.import terminate
        .ascii  'XBRA'
        dc.l    XBRA_ID
old_term:
        ds.l    1
my_term:
        bsr     get_curapp
        tst     d0
        beq.b   mytr1
        st      in_t1
        move.l  a7, a0
        lea     temp, a7
        pea     (a0)
        moveq   #-1, d0
        jsr     terminate
        clr.l   last_parent     ; 004
        move.l  (sp), sp
        sf      in_t1
        jsr     set_oldapp
mytr1:  move.l  old_term(pc), a0
        jmp     (a0)

outmsg: pea     (a3)
        move.l  a0, a3
om1:    clr     d0
        move.b  (a3)+, d0
        beq.b   omx
        move    d0, -(sp)
        move.l  #$30002, -(sp)  ; Bconout
        trap    #13
        addq    #6, sp
        bra.b   om1
omx:    move.l  (sp)+, a3
otrts:  rts

fatal:  pea     (a3)
        move.l  a0, a3
        lea     fatal_msg(pc), a0
        bsr.b   outmsg
        move.l  a3, a0
        bsr.b   outmsg
        lea     fatal_key(pc), a0
        bsr.b   outmsg
        move.l  #$20002, -(sp)  ; Bconin
        trap    #13
        addq    #4, sp
        cmpi.b  #'b', d0
        beq.b   fat1
        cmpi.b  #'B', d0
        bne.b   fat2
fat1:   illegal
fat2:   move.l  (sp)+, a3
        rts

.import Mfree
.export set_frame
set_frame:
        move    #1, -(sp)       ; 007
        pea     -1.w            ;
        move    #68, -(sp)      ; Mxalloc
        trap    #1              ;
        addq    #8, sp          ;
        tst.l   d0              ;
        sle     fix_mxall       ;
sf2:    tst.w   $59e.w
        beq.b   sf1
        move    #2, frame_off
sf1:    rts

.export my_keyvec, my_switch, my_genevent
my_keyvec:
my_switch:
my_genevent:
        moveq   #-1, d0
        rts

.export install_key
.import Kbdvbase, norecord, ROM_ver
/*************
fix1:   dc.b    2,  0x21, 5, 0          ; read address 5,0
fix2:   dc.b    5,  0x20, 5, 5, 1, 99, 0; write byte
fix3:   dc.b    2,  0x21, 5, 0          ; inquire new 5,0
fix4:   dc.b    5,  0x20, 5, 5, 1, 99, 0; reset old
fix_old:ds.b    1
wasstat:ds.b    1

fix_rel:lea     $fffffc00.w, a0
        move.b  (a0), d0
        btst    #7, d0
        beq.b   fr1
        btst    #0, d0
        beq.b   fr1
        moveq   #0, d0
        move.b  2(a0), d0
        lea     kbpack(pc), a0
        tst.b   (a0)
        bne.b   fr2
        cmpi.b  #$f6, d0
        bcs     newshift
        seq     wasstat         ; was it a status return?
        lea     pklist-$fc(pc), a1
        move.b  0(a1,d0), (a0)
fr2:    subq.b  #1, (a0)
        bne.b   fr3             ; end of packet?
        tst.b   wasstat         ; was it status return?
        beq.b   fr1
        move.b  d0, fix4+5      ; yes, store it for later
fr1:    rts
fr3:    tst.b   fix_old         ; are we fixing now?
        beq.b   fr1
        tst.b   wasstat         ; was it status return?
        beq.b   fr1
        cmpi.b  #1, (a0)        ; is there one byte left?
        bne.b   fr1
        sf      (a0)
        sf      fix_old
        move.l  kbvec(pc), a0
        move.l  kbshift(pc), a1         ; yes, fall through
***********/
krestore:
        move.l  old_kv(pc), (a0)
        move.b  old_shift(pc), (a1)
        move.b  repeat(pc), $484.w
        rts

endpkt: move    sr, d1
ik2:    tst.b   (a1)
        bne.b   ik2
        ori     #$700, sr
        tst.b   (a1)
        bne.b   ik3
        rts
ik3:    move    d1, sr
        bra.b   ik2

/*******
do_fix: move    d0, -(sp)
        moveq   #0, d0
        move.b  (a0)+, d0
        jsr     Ikbdws
        move    (sp)+, d0
        rts
********/

install_key:                    ; 005: TOS<1.6:$4d  TT: $4f  Falc: $5b
        move.w  ROM_ver, d1     ;
        cmpi.w  #$160, d1       ;
        bge.b   ik8             ;
        moveq   #$4d, d1        ;
        bra.b   ik9             ;
ik8:    cmpi.w  #$400, d1       ;
        bge.b   ik7             ;
        moveq   #$4f, d1        ;
        bra.b   ik9             ;
ik7:    moveq   #$5b, d1        ;
ik9:    move    d1, pkt_clr     ; 005: up to here
        move    d0, -(sp)
        bsr     Kbdvbase        ; 005: bsr
        lea     32(a0), a0
        move.l  a0, kbvec       ;;
        lea     kbpack(pc), a1
        move    (sp)+, d0
        beq.b   ik1
        bsr.b   endpkt
        move.l  kbshift(pc), a1
/*********
        move.b  keyrel(pc), d0
        beq.b   ik4
        move.l  #fix_rel, (a0)
        move    d1, sr
        move.l  a0, kbvec
        lea     fix1(pc), a0    ; read old value
        bsr.b   do_fix
        lea     fix2(pc), a0    ; write new value
        move.b  d0, 5(a0)
        bsr.b   do_fix
        st      fix_old
        lea     fix3(pc), a0    ; inquire bytes
        bsr.b   do_fix
ik5:    move.b  fix_old(pc), d0
        bne.b   ik5
        lea     fix4(pc), a0    ; reset old value
        bra.b   do_fix
**********/
ik4:    bsr.b   krestore
        move    d1, sr
        rts
ik1:    lea     4(a0), a1
        bsr.b   endpkt
        move.b  $484.w, repeat
        bclr    #1, $484.w
        move.l  kbshift(pc), a1
        move.b  (a1), old_shift
;;      sf      keyrel
        move.l  (a0), old_kv
        move.l  #rcv_key, (a0)
        move    d1, sr
        rts

rcv_key:
        lea     $fffffc00.w, a0
        move.b  (a0), d0
        btst    #7, d0
        beq.b   rv1
        btst    #0, d0
        beq.b   rv1
        moveq   #0, d0
        move.b  2(a0), d0
        lea     kbpack(pc), a0
        tst.b   (a0)
        bne.b   rv2
        cmpi.b  #$f6, d0
        bcs.b   newshift
        lea     pklist-$f6(pc), a1
        lea     kbpack(pc), a0          ; 005
        move.b  0(a1,d0), (a0)
rv2:    subq.b  #1, (a0)
rv1:    rts

rks0:   bset    #0, d1
        bra.b   rkx
rkc0:   bclr    #0, d1
        bra.b   rkx
rks1:   bset    #1, d1
        bra.b   rkx
rkc1:   bclr    #1, d1
        bra.b   rkx
rks2:   bset    #2, d1
        bra.b   rkx
rkc2:   bclr    #2, d1
        bra.b   rkx
rks3:   bset    #3, d1
        bra.b   rkx
rkc3:   bclr    #3, d1
        bra.b   rkx

newshift:
        lea.l   old_shift(pc), a0
        move.b  (a0), d1
        cmpi.b  #$36, d0
        beq.b   rks0
        cmpi.b  #$b6, d0
        beq.b   rkc0
        cmpi.b  #$2a, d0
        beq.b   rks1
        cmpi.b  #$aa, d0
        beq.b   rkc1
        cmpi.b  #$1d, d0
        beq.b   rks2
        cmpi.b  #$9d, d0
        beq.b   rkc2
        cmpi.b  #$38, d0
        beq.b   rks3
        cmpi.b  #$b8, d0
        beq.b   rkc3
        cmpi.b  #$3a, d0
        beq.b   rkt4
        btst    #7, d0
        beq.b   rk1
        move.l  kbvec(pc), a0
        adda.w  pkt_clr(pc), a0         ; 005: was $4d  TT: $4f  Falc: $5b
        clr.b   (a0)+
        clr.w   (a0)                    ; 005: was (a0)+
;;      lea     keyrel(pc), a0
;;      tst.b   (a0)
;;      bne.b   rk1
;;      move.b  d0, (a0)                ; key release
rk1:    rts
rkt4:   bchg    #4, d1
rkx:    move.b  d1, (a0)
        andi.b  #$f, d1
        subq.b  #3, d1
        bne.b   rk1
        st      norecord
        rts
old_shift:
        ds.b    1
kbpack: ds.b    1
keyrel: ds.b    1
repeat: ds.b    1
pklist: dc.b    8, 6, 3, 3, 3, 3, 7, 3, 2, 2
old_kv: ds.l    1
pkt_clr:ds.w    1

.export grab_curapp
.import wait_curapp
; 005: reworked quite a bit
grab_curapp:
        move    d3, -(sp)
        jsr     Pgetpid
        move    d0, d3
        ori     #$8000, d3
        cmp     lock_curapp, d3
        beq.b   lcu2
lcu1:   tas     lock_curapp
        beq.b   lcu3
        addq.b  #1, wait_curapp
        jsr     Syield
        subq.b  #1, wait_curapp
        bra.b   lcu1
lcu3:   move    d3, lock_curapp
lcu2:   move    (sp)+, d3
        rts

;;lcu4: jsr     breakpoint
;;      bra.b   lcu2

/************.export redo_func
redo_func:
        movem.l d3-d4/a2-a5, -(sp)
        move    d0, d3          ; type
        move.l  a0, a2          ; parms
        move.l  a1, a3          ; func
        move.l  sp, a5          ; save sp
        move    d3, d4
        asr     #8, d4
        andi    #$f, d4         ; pointers
        subq    #1, d4
        blt.b   rf1
        move.l  (a2)+, a0
        subq    #1, d4
        blt.b   rf1
        movea.l (a2)+, a1
        beq.b   rf1             ; leftover from cmp
        move    d4, d0
        add     d0, d0
        add     d0, d0
        sub     d0, sp
        move.l  sp, a4
        subq    #1, d4
rf2:    move.l  (a2)+, (a4)+
        dbra    d4, rf2
rf1:    move    d3, d4
        asr     #4, d4
        andi    #$f, d4
        subq    #1, d4
        blt.b   rf3
        move.l  (a2)+, d0
        subq    #1, d4
        blt.b   rf3
        move.l  (a2)+, d1
        subq    #1, d4
        blt.b   rf3
        move.l  (a2)+, d2
rf3:    move    d3, d4
        andi    #$f, d4
        subq    #1, d4
        blt.b   rf4
        move    (a2)+, d0
        subq    #1, d4
        blt.b   rf4
        move    (a2)+, d1
        subq    #1, d4
        blt.b   rf4
        move    (a2)+, d2
rf4:    jsr     (a3)
        move.l  a5, sp
        movem.l (sp)+, d3-d4/a2-a5
        rts*********/

/*.export Cconws
Cconws: pea     (a2)
        pea     (a0)
        move    #9, -(sp)
        trap    #1
        move    #1, (sp)        ; Cconin
        trap    #1
        addq    #6, sp
        move.l  (sp)+, a2
        rts */

.export Kbdvbase                ; 005: avoid crash in install_key(1)
Kbdvbase:
        pea     (a2)
        move.l  xkbdvb(pc), d0
        bne.b   kb1
        move    #$22, -(sp)
        trap    #14
        addq    #2, sp
        move.l  d0, xkbdvb
kb1:    move.l  d0, a0
        move.l  (sp)+, a2
        rts
xkbdvb: ds.l    1

.ifne TEST
.import procman
.ascii  'PROCMAN '
        dc.l    procman
        dc.l    curapp
tst_d1: ds.l    1
.endif

.export kbio, kbshift, kbbuf, kbsize, bp_addr
.export sleeping, preempt, is_PD
.export in_t2, block_app

old_stack:
        ds.l    1
kbio:   ds.l    1
kbshift:ds.l    1
kbsize: ds.w    1
curstack:
        ds.l    1
bp_addr:ds.l    1
kbbuf:  ds.l    1
kbvec:  ds.l    1
sm_save:ds.l    4
vtr_ptr:ds.l    1
t2_term:ds.l    1
pthptr: ds.l    1
pthret: ds.l    1
la_font:ds.l    1
la_base:ds.l    1
emult_out:
        ds.w    7
frame_off:
        ds.w    1
vtr_mode:
        ds.w    1
empty:  ds.w    1
preempt:ds.b    1
sleeping:
        ds.b    1
is_PD:  ds.b    1
in_vdi: ds.b    1

.IFNE GERMAN
fatal_msg:
        dc.b    $7, $1b, "H", "Schwerer Fehler in Geneva: ", 0
fatal_key:
        dc.b    $d, $a, "Abbruchversuch mit beliebiger Taste...", 0
t1_fatal:
        dc.b    "GEMDOS wurde rekursiv aufgerufen", 0
t2_fatal:
        dc.b    "AES wurde rekursiv aufgerufen", 0
bp_fatal:
        dc.b    "Breakpoint erreicht. Dr?cke B f?r Unterbrechung.", 0
es_fatal:
        dc.b    "exec_shell terminiert nicht", 0
ra_fatal:
        dc.b    "run_acc terminierte nicht", 0
.ELSE
fatal_msg:
        dc.b    $7, $1b, "HFatal error in Geneva: ", 0
fatal_key:
        dc.b    $d, $a, "Press any key to try to quit...", 0
t1_fatal:
        dc.b    "GEMDOS called recursively", 0
t2_fatal:
        dc.b    "AES called recursively", 0
bp_fatal:
        dc.b    "breakpoint reached. Press B to break.", 0
es_fatal:
        dc.b    "exec_shell did not terminate", 0
ra_fatal:
        dc.b    "run_acc did not terminate", 0
.ENDIF

.bss
old_text:
        ds.l    1
tmpst:  ds.b    2000    ; must be large enough for critical alert
temp:
        ds.b    1000
temp2:
;;my_bp:  ds.l    1
in_t1:  ds.w    1
;;t1_bp:        ds.l    1
in_t2:  ds.b    1
block_app:
        ds.b    1
havet2st:
        ds.b    1
fix_mxall:
        ds.b    1