#include "string.h"
#include "stdlib.h"
#include "tos.h"
#include "stdio.h"
#include "ctype.h"
#include "new_aes.h"
#include "vdi.h"
#include "xwind.h"
typedef struct { int x, y, w, h; } Rect;
#include "..\graphics.h"
#include "gnvahelp.h"
#include "..\font_sel.h"

#define LINEINC 50
#define LINELEN 100
#define JUMPS   20
#define NBUTS   15	/* 004 */

#define MODE_ALL -1
#define MODE_TEXT 1
#define MODE_SEL  2

#define HELP_ID		0x476E0100	/* Gn 1.0 */
#define HELP_ID1	0x476E0101	/* Gn 1.1 */
#define HELP_ID2	0x476E0102	/* Gn 1.2 */
#define SH_REGHELP	0x7108	/* 005: I am the Help program; intentionally overlaps with SHW_ENVIRON */

enum
{
   F_IMAGE=0, F_BUTTON, F_BAR, F_LINE, F_RBOX, F_RFBOX, F_RECFL,
   F_FILC, F_FILI, F_FILST, F_LINC, F_LINEND, F_LINST, F_WRITE, F_LWIDTH,
   F_EXTERN
};

int hand, nyb, wind;
long *tbl1, *tbl2;
struct Imgtbl
{
  long offset, unpacked;
} *imgtbl;
int rez, bl_rows, bl_cols, apid, line_num, line_cnt, line_len,
    sel_row, sel_col, sele_row, sele_col, draw_mode=MODE_ALL, jumps[JUMPS], jnum,
    jstart, jump_pos[JUMPS][2], min_wid, min_ht, search_ret, corn_x, corn_y,
    count, gr_rows, btn_jmp[NBUTS], sys_char_h, nimgs, lines;	/* 004: count was char */
char path[120], fname[13], info_txt[100], info_max, read_err,
    **title, search_top[30], know_cols, savepth[120], savename[13], name[40],
    geneva_fmt, in_rev, in_func, iconified,
    extern_hlp[13];	/* 004 */
unsigned char (*line)[][LINELEN+2], *line_pos;  /*, llen[LINES];*/

int vplanes, xmax, ymax, vwrap, xasp, yasp, colvals, colmax, colmax1;	/* SHOWPIC */
char TT_vid, rgb15, intel, falc_vid;

int work_in[] = { 1, 7, 1, 1, 1, 1, 1, 1, 1, 1, 2 };

long _Fseek( long offset, int seekmode );
long _Fread( long count, void *buf );

#define GNVAHELP
#define CJar_xbios      0x434A          /* "CJ" */
#define CJar_OK         0x6172          /* "ar" */
#define CJar(mode,cookie,value)         (int)xbios(CJar_xbios,mode,cookie,value)
#define _VDO_COOKIE 0x5F56444FL
#include "\source\neodesk\showpic.c"

PICTURE **imgs;

static struct		/* for Pure C format files */
{
  char info[84];
  long id;
  long tbl1_len,
       tbl2_start,
       tbl2_len;
  char fast[12];
  long caps_start,
       caps_len,
       caps_entries,
       sens_start,
       sens_len,
       sens_entries;
} help;

typedef struct header	/* for Geneva-format files */
{
  long id;
  int hdsize;		/* sizeof(HEADER) */
  long tbl_len,		/* longword, alphabetical pointers */
       caps_start,	/* 6-byte table, relative to entry */
       caps_len,
       caps_entries,
       sens_start,
       sens_len,
       sens_entries,
       img_start;
  int  img_len;
} HEADER;

HEADER header;

int cdecl draw_text( PARMBLK *pb );
void new_screen( int draw, int *xy );
void do_index(void);
int prev_jump( int num );
void jump_tbl( int i, char *s );
int open_help( char *s );
void get_corner(void);
void v_ftext16_mono( int handle, int x, int y, int *wstr, int strlen, int offset );

GRECT winner, wsize, max;
USERBLK ub_main = { draw_text };
OBJECT blank[NBUTS+1] = {
    { -1, -1, -1, G_USERDEF, TOUCHEXIT,       0, (long)&ub_main },
    { 0, -1, -1,  G_BUTTON,  SELECTABLE|EXIT, X_MAGIC|X_PREFER, 0L } },
    *menu, *icon;
Rect clip_rect;

void (*decode_func)( unsigned char c );	/* 004 */

#define HELP_SET_VER 0x0100
#define HELP_VER     0x0101
#define WIN_TYPE NAME|MOVER|CLOSER|FULLER|SIZER|UPARROW|DNARROW|VSLIDE|LFARROW|RTARROW|HSLIDE|INFO|SMALLER

struct Set
{
  int ver;
  int font[3], point[3];
  int rows[3], cols[3], txt[3];		/* 004: changed */
  int xoff[3], yoff[3];
  char topic[21], match, all;
  char hlp_path[80];
} set = { HELP_SET_VER, 1, 1, 1, 8, 9, 10, 18, 18, 20, 45, 75, 75, BLACK, RED, LRED };

void xmfree( void *addr )
{
  x_mfree(addr);
}

void *xmalloc( long size )
{
  void *ptr;

  x_malloc( &ptr, size );
  return ptr;
}

int xrealloc( void **start, long size )
{
  int i;

  if( !*start )
  {
    *start = xmalloc(size);
    return *start ? 0 : 1;
  }
  return x_realloc( start, size );
}

char *pathend( char *ptr )
{
  char *s;
  
  if( (s=strrchr(ptr,'\\')) == 0 ) return(ptr);
  return(s+1);
}

void enab_prev(void)
{
  int i;
  
  if( menu )
  {
    i = prev_jump( jnum );
    menu_ienable( menu, MPREV, hand>0 && jnum!=jstart && i!=jstart );
  }
}

void set_entries(void)
{
  int i;
  
  if( menu )
  {
    menu_ienable( menu, MFIND, i=hand>0 );
    menu_ienable( menu, MINDEX, i );
    menu_ienable( menu, MTEXT, i );
    menu_ienable( menu, MBLOCK, i=sel_row!=-99 );
    menu_ienable( menu, MCLIP, i );
    enab_prev();
  }
}

void set_name( char *s )
{
  if( wind>0 )
    if( hand>0 && s )
    {
      x_sprintf( name, "%s: %s", *title+2, s );
      wind_set( wind, WF_NAME, name );
    }
    else wind_set( wind, WF_NAME, *title + 2 );
}

void set_info(void)
{
  wind_set( wind, WF_INFO, info_txt );
  enab_prev();
}

void clear_info( int set )
{
  info_txt[0] = info_max = 0;
  jnum = jstart = 0;
  if( set ) set_info();
}

void add_info( char *s )
{
  if( !info_max )
    if( strlen(info_txt) + strlen(s) >= sizeof(info_txt) ) info_max=1;
    else strcat( info_txt, s );
}

void init_line(void)
{
  memset( (*line)[line_num]+1, ' ', LINELEN );
  (*line)[line_num][LINELEN+1] = 0;	/* 005 dynamic */
}

void shift_graphics(void)	/* 004 */
{
  int i;
  unsigned char temp[LINELEN+2], *ptr;
  long l;
  
  gr_rows = 0;
  extern_hlp[0] = 0;
  for( i=bl_rows; --i>=0; )
    if( *((ptr = (*line)[i])+1) == '\x1e' )
    {
      if( *(ptr+3) == F_EXTERN ) strncpy( extern_hlp, ptr+4, sizeof(extern_hlp)-1 );
      memcpy( temp, ptr, sizeof(temp) );
      memcpy( ptr, ptr+sizeof(temp), l=(bl_rows-i-1)*(LINELEN+2) );
      memcpy( ptr+l, temp, sizeof(temp) );
      bl_rows--;
      gr_rows++;
    }
}

void no_help( int draw )
{
  char **p;
  int i;
  
  for( line_num=bl_cols=0; line_num<5; line_num++ )
  {
    init_line();
    rsrc_gaddr( 15, EMPLINE+line_num, &p );
    if( !line_num ) x_sprintf( (*line)[line_num]+1, *p, HELP_VER );
    else strcpy( (*line)[line_num]+1, *p );
    if( (i=strlen( (*line)[line_num]+1 )) > bl_cols ) bl_cols = i;
    (*line)[line_num][i+1] = ' ';
  }
  bl_rows = 5;
  shift_graphics();	/* 004 */
  new_screen( draw, 0L );
  clear_info(draw);
  set_name(0L);
}

void free_pic( PICTURE **pic )
{
  if( *pic )
  {
    cmfree( (char **)&(*pic)->mfdb.fd_addr );
    cmfree( (char **)&(*pic)->pall );
    cmfree( (char **)&(*pic)->intens );
    cmfree( (char **)&(*pic)->palette );
    cmfree( (char **)pic );
  }
}

void free_imgs(void)
{
  int i;

  if( imgs )
  {
    for( i=0; i<nimgs; i++ )
      free_pic( &imgs[i] );
    xmfree(imgs);
    imgs = 0L;
  }
  nimgs = 0;
}

void free_help(void)	/* 004 */
{
  if( tbl1 )
  {
    xmfree(tbl1);
    tbl1 = 0L;
  }
  free_imgs();
}

void close_help(void)
{
  if( hand>0 )
  {
    Fclose(hand);
    hand=0;
  }
  free_help();
  if( wind > 0 ) no_help(0);
  set_entries();
}

int alert( int num, int close )
{
  char **p;
  
  if( close )
  {
    close_help();
    no_help(1);
  }
  rsrc_gaddr( 15, num, &p );
  return form_alert( 1, *p );
}

int buflen;

long _read( int hand, long size, void *out )
{
  static char buf[1024], *bufptr;
  int size0=size;
  
  while( size )
  {
    if( buflen<=0 )
      if( (buflen=Fread(hand,sizeof(buf),bufptr=buf)) < 0 ) return -1L;
      else if( !buflen ) return size0-size;
    *((char *)out)++ = *bufptr++;
    buflen--;
    size--;
  }
  return size0;
}

long _seek( long pos, int hand )	/* 004 */
{
  buflen = 0L;
  return Fseek( pos, hand, 0 );
}

unsigned char get_nyb(void)
{
  static unsigned char byte;
  
  if( !nyb )
  {
    if( read_err ) return -1;
    if( _read( hand, 1L, &byte ) != 1L )
    {
      read_err++;
      return -1;
    }
    nyb++;
    return byte>>4;
  }
  nyb--;
  return byte&0xF;
}

unsigned char get_byte(void)
{
  return get_nyb()<<4 | get_nyb();
}

int alloc_lines(void)		/* 005 */
{
/*  unsigned char *ol = (*line)[0];*/

  if( xrealloc( (void **)&line, (long)(lines+LINEINC)*(LINELEN+2) ) ) return 0;
/*  else if( line_pos ) line_pos = line_pos-ol+(*line)[0];*/
  lines += LINEINC;
  return 1;
}

void new_line( void )
{
  unsigned char *ptr;
  
  if( (*(ptr=&(*line)[line_num++][0]) = line_cnt) > bl_cols &&
      *(ptr+1)!='\x1e'/*004*/ ) bl_cols = line_cnt;
  count = in_rev = 0;
  if( line_num>=lines )		/* 005: dynamic */
    if( !alloc_lines() )
    {
      read_err++;
      return;
    }
  line_pos = (*line)[bl_rows=line_num]+1;
  line_cnt = line_len = 0;
  init_line();
}

void add_line( unsigned char c )
{
  char *ptr;
  
  if( c == '\n' && !count ) new_line();
  else if( (c != '\r' || count) && line_len<LINELEN )
  {
    if( count )
    {
      count--;
      if( in_func )	/* 004 */
      {
        count = c - 1;
        in_func = 0;
      }
    }
    else if( c==0x1d )
      if( !in_rev ) count = in_rev = 2;
      else in_rev = 0;
    else if( c==0x1e )	/* 004 */
    {
      if( line_pos != (ptr=(*line)[line_num]+1) && *ptr!='\x1e' )
      {	/* graphics must start line */
        line_num--;	/* back up to prev */
        new_line();	/* and reset to beginning */
      }
      count = in_func = 1;
    }
    else line_cnt++;
    line_len++;
    *line_pos++ = c;
  }
}

void init_unpack(void)
{
  line_num=line_cnt=line_len=0;
  line_pos=(*line)[0]+1;
  count = in_func = in_rev = 0;	/* 004 */
  bl_cols = 0;
/*  know_cols = 0;*/
  init_line();
}

#define N		 4096	/* size of ring buffer */
#define F		   18	/* upper limit for match_length */
#define THRESHOLD	2   /* encode string into position and length
						   if match_length is greater than this */
#define NIL			N	/* index for root of binary search trees */

unsigned long int
		textsize = 0,	/* text size counter */
		codesize = 0,	/* code size counter */
		printcount = 0;	/* counter for reporting progress every 1K bytes */
unsigned char
		text_buf[N + F - 1];	/* ring buffer of size N,
			with extra F-1 bytes to facilitate string comparison */
int		match_position, match_length,  /* of longest match.  These are
			set by the InsertNode() procedure. */
		lson[N + 1], rson[N + 257], dad[N + 1];  /* left & right children &
			parents -- These constitute binary search trees. */
FILE	*infile, *outfile;  /* input & output files */

long get_len;

int _getc(void)
{
  unsigned char c;
  
  if( !get_len ) return EOF;
  _read( hand, 1L, &c );
  if( read_err ) return EOF;
  get_len--;
  return c;
}

void Decode(void)	/* Just the reverse of Encode(). */
{
	int  i, j, k, r, c;
	unsigned int  flags;
	
	memset( text_buf, ' ', N-F );
/*	for (i = 0; i < N - F; i++) text_buf[i] = ' '; */
	r = N - F;  flags = 0;
	for ( ; ; ) {
		if (((flags >>= 1) & 256) == 0) {
			if ((c = _getc()) == EOF) break;
			flags = c | 0xff00;		/* uses higher byte cleverly */
		}							/* to count eight */
		if (flags & 1) {
			if ((c = _getc()) == EOF) break;
			(*decode_func)(c);  text_buf[r++] = c;  r &= (N - 1);	/* 004 */
		} else {
			if ((i = _getc()) == EOF) break;
			if ((j = _getc()) == EOF) break;
			i |= ((j & 0xf0) << 4);  j = (j & 0x0f) + THRESHOLD;
			for (k = 0; k <= j; k++) {
				c = text_buf[(i + k) & (N - 1)];
				(*decode_func)(c);  text_buf[r++] = c;  r &= (N - 1); /* 004 */
			}
		}
	}
	if( read_err ) alert( READERR, 1 );
}

void unpack(void)
{
  unsigned char dat, *ptr;
  int i;

  nyb = 0;
  init_unpack();
  while( !read_err )
  {
    dat = get_nyb();
    if( dat<12 ) add_line( help.fast[dat] );
    else if( dat==12 ) add_line( get_byte() );
    else if( dat==13 )
    {
      i = (get_nyb()<<8) | get_byte();
      i = *(tbl2+1+i) - (long)(ptr = *(char **)(tbl2+i));
      ptr += (long)tbl2;
      while( i-- )
        add_line( *ptr++ ^ 0xA3 );
    }
    else if( dat==14 ) new_line();
    else
    {
      new_line();
      return;
    }
  }
  if( read_err ) alert( READERR, 1 );
}

int search( char *topic, long start, long entries, long len, int cmp( const char *s1, const char *s2 ) )
{
  long l, min, max, ll, min0, max0;
  int *ent, *e, i, ret=0;
  char *s;

  if( !entries ) return 0;  
  if( (ent = xmalloc(len)) == 0 )
  {
    alert( ALNOMEM, 0 );
    return 0;
  }
  buflen = 0;
  if( Fseek( start, hand, 0 ) == start && _read( hand, len, ent ) == len )
      for( l=-1, max=entries, min=0;; )
  {
    ll = l;
    if( (l = (max+min+1)>>1) >= entries ) return 0;
    if( l == ll ) l--;
    e = (int *)((long)ent + 6*l);
    s = (char *)e + *(long *)e;
    min0 = min;
    max0 = max;
    if( (i = (*cmp)(s,topic)) < 0 ) min=l;
    else if( !i )
    {
      search_ret = geneva_fmt ? *(e+2) : (((*(e+2) & 0x7FF8) >> 1) - 1) >> 2;
      strcpy( search_top, s );
      ret = 1;
      break;
    }
    else max=l;
/*    if( max==min || !l || l==ll ) break; */
    if( min0==min && max0==max || !l ) break;
  }
  else alert( READERR, 1 );
  xmfree(ent);
  return ret;
}

void draw_sel(void)
{
  if( wind>0 )
  {
    draw_mode = MODE_SEL;
    x_wdial_draw( wind, 0, 8 );
    draw_mode = MODE_ALL;
  }
}

void unselect(void)
{
  if( sel_row!=-99 ) draw_sel();
  sel_row = sel_col = sele_row = sele_col = -99;
  set_entries();
}

void list_tbl( long *tbl )
{
  int i;
  unsigned char dat, *ptr;
  
  sel_row=-99;	/* 004 */
  unselect();	/* 004 */
  buflen = 0;
  if( Fseek( *tbl, hand, 0 ) != *tbl )
  {
    alert( READERR, 1 );
    return;
  }
  if( geneva_fmt )
  {
    get_len = *(tbl+1) - *tbl;
    decode_func = add_line;	/* 004 */
    init_unpack();	/* 004: moved here */
    Decode();
  }
  else unpack();
  if( line_cnt ) new_line();
  shift_graphics();	/* 004 */
}

char upcase;

int match( const char *str, const char *pat )
{
  char s, p /*, per=0*/;
  
  for(;;)
  {
    if( (p = *pat++) == '\0' )
      if( *str ) return(-1);
      else return(0);
    s = *str++;
    if( p == '*' )
    {
      if( !s ) return(0);
      str--;
      do
      {
/*        if( *str == '.' ) per=1;*/
        if( !match( str, pat ) ) return(0);
      }
      while( *str++ );
/*      if( *pat++ != '.' ) return(0); */
      if( *pat == '*' )
        if( !*(pat+1) ) return(0);
        else return(1);
      else if( *pat /*|| per*/ ) return(1);
      return(0);
    }
    else if( p == '?' )
    {
      if( !s ) return(1);
    }
    else
    {
      if( upcase )
      {
        p = toupper(p);
        s = toupper(s);
      }
      if( p > s ) return(-1);
      else if( p < s ) return(1);
    }
  }
}

void find( char *topic, int sens, int revert, int all )
{
  int ret=0, first=1, did_extern=0;
  int (*func)( const char *str, const char *pat ),
      (*func2)( const char *str, const char *pat );
  char temp[120], **p, *end;
  DTA dta, *old;

  if( strcspn( "*?", topic ) < 2 ) func = func2 = match;
  else
  {
    func = strcmpi;
    func2 = strcmp;
  }
  old = Fgetdta();	/* 004: moved */
  Fsetdta(&dta);
  if( !extern_hlp[0] ) did_extern = 1;	/* 004 */
  for(;;)
  {
    upcase = 1;
    if( !geneva_fmt )
    {
      if( (ret=search( topic, help.sens_start, help.sens_entries, help.sens_len, func ))==0 )
      {
        upcase = 0;
        if( (ret=search( topic, help.caps_start, help.caps_entries, help.caps_len, func2 ))==0 && !sens )
        {
          upcase = 1;
          ret=search( topic, help.caps_start, help.caps_entries, help.caps_len, func );
        }
      }
    }
    else if( (ret=search( topic, header.sens_start, header.sens_entries, header.sens_len, func ))==0 )
    {
      upcase = 0;
      if( (ret=search( topic, header.caps_start, header.caps_entries, header.caps_len, func2 ))==0 && !sens )
      {
        upcase = 1;
        ret=search( topic, header.caps_start, header.caps_entries, header.caps_len, func );
      }
    }
    if( ret || !all ) break;
    if( first && did_extern )		/* 004 */
    {
do_first:
      first=0;
      revert = 1;
      if( Fsfirst(set.hlp_path,0x31) ) break;
      if( did_extern && !strcmpi( dta.d_fname, extern_hlp ) )	/* 004 */
        if( Fsnext() ) break;
    }
    else if( did_extern )		/* 004 */
      if( Fsnext() ) break;
    strcpy( temp, set.hlp_path );
    end = pathend(temp);		/* 004 */
    if( !did_extern )			/* 004 */
    {
      did_extern = 1;
      strcpy( end, extern_hlp );
      if( Fsfirst( temp, 0x27 ) ) goto do_first;
    }
    else strcpy( end, dta.d_fname );
    if( !open_help(temp) ) break;
  }
  Fsetdta(old);
  if( !ret )
  {
    rsrc_gaddr( 15, ALNOTOP, &p );
    x_sprintf( temp, *p, topic );
    form_alert( 1, temp );
    if( revert ) do_index();
  }
  else jump_tbl( search_ret, search_top );
}

int prev_jump( int num )
{
  return --num<0 ? JUMPS-1 : num;
}

int next_jump( int num )
{
  return ++num==JUMPS ? 0 : num;
}

void jump_tbl( int i, char *s )
{
  int j, *xy=0L;
  
  if( i==-1 )
    if( jnum != jstart && prev_jump(jnum) != jstart )
    {
      jnum = prev_jump(jnum);
      xy = jump_pos[jnum];
      i = jumps[prev_jump(jnum)];
      for( j=jstart, s=info_txt; j!=jnum; )
      {
        j = next_jump(j);
        if( (s = strchr(s,'|')) == 0 ) break;
        s++;
      }
      if( s && s!=info_txt ) *(s-2) = 0;
    }
    else return;
  else
  {
    add_info( jnum==jstart ? " " : " | " );
    add_info(s);
    jump_pos[jnum][0] = (blank[0].ob_x - winner.g_x) / char_w;
    jump_pos[jnum][1] = (blank[0].ob_y - winner.g_y) / char_h;
    jumps[jnum] = i;
    jnum = next_jump(jnum);
    if( jnum==jstart )
    {
      jstart = next_jump(jstart);
      if( info_max ) info_txt[0] = 0;
    }
  }
  set_info();
  list_tbl( tbl1+i );
  new_screen( 1, xy );
}

void do_index(void)
{
  char **p;
  
  unselect();
  clear_info(0);
  rsrc_gaddr( 15, INDEX, &p );
  jump_tbl( 1, *p );
}

int select( int titl, char *path, int pthlen, char *name )
{
  int b;
  char **p;
  
  rsrc_gaddr( 15, titl, &p );
  if( x_fsel_input( path, pthlen, name, 1, &b, *p ) && b ) return 1;
  return 0;
}

int open_help( char *s )
{
  long l;
  char dflt=0, temp[13];
  int ret=0;
  
  if( !s )
  {
    strcpy( temp, pathend(path) );
    strcpy( pathend(path), fname );
    dflt++;
    s = path;
  }
  close_help();
  buflen = 0;
  if( (hand = Fopen( s, 0 )) > 0 )
  {
    set_name( pathend(s) );
    free_help();	/* 004 */
    geneva_fmt = 0;
    if( _read( hand, sizeof(header), &header ) == sizeof(header) )
      if( header.id == HELP_ID || header.id <= HELP_ID2/*005*/ )
      {
        geneva_fmt=1;
        if( header.id == HELP_ID )	/* 004 */
        {
          header.img_start = 0L;
          header.img_len = 0;
        }
        if( (tbl1 = (long *)xmalloc( l = header.tbl_len +
            header.caps_len + header.sens_len + header.img_len/*004*/)) != 0 )
        {
          if( header.hdsize != sizeof(header) ) _seek( header.hdsize, hand );  /* 004 */
          if( _read( hand, l, tbl1 ) == l )
          {
            imgtbl = (struct Imgtbl *)((long)tbl1+l-header.img_len);
            read_err=0;
            ret = 1;
          }
          else
          {
            free_help();	/* 004 */
            alert( READERR, 1 );
          }
        }
        else alert( ALMEMHLP, 1 );
      }
      else if( Fseek( 0L, hand, 0 ) == 0L )
      {
        buflen = 0;
        if( _read( hand, sizeof(help), &help ) != sizeof(help) ||
            help.id != 0x48322E30L ) alert( ALNOTHLP, 1 );
        else if( (tbl1 = (long *)xmalloc( l = help.tbl1_len + help.tbl2_len +
            help.caps_len + help.sens_len )) != 0 )
          if( _read( hand, l, tbl1 ) == l )
          {
            tbl2 = (long *)(help.tbl2_start - sizeof(help) + (long)tbl1);
            read_err=0;
            ret = 1;
          }
          else
          {
            free_help();	/* 004 */
            alert( READERR, 1 );
          }
        else alert( ALMEMHLP, 1 );
      }
      else alert( READERR, 1 );
    else alert( ALNOTHLP, 1 );
  }
  else alert( ALOPEN, 0 );
  if( dflt ) strcpy( pathend(path), temp );
  set_entries();
  if( !ret ) no_help(1);
  return ret;
}

void do_quit( int full )
{
  close_help();
  if( wind )
  {
    wind_close(wind);
    wind_delete(wind);
    wind=0;
  }
  if( full || _app )
  {
    if( vdi_hand )
    {
      no_fonts();
      v_clsvwk(vdi_hand);
    }
    rsrc_free();
    if( line ) x_mfree( line );
    appl_exit();
    exit(0);
  }
}

void gtext( int x, int y, char *t, int len, int rev )
{
  int arr[4];

  if( draw_mode&MODE_TEXT )
  {
    _vswr_mode( MD_REPLACE );
    if( is_scalable )
    {
      text_2_arr( t, &len );
      ftext16_mono( x, y, len );
    }
    else v_gtext( vdi_hand, x, y, t );
  }
  if( rev && draw_mode&MODE_SEL )
  {
    text_box( x, y, len, arr );
    _vsf_color( BLACK );
    _vswr_mode( MD_XOR );
    vr_recfl( vdi_hand, arr );
  }
}

int parms[96];

int parms_int( unsigned char *s, int n, int x, int y )
{
  int c, i, tc, add;
  static char nparm[] = { 4, 3, 4, -2, 4, 4, 4, 1, 1, 1, 1, 2, 1, 1, 1 },
            tocoord[] = { 2, 3, 4, -2, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0 },
              addxy[] = { 2, 2, 2, -2, 2, 2, 2 };

  if( *s >= sizeof(nparm) ) return 0;
  if( (c = nparm[*s]) < 0 ) c = n;
  if( *s < sizeof(addxy) )
    if( (add = addxy[*s]) < 0 ) add = n;
  if( (tc = tocoord[*s++]) < 0 ) tc = n;
  for( i=0; i<c; i++ )
  {
    parms[i] = *s++;
    if( i<tc )
      if( !(i&1) ) parms[i] = (i<add?x:0) + parms[i]*char_w + (char_w>>1);
      else parms[i] = (i<add?y:0) + parms[i]*char_h + (char_h>>1);
  }
  return c;
}

void parms_box(void)
{
  parms[2] += parms[0] - char_w - (char_w>>1);
  parms[3] += parms[1] - char_h - (char_h>>1);
}

int ufillcol, ufillint, ufillst, ulinecol, ulinstart, ulinend,
    ulinest, uwrite, uthick;	/* order is important */

void set_uvdi(void)
{
  _vsf_color( ufillcol );
  _vsf_interior( ufillint );
  _vsf_style( ufillst );
  _vsl_color( ulinecol );
  _vsl_ends( ulinstart, ulinend );
  _vsl_type( USERLINE );
  _vsl_udsty( ulinest );
  _vswr_mode( uwrite );
  _vsl_width( uthick );
}

void draw_img( int n )
{
  MFDB s;
  int cols[2];
  PICTURE *pic;

  pic = imgs[n];
  s.fd_addr = 0L;
  parms[6] = (parms[2] = pic->mfdb.fd_w - 1) + (parms[4] = parms[0] - (char_w>>1));
  parms[7] = (parms[3] = pic->mfdb.fd_h - 1) + (parms[5] = parms[1] - (char_h>>1));
  *(long *)&parms[0] = 0L;
  if( pic->mfdb.fd_nplanes>1 ) vro_cpyfm( vdi_hand, S_ONLY, parms, &pic->mfdb, &s );
  else
  {
    cols[0] = 1;
    cols[1] = 0;
    vrt_cpyfm( vdi_hand, MD_REPLACE, parms, &pic->mfdb, &s, cols );
  }
}

int cdecl draw_text( PARMBLK *pb )
{
  char *s, *t, dum, rev=0, und=0;
  int i, y, x, col, max, j;
  static int dflts[] = { BLACK, FIS_PATTERN, 8, BLACK,
      SQUARE, SQUARE, 0xFFFF, MD_REPLACE, 1 };
  
  _vs_clip( 1, (Rect *)&pb->pb_xc );
  if( (i = (pb->pb_yc - pb->pb_y)/char_h) < 0 ) i=0;
  if( (col = bl_rows) < set.rows[rez] ) col = set.rows[rez];
  if( (max = i + (pb->pb_hc+(char_h<<1)-1)/char_h) > col ) max = col;
  get_corner();
  if( draw_mode&MODE_TEXT && is_scalable ) _vst_charmap( 0 );
/*  _vswr_mode( MD_REPLACE );*/
  _vsf_color( BLACK );	/* 004 */
  _vsf_style( 0 );
  _vsf_interior( 1 );
  for( y=pb->pb_y+i*char_h; i<max; i++, y+=char_h )
  {
    s=t=(*line)[i]+1;
    x = pb->pb_x;
    col=0;
    rev = (i>sel_row || i==sel_row && sel_col<=0) &&
          (i<sele_row || i==sele_row && sele_col>0);
    while(*s)
    {
      while( *s && *s!=0x1d && *s!=0x1e &&		/* 004 */
          (rev || i!=sel_row || col!=sel_col) &&
          (!rev || i!=sele_row || col!=sele_col) )
      {
        s++;
        col++;
      }
      if( draw_mode&MODE_TEXT )
      {
        _vst_effects( und ? 8 : 0 );
        if( vplanes>1 ) _vst_color( und ? set.txt[rez] : BLACK );
      }
      dum = *s;
      *s = 0;
      if( *t ) gtext( x, y, t, s-t, rev );
      *s = dum;
      x += (s-t)*char_w;
      if( dum==0x1d )
        if( (und ^= 1) != 0 ) s+=3;
        else s++;
      else if( dum==0x1e )		/* 004 */
        s += 1 + *(unsigned char *)(s+1);
      else if( dum ) rev^=1;
      t = s;
    }
    if( (j=set.cols[rez]-col+corn_x) > 0 ) gtext( x, y,
        "                                 ", j, rev );
  }
  if( draw_mode&MODE_TEXT && is_scalable ) _vst_charmap( 1 );
  if( gr_rows )		/* 004 */
  {
    memcpy( &ufillcol, dflts, sizeof(dflts) );
    for( i=0; i<gr_rows; i++ )
    {
      s = (*line)[i+bl_rows]+1;
      while(*s)
      {
        while( *s && *s !='\x1e' ) s++;
        if( !*s ) break;
        j = *(unsigned char *)(++s) - 1;
        if( (x=parms_int( (unsigned char *)(++s), j, pb->pb_x, pb->pb_y )) > 0 )
            switch( *s )
      {
        case F_IMAGE:
          if( *(unsigned char *)(s+5) != 0xff )
              draw_img(*(unsigned char *)(s+5));
          break;
        case F_BUTTON:
          break;
        case F_BAR:
          parms_box();
          set_uvdi();
          v_bar( vdi_hand, parms );
          break;
        case F_LINE:
          set_uvdi();
          v_pline( vdi_hand, x>>1, parms );
          break;
        case F_RBOX:
          parms_box();
          set_uvdi();
          v_rbox( vdi_hand, parms );
          break;
        case F_RFBOX:
          parms_box();
          set_uvdi();
          v_rfbox( vdi_hand, parms );
          break;
        case F_RECFL:
          parms_box();
          set_uvdi();
          vr_recfl( vdi_hand, parms );
          break;
        case F_FILC:
          ufillcol = parms[0];
          break;
        case F_FILI:
          ufillint = parms[0];
          break;
        case F_FILST:
          ufillst = parms[0];
          break;
        case F_LINC:
          ulinecol = parms[0];
          break;
        case F_LINEND:
          ulinstart = parms[0];
          ulinend = parms[1];
          break;
        case F_LINST:
          ulinest = (parms[0]<<8) | parms[0];
          break;
        case F_WRITE:
          uwrite = parms[0];
          break;
        case F_LWIDTH:
          uthick = parms[0];
          break;
      }
        s += j;
        while( *s && *s != '\x1e' ) s++;
      }
    }
  }
  return pb->pb_currstate;
}

int is_sep( unsigned char c )
{
  return strchr( " ,.!?/*&%$\"\';:|[]{}\t\x1d\x1e", c ) != 0;	/* 004 */
}

int get_sel( int mode, char *out, int len, int full )
{
  static char *s, und, end;
  static int i, y, ymax;
  int olen;
  
  olen = 0;
  if( !mode )
  {
    und = end = 0;
    y = full ? 0 : sel_row;
    ymax = full ? bl_rows-1 : sele_row;
    i = 0;
    s = (*line)[y]+1;
  }
  if( end ) return 0;
  for(;;)
  {
    while( i<=(*line)[y][0] )
      if( *s=='\x1d' )
        if( (und ^= 1) != 0 ) s += 3;
        else s++;
      else if( *s=='\x1e' )		/* 004 */
          s += 1 + *(unsigned char *)(s+1);
      else
        if( full || y>sel_row || i>=sel_col )
        {
          if( !full && y==sele_row && i==sele_col )
          {
            *out = 0;
            end = 1;
            return olen;
          }
          *out++ = *s++;
          olen++;
          i++;
          if( !--len ) return olen;
        }
        else
        {
          i++;
          s++;
        }
    i = 0;
    s = (*line)[++y] + 1;
    if( y>ymax ) break;
    *out++ = '\r';
    *out++ = '\n';
    olen += 2;
    if( (len-=2) <= 0 ) return olen;
  }
  if( !full ) *out = 0;
  end = 1;
  return olen;
}

void get_coords( int *x, int *y )
{
  if( (*x-=blank[0].ob_x) < 0 ) *x = -1;
  else *x /= char_w;
  if( (*y-=blank[0].ob_y-1) < 0 ) *y = -1;
  else *y /= char_h;
}

int select_word( int x, int y, int sel )
{
  int i, ret, li;
  unsigned char *s, und, *last;
  
  get_coords( &x, &y );
  if( sel /*005*/ ) unselect();
  if( x<0 || y>=bl_rows ) return -1;
  for( i=li=0, und=0, s=last=(*line)[y]+1; *s; s++ )
    if( *s=='\x1e' )		/* 004 */
        s += 1 + *(unsigned char *)(s+1);
    else if( *s!='\x1d' )
    {
      if( !und && is_sep(*s) )
      {
        last=s;
        li = i;
      }
      if( !x-- ) break;
      i++;
    }
    else if( (und ^= 1) != 0 )
    {
      last = s;
      li = i;
      s += 2;
    }
  if( !*s || !und && is_sep(*s) ) return -1;
  if( *last++ == '\x1d' && und )
  {
    ret = (*last++<<8) | *last;
    if( !geneva_fmt )
    {
      ret = (ret&0x7fff)/2 - 2;
      if( !(ret&1) ) ret -= 4;
      ret >>= 2;
    }
    else if( ret==-1 ) ret = 0xfff;
  }
  else ret = -1;
  if( !sel ) return und;  /* 005 */
  sel_row = sele_row = y;
  if( und ) sel_col = li;
  else sel_col = li ? li+1 : 0;
  while( *s && (und || !is_sep(*s)) )
  {
    if( *s=='\x1d' ) break;
    i++;
    s++;
  }
  sele_col = i;
  draw_sel();
  return ret;
}

int still_on_word( int x0, int y0 )		/* 005 */
{
  int x, y, b, i, dum;

  i = select_word( x0, y0, 0 );
  graf_mkstate( &x, &y, &b, &dum );
  if( !(b&1) || i<=0 ) return i > 0;
  do
  {
    graf_mkstate( &x, &y, &b, &dum );
    if( select_word( x, y, 0 ) != i ) return 0;
  }
  while( b&1 );
  return 1;
}

void but_up(void)
{
  int b, dum;

  do
    graf_mkstate( &dum, &dum, &b, &dum );
  while( b&1 );
}

void calc_blank(void)
{
  int i;
  
  blank[0].ob_width = ((i=set.cols[rez])>bl_cols ? i : bl_cols)*char_w;
  blank[0].ob_height = ((i=set.rows[rez])>bl_rows ? i : bl_rows)*char_h;
  if( wind>0 )
  {
    wind_set( wind, X_WF_DIALWID, char_w );
    wind_set( wind, X_WF_DIALHT, char_h );
  }
}

void get_outer(void)
{
  int w, h;
  
  winner.g_x = (winner.g_x+(char_w>>1))/char_w*char_w;
  if( (w = (winner.g_w+(char_w>>1))/char_w) > LINELEN ) w = LINELEN;
  if( (h = (winner.g_h+(char_h>>1))/char_h) > lines ) h = lines;
  winner.g_w = w*char_w;
  winner.g_h = h*char_h;
  x_wind_calc( WC_BORDER, WIN_TYPE, X_MENU, winner.g_x, winner.g_y,
      winner.g_w, winner.g_h, &wsize.g_x, &wsize.g_y, &wsize.g_w,
      &wsize.g_h );
  if( wsize.g_w < min_wid ) winner.g_w += char_w;
  if( wsize.g_h < min_ht ) winner.g_h += char_h;
  set.cols[rez] = winner.g_w/char_w;
  set.rows[rez] = winner.g_h/char_h;
  set.xoff[rez] = winner.g_x/char_w;
  calc_blank();
  x_wind_calc( WC_BORDER, WIN_TYPE, X_MENU, winner.g_x, winner.g_y,
      winner.g_w, winner.g_h, &wsize.g_x, &wsize.g_y, &wsize.g_w,
      &wsize.g_h );
  set.yoff[rez] = wsize.g_y - max.g_y;
}

void get_inner(void)
{
  x_wind_calc( WC_WORK, WIN_TYPE, X_MENU, wsize.g_x, wsize.g_y,
      wsize.g_w, wsize.g_h, &winner.g_x, &winner.g_y, &winner.g_w,
      &winner.g_h );
  get_outer();
}

void read_string( char *s, int max, int size )
{
  char dummy;

  max--;
  if( size<=max )        /* the whole string fits */
      appl_read( apid, size, s );
  else
  {
    /* read what we can */
    appl_read( apid, max-1, s );
    s[max-1] = '*';     /* terminate the string */
    s[max] = 0;
    /* now, get the remaining bytes of the string */
    for( size=size-(max-1); size>0; size-- )
      appl_read( apid, 1, &dummy );
  }
}

unsigned char *img_tptr, *img_tmp;
long img_len;

long _Fseek( long offset, int seekmode )
{
  switch(seekmode)
  {
    case 0:
      img_tptr = img_tmp+offset;
      break;
    case 1:
      img_tptr += offset;
      break;
    case 2:
      img_tptr = img_tmp+img_len - offset;
  }
  if( img_tptr < img_tmp ) img_tptr = img_tmp;
  if( img_tptr > img_tmp+img_len ) img_tptr = img_tmp+img_len;
  return img_tptr-img_tmp;
}

long _Fread( long count, void *buf )
{
  long l;
        
  l = img_tmp+img_len-img_tptr;
  l = count < l ? count : l;
  memcpy( buf, img_tptr, l );
  img_tptr += l;
  return(l);
}

void put_img( unsigned char c )
{
  *img_tptr++ = c;
}

void find_img( unsigned char *s )	/* 004 */
{
  int j = *(s+1) - 4 - 2;
  unsigned char *t=0L, pl=0, p, *s0;
  long l;
  PICTURE *pic;
  
  s0 = s += 3 + 4;
  while( --j >= 0 )
  {
    if( (p = *(char *)&imgtbl[*s].offset) <= vplanes && p > pl )
    {
      pl = p;
      t = s;
    }
    else *s = 0xFF;
    s++;
  }
  if(t)
  {
    if( t!=s0 )
    {
      *s0 = *t;
      *t = 0xFF;
    }
    decode_func = put_img;
    get_len = (imgtbl[*s0+1].offset&0xFFFFFFL) - (l=imgtbl[*s0].offset&0xFFFFFFL);
    if( _seek( l, hand ) != l )
    {
      alert( READERR, 0 );
      *s0 = 0xff;
      return;
    }
    if( (img_tmp = img_tptr = xmalloc(img_len=imgtbl[*s0].unpacked)) == 0 )
    {
      alert( ALNOMEM, 0 );
      *s0 = 0xff;
      return;
    }
    read_err=0;
    Decode();
    buflen = 0L;	/* reset for next time */
    if( read_err ) read_err=0;
    else if( (pic = xmalloc(sizeof(PICTURE))) == 0 )
    {
      xmfree(img_tmp);
      alert( ALNOMEM, 0 );
      *s0 = 0xff;
      return;
    }
    else
    {
      if( xrealloc( (void **)&imgs, sizeof(PICTURE *)*(nimgs+1) ) )
        if( imgs )
        {
          xmfree( imgs );
          imgs = 0L;
        }
      img_tptr = img_tmp;
      if( !imgs || load_img(pic) ||
          fit_pic( pic, 1, -1, *(s0-2)*char_w, *(s0-1)*char_h ) ||
          transform_pic( pic, 1 ) )
      {
        xmfree(pic);
        xmfree(img_tmp);
        alert( ALNOMEM, 0 );
        *s0 = 0xff;
        return;
      }
      imgs[nimgs] = pic;
      *s0 = nimgs++;
      xmfree(img_tmp);
    }
  }
}

void new_screen( int draw, int *xy )
{
  int i, j;
  OBJECT *o;
  unsigned char *s, *p;

  /* clear the unused portion of screen */
  for( line_num=bl_rows+gr_rows/*004*/; line_num<lines; line_num++ )
  {
    (*line)[line_num][0] = 0;
    init_line();
  }
  /* 004: add buttons & images */
  *(long *)&blank[0].ob_head = -1L;
  if( gr_rows )
  {
    free_imgs();
    o=&blank[1];
    o->ob_height = char_h+2;
    if( char_h < sys_char_h ) o->ob_state |= X_SMALLTEXT;
    else o->ob_state &= ~X_SMALLTEXT;
    for( i=bl_rows, j=1; i<bl_rows+gr_rows && j<=NBUTS; i++ )
    {
      s = (*line)[i]+1;
      while( *s && *s!='\x1e' ) s++;
      while( *s )
      {
        if( *(s+2)==F_BUTTON )
        {
          memcpy( o, &blank[1], sizeof(OBJECT) );
          objc_add( blank, 0, j );
          p = s+3;
          o->ob_x = *p++ * char_w;
          o->ob_y = *p++ * char_h;
          o->ob_width = *p++ * char_w;
          p++;
          btn_jmp[j-1] = (*p++<<8) | *p++;
          o->ob_spec.free_string = p;
          while( *p && *p!='\x1d' ) p++;
          *p = 0;
          o++;
          j++;
        }
        else if( *(s+2)==F_IMAGE ) find_img(s);
        s += *(s+1) + 1;
        while( *s && *s!='\x1e' ) s++;
      }
    }
  }
  if( wind>0 && draw )
  {
    calc_blank();
    blank[0].ob_x = xy ? xy[0]*char_w + winner.g_x : winner.g_x;
    blank[0].ob_y = xy ? xy[1]*char_h + winner.g_y : winner.g_y;
    wind_set( wind, X_WF_DIALOG, blank );
  }
}

int open_wind(void)
{
  int i, dum;
  
  if( wind<=0 )
    if( (i=x_wind_create( WIN_TYPE, X_MENU, wsize.g_x,
        wsize.g_y, wsize.g_w, wsize.g_h )) > 0 )
    {
      blank[0].ob_x = winner.g_x;
      blank[0].ob_y = winner.g_y;
      unselect();
      wind = i;
      calc_blank();
      wind_set( i, X_WF_DIALOG, blank );
      wind_set( i, X_WF_MENU, menu );
      set_name(0L);
      wind_get( i, X_WF_MINMAX, &min_wid, &min_ht, &dum, &dum );
      clear_info(1);
      wind_open( i, wsize.g_x, wsize.g_y, wsize.g_w, wsize.g_h );
    }
    else
    {
      alert( ALWIND, 1 );
      return 0;
    }
  return 1;
}

void resize( GRECT *g )
{
  wind_set( wind, WF_CURRXYWH, g->g_x, g->g_y, g->g_w, g->g_h );
}

void new_size(void)
{
  get_inner();
  resize( &wsize );
  if( nimgs )	/* 004: re-read whole screen if it has IMG's */
      list_tbl( tbl1+jumps[prev_jump(jnum)] );
  new_screen( 1, 0L );	/* 004: moved here */
}

void get_font(void)
{
  if( font_sel( WINDOPTS, SYSTEM, &set.font[rez], &set.point[rez] ) )
      new_size();	/* 004: removed new_screen */
}

void initialize(void)
{
  int i;

  init_font( &set.font[rez], &set.point[rez] );
  _vst_alignment( 0, 5 );
  winner.g_x = set.xoff[rez]<<3;
  winner.g_w = char_w*set.cols[rez];
  winner.g_h = char_h*set.rows[rez];
  i = set.yoff[rez];
  get_outer();
  wsize.g_y = max.g_y + i;
  get_inner();
  if( wsize.g_w > max.g_w )
  {
    wsize.g_w = max.g_w;
    get_inner();
  }
  if( wsize.g_h > max.g_h )
  {
    wsize.g_h = max.g_h;
    get_inner();
  }
}

void save_settings(void)
{
  int ret, i;
  char temp[100];
  
  graf_mouse( BUSYBEE, 0L );
  if( x_shel_put( X_SHOPEN, "Geneva Help Viewer" ) > 0 )
  {
    x_sprintf( temp, "%v", set.ver );
    ret = x_shel_put( X_SHACCESS, temp );
    for( i=0; i<3 && ret>0; i++ )
    {
      x_sprintf( temp, "%d %d %d %d %d %d %d", set.font[i], set.point[i],
          set.rows[i], set.cols[i], set.xoff[i], set.yoff[i], set.txt[i] );
      ret = x_shel_put( X_SHACCESS, temp );
    }
    x_sprintf( temp, "%S %b %b", set.topic, set.match, set.all );
    if( ret>0 ) ret = x_shel_put( X_SHACCESS, temp );
    if( ret>0 ) ret = x_shel_put( X_SHACCESS, set.hlp_path );
    if( ret>0 ) x_shel_put( X_SHCLOSE, 0L );
  }
  graf_mouse( ARROW, 0L );
}

int load_settings(void)
{
  int ver, i, ret=1;
  char temp[100];
  struct Set s;
  
  graf_mouse( BUSYBEE, 0L );
  while( (i=x_shel_get( X_SHOPEN, 0, "Geneva Help Viewer" )) == -1 );
  if( i > 0 && x_shel_get( X_SHACCESS, sizeof(temp)-1, temp ) > 0 )
  {
    x_sscanf( temp, "%v", &ver );
    if( ver != HELP_SET_VER )
    {
      alert( ALCFGVER, 0 );
      graf_mouse( ARROW, 0L );
      return 0;
    }
    s.ver = HELP_SET_VER;
    for( i=0; i<3 && ret>0; i++ )
      if( (ret = x_shel_get( X_SHACCESS, sizeof(temp)-1, temp )) > 0 )
        x_sscanf( temp, "%d %d %d %d %d %d %d", &s.font[i], &s.point[i],
            &s.rows[i], &s.cols[i], &s.xoff[i], &s.yoff[i], &s.txt[i] );
    if( ret>0 && (ret = x_shel_get( X_SHACCESS, sizeof(temp)-1, temp )) > 0 )
        x_sscanf( temp, "%S %b %b", s.topic, &s.match, &s.all );
    if( ret>0 ) ret = x_shel_get( X_SHACCESS, sizeof(s.hlp_path), s.hlp_path );
    if( ret>0 )
    {
      x_shel_get( X_SHCLOSE, 0, 0L );
      memcpy( &set, &s, sizeof(set) );
      if( wind>0 )
      {
        initialize();
        new_size();	/* 004: removed new_screen */
      }
      graf_mouse( ARROW, 0L );
      return 1;
    }
    else alert( ALCFGERR, 0 );
  }
  graf_mouse( ARROW, 0L );
  return 0;
}

void sel_region( int x, int y, int ex, int ey )
{
  if( x==sel_col && y==sel_row )
  {
    if( ey==sele_row && ex==sele_col ) return;
    else if( ey<sele_row || ey==sele_row && ex<sele_col )
    { /* un-xor the tail end */
      sel_col = ex;
      sel_row = ey;
    }
    else
    { /* add the tail end */
      sel_col = sele_col;
      sel_row = sele_row;
      sele_col = ex;
      sele_row = ey;
    }
    draw_sel();
    sel_col = x;
    sel_row = y;
    sele_col = ex;
    sele_row = ey;
  }
  else if( ex==sele_col && ey==sele_row )
  {
    if( y<sel_row || y==sel_row && x<sel_col )
    { /* add the tail end */
      sele_col = sel_col;
      sele_row = sel_row;
      sel_col = x;
      sel_row = y;
    }
    else
    { /* un-xor the tail end */
      sele_col = x;
      sele_row = y;
    }
    draw_sel();
    sel_col = x;
    sel_row = y;
    sele_col = ex;
    sele_row = ey;
  }
  else
  {
    sel_col = x;
    sel_row = y;
    sele_col = ex;
    sele_row = ey;
    draw_sel();
  }
}

void get_corner(void)
{
  corn_x = winner.g_x;
  corn_y = winner.g_y;
  get_coords( &corn_x, &corn_y );
  if( corn_x<0 ) corn_x = 0;
  if( corn_y<0 ) corn_y = 0;
}

void scroll( int dir )
{
  int msg[8];
  
  msg[0] = WM_ARROWED;
  msg[3] = wind;
  msg[4] = dir;
  shel_write( SHW_SENDTOAES, 0, 0, (char *)msg, 0L );
  get_corner();
}

void unsel( int x1, int y1, int x2, int y2 )
{
  int t1, t2, t3, t4;
  
  t1 = sel_col;
  t2 = sel_row;
  t3 = sele_col;
  t4 = sele_row;
  sel_col = x1;
  sel_row = y1;
  sele_col = x2;
  sele_row = y2;
  draw_sel();
  sel_col = t1;
  sel_row = t2;
  sele_col = t3;
  sele_row = t4;
}

void sel_mouse( int x, int y, int b, int k )
{
  int first=1, shift, anc_x, anc_y, dir=0, xmax, ymax, dum;
  
  if( (xmax = set.cols[rez]) < bl_cols ) xmax = bl_cols;
  if( (ymax = set.rows[rez]) < bl_rows ) ymax = bl_rows;
  for(;;)
  {
    if( !first ) graf_mkstate( &x, &y, &b, &k );
    shift = k&3;
    get_corner();
    if( !x )
    {	/* mouse at left edge of screen: special case */
      x = corn_x-1;
      get_coords( &dum, &y );
    }
    else get_coords( &x, &y );
    if( !(b&1) ) break;
    if( x-corn_x>set.cols[rez] )
    {
      scroll( shift ? WA_RTPAGE : WA_RTLINE );
      if( (x = corn_x+set.cols[rez]) > xmax ) x = xmax;
    }
    else if( x<corn_x )
    {
      scroll( shift ? WA_LFPAGE : WA_LFLINE );
      x = corn_x;
    }
    if( y-corn_y>=set.rows[rez] )
    {
      scroll( shift ? WA_DNPAGE : WA_DNLINE );
      if( (y = corn_y+set.rows[rez]) >= ymax ) y = ymax-1;
      x = xmax;
    }
    else if( y<corn_y )
    {
      scroll( shift ? WA_UPPAGE : WA_UPLINE );
/*      if( (y = corn_y-1) < 0 ) corn_y = 0; */
      y = corn_y;
      x = -1;
    }
    if( first )
    {
      graf_mouse( TEXT_CRSR, 0L );
      if( shift && sel_row!=-99 )
        if( y<sel_row || y==sel_row && x<sel_col ) sel_region( x, y, anc_x=sel_col, anc_y=sel_row );
        else sel_region( anc_x=sel_col, anc_y=sel_row, x, y );
      else
      {
        anc_x = x;
        anc_y = y;
        unselect();
      }
    }
    else if( y<anc_y || y==anc_y && x<anc_x )
    {
      if( dir>0 ) unsel( anc_x, anc_y, sele_col, sele_row );
      sel_region( x, y, anc_x, anc_y );
      dir = -1;
    }
    else if( y>anc_y || y==anc_y && x>anc_x )
    {
      if( dir<0 ) unsel( sel_col, sel_row, anc_x, anc_y );
      sel_region( anc_x, anc_y, x, y );
      dir = 1;
    }
    first=0;
  }
  if( first )
    if( shift && sel_row!=-99 )
    {
      if( y<sel_row || y==sel_row && x<sel_col ) sel_region( x, y, sele_col, sele_row );
      else if( y>sel_row || y==sel_row && x>sel_col ) sel_region( sel_col, sel_row, x, y );
    }
    else unselect();
  graf_mouse( ARROW, 0L );
}

/**********
int get_clip( char *out )
{
  long map;
  int ret;
  char ok=0;
  DTA dta, *old;

  old = Fgetdta();
  Fsetdta(&dta);
  scrp_read(out);
  map = Drvmap();
  if( *out && *(out+1)==':' && (map & (*out&=0x5f)-'A')!=0 )
    if( Fsfirst(out,FA_SUBDIR) )
    {
      *pathend(out) = 0;
      if( !Fsfirst(out,FA_SUBDIR) ) ok=1;
    }
    else if( *pathend(out) )
    {
      strcat(out,"\\");
      ok = 1;
    }
  if( !ok )
  {
    out[0] = (map&(1<<2)) ? 'C' : 'A';
    strcpy( out+1, ":\\CLIPBRD" );
    if( Fsfirst(out,0x37) && Dcreate( out ) )
    {
      alert( BADCLIP, 0 );
      Fsetdta(old);
      return 0;
    }
    strcat( out, "\\" );
  }
  scrp_write(out);
  strcat( out, "SCRAP.*" );
  ret = Fsfirst( out, 0x23 );
  while( !ret )
  {
    strcpy( pathend(out), dta.d_fname );
    Fdelete( out );
    ret = Fsnext();
  }
  strcpy( pathend(out), "SCRAP.TXT" );
  Fsetdta(old);
  return 1;
} **********/

void save_txt( int full, int clip )
{
  int h, i, l;
  char temp[120+13];
  
  if( clip || select( SAVEPATH, savepth, sizeof(savepth), savename ) )
  {
    if( clip )
    {
      if( !x_scrp_get( temp, 1 ) ) return;
      strcat( temp, "SCRAP.TXT" );
    }
    else
    {
      strcpy( temp, savepth );
      strcpy( pathend(temp), savename );
    }
    if( (h=Fcreate(temp,0)) < 0 ) alert( NOCREAT, 0 );
    else
    {
      i = 0;
      while( (l=get_sel( i, temp, sizeof(temp)-1, full )) > 0 )
      {
        if( Fwrite( h, l, temp ) != l )
        {
          alert( NOWRITE, 0 );
          break;
        }
        i = 1;
      }
      Fclose(h);
    }
  }
}

void do_iconify( int buf[] )
{
  iconified = 1;
  wind_set( wind, X_WF_DIALOG, 0L );
  wind_set( wind, WF_ICONIFY, buf[4], buf[5], buf[6], buf[7] );
  /* get working area of iconified window */
  wind_get( wind, WF_WORKXYWH, &icon[0].ob_x, &icon[0].ob_y,
      &icon[0].ob_width, &icon[0].ob_height );
  /* center the icon within the form */
  icon[1].ob_x = (icon[0].ob_width - icon[1].ob_width) >> 1;
  icon[1].ob_y = (icon[0].ob_height - icon[1].ob_height) >> 1;
  /* new (buttonless) dialog in main window */
  wind_set( wind, X_WF_DIALOG, icon );
}

void do_uniconify( int buf[] )
{
  iconified = 0;
  /* turn dialog off so that icon's size and position will not change */
  wind_set( wind, X_WF_DIALOG, 0L );
  wind_set( wind, WF_UNICONIFY, buf[4], buf[5], buf[6], buf[7] );
  /* restore old dialog */
  wind_set( wind, X_WF_DIALOG, blank );
}

void send_uniconify(void)	/* 005 */
{
  int buf[8];

  wind_get( wind, WF_UNICONIFY, &buf[4], &buf[5], &buf[6], &buf[7] );
  do_uniconify(buf);
}

void get_planes(void)
{
  int ex[57];

  vq_extnd( vdi_hand, 1, ex );
  vplanes = ex[4];
}

int main( int argc, char *argv[] )
{
  int dum, i, event;
  char fulled=0, hlpfile[80], topic[21], **p;
  long cookie, vid;
  EVENT ev;

  ev.ev_mflags = MU_MESAG|X_MU_DIALOG|MU_KEYBD;
  /* default item selector path */
  path[0] = Dgetdrv()+'A';
  path[1] = ':';
  Dgetpath( path+2, 0 );
  strcpy( savepth, path );
  strcat( path, "\\*.HLP" );
  strcpy( set.hlp_path, path );		/* default aux help path */
  strcat( savepth, "\\*.TXT" );
  strcpy( savename, "HELP.TXT" );
  apid = appl_init();
  /* Tell the AES (or Geneva) that we understand AP_TERM message */
  shel_write( SHW_MSGTYPE, 1, 0, 0L, 0L );
  shel_write( SH_REGHELP, 0, 0, 0L, 0L );	/* 005 */
  if( !alloc_lines() )
  {
    form_error(8);
    goto bad;
  }
  if( rsrc_load("gnvahelp.rsc") )
  {
    /* Check to make sure Geneva is active */
    if( CJar( 0, GENEVA_COOKIE, &cookie ) == CJar_OK && cookie )
    {
      vdi_hand = graf_handle( &dum, &sys_char_h, &dum, &dum );	/* 004 */
      work_in[0] = Getrez() + 2;
      vdi_reset();
      v_opnvwk( work_in, &vdi_hand, work_out );
      if( vdi_hand )
      {
        xmax = work_out[0]+1;
        ymax = work_out[1]+1;
        xasp = work_out[3];
        yasp = work_out[4];
        colvals = work_out[39];
        get_planes();
        if( colvals<=512 )
        {
          colmax = 0x777;
          colmax1 = 7;
        }
        else
        {
          colmax = 0xFFF;
          colmax1 = 0xF;
        }
        if( CJar( 0, _VDO_COOKIE, &vid ) != CJar_OK ) vid = 0L;
        else vid = (char)(vid>>16L);
        TT_vid = vid==2;
        falc_vid = vid==3;
        wind_get( 0, WF_WORKXYWH, &max.g_x, &max.g_y, &max.g_w, &max.g_h );
        rez = 0;
        if( max.g_w > 320 )
        {
          rez = 1;
          if( max.g_h > 200-11 ) rez = 2;
        }
        load_settings();
        no_help(0);	/* 004: VDI setup was here */
        rsrc_gaddr( 0, HICON, &icon );
        rsrc_gaddr( 0, HMENU, &menu );
        menu[0].ob_state |= X_MAGIC;
        initialize();
      }
      else
      {
        alert( ALVDI, 0 );
        goto bad;
      }
      /* set the correct name in the menu */
      rsrc_gaddr( 15, TITLE, &title );
      menu_register( apid, *title );
      set_entries();
      if( _app )          /* running as PRG */
      {
        graf_mouse( ARROW, 0L );
        open_wind();
      }
      if( argc>1 )
      {
        strcpy( hlpfile, argv[1] );
        shel_find( hlpfile );
        if( open_help(hlpfile) )
          if( argc>2 ) find( argv[2], 0, 1, 1 );
          else do_index();
      }
      for(;;)
      {
        event = EvntMulti( &ev );
        if( event&MU_MESAG ) switch( ev.ev_mmgpbuf[0] )
        {
          case AC_OPEN:
            if( wind<=0 )
            {
              open_wind();
              break;
            }
            ev.ev_mmgpbuf[3] = wind;      /* fall through if already open */
          case WM_TOPPED:
            wind_set( ev.ev_mmgpbuf[3], WF_TOP );
            break;
          case WM_FULLED:
            if( !fulled ) *(GRECT *)&ev.ev_mmgpbuf[4] = max;
            else wind_get( wind, WF_PREVXYWH, &ev.ev_mmgpbuf[4], &ev.ev_mmgpbuf[5],
                &ev.ev_mmgpbuf[6], &ev.ev_mmgpbuf[7] );
            fulled ^= 1;
          case WM_MOVED:
          case WM_SIZED:
            if( !iconified )
            {
              wsize = *(GRECT *)&ev.ev_mmgpbuf[4];
              new_size();
            }
            else resize( (GRECT *)&ev.ev_mmgpbuf[4] );
            break;
          case AP_TERM:
            do_quit(1);
            break;
          case WM_CLOSED:
            do_quit(0);
            break;
          case AC_CLOSE:
            wind = 0;
            iconified = 0;
            ev.ev_mflags |= X_MU_DIALOG;
            break;
          case WM_ICONIFY:
          case WM_ALLICONIFY:
            if( ev.ev_mmgpbuf[3]==wind && !iconified ) /* main window */
            {
              do_iconify(ev.ev_mmgpbuf);
              ev.ev_mflags &= ~X_MU_DIALOG;
            }
            break;
          case WM_UNICONIFY:
            if( ev.ev_mmgpbuf[3]==wind && iconified ) /* main window */
            {
              do_uniconify(ev.ev_mmgpbuf);
              ev.ev_mflags |= X_MU_DIALOG;
            }
            break;
          case X_GET_HELP:
            read_string( topic, sizeof(topic), ev.ev_mmgpbuf[3] );
            read_string( hlpfile, sizeof(hlpfile), ev.ev_mmgpbuf[4] );
            if( open_wind() )
            {
              if( iconified )		/* 005 */
              {
                send_uniconify();
                ev.ev_mflags |= X_MU_DIALOG;
              }
              shel_find( hlpfile );
              if( open_help(hlpfile) )
              {
                strcpy( path, hlpfile );
                strcpy( pathend(path), "*.HLP" );
                strcpy( fname, pathend(path) );
                wind_set( wind, WF_TOP );
                find( topic, ev.ev_mmgpbuf[5], 1, 0 );
              }
            }
            break;
          case X_MN_SELECTED:
            if( !iconified ) switch( ev.ev_mmgpbuf[4] )
            {
              case MHELP:
                if( !select( HELPTITL, path, sizeof(path), fname ) || !open_help(0L) ) break;
              case MINDEX:
                do_index();
                break;
              case MQUIT:
                do_quit(0);
                break;
              case MFONT:
                get_font();
                break;
              case MFIND:
                rsrc_gaddr( 0, FIND, &form );
                set_if( FMATCH, set.match );
                set_if( FALL, set.all );
                if( make_form( FIND, FTOPIC, set.topic, 0L ) == FOK )
                    find( set.topic, set.match=form[FMATCH].ob_state&SELECTED,
                        0, set.all=form[FALL].ob_state&SELECTED );
                break;
              case MUSING:
                strcpy( hlpfile, "GNVAHELP.HLP" );
                shel_find( hlpfile );
                if( open_help(hlpfile) )
                {
                  rsrc_gaddr( 15, HELPSTR, &p );
                  find( *p, 0, 1, 0 );
                }
                break;
              case MPREV:
                jump_tbl( -1, 0L );
                break;
              case MSAVE:
                save_settings();
                break;
              case MLOAD:
                load_settings();
                break;
              case MHPATH:
                topic[0] = 0;
                strcpy( hlpfile, set.hlp_path );
                strcpy( pathend(hlpfile), "*.HLP" );
                if( select( HELPPATH, hlpfile, sizeof(set.hlp_path), topic ) )
                {
                  strcpy( set.hlp_path, hlpfile );
                  strcpy( pathend(set.hlp_path), "*.HLP" );
                }
                break;
              case MBLOCK:
                save_txt( 0, 0 );
                break;
              case MTEXT:
                save_txt( 1, 0 );
                break;
              case MCLIP:
                save_txt( 0, 1 );
                break;
            }
            menu_tnormal( menu, ev.ev_mmgpbuf[3], 1 );
        }
        if( event & MU_KEYBD )
        {
          i = ev.ev_mmokstate&3;
          switch( ev.ev_mkreturn&0xFF00 )
          {
            case 0x5000:
              scroll( i ? WA_DNPAGE : WA_DNLINE );
              break;
            case 0x4800:
              scroll( i ? WA_UPPAGE : WA_UPLINE );
              break;
            case 0x4B00:
              scroll( i ? WA_LFPAGE : WA_LFLINE );
              break;
            case 0x4D00:
              scroll( i ? WA_RTPAGE : WA_RTLINE );
              break;
          }
        }
        if( event&X_MU_DIALOG )
        {
          i = ev.ev_mmgpbuf[2]&0x7fff;
          if( ev.ev_mmgpbuf[3]==wind )
            if(i)	/* 004 */
            {
              x_wdial_change( wind, i, blank[i].ob_state & ~SELECTED );
              strcpy( hlpfile, blank[i].ob_spec.free_string );
              i = btn_jmp[i-1];
              if( i==-1 ) find( hlpfile, 0, 0, 1 );
              else jump_tbl( i, hlpfile );
            }
            else if( ev.ev_mmgpbuf[2]&0x8000 ||
                still_on_word( ev.ev_mmox, ev.ev_mmoy ) /* 005 */ )
            {
              i = select_word( ev.ev_mmox, ev.ev_mmoy, 1 );
              but_up();
              if( i>=0 )
              {
                get_sel( 0, hlpfile, sizeof(hlpfile), 0 );
                unselect();
                if( i==0xfff ) find( hlpfile, 0, 0, 1 );
                else jump_tbl( i, hlpfile );
              }
            }
            else sel_mouse( ev.ev_mmox, ev.ev_mmoy, ev.ev_mmobutton, ev.ev_mmokstate );
          set_entries();
        }
      }
    }
    else if( _app ) alert( ALNOGEN, 0 );
  }
  else form_alert( 1, "[1][|GNVAHELP.RSC not found!][OK]" );
bad:
  if( _app || _GemParBlk.global[1]==-1 ) do_quit(1);
  for(;;)
  {
    evnt_mesag(ev.ev_mmgpbuf);
    if( ev.ev_mmgpbuf[0]==AP_TERM ) do_quit(1);
  }
}
