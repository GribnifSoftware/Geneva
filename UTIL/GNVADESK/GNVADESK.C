/*                    Geneva Desktop Pattern
                           by Dan Wilga

*/

#define USE_RSC       1                 /* set to !0 if using a resource (RSC) file */

#define USE_VDI       1                 /* set to !0 if using VDI, else 0 */

#define REQUIRE_GNVA  1                 /* set to !0 if Geneva is required, else 0 */

#define MODELESS      0                 /* set to !0 if using modeless dialogs */

#define AS_ACC        1                 /* set to !0 if desk accessory allowed */
#define AS_PRG        0                 /* set to !0 if application allowed */

/* set to the types of events that evnt_multi() should respond to */
#define EVNT_TYPE   MU_MESAG|X_MU_DIALOG

/* window type for the main window */
#define WIN_TYPE    SMALLER|NAME|MOVER|CLOSER
/* extended window type for Geneva's x_wind_create(). Set to 0 if not used. */
#define XWIN_TYPE   X_MENU

#define USE_SMALLER 1   /* (WIN_TYPE&SMALLER) */
#define USE_MENU    1   /* (XWIN_TYPE&X_MENU) */

#if !USE_RSC    /* no resource file, so just define the strings */
  #define DESK_NAME   "  My Program\'s Name"
  #define RSC_NOTACC  "[1][This program cannot be|run as a desk accessory][OK]"
  #define RSC_NOTAPP  "[1][This program can only|be run as a desk accessory][OK]"
  #define RSC_NOGNVA  "[1][This program|requires Geneva][OK]"
  #define RSC_NOVDI   "[1][Could not open|a VDI workstation!][OK]"
  #define RSC_NOWIND  "[1][There are no more|windows available.|Please close a\
window|you no longer need.][OK]"

#else USE_RSC   /* using a RSC file, so define the constants */
  #define CNF_VER     "1.00"
  #define RSC_NAME    "gnvadesk.rsc"    /* set to the name of the resource */
  #define NO_RESOURCE "[1][Could not load|GNVADESK.RSC!][OK]"
  #include            "gnvadesk.h"      /* resource file constants, if using a RSC */
  #define RSC_DESK      TITLE               /* set to the RSC file index of the name in the Desk menu */
  #define RSC_MAINDIAL  MDIAL               /* set to the RSC file index of the main dialog */
  #define RSC_NOWIND    NOWIND               /* set to the RSC file index of the message */
  #if !AS_ACC
    #define RSC_NOTACC  X               /* set to the RSC file index of the message */
  #endif
  #if !AS_PRG
    #define RSC_NOTAPP  NOTAPP               /* set to the RSC file index of the message */
  #endif
  #if REQUIRE_GNVA
    #define RSC_NOGNVA  ALNOGEN               /* set to the RSC file index of the message */
  #endif
  #if USE_VDI
    #define RSC_NOVDI   ALNOVDI               /* set to the RSC file index of the message */
  #endif
  #if USE_MENU
    #define RSC_MENU    MMENU              /* set to the RSC file index of the main menu */
  #endif
  #if USE_SMALLER
    #define RSC_ICONIFY MICON               /* set to the RSC file index of the iconify icon */
  #endif
#endif USE_RSC

/* modify the #include list to suit your needs */
#include "new_aes.h"            /* includes aes.h */
#include "vdi.h"
#include "xwind.h"              /* Geneva extensions */
#include "tos.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "\source\neodesk\neocommn.h"

/* definitions for seeing if Geneva is present, by way of JARxxx */
#define CJar_cookie     0x434A6172L     /* "CJar" */
#define CJar_xbios      0x434A          /* "CJ" */
#define CJar_OK         0x6172          /* "ar" */
#define CJar(mode,cookie,value)         (int)xbios(CJar_xbios,mode,cookie,value)

char has_Geneva,
     have_desk,
     neo_pat, neo_pic,
     tneo_pat, tneo_pic;

int apid,               /* my application ID */
    fillnum=4,
    colnum=3,
    tfillnum,
    tcolnum,
    neo_apid,
    main_hand;          /* window handle of main window */

NEO_ACC *neo_acc;

LoadCookie *lc;

GRECT winner,          /* dimensions of working area of main window */
    wsize,              /* dimensions of outer area of main window */
    max;                /* working area of desktop */

#if USE_MENU            /* if window has a menu bar */
  OBJECT *menu;         /* points to the main window menu tree */
#endif

#if USE_SMALLER         /* if window can be iconified */
  char iconified;       /* true if main window is iconified */
  OBJECT *icon;       /* points to the iconify icon in RSC */
#endif

#if USE_RSC
  char **desk_name;     /* name in Desk menu for menu_register */
  OBJECT *main_dial, *fills, *colors,
      my_desk[1] = { -1, -1, -1, G_USERDEF, 0, 0, 0L,  0, 0, 0, 0 };
#else
  OBJECT main_dial[1] =
      {  /* object tree of dialog in main window */
        -1, -1, -1, G_BOX, 0, 0, 0L,  0, 0, 0, 0 };
#endif

#if USE_VDI
  int vdi_hand,         /* VDI device handle */
      work_in[] = { 1, 7, 1, 1, 1, 1, 1, 1, 1, 1, 2 },
      work_out[57];     /* returns from v_opnvwk() */
#endif

#if USE_RSC
/* Put up an alert, taking a string from the resource file */
int alert( int num )
{
  char **ptr;

  rsrc_gaddr( 15, num, &ptr );
  return form_alert( 1, *ptr );
}
#else USE_RSC
/* Put up an alert, using string passed to function */
int alert( char *ptr )
{
  return form_alert( 1, ptr );
}
#endif USE_RSC

/* Close and delete a window */
void close_wind( int *hand )
{
  if( *hand>=0 )
  {
    wind_close(*hand);
    wind_delete(*hand);
    *hand=0;
  }
}

void set_fsamp(void)
{
  main_dial[MPAT].ob_type = main_dial[MSAMP].ob_type =
      fills[tfillnum+1].ob_type;
  main_dial[MSAMP].ob_flags = tcolnum<<12;
  main_dial[MCOL].ob_spec.obspec.interiorcol = tcolnum;
}

void draw_samp(void)
{
  x_wdial_draw( main_hand, MSAMP, 8 );
}

unsigned int do_popup( OBJECT *o, int obj, OBJECT *pop, unsigned int val )
{
  MENU m, out;
  int x, y;

  m.mn_tree = pop;
  m.mn_menu = 0;
  m.mn_item = val+1;
  m.mn_scroll = 0;
  /* If this is the patterns or colors popup, then display a checkmark
     at the right value by setting it to a BOXCHAR */
  if( (char)pop[1].ob_type==G_BOX || (char)pop[1].ob_type==G_BOXCHAR )
    for( x=1; x<=pop[0].ob_tail; x++ )
      pop[x].ob_type = x==val+1 ? G_BOXCHAR : G_BOX;
  objc_offset( o, obj, &x, &y );
  if( x + pop[0].ob_width > max.g_w ) x = max.g_w - pop[0].ob_width;
  if( menu_popup( &m, x, y, &out ) ) return out.mn_item-1;
  return val;
}

void new_fill(void)
{
  set_fsamp();
  x_wdial_draw( main_hand, MPAT, 8 );
  draw_samp();
}

void new_color(void)
{
  set_fsamp();
  x_wdial_draw( main_hand, MCOL, 8 );
  draw_samp();
}

void set_if( OBJECT *tree, int num, int true )
{
  if( true ) tree[num].ob_state |= SELECTED;
  else tree[num].ob_state &= ~SELECTED;
}

int retries;
int find_neo( int *old )
{
  int buf[8];

  /* throw away if too many tries */
  if( old && ++retries>10 ) return 0;
  /* try to find NeoDesk */
  if( (neo_apid = appl_find("NEODESK ")) >= 0 )
  {
    /* put the message back in my queue */
    if( old ) appl_write( apid, 16, old );
    buf[0] = NEO_ACC_ASK;
    buf[1] = apid;
    buf[3] = NEO_ACC_MAGIC;
    buf[4] = apid;
    appl_write( neo_apid, 16, buf );
  }
  else return 0;
  return 1;
}

void new_desk(void)
{
  int x, y, w, h, id, dum;

  if( !neo_acc && (neo_pat || neo_pic) ) find_neo(0L);
  *(char *)&my_desk[0].ob_type = *(char *)&fills[fillnum+1].ob_type;
  my_desk[0].ob_flags = (colnum<<12) | (1<<11)/* no border */;
  wind_get( 0, WF_OWNER, &id, &dum, &dum, &dum );
  if( id == apid || id == 1 )
  {
    wind_update( BEG_UPDATE );
    wind_get( 0, WF_FIRSTXYWH, &x, &y, &w, &h );
    while( w && h )
    {
      form_dial( FMD_FINISH, 0, 0, 0, 0, x, y, w, h );
      wind_get( 0, WF_NEXTXYWH, &x, &y, &w, &h );
    }
    wind_update( END_UPDATE );
  }
}

void quit(void)
{
#if MODELESS
  int i;

  for( i=0; form[i].init; i++ )
    close_wind( &form[i].handle );      /* close any open dialogs */
#endif
  close_wind( &main_hand );             /* close main window */
#if USE_VDI
  if( vdi_hand ) v_clsvwk( vdi_hand );  /* close VDI workstation */
#endif
  if( have_desk ) wind_set( 0, X_WF_DFLTDESK, 0L, 1 );
#if USE_RSC
  rsrc_free();                          /* free resource */
#endif
  appl_exit();                          /* quit AES */
  exit(0);                              /* exit program */
}

void exit_main( int set, int close )
{
  if( close )
  {
    close_wind(&main_hand);
    if( _app ) quit();
  }
  if( set )
  {
    neo_pat = tneo_pat = main_dial[MNPAT].ob_state&SELECTED;
    neo_pic = tneo_pic = main_dial[MNPIC].ob_state&SELECTED;
    colnum = tcolnum;
    fillnum = tfillnum;
    new_desk();
  }
}

/* handle a click in the main window */
void click_main( int button )
{
  int i;

  switch( button )
  {
    case MPLEFT:
      if( --tfillnum<0 ) tfillnum = 35;
      new_fill();
      break;
    case MPRT:
      if( ++tfillnum>35 ) tfillnum = 0;
      new_fill();
      break;
    case MPAT:
      if( (i = do_popup( main_dial, MPAT, fills, tfillnum )) != tfillnum )
      {
        tfillnum = i;
        new_fill();
      }
      break;
    case MCLEFT:
      if( --tcolnum<0 ) tcolnum = 15;
      new_color();
      break;
    case MCRT:
      if( ++tcolnum>15 ) tcolnum = 0;
      new_color();
      break;
    case MCOL:
      if( (i = do_popup( main_dial, MCOL, colors, tcolnum )) != tcolnum )
      {
        tcolnum = i;
        new_color();
      }
      break;
    case MSET:
      exit_main( 1, 0 );
      break;
    case MOK:
      exit_main( 1, 1 );
      break;
    case MCANC:
      tcolnum = colnum;
      tfillnum = fillnum;
      tneo_pat = neo_pat;
      tneo_pic = neo_pic;
      exit_main( 0, 1 );
      break;
  }
  if( main_dial[button].ob_flags & EXIT )
  {
    set_if( main_dial, button, 0 );
    if( main_hand>0 ) x_wdial_draw( main_hand, button, 8 );
  }
}

/* handle a click in a dialog */
void do_dialog( int *buf )
{
  if( buf[3]==main_hand )               /* main window */
      click_main( buf[2]&0x7fff );      /* isolate just the object # */
}

/* generalized version of wind_calc that works with or without Geneva */
void wcalc( int type, int x, int y, int w, int h,
            int *ox, int *oy, int *ow, int *oh )
{
#if !REQUIRE_GNVA
  if( !has_Geneva ) wind_calc( type, WIN_TYPE, x, y, w, h,
      ox, oy, ow, oh );
  else
#endif
      x_wind_calc( type, WIN_TYPE, XWIN_TYPE, x, y, w, h,
      ox, oy, ow, oh );
}

/* calculate the size of the main dialog */
void get_inner(void)
{
  wcalc( WC_WORK, wsize.g_x, wsize.g_y, wsize.g_w, wsize.g_h,
      &winner.g_x, &winner.g_y, &winner.g_w, &winner.g_h );
}

/* calculate the size of the outer window */
void get_outer(void)
{
  wcalc( WC_BORDER, winner.g_x, winner.g_y, winner.g_w, winner.g_h,
      &wsize.g_x, &wsize.g_y, &wsize.g_w, &wsize.g_h );
}

#if USE_SMALLER
/* iconify the main window and close all other open windows */
void do_iconify( int handle, int buf[] )
{
  if( !has_Geneva ) return;             /* this code requires Geneva */
  /* turn dialog off so that applist's size and position will not change */
  wind_set( handle, X_WF_DIALOG, 0L );
  wind_set( handle, WF_ICONIFY, buf[4], buf[5], buf[6], buf[7] );
  /* get working area of iconified window */
  wind_get( handle, WF_WORKXYWH, &icon[0].ob_x, &icon[0].ob_y,
      &icon[0].ob_width, &icon[0].ob_height );
  /* center the icon within the form */
  icon[1].ob_x = (icon[0].ob_width - icon[1].ob_width) >> 1;
  icon[1].ob_y = (icon[0].ob_height - icon[1].ob_height) >> 1;
  /* new (buttonless) dialog in main window */
  wind_set( handle, X_WF_DIALOG, icon );
  iconified = 1;
}

/* uniconify the main window and/or dialogs */
void do_uniconify( int handle, int buf[] )
{
  if( !has_Geneva ) return; /* this code requires Geneva */
  /* briefly select the icon */
  x_wdial_change( handle, 1, icon[1].ob_state|SELECTED );
  icon[1].ob_state &= ~SELECTED;
  /* turn dialog off so that icon's size and position will not change */
  wind_set( handle, X_WF_DIALOG, 0L );
  wind_set( handle, WF_UNICONIFY, buf[4], buf[5], buf[6], buf[7] );
  /* restore old dialog */
  wind_set( handle, X_WF_DIALOG, main_dial );
  /* reopen all dialog windows, bottom first, ending with top */
  iconified = 0;
}
#endif USE_SMALLER

int make_form( int tree )
{
  GRECT r;
  int i, j, dif;
  OBJECT *form;

  rsrc_gaddr( 0, tree, &form );
  x_form_center( form, &r.g_x, &r.g_y, &r.g_w, &r.g_h );
  i = form_dial( X_FMD_START, 0, 0, 0, 0, r.g_x, r.g_y, r.g_w, r.g_h );
  if( !i ) form[1].ob_flags |= HIDETREE;
  else form[1].ob_flags &= ~HIDETREE;
  objc_draw( form, 0, 8, r.g_x, r.g_y, r.g_w, r.g_h );
  dif = form[0].ob_x - r.g_x;
  j = form_do( form, 0 );
  if( j > 0 && form[j].ob_flags&EXIT )
      set_if( form, j, 0 );
  r.g_x = form[0].ob_x - dif;
  r.g_y = form[0].ob_y - dif;
  form_dial( i ? X_FMD_FINISH : FMD_FINISH, 0, 0, 0, 0, r.g_x, r.g_y,
      r.g_w, r.g_h );
  return j;
}

void do_help( char *topic )
{
  char path[120], temp[120], *p, *t;

  strcpy( path, "GNVADESK.HLP" );
  if( !shel_find(path) ) strcpy( path, "GNVADESK.HLP" );
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

void save_settings(void)
{
  int ok;
  char temp[50];

  graf_mouse( BUSYBEE, 0 );
  if( x_shel_put( X_SHOPEN, "Geneva Desk" ) )
  {
    ok = x_shel_put( X_SHACCESS, CNF_VER );
    if( ok>0 )
    {
      x_sprintf( temp, "%d %d %x %x %b %b", wsize.g_x, wsize.g_y,
          colnum, fillnum, neo_pat, neo_pic );
      ok = x_shel_put( X_SHACCESS, temp );
    }
    if( ok>0 ) x_shel_put( X_SHCLOSE, 0L );
  }
  graf_mouse( ARROW, 0 );
}

void load_settings(void)
{
  char temp[50];
  int ok;

  while( (ok=x_shel_get( X_SHOPEN, 0, "Geneva Desk" )) == -1 );
  if( ok>0 )
  {
    ok = x_shel_get( X_SHACCESS, sizeof(temp), temp );
    if( !strcmp( temp, CNF_VER ) )
    {
      ok = x_shel_get( X_SHACCESS, sizeof(temp), temp );
      if( ok>0 )
      {
        x_sscanf( temp, "%d %d %x %x %b %b", &wsize.g_x, &wsize.g_y,
            &colnum, &fillnum, &neo_pat, &neo_pic );
        if( wsize.g_y < max.g_y ) wsize.g_y = max.g_y;
        if( wsize.g_y >= max.g_y+max.g_h ) wsize.g_y = max.g_y+max.g_h-1;
        if( wsize.g_x >= max.g_w+20 ) wsize.g_x = max.g_w-21;
      }
    }
    if( ok>0 ) x_shel_get( X_SHCLOSE, 0, 0L );
  }
}

void ack(void)
{
  int buf[8];

  buf[0] = DUM_MSG;
  buf[1] = apid;
  appl_write( neo_apid, 16, buf );
}

void do_msg( int *buf )
{
  int i;
  char **hlp;
  static char was_open;     /* has main window already been opened? */

  switch( buf[0] )
  {
    case NEO_ACC_INI:
      if( buf[3]==NEO_ACC_MAGIC )
      {
        (void)CJar( 0, LOAD_COOKIE, (long *)&lc );
        neo_acc = *(NEO_ACC **)&buf[4];
        neo_apid = buf[6];
        retries = 0;
        if( neo_pat || neo_pic ) new_desk();
      }
      break;
    case NEO_CLI_RUN:
    case NEO_ACC_PAS:
      if( buf[3]==NEO_ACC_MAGIC )
        if( !neo_acc ) find_neo(buf);
        else ack();
      break;
    case NEO_ACC_BAD:
      if( buf[3] == NEO_ACC_MAGIC ) neo_acc=0;
      break;
    case NEO_AC_OPEN:
    case AC_OPEN:
      if( main_hand<=0 )
      {
        if( !was_open )
        {
          get_inner();
          main_dial[0].ob_x = winner.g_x;
          main_dial[0].ob_y = winner.g_y;
          winner.g_w = main_dial[0].ob_width;
          winner.g_h = main_dial[0].ob_height;
          get_outer();
          was_open = 1;
        }
        main_hand =
            x_wind_create( WIN_TYPE, XWIN_TYPE,
            wsize.g_x, wsize.g_y, wsize.g_w, wsize.g_h );
        if( main_hand > 0 )
        {
          set_if( main_dial, MNPAT, tneo_pat );
          set_if( main_dial, MNPIC, tneo_pic );
          set_fsamp();
          get_inner();
          /* position the window dialog to the start */
          main_dial[0].ob_x = winner.g_x;
          main_dial[0].ob_y = winner.g_y;
          /* tell Geneva it's a windowed dialog */
          wind_set( main_hand, X_WF_DIALOG, main_dial );
#if USE_MENU
          /* put in the menu */
          wind_set( main_hand, X_WF_MENU, menu );
#endif
          /* default window name */
          wind_set( main_hand, WF_NAME, *desk_name+2 );
          wind_open( main_hand, wsize.g_x, wsize.g_y, wsize.g_w, wsize.g_h );
        }
        else alert( RSC_NOWIND );
        break;
      }
      buf[3] = main_hand;       /* fall through if already open */
    case WM_TOPPED:
      wind_set( buf[3], WF_TOP, buf[3] );
      break;
    case WM_MOVED:
    case WM_SIZED:
      wind_set( buf[3], WF_CURRXYWH, buf[4], buf[5], buf[6], buf[7] );
      if( buf[3]==main_hand )
      {
        wsize.g_x = buf[4];
        wsize.g_y = buf[5];
        get_inner();
      }
      break;
    case WM_CLOSED:
      if( buf[3]==main_hand )   /* main window? */
      {
        close_wind(&main_hand);
        if( _app ) quit();      /* running as a PRG, so quit */
      }
      break;
    case AC_CLOSE:
      main_hand=0;
#if USE_SMALLER
      iconified = 0;            /* just in case it was */
#endif
      break;
    case AP_TERM:
      /* I'm being told to quit by the AES */
      quit();
      break;
#if USE_SMALLER
    case WM_ICONIFY:
    case WM_ALLICONIFY:     /* treating both messages the same way */
      if( buf[3]==main_hand && !iconified ) /* main window */
          do_iconify( main_hand, buf );
      break;
    case WM_UNICONIFY:
      if( buf[3]==main_hand && iconified )  /* main window */
          do_uniconify( main_hand, buf);
      break;
#endif USE_SMALLER
#if USE_MENU
    case X_MN_SELECTED:
      /* handle main window menu here */
      if( !iconified ) switch( buf[4] )
      {
        case MQUIT:
          close_wind( &main_hand );
          if( _app ) quit();      /* running as a PRG, so quit */
          break;
        case MABOUT:
          make_form( ABOUT );
          break;
        case MSAVE:
          exit_main( 1, 0 );
          save_settings();
          break;
        case MHELP:
          rsrc_gaddr( 15, HELPMAIN, &hlp );
          do_help(*hlp);
          break;
      }
      menu_tnormal( menu, buf[3], 1 );
      break;
#endif USE_MENU
  }
}

/* An error occurred during initialization. */
void init_error(void)
{
  int buf[8];

  /* If either running as an app or under Geneva or MultiTOS, get out */
  if( _app || _GemParBlk.global[1]==-1 ) quit();
  /* run as a desk accessory, but an error occurred, so just go into an
     infinite loop looking for AP_TERM messages */
  for(;;)
  {
    evnt_mesag(buf);
    if( buf[0]==AP_TERM ) quit();
  }
}

void pline_inout( int *oarray, int i )
{
  oarray[6] = oarray[0] += i;
  oarray[4] = oarray[2] -= i;
  oarray[3] = oarray[1] += i;
  oarray[7] = oarray[5] -= i;
}
void pline_in( int *oarray )
{
  pline_inout( oarray, 1 );
}
void pline_out( int *oarray )
{
  pline_inout( oarray, -1 );
}

void draw_bx( int *box )
{
  int i, new[10], *n=new;

  for( i=8; --i>=0; )
    *n++ = *box++;
  *n++ = *(box-8);
  *n = *(box-7);
  v_pline( vdi_hand, 5, new );
}

void in_frame( int *box, int col )
{
  pline_in(box);
  vsl_color( vdi_hand, col );
  draw_bx(box);
}

void offset_objc( OBJECT *tree, int obj, int *x, int *y )
{
  register int parent=1, lastobj;

  *x = *y = 0;
  do
  {
    if( parent )
    {
      parent=0;
      *x += tree[obj].ob_x;
      *y += tree[obj].ob_y;
    }
    if( tree[obj = tree[lastobj=obj].ob_next].ob_tail == lastobj ) parent++;
  }
  while( obj >= 0 && lastobj );
}

void black_box( int *box, int w, int h, int out )
{
  box[2] = box[4] = (box[6] = box[0]) + w-1;
  box[5] = box[7] = (box[3] = box[1]) + h-1;
  if( out ) pline_out(box);
  vswr_mode( vdi_hand, MD_REPLACE );
  vsl_color( vdi_hand, 1 );
  draw_bx( box );
}

int cdecl ub_pattern( PARMBLK *pb )
{
  int c[8], i;
  OBJECT *o;
  MOST *z;
  unsigned long old_pic;
  long oldxy;

  if( pb->pb_tree!=fills && pb->pb_tree!=main_dial &&
      (neo_pat || neo_pic) )
    if( !lc || !lc->mas ) neo_acc = 0L;		/* has Neo quit? */
    else if( neo_acc && neo_acc->nac_ver >= 0x400 &&
        (z=neo_acc->mas->most)->desk &&
        (neo_pic && z->pic_ptr || neo_pat) )
    {
      oldxy = *(long *)&z->desk[0].ob_x;
      *(long *)&z->desk[0].ob_x = *(long *)&my_desk[0].ob_x;
      if( z->pic_ptr && !neo_pic )
      {
        old_pic = z->pic_ptr;
        z->pic_ptr = 0L;
      }
      else old_pic = 0L;
      i = (*z->desk[0].ob_spec.userblk->ub_code)(pb);
      if( old_pic ) z->pic_ptr = old_pic;
      *(long *)&z->desk[0].ob_x = oldxy;
      return i;
    }
  c[2] = (c[0] = pb->pb_xc) + pb->pb_wc - 1;
  c[3] = (c[1] = pb->pb_yc) + pb->pb_hc - 1;
  vs_clip( vdi_hand, 1, c );
  vswr_mode( vdi_hand, MD_REPLACE );
  i = *(unsigned char *)&((o=&pb->pb_tree[pb->pb_obj])->ob_type);
  vsf_interior( vdi_hand, i&0x80 ? FIS_HATCH : FIS_PATTERN );
  vsf_style( vdi_hand, i&0x7f );
  vsf_color( vdi_hand, o->ob_flags>>12 );
  offset_objc( pb->pb_tree, pb->pb_obj, &c[0], &c[1] );
  c[2] = o->ob_width + c[0] - 1;
  c[3] = o->ob_height + c[1] - 1;
  vr_recfl( vdi_hand, c );
  black_box( c, o->ob_width, o->ob_height, o->ob_flags&(1<<11) );
  if( pb->pb_currstate&SELECTED )
  {
    in_frame( c, 0 );
    in_frame( c, 1 );
  }
  return 0;
}

int load_rsc(void)
{
  static USERBLK ubp = { ub_pattern };
  int i;
  OBJECT *o;

  if( rsrc_load( RSC_NAME ) )
  {
    rsrc_gaddr( 0, RSC_MAINDIAL, &main_dial ); /* get the main window dialog */
    my_desk[0].ob_spec.userblk = main_dial[MSAMP].ob_spec.userblk =
        main_dial[MPAT].ob_spec.userblk = &ubp;
    rsrc_gaddr( 0, PFILLS, &fills );
    for( i=36, o=fills+1; --i>=0; o++ )
    {
      *((char *)&o->ob_type+1) = G_USERDEF;
      o->ob_spec.userblk = &ubp;
      o->ob_flags |= 1<<12;	/* black fill */
    }
    *(GRECT *)&my_desk[0].ob_x = max;
    rsrc_gaddr( 0, PCOLORS, &colors );
#if USE_RSC
    rsrc_gaddr( 15, RSC_DESK, &desk_name );     /* get the name in the Desk menu */
#endif
#if USE_MENU            /* if window has a menu bar */
    rsrc_gaddr( 0, RSC_MENU, &menu );          /* get the main window menu */
#endif
#if USE_SMALLER         /* if window can be iconified */
    rsrc_gaddr( 0, RSC_ICONIFY, &icon );       /* get the icon */
#endif
    return 1;
  }
  form_alert( 1, NO_RESOURCE );
  return 0;
}

#if USE_VDI
int open_vdi(void)
{
  int dum;

  vdi_hand = graf_handle( &dum, &dum, &dum, &dum );
  work_in[0] = Getrez() + 2;
  v_opnvwk( work_in, &vdi_hand, work_out );
  if( !vdi_hand )
  {
    alert( RSC_NOVDI );           /* could not open VDI workstation */
    return 0;
  }
  return 1;
}
#endif

void event_loop( int *buf )
{
  int event, dum, e;

  for(;;)
  {
    e = !neo_acc && (neo_pat || neo_pic) ? EVNT_TYPE|MU_TIMER : EVNT_TYPE;
    event = evnt_multi( e,  0,0,0,  0,0,0,0,0,
        0,0,0,0,0,  buf,  250,0,  &dum, &dum, &dum, &dum, &dum, &dum );

    if( event&MU_MESAG ) do_msg(buf);              /* process a message */

    if( e&MU_TIMER && !neo_acc ) find_neo(0L);

    if( event&X_MU_DIALOG ) do_dialog(buf);        /* user clicked in a dialog window */
  }
}

void main(void)
{
  char ok=1;            /* successful load of RSC, VDI, etc. */
  G_COOKIE *cookie;
  int buf[8];

  apid = appl_init();
  /* get desktop dimensions */
  wind_get( 0, WF_WORKXYWH, &max.g_x, &max.g_y, &max.g_w, &max.g_h );
  wsize.g_x = wsize.g_y = max.g_y + 5;

  /* Tell the AES (or Geneva) that we understand AP_TERM message */
  if( _GemParBlk.global[0] >= 0x400 ) shel_write( SHW_MSGTYPE, 1, 0, 0L, 0L );

#if USE_RSC
  if( ok ) ok = load_rsc();
#endif USE_RSC

#if USE_VDI
  if( ok ) ok = open_vdi();
#endif USE_VDI

  has_Geneva = CJar( 0, GENEVA_COOKIE, &cookie ) == CJar_OK && cookie &&
      cookie->ver >= 0x104;
#if REQUIRE_GNVA
  if( !has_Geneva )
  {
    alert( RSC_NOGNVA );
    ok = 0;
  }
#endif REQUIRE_GNVA

  if( ok )
  {
    /* Set the name in the Desk menu. With Geneva or MultiTOS,
       you can do this even when it's not running as an accessory */
    if( !_app || _GemParBlk.global[0] >= 0x400 )
    {
#if USE_RSC
      menu_register( apid, *desk_name );
#else
      menu_register( apid, DESK_NAME );
#endif
    }
    load_settings();
    tcolnum = colnum;
    tfillnum = fillnum;
    tneo_pat = neo_pat;
    tneo_pic = neo_pic;
    have_desk = 1;
    wind_set( 0, X_WF_DFLTDESK, my_desk, 1 );
    new_desk();
    if( _app )                      /* running as PRG */
    {
      graf_mouse( ARROW, 0L );
#if AS_PRG
      buf[0] = AC_OPEN;
      do_msg( buf );
      event_loop( buf );
#else AS_PRG
      alert( RSC_NOTAPP );
      quit();
#endif
    }
#if !AS_ACC
    else
    {
      alert( RSC_NOTACC );
      init_error();
    }
#endif /* !AS_ACC */
    event_loop( buf );
  }
  init_error();
}
