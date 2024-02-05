#include "new_aes.h"
#include "vdi.h"
#include "xwind.h"
#include "tos.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "mwclinea.h"
#include "multevnt.h"
#include "gnva_tos.h"
typedef struct { int x, y, w, h; } Rect;
#include "graphics.h"
#include "font_sel.h"

#define GNVATOS_VER	GENEVA_VER
#define TOSSET_VER	0x100
#define WIN_TYPE0 NAME|MOVER|CLOSER|FULLER|SIZER|UPARROW|DNARROW|VSLIDE|LFARROW|RTARROW|HSLIDE|SMALLER

#define CMDBUFLEN 256

#define MODE_ALL -1
#define MODE_TEXT 1
#define MODE_SEL  2
#define MODE_EVERY 4

#define REDIR_APND 1
#define REDIR_OVER 2

char draw_mode=-1, have_traps, has_mint,
     in_prog, in_echo, *tail, iconified,
     *in_t2,		/* 004 */
     my_name[9],
     redir_name[13], iredir_name[13], prog_name[13], savename[13],
     redir_path[120], iredir_path[120], prog_path[120], prog_tail[126],
     savepth[120],
     is_io, paused, child_term, redir_mode,
     *program,
     *screen,           /* start of screen buffer */
     *scrn_ptr;         /* start of first line of visible screen */
int corn_x, corn_y, wind, min_wid, min_ht, scrn_wid, scrn_line, line, child_id,
    rstart, rend, apid, sel_row, sel_col, sele_row, sele_col, child_ret, pipe[2], trun_fd;
Rect clip_rect;
long screen_size;

long bcon_nul;		/* 004: was unsigned */
char last_esc, inverse, discard, ctrlc, gt_ign_cr, ign_ctrlc, *cmdptr,
    cmdbuf[CMDBUFLEN+2], curs_is_on, curs_hide, old_show, show_curs=1, is_top,
    ichar_cr, insert, out_buf[81], *out_ptr=&out_buf[0], single;
int row, col, saver, savec, redir_hand=2, iredir_hand=2,
    cmdbufmax, lastcol, lastrow, oldrow, evkey, rez;
int old_celht, /*b_old_show, b_old_hide,*/ old_mx, old_my;
int work_in[] = { 1, 7, 1, 1, 1, 1, 1, 1, 1, 1, 2 };
int oarray[4];
IOREC *io;
GRECT winner, max, wsize;
int cdecl draw_text( PARMBLK *pb );
USERBLK ub_main = { draw_text };
OBJECT blank[] = { -1, -1, -1, G_USERDEF, TOUCHEXIT, 0, (OBSPEC)(long)&ub_main },
    *menu, *icon;
G_COOKIE *cookie;
extern void t1(), t13();
extern void (*t1adr)(), (*t13adr)();
extern char in_a7;

int write_redir( char *string );
long bconin( int hand );
void doecho( char *ptr );
extern linea0(void);
void get_inner(void);
void get_corner(void);
void flush_out(void);
int gtext( char *string );
void draw_current( int in_io );
void draw_range( int start, int end, int in_io );
void breakpoint(void);
void do_quit(void);
long setexc( int number, void (*newvec)(), void (**oldvec)(), int force );
void unselect( int draw );
void v_ftext16_mono( int handle, int x, int y, int *wstr, int strlen, int offset );

struct Set
{
  int ver;
  int win_type;
  union
  {
    struct
    {
      unsigned use_hot:1;
      unsigned set_la:1;
      unsigned termcap:1;
      unsigned use_argv:1;
      unsigned no_pause:1;
    } s;
    int i;
  }
  options;
  int font[3], point[3];
  int rows[3], cols[3];
  int xoff[3], yoff[3];
  int linelen[3], lines[3];
} set = { TOSSET_VER, WIN_TYPE0, { {1, 1, 1, 0, 0} },  1, 1, 1,  8, 9, 10,
          18, 18, 20,  45, 75, 75,  0, 0, 0,  0, 0, 0,  80, 80, 80,  100, 100, 100 };

#define MiNT_COOKIE 0x4d694e54L
#define CJar_xbios      0x434A          /* "CJ" */
#define CJar_OK         0x6172          /* "ar" */
#define CJar(mode,cookie,value)         (int)xbios(CJar_xbios,mode,cookie,value)

void set_lacurs(void)
{
  if( set.options.s.set_la )
  {
    V_CUR_CX = col;
    V_CUR_CY = row;
  }
}

void to_arr( Rect *r )
{
  oarray[2] = (oarray[0] = r->x) + r->w - 1;
  oarray[3] = (oarray[1] = r->y) + r->h - 1;
}

void vdi_box( int intcol, int writ, Rect *r )
{
  _v_mouse(0);
  _vsf_color( intcol );
  _vswr_mode( writ );
  if( r )
  {
    to_arr(r);
    vr_recfl( vdi_hand, oarray );
  }
  _v_mouse(1);
}

void blank_box( Rect *r, int mode )
{
  vdi_box( mode==MD_REPLACE ? 0 : 1, mode, r );
}

void inc_scrn( char **ptr )
{
  if( (*ptr += scrn_wid) > screen+screen_size-scrn_wid )
      *ptr = screen;
}

char *get_scr( int row, int col )
{
  int l;

  if( (l = (scrn_ptr-screen) / scrn_wid + scrn_line/*004*/ + row) >= set.lines[rez] ) l -= set.lines[rez];
  return( screen+(long)l*scrn_wid+col );
}

char *get_scr2( int row, int col )	/* 004 */
{
  return get_scr( row-scrn_line, col );
}

void clear_screen(void)
{
  int i, j;
  char *ptr;

  row=col=line=corn_y=scrn_line=0;	/* 004: added corn_y, scrn_line */
  scrn_ptr = screen;
  scrn_wid = ((set.linelen[rez]+7)>>3) + set.linelen[rez];
  for( i=-1, ptr=scrn_ptr; ++i<set.lines[rez]; )
  {
    memset( ptr, ' ', j=set.linelen[rez] );
    memset( ptr+j, 0, scrn_wid-j );
    ptr += scrn_wid;
  }
  unselect(0);
}

void erase_lines( int slin, int elin )
{
  int i, j;
  char *ptr;
  Rect r;

  if( slin <= elin )
  {
    for( i=slin-1; ++i<=elin; )
    {
      for( ptr=get_scr( i, 0 ), j=-1; ++j<set.linelen[rez]; )
        *ptr++ = ' ';
      while( j++<scrn_wid ) *ptr++ = '\0';
    }
    if( !iconified )
      if( is_top )
      {
        r.x = winner.g_x;
        r.y = blank[0].ob_y + (slin+scrn_line)*char_h;
/* 004        r.y = winner.g_y + slin*char_h; */
        r.w = winner.g_w;
        r.h = (elin-slin+1)*char_h;
        blank_box( &r, MD_REPLACE );
      }
      else draw_range( slin, elin, in_prog );
  }
}

void blit_lines( int froms, int frome, int tos, int erase )
{
  MFDB mfdb = { 0L };
  int i, px[8];

  if( froms <= frome )
  {
    if( !iconified )
    {
      px[6] = px[2] = (px[0] = px[4] = winner.g_x) + winner.g_w - 1;
      px[7] = px[3] = (px[1] = px[5] = winner.g_y) + (frome-froms+1)*char_h - 1;
      px[1] += (i=char_h*froms);
      px[3] += i;
      px[5] += (i=char_h*tos);
      px[7] += i;
      _v_mouse(0);
      vro_cpyfm( vdi_hand, 3, px, &mfdb, &mfdb );
      _v_mouse(1);
    }
    if( erase )
    {
      i = tos<froms ? frome : froms;
      erase_lines( i, i );
    }
  }
}

void erase_ln( int scol, int ecol )
{
  Rect r;
  int i;
  char *ptr, *ptr0;

  ptr0 = (ptr = get_scr( row, 0 )) + set.linelen[rez]-1 + 1;
  for( ptr+=(i=scol); i<=ecol; i++ )
  {
    *ptr++ = ' ';
    *(ptr0+(i>>3)) &= ~(1<<(i&7));
  }
  if( !iconified )
    if( is_top )
    {
      r.x = blank[0].ob_x + scol*char_w;
      r.y = blank[0].ob_y + (row+scrn_line)*char_h;
/* 004     r.x = winner.g_x + scol*char_w;
      r.y = winner.g_y + row*char_h; */
      r.w = (ecol+1)*char_w;
      r.h = char_h;	/* 004: was -1 */
      blank_box( &r, MD_REPLACE );
    }
    else draw_current(in_prog);
}

void yield(void)
{
  do
  {
    is_io = 1;
    appl_yield();
  }
  while( !single && paused<0 );
  get_corner();
}

int check_io( int func( char *ptr ) )
{
  unsigned int h, t;
  int i;

  if( (h=io->ibufhd) != (t=io->ibuftl) )
  {
    if( (h+=4) >= io->ibufsiz ) h = 0;
    if( t >= io->ibufsiz ) t = 0;
    if( (i = (*func)( (char *)io->ibuf+h )) != 0 )
    {
      io->ibufhd = h;
      return(i);
    }
    if( (i = (*func)( (char *)io->ibuf+t )) != 0 )
    {
      io->ibufhd = io->ibuftl;
      return(i);
    }
  }
  return(0);
}

int is_cqcs( char *ptr )
{
  if( ign_ctrlc ) return(0);
  switch( *(ptr+3) )
  {
    case '\023':
      return(1);
    case '\021':
      return(2);
    case '\03':
      return(3);
    default:
      return(0);
  }
}

void vdi_cursor( int force_off )
{
  char new_state;
  Rect r;

  if( show_curs && is_top && !iconified )
  {
    if( !force_off && col==lastcol && line==lastrow ) new_state = 1;
    else if( (new_state = !force_off) != 0 ) curs_is_on=0;
    if( new_state != curs_is_on )
    {
      curs_is_on = new_state;
      r.x = blank[0].ob_x+col*char_w;
      r.w = char_w;
      r.y = blank[0].ob_y+line*char_h;
      r.h = char_h;
      blank_box( &r, MD_XOR );
      lastcol = col;
      lastrow = line;
    }
  }
}

void vcurs_off(void)
{
  void vcur( int i );

  if( !curs_hide++ )
    if( (old_show = show_curs) != 0 )
    {
      vdi_cursor(1);
      show_curs = 0;
    }
}

void vcurs_on( int force )
{
  void vcur( int i );

  if( force || !--curs_hide )
  {
    if( (show_curs = force ? 1 : old_show) != 0 ) vdi_cursor(0);
    curs_hide = 0;
  }
  else if( curs_hide<0 ) curs_hide = 0;
}

void scroll( int dir )
{
  int msg[8];

  if( !iconified )
  {
    msg[0] = WM_ARROWED;
    msg[3] = wind;
    msg[4] = dir;
    shel_write( SHW_SENDTOAES, 0, 0, (char *)msg, 0L );
  }
  else if( dir==WA_DNLINE ) blank[0].ob_y -= char_h;
}

void set_dial( int io )
{
  if( !iconified )
  {
    is_io = io;
    wind_update(BEG_UPDATE);
    is_io = io;
    wind_set( wind, X_WF_DIALOG, blank );
    is_io = io;
    wind_update(END_UPDATE);
  }
}

void newline(void)
{
  char c;
  int r, r2;
  long l;

  if( ctrlc ) return;
/*  if( redir_hand != 2 )	004
  {
    l = bcon_nul;
    bcon_nul = 2L;
    write_redir( "\r\n" );
    bcon_nul = l;
  } */
  vcurs_off();
  col = 0;
  get_corner();
  if( row >= set.rows[rez]-1 )
  {
    oldrow--;
    if( line >= set.lines[rez]-1 )
    {
      inc_scrn( &scrn_ptr );
      r = line - scrn_line;		/* 004 */
      if( is_top )
      {
        blit_lines( 1, r2=set.rows[rez]-1, 0, 0/*004*/ );
        erase_lines( r, r );				/* 004 */
        if( (r2+=corn_y)!=scrn_line+r ) draw_range( r2, r2, in_prog );	/* 004 */
        if( !single ) yield();		/* 004 */
      }
      else
      {
        erase_lines( r, r );		/* 004: was row */
        set_dial( in_prog );
      }
    }
    else
    {
      line++;
      if( single ) blit_lines( 1, set.rows[rez]-1, 0, 1 );
      else if( corn_y == scrn_line )  /* 004 added if */
      {
        is_io = in_prog;
        scroll( WA_DNLINE );
      }
    }
  }
  else line++;
  get_corner();
  vcurs_on(0);
}

int gtext2( char *string )
{
  int ret;

  flush_out();
  gt_ign_cr = 1;	/* 004: moved here */
  ret = gtext(string);
  gt_ign_cr = 0;	/* 004 */
  return ret;
}

void ivmouse( int mode )
{
  if( !iconified ) _v_mouse(mode);
}

int gtext( char *string )
{
  int x, y, max, not_end, i, l, w;
  char is_nl, last, map;
  char *ptr, *ptr2;
  Rect r;

  last_esc = 0;
  if( ctrlc ) return(0);
  if( !in_echo ) write_redir( string );
  ivmouse(0);
  vcurs_off();
  ptr = string-1;
  if( !single && paused<0 ) yield();
  get_corner();
  if( (map=is_scalable&&!iconified) != 0 ) _vst_charmap( 0 );
  do
  {
    if( !gt_ign_cr )
    {
      ptr = strchr( string=ptr+1, '\n' );
      ptr2 = strchr( string, '\r' );
    }
    else ptr=ptr2=0;
    is_nl = 0;
    if( ptr && (ptr<ptr2 || !ptr2) ) is_nl++;
    else ptr = ptr2;
    if( ptr )
    {
      last = *ptr;
      *ptr = '\0';
    }
    do
    {
      x = col*char_w + blank[0].ob_x;
      y = line*char_h + blank[0].ob_y;
      do
      {
        switch( check_io( is_cqcs ) )
        {
          case 1:
            if( !paused ) paused = 1;
            break;
          case 2:
            if( paused>0 ) paused = 0;
            break;
          case 3:
            ctrlc++;
            newline();
cc:         set_lacurs();
            ivmouse(1);
            vcurs_on(0);
            if( paused>0 ) paused = 0;
            in_echo = 0;
            if( map ) _vst_charmap( 1 );
            return(0);
        }
        if( paused && !single ) yield();
      }
      while( paused );
      not_end = 0;
      if( (l = strlen(string)) != 0 )
      {
        max = set.linelen[rez]-1 - col + 1;
        if( (not_end = l>max) != 0 )
          if( (l = max) <= 0 ) goto wrp;
        memcpy( ptr2=get_scr( row, col ), string, l );
        for( ptr2+=max, i=col+l; --i>=col; )
          if( !inverse ) *(ptr2+(i>>3)) &= ~(1<<(i&7));
          else *(ptr2+(i>>3)) |= 1<<(i&7);
        if( !iconified )
          if( !is_top ) draw_current(in_prog);
          else
          {
            _vswr_mode(MD_REPLACE);
            if( is_scalable )
            {
              w = l;
              text_2_arr( (unsigned char *)string, &w );
              ftext16_mono( x, y, w );
            }
            else v_gtext( vdi_hand, x, y, string );
            if( inverse )
            {
              r.x = x;
              r.y = y;
              r.w = l*char_w;
              r.h = char_h;
              blank_box( &r, MD_XOR );
            }
          }
        if( not_end )
        {
wrp:      if( !discard )
          {
            newline();
            if( ctrlc ) goto cc;
          }
          else col = set.linelen[rez]-1;
          string += max;
        }
        else col += l;
      }
    }
    while( not_end );
    if(ptr)
    {
      *ptr = last;
      if( is_nl )
      {
        newline();
        if( ctrlc ) goto cc;
      }
      else col=0;
    }
  }
  while(ptr);
  set_lacurs();
  ivmouse(1);
  vcurs_on(0);
  in_echo=0;
  if( map ) _vst_charmap( 1 );
  return(1);
}

void check_rowcol(void)
{
  int i;

  if( row>(i=set.rows[rez]-1) ) row = i;
  else if( row<0 ) row=0;
  if( col>(i=set.linelen[rez]-1) ) col = i;
  else if( col<0 ) col=0;
  line = row + scrn_line;
}

#define timer (unsigned long *)0x4ba
static long last_time;

long _timer(void)
{
  return *timer;
}

long get_timer(void)
{
  if( has_mint ) return Supexec( _timer );
  return *timer;
}

long get_evkey(void)
{
  long ret;

  ret = (((long)evkey<<8)&0xFF0000L) | (evkey&0xFF);
  evkey = 0;
  return ret;
}

void write_key(void)
{
  long l;
  
  l = get_evkey();
  if( pipe[1] ) Fputchar( pipe[1], l, 1 );
}

char *read_tosrun( char *buf, char *p, char *out, int size )
{
  static int len;
  char c;

  for(;;)
  {
    if( !p || !len )
      if( (len = Fread( trun_fd, 256, p=buf )) <= 0 )
      {
        *out = 0;
        return 0L;
      }
    while(len--)
      if( (c = *p++) == 0 || c == ' ' )
      {
        *out = 0;
        return p;
      }
      else if( size )
      {
        *out++ = c;
        size--;
      }
  }
}

int read_pipe( int always )
{
  long l, h, len;
  char buf[256], *p, *p2;

  h = 1<<pipe[0];
  if( pipe[0] && Fselect( 1, &h, 0L, 0L )>0 && (always || get_timer() > last_time+25) )
  {
    len = Finstat(pipe[0]);
    wind_update(BEG_UPDATE);
    while( (l = Fread( pipe[0], !len||len>sizeof(buf)?sizeof(buf):len, buf )) > 0 )
    {
      bcon_nul = l;
      doecho(buf);
    }
    wind_update(END_UPDATE);
  }
  h = 1<<trun_fd;
  if( trun_fd && Fselect( 1, &h, 0L, 0L )>0 && Finstat(trun_fd) > 0 )
  {
    p = read_tosrun( buf, 0L, prog_path, sizeof(prog_path) );
    p = read_tosrun( buf, p, prog_path, sizeof(prog_path) );
    read_tosrun( buf, p, tail, sizeof(tail) );
    return 1;
  }
  return 0;
}

void flush_out(void)
{
  long b;

  if( out_ptr != out_buf )
  {
    *out_ptr = 0;
    b = bcon_nul;	/* 006 */
    bcon_nul = out_ptr - out_buf;
    doecho( out_ptr=out_buf );
    bcon_nul = b;	/* 006 */
  }
  else if( in_prog ) last_time = get_timer();
}

void out_one( char ch )
{
  if( out_ptr-out_buf >= sizeof(out_buf)-1 || get_timer() > last_time+50 )
      flush_out();
  *out_ptr++ = ch;
}

void doecho( char *ptr )
{
  register int i, j;
  register char c, oldc, nul;
  register char *ptr2, *ptr3;
  static char esc_str[] = "\033\b\t\a\f";

  if( ptr!=out_buf ) flush_out();
  if( in_prog ) last_time = get_timer();
  write_redir( ptr );
      vcurs_off();
      nul = bcon_nul>0;
      while( nul || *ptr )
      {
        switch( last_esc )
        {
          case 4:
            inverse = (*ptr&0xf)!=0;
          case 5:
            ptr++;
            last_esc = 0;
            goto nextch;
          case 3:
            col = *ptr - '\040';
            last_esc = 0;
            check_rowcol();
            ptr++;
            goto nextch;
          case 2:
            line = (row = *ptr - '\040') + scrn_line;
            last_esc++;
            check_rowcol();
            ptr++;
            goto nextch;
          case 1:
            ptr3 = ptr+1;
            ptr2 = ptr-1;
            last_esc = 0;
            goto sw;
        }
        if( !nul )
        {
          ptr2 = ptr;
          while( (c=*ptr2) != 0 && !strchr(esc_str,c) ) ptr2++;
          ptr3 = ptr2 + (c?1:0) + (c=='\033' && *(ptr2+1));
          if( ptr != ptr2 )
          {
            oldc = *ptr2;
            *ptr2 = '\0';
            in_echo = 1;
            gtext( ptr );
            *ptr2 = oldc;
          }
        }
        else
        {
/* 004          ptr2 = ptr-1;
          ptr3 = ptr+1;
          if( (c = strchr(esc_str,*ptr) ? *ptr : 0) == 0 )
          {
            temp[0] = *ptr;
            in_echo = 1;
            gtext(temp);
          }
          else if( c=='\033' )
            if( bcon_nul==1 )
            {
              last_esc=1;
              break;
            }
            else
            {
              ptr2++;
              ptr3++;
              bcon_nul--;
            } */
          ptr2 = ptr;
          while( bcon_nul && (c = strchr(esc_str,*ptr2) ? *ptr2 : 0) == 0 )
          {
            ptr2++;
            bcon_nul--;
          }
          if( ptr != ptr2 )
          {
            oldc = *ptr2;
            *ptr2 = '\0';
            in_echo = 1;
            gtext( ptr );
            *ptr2 = oldc;
          }
          if( !bcon_nul ) break;
          ptr3 = ptr2+1;
          if( c=='\033' )
          {
            last_esc = 1;
            ptr = ptr3;
            goto nextch;
          }
/*            if( bcon_nul==1 )
            {
              last_esc=1;
              break;
            }
            else
            {
              ptr3++;
              bcon_nul--;
            } */
        }
        if( c=='\b' ) goto escD;
        if( c=='\t' )
        {
          j = (col&0xFFF8) + 8 - col;
          if( (col += j) > set.linelen[rez]-1 )
            if( discard ) col=set.linelen[rez]-1;
            else col -= set.linelen[rez]-1;
        }
        else if( c=='\a' ) Bconout( 2, 7 );
        else if( c=='\f' ) goto escE;
        else if( c )
sw:         switch( *(ptr2+1) )
        {
          case '\0':
            last_esc=1;
            break;
          case 'A':
            row--;
            check_rowcol();
            break;
          case 'B':
            row++;
            check_rowcol();
            break;
          case 'C':
            col++;
            check_rowcol();
            break;
          case 'D':
escD:       col--;
	    check_rowcol();
            break;
          case 'E':
escE:       erase_lines( 0, set.rows[rez]-1 );
          case 'H':
            row = col = 0;
            line = scrn_line;
            break;
          case 'I':       /* up w/scroll */
            if( row )
            {
              row--;
              line--;
            }
            else blit_lines( 0, set.rows[rez]-1-1, 1, 1 );
            break;
          case 'J':       /* erase to eop */
            erase_ln( col, set.linelen[rez]-1 );
            erase_lines( row+1, set.rows[rez]-1 );
            break;
          case 'K':       /* erase to eol */
escK:       erase_ln( col, set.linelen[rez]-1 );
            break;
          case 'L':       /* ins line */
            blit_lines( row, set.rows[rez]-1, row+1, 1 );
            col = 0;
            break;
          case 'M':       /* del line */
            blit_lines( row+1, set.rows[rez]-1, row, 1 );
            col = 0;
            break;
          case 'Y':
            last_esc = 2;
            break;
          case 'Z':
            break;
          case 'b':
            last_esc = 5; /* ignore foreground color */
            break;
          case 'c':
            last_esc = 4; /* set background color */
            break;
          case 'd':       /* erase to (incl) cursor */
            erase_lines( 0, row-1 );	/* 004: added -1 */
            erase_ln( 0, col );
            break;
          case 'e':
            old_show = 1;
            break;
          case 'f':
            old_show = 0;
            break;
          case 'j':
            saver = row;
            savec = col;
            break;
          case 'k':
            line = (row = saver) + scrn_line;
            col = savec;
            check_rowcol();
            break;
          case 'l':
            col = 0;
            goto escK;
          case 'o':       /* erase beg line to (incl) curs */
            erase_ln( 0, col );
            break;
          case 'p':
            inverse = 1;
            break;
          case 'q':
            inverse = 0;
            break;
          case 'v':
            discard = 0;
            break;
          case 'w':
            discard = 1;
            break;
          case '=':
            break;
          case '>':
            break;
          default:
            ptr3--;
        }
        ptr = ptr3;
nextch: if( nul )		/* 004: was bcon_nul */
          if( --bcon_nul<=0 ) break;
      }
  vcurs_on(0);
  set_lacurs();
  bcon_nul = 0;
}

int is_ctrlc_( char *ptr )
{
  if( !ign_ctrlc && *(ptr+3) == '\03' ) return(1);
  return(0);
}

int is_ctrlc(void)
{
  unsigned int l;

  switch( check_io( is_ctrlc_ ) )
  {
    case 1:
      return(1);
    default:
      return(0);
  }
}

int _abort(void)
{
  char c;

  for(;;)
    if( ctrlc )
    {
      ctrlc=0;
      return 1;
    }
    else if( !is_ctrlc() ) return 0;
    else ctrlc++;
}

int alert( int num )
{
  char **p;

  rsrc_gaddr( 15, num, &p );
  return form_alert( 1, *p );
}

void al_err( int num, int err )
{
  char **p;

  rsrc_gaddr( 15, num, &p );
  x_form_error( *p, err );
}

void close_redir(void)
{
  if( redir_hand > 5 && redir_hand != iredir_hand ) Fclose(redir_hand);
  redir_hand = 2;
  menu_icheck( menu, MOUTPUT, 0 );
}

void close_iredir(void)
{
  if( iredir_hand > 5 && iredir_hand != redir_hand ) Fclose(iredir_hand);
  iredir_hand = 2;
  ichar_cr = 0;
  menu_icheck( menu, MINPUT, 0 );
}

int write_redir( char *string )
{
  long x;
  int h;

  if( (h=redir_hand) == 2 ) return 1;
  if( h > 5 )
  {
    x = bcon_nul>0 ? bcon_nul : strlen(string);
    if( Fwrite( h, x, string ) != x )
    {
      close_redir();
      alert( BADREDIR );
      return(0);
    }
    return(1);
  }
  else
  {
    h = h==3 || h==4 ? 7-h : h;     		/* avoid bug in ROMs */
    if( h != 5 ) while( *string )               /* ignore NULL: */
    {
      while( !Bcostat(h) )
        if( _abort() )
        {
          redir_hand = 2;
          return(0);
        }
      Bconout( redir_hand, *string++ );
    }
    return(1);
  }
}

/*** Input routines start here ***/

void zero_cmd(void)
{
  *(cmdptr = cmdbuf) = '\0';
}

/* must not destroy a1 */
void force_cono( int flag )
{
  static int old;

  if( !flag )
  {
    old = redir_hand;
    redir_hand = 2;
  }
  else redir_hand = old;
}

void force_coni( int flag )
{
  static int old;

  if( !flag )
  {
    old = iredir_hand;
    iredir_hand = 2;
  }
  else iredir_hand = old;
}

void flush_pipe(void)
{
  flush_out();
  if( has_mint ) read_pipe(1);
}

long hasch(void)
{
  long pos;
  int hand, ret;
  char c=0;

  flush_pipe();
  hand = iredir_hand;
  if( !single ) yield();
  if( hand < 5 )
  {
    if( !single && hand==2 ) return evkey ? -1 : 0;
    ret = Bconstat(hand);
  }
  else if( hand == 5 ) return(0);
  else
  {
    pos = Fseek( 0L, hand, 1 );
    if( ichar_cr )
    {
      Fread( hand, 1L, &c );
      if( c == '\n' ) pos++;
    }
    ret = Fseek( 0L, hand, 2 ) != pos;
    Fseek( pos, hand, 0 );
  }
  return(ret);
}

long bconinr( void )
{
  unsigned char ch;
  int h;

  flush_pipe();
  if( !ign_ctrlc && is_ctrlc() ) return(-1L);
  h = iredir_hand;
  if( h < 5 ) return( bconin(h) );
  else if( h==5 ) return(-2L);
  else
  {
getch:
    if( Fread( h, 1L, &ch ) != 1L ) return(-2L);
    else if( ichar_cr && ch == '\n' )
    {
      ichar_cr = 0;
      goto getch;
    }
    else return( (long)ch );
  }
}

void disp_cmd(int space)
{
  int j;

  oldrow = row;
  j = col;
  gtext( cmdptr );
  if( space ) gtext(" ");
  row = oldrow;
  col = j;
}

void to_right(void)
{
  if( *cmdptr )
  {
    cmdptr++;
    if( ++col > set.linelen[rez]-1 && row < set.rows[rez]-1 )
    {
      col = 0;
      row++;
      line++;
    }
  }
}

void backsp(void)
{
  if( --col < 0 )
  {
    col = set.linelen[rez]-1;
    row--;
    line--;
  }
}

void to_start(void)
{
  while( cmdptr > cmdbuf )
  {
    backsp();
    cmdptr--;
  }
}

void insert_char( int ch )
{
  static char string[2]=" ";
  char temp[CMDBUFLEN+2];
  int i, j;

  i = *cmdptr;
  if( insert ) strcpy( temp, cmdptr );
  *cmdptr++ = string[0] = ch;
  if( !i ) *cmdptr = '\0';
  gtext(string);
  if( insert )
  {
    strcpy( cmdptr, temp );
    disp_cmd(0);
  }
}

void clear_cmd(void)
{
  int j;
  void to_start(void);

  gtext( cmdptr );
  cmdptr += strlen(cmdptr);
  j = row;
  to_start();
  erase_ln( col, set.linelen[rez]-1 );
  if( row != j ) erase_lines( row+1, j );
}

int read_key( int k )
{
  register int i, j, key, ret=0;
  char *ptr, *ptr2;

  key = k & 0xFF;                 /* isolate just the ASCII value */
  vcurs_off();
  if( key=='\b' )
  {
    if( cmdptr > cmdbuf )
    {
      backsp();
      cmdptr--;
      strcpy( cmdptr, cmdptr+1 );
      disp_cmd(1);
    }
  }
  else if( key=='\t' )
  {
    j = (long)cmdptr - (long)cmdbuf;
    i = (j&0xFFF8) + 8 - j;
    while( strlen(cmdbuf) < cmdbufmax && i-- ) insert_char( ' ' );
    if( i>=0 ) Bconout(2,7);
  }
  else if( key == '\x7f' )               /* Del */
  {
    if( *cmdptr )
    {
      strcpy( cmdptr, cmdptr+1 );
      disp_cmd(1);
    }
  }
  else if( key == '\r' ) ret = 1;	/* CR */
  else if( key=='\03' )
  {
    ctrlc++;
    ret = 1;
  }
  else if( k == 0x6200 )                /* Help */
  {
  }
  else if( k == 0x4700 || key == '\x18' )
  {
    clear_cmd();  /* ^X or Clr */
    *cmdptr = '\0';
  }
  else if( k == 0x4d36 )        /* Shift-right */
  {
    while( *cmdptr ) to_right();
  }
  else if( k == 0x4b34 ) to_start();    /* Shift-left */
  else if( k == 0x4838 ) ret = 2;   /* Shift-up */
  else if( k == 0x5032 ) ret = 3;   /* Shift-down */
  else if( key == '\x1a' ) ret = 4;  /* ^Z */
  else if( key && key != '\n' && cmdptr < cmdbuf+cmdbufmax && (!insert ||
      strlen(cmdbuf) < cmdbufmax) ) insert_char(key);
  else if( k == 0x5200 ) insert = !insert;
  else if( k == 0x4800 )                /* Up */
  {
    if( cmdptr > cmdbuf+set.linelen[rez]-1 )
    {
      cmdptr -= set.linelen[rez]-1 + 1;
      row--;
      line--;
    }
  }
  else if( k == 0x5000 )                /* Down */
  {
    if( cmdptr < cmdbuf+strlen(cmdbuf)-(set.linelen[rez]-1) )
    {
      cmdptr += set.linelen[rez]-1 + 1;
      row++;
      line++;
    }
  }
  else if( k == 0x4B00 )                /* Left */
  {
    if( cmdptr > cmdbuf )
    {
      cmdptr--;
      backsp();
    }
  }
  else if( k == 0x4D00 ) to_right();    /* Right */
  else Bconout( 2, 7 );
  vcurs_on(0);
  return(ret);
}

void force_cur( int flag )
{
  static int oc, oh, oos;

  if( !flag )
  {
    oc = show_curs;
    oh = curs_hide;
    oos = old_show;
    vcurs_on(1);
  }
  else
  {
    if( oh ) vcurs_off();
    show_curs = oc;
    curs_hide = oh;
    old_show = oos;
  }
}

long bconin( int hand )
{
  if( !single && hand==2 )
  {
    do
      yield();
    while( !evkey );
    return get_evkey();
  }
  do
    if( !single ) yield();
  while( !Bconstat(hand) );
  /*check_help();*/
  return( Bconin(hand) );
}

int read_str(int ctrlz)
{
  long l;
  int i;

  gtext(cmdbuf);
  force_cur(0);
  for(;;)
  {
    i = read_key( (int)(((l=bconin(2)) >> 8) + (l&0xff)) );
    if( i==1 || ctrlz && i==4 ) break;
  }
  force_cur(1);
  return( i==4 );
}

char *next_str( char *ptr )
{
  return( ptr + strlen(ptr) + 1 );
}

int getstr( char *string, int len )
{
  int c, cnt;
  long l;
  char *ptr;

  flush_pipe();
  if( iredir_hand != 2 )
  {
    ptr = cmdbuf;
    cnt = 0;
    while( cnt<len && (l=bconinr())>=0 && (c=(char)l)!='\r' && c!='\n' && c )
    {
      *ptr++ = c;
      cnt++;
    }
    if( c == '\r' ) ichar_cr++;
    if( l==-1L ) ctrlc++;
    *ptr++ = '\0';
    if( l==-2L ) return(1);
    return(0);
  }
  else
  {
    if( len > CMDBUFLEN ) len = CMDBUFLEN;
    strncpy( cmdbuf, string, len );
    *(cmdbuf+len) = '\0';
    cmdptr = next_str(cmdbuf)-1;
    c = cmdbufmax;
    cmdbufmax = len;
    cnt = read_str(0);
    cmdbufmax = c;
    return(cnt);
  }
}

char have_linea;

void reset_linea(void)
{
  if( set.options.s.set_la && have_linea )
  {
    have_linea=0;
    V_CEL_HT = old_celht;
    V_CEL_MX = old_mx;
    V_CEL_MY = old_my;
  }
}

void set_linea(void)
{
  if( set.options.s.set_la && !have_linea )
  {
    have_linea=1;
    old_celht = V_CEL_HT;
    V_CEL_HT = char_h;
    old_mx = V_CEL_MX;
    V_CEL_MX = set.linelen[rez]-1;
    old_my = V_CEL_MY;
    V_CEL_MY = set.rows[rez]-1;
  }
}

void set_traps( int flag )
{
  if( has_mint ) return;
  if( !flag && !have_traps )
  {
    setexc( 0xb4, t13, &t13adr, 0 );
    setexc( 0x84, t1, &t1adr, 0 );
    have_traps = 1;
  }
  else if( flag && have_traps )
  {
    setexc( 0xb4, t13adr, 0L, 1 );
    setexc( 0x84, t1adr, 0L, 1 );
    in_a7 = 0;
    have_traps = 0;
  }
}

void get_coords( int *x, int *y )
{
  if( (*x-=blank[0].ob_x) < 0 ) *x = -1;
  else *x /= char_w;
  if( (*y-=blank[0].ob_y-1) < 0 ) *y = -1;
  else *y /= char_h;
}

void get_corner(void)
{
  int i;

  corn_x = winner.g_x;
  corn_y = winner.g_y;
  get_coords( &corn_x, &corn_y );
  if( corn_x<0 ) corn_x = 0;
  if( corn_y<0 ) corn_y = 0;
  if( (i=line-set.rows[rez]+1) > scrn_line ) scrn_line = i;
  row = line - scrn_line;
/* 004  row = line - corn_y; */
}

void redraw_lines( int first, int last )
{
  int i, j, k, y, px[4], l;
  char *ptr, *end, e;

  if( is_scalable ) _vst_charmap( 0 );
  l = set.linelen[rez];
  for( y=blank[0].ob_y+char_h*first, i=first-1; ++i<=last; y+=char_h )
  {
    if( draw_mode&MODE_TEXT )
    {
      _vswr_mode(MD_REPLACE);
      ptr = get_scr(i-scrn_line,0);
      e = *(end=ptr+l);
      *end = 0;
      if( is_scalable )
      {
        px[0] = strlen(ptr);
        text_2_arr( (unsigned char *)ptr, px );
        ftext16_mono( blank[0].ob_x, y, px[0] );
      }
      else v_gtext( vdi_hand, blank[0].ob_x, y, ptr );
      *end = e;
      for( ptr=end, j=l; --j>=0; )
        if( *(ptr+(j>>3)) & (1<<(j&7)) )
        {
          px[2] = (px[0] = blank[0].ob_x + j * char_w) + char_w - 1;
          px[3] = (px[1] = y) + char_h - 1;
          _vswr_mode( MD_XOR );
          vr_recfl( vdi_hand, px );
        }
      if( !curs_hide && i==line )
      {
        curs_is_on = 0;
        vdi_cursor(0);
      }
    }
    if( draw_mode&MODE_SEL && i>=sel_row && i<=sele_row )
    {
      j = i!=sel_row || sel_col<=0 ? 0 : sel_col;
      k = i!=sele_row || sele_col<=0 ? l : sele_col;
      px[2] = (px[0] = blank[0].ob_x + j * char_w) + (k-j)*char_w - 1;
      px[3] = (px[1] = y) + char_h - 1;
      _vswr_mode( MD_XOR );
      vr_recfl( vdi_hand, px );
    }
  }
  if( is_scalable ) _vst_charmap( 1 );
}

int cdecl draw_text( PARMBLK *pb )
{
  int i, max, erow;

  i = (pb->pb_yc - pb->pb_y)/char_h;
  if( (max = i + (pb->pb_hc+char_h-1)/char_h) > set.lines[rez] ) max = set.lines[rez];
  if( draw_mode==MODE_SEL )
  {
    erow = sele_col<0 ? sele_row-1 : sele_row;
    if( erow<i || sel_row>max ) return pb->pb_currstate;
    i = sel_row<i ? i : sel_row;
    max = erow>max ? max : erow;
  }
  else if( !(draw_mode&MODE_EVERY) )
  {
    if( rend<i || rstart>max ) return pb->pb_currstate;
    i = rstart<i ? i : rstart;
    max = rend>max ? max : rend;
  }
  _vs_clip( 1, (Rect *)&pb->pb_xc );
  redraw_lines( i, max );
  _vs_clip( 1, (Rect *)&winner );
  return pb->pb_currstate;
}

void draw_range( int start, int end, int in_io )
{
  int buf[8];
  char last;

  if( iconified ) return;
  rstart = start;
  rend = end;
  is_io = in_io;
  wind_update( BEG_UPDATE );
  last = draw_mode;
  draw_mode = ~MODE_EVERY;
  buf[0] = WM_REDRAW;
  buf[3] = wind;
  *(GRECT *)&buf[4] = winner;
  is_io = in_io;
  appl_write( buf[1]=apid, 16, buf );
  draw_mode = last;
  is_io = in_io;
  wind_update( END_UPDATE );
}

void draw_current( int in_io )
{
  draw_range( line, line, in_io );
}

#define TIOCSWINSZ	(('T'<< 8) | 12)
void set_winsz(void)
{
  struct winsize {
    short	ws_row;
    short	ws_col;
    short	ws_xpixel;
    short	ws_ypixel;
  } tw;

  tw.ws_ypixel = (tw.ws_row = lastrow+1) * char_h;
  tw.ws_xpixel = (tw.ws_col = lastcol+1) * char_w;
  Fcntl(pipe[0], (long)&tw, TIOCSWINSZ);
}

void calc_blank(void)
{
  int i;

  blank[0].ob_width = ((i=set.cols[rez])>set.linelen[rez] ? i : set.linelen[rez])*char_w;
  blank[0].ob_height = ((i=set.rows[rez])>set.lines[rez] ? i : set.lines[rez])*char_h;
  if( wind>0 )
  {
    wind_set( wind, X_WF_DIALWID, char_w );
    wind_set( wind, X_WF_DIALHT, char_h );
    if( in_prog && has_mint )
    {
      set_winsz();
      Pkill( child_id, SIGWINCH );
    }
  }
}

void set_hot(void)
{
  if( set.options.s.use_hot ) menu[0].ob_state |= X_MAGIC;
  else menu[0].ob_state &= ~X_MAGIC;
}

char *pathend( char *ptr )
{
  char *s;

  if( (s=strrchr(ptr,'\\')) == 0 ) return(ptr);
  return(s+1);
}

void wind_name( int end )
{
  wind_set( wind, WF_NAME, end ? pathend(program) : program );
}

int open_wind(void)
{
  int i, dum;

  menu_icheck( menu, MNAME, set.win_type&NAME );
  menu_icheck( menu, MVERT, set.win_type&VSLIDE );
  menu_icheck( menu, MHORIZ, set.win_type&HSLIDE );
  if( wind<=0 )
    if( (i=x_wind_create( set.win_type, X_MENU, wsize.g_x,
        wsize.g_y, wsize.g_w, wsize.g_h )) > 0 )
    {
      blank[0].ob_x = winner.g_x;
      blank[0].ob_y = winner.g_y;
      wind = i;
      calc_blank();
      wind_set( i, X_WF_DIALOG, blank );
      wind_set( i, X_WF_MENU, menu );
      wind_name(0);
      wind_get( i, X_WF_MINMAX, &min_wid, &min_ht, &dum, &dum );
      is_top = 1;
      wind_open( i, wsize.g_x, wsize.g_y, wsize.g_w, wsize.g_h );
      set_hot();
    }
    else
    {
      alert( NOWIND );
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
}

void curs_bottom( int always )
{
  int flag=0;

  get_corner();
  if( line < corn_y || line >= corn_y+set.rows[rez] )
  {
    if( (corn_y = line-set.rows[rez]+1) < 0 ) corn_y = 0;
    blank[0].ob_y = winner.g_y - corn_y*char_h;
    if( (corn_x = col-set.cols[rez]+1) < 0 ) corn_x = 0;
    blank[0].ob_x = winner.g_x - corn_x*char_w;
    flag = 1;
  }
  if( always || flag ) set_dial(0);
}

int intersect( Rect r1, Rect r2, Rect *res )
{
  res->x = r1.x < r2.x ? r2.x : r1.x;
  res->y = r1.y < r2.y ? r2.y : r1.y;
  res->w = r1.x+r1.w < r2.x+r2.w ? r1.x+r1.w-res->x : r2.x+r2.w-res->x;
  res->h = r1.y+r1.h < r2.y+r2.h ? r1.y+r1.h-res->y : r2.y+r2.h-res->y;
  return( res->w > 0 && res->h > 0 );
}

void miscopts(void)
{
  rsrc_gaddr( 0, MISCOPTS, &form );
  set_if( MOKEYS, set.options.s.use_hot );
  set_if( MOLINEA, set.options.s.set_la );
  set_if( MOTERM, set.options.s.termcap );
  set_if( MOARGV, set.options.s.use_argv );
  set_if( MOPAUSE, !set.options.s.no_pause );
  if( make_form( MISCOPTS, 0, 0L, 0L ) == MOOK )
  {
    set.options.s.use_hot = form[MOKEYS].ob_state&SELECTED;
    set_hot();
    set.options.s.set_la = form[MOLINEA].ob_state&SELECTED;
    set.options.s.termcap = form[MOTERM].ob_state&SELECTED;
    set.options.s.use_argv = form[MOARGV].ob_state&SELECTED;
    set.options.s.no_pause = !(form[MOPAUSE].ob_state&SELECTED);
  }
}

void get_font(void)
{
  if( font_sel( FONTOPTS, SYSTEM, &set.font[rez], &set.point[rez] ) )
  {
    new_size();
    curs_bottom(1);
  }
}

long calc_scroll(void)
{
  /* 004: rounded up to even # */
  return (set.lines[rez]*(long)(set.linelen[rez]+((set.linelen[rez]+7)>>3))+1L)&-2L;
}

void scr_nums( OBJECT *o, int draw )
{
  x_sprintf( o[SOCNUM].ob_spec.tedinfo->te_ptext, "%d", set.linelen[rez] );
  x_sprintf( o[SOLNUM].ob_spec.tedinfo->te_ptext, "%d", set.lines[rez] );
  x_sprintf( o[SOMEM].ob_spec.tedinfo->te_ptext, "%D", calc_scroll() );
  if( draw&1 ) objc_draw( o, SOCNUM, 0, 0, 0, 0, 0 );
  if( draw&2 ) objc_draw( o, SOLNUM, 0, 0, 0, 0, 0 );
  if( draw ) objc_draw( o, SOMEM, 0, 0, 0, 0, 0 );
}

void get_scrnum( OBJECT *o )
{
  if( (set.linelen[rez] = atoi( o[SOCNUM].ob_spec.tedinfo->te_ptext )) < 80 )
  {	/* 004: added check */
    set.linelen[rez] = 80;
    scr_nums( o, 1 );
  }
  if( (set.lines[rez] = atoi( o[SOLNUM].ob_spec.tedinfo->te_ptext )) > max_ht )
  {
    set.lines[rez] = max_ht;
    scr_nums( o, 2 );
  }
  else if( set.lines[rez] < 24 )
  {	/* 004: added check */
    set.lines[rez] = 24;
    scr_nums( o, 2 );
  }
}

void scrlup( OBJECT *o, int num )
{
  get_scrnum(o);
  switch( num&0x7fff )
  {
    case SOMEM:
    case SOMEM-1:
      scr_nums( o, -1 );
      break;
    case SOCMI:
      if( set.linelen[rez]>80 )
      {
        set.linelen[rez]--;
        scr_nums( o, 1 );
      }
      break;
    case SOCPL:
      if( set.linelen[rez]<999 )
      {
        set.linelen[rez]++;
        scr_nums( o, 1 );
      }
      break;
    case SOLMI:
      if( set.lines[rez]>24 )
      {
        set.lines[rez]--;
        scr_nums( o, 2 );
      }
      break;
    case SOLPL:
      if( set.lines[rez]<9999 && set.lines[rez]<max_ht )
      {
        set.lines[rez]++;
        scr_nums( o, 2 );
      }
      break;
  }
}

int new_scroll(void)
{
  long l;

  if( set.lines[rez] > max_ht ) set.lines[rez] = max_ht;
  l = calc_scroll();
  if( screen ) Mfree(screen);
  if( (screen = (char *)Malloc(l)) == 0 )
  {
    alert( ALNOMEM );
    return 0;
  }
  screen_size = l;
  clear_screen();
  return 1;
}

void get_scroll(void)
{
  OBJECT *o;
  int old_ll, old_l, ret;

  old_ll = set.linelen[rez];
  old_l = set.lines[rez];
  rsrc_gaddr( 0, SCRLOPTS, &o );
  scr_nums( o, 0 );
  ret = make_form( SCRLOPTS, 0, 0L, scrlup );
  get_scrnum(o);
  if( ret != SOOK || !new_scroll() )
  {
    set.linelen[rez] = old_ll;
    set.lines[rez] = old_l;
  }
  else
  {
    new_size();
    blank[0].ob_x = winner.g_x;
    blank[0].ob_y = winner.g_y;
    set_dial(0);
  }
}

void get_outer(void)
{
  int w, h;

  winner.g_x = (winner.g_x+(char_w>>1))/char_w*char_w;
  for(;;)	/* 004 */
  {
    if( (w = (winner.g_w+(char_w>>1))/char_w) > set.linelen[rez] ) w = set.linelen[rez];
    if( (h = (winner.g_h+(char_h>>1))/char_h) > set.lines[rez] ) h = set.lines[rez];
    winner.g_w = w*char_w;
    winner.g_h = h*char_h;
    x_wind_calc( WC_BORDER, set.win_type, X_MENU, winner.g_x, winner.g_y,
        winner.g_w, winner.g_h, &wsize.g_x, &wsize.g_y, &wsize.g_w,
        &wsize.g_h );
    if( wsize.g_w > max.g_w + 2 ) winner.g_w -= char_w;
    else if( wsize.g_h > max.g_h ) winner.g_h -= char_h;
    else break;
  }
  if( wsize.g_w < min_wid ) winner.g_w += char_w;
  if( wsize.g_h < min_ht ) winner.g_h += char_h;
  set.cols[rez] = winner.g_w/char_w;
  set.rows[rez] = winner.g_h/char_h;
  set.xoff[rez] = winner.g_x/char_w;
  calc_blank();
  x_wind_calc( WC_BORDER, set.win_type, X_MENU, winner.g_x, winner.g_y,
      winner.g_w, winner.g_h, &wsize.g_x, &wsize.g_y, &wsize.g_w,
      &wsize.g_h );
  set.yoff[rez] = wsize.g_y - max.g_y;
  _vs_clip( 1, (Rect *)&winner );
}

void get_inner(void)
{
  x_wind_calc( WC_WORK, set.win_type, X_MENU, wsize.g_x, wsize.g_y,
      wsize.g_w, wsize.g_h, &winner.g_x, &winner.g_y, &winner.g_w,
      &winner.g_h );
  get_outer();
}

void size_wind(void)
{
  int i;

  winner.g_x = set.xoff[rez]*char_w;	/* 004: was <<3 */
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

void initialize(void)
{
  init_font( &set.font[rez], &set.point[rez] );
  _vst_alignment( 0, 5 );
  size_wind();
}

void close_wind(void)
{
  if( wind )
  {
    wind_close(wind);
    wind_delete(wind);
    wind=0;
    iconified=0;
  }
}

void do_quit(void)
{
  if( screen ) Mfree(screen);	/* 004 */
  close_redir();
  close_iredir();
  close_wind();
  if( vdi_hand )
  {
    no_fonts();
    v_clsvwk(vdi_hand);
  }
  rsrc_free();
  appl_exit();
  reset_linea();
  set_traps(1);
  if( pipe[0] ) Fclose(pipe[0]);
  exit(0);
}

void save_settings(void)
{
  int ret, i;
  char temp[100];

  graf_mouse( BUSYBEE, 0L );
  if( x_shel_put( X_SHOPEN, "Geneva TOS" ) > 0 )
  {
    x_sprintf( temp, "%v", set.ver );
    ret = x_shel_put( X_SHACCESS, temp );
    x_sprintf( temp, "%x %x", set.win_type, set.options.i );
    if( ret>0 ) ret = x_shel_put( X_SHACCESS, temp );
    for( i=0; i<3 && ret>0; i++ )
    {
      x_sprintf( temp, "%d %d %d %d %d %d %d %d", set.font[i], set.point[i],
          set.rows[i], set.cols[i], set.xoff[i], set.yoff[i], set.lines[i], set.linelen[i] );
      ret = x_shel_put( X_SHACCESS, temp );
    }
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
  while( (i=x_shel_get( X_SHOPEN, 0, "Geneva TOS" )) == -1 );
  if( i>0 && x_shel_get( X_SHACCESS, sizeof(temp)-1, temp ) > 0 )
  {
    x_sscanf( temp, "%v", &ver );
    if( ver != TOSSET_VER )
    {
      alert( ALCFGVER );
      graf_mouse( ARROW, 0L );
      return 0;
    }
    s.ver = TOSSET_VER;
    if( (ret = x_shel_get( X_SHACCESS, sizeof(temp)-1, temp )) > 0 )
        x_sscanf( temp, "%x %x", &s.win_type, &s.options.i );
    for( i=0; i<3 && ret>0; i++ )
      if( (ret = x_shel_get( X_SHACCESS, sizeof(temp)-1, temp )) > 0 )
        x_sscanf( temp, "%d %d %d %d %d %d %d %d", &s.font[i], &s.point[i],
            &s.rows[i], &s.cols[i], &s.xoff[i], &s.yoff[i], &s.lines[i],
            &s.linelen[i] );
    if( ret>0 )
    {
      if( set.win_type&NAME ) set.win_type |= SMALLER;	/* 004 */
      i = set.win_type;
      x_shel_get( X_SHCLOSE, 0, 0L );
      memcpy( &set, &s, sizeof(set) );
      if( wind>0 )
      {
        set_hot();
        if( i != set.win_type )
        {
          close_wind();
          initialize();
          if( !open_wind() ) do_quit();
        }
        else initialize();
        curs_bottom(1);
      }
      graf_mouse( ARROW, 0L );
      return 1;
    }
    else alert( ALCFGERR );
  }
  graf_mouse( ARROW, 0L );
  return 0;
}

void do_about(void)
{
  char temp[200], **p;

  rsrc_gaddr( 15, ALABOUT, &p );
  x_sprintf( temp, *p, GNVATOS_VER );
  form_alert( 1, temp );
}

void force_40( int id, int io )
{
  APPFLAGS apf;

  /* force AES 4.0 messages so that we will always get CH_EXIT */
  is_io = io;
  x_appl_flags( 3, id, &apf );
  apf.flags.s.AES40_msgs = 1;
  is_io = io;
  x_appl_flags( 4, id, &apf );
}

int do_pexec( char **ptr )
{
  SHWRCMD s;
  int buf[8];

  if( child_id>0 ) force_40( child_id, 1 );
  s.name = *ptr;
  s.environ = *(ptr+2);
  is_io = 1;
  if( !shel_write( SHW_RUNANY|SHD_ENVIRON, 0, 0, (char *)&s,
      *(ptr+1) ) ) return -32;
  for(;;)
  {
    is_io = 1;
    evnt_mesag(buf);
    if( buf[0] == CH_EXIT ) return buf[4];
  }
}

int new_file( char *path )
{
  int h, i;
  DTA dta;

  Fsetdta(&dta);
  if( !Fsfirst(path,0x37) )
  {
    if( dta.d_attrib&FA_SUBDIR )
    {
      form_error(-36);
      return -1;
    }
    if( (i = redir_mode) == 0 ) i = alert( ALEXISTS );
    if( i==3 ) return 0;
    if( i==REDIR_OVER )
      if( (i=Fdelete(path)) != 0 )
      {
        al_err( ALDELETE, i );
        return -1;
      }
      else goto create;
    if( (h=Fopen(path,2)) < 0 )
    {
      al_err( ALOPEN, h );
      return -1;
    }
    Fseek( 0L, h, 2 );
  }
  else
  {
create:
    if( (h=Fcreate(path,0)) < 0 )
    {
      al_err( NOCREAT, h );
      return -1;
    }
  }
  return h;
}

int open_redir( char *path )
{
  int h;

  if( (h=new_file(path)) < 0 ) return 1;
  else if( !h ) return 0;
  redir_hand = h;
  menu_icheck( menu, MOUTPUT, 1 );
  return 0;
}

int open_iredir( char *path )
{
  int h;

  if( (h = Fopen(path,0)) != 0 ) iredir_hand = h;
  else
  {
    al_err( ALOPEN, h );
    return 1;
  }
  menu_icheck( menu, MINPUT, 1 );
  return 0;
}

#define TTP_SHORT 124-38*3
#define TTP_ADD ((char *)f->mem_ptr)
void set_longedit( OBJECT *o, int count )
{
  while( --count > 0 )
    (o++)->ob_spec.tedinfo->te_tmplen = X_LONGEDIT;
}

void ttp_mode( OBJECT *o )
{
  TEDINFO *ted;
  int w, u;

  ted = o[PARM1+3].ob_spec.tedinfo;
  w = o[PARM1+1].ob_width / 38;
  u = set.options.s.use_argv;
  o[PARM1+3].ob_width = (!u ? TTP_SHORT : 38)*w;
  ted->te_txtlen = ted->te_tmplen = !u ? TTP_SHORT+1 : 39;
  ted->te_pvalid[TTP_SHORT] = !u ? 0 : 'X';
  ted->te_ptmplt[TTP_SHORT] = !u ? 0 : '_';
  set_longedit( o+PARM1, u ? 5 : 4 );
  if( !u ) ted->te_tmplen = TTP_SHORT+1;
  hide_if( o, u, PARM1+4 );
}

void i_ttp(void)
{
  OBJECT *o;
  register int i, j;
  register char *ptr, *t=tail+1;

  rsrc_gaddr( 0, PARAMS, &o );
  for( j=0, i=PARM1; i<PARM1+5; i++ )
  {
    ptr = o[i].ob_spec.tedinfo->te_ptext;
    if( j<strlen(t) )
    {
      memcpy( ptr, t+j, 38 );
      *(ptr+38) = '\0';
      j += 38;
    }
    else *ptr='\0';
  }
}

void tail_len(void)
{
  int i;

  i = strlen(tail+1);
  tail[0] = i<126 ? i : '\xff';
}

int open_prog( char *path )
{
  int i;
  OBJECT *o;

  strcpy( prog_path, path );
  if( tail!=prog_tail ) Mfree(tail);
  tail = prog_tail;
  tail[0] = ' ';
  tail[1] = 0;
  rsrc_gaddr( 0, PARAMS, &o );
  ttp_mode(o);
  if( make_form( PARAMS, 0, 0L, 0L ) == PARMOK )
    for( i=PARM1; i <= (set.options.s.use_argv ? PARM1+4 : PARM1+3); i++ )
        strcat( tail, o[i].ob_spec.tedinfo->te_ptext );
  else prog_name[0] = 0;
  tail_len();
  return 0;
}

int select( int titl, char *path, int pthlen, char *name, int func(char *path) )
{
  int b;
  char **p;
  char temp[200];

  rsrc_gaddr( 15, titl, &p );
  for(;;)
    if( x_fsel_input( strupr(path), pthlen, strupr(name), 1, &b, *p ) && b )
    {
      strcpy( temp, path );
      strcpy( pathend(temp), name );
      if( !func || !(*func)(temp) ) return 1;
    }
    else return 0;
}

void set_entries(void)
{
  int i;

  menu_ienable( menu, MBLOCK, i=sel_row!=-99 );
  menu_ienable( menu, MCLIP, i );
  paused = i ? -1 : 0;
}

void draw_sel(void)
{
  char last;

  if( wind>0 )
  {
    last = draw_mode;
    draw_mode = MODE_SEL;
    x_wdial_draw( wind, 0, 8 );
    draw_mode = last;
  }
}

void unselect( int draw )
{
  if( sel_row!=-99 && draw ) draw_sel();
  sel_row = sel_col = sele_row = sele_col = -99;
  set_entries();
}

int is_sep( unsigned char c )
{
  return strchr( " ,.!?/*&%$\"\';:|[]{}\t\x1d", c ) != 0;
}

void select_word( int x, int y )
{
  int i, li;
  char *s;

  get_coords( &x, &y );
  unselect(1);
  paused = -1;
  if( x<0 || y>=set.lines[rez] ) return;
  for( li=0, i=0, s=get_scr2(y,0); i<set.linelen[rez]; s++, i++ )
  {
    if( is_sep(*s) ) li = i;
    if( !x-- ) break;
  }
  if( i==set.linelen[rez] || is_sep(*s) ) return;
  sel_row = sele_row = y;
  sel_col = li ? li+1 : 0;
  while( *s && !is_sep(*s) )
  {
    i++;
    s++;
  }
  sele_col = i;
  draw_sel();
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

  xmax = set.linelen[rez];
  ymax = set.lines[rez];
  paused = -1;
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
      y = corn_y-1;
      scroll( shift ? WA_UPPAGE : WA_UPLINE );
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
        unselect(1);
        paused = -1;
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
    else unselect(1);
  graf_mouse( ARROW, 0L );
}

void but_up(void)
{
  int b, dum;

  do
    graf_mkstate( &dum, &dum, &b, &dum );
  while(b&1);
}

int get_sel( int mode, char *out, int len, int full )
{
  static char *s, end;
  static int i, y, ymax;
  int olen, imax, j;
  char *s2, *s3;

  olen = 0;
  if( !mode )
  {
    end = 0;
    y = full ? 0 : sel_row;
    ymax = full ? line : sele_row;
    i = full || sel_col<0 ? 0 : sel_col;
    s = get_scr2(y,i);
  }
  if( end ) return 0;
  for(;;)
  {
    imax = set.linelen[rez]-1;
    if( !full && y==sele_row && sele_col>=0 )
    {
      imax = sele_col;
      s2 = s-i+imax;
    }
    else for( s2=s+(j=imax-i); --j>=0 && *s2==' '; s2-- );
    while( s<=s2 )
    {
      *out++ = *s++;
      olen++;
      i++;
      if( !--len ) return olen;
    }
    if( ++y>ymax ) break;
    s = get_scr2(y,i=0);
    *out++ = '\r';
    *out++ = '\n';
    olen += 2;
    if( (len-=2) <= 0 ) return olen;
  }
  if( !full ) *out = 0;
  end = 1;
  return olen;
}

void save_txt( int full, int clip )
{
  int h, i, l;
  long len;
  char temp[120+13];

  if( clip || select( SAVEPATH, savepth, sizeof(savepth), savename, 0L ) )
  {
    if( clip )
    {
      if( !x_scrp_get( temp, 1 ) ) return;
      strcat( temp, "SCRAP.TXT" );
      if( (h=Fcreate(temp,0)) < 0 )
      {
        al_err( NOCREAT, h );
        return;
      }
    }
    else
    {
      strcpy( temp, savepth );
      strcpy( pathend(temp), savename );
      if( (h=new_file(temp)) <= 0 ) return;
    }
    i = 0;
    while( (l=get_sel( i, temp, sizeof(temp)-1, full )) > 0 )
    {
      if( (len=Fwrite( h, l, temp )) != (long)l )
        if( len<0 )
        {
          al_err( NOWRITE, (int)len );
          break;
        }
        else
        {
          alert( ALFULL );
          break;
        }
      i = 1;
    }
    Fclose(h);
  }
}

void do_help( char *topic )
{
  if( !x_help( topic, "GNVA_TOS.HLP", 0 ) ) alert( NOHELP );
}

int env_str( char *fmt, int num, char *buf )
{
  x_sprintf( buf, fmt, num );
  return strlen(buf)+1;
}

void sigchild(void)
{
  child_term = 1;
  child_ret = (int)Pwait3( 1, 0L );
}

void sigterm(void)
{
  Pkill( child_id, SIGKILL );
  exit(0);
}

int run_prg( int mode, int gr, char *path, char *tail, char *envp[], char *argv0 )
{
  SHWRCMD shwrcmd;
  int ret, i, len, dum, id, oi, oo, og;
  long oldblock;
  char *env, *e, buf[100], temp[120];
  BASPAG *bp;
  static char cols[]="COLUMNS=%d", lines[]="LINES=%d", term[]="TERM=vt52"/* 006: was geneva*/,
      tcap[]="TERMCAP=%stermcap", termname[]="U:\\PIPE\\gtos.A";

  if( !argv0 || !*argv0 )
  {
    temp[0] = Dgetdrv()+'A';
    temp[1] = ':';
    Dgetpath( (argv0=temp)+2, 0 );
    strcat( temp, "\\" );	/* 004 */
  }
  for( i=len=0; envp[i]; i++ )
    len += strlen(envp[i])+1;
  len += env_str( cols, set.linelen[rez], buf );
  len += env_str( lines, set.rows[rez], buf );
  if( set.options.s.termcap ) len += sizeof(term)+sizeof(tcap)-2+strlen(argv0)+1;
  len = (len+1)&-2;
  if( (env=(char *)Malloc(len)) != 0 )
  {
    for( e=env, i=0; envp[i]; i++ )
      if( strncmp( envp[i], "COLUMNS=", 8 ) && strncmp( envp[i], "LINES=", 6 ) &&
          strncmp( envp[i], "TERM=", 5 ) && strncmp( envp[i], "TERMCAP=", 8 ) )
      {
        strcpy( e, envp[i] );
        e += strlen(e)+1;
      }
    e += env_str( cols, set.linelen[rez], e );
    e += env_str( lines, set.rows[rez], e );
    if( set.options.s.termcap )
    {
      strcpy( e, term );
      e += sizeof(term);
      x_sprintf( e, tcap, argv0 );
      *(e+strlen(e)+1) = 0;
    }
    else *e = 0;
  }
  shwrcmd.dflt_dir = shwrcmd.name = path;
  shwrcmd.environ = env ? env : envp[0];
  tail_len();
  if( mode==SHW_RUNAPP && has_mint )
  {
    child_term = 0;
    if( !pipe[0] )
    {
      for (i = 0; i < 26; i++)
      {
        termname[13] = 'A' + i;
        pipe[0] = Fcreate(termname, FA_SYSTEM|FA_HIDDEN);
        if (pipe[0] > 0) {
          /* set to non-delay mode, so Fread() won't block */
          (void)Fcntl(pipe[0], (long)0x100, 4);
          break;
        }
      }
      if( pipe[0]<=0 )
      {
        if( pipe[0]<0 ) form_error(pipe[0]);
        pipe[0] = 0;
        if( env ) Mfree(env);
        return 0;
      }
      Psignal( SIGCHLD, (void *)sigchild );
      Psignal( SIGTERM, (void *)sigterm );
      pipe[1] = pipe[0];
    }
    set_winsz();
    og = Pgetpgrp();
    Psetpgrp(0,0);
    oi = Fdup(0);
    oo = Fdup(1);
    i = Fopen(termname,2);
    Fforce( 1, i );
    Fforce( -1, i );
    Fforce( 0, i );
    Fclose(i);
    ret = Pexec( 100, path, tail, shwrcmd.environ );
    Fforce( 1, oo );
    Fforce( -1, oi );
    Fforce( 0, oi );
    Psetpgrp(Pgetpid(),og);
    if( ret<0 )
    {
      form_error(ret);
      if( env ) Mfree(env);
      return 0;
    }
    child_id = ret;
  }
  else
  {
    ret = shel_write( mode|SHD_ENVIRON, gr, set.options.s.use_argv,
       (char *)&shwrcmd, tail );
    if( mode==SHW_RUNAPP )
      if( !appl_search( X_APS_CHILD0, 0L, &dum, &id ) ) child_id = -2;
      else child_id = id;
  }
  if( env ) Mfree(env);
  return ret;
}

void set_acc( char *acc, char *program )
{
  if( *program )
  {
    strcpy( acc, "  TOS: " );
    strcat( acc, pathend(program) );
  }
  else acc = "  Geneva TOS";
  menu_register( apid, acc );
}

void do_iconify( int buf[] )
{
  wind_update( BEG_UPDATE );
  iconified = 1;
  graf_mouse( X_MRESET, 0L );
  mouse_hide = 0;
  wind_set( wind, X_WF_DIALOG, 0L );
  wind_set( wind, WF_ICONIFY, buf[4], buf[5], buf[6], buf[7] );
  wind_update( END_UPDATE );
  /* get working area of iconified window */
  wind_get( wind, WF_WORKXYWH, &icon[0].ob_x, &icon[0].ob_y,
      &icon[0].ob_width, &icon[0].ob_height );
  /* center the icon within the form */
  icon[1].ob_x = (icon[0].ob_width - icon[1].ob_width) >> 1;
  icon[1].ob_y = (icon[0].ob_height - icon[1].ob_height) >> 1;
  /* new (buttonless) dialog in main window */
  wind_set( wind, X_WF_DIALOG, icon );
  wind_name(1);
}

void do_uniconify( int buf[] )
{
  /* turn dialog off so that icon's size and position will not change */
  wind_set( wind, X_WF_DIALOG, 0L );
  wind_name(0);
  wind_set( wind, WF_UNICONIFY, buf[4], buf[5], buf[6], buf[7] );
  /* restore old dialog */
  wind_update( BEG_UPDATE );	/* prevent output while still iconified */
  wind_set( wind, X_WF_DIALOG, blank );
  iconified = 0;
  wind_update( END_UPDATE );
}

void redir_tail( int *argc, char *argv[], int i, char *path, char *name, int func(char *path) )
{
  char *p;
  int np;

  np = 1;
  argv += i;
  redir_mode = REDIR_OVER;
  if( *(p=argv[0]+1) == '>' )	/* >> */
  {
    p++;
    redir_mode = REDIR_APND;
  }
  if( *p ) strcpy( path, p );
  else if( i+1 >= *argc ) return;
  else
  {
    strcpy( path, argv[1] );
    np++;
  }
  (*func)( path );
  strcpy( name, pathend(path) );
  strcpy( pathend(path), "*.*" );
  memcpy( argv, argv+np, ((*argc-=np)-i)<<2 );
  redir_mode = 0;
}

int get_tail_len( int *argc, char *argv[] )	/* 004 */
{
  int len, i, np;
  char r, *ptr;

  for( len=0, i=2; i<*argc; i++ )
    if( (r=argv[i][0])=='>' ) redir_tail( argc, argv, i,
         redir_path, redir_name, open_redir );
    else if( r=='<' ) redir_tail( argc, argv, i,
         iredir_path, iredir_name, open_iredir );
    else len += strlen(argv[i])+1;
  return (len+2)&-2;
}

int term_child(void)
{
  if( in_prog && child_id>=0 )
  {
    if( has_mint ) Pkill( child_id, SIGKILL );
    else if( !x_appl_term( child_id, 0, 1 ) ) return 0;
    in_prog = 0;
    child_id = -1;
  }
  return 1;
}

int close_win(void)
{
  if( !in_prog ) do_quit();
  else switch( alert(ALTERMTOS) )
  {
    case 1:
      return 1;
    case 2:
      term_child();
  }
  return 0;
}

void reset_stdio(void)			/* 006 */
{
  extern int nstddh, stdh[5];
  
  stdh[0] = -1;
  stdh[1] = 0;
  stdh[2] = 1;
  stdh[3] = 2;
  stdh[4] = 3;
  nstddh = 0;
}

void main( int argc, char *argv[], char *envp[] )
{
  int dum, buf[8], i;
  char *env, fulled=0, waitkey=0, acc[30], temp2[120], **p, *s;
  static EMULTI em = { MU_MESAG|MU_KEYBD|MU_BUTTON, 2, 1, 1, 0,0,0,0,0, 0,0,0,0,0, 250 };

  linea0();
  apid = appl_init();
  redir_path[0] = Dgetdrv()+'A';
  redir_path[1] = ':';
  Dgetpath( redir_path+2, 0 );
  strcat( redir_path, "\\*.*" );
  strcpy( savepth, redir_path );
  strcpy( iredir_path, redir_path );
  strcpy( prog_path, redir_path );
  strcpy( pathend(prog_path), "gnva_tos.rsc" );
  if( rsrc_load("gnva_tos.rsc") || rsrc_load(prog_path) )
  {
    *pathend(prog_path) = 0;
    /* Check to make sure Geneva is active */
    if( CJar( 0, GENEVA_COOKIE, &cookie ) != CJar_OK || !cookie )
    {
      alert( ALNOGEN );
      do_quit();
    }
    if( (in_t2 = (char *)(cookie->xaes_funcs[111])) == 0 || *in_t2 ) in_t2 = "\1";
    has_mint = CJar( 0, MiNT_COOKIE, 0L ) == CJar_OK;
    if( has_mint )
    {
      em.type |= MU_TIMER;
      trun_fd = Fcreate("U:\\PIPE\\TOSRUN", FA_SYSTEM|FA_HIDDEN);
      if (trun_fd <= 0)	trun_fd = 0;
      else Fcntl( trun_fd, 0x100, 4 );
    }
    rsrc_gaddr( 0, TOSICON, &icon );
    rsrc_gaddr( 0, MAIN, &menu );
    menu[0].ob_state |= X_MAGIC;
  }
  else
  {
    form_alert( 1, "[1][GNVA_TOS.RSC|not found][OK]" );
    do_quit();
  }
  if( argc<2 )
  {
    program = "";
    tail = "\0";
  }
  else
  {
    program = argv[1];
    if( (tail = (char *)Malloc(get_tail_len(&argc,argv))) != 0 )
    {
      tail[0] = tail[1] = 0;
      for( i=2; i<argc; i++ )
      {
        strcat( tail, " " );
        strcat( tail, argv[i] );
      }
      tail_len();
    }
    else tail = "\0";
  }
  i_ttp();
  if( program[0] )	/* 004: find program and set in prog_path if valid */
  {
    strcpy( temp2, program );
    if( temp2[0]==':' || shel_find( temp2 ) ) strcpy( prog_path, temp2 );
  }
  force_40( apid, 0 );
  single = _GemParBlk.global[1] != -1;
  vdi_hand = graf_handle( &dum, &char_h, &dum, &dum );
  work_in[0] = Getrez() + 2;
  v_opnvwk( work_in, &vdi_hand, work_out );
  if( !vdi_hand )
  {
    alert( ALVDI );
    do_quit();
  }
  vdi_reset();
  rez = 0;
  wind_get( 0, WF_WORKXYWH, &max.g_x, &max.g_y, &max.g_w, &max.g_h );
  if( max.g_w > 320 )
  {
    rez = 1;
    if( max.g_h > 200-11 ) rez = 2;
  }
  load_settings();
  _vswr_mode( MD_REPLACE );
  _vsf_color( 0 );
  initialize();
  if( !new_scroll() ) do_quit();
  if( open_wind() )
  {
    io = Iorec(1);
    set_traps(0);
    set_linea();
    vcurs_on(1);
    env = envp[0];
    strcpy( prog_name, pathend(prog_path) );	/* 004: was (program) */
    strcpy( pathend(prog_path),"*.*" );
new_prog:
    if( single && program[0] && !has_mint )
    {
      tail[0] = (char)strlen(tail+1);
      child_id = -1;
      reset_stdio();	/* 006 */
      if( (buf[4] = (int)Pexec( 0, program, tail, env )) < 0 )
          al_err( ALPRGOP2, buf[4] );
      goto ch_exit;
    }
    shel_write( SHW_MSGTYPE, 1, 0, 0L, 0L );
    *pathend(argv[0]) = 0;
    set_acc( acc, program );
    wind_name(0);
    child_id = -1;
    if( program[0] )
    {
      reset_stdio();	/* 006 */
      if( !run_prg( SHW_RUNAPP, 1/*006: was 0*/, program, tail, envp, argv[0] ) ) goto quit;
      in_prog = 1;
    }
    for(;;)
    {
      if( has_mint )
      {
        if( read_pipe(0) ) goto open_prog;
        if( child_term )
        {
          buf[4] = child_ret;
          goto ch_term;
        }
      }
      multi_evnt( &em, buf );
      if( em.event & MU_MESAG )
      {
        switch( buf[0] )
        {
          case WM_FULLED:
            if( !fulled ) *(GRECT *)&buf[4] = max;
            else wind_get( wind, WF_PREVXYWH, &buf[4], &buf[5],
                &buf[6], &buf[7] );
            fulled ^= 1;
          case WM_SIZED:
            wsize = *(GRECT *)&buf[4];
            new_size();
            curs_bottom(1);
            break;
          case AC_CLOSE:
            wind = 0;
            iconified = 0;
            em.type |= (MU_KEYBD|MU_BUTTON);
            break;
          case WM_MOVED:
            if( !iconified )
            {
              wsize = *(GRECT *)&buf[4];
              new_size();
            }
            else resize( (GRECT *)&buf[4] );
            break;
          case WM_TOPPED:
            is_top = 1;
            wind_set( buf[3], WF_TOP );
            break;
          case CH_EXIT:
ch_exit:    if( child_id<0 || buf[3]==child_id )
            {
ch_term:      if( buf[4]!=0 )
              {
                in_prog = 0;
                flush_pipe();		/* 004 */
                force_cono(0);		/* 004 */
                rsrc_gaddr( 15, RETURN, &p );
                doecho( "\r\n\033p" );
                x_sprintf( temp2, *p, buf[4] );
                doecho(temp2);
                doecho( "\033q\r\n" );
                force_cono(1);		/* 004 */
              }
              child_term = 0;
              goto quit;
            }
            break;
          case AP_TERM:
ap_term:    if( !term_child() ) break;
	          waitkey = -1;
quit:       child_id = -1;
            in_prog = 0;
            reset_linea();
            vcurs_off();
            set_traps(1);
            set_acc( acc, "" );
            if( ++waitkey )
              if( set.options.s.no_pause ) do_quit();
              else
              {
                flush_pipe();
                force_cono(0);
                rsrc_gaddr( 15, TERMTXT, &p );
                doecho( "\r\n\033p" );
                doecho( *p );
                doecho( "\033q\r\n" );
                force_cono(1);
                em.event &= ~MU_KEYBD;	/* 004 */
              }
            else do_quit();
            break;
          case WM_ONTOP:
            is_top = 1;
            draw_current(0);
            break;
          case WM_UNTOPPED:
            is_top = 0;
            break;
          case WM_CLOSED:
            if( close_win() ) goto ap_term;
            break;
          case WM_ICONIFY:
          case WM_ALLICONIFY:
            if( buf[3]==wind && !iconified ) /* main window */
            {
              do_iconify(buf);
              em.type &= ~(MU_KEYBD|MU_BUTTON);
            }
            break;
          case WM_UNICONIFY:
            if( buf[3]==wind && iconified ) /* main window */
            {
              do_uniconify(buf);
              em.type |= (MU_KEYBD|MU_BUTTON);
            }
            break;
          case X_MN_SELECTED:
            if( !iconified ) switch( buf[4] )
            {
              case MABOUT:
                do_about();
                break;
              case MOPEN:
                strcpy( pathend(prog_path), "*.*" );
                if( select( OPENPATH, prog_path, sizeof(prog_path), prog_name, open_prog ) &&
                    prog_name[0] )
open_prog:        if( !in_prog )
                  {
                    set_traps(0);
                    set_linea();
                    vcurs_on(1);
                    program=prog_path;
                    waitkey = 0;
                    menu_tnormal( menu, buf[3], 1 );
                    goto new_prog;
                  }   /* probably new instance of GNVA_TOS */
                  else if( !run_prg( SHW_RUNANY|SHD_DFLTDIR, 1,
                      prog_path, tail, envp, argv[0] ) ) alert( ALPRGOPEN );
                break;
              case MQUIT:
                if( close_win() )
                {
                  menu_tnormal( menu, buf[3], 1 );
                  goto ap_term;
                }
                break;
              case MOUTPUT:
                if( redir_hand!=2 ) close_redir();
                else select( OUTPATH, redir_path, sizeof(redir_path), redir_name, open_redir );
                break;
              case MINPUT:
                if( iredir_hand!=2 ) close_iredir();
                else select( INPATH, iredir_path, sizeof(iredir_path), iredir_name, open_iredir );
                break;
              case MSAVE:
                save_settings();
                break;
              case MRELOAD:
                load_settings();
                break;
              case MFONT:
                get_font();
                break;
              case MNAME:
                set.win_type ^= NAME|MOVER|CLOSER|FULLER|SMALLER;
                close_wind();
                size_wind();
                open_wind();
                break;
              case MHORIZ:
                set.win_type ^= LFARROW|RTARROW|HSLIDE;
                if( !(set.win_type&(HSLIDE|VSLIDE)) ) set.win_type &= ~SIZER;
                else set.win_type |= SIZER;
                close_wind();
                size_wind();
                open_wind();
                break;
              case MVERT:
                set.win_type ^= UPARROW|DNARROW|VSLIDE;
                if( !(set.win_type&(HSLIDE|VSLIDE)) ) set.win_type &= ~SIZER;
                else set.win_type |= SIZER;
                close_wind();
                size_wind();
                open_wind();
                break;
              case MSCROLL:
                get_scroll();
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
              case MMISC:
                miscopts();
                break;
              case MHELP:
                do_help( "Geneva TOS" );
                break;
            }
            menu_tnormal( menu, buf[3], 1 );
            break;
        }
      }
      i = em.mouse_k&3;
      if( em.event & MU_KEYBD ) {
        if (em.mouse_k & 4 && set.options.s.use_hot) {
          switch (em.key & 0xFF00) {
            case 0x5000:
              scroll(i ? WA_DNPAGE : WA_DNLINE);
              break;
            case 0x4800:
              scroll(i ? WA_UPPAGE : WA_UPLINE);
              break;
            case 0x7300:
              scroll(i ? WA_LFPAGE : WA_LFLINE);
              break;
            case 0x7400:
              scroll(i ? WA_RTPAGE : WA_RTLINE);
              break;
            case 0x7700:
              curs_bottom(0);
              break;
            default:
              if (waitkey) do_quit();
              if (in_prog) unselect(1);
              evkey = em.key;
              if (has_mint) write_key();
          }
        }
        else {
          if (waitkey) do_quit();
          if (in_prog) unselect(1);
          evkey = em.key;
          if (has_mint) write_key();
        }
      }
      if( em.event&MU_BUTTON )
      {
        if( em.times==2 )
        {
          select_word( em.mouse_x, em.mouse_y );
          but_up();
        }
        else sel_mouse( em.mouse_x, em.mouse_y, em.mouse_b, em.mouse_k );
        set_entries();
      }
    }
  }
  else do_quit();
}
