#include <ctype.h>
#include "new_aes.h"
#include "xwind.h"              /* Geneva extensions */
#include "tos.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "gnva_mac.h"            /* resource file constants */
#include "common.h"
#include "multevnt.h"

unsigned char kt_control[128] = {
  0x00, 0x1B, 0x11, 0x00, 0x13, 0x14, 0x15, 0x1E, 0x17, 0x18, 0x19, 0x10, 0x1F,
  0x1D, 0x08, 0x09, 0x11, 0x17, 0x05, 0x12, 0x14, 0x19, 0x15, 0x09, 0x0F, 0x10,
  0x1B, 0x1D, 0x0A, 0x00, 0x01, 0x13, 0x04, 0x06, 0x07, 0x08, 0x0A, 0x0B, 0x0C,
  0x1B, 0x07, 0x00, 0x00, 0x1C, 0x1A, 0x18, 0x03, 0x16, 0x02, 0x0E, 0x0D, 0x0C,
  0x0E, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00,
  0x0B, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x09, 0x0F, 0x0A, 0x17,
  0x18, 0x19, 0x14, 0x15, 0x1E, 0x11, 0x11, 0x13, 0x10, 0x0E, 0x0A, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };


/********************* Dialog manager routines *********************/

/* calculate a window's border based on the size of an object tree */
void calc_bord( int type, int xtype, OBJECT *tree, GRECT *g )
{
  x_wind_calc( WC_BORDER, type, xtype, tree[0].ob_x, tree[0].ob_y,
      tree[0].ob_width, tree[0].ob_height, &g->g_x, &g->g_y,
      &g->g_w, &g->g_h );
}

/* open a modeless dialog */
void start_form( int fnum, int tnum, int type, int xtype )
{
  FORM *f = &form[fnum];
  int dum, hand;
  GRECT out;

  if( f->handle>0 )                     /* window is already open */
  {
    if( f->exit==msg_exit )
    {
      msg_init( f->tree );
      wind_set( f->handle, X_WF_DIALOG, f->tree );
    }
    wind_set( f->handle, WF_TOP );
  }
  else
  {
    if( !f->tree )                      /* dialog not used before */
    {
      rsrc_gaddr( 0, tnum, &f->tree );
      f->tree[1].ob_flags |= HIDETREE;  /* hide the "title" */
      if( f->wind.g_y>0 )
      {
        if( f->wind.g_y < max.g_y ) f->wind.g_y = max.g_y;
        else if( f->wind.g_y > (dum=max.g_y+max.g_h-1) )
             f->wind.g_y = dum;
        if( f->wind.g_x > (dum=max.g_x+max.g_w-25) ) f->wind.g_x = dum;
        x_wind_calc( WC_WORK, type, xtype, f->wind.g_x, f->wind.g_y,
            100, 100, &f->tree[0].ob_x, &f->tree[0].ob_y, &dum, &dum );
      }
      calc_bord( type, xtype, f->tree, &out ); /* fit a window around it */
      if( f->wind.g_y<=0 )
      {
        /* center the window on the screen */
        out.g_x = (max.g_w-out.g_w)/2 + max.g_x;
        out.g_y = (max.g_h-out.g_h)/2 + max.g_y;
        if( out.g_y < max.g_y ) out.g_y = max.g_y;
        /* and reposition the dialog at this location */
        x_wind_calc( WC_WORK, type, xtype, out.g_x, out.g_y, out.g_w,
            out.g_h, &f->tree[0].ob_x, &f->tree[0].ob_y, &dum, &dum );
      }
    }
    else calc_bord( type, xtype, f->tree, &out );
    if( (hand=x_wind_create( type, xtype, out.g_x, out.g_y, out.g_w,
        out.g_h )) > 0 )
    {
      f->handle = hand;
      if( (*f->init)( f->tree ) )       /* initialize the dialog */
      {
        /* tell Geneva it's a dialog in a window */
        wind_set( hand, X_WF_DIALOG, f->tree );
        /* set the name according to the text in the hidden title object */
        wind_set( hand, WF_NAME, f->tree[1].ob_spec.free_string );
        f->wind = out;
        wind_open( hand, out.g_x, out.g_y, out.g_w, out.g_h );
      }
      else close_wind( &f->handle );
    }
    else alert( NOWIND );
  }
}

/* process input from the user to a modeless dialog */
void use_form( int hand, int num )
{
  int i, j, but;
  FORM *f;

  if( !hand ) return;
  for( f=&form[0]; f->init; f++ )
    if( f->handle == hand )             /* found the right window */
    {
      but = num&0x7FFF;                 /* treat double-clicks as singles */
      if( f->tree[but].ob_flags & TOUCHEXIT )
        if( f->touch ) (*f->touch)( f->tree, but );     /* handle the click */
      if( f->tree[but].ob_flags&EXIT )
      {
        /* reset the object */
        f->tree[but].ob_state &= ~SELECTED;
        /* process the event and close the window if necessary */
        if( *(char *)&f->tree[but].ob_type==X_HELP )
        {
          do_help(f->tree[1].ob_spec.free_string);
          x_wdial_draw( f->handle, but, 8 );
        }
        else if( f->exit && (*f->exit)( f->tree, num ) ) close_wind(&f->handle);
        else x_wdial_draw( f->handle, but, 8 );         /* just draw button */
      }
      return;
    }
}

void key_str( OBJECT *o, int num, KEYCODE *key, int ascii )
{
  int i;
  unsigned char c, c2;
  char *out;
  static unsigned char
      keycode[] = { 0x1C, 1, 0xf, 0xe, 0x53, 0x52, 0x62, 0x61, 0x47, 0x48, 0x50,
          0x4d, 0x4b, 0x72, 0x60, 0x39 },
      keynam[][6] = { "Ret", "Esc", "Tab", "Bksp", "Del", "Ins", "Help", "Undo",
          "Home", "", "", "", "", "Enter", "ISO", "Space" }, fmt[]="F%d",
          kpfmt[]="kp %c";

  out = !num ? (char *)o : o[num].ob_spec.free_string;
  if( key->shift&4 ) *out++ = '^';
  if( key->shift&3 ) *out++ = '\1';
  if( key->shift&8 ) *out++ = '\7';
  if( (c = key->scan) != 0 )
  {
    *out = 0;
    if( c==0x74 ) c=0x4d;                       /* ^right -> right */
    else if( c==0x73 ) c=0x4b;                  /* ^left  -> left */
    else if( c==0x77 ) c=0x47;                  /* ^home  -> home */
    else if( c>=0x78 && c<=0x83 ) c -= 0x76;    /* ^F1-10 -> F1-10 */
    for( i=0; i<sizeof(keycode); i++ )
      if( keycode[i] == c ) strcpy( out, keynam[i] );
    if( c >= 0x3b && c <= 0x44 ) x_sprintf( out, fmt, c-0x3a ); /* F1-10 */
    else if( c >= 0x54 && c <= 0x5d )
        x_sprintf( out, fmt, c-0x53 );                    /* shift F1-10 */
    if( !*out )
    {
      c2 = toupper( *(Keytbl( (void *)-1L, (void *)-1L, (void *)-1L )->unshift + c) );
      if( c >= 0x63 && c <= 0x72 || c == 0x4a || c == 0x4e )
          x_sprintf( out, kpfmt, c2 );                    /* keypad key */
      else
      {
        *out++ = c2;                                      /* unshifted char */
        *out = '\0';
      }
    }
    else out += strlen(out);
    if( ascii && key->ascii )
    {
      *out++ = ' ';
      *out++ = key->ascii;
      *out = 0;
    }
  }
  else if( (c = key->ascii) != 0 )   /* ASCII value is used instead of scan code */
    if( c == ' ' ) strcpy( out, "Space" );
    else
    {
      *out++ = toupper(c);
      *out = 0;
    }
  else strcpy( out, "???" );
}

#pragma warn -par
int a_init( OBJECT *o )
{
  return 1;
}
void a_touch( OBJECT *o, int num )
{
  close_wind(&form[2].handle);
}
int a_exit( OBJECT *o, int num )
{
  return 1;
}

int ms_init( OBJECT *o )
{
  char **t, temp[20];

  memset( mac_buf, -1, (long)macsize*sizeof(MACBUF) );
  rsrc_gaddr( 15, MSGSTR, &t );
  key_str( (OBJECT *)temp, 0, &start_end, 0 );
  x_sprintf( o[MSGSEG].ob_spec.free_string, *t, temp );
  mac_count = 0;
  mac_end = -1;
  ms_update();
  return 1;
}

void ms_touch( OBJECT *o, int num )
{
  close_wind(&form[1].handle);
}
#pragma warn +par

int hide_if(OBJECT *tree, int i, int truth )
{
  unsigned int *f = &tree[i].ob_flags;

  if( truth ) *f &= ~HIDETREE;
  else *f |= HIDETREE;
}

void dflt_if( OBJECT *tree, int i, int truth )
{
  unsigned int *f = &tree[i].ob_flags;

  if( !truth ) *f &= ~DEFAULT;
  else *f |= DEFAULT;
}

void ms_update(void)
{
  hide_if( form[1].tree, MSGCOUNT, mac_end>=0 );
  hide_if( form[1].tree, MSGCOUNT-1, mac_end>=0 );
  x_sprintf( form[1].tree[MSGCOUNT].ob_spec.tedinfo->te_ptext, "%d",
      mac_count );
}

void update_msg(void)
{
  int c;

  if( mac_end<0 ) return;
  c = get_mac_end( &mac_end );
  if( c != mac_count )
  {
    mac_count = c;
    ms_update();
    x_wdial_draw( form[1].handle, MSGCOUNT-1, 0 );
    x_wdial_draw( form[1].handle, MSGCOUNT, 0 );
  }
}

OBJECT ascii_tbl[] = {
{ -1, 1,  6, G_BOX,    NONE, NORMAL, 0xFF1170L, 0, 0, 51, 5 },
{ 2, -1, -1, G_STRING, TOUCHEXIT, NORMAL, (long)"\x01\x02\x03\x04\x05\x06\x07\x08\
\x09\x0A\x0B\x0C\x0D\x0E\x0F\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\
\x1C\x1D\x1E\x1F\x20\x21\x22\x23\x24\x25\x26\x27\x28\x29\x2A\x2B\x2C\x2D\x2E\
\x2F\x30\x31\x32\x33", 0, 0, 51, 1 },
{ 3, -1, -1, G_STRING, TOUCHEXIT, NORMAL, (long)"456789:;<=>?@ABCDEFGHIJKLMNOPQRS\
TUVWXYZ[\\]^_\`abcdef", 0, 1, 51, 1 },
{ 4, -1, -1, G_STRING, TOUCHEXIT, NORMAL, (long)"ghijklmnopqrstuvwxyz{|}~�������\
�������������������", 0, 2, 51, 1 },
{ 5, -1, -1, G_STRING, TOUCHEXIT, NORMAL, (long)"��������������������������������\
�������������������", 0, 3, 51, 1 },
{ 6, -1, -1, G_STRING, TOUCHEXIT, NORMAL, (long)"��������������������������������\
�������������������", 0, 4, 51, 1 },
{ 0, -1, -1, G_IBOX, HIDETREE, NORMAL, 0L, 0, 0, 1, 1 } };

int set_asc_cur( int x, int y )
{
  int ret, w;

  y--;
  ret = (x-ascii_tbl[0].ob_x)/(w=ascii_tbl[0].ob_width/51);
  ascii_tbl[6].ob_x = ret*w;
  ascii_tbl[6].ob_y = y*ascii_tbl[1].ob_height;
  ascii_tbl[6].ob_flags &= ~HIDETREE;
  return ret + y*51 + 1;
}

void fixy( int x, int *i )
{
  *i = ((*i&0xFF)*x) + (*i>>8);
}

void obfix( int x, int y, OBJECT *tree )
{
  int *i = &tree->ob_x;

  fixy( x, i++ );
  fixy( y, i++ );
  fixy( x, i++ );
  fixy( y, i );
  if( x==6 && (char)tree->ob_type==G_STRING )
      tree->ob_state |= (X_MAGIC|X_SMALLTEXT);
}

int do_ascii(void)
{
  int kr, x, y, b, k, i;
  GRECT g;

  if( ascii_tbl[0].ob_height==5 )
  {
    if( max.g_w > char_w*51 )
    {
      x = char_w;
      y = char_h;
    }
    else x=y=6;
    for( i=0; i<sizeof(ascii_tbl)/sizeof(OBJECT); i++ )
      obfix( x, y, &ascii_tbl[i] );
  }
  ascii_tbl[0].ob_x = (max.g_w-ascii_tbl[0].ob_width)>>1;
  ascii_tbl[0].ob_y = max.g_y+2;
  g = *(GRECT *)&ascii_tbl[0].ob_x;
  g.g_x--;
  g.g_y--;
  g.g_w += 2;
  g.g_h += 2;
  if( !x_graf_blit( &g, 0L ) ) return 0;
  objc_draw( ascii_tbl, 0, 8, 0, 0, 0, 0 );
  but_up();
  for(;;)
  {
    evnt_button( 1, 1, 1, &x, &y, &b, &k );
    if( (kr=objc_find( ascii_tbl, 0, 8, x, y )) > 0 )
    {
      kr = set_asc_cur( x, kr );
      ascii_tbl[6].ob_state |= SELECTED;
      objc_draw( ascii_tbl, 6, 0, 0, 0, 0, 0 );
      but_up();
      ascii_tbl[6].ob_state &= ~SELECTED;
      ascii_tbl[6].ob_flags |= HIDETREE;
    }
    else kr = -1;
    x_graf_blit( 0L, &g );
    return kr;
  }
}

int find_kt( unsigned char *k, unsigned char c )
{
  unsigned char *p;

  if( (p = memchr( k, c, 128 )) == 0L ) return -1;
  return p-k;
}

int keypress( int *key )
{
  KEYTAB *kt;
  int i;

  kt = Keytbl( (void *)-1L, (void *)-1L, (void *)-1L );
  if( (i = find_kt( kt->unshift, *key )) >= 0 )
  {
    *(char *)key = i;
    return 0;
  }
  else if( (i = find_kt( kt->shift, *key )) >= 0 )
  {
    *(char *)key = i;
    return 1;
  }
  else if( (i = find_kt( kt_control, *key )) >= 0 )
  {
    *(char *)key = i;
    return 4;
  }
  return 0;
}

void read_key( OBJECT *o, int num, KEYCODE *k, int hand, char *title, int ascii )
{
  int dum;
  char **t;
  EMULTI emulti;

  rsrc_gaddr( 15, GETKEY, &t );
  wind_set( hand, WF_NAME, *t );
  wind_update( BEG_UPDATE );
  in_read = *k;
  emulti.type = MU_KEYBD|MU_BUTTON;
  emulti.clicks = ascii?0x101:1;
  emulti.mask = ascii?3:2;
  emulti.state = ascii?0:2;
  multi_evnt( &emulti, &dum );
  in_read.scan = -1;
  if( emulti.event&MU_BUTTON )
    if( emulti.mouse_b&2 ) emulti.key = 0;		/* no emulti.key */
    else
    {
      emulti.key = do_ascii();
      emulti.mouse_k = keypress( &emulti.key );
    }
  wind_update( END_UPDATE );
  wind_set( hand, WF_NAME, title );
  if( emulti.key != -1 )
  {
    if( emulti.mouse_k&3 ) emulti.mouse_k |= 3;               /* one shift key->both shift keys */
    k->shift = emulti.mouse_k&0xf;
    k->scan = *(char *)&emulti.key;
    k->ascii = emulti.key;
    key_str( o, num, k, ascii );
  }
  set_if( o, num, 0 );
  x_wdial_draw( hand, num, 0 );
}

void short_path( char *p, char *s, int len, int maxlen )
{
  char *ptr;
  int max, i, l, plus=3;

    max = strlen(p);
    if( len >= maxlen ) len = maxlen-1;
    if( max<len )
    {
      strncpy( s, p, max );
      s[max] = 0;
    }
    else
    {
      ptr = strchr( p+plus, '\\' ) + 1;
      strncpy( s, p, ptr-p );
      strcpy( s+(ptr-p), "..." );
      len -= (l = strlen(s));
      while( ptr && (i=p+max-ptr) > len )
        ptr = strchr( ptr+1, '\\' );
      if( ptr )
      {
        strncpy( s+l, ptr, i );
        s[l+i] = 0;
      }
    }
}

void gma_onoff( OBJECT *o, int state )
{
  o[MOGMAT].ob_state = (o[MOGMAT].ob_state&~SELECTED)|state;
  o[MOGMA].ob_state = (o[MOGMA].ob_state&~SELECTED)|state;
  x_wdial_draw( form[0].handle, MOGMAT, 0 );
  x_wdial_draw( form[0].handle, MOGMA, 0 );
}

char gma_tpath[120];
KEYCODE gma_tkey;

int mo_exit( OBJECT *o, int num )
{
  int n;

  switch(num)
  {
    case MOGMAT:
    case MOGMA:
      gma_onoff( o, SELECTED );
      if( fselect( gma_path, "*.GMA", MACDIR ) )
      {
        *spathend(gma_path) = 0;
        short_path( gma_path, o[MOGMA].ob_spec.tedinfo->te_ptext,
            o[MOGMA].ob_width/6, 40 );
        x_wdial_draw( form[0].handle, MOGMA, 0 );
      }
      gma_onoff( o, 0 );
      return 0;
    case MOOK:
      gma_auto = o[MOAUTO].ob_state&SELECTED;
      if( !rec_mac )
      {
        x_sscanf( o[MOSIZE].ob_spec.tedinfo->te_ptext, "%d", &n );
        if( !xrealloc( (void **)&mac_buf, (long)n*sizeof(MACBUF) ) ) macsize=n;
      }
      return 1;
    default:
      memcpy( &start_end, &gma_tkey, sizeof(KEYCODE) );
      strcpy( gma_path, gma_tpath );
      return 1;
  }
}

int mo_init( OBJECT *o )
{
  memcpy( &gma_tkey, &start_end, sizeof(KEYCODE) );
  strcpy( gma_tpath, gma_path );
  x_sprintf( o[MOSIZE].ob_spec.tedinfo->te_ptext, "%d", macsize );
  key_str( o, MOKEY, &start_end, 0 );
  set_if( o, MOAUTO, gma_auto );
  short_path( gma_path, o[MOGMA].ob_spec.tedinfo->te_ptext,
      o[MOGMA].ob_width/6, 44 );
  return 1;
}

void mo_touch( OBJECT *o, int num )
{
  if( num==MOKEY ) read_key( o, MOKEY, &start_end, form[0].handle,
      o[1].ob_spec.free_string, 0 );
}

int at_init( OBJECT *o )
{
  static char once;

  if( !once )
  {
    once = 1;
    strcpy( o[ALLEDIT].ob_spec.tedinfo->te_ptext, "0" );
  }
  return 1;
}

int at_exit( OBJECT *o, int num )
{
  if( num==ALLOK && tim_edesc ) all_timers( tim_edesc,
      atol(o[ALLEDIT].ob_spec.tedinfo->te_ptext) );
  tim_edesc = 0L;
  return 1;
}

char save_date[8][21], save_time[8][21];

int d_init( OBJECT *o )
{
  int i;
  
  for( i=0; i<8; i++ )
  {
    strcpy( save_date[i], date_fmt[i] );
    set_if( o, DFDD0+i, i==d_date );
    strcpy( save_time[i], time_fmt[i] );
    set_if( o, DFDT0+i, i==d_time );
  }
  for( i=0; i<3; i++ )
    set_if( o, DFBELL+i, i==d_sound );
  return 1;
}

void d_touch( OBJECT *o, int num )
{
  if( num==DFDTEST )
  {
    updt_datepop();
    do_popup( o, DFDATE0, date_pop, -1 );
  }
  else
  {
    updt_timepop();
    do_popup( o, DFTIME0, time_pop, -1 );
  }
}

int d_exit( OBJECT *o, int num )
{
  int i;
  
  for( i=0; i<8; i++ )
    if( num==DFOK )
    {
      if( o[DFDD0+i].ob_state&SELECTED ) d_date = i;
      if( o[DFDT0+i].ob_state&SELECTED ) d_time = i;
    }
    else
    {
      strcpy( date_fmt[i], save_date[i] );
      strcpy( time_fmt[i], save_time[i] );
    }
  for( i=0; i<3; i++ )
    if( o[DFBELL+i].ob_state&SELECTED ) d_sound = i;
  updt_datepop();
  updt_timepop();
  return 1;
}

int msg_init( OBJECT *o )
{
  int i;

  hide_if( o, NMOK, i=msg_edit!=0 );
  hide_if( o, NMCANC, i );
  hide_if( o, NMCLOSE, !i );
  dflt_if( o, NMCLOSE, !i );
  dflt_if( o, NMOK, i );
  for( o+=(i=NM1); i<NM1+5; i++, o++ )
  {
    strcpy( o->ob_spec.tedinfo->te_ptext, (*msg_mac)[i-NM1] );
    if( msg_edit )
    {
      (char)o->ob_type = G_FTEXT;
      o->ob_flags |= EDITABLE;
      if( i<NM1+4 ) o->ob_spec.tedinfo->te_tmplen = X_LONGEDIT;
    }
    else
    {
      (char)o->ob_type = G_TEXT;
      o->ob_flags &= ~EDITABLE;
    }
  }
  return 1;
}

int msg_exit( OBJECT *o, int num )
{
  int i;
  
  if( num==NMOK && msg_edit )
  {
    for( o+=(i=NM1); i<NM1+5; i++, o++ )
      strcpy( (*msg_mac)[i-NM1], o->ob_spec.tedinfo->te_ptext );
    msg_str( msg_edit, msg_ind, 1 );
  }
  return 1;
}
