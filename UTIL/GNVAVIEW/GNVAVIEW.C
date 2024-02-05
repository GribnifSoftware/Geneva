#include "new_aes.h"
#include "vdi.h"
#include "xwind.h"
#include "tos.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include <ctype.h>
#include "multevnt.h"
#include "gnvaview.h"
typedef struct { int x, y, w, h; } Rect;
#include "..\graphics.h"
#include <neo_acc.h>
#include "..\font_sel.h"

#define TOSSET_VER	0x100
#define WIN_TYPE0 NAME|MOVER|CLOSER|FULLER|SIZER|UPARROW|DNARROW|VSLIDE|LFARROW|RTARROW|HSLIDE|SMALLER

#define MODE_ALL -1
#define MODE_TEXT 1
#define MODE_SEL  2
#define MODE_EVERY 4

char draw_mode=-1,
     in_prog, in_echo, iconified,
     my_name[9],
     prog_name[13], savename[13],
     prog_path[120],
     savepth[120],
     paused, in_sel,
     **more,
     *program,
     *screen,           /* start of screen buffer */
     *scrn_ptr;         /* start of first line of visible screen */
int corn_x, corn_y,	/* start of screen visible to user */
    wind, min_wid, min_ht, scrn_wid,
    line,		/* current line of VT-52 */
    scrn_line,		/* top line of VT-52 screen */
    filerow,		/* counter for More */
    rstart, rend, apid, sel_row, sel_col, sele_row, sele_col,
    neo_apid, retries;
Rect clip_rect;
long screen_size;

long bcon_nul;
char last_esc, inverse, discard, ctrlc, gt_ign_cr, ign_ctrlc,
    is_top, ichar_cr, insert, fileerr, readall, waitkey;
int row, col, saver, savec, lines, linelen,
    lastcol, lastrow, rez;
int work_in[] = { 1, 7, 1, 1, 1, 1, 1, 1, 1, 1, 2 };
int oarray[4];
IOREC *io;
GRECT winner, max, wsize;
int cdecl draw_text( PARMBLK *pb );
USERBLK ub_main = { draw_text };
OBJECT blank[] = { -1, -1, -1, G_USERDEF, TOUCHEXIT, 0, (long)&ub_main },
    *menu, *icon;
G_COOKIE *cookie;
NEO_ACC *neo_acc=0L;                    /* points to NeoDesk's functions */

long bconin( int hand );
void doecho( char *ptr );
void get_inner(void);
void get_corner(void);
int gtext( char *string );
void draw_current(void);
void draw_range( int start, int end );
void breakpoint(void);
void do_quit(void);
long setexc( int number, void (*new)(), void (**old)(), int force );
void unselect( int draw );
int new_scroll( int new, int lns, int llen );
void v_ftext16_mono( int handle, int x, int y, int *wstr, int strlen, int offset );
void wind_name( int end );

struct Set
{
  int ver;
  int win_type;
  union
  {
    struct
    {
      unsigned hex    :1;
      unsigned readall:1;
      unsigned feed   :1;
    } s;
    int i;
  }
  options;
  int font[3], point[3];
  int rows[3], cols[3];
  int xoff[3], yoff[3];
  int linelen[3], lines[3];
} set = { TOSSET_VER, WIN_TYPE0, { {0,0,1} },  1, 1, 1,  8, 9, 10,
          18, 18, 20,  45, 75, 75,  0, 0, 0,  0, 0, 0,  80, 80, 80,  100, 100, 100 };

#define CJar_xbios      0x434A          /* "CJ" */
#define CJar_OK         0x6172          /* "ar" */
#define CJar(mode,cookie,value)         (int)xbios(CJar_xbios,mode,cookie,value)

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

  if( (l = (scrn_ptr-screen) / scrn_wid + scrn_line + row) >= lines ) l -= lines;
  return( screen+(long)l*scrn_wid+col );
}

char *get_scr2( int row, int col )
{
  return get_scr( row-scrn_line, col );
}

void clear_screen(void)
{
  int i, j;
  char *ptr;

  row=col=line=corn_y=scrn_line=0;	/* 004: added corn_y, scrn_line */
  scrn_ptr = screen;
  for( i=-1, ptr=scrn_ptr; ++i<lines; )
  {
    memset( ptr, ' ', j=linelen );
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
      for( ptr=get_scr( i, 0 ), j=-1; ++j<linelen; )
        *ptr++ = ' ';
      while( j++<scrn_wid ) *ptr++ = '\0';
    }
    if( !iconified )
      if( is_top )
      {
        r.x = winner.g_x;
        r.y = blank[0].ob_y + (slin+scrn_line)*char_h;
        r.w = winner.g_w;
        r.h = (elin-slin+1)*char_h;
        blank_box( &r, MD_REPLACE );
      }
      else draw_range( slin, elin );
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

  ptr0 = (ptr = get_scr( row, 0 )) + linelen-1 + 1;
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
      r.w = (ecol+1)*char_w;
      r.h = char_h;
      blank_box( &r, MD_REPLACE );
    }
    else draw_current();
}

void yield(void)
{
/*  appl_yield();*/
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

int check_blank( int pw, int ph, long xy )
{
  int i;

  if( blank[0].ob_x < (i=winner.g_x+pw-blank[0].ob_width) ) blank[0].ob_x = i;
  else if( blank[0].ob_x > (i=winner.g_x) ) blank[0].ob_x = i;
  if( blank[0].ob_y < (i=winner.g_y+ph-blank[0].ob_height) ) blank[0].ob_y = i;
  else if( blank[0].ob_y > (i=winner.g_y) ) blank[0].ob_y = i;
  return *(long *)&blank[0].ob_x != xy;
}

void scroll( int dir )
{
  int msg[8], i, w, h;
  long xy;

  if( !iconified )
  {
    if( (dir < WA_LFPAGE ? set.win_type&VSLIDE : set.win_type&HSLIDE) != 0 )
    {
      msg[0] = WM_ARROWED;
      msg[3] = wind;
      msg[4] = dir;
      shel_write( SHW_SENDTOAES, 0, 0, (char *)msg, 0L );
    }
    else
    {
      get_corner();
      h = set.rows[rez]*char_h;
      w = set.cols[rez]*char_w;
      xy = *(long *)&blank[0].ob_x;
      switch(dir)
      {
        case WA_UPPAGE:
          blank[0].ob_y += h;
          break;
        case WA_DNPAGE:
          blank[0].ob_y -= h;
          break;
        case WA_UPLINE:
          blank[0].ob_y += char_h;
          if( is_top )
          {
            if( !check_blank( w, h, xy ) ) return;
            blit_lines( 0, set.rows[rez]-2, 1, 0 );
            get_corner();
            draw_range( corn_y, corn_y );
            return;
          }
          break;
        case WA_DNLINE:
          blank[0].ob_y -= char_h;
          if( is_top )
          {
            if( !check_blank( w, h, xy ) ) return;
            blit_lines( 1, i=set.rows[rez]-1, 0, 0 );
            get_corner();
            i += corn_y;
            draw_range( i, i );
            return;
          }
          break;
        case WA_LFPAGE:
          blank[0].ob_x += w;
          break;
        case WA_RTPAGE:
          blank[0].ob_x -= w;
          break;
        case WA_LFLINE:
          blank[0].ob_x += char_w;
          break;
        case WA_RTLINE:
          blank[0].ob_x -= char_w;
      }
      if( !check_blank( w, h, xy ) ) return;
      x_wdial_draw( wind, 0, 0 );
    }
  }
/*  else if( dir==WA_DNLINE ) blank[0].ob_y -= char_h;*/
}

void _calc_blank(void)
{
  int i;

  blank[0].ob_width = ((i=set.cols[rez])>linelen ? i : linelen)*char_w;
  blank[0].ob_height = ((i=set.rows[rez])>lines ? i : lines)*char_h;
}

void set_dial(void)
{
  if( !iconified )
  {
    wind_update(BEG_UPDATE);
    if( set.win_type&VSLIDE ) wind_set( wind, X_WF_DIALOG, blank );
    else x_wdial_draw( wind, 0, 0 );
    wind_update(END_UPDATE);
  }
}

int inc_buffer(void)
{
  int lns;
  long l;

  if( (lns = lines+20) >= max_ht )
    if( (lns = max_ht-1) == lines ) return 1;
  l = scrn_ptr - screen;
  if( !new_scroll( 0, lns, linelen ) ) return 2;
  scrn_ptr = l+screen;
  return 0;
}

void newline(void)
{
  char c;
  int r, r2;
  long l;

  if( ctrlc ) return;
  col = 0;
  get_corner();
  filerow++;
  if( row >= set.rows[rez]-1 )
    if( readall && line >= lines-2 )	/* leave enough room for More */
      if( (fileerr=inc_buffer()) == 0 )
          erase_lines( ++line-scrn_line, lines-scrn_line-1 );
      else
      {
        readall = 0;
        wind_name(1);
        line++;
      }
    else if( line >= lines-1 )
    {
      inc_scrn( &scrn_ptr );
      r = line - scrn_line;
      if( is_top )
      {
        blit_lines( 1, r2=set.rows[rez]-1, 0, 0 );
        erase_lines( r, r );
        if( (r2+=corn_y)!=scrn_line+r ) draw_range( r2, r2 );
        yield();
      }
      else
      {
        erase_lines( r, r );
        set_dial();
      }
    }
    else
    {
      line++;
      if( corn_y == scrn_line ) scroll( WA_DNLINE );
    }
  else line++;
  get_corner();
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
  ivmouse(0);
  ptr = string-1;
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
/*      do
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
cc:         ivmouse(1);
            if( paused>0 ) paused = 0;
            in_echo = 0;
            if( map ) _vst_charmap( 1 );
            return(0);
        }
        if( paused ) yield();
      }
      while( paused ); */
      not_end = 0;
      if( (l = strlen(string)) != 0 )
      {
        max = linelen-1 - col + 1;
        if( (not_end = l>max) != 0 )
          if( (l = max) <= 0 ) goto wrp;
        memcpy( ptr2=get_scr( row, col ), string, l );
        for( ptr2+=max, i=col+l; --i>=col; )
          if( !inverse ) *(ptr2+(i>>3)) &= ~(1<<(i&7));
          else *(ptr2+(i>>3)) |= 1<<(i&7);
        if( !iconified && y>=0 )
          if( !is_top ) draw_current();
          else
          {
            _vswr_mode(MD_REPLACE);
            if( is_scalable )
            {
              w = l;
              text_2_arr( string, &w );
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
/*            if( ctrlc ) goto cc;*/
            if( ctrlc ) goto end;
          }
          else col = linelen-1;
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
/*        if( ctrlc ) goto cc; */
        if( ctrlc ) goto end;
      }
      else col=0;
    }
  }
  while(ptr);
end:
  ivmouse(1);
  in_echo=0;
  if( map ) _vst_charmap( 1 );
  return(1);
}

void check_rowcol(void)
{
  int i;

  if( row>(i=set.rows[rez]-1) ) row = i;
  else if( row<0 ) row=0;
  if( col>(i=linelen-1) ) col = i;
  else if( col<0 ) col=0;
  line = row + scrn_line;
}

void doecho( char *ptr )
{
  register int i, j;
  register char c, oldc, nul;
  register char *ptr2, *ptr3;
  static char esc_str[] = "\033\b\t\a\f";

      nul = bcon_nul>0;
      while( *ptr || nul )
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
          ptr3 = ptr2;
          ptr3++;
          if( c=='\033' )
            if( bcon_nul==1 )
            {
              last_esc=1;
              break;
            }
            else
            {
              ptr3++;
              bcon_nul--;
            }
        }
        if( c=='\b' ) goto escD;
        if( c=='\t' )
        {
          col = (col&0xFFF8) + 8;
          check_rowcol();
        }
        else if( c=='\a' )
        {
          if( !iconified ) Bconout( 2, 7 );
        }
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
/*            if( row )
            {
              row--;
              line--;
            } */
            break;
          case 'B':
            row++;
            check_rowcol();
/*            if( row < set.rows[rez]-1 )
            {
              row++;
              line++;
            } */
            break;
          case 'C':
/*            if( col < linelen-1 ) col++; */
            col++;
            check_rowcol();
            break;
          case 'D':
escD:       /*if( col ) col--;*/
	    col--;
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
            erase_ln( col, linelen-1 );
            erase_lines( row+1, set.rows[rez]-1 );
            break;
          case 'K':       /* erase to eol */
escK:       erase_ln( col, linelen-1 );
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
            erase_lines( 0, row-1 );
            erase_ln( 0, col );
            break;
          case 'e':
            break;
          case 'f':
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
nextch: if( nul )
          if( --bcon_nul<=0 ) break;
      }
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
/*      errs( AEINVFN, "Control-C pressed." );*/
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

char *next_str( char *ptr )
{
  return( ptr + strlen(ptr) + 1 );
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
}

void redraw_lines( int first, int last )
{
  int i, j, k, y, px[4], l;
  char *ptr, *end, e;

  if( is_scalable ) _vst_charmap( 0 );
  l = linelen;
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
        text_2_arr( ptr, px );
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

  _vs_clip( 1, (Rect *)&pb->pb_xc );
  if( !readall )
  {
  i = (pb->pb_yc - pb->pb_y)/char_h;
  if( (max = i + (pb->pb_hc+char_h-1)/char_h) > lines ) max = lines;
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
  redraw_lines( i, max );
  }
  else blank_box( (Rect *)&pb->pb_x, MD_REPLACE );
  _vs_clip( 1, (Rect *)&winner );
  return pb->pb_currstate;
}

void draw_range( int start, int end )
{
  int buf[8];
  char last;

  if( iconified ) return;
  rstart = start;
  rend = end;
  wind_update( BEG_UPDATE );
  last = draw_mode;
  draw_mode = ~MODE_EVERY;
  buf[0] = WM_REDRAW;
  buf[3] = wind;
  *(GRECT *)&buf[4] = winner;
  appl_write( buf[1]=apid, 16, buf );
  draw_mode = last;
  wind_update( END_UPDATE );
}

void draw_current(void)
{
  draw_range( line, line );
}

void calc_blank(void)
{
  _calc_blank();
  if( wind>0 )
  {
    wind_set( wind, X_WF_DIALWID, char_w );
    wind_set( wind, X_WF_DIALHT, char_h );
  }
}

char *pathend( char *ptr )
{
  char *s;

  if( (s=strrchr(ptr,'\\')) == 0 ) return(ptr);
  return(s+1);
}

void wind_name( int end )
{
  int i;
  char **p, *s, bar[11];
  static char temp[40];

  s = pathend(program);
  if( !in_prog && !line || !end ) strcpy( temp, s );
  else
  {
    if( end>=2 )
    {
      bar[10] = 0;
      for( i=10; --i>=0; )
        bar[i] = i<end-2 ? '�' : '-';
    }
    else bar[0] = 0;
    rsrc_gaddr( 15, waitkey ? SEOF : (readall ? LOADING : PAGING), &p );
    x_sprintf( temp, *p, s, bar );
  }
  wind_set( wind, WF_NAME, temp );
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
      _calc_blank();	/* WF_DIALOG will corrupt this if sliders off */
      wind_set( i, X_WF_MENU, menu );
      wind_name(1);
      wind_get( i, X_WF_MINMAX, &min_wid, &min_ht, &dum, &dum );
      is_top = 1;
      wind_open( i, wsize.g_x, wsize.g_y, wsize.g_w, wsize.g_h );
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
  if( always || flag ) set_dial();
}

int intersect( Rect r1, Rect r2, Rect *res )
{
  res->x = r1.x < r2.x ? r2.x : r1.x;
  res->y = r1.y < r2.y ? r2.y : r1.y;
  res->w = r1.x+r1.w < r2.x+r2.w ? r1.x+r1.w-res->x : r2.x+r2.w-res->x;
  res->h = r1.y+r1.h < r2.y+r2.h ? r1.y+r1.h-res->y : r2.y+r2.h-res->y;
  return( res->w > 0 && res->h > 0 );
}

void get_font(void)
{
  if( font_sel( FONTOPTS, SYSTEM, &set.font[rez], &set.point[rez] ) )
  {
    new_size();
    curs_bottom(1);
  }
}

long calc_scroll( int lines, int linelen )
{
  return (lines*(long)(scrn_wid=linelen+((linelen+7)>>3))+1L)&-2L;
}

void scr_nums( OBJECT *o, int draw )
{
  x_sprintf( o[SOCNUM].ob_spec.tedinfo->te_ptext, "%d", set.linelen[rez] );
  x_sprintf( o[SOLNUM].ob_spec.tedinfo->te_ptext, "%d", set.lines[rez] );
  x_sprintf( o[SOMEM].ob_spec.tedinfo->te_ptext, "%D",
      calc_scroll( set.lines[rez], set.linelen[rez] ) );
  if( draw&1 ) objc_draw( o, SOCNUM, 0, 0, 0, 0, 0 );
  if( draw&2 ) objc_draw( o, SOLNUM, 0, 0, 0, 0, 0 );
  if( draw ) objc_draw( o, SOMEM, 0, 0, 0, 0, 0 );
}

void get_scrnum( OBJECT *o )
{
  if( (set.linelen[rez] = atoi( o[SOCNUM].ob_spec.tedinfo->te_ptext )) < 80 )
  {
    set.linelen[rez] = 80;
    scr_nums( o, 1 );
  }
  if( (set.lines[rez] = atoi( o[SOLNUM].ob_spec.tedinfo->te_ptext )) > max_ht )
  {
    set.lines[rez] = max_ht;
    scr_nums( o, 2 );
  }
  else if( set.lines[rez] < 24 )
  {
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

int new_scroll( int new, int lns, int llen )
{
  long l;

  l = calc_scroll( lines = lns, linelen = llen );
  if( screen )
  {
    if( x_realloc( (void **)&screen, l ) )
    {
      alert( ALNOMEM );
      return 0;
    }
  }
  else
  {
    x_malloc( (void **)&screen, l );
    if( !screen )
    {
      alert( ALNOMEM );
      return 0;
    }
  }
  screen_size = l;
  if( new ) void clear_screen();
  return 1;
}

void home_dial(void)
{
  blank[0].ob_x = winner.g_x;
  blank[0].ob_y = winner.g_y;
  set_dial();
}

void get_scroll(void)
{
  OBJECT *o;
  int old_ll, old_l, ret;

  old_ll = set.linelen[rez];
  old_l = set.lines[rez];
  rsrc_gaddr( 0, SCRLOPTS, &o );
  scr_nums( form=o, 0 );
  set_if( SOALL-1, !set.options.s.readall );
  set_if( SOALL, set.options.s.readall );
  ret = make_form( SCRLOPTS, 0, 0L, scrlup );
  get_scrnum(o);
  if( ret != SOOK || !new_scroll( 1, set.lines[rez], set.linelen[rez] ) )
  {
    set.linelen[rez] = old_ll;
    set.lines[rez] = old_l;
  }
  else
  {
    set.options.s.readall = o[SOALL].ob_state&SELECTED;
    if( set.linelen[rez] != old_ll || set.lines[rez] != old_l )
    {
      new_size();
      home_dial();
    }
  }
}

void get_outer(void)
{
  int w, h;

  winner.g_x = (winner.g_x+(char_w>>1))/char_w*char_w;
  for(;;)
  {
    if( (w = (winner.g_w+(char_w>>1))/char_w) > linelen ) w = linelen;
    if( (h = (winner.g_h+(char_h>>1))/char_h) > lines ) h = lines;
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
  if( scrn_line > (h=line-set.rows[rez]+1) ) scrn_line = h;
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
  int buf[8];

  if( !_app && _GemParBlk.global[1]!=-1 ) for(;;)
  {
    evnt_mesag(buf);
    if( buf[0]==AP_TERM ) break;
  }
  if( screen ) x_mfree(screen);
  close_wind();
  if( vdi_hand )
  {
    no_fonts();
    v_clsvwk(vdi_hand);
  }
  rsrc_free();
  appl_exit();
  exit(0);
}

void save_settings(void)
{
  int ret, i;
  char temp[100];

  graf_mouse( BUSYBEE, 0L );
  if( x_shel_put( X_SHOPEN, "Geneva View" ) > 0 )
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
  while( (i=x_shel_get( X_SHOPEN, 0, "Geneva View" )) == -1 );
  if( i > 0 && x_shel_get( X_SHACCESS, sizeof(temp)-1, temp ) > 0 )
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
  make_form( ABOUT, 0, 0L, 0L );
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
    else if( (i=alert( ALEXISTS )) == 3 ) return 0;
    else if( i==2 )
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

int open_prog( char *path )
{
  strcpy( prog_path, path );
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
/*  menu_ienable( menu, MPBLOCK, i );*/
  in_sel = i ? -1 : 0;
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
  if( x<0 || y>=lines ) return;
  for( li=0, i=0, s=get_scr2(y,0); i<linelen; s++, i++ )
  {
    if( is_sep(*s) ) li = i;
    if( !x-- ) break;
  }
  if( i==linelen || is_sep(*s) ) return;
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

  xmax = linelen;
  ymax = lines;
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
    imax = linelen-1;
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
    if( clip==2 )	/* printer */
    {
      if( (h=Fopen("PRN:",1)) < -3 ) return;
    }
    else if( clip==1 )
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
    if( clip==2 && set.options.s.feed ) Fwrite( h, 1L, "\f" );
    Fclose(h);
  }
}

void do_help( char *topic )
{
  char path[120], temp[120], *p, *t;

  strcpy( path, "GNVAVIEW.HLP" );
  if( !shel_find(path) ) strcpy( path, "GNVAVIEW.HLP" );
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

void set_acc( char *program )
{
  char **p;
  static char acc[30], did_reg;

  if( *program )
  {
    rsrc_gaddr( 15, ACCFILE, &p );
    strcpy( acc, *p );
    strcat( acc, pathend(program) );
  }
  else
  {
    rsrc_gaddr( 15, ACCNAME, &p );
    strcpy( acc, *p );
  }
  if( !did_reg || _app ) menu_register( apid, acc );
  did_reg = 1;
}

void do_iconify( int buf[] )
{
  wind_update( BEG_UPDATE );
  iconified = 1;
  graf_mouse( X_MRESET, 0L );
  mouse_hide = 0;
  wind_set( wind, X_WF_DIALOG, 0L );
  wind_set( wind, WF_ICONIFY, buf[4], buf[5], buf[6], buf[7] );
  wind_name(0);
  wind_update( END_UPDATE );
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
  /* turn dialog off so that icon's size and position will not change */
  wind_set( wind, X_WF_DIALOG, 0L );
  wind_name(1);
  wind_set( wind, WF_UNICONIFY, buf[4], buf[5], buf[6], buf[7] );
  /* restore old dialog */
  wind_update( BEG_UPDATE );	/* prevent output while still iconified */
  iconified = 0;
  wind_set( wind, X_WF_DIALOG, blank );
  _calc_blank();	/* WF_DIALOG will corrupt this if sliders off */
  wind_update( END_UPDATE );
}

void cls(void)
{
  calc_blank();
  clear_screen();
  home_dial();
}

char filebuf[513]  /* mult of 8 + 1 */, *fileptr;
long filelen, filepos, filesize;
int filestat;

void quitclose(void)
{
  if( _app ) do_quit();
  else close_wind();
}

void page_key( int sh, int key )
{
  int i = sh&3;

  switch( key&0xFF00 )
  {
    case 0x2E00:	/* ^C */
      if( sh==4 ) quitclose();
      return;
    case 0x1000:	/* Q or q */
      if( !(sh&0xC) ) quitclose();
      return;
    case 0x0100:	/* Esc */
    case 0x6100:	/* Undo */
      if( !sh ) quitclose();
      return;
    case 0x4800:	/* up arrow */
      scroll( i ? WA_UPPAGE : WA_UPLINE );
      return;
    case 0x5000:	/* down arrow */
      scroll( i ? WA_DNPAGE : WA_DNLINE );
      return;
    case 0x4b00:
      scroll( i ? WA_LFPAGE : WA_LFLINE );
      return;
    case 0x4d00:
      scroll( i ? WA_RTPAGE : WA_RTLINE );
      return;
    case 0x4700:	/* Clr/Home */
      if(i) curs_bottom(0);
      else home_dial();
      return;
    case 0x3900:	/* Space */
      if( waitkey ) quitclose();
      break;
    case 0x3200:	/* ^M */
      if( sh!=4 )
      {
        if( waitkey ) quitclose();
        return;
      }
    case 0x1C00:
      filerow = set.rows[rez]-2;
      break;
    default:
      if( waitkey ) quitclose();
      return;
  }
  unselect(1);
  if( paused )
  {
    paused = 0;
    doecho( "\033K" );	/* erase More */
  }
}

void echo_lines(void)
{
  char *p2, buf[80], temp[2]="X", stop=0;
  unsigned char *p;
  long l;
  int i, j;

  get_corner();
  if( !paused && !in_sel && !iconified )
  {
    if( readall )
    {
      if( (Kbshift(-1)&3)==3 ) stop = 1;
      i = filesize ? filepos*11L/filesize : 0;
      if( i != filestat ) wind_name( (filestat=i)+2 );
      iconified = 1;
    }
    while( filelen>0 && !fileerr )
    {
      if( set.options.s.hex )
      {
        gt_ign_cr = 1;
        if( (i = filepos&0xf) == 0 && filelen>=16 )
        {
          p = fileptr + 16;
          x_sprintf( buf, "%06X  %02x %02x %02x %02x %02x %02x %02x %02x\
 %02x %02x %02x %02x %02x %02x %02x %02x  ", filepos, *--p, *--p, *--p, *--p, *--p,
                *--p, *--p, *--p, *--p, *--p, *--p, *--p, *--p, *--p, *--p, *--p  );
          for( i=16, p2=buf+57; --i>=0; p++ )
            *p2++ = *p ? *p : '\x10';
          *p2 = 0;
          gtext(buf);
          filelen -= 16;
          filepos += 16;
        }
        else
        {
          if( !i )
          {
            x_sprintf( buf, "%06X", filepos );
            col = 0;
            gtext(buf);
          }
          for( j=i, p=fileptr; j<16; p++, j++ )
          {
            col = 8 + j*3;
            x_sprintf( buf, "%02x", *p );
            gtext(buf);
            col = 57 + j;
            if( (temp[0] = *p) == 0 ) temp[0] = '\x10';
            gtext(temp);
            filepos++;
            if( !--filelen ) break;
          }
        }
        if( !(filepos & 0xf) ) newline();
        gt_ign_cr = 0;
      }
      else
      {
        for( p=fileptr, l=0; (*p=='\r'||!iscntrl(*p)) && l<filelen; p++, l++ );
        if( l<filelen )
        {
          l++;
          p++;
        }
        bcon_nul = l;
        doecho(fileptr);
        filelen -= l;
        filepos += l;
      }
      fileptr = p;
      if( stop )
      {
        readall = 0;
        wind_name(1);
        fileerr = 4;
      }
      if( fileerr || !readall && filerow>=set.rows[rez]-1 )
      {
        filerow = 0;
        temp[0] = inverse;
        i = savec;
        j = saver;
        doecho( "\033j\033p" );
        gtext( *more );
        doecho( "\033q\033k" );
        savec = i;
        saver = j;
        inverse = temp[0];
        paused = 1;
        break;
      }
    }
    iconified = 0;
  }
}

int open_file( int file )
{
  DTA dta;

  if( file<=0 && program[0] )
    if( (file = Fopen(program,0)) > 0 )
    {
      Fsetdta(&dta);
      if( !Fsfirst(program,0x37) ) filesize = dta.d_length;
      else filesize = 0L;
      filelen = filepos = 0L;
      in_prog = 1;
      filerow = 0;
      filestat = -1;
      fileerr = ctrlc = 0;
      readall = set.options.s.readall;
      wind_name(1);
      set_acc( program );
    }
    else
    {
      al_err( ALOPEN, file );
/*      program[0] = 0; */
    }
  return file;
}

void check_feed(void)
{
  menu_icheck( menu, MFEED, set.options.s.feed );
}

void check_hex(void)
{
  menu_icheck( menu, MHEX, set.options.s.hex );
}

void ack(void)                                   /* acknowledge NeoDesk's message */
{
  int buf[8];

  buf[0] = DUM_MSG;                     /* always ack with this message */
  appl_write( buf[1]=neo_apid, 16, buf );
}

void find_neo( int *buf )
{
  /* throw away if too many tries */
  if( ++retries>10 ) return;
  /* try to find NeoDesk */
  if( (neo_apid = appl_find("NEODESK ")) >= 0 )
  {
    /* put the message back in my queue */
    appl_write( apid, 16, buf );
    buf[0] = NEO_ACC_ASK;         /* 89 */
    buf[1] = apid;
    buf[3] = NEO_ACC_MAGIC;
    buf[4] = apid;
    appl_write( neo_apid, 16, buf );
  }
}

void main( int argc, char *argv[] )
{
  int dum, buf[8], i, file=0;
  char fulled=0, acc[30], temp2[120], **p, *s;
  static EMULTI em = { MU_MESAG|MU_KEYBD|MU_BUTTON, 2, 1, 1 };

  apid = appl_init();
  prog_path[0] = Dgetdrv()+'A';
  prog_path[1] = ':';
  Dgetpath( prog_path+2, 0 );
  strcat( prog_path, "\\*.*" );
  strcpy( savepth, prog_path );
  if( rsrc_load("gnvaview.rsc") )
  {
    *pathend(prog_path) = 0;
    /* Check to make sure Geneva is active */
    if( CJar( 0, GENEVA_COOKIE, &cookie ) != CJar_OK || !cookie )
    {
      alert( ALNOGEN );
      do_quit();
    }
    rsrc_gaddr( 0, TOSICON, &icon );
    rsrc_gaddr( 0, MAIN, &menu );
    rsrc_gaddr( 15, MORE, &more );
    menu[0].ob_state |= X_MAGIC;
  }
  else
  {
    form_alert( 1, "[1][GNVAVIEW.RSC|not found][OK]" );
    do_quit();
  }
  if( argc<2 ) program = "";
  else program = argv[1];
  if( program[0] )	/* 004: find program and set in prog_path if valid */
  {
    strcpy( temp2, program );
    if( temp2[0]==':' || shel_find( temp2 ) ) strcpy( prog_path, temp2 );
  }
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
  lines = set.lines[rez];
  linelen = set.linelen[rez];
  initialize();
  if( lines > max_ht ) lines = set.lines[rez] = max_ht;
  if( !new_scroll( 1, lines, linelen ) ) do_quit();
  check_hex();
  check_feed();
  if( !_app || open_wind() )
  {
    io = Iorec(1);
    strcpy( prog_name, pathend(prog_path) );	/* 004: was (program) */
    strcpy( pathend(prog_path),"*.*" );
new_prog:
    shel_write( SHW_MSGTYPE, 1, 0, 0L, 0L );
    set_acc( "" );
    wind_name(1);
    file = open_file(file);
    for(;;)
    {
      if( fileerr )
      {		/* continue on with file in More mode */
        if( fileerr != 4 ) Bconout(2,7);
        fileerr = 0;
        new_size();
        calc_blank();
        home_dial();
      }
      else if( file>0 )
      {
        if( !filelen )
          if( (filelen = Fread( file, sizeof(filebuf)-1, fileptr=filebuf )) <= 0 )
          {
            Fclose(file);
            file = 0;
            waitkey = 1;
	    in_prog = 0;
            rsrc_gaddr( 15, TERMTXT, &p );
            if( readall ) iconified = 1;
            doecho( "\r\n\033p" );
            doecho( *p );
            doecho( "\033q\r\n" );
            iconified = 0;
            if( readall )
            {
              readall = 0;
              i = (i=line+1)>24 ? i : 24;
              if( i != lines ) new_scroll( 0, i, linelen );
              new_size();
              calc_blank();
              home_dial();
            }
	    wind_name(1);
          }
        if( filelen>0 )
        {
          echo_lines();
          if( !paused && !filelen || fileerr )
          {
            buf[0] = 0x1234;	/* dummy message to cause next read */
            buf[2] = 0;
            appl_write( buf[1] = apid, 16, buf );
          }
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
          case AC_OPEN:
          case NEO_AC_OPEN:
            open_wind();
            break;
          case AC_CLOSE:
            wind = 0;
            iconified = 0;
            em.type |= (MU_KEYBD|MU_BUTTON);
            neo_acc = 0L;
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
          case AP_TERM:
            do_quit();
            break;
          case WM_ONTOP:
            is_top = 1;
            draw_current();
            break;
          case WM_UNTOPPED:
            is_top = 0;
            break;
          case WM_CLOSED:
            quitclose();
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
          case NEO_ACC_INI:                       /* NeoDesk knows we are here */
            if( buf[3]==NEO_ACC_MAGIC )
            {
              retries = 0;
              neo_acc = *(NEO_ACC **)&buf[4];  /* set pointer to Neo's fncns */
              neo_apid = buf[6];               /* NeoDesk's AES ID */
            }
            break;
          case NEO_CLI_RUN:                       /* run a batch file */
            if( buf[3]==NEO_ACC_MAGIC )
              if( !neo_acc ) find_neo(buf);
              else ack();
            break;
          case NEO_ACC_PAS:
            if( buf[3]==NEO_ACC_MAGIC )
              if( !neo_acc ) find_neo(buf);
              else
              {
                program[0] = 0;
                while( ((int cdecl (*)( char **ptr ))(*neo_acc->list_files))( &s ) )
                  if( !program[0] ) strcpy( program, s );
                ack();                                /* you MUST ack NeoDesk!! */
                if( wind>0 || open_wind() ) file = open_file(file);
              }
            break;
          case NEO_ACC_BAD:
            if( buf[3] == NEO_ACC_MAGIC )
            {
              neo_acc=0L;
              retries = 0;
            }
            break;
          case X_MN_SELECTED:
            if( !iconified ) switch( buf[4] )
            {
              case MABOUT:
                do_about();
                break;
              case MFEED:
                set.options.s.feed ^= 1;
                check_feed();
                break;
              case MHEX:
                set.options.s.hex ^= 1;
                check_hex();
                goto reopen;
              case MOPEN:
                strcpy( pathend(prog_path), "*.*" );
                if( select( OPENPATH, prog_path, sizeof(prog_path), prog_name, open_prog ) &&
                    prog_name[0] )
                {
                  program = prog_path;
reopen:           if( in_prog )
                  {
                    Fclose(file);
                    in_prog = readall = 0;
                    file = 0;
                  }
                  paused = 0;
                  unselect(0);
                  cls();
                  waitkey = 0;
                  file = open_file(file);
                }
                break;
              case MQUIT:
                quitclose();
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
              case MCLIP:
                save_txt( sel_row==-99, 1 );
                break;
              case MPRINT:
                save_txt( sel_row==-99, 2 );
                break;
/*              case MMISC:
                miscopts();
                break; */
              case MHELP:
                do_help( "Geneva View" );
                break;
            }
            menu_tnormal( menu, buf[3], 1 );
            break;
        }
      }
      i = em.mouse_k&3;
      if( em.event & MU_KEYBD )
        if( em.mouse_k&4 ) switch( em.key&0xFF00 )
        {
          case 0x5000:
            scroll( i ? WA_DNPAGE : WA_DNLINE );
            break;
          case 0x4800:
            scroll( i ? WA_UPPAGE : WA_UPLINE );
            break;
          case 0x7300:
            scroll( i ? WA_LFPAGE : WA_LFLINE );
            break;
          case 0x7400:
            scroll( i ? WA_RTPAGE : WA_RTLINE );
            break;
          case 0x7700:
            curs_bottom(0);
            break;
          default:
            if( in_prog || waitkey ) page_key( em.mouse_k, em.key );
        }
        else if( in_prog || waitkey ) page_key( em.mouse_k, em.key );
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
