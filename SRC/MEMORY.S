.import _BasPag
.import bp_addr

.import fake_bp

/***
.export Malloc
Malloc:
	move.l	bp_addr(pc), a1
	move.l	(a1), -(a7)
	move.l	#fake_bp, (a1)
	MOVE.L    A2,-(A7)
	MOVE.L    D0,-(A7)
	MOVE.W    #$0048,-(A7)
	TRAP      #1
	ADDQ.W    #6,A7
	MOVEA.L   D0,A0
	MOVE.L    (A7)+,A2
	move.l	bp_addr(pc), a1
	move.l	(a7)+, (a1)
	RTS


.export Mfree
Mfree:
	move.l	bp_addr(pc), a1
	move.l	(a1), -(a7)
	move.l	#fake_bp, (a1)
	MOVE.L    A2,-(A7)
	MOVE.L    A0,-(A7)
	MOVE.W    #$0049,-(A7)
	TRAP      #1
	ADDQ.W    #6,A7
	MOVE.L    (A7)+,A2
	move.l	bp_addr(pc), a1
	move.l	(a7)+, (a1)
	RTS
***/

pex1:	MOVE.L    $0008(A7),-(A7)
	PEA       (A1)
	PEA       (A0)
	MOVE.W    D0,-(A7)
	MOVE.W    #$004B,-(A7)
	TRAP      #1
	LEA       $0010(A7),A7
	MOVE.L    (A7)+,A2
	RTS

.import memcpy, has_mint
.import magic_bp, breakpoint
.export Pexec
Pexec:
	PEA       (A2)
	tst.b	  has_mint
	bne.b	  pex1
	move.l	  bp_addr(pc), a2
	move.l	  (a2), -(sp)
	move.l	  #fake_bp, (a2)
	MOVE.L    $000C(A7),-(A7)
	PEA       (A1)
	PEA       (A0)
	MOVE.W    D0,-(A7)
	MOVE.W    #$004B,-(A7)
	lea	  fake_bp, a0
	lea	  magic_bp, a1
	move.l    #$100, d0
	bsr       memcpy
	TRAP      #1
	LEA       $0010(A7),A7
	move.l	  bp_addr(pc), a2
	move.l	  (sp)+, (a2)
	MOVE.L    (A7)+,A2
	RTS

.import in_t1, old_t1, no_set, ROM_ver
old_sp:	ds.l	1
mall_mode:
	ds.w	1	; 004: memory protection Global
.export Malloc, mall_mode
Malloc:
	move	mall_mode(pc), d1
	bne.b	Mxalloc
	pea	(a2)
mall1:	move.l	a7, old_sp
	MOVE.L  D0,-(A7)
	MOVE.W  #$0048,-(A7)
mf:	st	no_set
	tst.b	in_t1
	bne.b	mf1
	TRAP    #1
mfret:	move.l	old_sp(pc), sp
	sf	no_set
	MOVEA.L D0,A0
	MOVE.L  (A7)+,A2
	RTS


.export Mxalloc
Mxalloc:
	pea	(a2)
	move	ROM_ver, d2
	cmpi	#$206, d2
	blt.b	mall1
mxa1:	move.l	a7, old_sp
	move	d1, -(a7)
	MOVE.L  D0,-(A7)
	MOVE.W  #$0044,-(A7)
	bra.b	mf

.export Mfree
Mfree:
	pea	(a2)
	move.l	a7, old_sp
	pea	(a0)
	MOVE.W  #$0049,-(A7)
	bra.b	mf

.export Mshrink
Mshrink:
	pea	(a2)
	move.l	a7, old_sp
	move.l	d1, -(a7)
	pea	(a0)
	move	d0, -(a7)
	MOVE.W  #$004A,-(A7)
	bra.b	mf

mf1:	tst	$59e.w
	beq.b	mf2
	clr	-(sp)
mf2:	pea	mfret(pc)
	move	sr, -(sp)
	move.l	old_t1, a0
	jmp	(a0)

.export lock_mem
.import Pgetpid, Syield, preempt
memlock:ds.b	1			; must be in this order
memllev:ds.b	1			;
memid:	ds.w	1			;
lock_mem:				; 006: make lock and id separate
	move.b	preempt(pc), d0
	bne.b	lm3
	rts
;;lm2:	move	memllev(pc), d0		006
;;	bne.b	lmi
lm3:	jsr	Pgetpid
	tas	memlock
	beq.b	lm4
	cmp	memid(pc), d0		; 006
	beq.b	lmi			;
	jsr	Syield
	bra.b	lm3
lm4:	move	d0, memid		; 006: into memid
lmi:	addq.b	#1, memllev		; 006: .b
	rts

.export unlock_mem
unlock_mem:
	move.b	preempt(pc), d0
	bne.b	um2
	rts
um2:	subq.b	#1, memllev		; 006: .b
	bne.b	um3
	clr.l	memlock			; 006: .l
um3:	rts
