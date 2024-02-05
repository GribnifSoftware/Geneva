.export t13
.export t13adr
.export t1
.export t1adr
.export nstddh, stdh
.import doecho
.import out_one
.import gtext2
.import bcon_nul
.import bconinr
.import hasch
.import iredir_hand
.import strlen
.import getstr
.import ctrlc
.import ign_ctrlc
.import col
.import force_cono
.import gt_ign_cr
.import cmdbuf
.import zero_cmd

.export la_init, linea0
la_init:        .ds.l   4

regs reg d0/a0-a2
linea0:         pea	(a2)
                .dc.w   $a000
                movem.l #regs, la_init
                move.l  (sp)+, a2
                rts

_longframe      equ     $59e
HNDLS   equ     5

ssi:    illegal
setstack:
        tas     in_io
        bne.b   ssi
        move.l  d2, -(sp)
        move.l  usp, a1
        move.l  a1, old_usp
        lea     stackend(pc), a1
        move.l  a1, usp
        move.l  8(sp), a1
        move.l  a7, old_a7
        lea     new_a7(pc), a7
        jsr     (a1)
        movea.l old_a7(pc), a7
        movea.l old_usp(pc), a1
        move.l  a1, usp
        move.l  (sp)+, d2
        clr.b   in_io
        rts

_bconst:lea     hasch(pc), a1
        bsr.b   bcst
        tst.l   d0
        beq.b   bcs1
        moveq   #-1, d0
bcs1:   rts
_bconin:lea     bconinr(pc), a1
bcst:   pea	(a1)
        bsr.b   setstack
        addq    #4, sp
        moveq  #-1, d1	;
        cmp.l  d1, d0	;
        beq.b  bcctrlc	;
        rts
bcctrlc:move.l  #$002e0003, d0	;
        rts			;

/*
.import	flush_out
bconst:
bconin:
        tst.b   in_io
        bne.b   t133
        bsr	test_app
        bne.b	t133
        bsr	copy_on
	lea	flush_out(pc), a1
	bsr.b	bcst
        bsr	copy_off
	bra.b	t133
*/

.import	force_coni, in_t2
bconst: lea     _bconst(pc), a2
        bra.b   bco
bconin: lea     _bconin(pc), a2
bco:    cmpi    #2, (a0)
        bne.b   t133
        move.b  in_io(pc), d0
        move.l	in_t2(pc), a1		; 004
        or.b	(a1), d0		; 004
        bne.b   t133
        bsr	test_app
        bne.b	t133
;        cmpi	#-1, child_id	004
;        beq.b	t133		004
        bsr	copy_on
        st	ign_ctrlc
        moveq   #0, d0
        bsr   force_coni
        jsr     (a2)
        move.l  d0, -(sp)
        moveq   #1, d0
        bsr     force_coni
        move.l  (sp)+, d0
        sf	ign_ctrlc
        bsr	copy_off
        rte

        dc.b    'XBRA'
        dc.b    'GnTS'
t13adr: ds.l    1
t13:    move.l  usp, a0
        btst    #5, (sp)
        beq.b   t131
        lea     6(sp), a0
        adda	framesize(pc), a0
t131:   move    (a0)+, d0
        subq    #1, d0
        beq.b   bconst
        subq    #1, d0          ; Bconin
        beq.b   bconin
        subq    #1, d0          ; Bconout
        beq.b   t132
;        cmpi	#3, d0
;        beq.b	t132
t133:   movea.l t13adr(pc), a0
        jmp     (a0)
t132:   cmpi.w  #$2, (a0)
        beq.b   t134
        cmpi.w  #$5, (a0)
        bne.b   t133
        tst.b   in_io
        bne.b   t133
        bsr	test_app
        bne.b	t133
        st      ign_ctrlc
        lea     3(a0), a1
        lea     temp(pc), a0
        move.b  (a1), (a0)
        beq.b   t139
        bsr.b	copy_on
;        move.l	a7, a1
;        lea	tempa7, a7
;        pea	(a1)
        pea     (a0)
        moveq   #0, d0
        bsr     force_cono
        move.l  (sp)+, a0
;        st      gt_ign_cr		004: moved into gtext2
        pea     gtext2(pc)
        bsr     setstack
        addq    #4, sp
        moveq   #1, d0
        bsr     force_cono
t138:   moveq   #0, d0
        sf      ign_ctrlc
        bsr.b	copy_off
;        move.l	(sp), sp
        rte
t139:   addq    #1, col
        bra.b   t138
t134:   tst.b   in_io
        bne.b   t133
        bsr	test_app
        bne	t133
        st      ign_ctrlc
        bsr.b	copy_on
;        move.l	a7, a1
;        lea	tempa7, a7
;        pea	(a1)
        lea     3(a0), a1
        moveq   #0, d0
        bsr     force_cono
        move.b	(a1), d0
        pea	out_one(pc)
        bsr	setstack
        addq	#4, sp
        moveq   #1, d0
        bsr     force_cono
        sf      ign_ctrlc
;        move.l	(sp), sp
	bsr.b	copy_off
        rte

.import AES_pb

copy_on:
	st.b	in_a7
	move.l	a1, cp1
	move.l	a7, cp2
	lea	24(sp), a1
	lea	tempa7(pc), sp
	move.l	-(a1), -(sp)
	move.l	-(a1), -(sp)
	move.l	-(a1), -(sp)
	move.l	-(a1), -(sp)
	move.l	-(a1), -(sp)
	move.l	-(a1), -(sp)
	move.l	cp1(pc), a1
	rts

copy_off:
	movem.l	a0-a1, cp1
	move.l	a7, a1
	move.l	cp2(pc), sp
	move.l	sp, a0
	move.l	(a1)+, (a0)+
	move.l	(a1)+, (a0)+
	move.l	(a1)+, (a0)+
	move.l	(a1)+, (a0)+
	move.l	(a1)+, (a0)+
	move.l	(a1), (a0)
	movem.l	cp1(pc), a0-a1
	sf.b	in_a7
	rts

cp1:	ds.l	2
cp2:	ds.l	1

testh:	move    (a0)+, d0		; 006: changed lots
	cmpi	#6, d0
	blt.b	th3
        move    nstddh(pc), d1
        beq.b   th3
        subq    #1, d1
        lea     dhndls(pc), a1
th2:    cmp     2(a1), d0
	addq	#4, a1
        dbeq    d1, th2
        bne.b   th3
	move	-4(a1), d0
th3:    cmpi    #-1, d0
        beq.b   th4			; we care about this handle
        cmpi	#3, d0
        bhi.b	th1			; >=4 or <-1
th4:	add	d0, d0
	lea	stdh(pc), a1
	move	2(a1,d0), d0		; get redirected handle
	cmpi	#-1, d0
	beq.b	thx
	cmpi	#1, d0
	bhi.b	th1
thx:	moveq	#0, d1			; we care about this handle, set Z
        rts
th1:	moveq   #1, d1
	rts

frreg reg d2/a6
fread:  bsr.b   testh
        bne     txx
fr1:    move.l  (a0)+, d0       ; length
        beq     fw0
        clr.b   in_t1
        movem.l #frreg, -(sp)
        move.l	#$FFFF, d2
        cmp.l	d2, d0
        bls.b	fr2
        move.l	d2, d0
fr2:    move.l  (a0)+, a6
        move.l  a6, a0
        clr.b   (a0)
        pea     getstr(pc)      ; needs d0, a0
        bsr     setstack
        addq    #4, sp
        subq    #1, d0
        moveq   #-1, d1
        bsr.b	tctrlc		; 006
        lea     cmdbuf(pc), a1
fr3:    addq    #1, d1
        move.b  (a1)+, (a6)+
        dbeq    d0, fr3
        bsr     zero_cmd
        movem.l (sp)+, #frreg
        move	d1, d0		; 006
        ext.l	d0		; 006
        bra     fwr

test_ctrlc:
        moveq   #3, d1
        cmp     d1, d0
        beq.b   pterm32
tctrlc: tst.b   ctrlc
        bne.b   pterm32
        rts
pterm32:clr.b   ctrlc
        move.l  #$004CFFE0, -(sp)       ; Pterm(-32)
        trap    #1

fwrite: bsr	testh			; 006
	bne 	txx			;
fw1:    move.l  (a0), bcon_nul  	; length
        beq.b   fw0
        move.l  (a0)+, -(sp)
        clr.b   in_t1
        cmpi.l	#1, (sp)		; 006: optimize
        beq.b	fw2			;
        bsr     cconws          	; uses (a0) for pointer
fw3:    move.l  (sp)+, d0
        bra.b   fwr
fw0:    moveq   #0, d0
fwr:    movem.l (sp)+, #t1reg
        clr.b   in_t1
        bsr	copy_off
        rte
fw2:	move.l	(a0), a1
	bsr	outch
	bra.b	fw3

fseek:  addq    #4, a0
        bsr     testh
        bne.b	txx
fsk1:   moveq   #0, d0
        bra.b   fwr
t10:    illegal

t1xtbl:	dc.w	fclose-txx, fread-txx, fwrite-txx, 0, fseek-txx, 0, 0
	dc.w    fdup-txx, fforce-txx, 0, 0, 0, 0, pexec-txx

temp1:	ds.l	1
t1reg reg a1/d1-d2      ; critical to fdup, fforce
        dc.b    'XBRA'
        dc.b    'GnTS'
t1adr:  ds.l    1
t1:     bsr	test_app
        bne.b	tx0
        tas     in_t1
        bne.b   t10
        move.l  usp, a0
        btst    #5, (sp)
        beq.b   t11
        lea     6(sp), a0
        adda	framesize(pc), a0
t11:    move    (a0)+, d0
	bsr	copy_on
        movem.l #t1reg, -(sp)
        cmpi    #$10, d0
        bls.b   ccon
        subi	#$3e, d0		; 006
        blt.b	txx			;
        cmpi	#$4b-$3e, d0		; 006
        bhi.b	txx			;
        add	d0, d0			;
        move	t1xtbl(pc,d0), d0	;
        jmp	txx(pc,d0)		;
;        subi    #$3e, d0
;        beq.b   fclose		; $3e
;        subq    #1, d0
;        beq.b   fread		; $3f
;        subq    #1, d0
;        beq.b   fwrite		; $40
;        subq    #2, d0
;        beq.b   fseek		; $42
;        subq    #3, d0
;        beq.b   fdup		; $45
;        subq    #1, d0
;        beq.b   fforce		; $46
;        subq	#5, d0
;        beq.b	_pexec
txx:    movem.l (sp)+, #t1reg
        clr.b   in_t1
        bsr	copy_off
tx0:    movea.l t1adr(pc), a0
        jmp     (a0)
txz:    addq    #4, sp		; bad ccon function #
        bra.b   txx

fdup:   lea     _fdup(pc), a1
        bra.b   fd1

fforce: lea     _fforce(pc), a1
	move	2(a0), fnstd
fd1:    move    (a0), d0
        cmpi	#-1, d0		; 006
        blt.b	txx		;
	cmpi	#3, d0		;
        bhi.b   txx		;
        move    d0, fstd
        move.l  14(sp), fsave
        move.l  a1, 14(sp)
        bra.b   txx

ccon:   bra     ccon2

fclose: move	(a0), d1		; 006: changed lots
	lea	stdh+5*2(pc), a0
	moveq	#4, d0
stdc1:	cmp	-(a0), d1
	dbeq	d0, stdc1
	bne.b	stdcx
	move	d0, (a0)
stdcx:	move    nstddh(pc), d0		; close duped handles
        beq.b   txx
        lea	dhndls(pc), a0
        subq	#1, d0
t1_30:  cmp     2(a0), d1
	beq.b	t1_31
	addq	#4, a0
        dbra    d0, t1_30
        bra.b	txx
t1_31:  move.l  (a0)+, -4(a0)
        dbra    d0, t1_31
        bra 	txx

_fforce:tst.l	d0		; 006: changed a lot
	bne.b	fdx
        move	d1, -(sp)
	move	fstd(pc), d1
	add	d1, d1
	lea	stdh(pc), a0
	move	fnstd(pc), 2(a0,d1)
	bra.b	fdxx

_fdup:  tst.l   d0		; 006: changed a lot
        blt.b   fdx
        move	d1, -(sp)
        move    nstddh(pc), d1
        cmpi    #HNDLS, d1
        beq.b   fdxx
        add	d1, d1
        add	d1, d1
        lea	dhndls(pc), a0
        move	fstd(pc), 0(a0,d1)
        move	d0, 2(a0,d1)
        add	#1, nstddh
fdxx:   move	(sp)+, d1
fdx:    move.l  fsave(pc), a0
        jmp     (a0)
        
.import	do_pexec
pexec:	move	(a0)+, d0
	bne	txx
	move	child_id(pc), d0	; single-tasking, or not yet known
	blt	txx
	pea	do_pexec(pc)
	bsr	setstack
	addq	#4, sp
	bra	fwr

ccon2:  add     d0, d0
        clr.b   in_t1
        move    t1tbl(pc,d0), d0
        jsr     t1tbl(pc,d0)
        movem.l (sp)+, #t1reg
        bsr	copy_off
        rte

t1tbl:  dc.w    txz-t1tbl, cconin-t1tbl, cconout-t1tbl, txz-t1tbl
        dc.w    txz-t1tbl, txz-t1tbl, crawio-t1tbl, crawcin-t1tbl
        dc.w    cnecin-t1tbl, cconws-t1tbl, cconrs-t1tbl, _bconst-t1tbl
        dc.w    txz-t1tbl, txz-t1tbl, txz-t1tbl, txz-t1tbl, cconos-t1tbl

cconws: movea.l (a0), a1
        move.l  a1, a0
        pea     doecho(pc)
        bsr     setstack
        addq    #4, sp
        moveq   #0, d0
        bra     test_ctrlc

crawio: cmpi    #$FF, (a0)
        beq.b   crawio_
        lea     1(a0), a1
        st      ign_ctrlc
        bsr.b   outch
        sf      ign_ctrlc
        rts
crawio_:st      ign_ctrlc
        bsr     _bconst
        sf      ign_ctrlc
        bne.b   crawcin
        rts

cconout:lea     1(a0), a1
        bsr.b   outch
        bra     test_ctrlc

cconin: bsr     _bconin
        bsr     test_ctrlc
        tst.l   d0
        bge.b   cconech
cco3:   rts

crawcin:st      ign_ctrlc
        bsr     _bconin
        sf      ign_ctrlc
        rts

outch:
        move.b	(a1), d0
        pea	out_one(pc)
        bsr	setstack
        addq	#4, sp
        moveq	#0, d0
        rts

cconech:
        move    iredir_hand(pc), d1
        cmpi    #2, d1
        bne.b   cco3
        move.l  d0, -(sp)
        lea     3(sp), a1
        bsr.b   outch
        move.l  (sp)+, d0
        bra     test_ctrlc

cconrs: move.l  (a0), a0
        pea	(a0)
        clr     d0
        move.b  (a0), d0
        beq.b   ccrs2
        move    d0, -(sp)
        addq    #2, a0
        clr.b   (a0)
        pea	(a0)
        pea     getstr(pc)
        bsr     setstack
        addq    #4, sp
        move.l  (sp)+, a0
        move    (sp)+, d0
        subq    #1, d0
        moveq   #-1, d1
        lea     cmdbuf(pc), a1
ccrs1:  addq    #1, d1
        move.b  (a1)+, (a0)+
        dbeq    d0, ccrs1
        move.l  (sp)+, a0
        move.b  d1, 1(a0)
        bsr     zero_cmd
        bra     test_ctrlc
ccrs2:  clr.b   1(a0)
        bra     test_ctrlc

cnecin: bsr     _bconin
        bra     test_ctrlc

cconos: moveq   #-1, d0
        rts

.import child_id, cookie
test_app:
	tst.b	in_a7
	bne.b	ta3			; 004: was ta1
	pea	(a0)
	move	child_id(pc), d0
	cmpi	#-1, d0
	beq.b	ta0
	move.l	cookie(pc), a0
	cmp	6(a0), d0
;;	beq.b	ta0			; 004: this stuff not needed
ta0:	movea.l	(sp)+, a0
;;ta1:	moveq	#0, d0			; also reversed return in Z
ta3:	rts
;;ta0:	move.l	(sp)+, a0
;;	moveq	#1, d0
;;	rts

/*
.import single, AES_pb
.export appl_yield
appl_yield:;tst.b     single
	   ;bne.b     ay1
;;	move.l	a7, a0
;;	move.l  old_a7(pc), sp
;;	pea	(a0)
	lea	aypb(pc), a0
	move.l	a0, d1
	move.l	AES_pb+4(pc), 4(a0)
        PEA.L   (A2)
        MOVE.W  #$00C8,D0
        TRAP    #2
        MOVEA.L (A7)+,A2
;;        move.l	(sp), sp
ay1:    RTS

actrl:	dc.w	$11, $0, $1, $0, $0
adum:	ds.l	1
aypb:	dc.l	actrl, 0, adum, adum, adum, adum
*/

stdh:	dc.w	-1, 0, 1, 2, 3	; 006

.bss
	ds.l	100
tempa7:
        ds.l    100
stackend:
        ds.l    500
new_a7:

framesize:
	ds.w	1
nstddh: ds.w    1		; 006
dhndls: ds.l    HNDLS		; 006  standard|non-standard
fsave:  ds.l    1
fnstd:	ds.w	1		; 006: most be in this order
fstd:	ds.w	1		;
fmode:  ds.w    1
intout2:ds.w	20
old_usp:ds.l    1
old_a7: ds.l    1
in_t1:  ds.b    1
in_io:  ds.b    1
in_a7:	ds.b	1
.export in_a7

.text
temp:   dc.b    ' ', 0

.export _aes, _aesP, _xaes
.import AES_pb, Gem_pb, set_linea, reset_linea
.import is_io
_aes:      LEA.L     Gem_pb(pc),A1
_aesP:
           PEA.L     (A2)
           clr.l     (a1)+
           clr.l     (a1)+
           clr.w     (a1)
           MOVEP.L   D1,-$0007(A1)
_aesx:     pea	     (a3)
           lea       AES_pb(pc),a3
           tst.b     is_io
           beq.b     gi0
           clr.b     is_io
           lea       AESpb2(pc),a3
           bsr	     reset_linea
gi0:       MOVE.W    #$00C8,D0
	   move.l    a3, d1
           TRAP      #2
           cmp.l     #AESpb2, a3
           bne.b     gi1
           bsr	     set_linea
gi1:       move.l    12(a3), a0
           move.l    (sp)+,a3
           MOVE.W    (A0)+,D0
           MOVEA.L   (A7)+,A2
           RTS

_xaes:     LEA.L     Gem_pb+2(pc),A1
           PEA.L     (A2)
           clr.l     (A1)+
           clr.l     (A1)
           move.b    #$71, -6(a1)
           MOVEP.L   D1,-5(A1)
           bra.b     _aesx

AESpb2:
           .DC.L   Gem_pb
           .DC.L   Gem_pb+$0000001E
           .DC.L   Gem_pb+$0000003C
           .DC.L   intout2
           .DC.L   Gem_pb+$0000025C
           .DC.L   Gem_pb+$0000029C

.export setexc
setexc:
        movem.l a2-a5/d3, -(sp)
        move.w  d0, a2
        move.w  d0, a5
        move.w  d1, d3
        move.l  a0, a3
        move.l  a1, a4
        pea     0.w
        move    #$20, -(sp)
        trap    #1
        tst.w	$59e.w
        beq.b	se2
        move	#2, framesize
se2:    move.l  d0, 2(sp)
        move.l  a4, d0
        bne.b   se1
ne1:    move.l  (a2), a0
        move.l  a0, d0
        beq.b   force
        cmpi.l  #'XBRA', -12(a0)
        bne.b   force
        cmpi.l  #'GnTS', -8(a0)
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
sx:     trap    #1
        addq    #6, sp
        move    d3, d0
        movem.l (sp)+, a2-a5/d3
        rts

.export breakpoint
breakpoint:illegal
	   rts
