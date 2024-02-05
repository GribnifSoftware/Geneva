#include "new_aes.h"
#include "xwind.h"              /* Geneva extensions */
#include "tos.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "lerrno.h"
#include "gnva_mac.h"
#include "common.h"

/* Put up an alert, taking a string from the resource file */
int alert( int num )
{
  char **ptr;

  rsrc_gaddr( 15, num, &ptr );
  return form_alert( 1, *ptr );
}

int set_ap( int id )
{
  return (*(int (*)( int apid ))(cookie->xaes_funcs[6]))( id );
}

void xmfree( void *addr )
{
  if( aes_ok ) x_mfree(addr);
  else if( set_ap(apid) )
  {
    (*(int (*)( void *addr ))(cookie->xaes_funcs[0x87]))( addr );
    set_ap(-1);
  }
}

void *xmalloc( long size )
{
  void *ptr;

  if( aes_ok )
  {
    x_malloc( &ptr, size );
    if( !ptr )
    {
      alert(NOMEMORY);
      return 0L;
    }
  }
  else if( set_ap(apid) )
  {
    (*(void (*)( void **addr, long size ))(cookie->xaes_funcs[0x86]))
        ( &ptr, size );
    set_ap(-1);
  }
  else return 0L;
  return ptr;
}

int xrealloc( void **start, long size )
{
/*  void *ptr;

  if( !size ) ptr = 0L;
  else
  {
    if( (ptr = xmalloc(size)) == 0 ) return 1;
    if( *start ) memcpy( ptr, *start, size );
  }
  if( *start ) xmfree(*start);
  *start = ptr;
  return 0; */
  int i;

  if( !*start )
  {
    *start = xmalloc(size);
    return *start ? 0 : 1;
  }
  if( aes_ok )
  {
    if( x_realloc( start, size ) )
    {
      alert(NOMEMORY);
      return 1;
    }
  }
  else if( set_ap(apid) )
  {
    i = (*(int (*)( void **addr, long size ))(cookie->xaes_funcs[0x89]))
        ( start, size );
    set_ap(-1);
    if(i) return 1;
  }
  else return 1;
  return 0;
}

int add_thing( void **start, int *total, int *remain, void *add,
    long size )
{
  if( !*start )
  {
    *total = 0;
    if( (*start = xmalloc( (*remain=15)*size )) == 0 ) return 0;
  }
  else if( !*remain )
    if( xrealloc( start, ((*remain=15)+*total)*size ) ) return 0;
  if( add ) memcpy( (char *)(*start) + (*total * size), add, size );
  ++(*total);
  --(*remain);
  if( *total<0 ) return 0;
  return 1;
}

int add_obj( OBJECT **list, int *total, int *remain, OBJECT *o )
{
  if( *list && *total==0x7fff )
  {
    if( aes_ok ) alert(OBJBIG);
    return 0;
  }
  return add_thing( (void **)list, total, remain, o, sizeof(OBJECT) );
}

/* Close and delete a window */
void close_wind( int *hand )
{
  if( *hand>0 )
  {
    if( aes_ok )
    {
      wind_close(*hand);
      wind_delete(*hand);
    }
    else if( set_ap(apid) )
    {
      (*(int (*)( int h ))(cookie->aes_funcs[0x66-10]))(*hand);
      (*(int (*)( int h ))(cookie->aes_funcs[0x67-10]))(*hand);
      set_ap(-1);
    }
    *hand=0;
  }
}

int chknum;

unsigned int rol( int n ) 0xE358;

int chksm( void *place, int chk, int size )
{
  chk += chknum++;
  for(;;)
  {
    chk = rol(chk);
    if( !size-- ) break;
    chk ^= *((char *)place)++;
  }
  return chk;
}

int chksmstr( char *place, int chk )
{
  return chksm( place, chk, strlen(place) );
}

int chksum(void)
{
  int sum=0, i, j, k;
  MACDESC *m;
  MACBUF *mb;

  chknum=0;
  for( i=iglobl, m=mdesc; --i>=0; m++ )
  {
    sum = chksmstr( m->name, sum );
    sum = chksm( &m->key, sum, sizeof(m->key) );
    sum = chksm( &m->auto_on, sum, 4*sizeof(int) );
    for( mb=m->mb, j=m->len; --j>=0; mb++ )
      if( mb->type==TYPE_MSG )
        for( k=5; --k>=0; )
          sum = chksmstr( (*mb->u.msg)[k], sum );
      else sum = chksm( mb, sum, sizeof(MACBUF) );
  }
  return sum;
}

int test_close(void)
{
  int i;

  if( (i=chksum()) != last_chk )
  {
    if( !aes_ok )
    {
      Bconout(2,7);
      return 0;
    }
    switch( alert( NEEDSAVE ) )
    {
      case 1:	/* save */
        return do_save(0);
      case 2:	/* abandon */
        last_chk = i;
        return 1;
      case 3:	/* cancel */
        return 0;
    }
  }
  return 1;
}

void do_help( char *topic )
{
  char path[120], temp[120], *p, *t;

  strcpy( path, "GNVA_MAC.HLP" );
  if( !shel_find(path) ) strcpy( path, "GNVA_MAC.HLP" );
  else
  {
    strcpy( temp, path );
    p = path;
    t = temp;
    if( t[1] != ':' )		/* no drive letter */
    {
      *p++ = Dgetdrv()+'A';
      *p++ = ':';
    }
    else			/* copy drive letter: */
    {
      *p++ = *t++;
      *p++ = *t++;
    }
    if( *t != '\\' )		/* not an absolute path */
    {
      Dgetpath( p, 0 );		/* get my directory */
      p += strlen(p);
      *p++ = '\\';
    }
    strcpy( p, t );
  }
  if( !x_help( topic, path, 0 ) ) alert( NOHELP );
}

/* de/select an object */
void set_if( OBJECT *tree, int num, int true )
{
  unsigned int *i = &tree[num].ob_state;

  if( true ) *i |= SELECTED;
  else *i &= ~SELECTED;
}

int eselfunc( int func( EDITDESC *e, int num ), EDITDESC *e, int backward )
{
  int i;

  if( !backward )
  {
    i = evnt_list(e,0);
    if( i>=0 )
      do
        if( !(*func)(e,i) ) return 0;
      while( (i = evnt_list(e,1)) >= 0 );
  }
  else
    for( i=e->refs; --i>=0; )
      if( e->o[e->ref[i]].ob_state&SELECTED )
        if( !(*func)(e,i) ) return 0;
  return 1;
}

int selfunc( int func(int num), int backward )
{
  int i;

  if( !backward )
  {
    i = mac_list(0);
    if( i>=0 )
      do
        if( !(*func)(i) ) return 0;
      while( (i = mac_list(1)) >= 0 );
  }
  else
    for( i=nmacs; --i>=0; )
      if( maclist[mdesc[i].obj].ob_state&SELECTED )
        if( !(*func)(i) ) return 0;	/* maclist & mdesc may change */
  return 1;
}

char *spathend( char *p )
{
  char *p2;

  if( (p2 = strrchr(p,'\\')) != 0 ) return p2+1;
  return p;
}

int fselect( char *path, char *templ, int title )
{
  int b, r;
  char fname[13], *ptr, **t;

  strcpy( fname, ptr=spathend(path) );
  strcpy( ptr, templ );
  rsrc_gaddr( 15, title, &t );
  r = x_fsel_input( path, 120, fname, 1, &b, *t );
  ptr = spathend(path);
  if( r && b )
  {
    strcpy( ptr, fname );
    return 1;
  }
  *ptr = 0;
  return 0;
}

void arrow(void)
{
  graf_mouse( ARROW, 0L );
}

void bee(void)
{
  graf_mouse( BUSYBEE, 0L );
}

char *pn( char *b, unsigned int n, int w, int *lim )
{
  int i;
  char nb[20];

  i = 0;
  if( !n ) nb[i++] = '0';
  else
    for (; n; n /= 10)
      nb[i++] = (n%10)+'0';
  w -= i;
  while (w-- > 0) nb[i++] = '0';
  while (*lim>0 && i--)
  {
    *b++ = nb[i];
    --*lim;
  }
  return b;
}

void _strftime( char *buf, char *fmt, int lim, unsigned int dt, unsigned int time )
{
  char c, sep, *p;
  int m, d, y, h, mn, s, n, w;

  m = (dt>>5)&0xf;
  d = dt&0x1f;
  y = (dt>>9)+1980;
  if( (sep = (char)idt) == 0 ) sep = '/';
  h = time>>11;
  mn = (time>>5)&0x3f;
  s = (time&0x1f)<<1;
  while( lim>0 && (c = *fmt++) != 0 ){
    if (c == '%') {
      w = 0;
      switch (*fmt++) {
        case '/':
          *buf++ = sep;
          lim--;
          break;
        case '%':
          *buf++ = '%';
          lim--;
          break;
        case 'b':
          if( (lim-=3) >= 0 )
          {
            strncpy( buf, *smonths+(m-1)*3, 3 );
            buf += 3;
          }
          break;
        case 'B':
          p = months[m-1];
          goto cpy;
        case 'r':	/* internal: 2-digit day */
          n = d;
          w = 2;
          goto dopn;
        case 'd':
          n = d;
          goto dopn;
        case 'H':
          n = h;
          w = 2;
          goto dopn;
        case 'I':
          if( (n = h) == 0 ) n = 12;
          else if( n>12 ) n -= 12;
          goto dopn;
        case 'q':	/* internal: 2-digit month */
          n = m;
          w = 2;
          goto dopn;
        case 'm':
          n = m;
          goto dopn;
        case 'M':
          n = mn;
          w = 2;
          goto dopn;
        case 'p':
          p = h>=12 ? "pm" : "am";
cpy:	  while( *p && --lim>=0 )
	    *buf++ = *p++;
	  break;
        case 's':
          n = s;
          w = 2;
          goto dopn;
        case 'y':
          n = y%100;
          w = 2;
          goto dopn;
        case 'Y':
          n = y;
          w = 4;
dopn:	  buf = pn( buf, n, w, &lim );
      }
    }
    else
    {
      *buf++ = c;
      lim--;
    }
  }
  *buf = 0;
}
