.import apid, new_apid, keypress, new_app, shift

.export my_keyvec, my_appvec, my_genvec
.export old_keyvec, old_appvec, old_genvec

	dc.l	'XBRA'
	dc.l	'Vtst'
old_keyvec:
	ds.l	1
my_keyvec:
	move.l	(a0), d0	; get the keypress
	tst.b	d0		; is there an ASCII char?
	bne.b	mkv1		; yes, generate an event
	move.l	old_keyvec(pc), -(sp)	; no, chain to previous
	rts
mkv1:	move.l	d0, keypress	; save the key
	move	apid(pc), d0	; return my apid
	clr.l	(a0)		; and clear out the keypress
	rts			; don't chain


	dc.l	'XBRA'
	dc.l	'Vtst'
old_appvec:
	ds.l	1
my_appvec:
	move.l	a0, new_app	; name of process
	move	d0, new_apid	; id of process
	move	apid(pc), d0	; return my apid
	rts			; don't chain (not a good idea in general)


	dc.l	'XBRA'
	dc.l	'Vtst'
old_genvec:
	ds.l	1
my_genvec:
	tst.b	shift		; app must clear "shift" before
	bne.b	mgv2		; we can do another event
	move.l	#$000BFFFF, -(sp)
	trap	#13		; Kbshift(-1)
	addq	#4, sp
	andi	#1, d0		; is the right Shift key held?
	bne.b	mgv1		; yes, generate an event
mgv2:	move.l	old_genvec(pc), -(sp)	; no, chain to previous
	rts
mgv1:	move	apid(pc), d0	; return my apid
	st	shift		; set the semaphore
	rts			; don't chain

