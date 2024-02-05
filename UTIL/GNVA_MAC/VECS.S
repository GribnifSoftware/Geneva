.import apid, keypress, test_key, auto_load, auto_test

.export my_keyvec, my_appvec, my_genvec, do_auto
.export old_keyvec, old_appvec, old_genvec

	dc.l	'XBRA'
	dc.l	'GMac'
old_keyvec:
	ds.l	1
my_keyvec:
	pea	(a0)
	bsr	test_key
	move.l	(sp)+, a0
	subq	#1, d0
	bge.b	mkv1
	move.l	old_keyvec(pc), -(sp)	; chain to previous
	rts
mkv1:	subq	#1, d0
	bge.b	mkv2
	moveq	#-1, d0		; no event, no chain
	rts
mkv2:	move	apid(pc), d0	; return my apid
	rts			; don't chain


	dc.l	'XBRA'
	dc.l	'GMac'
old_appvec:
	ds.l	1
my_appvec:
	move	d0, -(sp)
	pea	(a0)
	bsr	auto_test
	move.l	(sp)+, a0
	move	(sp)+, d0
	move.l	old_appvec(pc), -(sp)	; chain to previous
	rts


	dc.l	'XBRA'
	dc.l	'GMac'
old_genvec:
	ds.l	1
my_genvec:
	move.b	do_auto(pc), d0
	bne.b	mg1
mg2:	move.l	old_genvec(pc), -(sp)	; chain to previous
mg3:	rts
mg1:	bsr	auto_load
	tst	d0
	blt.b	mg2
	rts

do_auto:ds.b	2

/********************** Geneva 004 patch *********************/
.export install_key, ROM_ver, norecord, kbshift
.import Kbdvbase

krestore:
	move.l	old_kv(pc), (a0)
	move.b	old_shift(pc), (a1)
	move.b	repeat(pc), $484.w
	rts

endpkt:	move	sr, d1
ik2:	tst.b	(a1)
	bne.b	ik2
	ori	#$700, sr
	tst.b	(a1)
	bne.b	ik3
	rts
ik3:	move	d1, sr
	bra.b	ik2

install_key:			; 005: TOS<1.6:$4d  TT: $4f  Falc: $5b
	move.w	ROM_ver(pc), d1	;
	cmpi.w	#$160, d1	;
	bge.b	ik8		;
	moveq	#$4d, d1	;
	bra.b	ik9		;
ik8:	cmpi.w	#$400, d1	;
	bge.b	ik7		;
	moveq	#$4f, d1	;
	bra.b	ik9		;
ik7:	moveq	#$5b, d1	;
ik9:	move	d1, pkt_clr	; 005: up to here
	move	d0, -(sp)
	jsr	Kbdvbase
	lea	32(a0), a0
	move.l	a0, kbvec	;;
	lea	kbpack(pc), a1
	move	(sp)+, d0
	beq.b	ik1
	bsr.b	endpkt
	move.l	kbshift(pc), a1
ik4:	bsr.b	krestore
	move	d1, sr
	rts
ik1:	lea	4(a0), a1
	bsr.b	endpkt
	move.b  $484.w, repeat
	bclr	#1, $484.w
	move.l	kbshift(pc), a1
	move.b	(a1), old_shift
	move.l	(a0), old_kv
	move.l	#rcv_key, (a0)
	move	d1, sr
	rts

rcv_key:
	lea	$fffffc00.w, a0
	move.b	(a0), d0
	btst	#7, d0
	beq.b	rv1
	btst	#0, d0
	beq.b	rv1
	moveq	#0, d0
	move.b	2(a0), d0
	lea	kbpack(pc), a0
	tst.b	(a0)
	bne.b	rv2
	cmpi.b	#$f6, d0
	bcs.b	newshift
	lea	pklist-$f6(pc), a1
	lea	kbpack(pc), a0		; 005
	move.b	0(a1,d0), (a0)
rv2:	subq.b	#1, (a0)
rv1:	rts

rks0:	bset	#0, d1
	bra.b	rkx
rkc0:	bclr	#0, d1
	bra.b	rkx
rks1:	bset	#1, d1
	bra.b	rkx
rkc1:	bclr	#1, d1
	bra.b	rkx
rks2:	bset	#2, d1
	bra.b	rkx
rkc2:	bclr	#2, d1
	bra.b	rkx
rks3:	bset	#3, d1
	bra.b	rkx
rkc3:	bclr	#3, d1
	bra.b	rkx

newshift:
	lea.l	old_shift(pc), a0
	move.b	(a0), d1
	cmpi.b	#$36, d0
	beq.b	rks0
	cmpi.b	#$b6, d0
	beq.b	rkc0
	cmpi.b	#$2a, d0
	beq.b	rks1
	cmpi.b	#$aa, d0
	beq.b	rkc1
	cmpi.b	#$1d, d0
	beq.b	rks2
	cmpi.b	#$9d, d0
	beq.b	rkc2
	cmpi.b	#$38, d0
	beq.b	rks3
	cmpi.b	#$b8, d0
	beq.b	rkc3
	cmpi.b	#$3a, d0
	beq.b	rkt4
	btst	#7, d0
	beq.b	rk1
	move.l	kbvec(pc), a0
	adda.w	pkt_clr(pc), a0		; 005: was $4d  TT: $4f  Falc: $5b
	clr.b	(a0)+
	clr.w	(a0)			; 005: was (a0)+
rk1:	rts
rkt4:	bchg	#4, d1
rkx:	move.b	d1, (a0)
	andi.b	#$f, d1
	subq.b	#3, d1
	bne.b	rk1
	move.l	norecord(pc), a0	; rts to st norecord
	st	(a0)
	rts

pklist: dc.b	8, 6, 3, 3, 3, 3, 7, 3, 2, 2

.bss
old_shift:
	ds.b	1
kbpack: ds.b	1
keyrel:	ds.b	1
repeat:	ds.b	1
old_kv:	ds.l	1
norecord:
	ds.l	1
pkt_clr:ds.w	1
ROM_ver:ds.w	1
kbshift:ds.l	1
kbvec:	ds.l	1
