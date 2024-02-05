/*                    Geneva Application Skeleton
                              by Dan Wilga

 This source code can be freely incorporated into any application, in whole
 or in part. Gribnif Software makes no warranty as to the suitability of
 this code for any purpose.

 This code is intended primarily for use with the Pure/Turbo C compilers,
 but could be adapted to other environments. It is designed to aid in
 creating an application that works as a desk accessory and/or a regular
 PRG. It assumes that the application should open a window containing a
 dialog (like the Task Manager) and, optionally, a menu bar.

 By default, this code will compile without any changes. But it doesn't
 do much.

 Below are some constants used in this source that should be modified to
 suit the application being written:  */

#define USE_RSC       0                 /* set to !0 if using a resource (RSC) file */

#define USE_VDI       0                 /* set to !0 if using VDI, else 0 */

#define REQUIRE_GNVA  1                 /* set to !0 if Geneva is required, else 0 */

#define MODELESS      0                 /* set to !0 if using modeless dialogs */

#define AS_ACC        1                 /* set to !0 if desk accessory allowed */
#define AS_PRG        1                 /* set to !0 if application allowed */

/* set to the types of events that evnt_multi() should respond to */
#define EVNT_TYPE   MU_MESAG|X_MU_DIALOG

/* window type for the main window */
#define WIN_TYPE    SMALLER|FULLER|NAME|MOVER|SIZER|CLOSER|\
                    UPARROW|DNARROW|VSLIDE|\
                    LFARROW|RTARROW|HSLIDE
/* extended window type for Geneva's x_wind_create(). Set to 0 if not used. */
#define XWIN_TYPE   0   /* could be set to X_MENU for a menu bar */

#define USE_SMALLER 1   /* (WIN_TYPE&SMALLER) */
#define USE_MENU    0   /* (XWIN_TYPE&X_MENU) */

#if !USE_RSC    /* no resource file, so just define the strings */
  #define DESK_NAME   "  My Program\'s Name"
  #define RSC_NOTACC  "[1][This program cannot be|run as a desk accessory][OK]"
  #define RSC_NOTAPP  "[1][This program can only|be run as a desk accessory][OK]"
  #define RSC_NOGNVA  "[1][This program|requires Geneva][OK]"
  #define RSC_NOVDI   "[1][Could not open|a VDI workstation!][OK]"
  #define RSC_NOWIND  "[1][There are no more|windows available.|Please close a\
window|you no longer need.][OK]"

#else USE_RSC   /* using a RSC file, so define the constants */
  #define RSC_NAME    "resource.rsc"    /* set to the name of the resource */
  #define NO_RESOURCE "[1][Could not load|RESOURCE.RSC!][OK]"
  #include            "resource.h"      /* resource file constants, if using a RSC */
  #define RSC_DESK      X               /* set to the RSC file index of the name in the Desk menu */
  #define RSC_MAINDIAL  X               /* set to the RSC file index of the main dialog */
  #define RSC_NOWIND    X               /* set to the RSC file index of the message */
  #if !AS_ACC
    #define RSC_NOTACC  X               /* set to the RSC file index of the message */
  #endif
  #if !AS_PRG
    #define RSC_NOTAPP  X               /* set to the RSC file index of the message */
  #endif
  #if REQUIRE_GNVA
    #define RSC_NOGNVA  X               /* set to the RSC file index of the message */
  #endif
  #if USE_VDI
    #define RSC_NOVDI   X               /* set to the RSC file index of the message */
  #endif
  #if USE_MENU
    #define RSC_MENU    X               /* set to the RSC file index of the main menu */
  #endif
  #if USE_SMALLER
    #define RSC_ICONIFY X               /* set to the RSC file index of the iconify icon */
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

/* definitions for seeing if Geneva is present, by way of JARxxx */
#define CJar_cookie     0x434A6172L     /* "CJar" */
#define CJar_xbios      0x434A          /* "CJ" */
#define CJar_OK         0x6172          /* "ar" */
#define CJar(mode,cookie,value)         (int)xbios(CJar_xbios,mode,cookie,value)

char has_Geneva;        /* is Geneva running? */

int apid,               /* my application ID */
    main_hand;          /* window handle of main window */

GRECT winner =          /* dimensions of working area of main window */
        { 75, 75, 100, 100 },
    wsize,              /* dimensions of outer area of main window */
    max;                /* working area of desktop */

#if USE_MENU            /* if window has a menu bar */
  OBJECT *menu;         /* points to the main window menu tree */
#endif

#if USE_SMALLER         /* if window can be iconified */
  char iconified;       /* true if main window is iconified */
  #if USE_RSC
    OBJECT *icon;       /* points to the iconify icon in RSC */
  #else
    OBJECT icon[2] =
      {                 /* object tree of iconify "icon" (just a G_BOX here) */
        { -1, 1, 1,  G_BOX, 0, 0,       0L,        0, 0, 72, 40 },
        { 0, -1, -1, G_BOX, 0, CROSSED, 0xFFF000L, 0, 0, 32, 40 } };
  #endif
#endif

#if USE_RSC
  char **desk_name;     /* name in Desk menu for menu_register */
  OBJECT *main_dial;    /* object tree of dialog in main window */
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

#if MODELESS
  void start_form( int fnum, int tnum, int type, int xtype );
  /* include function prototypes for init, touch, and exit functions here */

  /* describe a modeless dialog */
  typedef struct
  {
    int handle;                           /* handle of window containing dial */
    OBJECT *tree;                         /* dial's object tree */
    int (*init)( OBJECT *o );             /* function to initialize it */
    void (*touch)( OBJECT *o, int num );  /* called when TOUCHEXIT is clicked */
    int (*exit)( OBJECT *o, int num );    /* called when EXIT is clicked */
    GRECT wind;                           /* outer window coords before iconify */
    int place;
  } FORM;

  FORM form[] = { /* list your dialogs here */
                  { 0, 0L, 0L } };                        /* end of list */
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

/********************* Dialog manager routines *********************/
#if MODELESS

/* calculate a window's border based on the size of an object tree */
void calc_bord( int type, int xtype, OBJECT *tree, GRECT *g )
{
  x_wind_calc( WC_BORDER, type, xtype, tree[0].ob_x, tree[0].ob_y,
      tree[0].ob_width, tree[0].ob_height, &g->g_x, &g->g_y,
      &g->g_w, &g->g_h );
}

/* Open a modeless dialog. Parameters:
     fnum:  index in the forms[] list
     tnum:  RSC file index of the tree
     type:  regular gadgets to include in the window
     xtype: Geneva-specific (extended) gadgets in the window */
void start_form( int fnum, int tnum, int type, int xtype )
{
  FORM *f = &form[fnum];
  int dum, hand;
  GRECT out;

  if( !has_Geneva ) return;             /* this code requires Geneva */
  if( f->handle>0 )                     /* window is already open */
      wind_set( f->handle, WF_TOP );    /* so top it, instead */
  else
  {
    if( !f->tree )                      /* dialog not used before */
    {
      rsrc_gaddr( 0, tnum, &f->tree );
      f->tree[1].ob_flags |= HIDETREE;  /* hide the "title" */
      if( f->wind.g_y>0 )
      {
        f->tree[0].ob_x = f->wind.g_x;
        f->tree[0].ob_y = f->wind.g_y;
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
    if( (hand=x_wind_create( type, xtype, out.g_x,
        out.g_y, out.g_w, out.g_h )) > 0 )
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
    else alert( RSC_NOWIND );
  }
}

/* process input from the user to a modeless dialog */
void use_form( int hand, int num )
{
  int i, j, but;
  FORM *f;

  if( !hand || !has_Geneva ) return;    /* this code requires Geneva */
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
        if( f->exit && (*f->exit)( f->tree, num ) ) close_wind(&f->handle);
        else x_wdial_draw( f->handle, but, 8 );         /* just draw button */
      }
      return;
    }
}

/* handle a click in the main window */
void click_main( int button )
{
  /* replace this with code to act on the button */
  button++;
}

/* handle a click in a dialog */
void do_dialog( int *buf )
{
  if( buf[3]==main_hand )               /* main window */
      click_main( buf[2]&0x7fff );      /* isolate just the object # */
  else use_form( buf[3], buf[2] );      /* process another dialog */
}
#endif MODELESS

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
#if USE_RSC
  rsrc_free();                          /* free resource */
#endif
  appl_exit();                          /* quit AES */
  exit(0);                              /* exit program */
}

#if USE_SMALLER
/* iconify the main window and close all other open windows */
void do_iconify( int handle, int buf[] )
{
#if MODELESS
  int top, dum, wind, i, place;
#endif

  if( !has_Geneva ) return;             /* this code requires Geneva */
#if MODELESS
  /* close all open dialog windows, but don't delete */
  wind_update( BEG_UPDATE );
  /* get the next window up from the desktop */
  wind_get( 0, WF_OWNER, &dum, &dum, &wind, &dum );
  for( place=0; wind; ) /* wind=0 when past top window */
  {
    for( i=0; form[i].init; i++ )
      if( form[i].handle==wind )        /* it's one of my windows */
      {
        wind_get( wind, WF_CURRXYWH, &form[i].wind.g_x, &form[i].wind.g_y,
            &form[i].wind.g_w, &form[i].wind.g_h );
        form[i].place = place++;        /* keep track of its order */
        /* get next window up before closing, because you can't find
           out the next window relative to one that's already closed! */
        wind_get( wind, WF_OWNER, &dum, &dum, &wind, &dum );
        wind_close( form[i].handle );   /* close, but don't delete */
        break;
      }
    if( !form[i].init ) /* do we already know the next window? */
    {
      /* get next window up */
      wind_get( wind, WF_OWNER, &dum, &dum, &wind, &dum );
    }
  }
  wind_update( END_UPDATE );
#endif MODELESS
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
#if MODELESS
  int i, place, count;
#endif MODELESS

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
#if MODELESS
  place = 0;
  do
  {
    for( count=i=0; form[i].init; i++ )
      if( form[i].handle>0 )
        if( form[i].place==place )
        {
          wind_open( form[i].handle, form[i].wind.g_x, form[i].wind.g_y,
              form[i].wind.g_w, form[i].wind.g_h );
          place++;
        }
        else if( form[i].place > place ) count++;  /* go back again */
  } while( count );
#endif MODELESS
  iconified = 0;
}
#endif USE_SMALLER

void do_msg( int *buf )
{
  int i;
  static char was_open;     /* has main window already been opened? */

  switch( buf[0] )
  {
    case AC_OPEN:
      if( main_hand<=0 )
      {
        if( !was_open )
        {
          main_dial[0].ob_x = winner.g_x;
          main_dial[0].ob_y = winner.g_y;
          /* make dialog size of the desktop */
          main_dial[0].ob_width = max.g_w;
          main_dial[0].ob_height = max.g_h;
          get_outer();
          was_open = 1;
        }
        if( has_Geneva ) main_hand =
            x_wind_create( WIN_TYPE, XWIN_TYPE,
            wsize.g_x, wsize.g_y, wsize.g_w, wsize.g_h );
#if !REQUIRE_GNVA
        else main_hand =
            wind_create( WIN_TYPE,
            wsize.g_x, wsize.g_y, wsize.g_w, wsize.g_h );
#endif
        if( main_hand > 0 )
        {
          get_inner();
          /* position the window dialog to the start */
          main_dial[0].ob_x = winner.g_x;
          main_dial[0].ob_y = winner.g_y;
          if( has_Geneva )
          {
            /* tell Geneva it's a windowed dialog */
            wind_set( main_hand, X_WF_DIALOG, main_dial );
#if USE_MENU
            /* put in the menu */
            wind_set( main_hand, X_WF_MENU, menu );
#endif
          }
          /* default window name */
          wind_set( main_hand, WF_NAME, "" );
          wind_open( main_hand, wsize.g_x, wsize.g_y, wsize.g_w, wsize.g_h );
        }
        else alert( RSC_NOWIND );
        break;
      }
      buf[3] = main_hand;       /* fall through if already open */
    case WM_TOPPED:
      wind_set( buf[3], WF_TOP, buf[3] );
      break;
    case WM_FULLED:
      break;                    /* add your own code here */
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
#if MODELESS
      for( i=0; form[i].init; i++ )
        form[i].handle = 0;
#endif MODELESS
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
#if USE_SMALLER
      if( !iconified )
#endif USE_SMALLER
        switch( buf[4] )
      {
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

#if USE_RSC
int load_rsc(void)
{
  if( rsrc_load( RSC_NAME ) )
  {
    rsrc_gaddr( 0, RSC_MAINDIAL, &main_dial ); /* get the main window dialog */
#if USE_RSC
    rsrc_gaddr( 15, RSC_DESK, &desk_name );    /* get the name in the Desk menu */
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
#endif USE_RSC

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
  int event, dum;

  for(;;)
  {
    /* change this to include the parameters you need */
    event = evnt_multi( EVNT_TYPE,  0,0,0,  0,0,0,0,0,
        0,0,0,0,0,  buf,  0,0,  &dum, &dum, &dum, &dum, &dum, &dum );

    if( event&MU_MESAG ) do_msg(buf);              /* process a message */

#if MODELESS
    if( event&X_MU_DIALOG ) do_dialog(buf);        /* user clicked in a dialog window */
#endif

    /* add other types of events here */
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

  /* Tell the AES (or Geneva) that we understand AP_TERM message */
  if( _GemParBlk.global[0] >= 0x400 ) shel_write( SHW_MSGTYPE, 1, 0, 0L, 0L );

#if USE_RSC
  if( ok ) ok = load_rsc();
#endif USE_RSC

#if USE_VDI
  if( ok ) ok = open_vdi();
#endif USE_VDI

  has_Geneva = CJar( 0, GENEVA_COOKIE, &cookie ) == CJar_OK && cookie;
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
