/*
 * General-purpose memory allocator compatible with MWC but much faster. In
 * addition, blocks are coalesced when they're freed, and if this results
 * in an arena no allocated blocks, it's returned to the OS.
 *
 * The functions here have the same names and bindings as the MWC
 * memory manager, which is the same as the UNIX names and bindings
 * plus lmalloc etc.
 *
 * malloc() calls are satisfied by using Malloc() to get large arenas,
 * which are then chopped up and returned from malloc().
 *
 * Placed unsupported in the public domain by Allan Pratt, April 22, 1991.
 */

/*
 * configuration section: define NO_VOID_STAR if your compiler can't
 * deal with it (Alcyon can't), and leave it undefined if it can.
 *
 * Define DEBUGON to write characters to stdout if you want to see some of
 * what's going on.
 *
 * The only dependency from osbind.h is Malloc().
 *
 * There is a little protection here against using "too many" blocks, but
 * the TOS 1.0 / TOS 1.2 limits on allocating blocks are a great reason to
 * upgrade to TOS 1.4.  The protection is in MAXARENA and minarena;
 * see those comments for details.  There are no guarantees.
 */

#include <tos.h>
#include <string.h>
#include "aes.h"
#include "win_var.h"
#include "win_inc.h"
#include "debugger.h"

#ifdef DEBUG_ON
  #define MEM_DEBUG
#endif

#define TOS_ver ((*(SYSHDR **)0x4f2)->os_version)

#define VOID_STAR void *

#ifdef DEBUGON
#define DEBUG(c) Cconout(c)
#else
#define DEBUG(c) /*nothing*/
#endif

#define NULL ((char *)0)

/*
 * block header: every memory block has one.
 * A block is either allocated or on the free
 * list of the arena.  There is no allocated list: it's not necessary.
 * The next pointer is only valid for free blocks: for allocated blocks
 * maybe it should hold a verification value..?
 *
 * Zero-length blocks are possilbe free; they hold space which might
 * get coalesced usefully later on.
 */

struct block {
    struct block *b_next;   /* NULL for last guy; next alloc or next free */
    long b_size;
};

/*
 * arena header: every arena has one.  Each arena is always completely
 * filled with blocks; the first starts right after this header.
 */

struct arena {
    struct arena *a_next;
    struct block *a_ffirst;
    long a_size;
};

struct used_bl {
    struct used_bl *next;
    long size;
    int id;
} *last_used;

/*
 * Arena linked-list pointer, and block size.  The block size is initialized
 * to Malloc(-1L)/20 when you start up, because somebody said that 20
 * Malloc blocks per process was a reasonable maximum.  This is hopelessly
 * unbalanced: 25K on a 520ST and 200K on a Mega 4, so it's tempered by
 * the constant MAXARENA as the largest minimum you'll get (I chose 100K).
 */

struct arena *a_first = (struct arena *)NULL;
static long minarena = -1L;

#define MAXARENA (100L*1024L)
#define MINARENA (1024L*8)

BASPAG fake_bp;

long get_ver(void)
{
  return TOS_ver;
}

#ifdef MEM_DEBUG

static int chksum;
int mchksm( void *ptr, int size )
{
  int sum=0;

  size>>=1;
  while( --size >= 0 )
    sum = ((sum+1)<<1) + *((int *)ptr)++;
  return sum;
}

int mem_chk(void)
{
  struct arena *a;
  register struct block *b;
  int sum=0;

  for (a = a_first; a; a = a->a_next)
  {
    sum += mchksm( a, sizeof(struct arena) );
    for (b = a->a_ffirst; b; b = b->b_next)
      sum += mchksm( b, sizeof(struct block) );
  }
  return sum;
}
#endif

void *small_malloc( long len, int mode )	/* 005 */
{
  void *p=0L, *n;
  long l;

  if( len>=sizeof(long) && best_malloc )
  {
    for(;;)
    {
      l = (long)Mxalloc(-1L,mode);
      if( l<len || (n = Mxalloc(l,mode)) == 0 ) break;
      *(void **)n = p;
      p = n;
    }
    if( !p ) return 0L;
    n = *(void **)p;
    Mshrink( 0, p, len );
    if( !n ) return p;
    do
    {
      l = *(long *)n;
      Mfree(n);
    } while( (n = (void *)l) != 0 );
    return p;
  }
  return Mxalloc(len,mode);
}

char *lalloc( long size, int id )
{
    struct arena *a, *a2;
    register struct block *b, *mb, **q;
    register long temp;

    DEBUG('A');
    /* make sure size is a multiple of four; fail if it's negative */
    if (size <= 0)
    {
      DEBUGGER(LALLOC,BADSIZE,0);
      return 0;
    }
    size = (size+3+sizeof(struct used_bl)) & ~3;

    lock_mem();

#ifdef MEM_DEBUG
    if( chksum != mem_chk() )
    {
      DEBUGGER(LALLOC,BADCHK,0);
      chksum = mem_chk();
      unlock_mem();
      return 0;
    }
#endif
    for (a = a_first; a; a = a->a_next) {
        for (b = *(q = &a->a_ffirst); b; b = *(q = &b->b_next)) {
            /* if big enough, use it */
            if (b->b_size >= size) {

                /* got one */
                mb = b;

                /* cut the free block into an allocated part & a free part */
                temp = mb->b_size - size - sizeof(struct block);
                if (temp >= 0) {
                    /* large enough to cut */
                    DEBUG('c');
                    b = (struct block *)(((char *)(b+1)) + size);
                    b->b_size = temp;
                    b->b_next = mb->b_next;
                    *q = b;
                    mb->b_size = size;
                }
                else {
                    /* not big enough to cut: unlink this from free list */
                    DEBUG('w');
                    *q = mb->b_next;
                }
                mb++;
                ((struct used_bl *)mb)->next = last_used;
                ((struct used_bl *)mb)->size = size;
                ((struct used_bl *)mb)->id = id;
                last_used = (struct used_bl *)mb;
#ifdef MEM_DEBUG
		chksum = mem_chk();
#endif
                unlock_mem();
                return (char *)mb+sizeof(struct used_bl);
            }
        }
    }

    /* no block available: get a new arena */

    if (minarena<0) {
        if( (int)Supexec(get_ver) > 0x102 ) minarena = MINARENA;
        else minarena = (long)Malloc(-1L) / 20;
        if (minarena > MAXARENA) minarena = MAXARENA;
/*        memcpy( &fake_bp, _BasPag, sizeof(BASPAG) );*/
    }

    if (size < minarena) {
        DEBUG('m');
        temp = minarena;
    }
    else {
        DEBUG('s');
        temp = size;
    }
	/* 005: was Malloc(len) */
    a = (struct arena *)small_malloc(temp +
                                sizeof(struct arena) +
                                sizeof(struct block), 3);

    /* if Malloc failed return failure */
    if (a == 0) {
        DEBUG('x');
        unlock_mem();
        return 0;
    }

    a->a_size = temp + sizeof(struct block);
    /* 004: new blocks are now added to the end of the list */
    /* was: a->a_next = a_first; a_first = a; */
    a->a_next = 0L;
    if( (a2=a_first) != 0 )
    {
      for( ; a2->a_next; a2 = a2->a_next);
      a2->a_next = a;
    }
    else a_first = a;
    mb = (struct block *)(a+1);
    mb->b_next = 0;
    mb->b_size = size;

    if (temp > (size + sizeof(struct block))) {
        DEBUG('c');
        b = a->a_ffirst = (struct block *)(((char *)(mb+1)) + size);
        b->b_next = 0;
        b->b_size = temp - size - sizeof(struct block);
    }
    else {
        a->a_ffirst = 0;
    }

    mb++;
    ((struct used_bl *)mb)->next = last_used;
    ((struct used_bl *)mb)->size = size;
    ((struct used_bl *)mb)->id = id;
    last_used = (struct used_bl *)mb;
#ifdef MEM_DEBUG
    chksum = mem_chk();
#endif
    unlock_mem();
    return (char *)mb+sizeof(struct used_bl);
}

void shrink( struct arena *a, struct block *fb, struct block *pb )	/* 004 */
{
  long newblock;

  /* if size was above minarena, shrink it if possible */
  if( a->a_size-sizeof(struct block) > minarena && !fb->b_next )
  {
    newblock = a->a_size-fb->b_size+sizeof(struct block);
    if( newblock < minarena )
    {
      newblock = minarena;
      fb->b_size = (long)(a+1)+newblock - (long)(fb+1);
    }
    else if( !pb ) a->a_ffirst = 0L;
    else pb->b_next = 0L;
    Mshrink( 0, a, (a->a_size=newblock)+sizeof(struct arena) );
  }
}

int lfree( VOID_STAR xfb )
{
    struct arena *a, **qa;
    struct used_bl *u, *u2;
    register struct block *b;
    register struct block *pb;
    register struct block *fb = (struct block *)((char *)xfb-sizeof(struct used_bl));

    lock_mem();
    DEBUG('F');
    /* set fb (and b) to header start */
    b = --fb;

#ifdef MEM_DEBUG
    if( chksum != mem_chk() )
    {
      DEBUGGER(LFREE,BADCHK,0);
      chksum = mem_chk();
      unlock_mem();
      return 0;
    }
#endif

    /* the arena this block lives in */
    for (a = *(qa = &a_first); a; a = *(qa = &a->a_next)) {
        if ((long)b > (long)a && (long)b < ((long)a + a->a_size)) goto found;
    }
    DEBUGGER(LFREE,NOARENA,0);
    unlock_mem();
    return -1;

found:
    /* Found it! */
    /* a is this block's arena */

    /* remove from used list */
    for( u=last_used, u2=0L; u; u2=u, u=u->next )
      if( (char *)(u+1) == (char *)xfb )
      {
        if( u2 ) u2->next = u->next;
        else last_used = u->next;
        break;
      }
    if( !u )
    {
      DEBUGGER(LFREE,NOUNLINK,0);
      unlock_mem();
      return -1;
    }

    /* set pb to the previous free block in this arena, b to next */
    for (pb = 0, b = a->a_ffirst;
         b && (b < fb);
         pb = b, b = b->b_next)
      if( b==pb )
      {
        DEBUGGER(LFREE,BADLINK,0);
        unlock_mem();
        return -1;
      }

    fb->b_next = b;

    /* Coalesce backwards: if any prev ... */
    if (pb) {
        /* if it's adjacent ... */
        if ((((long)(pb+1)) + pb->b_size) == (long)fb) {
            DEBUG('b');
            pb->b_size += sizeof(struct block) + fb->b_size;
            fb = pb;
        }
        else {
            /* ... else not adjacent, but there is a prev free block */
            /* so set its next ptr to fb */
            pb->b_next = fb;
        }
    }
    else {
        /* ... else no prev free block: set arena's free list ptr to fb */
        a->a_ffirst = fb;
    }

    /* Coalesce forwards: b holds start of free block AFTER fb, if any */
    if (b && (((long)(fb+1)) + fb->b_size) == (long)b) {
        DEBUG('f');
        fb->b_size += sizeof(struct block) + b->b_size;
        fb->b_next = b->b_next;
    }

    /* if, after coalescing, this arena is entirely free, Mfree it! */
    if ( (long)(a->a_ffirst) == (long)(a+1) &&
        (a->a_ffirst->b_size + sizeof(struct block)) == a->a_size) {
            DEBUG('!');
            *qa = a->a_next;
            (void)Mfree(a);
    }
    else shrink( a, fb, pb );	/* 004 */

#ifdef MEM_DEBUG
    chksum = mem_chk();
#endif
    unlock_mem();
    return 0;   /* success! */
}

int lfreeall( int id )
{
  struct used_bl *u, *bl;

  for( u=last_used; u; )
    if( u->id == id )
    {
      bl = u;
      u = u->next;
      if( lfree((char *)bl+sizeof(struct used_bl)) ) return 0;
    }
    else u = u->next;
  return 1;
}

int lshrink( VOID_STAR xfb, long newsize )
{
    struct arena *a;
    struct used_bl *u;
    register struct block *b, *nb, *pb;
    register struct block *fb = (struct block *)((char *)xfb-sizeof(struct used_bl));
    long delta;

    lock_mem();
    /* set fb (and b) to header start */
    b = --fb;

#ifdef MEM_DEBUG
    if( chksum != mem_chk() )
    {
      DEBUGGER(LSHRINK,BADCHK,0);
      chksum = mem_chk();
      unlock_mem();
      return -1;
    }
#endif

    if( newsize<=0L )
    {
      unlock_mem();
      return -1;	/* bad newsize size */
    }

    /* the arena this block lives in */
    for (a = a_first; a; a = a->a_next) {
        if ((long)b > (long)a && (long)b < ((long)a + a->a_size)) goto found;
    }
    DEBUGGER(LSHRINK,NOARENA,0);
    unlock_mem();
    return -1;

found:
    /* Found it! */
    /* a is this block's arena */

    newsize = (newsize + sizeof(struct used_bl) + 3) & ~3;
    /* shrink in used list */
    for( u=last_used; u; u=u->next )
      if( (char *)(u+1) == (char *)xfb )
        if( (delta = u->size - newsize) < 0 )
        {
          DEBUGGER(LSHRINK,NEWGTOLD,0);
          unlock_mem();
          return -1;
        }
        else
        {
          u->size = newsize;
          if( delta <= sizeof(struct used_bl) )
              goto ret0;		/* not enough savings to be worth it */
          break;
        }
    if( !u )
    {
      DEBUGGER(LSHRINK,NOUNLINK,0);
      unlock_mem();
      return -1;
    }

    /* set pb to the previous free block in this arena, b to next */
    for (pb = 0, b = a->a_ffirst;
         b && (b < fb);
         pb = b, b = b->b_next)
      if( b==pb )
      {
        DEBUGGER(LSHRINK,BADLINK,0);
        unlock_mem();
        return -1;
      }

    fb->b_size = newsize;

    /* create a newsize free block */
    fb = (struct block *)((long)(fb+1) + newsize);
    fb->b_next = b;
    fb->b_size = delta - sizeof(struct block);

    if( !pb ) {
        /* no prev free block: set arena's free list ptr to fb */
        a->a_ffirst = fb;
    }

    /* Coalesce forwards: b holds start of free block AFTER fb, if any */
    if (b && (((long)(fb+1)) + fb->b_size) == (long)b) {
        fb->b_size += sizeof(struct block) + b->b_size;
        fb->b_next = b->b_next;
    }

    shrink( a, fb, pb );

ret0:
#ifdef MEM_DEBUG
    chksum = mem_chk();
#endif
    unlock_mem();
    return 0;   /* success! */
}

int lrealloc( void **xfb, long size )
{
    struct arena *a;
    struct used_bl *u;
    register struct block *b;
    register struct block *pb, *mb;
    register struct block *fb = (struct block *)((char *)*xfb-sizeof(struct used_bl));
    long newblock, temp;
    void *newb;

    lock_mem();
    /* set fb (and b) to header start */
    b = --fb;

#ifdef MEM_DEBUG
    if( chksum != mem_chk() )
    {
      DEBUGGER(LREALLOC,BADCHK,0);
      chksum = mem_chk();
      unlock_mem();
      return -1;
    }
#endif

    /* the arena this block lives in */
    for (a = a_first; a; a = a->a_next) {
        if ((long)b > (long)a && (long)b < ((long)a + a->a_size)) goto found;
    }
    DEBUGGER(LREALLOC,NOARENA,0);
    unlock_mem();
    return -1;

found:
    /* Found it! */
    /* a is this block's arena */

    newblock = (size + sizeof(struct used_bl) + 3) & ~3;
    /* find in used list */
    for( u=last_used; u; u=u->next )
      if( (char *)(u+1) == (char *)*xfb )
        if( u->size==newblock )
        {
          unlock_mem();
          return 0;	/* no change */
        }
        else if( u->size > newblock )	/* shrink it */
        {
          lshrink( *xfb, size );
          unlock_mem();
          return 0;
        }
        else break;
    if( !u )
    {
      DEBUGGER(LREALLOC,NOUNLINK,0);
      unlock_mem();
      return -1;
    }

    /* set pb to the previous free block in this arena, b to next */
    for (pb = 0, b = a->a_ffirst;
         b && (b < fb);
         pb = b, b = b->b_next)
      if( b==pb )
      {
        DEBUGGER(LREALLOC,BADLINK,0);
        unlock_mem();
        return -1;
      }

    /* expand forward, if contiguous and large enough */
    if( b && (long)u+u->size == (long)b &&
        (temp=b->b_size+sizeof(struct block)+u->size) >= newblock )
    {
      if( temp-newblock > sizeof(struct used_bl) )
      {
        mb = (struct block *)((long)u + newblock);
        mb->b_next = b->b_next;	/* destroying b, so skip it */
        mb->b_size = temp-newblock-sizeof(struct block);
        u->size = fb->b_size = newblock;
      }
      else
      {
        u->size = fb->b_size = temp;
        mb = b->b_next;
      }
      if( pb ) pb->b_next = mb;
      else a->a_ffirst = mb;	/* first free in arena */
#ifdef MEM_DEBUG
      chksum = mem_chk();
#endif
      unlock_mem();
      return 0;
    }
    if( (newb = lalloc( size, u->id )) == 0 )
    {
      unlock_mem();
      return -1;
    }
    memcpy( newb, *xfb, u->size-sizeof(struct used_bl) );
    lfree(*xfb);
    *xfb = newb;

#ifdef MEM_DEBUG
    chksum = mem_chk();
#endif
    unlock_mem();
    return 0;   /* success! */
}
