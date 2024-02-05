#include "new_aes.h"
#include "vdi.h"
#include "xwind.h"
#include "tos.h"
#include "test.h"
#include "string.h"
#include "stdio.h"
#include <multevnt.h>

void v_ftext16( int handle, int x, int y, int *wstr, int strlen );
void v_ftext16_mono( int handle, int x, int y, int *wstr, int strlen, int offset );
void v_ftext_offset16( int handle, int x, int y, int *wstr, int strlen, int *offset );

char *charset[5] = { "\x01\x02\x03\x04\x05\x06\x07\x08\
\x09\x0A\x0B\x0C\x0D\x0E\x0F\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\
\x1C\x1D\x1E\x1F\x20\x21\x22\x23\x24\x25\x26\x27\x28\x29\x2A\x2B\x2C\x2D\x2E\
\x2F\x30\x31\x32\x33",
"456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_\`abcdef",
"ghijklmnopqrstuvwxyz{|}~€‚ƒ„…†‡ˆ‰Š‹ŒŽ‘’“”•–—˜™",
"š›œžŸ ¡¢£¤¥¦§¨©ª«¬­®¯°±²³´µ¶·¸¹º»¼½¾¿ÀÁÂÃÄÅÆÇÈÉÊËÌ",
"ÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþÿ" };

#define WTYPE MOVER|SIZER|UPARROW|DNARROW|VSLIDE|LFARROW|RTARROW|HSLIDE|FULLER

#define CJar_cookie	0x434A6172L	/* "CJar" */
#define CJar_xbios	0x434A		/* "CJ" */
#define	CJar_OK		0x6172		/* "ar" */
#define CJar(mode,cookie,value)		(int)xbios(CJar_xbios,mode,cookie,value)

/*OBJECT blank[] = { -1, -1, -1, G_BOX, 32, 0, 0xFF1070L };*/

int work_in[] = { 1, 7, 1, 1, 1, 1, 1, 1, 1, 1, 2 },
    work_out[57];       /* returns from v_opnvwk() */

struct
{
  long code;
  long event;
} record[2000];
int rnum;

void state( char *s )
{
  Cconws( "\033H\r\n\n" );
  Cconws(s);
  Cconws( "\033K" );
  Bconin(2);
}

void dump( void *s, int len )
{
  char buf[30];

  sprintf( buf, "\033H\r\n\n\n%08lX:\r\n", s );
  Cconws(buf);
  while( len-- )
  {
    sprintf( buf, "%02X ", *((unsigned char *)s)++ );
    Cconws(buf);
  }
}

/*
long time[10];
int tnum;

void get_timer(void)
{
  time[tnum++] = *(long *)0x4ba;
}
*/
/*
void load_mouse( char *name, int type, int index )
{
  int hand;
  ANI_MOUSE mouse;

  if( (hand = Fopen( name, 0 )) > 0 )
  {
    if( type )
    {
      Fread( hand, 32*sizeof(MFORM), &mouse.form );
      Fread( hand, 2*sizeof(int), &mouse.frames );
    }
    else
    {
      Fread( hand, sizeof(MFORM), &mouse.form[0] );
      mouse.frames = 1;
      mouse.delay = 0;
    }
    Fclose(hand);
    graf_mouse( X_SET_SHAPE+index, (MFORM *)&mouse );
  }
}
*/

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

void ftext16( int handle, int x, int y, unsigned char *str )
{
  int arr[200], off[200][2], len=-1;

  vst_charmap( handle, 0 );
  while( (arr[++len] = char_xlate[*str++]) >= 0 )
  {
    off[len][0] = 8;
    off[len][1] = 0;
  }
  off[len+1][0] = 8;
  off[0][0] = off[len+1][1] = 0;
  if( len ) v_ftext_offset16( handle, x, y, arr, len, (int *)off );
  vst_charmap( handle, 1 );
}

/*
void ascii(int handle)
{
  int i, j, y, arr[10];
  char temp[4];

  Cconws( "\033E" );
  vst_charmap( handle, 0 );
  for( i=0, y=13; i<600; i++ )
  {
    sprintf( temp, "%03d", i );
    for( j=0; j<3; j++ )
      arr[j] = temp[j]-' ';
    arr[3] = 0;
    arr[4] = i;
    v_ftext16_mono( handle, (i%8)*72, y, arr, 5, 12 );
    if( (i%8)==7 )
      if( (y += 18) > 400-9 )
      {
        Bconin(2);
        Cconws( "\033E" );
        y = 13;
      }
  }
  Bconin(2);
}
*/

void Lineaa(void) 0xA00A;
void trap2(int d0) 0x4e42;

int main(void)
{
  OBJECT *menu, *ob, *sub, *winmenu;
  int buf[8], i, hand, maxw, dum, wx, wy, ww, wh, vdi, apid;
  long cook, gl1, gl2;
  char temp[80], new_aes, has_gnva=0, has_gdos;
  GRECT o, r;
  SETTINGS set;
  OBJECT *wintree, *demo=0L;
  MENU pop, pop2, subm;
  static EMULTI e = { /*MU_M1|MU_M2|MU_TIMER|*/MU_MESAG };

  pop.mn_item = 6;
  apid = appl_init();
  new_aes = _GemParBlk.global[0] >= 0x400;
  if( new_aes ) shel_write( 9, 1, 0, 0L, 0L );
  /* check to make sure Geneva is active */
  if( CJar( 0, GENEVA_COOKIE, &cook ) != CJar_OK || !cook )
      /*form_alert( 1, "[1][|Geneva is not present][Ok]" ) */  ;
  else has_gnva++;
  if( rsrc_load("test.rsc") )
  {
/*    load_mouse( "h:\\mouseka\\exclama.dat", 0, ARROW );
    load_mouse( "h:\\mouseka\\jump.mkm", 1, BUSYBEE ); */
    gl1 = *(long *)&_GemParBlk.global[5];
    gl2 = *(long *)&_GemParBlk.global[7];
    if( rsrc_load( "c:\\DESKCICN.RSC" ) )
        rsrc_gaddr( 0, 0, &demo );
    *(long *)&_GemParBlk.global[5] = gl1;
    *(long *)&_GemParBlk.global[7] = gl2;
    graf_mouse( ARROW, 0 );
    rsrc_gaddr( 0, 0, &menu );
    menu[0].ob_state |= X_MAGIC;
    rsrc_gaddr( 0, POPUPS, &sub );
    subm.mn_tree = sub;
    subm.mn_menu = POP1;
    subm.mn_item = SUB3-1;
    subm.mn_scroll = 0;
    if( new_aes ) menu_attach( 1, menu, POPSTART, &subm );
    subm.mn_item = SUB3-2;
    if( new_aes ) menu_attach( 1, menu, POPSTART+1, &subm );
    pop.mn_tree = sub;
    pop.mn_menu = POP2;
    pop.mn_scroll = 0;
    pop.mn_item = POP2+10;
    if( new_aes ) menu_attach( 1, sub, MPOP, &pop );
    menu_bar( menu, 1 );
    rsrc_gaddr( 0, WINMENU, &winmenu );
    winmenu[0].ob_state |= X_MAGIC;
    rsrc_gaddr( 0, TREE6, &wintree );
    maxw = wintree[0].ob_width;
    wintree[0].ob_x = 50;
    wintree[0].ob_y = 50;
    if( has_gnva ) x_wind_calc( WC_BORDER, WTYPE, X_MENU, 50, 75, maxw, 100, &wx, &wy, &ww, &wh );
    else wind_calc( WC_BORDER, WTYPE, 50, 75, maxw, 100, &wx, &wy, &ww, &wh );
    if( has_gnva ) hand = x_wind_create( WTYPE, X_MENU, wx, wy, ww, wh );
    else hand = wind_create( WTYPE, wx, wy, ww, wh );
    if( hand>0 )
    {
      wind_set( hand, X_WF_MENU, winmenu );
      wind_set( hand, X_WF_DIALOG, wintree );
      wind_open( hand, wx, wy, ww, wh );
    }
/***    if( CJar( 0, 0x46534d43L, &cook ) == CJar_OK && *(long *)cook==0x5f535044L )
    {
      vdi = graf_handle( buf, buf, buf, buf );
      v_opnvwk( work_in, &vdi, work_out );
      vst_load_fonts( vdi, 0 );
      vst_font( vdi, 6154 );
      vst_setsize( vdi, 13, buf, buf, buf, buf );
/*      ascii(vdi);*/
      for( i=0; i<5; i++ )
      {
/*        Cconws( "\033Hg" );*/
        v_gtext( vdi, 0, i*18*2+50, charset[i] );
/*        Bconin(2);
        Cconws( "\033Hf" ); */
        ftext16( vdi, 0, i*18*2+18+50, charset[i] );
/*        Bconin(2); */
      }
      vst_unload_fonts( vdi, 0 );
      v_clsvwk( vdi );
/*      rsrc_gaddr( 0, 2, &ob );
      for( i=1; i<=5; i++ )
      {
        ob[i].ob_spec.tedinfo->te_font = GDOS_MONO;
        ob[i].ob_spec.tedinfo->te_junk1 = 3;
        ob[i].ob_spec.tedinfo->te_junk2 = 15;
      }
      ob[0].ob_y = 100;
      objc_draw( ob, 0, 8, 0, 0, 1000, 1000 );
      Bconin(2); */
    }*****/
    *(GRECT *)&e.m1x = *(GRECT *)&menu[1].ob_x;
    wind_get( 0, WF_WORKXYWH, &e.m2x, &e.m2y, &e.m2w, &e.m2h );
    for(;;)
    {
      multi_evnt( &e, buf );
      if( e.event&MU_TIMER )
      {
/*        wind_update( BEG_UPDATE ); */
        Cconws( "\033H\r\n\nTimer event" );
/*        wind_update( END_UPDATE );*/
      }
      if( e.event&MU_M1 )
      {
/*        wind_update( BEG_UPDATE );*/
        Cconws( "\033H\r\n\n\nInside R1 (menu)" );
/*        wind_update( END_UPDATE ); */
      }
      if( e.event&MU_M2 )
      {
/*        wind_update( BEG_UPDATE ); */
        Cconws( "\033H\r\n\n\nInside R2 (desk)" );
/*        wind_update( END_UPDATE ); */
      }
      if( e.event&MU_MESAG ) switch( buf[0] )
      {
        case AP_TERM:
          form_alert( 1, "[1][Got AP_TERM][OK]" );
          appl_exit();
          return 0;
        case SHUT_COMPLETED:
          form_alert( 1, "[1][SHUT_COMPLETED][OK]" );
          break;
        case AP_TFAIL:
          form_alert( 1, "[1][AP_TFAIL][OK]" );
          break;
        case WM_FULLED:
          wind_get( buf[3], WF_FULLXYWH, buf+4, buf+5, buf+6, buf+7 );
        case WM_MOVED:
        case WM_SIZED:
          dump( &buf[0], 5*2 );
          if( buf[4]==-1 ) break;
          if( buf[6]>maxw ) buf[6] = maxw;
          if( buf[7]>wintree[0].ob_height ) buf[7] = wintree[0].ob_height;
          wind_set( buf[3], WF_CURRXYWH, buf[4], buf[5], buf[6], buf[7] );
          break;
        case X_MN_SELECTED:
          graf_mouse( M_OFF, 0 );
          dump( &buf[3], 5*2 );
          graf_mouse( M_ON, 0 );
          menu_tnormal( winmenu, buf[3], 1 );
          break;
        case WM_TOPPED:
          wind_set( buf[3], WF_TOP );
          buf[4] = -1;
          buf[0] = WM_SIZED;
          appl_write( apid, 16, buf );
          break;
        case MN_SELECTED:
          if( !new_aes || *(OBJECT **)&buf[5] == menu ) switch( buf[4] )
          {
            case SET:
              graf_mouse( M_OFF, 0 );
              memset( &set, -1, sizeof(set) );
              state( "len=10" );
              x_settings( 0, 10, &set );
              dump( &set, sizeof(set) );
              memset( &set, -1, sizeof(set) );
              state( "len=-1" );
              x_settings( 0, -1, &set );
              dump( &set, sizeof(set) );
              state( "toggle pulldown menus" );
              set.flags.s.pulldown ^= 1;
              x_settings( 1, sizeof(set), &set );
              x_settings( 0, -1, &set );
              dump( &set, sizeof(set) );
              graf_mouse( M_ON, 0 );
              break;
            case SPRINT:
              x_sprintf( temp, "[1][xsprintf: %d %s %D][Ok]", 1, "2", 3L );
              form_alert( 1, temp );
              break;
            case BLIT:
              graf_mouse( M_OFF, 0 );
              wind_update( BEG_UPDATE );
              rsrc_gaddr( 0, 1, &ob );
              form_center( ob, &o.g_x, &o.g_y, &o.g_w, &o.g_h );
              state( "Save background" );
              x_graf_blit( &o, 0L );
              state( "Draw" );
              objc_draw( ob, 0, 8, o.g_x, o.g_y, o.g_w, o.g_h );
              state( "Unblit" );
              x_graf_blit( 0L, &o );
              state( "Draw" );
              objc_draw( ob, 0, 8, o.g_x, o.g_y, o.g_w, o.g_h );
              state( "Move" );
              r = o;
              for( i=0; i<5; i++ )
              {
                r.g_x += 20;
                r.g_y += 20;
                x_graf_blit( &o, &r );
                o = r;
              }
              wind_update( END_UPDATE );
              graf_mouse( M_ON, 0 );
              break;
            case UPDATE:
              form_dial( FMD_FINISH, 0, 0, 0, 0, 0, 0, 10000, 10000 );
              break;
            case TINFO:
              while( Kbshift(-1)!=3 ) trap2( 0xC9 );
/***              i = form_alert( 1, "[0][Shel write test mode:][0|1|Cancel]" ) - 1;
              if( i == 2 ) break;
              Dsetdrv(2); Dsetpath("\\");
              shel_write( i, 0, 0, "C:\\test.prg", "\0" );***/
/***              Cconws( "\033HPreempt wind_update test: both Shift keys to stop\n" );
              wind_update(BEG_UPDATE);
              while( Kbshift(-1)!=3 );
              appl_yield();
              while( Kbshift(-1)==3 );
              wind_update(END_UPDATE);***/
/*****              rsrc_gaddr( 0, 6, &ob );
/*              ob[1].ob_spec.tedinfo->te_tmplen = X_LONGEDIT;
              ob[2].ob_spec.tedinfo->te_tmplen = X_LONGEDIT;
              ob[4].ob_spec.tedinfo->te_tmplen = X_LONGEDIT;
              ob[4].ob_spec.tedinfo->te_ptext[10] = 0; */
              form_center( ob, &o.g_x, &o.g_y, &o.g_w, &o.g_h );
              objc_draw( ob, 0, 8, o.g_x, o.g_y, o.g_w, o.g_h );
              Cconws( "press esc to quit" );
              buf[0] = 0;
              do
              {
                objc_edit( ob, 1, 0, buf, ED_INIT );
              } while( (char)Bconin(2)!='\033' );
/*              ob[form_do(ob,1)].ob_state &= ~SELECTED; */
              form_dial( FMD_FINISH, 0, 0, 0, 0, 0, 0, 32000, 32000 );
/*              state( "appl_trecord" );
              appl_trecord( temp, sizeof(temp)/6 );
              dump( temp, sizeof(temp) );
              Bconin(2);
              state( "appl_tplay" );
              appl_tplay( temp, sizeof(temp)/6, 20 ); */ ********/
              break;
            case FORMS:
              if( demo ) ob = demo;
              else rsrc_gaddr( 0, 1, &ob );
              form_center( ob, &o.g_x, &o.g_y, &o.g_w, &o.g_h );
              objc_draw( ob, 0, 8, o.g_x, o.g_y, o.g_w, o.g_h );
              Bconin(2);
              form_dial( FMD_FINISH, 0, 0, 0, 0, 0, 0, 32000, 32000 );
              dump( ob[1].ob_spec.tedinfo, 100 );
              break;
            case POP:
              pop.mn_menu = POP2;
              pop.mn_scroll = pop.mn_item = POP2+1;
              graf_mkstate( buf, buf+1, buf+2, buf+2 );
              i = menu_popup( &pop, buf[0], buf[1], &pop2 );
              sprintf( temp, "[3][Return: %d|item: %d][Ok]", i, pop2.mn_item );
              pop.mn_item = pop2.mn_item;
              form_alert( 1, temp );
              break;
            case QUIT:
              appl_exit();
              return -1;
            default:
	      graf_mouse( M_OFF, 0 );
              dump( &buf[3], 5*2 );
              graf_mouse( M_ON, 0 );
          }
          else
          {
            graf_mouse( M_OFF, 0 );
            dump( &buf[3], 5*2 );
            graf_mouse( M_ON, 0 );
          }
          menu_tnormal( menu, buf[3], 1 );
          break;
        default:
          graf_mouse( M_OFF, 0 );
          dump( &buf[0], 8*2 );
          graf_mouse( M_ON, 0 );
          Bconin(2);
      }
    }
  }
  appl_exit();
  return 0;
}
