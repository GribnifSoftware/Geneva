#include "vdi.h"
#include "new_aes.h"
#include "xwind.h"
#include "tos.h"
typedef struct { int x, y, w, h; } Rect;
#define _GRAPHICS
#include "graphics.h"
#include "string.h"

extern Rect clip_rect;
extern int vdi_hand;
int wr_mode, txt_point;	/* 004: was static */
char my_mouse, true_type;

static int txt_color, halign, valign,
    ln_pat, ln_type=-1, ln_color, f_style, f_color, f_inter, effects,
    l_start, l_end, first_char, lwidth, ch_mode;

int text_arr[sizeof(_VDIParBlk.intin)/2];
char text_off[sizeof(_VDIParBlk.intin)/2], char_off[256];

#define CJar_xbios      0x434A          /* "CJ" */
#define CJar_OK         0x6172          /* "ar" */
#define CJar(mode,cookie,value)         (int)xbios(CJar_xbios,mode,cookie,value)
#define FSMC		0x46534D43L
#define NVDI		0x4E564449L
#define _SPD		0x5F535044L

void v_ftext16_mono( int handle, int x, int y, int *wstr, int strlen, int offset );
void v_ftext16( int handle, int x, int y, int *wstr, int strlen );

void vdi_reset(void)
{
  long *cookie;

  wr_mode = halign = valign = ln_type = ln_color = f_style = f_color =
      f_inter = gr_font = clip_on = l_start = l_end = txt_color = -1;
  effects = 0;
  if( (scalable = CJar(0,FSMC,&cookie) == CJar_OK) != 0 )
  {
    speedo = *cookie == _SPD;
    NVDI3 = CJar(0,NVDI,&cookie) == CJar_OK && cookie && *(int *)cookie>=0x300;
  }
  else speedo = 0;
  ch_mode = 1;		/* 004 */
  pfont_id = -99;	/* 007 */
  pfont_mode = IBM;	/* 007 */
}

void _v_mouse( int flag )
{
  my_mouse+=2;
  switch( flag )
  {
    case 0:
      /*if( !mouse_hide++ )*/ v_hide_c( vdi_hand );
      mouse_hide++;
      break;
    case 1:
      /*if( !--mouse_hide )*/ v_show_c( vdi_hand, 1 );
      mouse_hide--;
      break;
    case 2:
      v_show_c( vdi_hand, 0 );
      mouse_hide = 0;	/* 004 */
      break;
  }
  if( (my_mouse-=2) < 0 ) my_mouse = 0;
}

void _vs_clip( int flag, Rect *r )
{
  int p[4];
  static Rect off = { 0, 0, 32767/2, 32767/2 };

  if( !flag ) r = &off;
  p[2] = (p[0] = r->x) + r->w - 1;
  p[3] = (p[1] = r->y) + r->h - 1;
  if( clip_on != flag || *(long *)&clip_arr[0] != *(long *)&p[0] ||
      *(long *)&clip_arr[2] != *(long *)&p[2] )
  {
    clip_rect = *r;
    *(long *)&clip_arr[0] = *(long *)&p[0];
    *(long *)&clip_arr[2] = *(long *)&p[2];
    vs_clip( vdi_hand, clip_on=flag, p );
  }
}

void _vswr_mode( int mode )
{
  if( mode != wr_mode ) vswr_mode( vdi_hand, wr_mode=mode );
}

int txt_adv( int ch )
{
  int advx, advy, remx, remy, w;

  vqt_advance( vdi_hand, ch, &advx, &advy, &remx, &remy );
  w = advx; /* + ((unsigned)remx>=16384/2); */
  remx >>= 12;
  if( remx==1 ) w++;
  else if( remx==2 ) w--;
  return w;
}

int _vst_point( int point, int *char_width,
                int *char_height, int *cell_width, int *cell_height )
{
  if( point != txt_point )
    if( !scalable || !is_scalable ) txt_point =
        vst_point( vdi_hand, point, &txt_chw, &txt_chh, &txt_clw, &txt_clh );
    else
    {
      if( NVDI3 ) memset( char_off, 0x7f, sizeof(char_off) );
      txt_point = vst_arbpt( vdi_hand, point, &txt_chw, &txt_chh, &txt_clw, &txt_clh );
      txt_clw = txt_adv('M');
    }
  *char_width = txt_chw;
  *char_height = txt_chh;
  *cell_width = txt_clw;
  *cell_height = txt_clh;
  return txt_point;
}

void _vst_color( int color )
{
  if( color != txt_color ) vst_color( vdi_hand, txt_color=color );
}

void _vst_alignment( int hor, int vert )
{
  int dum;

  if( hor!=halign || vert!=valign ) vst_alignment( vdi_hand,
      halign=hor, valign=vert, &dum, &dum );
}

void _vst_effects( int eff )
{
  if( eff!=effects ) vst_effects( vdi_hand, effects=eff );
}

void _vsl_udsty( int pat )
{
  if( ln_pat != pat ) vsl_udsty( vdi_hand, ln_pat=pat );
}

void _vsl_type( int type )
{
  if( ln_type != type ) vsl_type( vdi_hand, ln_type=type );
}

void _vsl_ends( int start, int end )	/* 004 */
{
  if( l_start != start || l_end != end )
      vsl_ends( vdi_hand, l_start=start, l_end=end );
}

void _vsl_color( int color )
{
  if( color != ln_color ) vsl_color( vdi_hand, ln_color=color );
}

void _vsf_style( int style )
{
  if( style != f_style ) vsf_style( vdi_hand, f_style=style );
}

void _vsf_color( int color )
{
  if( color != f_color ) vsf_color( vdi_hand, f_color=color );
}

void _vsl_width( int width )	/* 004 */
{
  if( width != lwidth ) vsl_width( vdi_hand, lwidth=width );
}

void _vsf_interior( int style )
{
  if( style != f_inter ) vsf_interior( vdi_hand, f_inter=style );
}

int _vst_font( int id, int scale )
{
  if( id==X_SYSFONT ) id = pfont_id;		/* 007 */
  if( gr_font != id )
  {
    if( !have_fonts && id!=1 && vq_gdos() )
    {
      vst_load_fonts( vdi_hand, 0 );
      have_fonts = 1;
    }
    if( id==-99 ) id = X_SYSFONT;	/* 007 */
    if( (gr_font = vst_font( vdi_hand, id )) != id ) {
      is_scalable = 0;
      if( id==X_SYSFONT ) {		/* 007 */
        pfont_id = 1;
        pfont_mode = IBM;
      }
    }
    else {				/* 007 */
      if( (is_scalable = scale) != 0 ) first_char = id<0 ? 32 : 0;
      if( id==X_SYSFONT ) {		/* 007 */
        pfont_id = X_SYSFONT;
        pfont_mode = scale ? GDOS_PROP : GDOS_BITM;
      }
    }
    txt_point = -1;
    true_type = NVDI3 && id&0x4000;	/* 004 */
  }
  return gr_font;
}

int char_xlate[256] = { -1,
    0x0127, 0x0124, 0x0126, 0x0125, 0x0150, 0x022F, 0x0161, 0x012E,
    0x01F9, 0x001F, 0x014A, 0x001F, 0x001F, 0x020A, 0x020B, 0x0010,
    0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018,
    0x0019, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x0000,
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
    19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34,
    35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
    51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66,
    67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82,
    83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94,
    0x013A, 0x0094, 0x00D7, 0x00FC, 0x0102, 0x0100, 0x0104, 0x0075,
    0x0095, 0x00F8, 0x00F6, 0x00FA, 0x00EC, 0x00EE, 0x00F0, 0x00FF,
    0x0071, 0x00FB, 0x0076, 0x0072, 0x00CB, 0x00CD, 0x00C9, 0x00D5,
    0x00D3, 0x00DD, 0x00CE, 0x00D8, 0x0062, 0x0061, 0x0112, 0x0079,
    0x0063, 0x0106, 0x00F2, 0x00C7, 0x00D1, 0x00C3, 0x00C4, 0x001F,
    0x001F, 0x007F, 0x0136, 0x0135, 0x0099, 0x0097, 0x0080, 0x007D,
    0x007E, 0x00FE, 0x00CF, 0x0073, 0x0077, 0x0078, 0x0074, 0x0103,
    0x00FD, 0x00D0, 0x0088, 0x0082, 0x006C, 0x0117, 0x01F7, 0x01F8,
    0x014E, 0x0113, 0x0114, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F,
    0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F,
    0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F,
    0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x006E, 0x001F,
    0x012F, 0x013F, 0x0140, 0x0139, 0x0146, 0x013C, 0x0147, 0x0145,
    0x0148, 0x013D, 0x0144, 0x013E, 0x0141, 0x01AC, 0x0149, 0x012A,
    0x012B, 0x011F, 0x011E, 0x0122, 0x0123, 0x012C, 0x012D, 0x011D,
    0x0121, 0x01F3, 0x01F4, 0x0155, 0x012E, 0x021F, 0x00A0, 0x00A1,
    0x00E6 };

void text_2_arr( unsigned char *str, int *num )
{
  int *arr = text_arr, n;
  char *off = text_off, o;

  if( *num > sizeof(_VDIParBlk.intin)/2 ) *num = sizeof(_VDIParBlk.intin)/2;
  n = *num;
  if( !NVDI3 ) while( n-- && (*arr++ = char_xlate[*str++]+first_char) >= 0 );
  else
    for( ; n--; str++ )
    {
      if( !true_type )
      {
        if( (*arr++ = char_xlate[*str]+first_char) < 0 ) break;
      }
      else if( (*arr++ = *str) == 0 ) break;
      if( (o = char_off[*str]) == (char)0x7f ) o = char_off[*str] =
          (txt_clw - txt_adv(*(arr-1)))>>1;
      *off++ = o;
    }
}

void text_box( int x, int y, int len, int *arr )
{
  arr[2] = (arr[0] = x) + txt_clw*len-1;
  arr[3] = (arr[1] = y) + txt_clh-1;
}

int txt_baseline(void)
{
  int dum, dist[5], eff[3];
  
  vqt_fontinfo( vdi_hand, &dum, &dum, dist, &dum, eff );
  return dist[4];
}

void ftext16_mono( int x, int y, int len )
{
  int arr[4], und, *txt;
  char *t;

  if( NVDI3 )
  {
    txt = text_arr;
    text_box( x, y, len, arr );
    if( wr_mode==MD_REPLACE )
    {
      _vsf_color( WHITE );
      vr_recfl( vdi_hand, arr );
    }
    if( effects&8 )	/* underline */
    {
      vst_effects( vdi_hand, effects&~8 );
      und = 1;
    }
    else und = 0;
    t = text_off;
    while( --len>=0 )
    {
      v_ftext16( vdi_hand, x + *t++, y, txt++, 1 );
      x += txt_clw;
    }
    if( und )
    {
      _vsl_type(SOLID);
      _vsl_color(BLACK);
      arr[3] = arr[1] += txt_baseline()+1;
      v_pline( vdi_hand, 2, arr );
    }
  }
  else v_ftext16_mono( vdi_hand, x, y, text_arr, len, txt_clw );
}

void _vst_charmap( int mode )		/* 004 */
{
  if( true_type ) mode = 1;
  if( mode != ch_mode ) vst_charmap( vdi_hand, ch_mode=mode );
}
