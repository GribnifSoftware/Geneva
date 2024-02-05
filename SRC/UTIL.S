.export u_ptext, u_tedinfo, u_object, u_obspec, sel_if, is_sel, is_disab, is_hid, Kbdvbase

.module u_ptext
	ext.l	d0
	lsl.l	#3, d0
	move.l	d0, d1
	add.l	d0, d0
	add.l	d1, d0
	move.l	12(a0,d0.l), a0
	move.l	(a0), a0
	rts

.module u_tedinfo
	ext.l	d0
	lsl.l	#3, d0
	move.l	d0, d1
	add.l	d0, d0
	add.l	d1, d0
	move.l	12(a0,d0.l), a1
	btst	#0, 8(a0,d0.l)		; INDIRECT ob_flag
	bne.b	ut1
	move.l	a1, a0
	rts
ut1:	move.l	(a1), a0
	rts

.module u_obspec
	ext.l	d0
	lsl.l	#3, d0
	move.l	d0, d1
	add.l	d0, d0
	add.l	d1, d0
	lea	12(a0,d0.l), a0
	rts

.module u_object
	ext.l	d0
	lsl.l	#3, d0
	adda.l	d0, a0
	add.l	d0, d0
	adda.l	d0, a0
	rts

.module sel_if
	bsr	u_object
	move	d1, d0
	bne.b	sel1
	bclr	#0, 11(a0)
	rts
sel1:	bset	#0, 11(a0)
	rts

.module is_sel
	moveq	#0, d1
	bra	statetest

.module is_disab
	moveq	#3, d1
statetest:
	bsr	u_object
	moveq	#0, d0
	btst	d1, 11(a0)
	sne	d0
	rts

.module is_hid
	bsr	u_object
	moveq	#0, d0
	btst	#7, 9(a0)
	sne	d0
	rts
