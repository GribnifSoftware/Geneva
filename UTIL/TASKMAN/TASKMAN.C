/*                    The Geneva Task Manager
                            by Dan Wilga
              Copyright � 1994-95, Gribnif Software
                        All rights reserved.

 This source code is provided to registered owners of Geneva as a learning
 tool. No portion of this code can be sold or used for commercial purposes
 without the permission of Gribnif Software. Modified versions of the Task
 Manager may not be distributed.

*/

#include "new_aes.h"
#include "vdi.h"
#include "xwind.h"              /* Geneva extensions */
#include "tos.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "taskman.h"            /* resource file constants */

/* version in GENEVA.CNF */
#define TASKMAN_VER "1.04"

/* window type for the main window */
#define WIN_TYPE SMALLER|FULLER|NAME|MOVER|SIZER|CLOSER|UPARROW|DNARROW|VSLIDE

/* hold info for up to 64 applications */
#define LISTAPPS 64

/* definitions for seeing if Geneva is present, by way of JARxxx */
#define CJar_cookie     0x434A6172L     /* "CJar" */
#define _VDO_COOKIE     0x5F56444FL     /* _VDO */
#define FSMC            0x46534D43L     /* FSMC FSM/Speedo cookie */
#define _SPD            0x5F535044L     /* _SPD cookie value */
#define CJar_xbios      0x434A          /* "CJ" */
#define CJar_OK         0x6172          /* "ar" */
#define CJar(mode,cookie,value)         (int)xbios(CJar_xbios,mode,cookie,value)

/* window object tree */
OBJECT applist[LISTAPPS+1] = { -1, -1, -1, G_BOX, TOUCHEXIT, 0, 0x1070L };
/* new item in the Tasks list */
OBJECT newtask[] = { -1, -1, -1, G_STRING, SELECTABLE|TOUCHEXIT|RBUTTON, X_MAGIC,
    0L, 0, 0, 0x060A, 1 };
/* window dimensions, inside and outside */
GRECT winner, max, wsize = { 50, 60, 250, 100 };

struct App
{
  char name[12];
  int num, asleep;
} app[LISTAPPS];

typedef struct
{
  int state, color[2];
} GADGET;       /* gadget colors and state */

int apid,       /* my application ID */
    apps,       /* number of apps in use */
    main_hand,  /* window handle of main window */
    char_w,     /* width of a char in G_STRING */
    char_h,     /* height of a char in G_STRING */
    tasknum,    /* index of currently selected task (0=none) */
    flagmode,   /* 0 for global flag editing */
    flagnum,    /* index or apid of flag being edited */
    fknum,      /* number (0-4) of flag key being edited */
    dattnum,    /* dialog attribute being edited */
    color_mode, /* 0 = 1-plane, 1 = 2-plane, 2 = 4-plane, 3 = 8+ plane */
    falc_vid,   /* Falcon video word */
    falc_vid0,  /* initial value */
    ST_rez;     /* ST-compatible Falcon video rez */

char **new_flag,/* points to the string "--New--" within the RSC file */
     **sys_font,/* points to the string "System" within the RSC file */
     iconified, /* true if the window is iconified */
     was_new,   /* != 0 if the user used the New button in flag editor */
     wg_top=1,  /* showing gadget colors of topped window */
     path[120], /* path in item selector */
     flag_path[120];	/* path in Flags item selector */

GADGET *gad_tmp;/* temporary memory for gadget attributes */

SETTINGS set0,  /* Geneva settings when dialog got opened (for Cancel) */
         set;   /* current settings */

APPFLAGS apf0,  /* flags when dialog got opened */
         apf;   /* current flags */

OBJECT *menu,           /* points to the main window menu tree */
       *colors,         /* points to the colors popup menu */
       *patterns,       /* points to the patterns popup menu */
       *wfontpop,       /* points to the fonts popup menu */
       *stylepop,       /* points to the menu separator styles popup */
       *kpop,           /* points to the key types popup */
       *gad_tree,       /* allocated window gadgets tree */
       *icon;           /* points to the iconify icon */

TEDINFO gad_mover =     /* gadget colors mover */
          { 0L, 0L, 0L, IBM, 0, TE_CNTR, 0, 0, 1 },
        gad_info =      /* gadget colors info bar */
          { 0L, 0L, 0L, IBM, 0, TE_LEFT, 0, 0, 1 },
        gad_menu =      /* gadget colors menu bar */
          { 0L, 0L, 0L, IBM, 0, TE_LEFT, 0, 0, 1 };

#define MENU_START  -5
#define APP_SWITCH  -4
#define APP_SLEEP   -3
#define ASCII_TABLE -2
#define REDRAW_ALL  -1
int xkey=MENU_START;		/* index of key being edited in the Keyboard dialog */

int vdi_hand,           /* VDI device handle */
    work_in[] = { 1, 7, 1, 1, 1, 1, 1, 1, 1, 1, 2 },
    work_out[57],       /* returns from v_opnvwk() */
    has_gdos,           /* indicates presence of GDOS */
    fonts,              /* number of fonts available */
    font_wid,           /* width of one char in the current font */
    font_ht,            /* height of one char in the current font */
    font_num,           /* index of the current font */
    wg_num=WGCLOSE;     /* index of current window gadget */

char font_scalable;     /* is the current font scalable? */

XFONTINFO fontinfo,     /* info about the font used by Geneva */
          fontinfo0;    /* initial state of fontinfo */

typedef struct
{
  int id;
  char name[34],
       scale;           /* is the font a scalable Speedo/FSM font? */
} FONT_DESC;

FONT_DESC *fontlist;

/* some function prototypes */
void close_vdi(void);
int d_exit( OBJECT *o, int num );
int d_init( OBJECT *o );
void d_touch( OBJECT *o, int num );
int ff_init( OBJECT *o );
int ff_exit( OBJECT *o, int num );
int fk_init( OBJECT *o );
void fk_touch( OBJECT *o, int num );
int fo_exit( OBJECT *o, int num );
int fo_init( OBJECT *o );
void fo_touch( OBJECT *o, int num );
int fv_exit( OBJECT *o, int num );
int fv_init( OBJECT *o );
void fv_touch( OBJECT *o, int num );
int k_exit( OBJECT *o, int num );
int k_init( OBJECT *o );
void k_touch( OBJECT *o, int num );
void k_mode( OBJECT *o, int fnum, int mode, int draw );
void k_set( OBJECT *o, int is_main, int draw );
int mo_exit( OBJECT *o, int num );
int mo_init( OBJECT *o );
void mo_touch( OBJECT *o, int num );
int vo_exit( OBJECT *o, int num );
int vo_init( OBJECT *o );
int wo_exit( OBJECT *o, int num );
int wo_init( OBJECT *o );
void wo_touch( OBJECT *o, int num );
int wg_exit( OBJECT *o, int num );
int wg_init( OBJECT *o );
void wg_touch( OBJECT *o, int num );
void set_if( OBJECT *tree, int num, int true );
void start_form( int fnum, int tnum, int type, int xtype );
int drag_wind( int hand );

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

FORM form[] = { { 0, 0L, k_init, k_touch, k_exit },     /* Keyboard */
                { 0, 0L, mo_init, mo_touch, mo_exit },  /* Misc Options */
                { 0, 0L, d_init, d_touch, d_exit },     /* Dialog Options */
                { 0, 0L, wo_init, wo_touch, wo_exit },  /* Window Options */
                { 0, 0L, vo_init, 0L, vo_exit },        /* Video */
                { 0, 0L, fo_init, fo_touch, fo_exit },  /* Flags */
                { 0, 0L, fk_init, fk_touch, k_exit },   /* Flag Keys */
                { 0, 0L, fv_init, fv_touch, fv_exit },  /* Falcon Video */
                { 0, 0L, ff_init, 0L, ff_exit },        /* Find flag */
                { 0, 0L, wg_init, wg_touch, wg_exit },  /* Gadget colors */
                { 0, 0L, 0L } };                        /* end of list */

/* Put up an alert, taking a string from the resource file */
int alert( int num )
{
  char **ptr;

  rsrc_gaddr( 15, num, &ptr );
  return form_alert( 1, *ptr );
}

/* Put up an alert, using a format string */
int spf_alert( char *s, char *s2 )
{
  char buf[200];

  x_sprintf( buf, s, s2 );
  return( form_alert( 1, buf ) );
}

/* Close and delete a window */
void close_wind( int *hand )
{
  if( *hand>0 )
  {
    /* if this is the Window options dialog, close the VDI workstation */
    if( *hand==form[3].handle ) close_vdi();
    wind_close(*hand);
    wind_delete(*hand);
    *hand=0;
  }
}

/* Get Geneva's current settings */
void get_set(void)
{
  x_settings( 0, sizeof(SETTINGS), &set0 );     /* get initial values */
  x_settings( 0, sizeof(SETTINGS), &set );      /* get temporary values */
}

void do_help( char *topic )
{
  if( !x_help( topic, "TASKMAN.HLP", 0 ) ) alert( NOHELP );
}

/* de/select an object */
void set_if( OBJECT *tree, int num, int truth )
{
  if( truth ) tree[num].ob_state |= SELECTED;
  else tree[num].ob_state &= ~SELECTED;
}

/* hide or show an object */
int hide_if(OBJECT *o, int num, int truth )
{
  if( !truth ) o[num].ob_flags |= HIDETREE;
  else o[num].ob_flags &= ~HIDETREE;
}

/* disable/enable an object */
void enab_if( OBJECT *o, int num, int truth )
{
  if( truth ) o[num].ob_state &= ~DISABLED;
  else o[num].ob_state |= DISABLED;
}

/* set an object to be editable or not editable */
void edit_if( OBJECT *o, int num, int truth )
{
  if( !truth ) o[num].ob_flags &= ~EDITABLE;
  else o[num].ob_flags |= EDITABLE;
}

/* display a popup menu */
unsigned int do_popup( OBJECT *o, int obj, OBJECT *pop, unsigned int val )
{
  MENU m, out;
  int x, y;

  m.mn_tree = pop;
  m.mn_menu = 0;
  m.mn_item = val+1;
  m.mn_scroll = 1;
  /* If this is the patterns or colors popup, then display a checkmark
     at the right value by setting it to a BOXCHAR */
  if( (char)pop[1].ob_type==G_BOX || (char)pop[1].ob_type==G_BOXCHAR )
    for( x=1; x<=pop[0].ob_tail; x++ )
      pop[x].ob_type = x==val+1 ? G_BOXCHAR : G_BOX;
  objc_offset( o, obj, &x, &y );
  if( menu_popup( &m, x, y, &out ) ) return out.mn_item-1;
  return val;
}

/********************* Misc. Options Dialog routines *******************/

/* initialize Misc Options dialog */
int mo_init( OBJECT *o )
{
  get_set();
  set_if( o, MOPULL, set.flags.s.pulldown );
  set_if( o, MOINS, set.flags.s.insert_mode );
  set_if( o, MOFOLL, set.flags.s.alerts_under_mouse );
  set_if( o, MOCOL, set.flags.s.fsel_1col );
  set_if( o, MOLONG, set.flags.s.long_titles );
  set_if( o, MOGROW, set.flags.s.grow_shrink );
  set_if( o, MOTOP, set.flags.s.tear_aways_topped );
  set_if( o, MOUPDATE, set.flags.s.auto_update_shell );
  set_if( o, MOALERT, set.flags.s.alert_mode_change );
  set_if( o, MOVIDEO, set.flags.s.ignore_video_mode );
  set_if( o, MOMOUSE, set.flags.s.mouse_on_off );
  set_if( o, MOCOLOR, set.flags.s.preserve_palette );
  set_if( o, MONWIND, set.flags.s.no_alt_modal_equiv^1 );
  set_if( o, MOWIND, set.flags.s.no_alt_modeless_eq^1 );
  set_if( o, MOTOPALL, set.flags.s.top_all_at_once );
  x_sprintf( o[MOGAD].ob_spec.tedinfo->te_ptext, "%d", set.gadget_pause*20 );
  return 1;
}

/* Misc Options touchexit event */
void mo_touch( OBJECT *o, int num )
{
  num++;                /* object clicked on, not used here */
  /* get all settings from the dialog */
  set.flags.s.pulldown = o[MOPULL].ob_state&SELECTED;
  set.flags.s.insert_mode = o[MOINS].ob_state&SELECTED;
  set.flags.s.alerts_under_mouse = o[MOFOLL].ob_state&SELECTED;
  set.flags.s.fsel_1col = o[MOCOL].ob_state&SELECTED;
  set.flags.s.long_titles = o[MOLONG].ob_state&SELECTED;
  set.flags.s.grow_shrink = o[MOGROW].ob_state&SELECTED;
  set.flags.s.tear_aways_topped = o[MOTOP].ob_state&SELECTED;
  set.flags.s.auto_update_shell = o[MOUPDATE].ob_state&SELECTED;
  set.flags.s.alert_mode_change = o[MOALERT].ob_state&SELECTED;
  set.flags.s.ignore_video_mode = o[MOVIDEO].ob_state&SELECTED;
  set.flags.s.mouse_on_off = o[MOMOUSE].ob_state&SELECTED;
  set.flags.s.preserve_palette = o[MOCOLOR].ob_state&SELECTED;
  set.flags.s.no_alt_modal_equiv = !(o[MONWIND].ob_state&SELECTED);
  set.flags.s.no_alt_modeless_eq = !(o[MOWIND].ob_state&SELECTED);
  set.flags.s.top_all_at_once = o[MOTOPALL].ob_state&SELECTED;
  set.gadget_pause = atoi(o[MOGAD].ob_spec.tedinfo->te_ptext)/20;
  /* set them in Geneva */
  x_settings( 1, sizeof(SETTINGS), &set );
}

/* Misc Options exit event */
int mo_exit( OBJECT *o, int num )
{
  if( num==MOHELP )
  {
    do_help( o[1].ob_spec.free_string );
    return 0;
  }
  else if( num!=MOOK ) x_settings( 1, sizeof(SETTINGS), &set0 );    /* Cancel */
  else mo_touch( o, 0 );        /* get settings and set in Geneva */
  return 1;                     /* close dialog */
}

/************************ Flags Dialog routines ***********************/

char *appname( int num )
{
  return !(app[num].asleep&2) ? app[num].name+3 : app[num].name+2;
}

/* get current flags from dialog */
void fo_stat( OBJECT *o )
{
  char NotFirstOrTemp;

  NotFirstOrTemp = !flagmode && flagnum;
  x_appl_flags( flagmode, flagnum, &apf0 );
  apf = apf0;
  x_form_filename( o, FONAME, 0, flagmode ? appname(tasknum-1) : apf.name );
  edit_if( o, FONAME, NotFirstOrTemp );
  hide_if( o, FONAME, flagmode || flagnum );
  hide_if( o, FOSEL, NotFirstOrTemp );
  hide_if( o, FOPREV, NotFirstOrTemp );
  o[FODESC].ob_spec.tedinfo->te_ptext = apf.desc;
  edit_if( o, FODESC, NotFirstOrTemp );
  hide_if( o, FONEXT, !flagmode && x_appl_flags( 0, flagnum+1, 0L ) );
  hide_if( o, FOFIND, !flagmode );
  set_if( o, FOSPEC, apf.flags.s.special_types );
  set_if( o, FOLIMIT, apf.flags.s.limit_memory );
  set_if( o, FOCLEAR, apf.flags.s.clear_memory );
  set_if( o, FOWMAX, apf.flags.s.maximize_wind );
  set_if( o, FOMAXRED, apf.flags.s.optim_redraws );
  set_if( o, FOKEEP, apf.flags.s.keep_deskmenu );
  set_if( o, FOAES40, apf.flags.s.AES40_msgs );
  set_if( o, FOREDRAW, apf.flags.s.exit_redraw );
  x_sprintf( o[FOLNUM].ob_spec.tedinfo->te_ptext, "%d",
      apf.flags.s.mem_limit );
  set_if( o, FOMULTI, apf.flags.s.multitask );
  hide_if( o, FOMULTI, !flagmode );
  set_if( o, FOHAND, apf.flags.s.limit_handles );
  hide_if( o, FOHAND, !flagmode );
  set_if( o, FOLEFT, apf.flags.s.off_left );
  set_if( o, FOROUND, apf.flags.s.round_buttons );
  set_if( o, FOAUTO, apf.flags.s.kbd_equivs );
  set_if( o, FOUNDRAW, apf.flags.s.undo_equivs );
  hide_if( o, FODEL, !flagmode && flagnum );
  hide_if( o, FONEW, !flagmode );
  if( form[5].handle>0 ) wind_set( form[5].handle, X_WF_DIALOG, form[5].tree );
  if( form[6].handle>0 ) k_set( form[6].tree, 0, 1 );
}

/* initialize Flags dialog */
int fo_init( OBJECT *o )
{
  char **ptr;

  /* set the dialog's title, depending on which mode the user requested */
  rsrc_gaddr( 15, flagmode ? FTMPSTR : FPERSTR, &ptr );
  o[1].ob_spec.free_string = *ptr;
  fo_stat(o);
  was_new = 0;
  return 1;
}

/* alter the flags settings according to what the user has changed */
int fo_set( OBJECT *o, int warn )
{
  x_form_filename( o, FONAME, 1, apf.name );    /* get file name from dialog */
  /* was it left as "--New--" ? */
  if( flagmode || !flagnum || apf.name[0] && strncmp(apf.name,*new_flag,12) )
  {
    apf.flags.s.special_types = o[FOSPEC].ob_state&SELECTED;
    apf.flags.s.limit_memory = o[FOLIMIT].ob_state&SELECTED;
    apf.flags.s.clear_memory = o[FOCLEAR].ob_state&SELECTED;
    apf.flags.s.maximize_wind = o[FOWMAX].ob_state&SELECTED;
    apf.flags.s.optim_redraws = o[FOMAXRED].ob_state&SELECTED;
    apf.flags.s.exit_redraw = o[FOREDRAW].ob_state&SELECTED;
    apf.flags.s.keep_deskmenu = o[FOKEEP].ob_state&SELECTED;
    apf.flags.s.AES40_msgs = o[FOAES40].ob_state&SELECTED;
    apf.flags.s.mem_limit = atoi( o[FOLNUM].ob_spec.tedinfo->te_ptext );
    apf.flags.s.multitask = o[FOMULTI].ob_state&SELECTED;
    apf.flags.s.limit_handles = o[FOHAND].ob_state&SELECTED;
    apf.flags.s.off_left = o[FOLEFT].ob_state&SELECTED;
    apf.flags.s.round_buttons = o[FOROUND].ob_state&SELECTED;
    apf.flags.s.kbd_equivs = o[FOAUTO].ob_state&SELECTED;
    apf.flags.s.undo_equivs = o[FOUNDRAW].ob_state&SELECTED;
    x_appl_flags( flagmode+1, flagnum, &apf );
    return 1;
  }
  else
  {
    if( warn ) alert( ALNONAME );       /* give a warning alert */
    return 0;
  }
}

/* Flags dialog touchexit event */
void fo_touch( OBJECT *o, int num )
{
  char temp2[50], name[13], **str;
  int but;

  switch(num)
  {
    case FOPREV:                /* Prev button */
      if( flagnum && fo_set( o, 1 ) )
      {
        flagnum--;
        fo_stat(o);
        was_new = 0;
      }
      break;
    case FONEXT:                /* Next button */
      /* is there another flag after this one? */
      if( x_appl_flags( flagmode, flagnum+1, 0L ) && fo_set( o, 1 ) )
      {
        flagnum++;
        fo_stat(o);
        was_new = 0;
      }
      break;
    case FOFIND:		/* Find button */
      if( !fo_set( o, 1 ) ) return;
      start_form( 8, FLFIND, NAME|MOVER, 0 );
      break;
    case FOSEL:			/* select name of app */
      if( !flag_path[0] ) strcpy( flag_path, path );
      strcpy( name, apf.name );
      rsrc_gaddr( 15, FOSELSTR, &str );
      x_sprintf( temp2, *str, apf.desc );
      if( x_fsel_input( flag_path, sizeof(flag_path), name, 1, &but, temp2 ) &&
          but && name[0] )   /* ask for name */
      {
        x_form_filename( o, FONAME, 0, name );      /* set file name into dialog */
        x_wdial_draw( form[5].handle, FONAME, 0 );  /* and draw it */
      }
      break;
    default:
      fo_set( o, 0 );  /* all others, just set the new values */
      break;
  }
}

/* Flags dialog exit event */
int fo_exit( OBJECT *o, int num )
{
  switch(num)
  {
    case FOHELP:
      do_help( o[1].ob_spec.free_string );
      return 0;
    case FODEL:         /* delete flag */
      x_appl_flags( 2, flagnum, 0L );
      flagnum--;
      fo_stat(o);
      was_new = 0;
      return 0;
    case FONEW:         /* new flag */
      if( fo_set( o, 1 ) )      /* first, update the current flag */
      {
        x_appl_flags( 0, 0, &apf );     /* get default */
        strcpy( apf.name, *new_flag );  /* set name to "--New--" */
        apf.desc[0] = 0;
        x_appl_flags( 1, -1, &apf );    /* create new */
        while( x_appl_flags( 0, flagnum+1, &apf ) ) flagnum++;  /* find end */
        apf0 = apf;
        fo_stat(o);                     /* show it */
        was_new = 1;
      }
      return 0;
    case FOKEYS:        /* Keys... button */
      if( !fo_set( o, 1 ) ) return 0;
      start_form( 6, FLKEYS, NAME|MOVER, 0 );
      return 0;
    case FOOK:          /* OK button */
      if( !fo_set( o, 1 ) ) return 0;
      break;
    default:            /* Cancel */
      if( !was_new ) x_appl_flags( flagmode+1, flagnum, &apf0 );
      else x_appl_flags( 2, flagnum, 0L );      /* delete it */
  }
  /* exiting, so close the Keys dialog */
  if( form[6].handle>0 ) close_wind(&form[6].handle);
  return 1;
}

/* start the Flags dialog */
void start_flags( int perm )
{
  APPFLAGS f, f2;
  int i;

  if( perm )
  {
    flagmode = 0;
    flagnum = 0;
    if( tasknum>0 )
    {
      /* ask Geneva which flags it is using */
      x_appl_flags( X_APF_GET_ID, app[tasknum-1].num, &f );
      /* and try to find it in the list */
      i=0;
      while( x_appl_flags( X_APF_GET_INDEX, i, &f2 ) )
        if( !strcmp( f2.name, f.name ) )
        {
          flagnum = i;
          break;
        }
        else i++;
    }
  }
  else
  {
    flagmode = X_APF_GET_ID;
    flagnum = app[tasknum-1].num;
  }
  start_form( 5, FLAGOPTS, NAME|MOVER, 0 );
}

/*********************** Flags->Find routines *************************/
/* initialize Flags->Find dialog */
int ff_init( OBJECT *o )
{
  int h, flags;

  if( (h=form[5].handle)>0 )
  {
    /* Make sure the user can't select any options in the Flags dialog */
    wind_get( h, X_WF_DIALFLGS, &flags );
    wind_set( h, X_WF_DIALFLGS, flags & ~X_WD_ACTIVE );
  }
  o[FLFNAME].ob_spec.tedinfo->te_ptext[0] = 0;
  return 1;
}

/* Pattern matching routine. Returns 1 if pat matches str */
int match( char *str, char *pat )
{
  char s, p, per=0;

  for(;;)
  {
    if( (p = *pat++) == '\0' )
      if( *str ) return 0;
      else return 1;
    s = *str++;
    if( p == '*' )
    {
      if( !s ) return 1;
      str--;
      do
      {
        if( *str == '.' ) per=1;
        if( match( str, pat ) ) return 1;
      }
      while( *str++ );
      if( *pat++ != '.' ) return 0;
      if( *pat == '*' )
        if( !*(pat+1) ) return 1;
        else return 0;
      else if( *pat || per ) return 0;
      return(1);
    }
    else if( p == '?' )
    {
      if( !s ) return 0;
    }
    else if( p != s ) return 0;
  }
}

/* Flags->Find exit button */
int ff_exit( OBJECT *o, int num )
{
  int h, flags, i;
  char temp[13];
  APPFLAGS apfl;

  if( (h=form[5].handle)>0 )
  {
    /* Allow the user to select options in the Flags dialog again */
    wind_get( h, X_WF_DIALFLGS, &flags );
    wind_set( h, X_WF_DIALFLGS, flags | X_WD_ACTIVE );
  }
  if( num==FLFOK )
  {
    i = 0;
    /* Get find string */
    x_form_filename( o, FLFNAME, 1, temp );
    /* Search for the first match, excluding default. This is the same
       order Geneva uses */
    while( x_appl_flags( 0, ++i, &apfl ) )
      if( match( temp, apfl.name ) )
      {
        flagnum = i;
        fo_stat(form[5].tree);
        was_new = 0;
        return 1;
      }
    /* No match, go to default */
    flagnum = 0;
    fo_stat(form[5].tree);
    was_new = 0;
  }
  return 1;
}

/********************* Flags->Keys Dialog routines ********************/

/* Note: the Flags->Keys and the main Keyboard dialog have a similar
   structure, so many routines are actually used for both dialogs */

/* These are not defined in xwind.h, because they are not part of the
   SETTINGS.wind_keys[] array */
#define XS_CYCLE2   13
#define XS_ICONIFY  14
#define XS_ALLICON  15
#define XS_PROCMAN  16

/* initialize Flags->Keys dialog */
int fk_init( OBJECT *o )
{
  int i;

  /* set starting key number */
  if( !flagmode && !flagnum ) fknum = 1;
  else fknum = 0;
  for( i=0; i<4; i++ )
    set_if( o, i+FLOPEN, i==fknum );
  k_mode( o, 6, 0, 0 );
  k_set( o, 0, 0 );
  return 1;
}

/* go to a new key type */
void new_key( int num, int convert )
{
  static int keyx[] = { XS_CLOSE, 99, XS_ICONIFY, XS_CYCLE, XS_FULL,
      XS_LFINFO, 99, XS_RTINFO };

  if( convert )         /* convert from a window object index to a key type */
  {
    if( --num > sizeof(keyx)/sizeof(int) ) return;
    num = keyx[num];
  }
  if( num!=99 && num!=xkey )    /* is it a new key type? */
  {
    xkey = num;
    k_set( form[0].tree, 1, 1 );
  }
}

/* key the key shift status */
unsigned char k_shift( OBJECT *o, int base )
{
  return ((o[KOALT+base-KOBOX].ob_state&SELECTED)<<3) |
         ((o[KOCNTRL+base-KOBOX].ob_state&SELECTED)<<2) |
         ((o[KOSHIFT+base-KOBOX].ob_state&SELECTED) ? 3 : 0);
}

/* process a Flags->Keys touchexit event */
void fk_touch( OBJECT *o, int num )
{
  if( num<FLOPEN || num>FLRES3 )      /* not a new key type */
  {
    (&apf.open_key+fknum)->shift = k_shift( o, FLBOX );  /* set the shift state */
    fo_set( form[5].tree, 1 );
  }
  else if( fknum != num-FLOPEN && !(o[num].ob_state&DISABLED) )
  {
    fknum = num-FLOPEN;                 /* go to a new key type */
    k_set( o, 0, 1 );
  }
}

/********************** Keyboard Dialog routines *********************/

/* set the default slider positions and turn off event reporting */
OBJECT *wind_setup( int hand, int get_tree )
{
  WIND_TREE wt;

  wt.tree = 0L;
  wind_set( hand, WF_HSLIDE, 500 );
  wind_set( hand, WF_HSLSIZE, 0 );
  wind_set( hand, WF_VSLIDE, 500 );
  wind_set( hand, WF_VSLSIZE, 0 );
  wt.handle = hand;
  x_wind_tree( X_WT_GETCNT, &wt );            /* get window flags */
  /* don't react to sliders or window gadgets */
  wt.flag &= ~(X_WTFL_SLIDERS|X_WTFL_CLICKS);
  if( get_tree )
  {
    /* allocate some memory for the entire tree */
    x_malloc( (void **)&wt.tree, wt.count*sizeof(OBJECT) );
    if( !wt.tree ) return 0L;
    wt.flag &= ~X_WTFL_RESIZE;                /* don't change sizes or colors */
    x_wind_tree( X_WT_READ, &wt );            /* copy it in */
  }
  else wt.count = -1;                         /* don't change tree */
  x_wind_tree( X_WT_SET, &wt );               /* set the tree and/or count */
  return wt.tree;
}

/* initialize main Keyboard dialog */
int k_init( OBJECT *o )
{
  int hand = form[0].handle;

  k_mode( o, 0, 0, 0 );
  get_set();                                  /* get settings */
  k_set( o, 1, 0 );                           /* set in dialog */
  wind_setup( hand, 0 );
  return 1;
}

/* get the address of a keycode in the SETTINGS structure */
KEYCODE *get_keycode( int num )
{
  return num<XS_CYCLE2 ? &set.wind_keys[xkey] :
      &set.cycle_in_app+num-XS_CYCLE2;
}

int kpop_ind(void)
{
  if( xkey==XS_PROCMAN ) return KOPROCESS;
  if( xkey>=0 ) return KOGAD;
  return xkey-MENU_START+KOMENU;
}

/* process a main Keyboard touchexit event */
void k_touch( OBJECT *o, int num )
{
  char sh;
  int i;
  static char tbl[] = { MENU_START, APP_SWITCH, APP_SLEEP, ASCII_TABLE,
      REDRAW_ALL, XS_PROCMAN };

  if( num>=KOCNTRL && num<=KOALT )      /* not a new key type */
  {
    get_keycode(xkey)->shift = k_shift( o, KOBOX );   /* set the shift state */
    x_settings( 1, sizeof(SETTINGS), &set );
  }
  else if( num==KOONEXT )
  {
    if( (i=kpop_ind()+1) > KOPROCESS ) i = KOMENU;
    new_key( tbl[i-KOMENU], 0 );
  }
  else if( num==KOOTHER )
    if( (num=do_popup( o, KOOTHER, kpop, i=kpop_ind()-1 )) != i )
        new_key( tbl[num+1-KOMENU], 0 );
}

/* process a main Keyboard / Flags->Keys exit event */
int k_exit( OBJECT *o, int num )
{
  int sh=0, key=0, dum, is_main;
  KEYCODE *k;

  is_main = o == form[0].tree;  /* is it the main one, or the Flags one? */
  if( !is_main ) num -= FLBOX-KOBOX;
  switch( num )
  {
    case KOHELP:
      do_help( o[1].ob_spec.free_string );
      return 0;
    case KOQUIT:
      return 1;
    case KOREAD:        /* read a keypress from user */
      k_mode( o, is_main?0:6, 1, 1 );   /* display "Press key" message */
      wind_update( BEG_UPDATE );
      evnt_multi( MU_KEYBD,  0,0,0,  0,0,0,0,0,
          0,0,0,0,0,  &dum,  0,0,  &dum, &dum, &dum, &sh, &key, &dum );
      wind_update( END_UPDATE );
      k_mode( o, is_main?0:6, 0, 1 );   /* undraw message */
      if( sh&3 ) sh |= 3;               /* one shift key->both shift keys */
      /* fall through to process key */
    case KOCLEAR:       /* key=sh=0 already for clear */
      k = is_main ? get_keycode(xkey) : &apf.open_key+fknum;
      k->shift = sh&0xf;
      k->scan = key>>8;
      k->ascii = 0;
      k_set( o, is_main, 1 );
      if( is_main ) x_settings( 1, sizeof(SETTINGS), &set );
      else fo_set( form[5].tree, 1 );
      return 0;
    default:            /* cancel */
      if( is_main ) x_settings( 1, sizeof(SETTINGS), &set0 );
      return 0;
  }
}

/* show/hide the "Press the key to assign..." message */
void k_mode( OBJECT *o, int fnum, int mode, int draw )
{
  hide_if( o, KOCLICK, !mode );
  hide_if( o, KOPRESS, mode );
  if( draw ) x_wdial_draw( form[fnum].handle, KOBACK, 8 );
}

/* translate a key code to a description and set it in the dialog */
void k_set( OBJECT *o, int is_main, int draw )
{
  int i;
  unsigned char *ptr, c, c2;
  char **p;
  KEYCODE *key;
  OBJECT *box;
  static unsigned char
      keycode[] = { 0, 1, 0xf, 0xe, 0x53, 0x52, 0x62, 0x61, 0x47, 0x48, 0x50,
          0x4d, 0x4b, 0x72, 0x60, 0x39 },
      keynam[][6] = { "???", "Esc", "Tab", "Bksp", "Del", "Ins", "Help", "Undo",
          "Home", "", "", "", "", "Enter", "ISO", "Space" }, fmt[]="F%d",
          kpfmt[]="kp %c";

  key = is_main ? get_keycode(xkey) : &apf.open_key+fknum;
  box = is_main ? &o[KOBOX] : &o[FLBOX];
  set_if( box, KOSHIFT-KOBOX, key->shift&3 );
  set_if( box, KOCNTRL-KOBOX, key->shift&4 );
  set_if( box, KOALT-KOBOX, key->shift&8 );
  *(ptr = box[KOKEY-KOBOX].ob_spec.free_string) = '\0'; /* blank by default */
  if( (c = key->ascii) != 0 )   /* ASCII value is used instead of scan code */
    if( c == ' ' ) strcpy( ptr, "Space" );
    else
    {
      *ptr++ = c;
      *ptr = 0;
    }
  else
  {
    c = key->scan;
    if( c==0x74 ) c=0x4d;                       /* ^right -> right */
    else if( c==0x73 ) c=0x4b;                  /* ^left  -> left */
    else if( c==0x77 ) c=0x47;                  /* ^home  -> home */
    else if( c>=0x78 && c<=0x83 ) c -= 0x76;    /* ^F1-10 -> F1-10 */
    for( i=0; i<sizeof(keycode); i++ )
      if( keycode[i] == c ) strcpy( ptr, keynam[i] );
    if( c >= 0x3b && c <= 0x44 ) x_sprintf( ptr, fmt, c-0x3a ); /* F1-10 */
    else if( c >= 0x54 && c <= 0x5d )
        x_sprintf( ptr, fmt, c-0x53 );                    /* shift F1-10 */
    if( !*ptr )
    {
      c2 = *(Keytbl( (void *)-1L, (void *)-1L, (void *)-1L )->unshift + c);
      if( c >= 0x63 && c <= 0x72 || c == 0x4a || c == 0x4e )
          x_sprintf( ptr, kpfmt, c2 );                    /* keypad key */
      else
      {
        *ptr++ = c2;                                      /* unshifted char */
        *ptr = '\0';
      }
    }
  }
  /* get a description for the key type */
  rsrc_gaddr( 15, is_main ? S0+xkey : FLK0+fknum, &p );
  box->ob_spec.free_string = *p;
  if( !is_main )
  {
    /* set either the flag's name or description in the dialog */
    o[FLOPEN-1].ob_spec.free_string = flagmode ? appname(tasknum-1) :
        apf.desc[0] ? apf.desc : apf.name;
    /* Activate is always disabled for the default flags */
    enab_if( o, FLOPEN, flagmode || flagnum );
  }
  else
  {
    for( i=KOGAD; i<=KOPROCESS; i++ )
      if( i==kpop_ind() ) kpop[i].ob_state |= CHECKED;
      else kpop[i].ob_state &= ~CHECKED;
    p = &o[KOOTHER].ob_spec.free_string;
    ptr = kpop[kpop_ind()].ob_spec.free_string+2;
    if( *p != ptr )
    {
      *p = ptr;
      if( draw ) x_wdial_draw( form[0].handle, KOOTHER, 0 );
    }
  }
  if( draw )
    if( is_main ) x_wdial_draw( form[0].handle, KOBOX, 8 );
    else x_wdial_draw( form[6].handle, FLBOX, 8 );
}

/******************* Falcon Video Dialog routines ******************/

/* get the equivalent ST resolution for a Falcon mode, if possible */
void get_ST_rez(void)
{
  int j;

  j = falc_vid & (VERTFLAG|STMODES|COL80|NUMCOLS);
  ST_rez = -1;
  if( j==(COL40|BPS4|STMODES|VERTFLAG) ) ST_rez = 0;      /* ST low */
  else if( j==(COL80|BPS2|STMODES|VERTFLAG) ) ST_rez = 1; /* ST med */
  else if( j==(COL80|BPS1|STMODES) ) ST_rez = 2;          /* ST high */
}

/* update the Falcon Video dialog */
void fv_stat( OBJECT *o, int draw )
{
  int i, j;

  /* set 40/80 columns buttons */
  set_if( o, FALTC40, (j=falc_vid&COL80)==0 );
  set_if( o, FALTC80, j );
  /* set ST-compatible rez */
  get_ST_rez();
  for( i=0; i<3; i++ )
    set_if( o, FALSTL+i, i==ST_rez );
  /* set # of bitplanes/colors */
  for( i=0, j=falc_vid&NUMCOLS; i<5; i++ )
    set_if( o, FALC2+i, i==j );
  /* can't have 2-color, 40-columns */
  enab_if( o, FALTC40, (falc_vid&NUMCOLS) != BPS1 );
  enab_if( o, FALC2, (falc_vid&COL80) != COL40 );
  /* set interlace/double line */
  set_if( o, FALDBL, falc_vid&VERTFLAG );
  if( falc_vid & VGA_FALCON )           /* it's a VGA monitor */
  {
    /* can't have 80-columns, True Color */
    enab_if( o, FALTC80, (falc_vid&NUMCOLS) != BPS16 );
    enab_if( o, FALC2+4, (falc_vid&COL80) != COL80 );
  }
  if( draw )
  {
    x_wdial_draw( form[7].handle, FALTC40-1, 8 );
    x_wdial_draw( form[7].handle, FALSTL-1, 8 );
    x_wdial_draw( form[7].handle, FALC2-1, 8 );
    x_wdial_draw( form[7].handle, FALDBL, 8 );
  }
}

/* initialize Falcon video dialog */
int fv_init( OBJECT *o )
{
  char **p;

  /* get current values */
  falc_vid = falc_vid0 = Vsetmode(-1);
  /* set the text of a button to either "Interlace" or "Double line" */
  rsrc_gaddr( 15, mon_type()!=2 ? INTRLACE : DOUBLE, &p );
  o[FALDBL].ob_spec.free_string = *p;
  /* set dialog */
  fv_stat( o, 0 );
  return 1;
}

/* process a Falcon video touchexit event */
void fv_touch( OBJECT *o, int num )
{
  int old_ST, old_vid, i;

  if( !(o[num].ob_state&DISABLED) )
  {
    /* save current values */
    old_ST = ST_rez;
    old_vid = falc_vid;
    /* only modify some of the bits; set them to 0 by default */
    falc_vid &= ~(VERTFLAG|STMODES|COL80|NUMCOLS);
    if( num==FALSTL ) falc_vid |= (STMODES|COL40|BPS4|VERTFLAG);         /* ST low */
    else if( num==FALSTM ) falc_vid |= (STMODES|COL80|BPS2|VERTFLAG);    /* ST med */
    else if( num==FALSTH ) falc_vid |= (STMODES|COL80|BPS1);    /* ST high */
    else
    {
      if( o[FALTC80].ob_state&SELECTED ) falc_vid |= COL80;     /* 80 cols */
      if( o[FALDBL].ob_state&SELECTED ) falc_vid |= VERTFLAG;   /* interlace */
      for( i=1; i<5; i++ )
        if( o[FALC2+i].ob_state&SELECTED ) falc_vid |= i;       /* planes */
    }
    get_ST_rez();
    /* only redraw the dialog if something important has changed */
    if( ((old_vid&NUMCOLS)==BPS1) != ((falc_vid&NUMCOLS)==BPS1) ||
        (old_vid&COL80) != (falc_vid&COL80) || falc_vid&VGA_FALCON &&
        ((old_vid&NUMCOLS)==BPS16) != ((falc_vid&NUMCOLS)==BPS16) ||
        ST_rez != old_ST ) fv_stat( o, 1 );
  }
}

/* Falcon video dialog exit button */
int fv_exit( OBJECT *o, int num )
{
  int rez, i;

  if( num==FALHELP )
  {
    do_help( o[1].ob_spec.free_string );
    return 0;
  }
  else if( num==FALOK && falc_vid != falc_vid0 )
     shel_write( SHW_NEWREZ, falc_vid, 1, 0L, 0L );  /* tell Geneva to change */
  return 1;
}

/******************* ST/TT Video Dialog routines *******************/

/* translate a rez in the Video dialog to a Setscreen() rez number */
char reztbl[] = { 0, 1, 2, 7, 4, 6 };

/* Video dialog exit button */
int vo_exit( OBJECT *o, int num )
{
  int rez, i;

  if( num==VOHELP )
  {
    do_help( o[1].ob_spec.free_string );
    return 0;
  }
  else if( num==VOOK )
    for( i=VOSTLOW; i<=VOSTLOW+5; i++ )
      if( o[i].ob_state & SELECTED )
        if( (rez = reztbl[i-VOSTLOW]) != Getrez() )
          /* tell Geneva to change */
          shel_write( SHW_NEWREZ, rez+2, 0, 0L, 0L );
  return 1;
}

/* initialize Video dialog */
int vo_init( OBJECT *o )
{
  int rez, i;
  long vid;
  char TT_vid;

  rez = Getrez();               /* get current rez */
  /* Is this a TT? Can all ST resolutions be chosen? */
  TT_vid = CJar( 0, _VDO_COOKIE, &vid ) == CJar_OK && (int)(vid>>16)==2;
  for( i=VOSTLOW; i<=VOSTLOW+5; i++ )
    if( rez==8 && i!=VOSTLOW+5 || TT_vid && i==VOSTLOW+5 ||
        !TT_vid && rez==2 && i!=VOSTLOW+2 ||
        !TT_vid && rez!=2 && i==VOSTLOW+2 ||
        !TT_vid && i>VOSTLOW+2 )
    {
      o[i].ob_state |= DISABLED;
      o[i].ob_state &= ~SELECTED;
    }
    else
    {
      o[i].ob_state &= ~DISABLED;
      if( reztbl[i-VOSTLOW]==rez ) o[i].ob_state |= SELECTED;
      else o[i].ob_state &= ~SELECTED;
    }
  return 1;
}

/********************* Dialog Options routines *********************/

/* return an object index for the Dialog Options sample buttons, along
   with its color preferences */
int get_prefer( OB_PREFER **op )
{
  int ret;

  switch( dattnum )
  {
    case 0:
      *op = set.color_root;
      ret = DOSBOX;
      break;
    case 1:
      *op = set.color_3D;
      ret = DOS3D;
      break;
    case 2:
      *op = set.color_exit;
      ret = DOSEXIT;
      break;
    case 3:
      *op = set.color_other;
      ret = DOSOTHER;
      break;
  }
  *op += color_mode;
  return ret;
}

/* set one bit of the state of a sample object, and also set the
   state of the appropriate checkbox */
void d_obj( int truth, unsigned int *state, int bit, OBJECT *o, int obj )
{
  if( truth ) *state |= bit;
  set_if( o, obj, truth );
}

/* show a color/fill pattern sample */
void d_sample( OBJECT *o, int num, int fill, int val )
{
  o[num].ob_spec.index = (o[num].ob_spec.index&0xFFFF0000L) |
      (unsigned int)(fill ? patterns[val+PFIL0].ob_spec.index :
      colors[val+PCOL0].ob_spec.index);
}

/* hide text and fill boxes based on Atari 3D state */
void d_hide( OBJECT *o, OB_PREFER *op, int draw )
{
  int on, show;

  on = (o[DOFBOX+1].ob_flags&HIDETREE) == 0;
  show = !op->s.atari_3D;
  if( show != on )
  {
    hide_if( o, DTBOX+1, show );
    hide_if( o, DOFBOX+1, show );
    if( draw )
    {
      x_wdial_draw( form[2].handle, DTBOX, 8 );
      if( !(draw&4) ) x_wdial_draw( form[2].handle, DOFBOX, 8 );
    }
  }
}

/* update all Dialog Options objects */
void d_stat( OBJECT *o, int draw )
{
  OB_PREFER *op;
  unsigned int obj, *state, *flags;
  OBJECT *ch;

  /* get the currently chosen type of object */
  obj = get_prefer( &op );
  ch = &o[obj];
  /* default state: everything off */
  *(state = &ch->ob_state) = X_MAGIC;
  *(flags = &ch->ob_flags) = TOUCHEXIT;
  /* add each attribute */
  d_obj( op->s.outlined,    state, OUTLINED,     o, DOUTLINE );
  d_obj( op->s.shadowed,    state, SHADOWED,     o, DSHADOW );
  d_obj( op->s.draw_3D,     state, X_DRAW3D,     o, DGNVA3D );
  d_obj( op->s.rounded,     state, X_ROUNDED,    o, DROUND );
  d_obj( op->s.shadow_text, state, X_SHADOWTEXT, o, DTSHAD );
  d_obj( op->s.bold_shadow, flags, X_BOLD,       o, DTBOLD );
  d_obj( op->s.atari_3D,    flags, obj==DOSBOX ? FL3DBAK : FL3DACT,
      o, DATARI3D );
  set_if( o, DOPAQUE, op->s.textmode );
  /* and set the correct sample with the current state */
  if( op->s.atari_3D )
  { /* even though these are G_BOXTEXT objects, emulate G_BUTTONs */
    if( obj==DOSBOX ) ch->ob_spec.index =
        (ch->ob_spec.index&0xFFFF0000L) | 0x1000;
    else
    {
      ch->ob_spec.tedinfo->te_color = 0x1000;
      if( ch->ob_height >= char_h )
      { /* adjust size for expanded border */
        ch->ob_x += 2;
        ch->ob_y += 2;
        ch->ob_width -= 4;
        ch->ob_height -= 4;
      }
    }
  }
  else
  {
    if( obj==DOSBOX ) ch->ob_spec.index =
        (ch->ob_spec.index&0xFFFF0000L) | (unsigned int)op->l;
    else
    {
      ch->ob_spec.tedinfo->te_color = (unsigned int)op->l;
      if( ch->ob_height < char_h )
      { /* it was an Atari 3D, so reset size */
        ch->ob_y -= 2;
        ch->ob_x -= 2;
        ch->ob_width += 4;
        ch->ob_height += 4;
      }
    }
  }
  /* set the popup samples */
  d_sample( o, DOFLEFT+1, 1, op->s.fillpattern );
  d_sample( o, DOPLEFT+1, 0, op->s.interiorcol );
  d_sample( o, DOTLEFT+1, 0, op->s.textcol );
  d_sample( o, DOBLEFT+1, 0, op->s.framecol );
  d_hide( o, op, draw );
  if( draw&1 ) x_wdial_draw( form[2].handle, obj==DOSBOX ? DOSBOX-1 : DOSBOX,
     8 );
  if( draw&2 ) x_wdial_draw( form[2].handle, DOBBOX, 8 );
  if( draw&4 )
  {
    x_wdial_draw( form[2].handle, DOTBOX, 8 );
    x_wdial_draw( form[2].handle, DOFBOX, 8 );
  }
  if( draw&8  ) x_wdial_draw( form[2].handle, DOFLEFT+1, 0 );
  if( draw&16 ) x_wdial_draw( form[2].handle, DOPLEFT+1, 0 );
  if( draw&32 ) x_wdial_draw( form[2].handle, DOTLEFT+1, 0 );
  if( draw&64 ) x_wdial_draw( form[2].handle, DOBLEFT+1, 0 );
}

/* initialize the Dialog Options dialog */
int d_init( OBJECT *o )
{
  int old;

  get_set();
  old = dattnum;
  for( dattnum=0; dattnum<4; dattnum++ )
    if( dattnum!=old ) d_stat( o, 0 );  /* skip the current one */
  dattnum = old;
  d_stat( o, 0 );       /* do the current one last */
  return 1;
}

/* force the object type radio buttons to be updated after the user
   clicks on one of the sample objects */
void touch_it( OBJECT *o, int num )
{
  int dum;

  form_button( o, num, 1, &dum );
  d_touch( o, num );
}

/* process a touchexit object in the Dialog Options */
void d_touch( OBJECT *o, int num )
{
  OB_PREFER *op;
  int update;

  if( num>=DOOUT && num<=DOOTHER )      /* new object type */
  {
    if( (num-=DOOUT) != dattnum )       /* and it really is new */
    {
      dattnum = num;
      update = 4|2;
    }
  }
  else
  {
    update = 1;
    get_prefer( &op );                  /* get current state */
    switch( num )
    {
      case DOSBOX:
      case DOSBOXT:
        touch_it( o, DOOUT );           /* switch to outer box */
        return;
      case DOS3D:
        touch_it( o, DO3DBUT );         /* switch to 3D button */
        return;
      case DOSEXIT:
        touch_it( o, DOEXIT );          /* switch to exit button */
        return;
      case DOSOTHER:
        touch_it( o, DOOTHER );         /* switch to other */
        return;
      case DOUTLINE:                    /* toggle outline */
        op->s.outlined ^= 1;
        break;
      case DSHADOW:                     /* toggle shadow */
        op->s.shadowed ^= 1;
        break;
      case DTSHAD:                      /* toggle text shadow */
        op->s.shadow_text ^= 1;
        break;
      case DTBOLD:                      /* toggle bold shadow */
        op->s.bold_shadow ^= 1;
        break;
      case DATARI3D:                    /* toggle Atari 3D effect */
        if( (op->s.atari_3D ^= 1) != 0 )
        {
          op->s.draw_3D = 0;
          x_wdial_change( form[2].handle, DGNVA3D, o[DGNVA3D].ob_state&~SELECTED );
        }
        break;
      case DGNVA3D:                     /* toggle Geneva 3D effect */
        if( (op->s.draw_3D ^= 1) != 0 )
        {
          op->s.atari_3D = 0;
          x_wdial_change( form[2].handle, DATARI3D, o[DATARI3D].ob_state&~SELECTED );
        }
        break;
      case DROUND:                      /* toggle round */
        op->s.rounded ^= 1;
        break;
      case DOPAQUE:                     /* toggle text transparent/opaque */
        op->s.textmode ^= 1;
        break;
      case DOFLEFT:                     /* previous fill pattern */
        op->s.fillpattern--;
        update |= 8;
        break;
      case DOFLEFT+2:                   /* next fill pattern */
        op->s.fillpattern++;
        update |= 8;
        break;
      case DOFLEFT+1:                   /* fill pattern popup */
        op->s.fillpattern = do_popup( o, DOFLEFT+1, patterns, op->s.fillpattern );
        update |= 8;
        break;
      case DOPLEFT:                     /* previous interior color */
        op->s.interiorcol--;
        update |= 16;
        break;
      case DOPLEFT+2:                   /* next interior color */
        op->s.interiorcol++;
        update |= 16;
        break;
      case DOPLEFT+1:                   /* interior color popup */
        op->s.interiorcol = do_popup( o, DOPLEFT+1, colors, op->s.interiorcol );
        update |= 16;
        break;
      case DOTLEFT:                     /* previous text color */
        op->s.textcol--;
        update |= 32;
        break;
      case DOTLEFT+2:                   /* next text color */
        op->s.textcol++;
        update |= 32;
        break;
      case DOTLEFT+1:                   /* text color popup */
        op->s.textcol = do_popup( o, DOTLEFT+1, colors, op->s.textcol );
        update |= 32;
        break;
      case DOBLEFT:                     /* previous frame color */
        op->s.framecol--;
        update |= 64;
        break;
      case DOBLEFT+2:                   /* next frame color */
        op->s.framecol++;
        update |= 64;
        break;
      case DOBLEFT+1:                   /* frame color popup */
        op->s.framecol = do_popup( o, DOBLEFT+1, colors, op->s.framecol );
        update |= 64;
        break;
    }
  }
  d_stat( o, update );
}

/* Dialog Options exit button */
int d_exit( OBJECT *o, int num )
{
  if( num==DOHELP )
  {
    do_help( o[1].ob_spec.free_string );
    return 0;
  }
  else if( num==DOOK ) x_settings( 1, sizeof(SETTINGS), &set );  /* set perm */
  return 1;
}

/********************* Window Options routines *********************/

/* close the VDI workstation used in the Window Options dialog, and
   free fonts lists */
void close_vdi(void)
{
  if( wfontpop )
  {
    x_mfree(wfontpop);
    wfontpop = 0L;
    x_mfree(fontlist);
    fontlist = 0L;
  }
  if( vdi_hand )
  {
    if( has_gdos )
    {
      vst_font( vdi_hand, 1 );		/* reset to system font */
      vst_unload_fonts( vdi_hand, 0 );
    }
    v_clsvwk( vdi_hand );
    vdi_hand = 0;
  }
}

/* update the sample window */
void wo_update( OBJECT *o, int draw )
{
  int w, h, cw, ch, i;
  TEDINFO *ted;
  static int gadgets[5] = { XWMOVE, XWBACK, XWFULL, XWINFO, XWUP };

  w = o[XWOUTER].ob_width;              /* sample window width */
  h = o[XWOUTER].ob_height;             /* height */
  cw = font_wid + fontinfo.gadget_wid;  /* min. width of a gadget */
  ch = font_ht + fontinfo.gadget_ht;    /* min. height */
  /* Now set the dimensions of all of the window gadgets. This makes you
     appreciate just how much fun it is for Geneva to do this with real
     windows */
  o[XWMOVE].ob_width = o[XWBACK].ob_width = o[XWFULL].ob_width =
      o[XWUP].ob_width = o[XWVBIGSL].ob_width = o[XWVSMLSL].ob_width = cw;
  o[XWMOVE].ob_height = o[XWBACK].ob_height = o[XWFULL].ob_height =
      o[XWUP].ob_height = o[XWINFO].ob_height = o[XWVSMLSL].ob_height = ch;
  o[XWMOVE].ob_width = w - cw*2 + 2;
  o[XWBACK].ob_x = w - cw*2 + 1;
  o[XWFULL].ob_x = o[XWUP].ob_x = o[XWVBIGSL].ob_x = w - cw;
  o[XWINFO].ob_y = ch - 1;
  o[XWINFO].ob_width = w;
  o[XWUP].ob_y = (ch-1)*2;
  o[XWVBIGSL].ob_y = (ch-1)*3;
  o[XWVBIGSL].ob_height = h - (ch-1)*3;
  /* if a gadget goes off the bottom, make sure it won't show */
  if( o[XWUP].ob_y + ch >= h ) o[XWUP].ob_height = -5;
  if( o[XWINFO].ob_y + ch >= h ) o[XWINFO].ob_height = -5;
  if( o[XWVSMLSL].ob_y + o[XWVBIGSL].ob_y + ch >= h )
      o[XWVSMLSL].ob_height = -5;
  /* set the TEDINFOs up to display the correct font */
  for( i=0; i<5; i++ )
  {
    ted = o[gadgets[i]].ob_spec.tedinfo;
    ted->te_font = font_scalable ? GDOS_MONO : GDOS_BITM;
    /* some bindings may be redefined for this */
    ted->te_junk1 = fontinfo.font_id;
    ted->te_junk2 = fontinfo.point_size;
  }
  if( draw ) x_wdial_draw( form[3].handle, XWOUTER, 8 );
  /* set this info in Geneva */
  x_appl_font( 1, 0, &fontinfo );
}

/* use a VDI call to choose a new point size */
int _vst_point( int point, int *wid, int *ht )
{
  int dum, advx, advy, remx, remy, ret;

  if( !font_scalable )          /* not a Speedo/FSM font */
  {
    /* this actually gets the closest point size, going lower if necessary */
    ret = vst_point( vdi_hand, point, &dum, &dum, wid, ht );
  }
  else
  {
    /* set to arbitrary point size */
    ret = vst_arbpt( vdi_hand, point, &dum, &dum, wid, ht );
    /* get the offset to the next char; Geneva uses the widest
       char in the font, the M */
    vqt_advance( vdi_hand, 'M', &advx, &advy, &remx, &remy );
    *wid = advx;
    /* round the number of pixels up/down */
    remx >>= 12;
    if( remx==1 ) (*wid)++;
    else if( remx==2 ) (*wid)--;
  }
  return ret;
}

/* a new point size has been chosen */
void new_point( OBJECT *o, int draw )
{
  fontinfo.point_size = _vst_point( fontinfo.point_size, &font_wid, &font_ht );
  /* set the new number */
  x_sprintf( o[WOPT].ob_spec.free_string, "%d", fontinfo.point_size );
  /* reset and draw the sample */
  wo_update( o, draw );
  /* redraw the point number, if necessary */
  if( draw ) x_wdial_draw( form[3].handle, WOPT, 8 );
}

/* one of the gadget borders has changed */
void new_gadget( OBJECT *o, int draw )
{
  x_sprintf( o[WOWID].ob_spec.free_string, "%d", fontinfo.gadget_wid );
  x_sprintf( o[WOHT].ob_spec.free_string, "%d", fontinfo.gadget_ht );
  wo_update( o, draw );
  if( draw )
  {
    x_wdial_draw( form[3].handle, WOWID, 8 );
    x_wdial_draw( form[3].handle, WOHT, 8 );
  }
}

/* choose another font to use */
void set_fnum( OBJECT *o, int i, int draw )
{
  int dum;

  font_num = i;
  /* set the name of the current font in the dialog */
  o[WOFONT].ob_spec.free_string = fonts ?
      wfontpop[i+1].ob_spec.free_string+1 : *sys_font;
  if( draw ) x_wdial_draw( form[3].handle, WOFONT, 8 );
  font_scalable = fonts ? fontlist[i].scale : 0;
  vst_font( vdi_hand, fontinfo.font_id = fonts ? fontlist[i].id : 1 );
  /* reset point size because the previous point size might not be possible */
  new_point( o, draw );
}

/* get the current font info from Geneva */
void get_font( OBJECT *o, int draw )
{
  int i;

  x_appl_font( 0, 0, &fontinfo0 );                      /* initial value */
  memcpy( &fontinfo, &fontinfo0, sizeof(fontinfo) );    /* changeable copy */
  /* find the font name that matches Geneva's font ID */
  for( i=0; i<fonts; i++ )
    if( fontlist[i].id == fontinfo.font_id )
    {
      set_fnum( o, i, draw );
      return;
    }
  set_fnum( o, 0, 0 );	/* default to system font */
}

/* allocate an item, tacking it onto the end of an existing list */
int add_thing( void **ptr, int num, void *newthing, int size )
{
  int ok;

  num *= size;
  if( !*ptr )
  {
    x_malloc( ptr, num );
    ok = *ptr != 0;
  }
  else ok = !x_realloc( ptr, num );
  if( ok )
  {
    if(newthing) memcpy( (char *)(*ptr)+num-size, newthing, (size_t)size );
    return 1;
  }
  return 0;
}

/* add a new font name to the popup list */
int add_font( int id, char *name, char scale )
{
  int i, w;
  char *ptr;
  static OBJECT newobj = { -1, -1, -1, G_STRING, 0, 0, 0L, 0, 0, 0, 0 };

  if( !wfontpop )
  {
    if( !add_thing( (void **)&wfontpop, 1, &colors[0], sizeof(OBJECT) ) )
        return 0;
    wfontpop[0].ob_width = char_w*24+4;
    wfontpop[0].ob_next = wfontpop[0].ob_head = wfontpop[0].ob_tail = -1;
    newobj.ob_height = char_h;
    fonts = 0;
  }
  if( !add_thing( (void **)&wfontpop, fonts+2, &newobj, sizeof(OBJECT) ) ) return 0;
  if( !add_thing( (void **)&fontlist, fonts+1, 0L, sizeof(FONT_DESC) ) ) return 0;
  fontlist[fonts].id = id;
  fontlist[fonts].scale = scale;
  wfontpop[fonts+1].ob_y = fonts*char_h;
  wfontpop[fonts+1].ob_width = wfontpop[0].ob_width;
  ptr = fontlist[fonts].name;
  *ptr = ' ';
  /* if the font id is 1, then it's the default system font, otherwise
     use its real name */
  strncpy( ptr+1, id==1 ? *sys_font : name, 32 );
  *(ptr+33) = 0;
  /* add this object to the fonts popup */
  objc_add( wfontpop, 0, fonts+1 );
  if( (w = strlen(ptr)*char_w) > wfontpop[0].ob_width )
      wfontpop[0].ob_width = w;
  fonts++;
  for( i=0; i<fonts; i++ )
  {     /* always do this whole loop so that names get reset after add_thing */
    wfontpop[i+1].ob_spec.free_string = fontlist[i].name;
    wfontpop[i+1].ob_width = wfontpop[0].ob_width;
  }
  return 1;
}

/* set the size and type of the separator style sample */
void set_sepsamp( OBJECT *o, int draw )
{
  int x, i;

  x = o[WOSSAMP].ob_x;  /* save for later */
  /* copy the object from stylepop without ob_next, ob_head, ob_tail */
  memcpy( &o[WOSSAMP].ob_type, &stylepop[set.graymenu+1].ob_type,
      sizeof(OBJECT)-2-2-2 );
  o[WOSSAMP].ob_x = x;          /* restore */
  o[WOSSAMP].ob_y %= char_h;    /* align it vertically */
  o[WOSSAMP].ob_flags |= TOUCHEXIT;
  /* check the right item in the popup list */
  for( i=PSEP0; i<PSEP0+16; i++ )
    if( i-PSEP0 == set.graymenu ) stylepop[i].ob_state |= CHECKED;
    else stylepop[i].ob_state &= ~CHECKED;
  if( draw )
  {
    x_settings( 1, sizeof(SETTINGS), &set );  /* set menu style perm */
    x_wdial_draw( form[3].handle, WOSPOP, 8 );
  }
}

/* initialize the Window options dialog */
int wo_init( OBJECT *o )
{
  int i, j, total, min, max, id, dum[5], maxw, wid;
  char namebuf[33],
       speedo;          /* is it Speedo GDOS? */
  long *cookie;
  static int gadgets[7][2] = { XWMOVE, WGMOVE, XWBACK, WGBACK,
      XWFULL, WGFULL, XWINFO, WGINFO, XWUP, WGUP, XWVBIGSL,
      WGVBIGSL, XWVSMLSL, WGVSMLSL };

  if( vdi_hand ) return 1;      /* just in case the dialog is already open */
  vdi_hand = graf_handle( &char_w, dum, dum, dum );
  work_in[0] = Getrez() + 2;
  v_opnvwk( work_in, &vdi_hand, work_out );
  if( !vdi_hand )
  {
    alert( ALNOVDI );           /* could not open VDI workstation */
    return 0;
  }
  total = work_out[10];         /* number of system fonts */
  has_gdos = vq_gdos();         /* is GDOS present? */
  speedo = CJar(0, FSMC, &cookie) == CJar_OK && *cookie == _SPD;
  graf_mouse( BUSYBEE, 0L );
  if( has_gdos )                /* yes, load fonts */
      total += vst_load_fonts( vdi_hand, 0 );   /* system fonts + loaded fonts */
  /* loop through the list, only allowing monospaced bitmapped fonts,
     or Speedo GDOS fonts */
  for( i=1; i<=total; i++ )
  {
    namebuf[32] = 0;
    id = vqt_name( vdi_hand, i, namebuf );      /* font name */
    if( id != -1 )                              /* not a font to skip */
    {
      vst_font( vdi_hand, id );                 /* choose it */
      font_scalable = namebuf[32];              /* Speedo/FSM scalable flag */
      if( !font_scalable || !speedo )
      { /* check to see that this bitmapped or FSM font is monospaced */
        vqt_fontinfo( vdi_hand, &min, &max, dum, &maxw, dum );
        /* for 004, avoid a bug in NVDI 3 by only checking first 254 chars */
        if( (unsigned)min>1 || (unsigned)max<254 ) continue;
        /* go from the first to last character, making sure that each one is
           the full cell width */
        maxw = -1;
        for( j=1; j<=254; j++ )
        {
          vqt_width( vdi_hand, j, &wid, dum, dum );
          if( maxw>=0 && wid != maxw ) break; /* stop at the first odd char */
          maxw = wid;
        }
        if( j<255 ) continue;                   /* is the font monospaced? */
      }
      if( !add_font( id, namebuf, namebuf[32] && id!=1 ) )
      {
        if( !wfontpop )
        {
          graf_mouse( ARROW, 0L );
          return 0;
        }
        total = i;
        break;
      }
    }
  }
  graf_mouse( ARROW, 0L );
  if( !wfontpop )
    if( !add_font( 1, namebuf, 0 ) ) return 0;
  wfontpop[0].ob_height = fonts*char_h;
  /* get default window colors & states for sample */
  for( i=0; i<7; i++ )
  {
    j = gadgets[i][0];          /* dialog object to change */
    dum[0] = gadgets[i][1];     /* element # for wind_get() */
    wind_get( 0, X_WF_DCOLSTAT, &dum[0], &dum[1], &dum[2], &dum[3] );
    /* Set the object's color. If it's a G_BOXTEXT, set the TEDINFO */
    if( (char)o[j].ob_type == G_BOXTEXT )
        o[j].ob_spec.tedinfo->te_color = dum[1];
    else o[j].ob_spec.index = (o[j].ob_spec.index&0xFFFF0000L) | dum[1];
    o[j].ob_state = dum[3];
  }
  get_font( o, 0 );
  new_gadget( o, 0 );
  get_set();
  set_sepsamp( o, 0 );
  return 1;
}

/* Window options dialog exit event */
int wo_exit( OBJECT *o, int num )
{
  switch( num )
  {
    case WOHELP:
      do_help( o[1].ob_spec.free_string );
      return 0;
    case WODFLT:
      fontinfo.font_id = fontinfo.point_size = fontinfo.gadget_wid =
          fontinfo.gadget_ht = -2;      /* reset to defaults */
      x_appl_font( 1, 0, &fontinfo );   /* set */
      x_appl_font( 0, 0, &fontinfo );   /* get new values */
      get_font( o, 1 );
      new_gadget( o, 1 );
      return 0;
    case WOOK:
      /* Have the font settings changed? If so, alert the user */
      if( memcmp( &fontinfo, &fontinfo0, sizeof(fontinfo) ) ) alert( ALSAVE );
      return 1;
    default:
      x_appl_font( 1, 0, &fontinfo0 );  /* restore initial values */
      x_settings( 1, sizeof(SETTINGS), &set0 );  /* reset menu style */
      return 1;
  }
}

/* Window options dialog touchexit event */
void wo_touch( OBJECT *o, int num )
{
  int pt, dum;

  switch( num )
  {
    case WOFONT:                /* do the fonts popup list */
      if( fonts>1 ) set_fnum( o, do_popup( o, WOFONT, wfontpop, font_num ), 1 );
      break;
    case WOPUP:                 /* go up one point size */
      /* start at point+1 and continue until the size changes, since all sizes
         may not be available */
      for( pt=fontinfo.point_size+1; pt<=25; pt++ )
        if( _vst_point( pt, &dum, &dum ) == pt )
        {
          fontinfo.point_size = pt;
          new_point( o, 1 );
          break;
        }
      break;
    case WOPDWN:
      /* them Speedo fonts can get pretty small... */
      if( fontinfo.point_size < 4 ) break;
      /* find the next lower point size */
      if( _vst_point( pt=fontinfo.point_size-1, &dum, &dum ) <= pt )
      {
        fontinfo.point_size = pt;
        new_point( o, 1 );
      }
      break;
    case WOWUP:                         /* increase the gadget border width */
      if( fontinfo.gadget_wid<25 )
      {
        fontinfo.gadget_wid++;
        new_gadget( o, 1 );
      }
      break;
    case WOWDWN:                        /* decrease the gadget border width */
      if( fontinfo.gadget_wid>0 )
      {
        fontinfo.gadget_wid--;
        new_gadget( o, 1 );
      }
      break;
    case WOHUP:                         /* increase the gadget border height */
      if( fontinfo.gadget_ht<25 )
      {
        fontinfo.gadget_ht++;
        new_gadget( o, 1 );
      }
      break;
    case WOHDWN:                        /* decrease the gadget border height */
      if( fontinfo.gadget_ht>0 )
      {
        fontinfo.gadget_ht--;
        new_gadget( o, 1 );
      }
      break;
    case WOSLEFT:                       /* previous menu separator style */
      if( --set.graymenu<0 ) set.graymenu = 15;
      set_sepsamp( o, 1 );
      break;
    case WOSRT:                         /* next menu separator style */
      if( ++set.graymenu>15 ) set.graymenu = 0;
      set_sepsamp( o, 1 );
      break;
    case WOSSAMP:
    case WOSPOP:
      set.graymenu = do_popup( o, WOSPOP, stylepop, set.graymenu+PSEP0-1 ) -
          PSEP0 + 1;
      set_sepsamp( o, 1 );
      break;
  }
}

/********************* Window Colors routines **********************/

/* free memory for window object tree and temp storage for attributes */
void free_gadgets(void)
{
  if( gad_tree )
  {
    x_mfree(gad_tree);
    gad_tree = 0L;
  }
  if( gad_tmp )
  {
    x_mfree(gad_tmp);
    gad_tmp = 0L;
  }
}

/* read or set the permanent gadget colors */
void wg_getset( int getset )
{
  GADGET *g;
  int n;

  for( g=gad_tmp, n=WGCLOSE; n<=WGSIZE; n++, g++ )
    if( !getset ) wind_get( 0, X_WF_DCOLSTAT, &n, &g->color[1], &g->color[0], &g->state );
    else wind_set( 0, X_WF_DCOLSTAT, n, g->color[1], g->color[0], g->state );
}

/* set the attributes of all gadgets */
void wg_states(void)
{
  int n, *color, top, untop;
  GADGET *g;
  OBJECT *o = &gad_tree[WGCLOSE];

  for( g=gad_tmp, n=WGCLOSE; n<=WGSIZE; n++, g++, o++ )
  {
    /* for a G_BOXTEXT use the tedinfo->te_color */
    color = (char)o->ob_type == G_BOXTEXT ?
        (int *)&o->ob_spec.tedinfo->te_color :
        (int *)&o->ob_spec.index + 1;
    *color = g->color[wg_top];
    o->ob_state = g->state;
    if( n==wg_num ) o->ob_state |= OUTLINED;
    if( n==WGMENU )             /* menu text is special */
    {
      *color = (*color & 0xF07F) | (BLACK<<8);  /* always black and transparent */
      o->ob_state &= ~X_SHADOWTEXT;             /* no shadow */
    }
  }
}

/* set a dialog button based on the state of an atribute bit */
void wg_obj( int bit, OBJECT *o, int obj )
{
  set_if( o, obj, gad_tree[wg_num].ob_state&bit );
}

/* add one object to the tree */
void wg_add_ob( int ob )
{
  if( ob!=WGVSMLSL && ob!=WGHSMLSL && ob!=WGVSMLSL2 && ob!=WGHSMLSL2 )
      objc_add( gad_tree, 0, ob );
}

/* a neat trick to get the outlined object to draw correctly: reorder
   the tree so that it comes last */
void wg_order(void)
{
  int i;
  OBJECT *o = gad_tree;

  o[0].ob_head = o[0].ob_tail = -1;
  o += WGCLOSE;
  for( i=WGCLOSE; i<=WGSIZE; i++, o++ )
  {
    if( i!=wg_num )
    {   /* not the important object, so just add it in sequence */
      wg_add_ob(i);
      o->ob_state &= ~OUTLINED;
    }
    else o->ob_state |= OUTLINED;
  }
  wg_add_ob(wg_num);    /* add the important object last */
}

/* some defines used to extract the parts of a color */
#define BF_OPAQ(x)      ((x>>7)&1)
#define BF_FILL(x)      ((x>>4)&7)
#define BF_INTER(x)     (x&15)
#define BF_TEXT(x)      ((x>>8)&15)
#define BF_BORD(x)      ((x>>12)&15)

/* update the dialog objects to reflect the current gadget's attributes */
void wg_stat( OBJECT *o, int draw )
{
  unsigned int col;

  wg_obj( X_SHADOWTEXT, o, WGDSHAD );
  wg_obj( X_DRAW3D,     o, WGD3D );
  col = gad_tmp[wg_num-1].color[wg_top];        /* color of gadget */
  set_if( o, WGDOPAQ, BF_OPAQ(col) );           /* opaque? */
  /* set the popup samples */
  d_sample( o, WGDPLEFT+1, 1, BF_FILL(col) );   /* fill pattern */
  d_sample( o, WGDCLEFT+1, 0, BF_INTER(col) );  /* interior color */
  d_sample( o, WGDTLEFT+1, 0, BF_TEXT(col) );   /* text color */
  d_sample( o, WGDBLEFT+1, 0, BF_BORD(col) );   /* border color */
  hide_if( o, WGDTINV, wg_num!=WGMENU );        /* hide text atts if Menu */
  if( draw ) x_wdial_draw( form[9].handle, 0, 8 );  /* draw everything */
}

/* draw one gadget and its children */
void wg_draw( int obj )
{
  int x, y;

  wind_update( BEG_UPDATE );
  /* temporarily prevent the dialog from drawing */
  hide_if( form[9].tree, 0, 0 );
  objc_offset( gad_tree, obj, &x, &y );
  form_dial( FMD_FINISH, 0, 0, 0, 0, x-3, y-3,
      gad_tree[obj].ob_width+6, gad_tree[obj].ob_height+6 );
  /* restore the dialog so it will draw */
  hide_if( form[9].tree, 0, 1 );
  wind_update( END_UPDATE );
}

/* initialize the Window Colors dialog */
int wg_init( OBJECT *o )
{
  int hand=form[9].handle, vspl, hspl, i;
  char **p;

  /* set the window's default attributes */
  wind_set( hand, X_WF_MENU, menu );    /* just temporary */
  wind_set( hand, X_WF_HSLIDE2, 500 );
  wind_set( hand, X_WF_HSLSIZE2, 0 );
  wind_set( hand, X_WF_VSLIDE2, 500 );
  wind_set( hand, X_WF_VSLSIZE2, 0 );
  wind_set( hand, X_WF_VSPLIT, (vspl=o[WGDFBOX].ob_y+o[WGDFBOX].ob_height)+1 );
  wind_set( hand, X_WF_HSPLIT, (hspl=o[WGDFBOX].ob_x+o[WGDFBOX].ob_width)+1 );
  /* set the rest of the attributes and copy the tree */
  if( (gad_tree = wind_setup( hand, 1 )) == 0L ) return 0;
  /* allocate some memory for attributes */
  x_malloc( (void **)&gad_tmp, WGSIZE*sizeof(GADGET) );
  if( !gad_tmp ) return 0;
  /* make sure some gadgets are visible */
  hide_if( gad_tree, WGMNLEFT, 1 );
  hide_if( gad_tree, WGMNRT, 1 );
  hide_if( gad_tree, WGILEFT, 1 );
  hide_if( gad_tree, WGIRT, 1 );
  /* position the segments of the dialog so that they fit exactly
     within the split bars */
  o[WGDBBOX].ob_x = o[WGDXBOX].ob_x = hspl + gad_tree[WGHSPLIT].ob_width;
  o[WGDBBOX].ob_width = o[WGDXBOX].ob_width = o[0].ob_width - o[WGDBBOX].ob_x;
  o[WGDTBOX].ob_y = o[WGDXBOX].ob_y = vspl + gad_tree[WGVSPLIT].ob_height;
  o[WGDTBOX].ob_height = o[WGDXBOX].ob_height = o[0].ob_height - o[WGDBBOX].ob_y;
  /* position the Info and Menu bars, and left/right arrows */
  gad_tree[WGIRT].ob_x = gad_tree[WGMNRT].ob_x =
      gad_tree[0].ob_width - gad_tree[WGMNRT].ob_width;
  gad_tree[WGINFO].ob_x = gad_tree[WGMENU].ob_x =
      gad_tree[WGMNLEFT].ob_width - 1;
  gad_tree[WGINFO].ob_width = gad_tree[WGMENU].ob_width =
      gad_tree[0].ob_width - 2*gad_tree[WGMNLEFT].ob_width + 2;
  /* set the TEDINFOs for the Name and Info bars */
  gad_tree[WGMOVE].ob_spec.tedinfo = &gad_mover;
  gad_tree[WGINFO].ob_spec.tedinfo = &gad_info;
  /* make the Menu bar a BOXTEXT and give it a TEDINFO */
  gad_tree[WGMENU].ob_type = G_BOXTEXT;
  gad_tree[WGMENU].ob_spec.tedinfo = &gad_menu;
  /* set the TEDINFO strings */
  gad_mover.te_ptext = o[1].ob_spec.free_string;
  rsrc_gaddr( 15, WGDINFO, &p );
  gad_info.te_ptext = *p;
  rsrc_gaddr( 15, WGDMENU, &p );
  gad_menu.te_ptext = *p;
  wg_getset(0);                         /* read gadget attribues */
  wg_states();                          /* set everything */
  wg_order();                           /* re-order the tree */
  wg_stat( o, 0 );                      /* set dialog */
  set_if( o, WGDTOP, wg_top );          /* set Topped button */
  wind_set( hand, X_WF_MENU, 0L );      /* turn menu back off */
  return 1;
}

/* increment/decrement one portion of a color word */
void add_bits( int mask, int num )
{
  unsigned int *c;

  c = (unsigned int *)&gad_tmp[wg_num-1].color[wg_top];
  *c = (*c & ~mask) | (((*c & mask)+num) & mask);
  wg_states();                          /* update the gadgets */
}

/* affect one portion of a color word based on the state of a dialog button */
void set_bit( int *i, OBJECT *o, int ob, int bit )
{
  if( o[ob].ob_state&SELECTED )
  {
    *i |= bit;
    if( ob != WGDOPAQ ) *i |= X_MAGIC;
  }
  else *i &= ~bit;
  wg_states();                          /* update the gadgets */
}

/* use a popup menu to change a field in a color word */
int pop_bits( int type, OBJECT *o, int ob, int bit )
{
  unsigned int *c, val, ret, mask;

  mask = !type ? 15 : 7;
  c = (unsigned int *)&gad_tmp[wg_num-1].color[wg_top];
  val = (*c>>bit)&mask;
  ret = do_popup( o, ob, !type ? colors : patterns, val );
  if( ret==val ) return 0;
  *c = (*c & ~(mask<<bit)) | (ret<<bit);
  wg_states();
  return 1;
}

/* Window Colors touchexit event */
void wg_touch( OBJECT *o, int num )
{
  int draw=wg_num,      /* default to redrawing the current gadget */
      updt_pop=0,       /* default is no change in popups */
      dum;

  switch(num)
  {
    case WGDFBOX:
    case WGDBBOX:
    case WGDXBOX:
    case WGDTBOX:
    case WGDTINV:
      if( drag_wind( form[9].handle ) )
      {
        wind_get( form[9].handle, WF_CURRXYWH, &gad_tree[0].ob_x,
            &gad_tree[0].ob_y, &dum, &dum );
        wg_draw(0);
      }
      break;
    case WGDBLEFT:                      /* border color-- */
      add_bits( 15<<12, 15<<12 );
      updt_pop = WGDBLEFT+1;
      break;
    case WGDBLEFT+2:                    /* border color++ */
      add_bits( 15<<12, 1<<12 );
      updt_pop = WGDBLEFT+1;
      break;
    case WGDBLEFT+1:                    /* border color popup */
      if( !pop_bits( 0, o, WGDBLEFT+1, 12 ) ) draw = -1;
      updt_pop = WGDBLEFT+1;
      break;
    case WGDPLEFT:                      /* fill pattern-- */
      add_bits( 7<<4, 7<<4 );
      updt_pop = WGDPLEFT+1;
      break;
    case WGDPLEFT+2:                    /* fill pattern++ */
      add_bits( 7<<4, 1<<4 );
      updt_pop = WGDPLEFT+1;
      break;
    case WGDPLEFT+1:                    /* fill pattern popup */
      if( !pop_bits( 1, o, WGDPLEFT+1, 4 ) ) draw = -1;
      updt_pop = WGDPLEFT+1;
      break;
    case WGDCLEFT:                      /* fill color-- */
      add_bits( 15, -1 );
      updt_pop = WGDCLEFT+1;
      break;
    case WGDCLEFT+2:                    /* fill color++ */
      add_bits( 15, 1 );
      updt_pop = WGDCLEFT+1;
      break;
    case WGDCLEFT+1:                    /* fill color popup */
      if( !pop_bits( 0, o, WGDCLEFT+1, 0 ) ) draw = -1;
      updt_pop = WGDCLEFT+1;
      break;
    case WGDTLEFT:                      /* text color-- */
      add_bits( 15<<8, 15<<8 );
      updt_pop = WGDTLEFT+1;
      break;
    case WGDTLEFT+2:                    /* text color++ */
      add_bits( 15<<8, 1<<8 );
      updt_pop = WGDTLEFT+1;
      break;
    case WGDTLEFT+1:                    /* text color popup */
      if( !pop_bits( 0, o, WGDTLEFT+1, 8 ) ) draw = -1;
      updt_pop = WGDTLEFT+1;
      break;
    case WGD3D:                         /* toggle 3D attribute */
      set_bit( &gad_tmp[wg_num-1].state, o, WGD3D, X_DRAW3D );
      break;
    case WGDSHAD:                       /* toggle SHADOWED */
      set_bit( &gad_tmp[wg_num-1].state, o, WGDSHAD, X_SHADOWTEXT );
      break;
    case WGDOPAQ:                       /* toggle OPAQUE */
      set_bit( &gad_tmp[wg_num-1].color[wg_top], o, WGDOPAQ, (1<<7) );
      break;
    case WGDTOP:                        /* toggle Topped */
      wg_top ^= 1;
      wg_states();                      /* reset all gadgets */
      draw = 0;                         /* draw starting at root */
      break;
  }
  if( draw >= 0 ) wg_draw( draw );      /* maybe redraw gadget(s) */
  if( updt_pop )                        /* update a popup sample */
  {
    wg_stat( o, 0 );
    x_wdial_draw( form[9].handle, updt_pop, 0 );
  }
}

/* Window Colors exit event */
int wg_exit( OBJECT *o, int num )
{
  if( num==WGDHELP )
  {
    do_help( o[1].ob_spec.free_string );
    return 0;
  }
  if( num==WGDOK ) wg_getset(1);        /* OK button: change permanently */
  free_gadgets();
  return 1;
}

/* convert a WA_UPPAGE...WA_RTLINE message to a window gadget number */
int wg_conv( int num, int area2 )
{
  static char gads1[] = { WGVBIGSL, WGVBIGSL, WGUP, WGDOWN, WGHBIGSL,
      WGHBIGSL, WGLEFT, WGRT };
  static char gads2[] = { WGVBIGSL2, WGVBIGSL2, WGUP2, WGDOWN2, WGHBIGSL2,
      WGHBIGSL2, WGLEFT2, WGRT2 };

  if( area2 ) return gads2[num];
  return gads1[num];
}

/* find which gadget mouse is over */
int cc_obj( OBJECT *o, int mx, int my )
{
  return objc_find( o, 0, 8, mx, my );
}

/* drag an outline from one gadget to another */
void copy_color( OBJECT *o, int old )
{
  int mx, my, mw, mh, inew, bx, by;

  graf_mkstate( &mx, &my, &mw, &mh );
  /* if mouse button is up or this is not a gadget, get out */
  if( !(mw&1) || cc_obj( o, mx, my )<=0 ) return;
  /* get coordinates of old (starting) gadget */
  objc_offset( o, old, &mx, &my );
  mw = o[old].ob_width;
  mh = o[old].ob_height;
  /* get coordinates of window border */
  objc_offset( o, 0, &bx, &by );
  wind_update( BEG_UPDATE );
  graf_mouse( POINT_HAND, 0L );
  if( graf_dragbox( mw, mh, mx, my, bx, by, o[0].ob_width,
      o[0].ob_height, &mx, &my ) )
  {
    graf_mkstate( &mx, &my, &mw, &mh );
    /* is the new location a gadget different from the starting one? */
    if( (inew = cc_obj( o, mx, my )) > 0 && inew != old )
    {
      memcpy( &gad_tmp[inew-1], &gad_tmp[old-1], sizeof(GADGET) );
      wg_states();              /* reset state of new gadget */
      wg_draw( inew );          /* and draw it */
    }
  }
  graf_mouse( ARROW, 0L );
  wind_update( END_UPDATE );
}

/* either select a new gadget or copy */
void wg_new( int num )
{
  if( num != wg_num )
  {                                             /* new gadget */
    gad_tree[wg_num].ob_state &= ~OUTLINED;     /* turn off outline */
    wg_draw(wg_num);                            /* and draw it */
    wg_num = num;                               /* set new */
    wg_order();                                 /* reorder tree */
    wg_stat( form[9].tree, 1 );                 /* set dialog and draw */
    wg_draw(num);                               /* draw new gadget */
  }
  copy_color( gad_tree, wg_num );               /* maybe copy attribtues */
}

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
      wind_set( f->handle, WF_TOP );    /* so top it, instead */
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
        if( f->wind.g_x >= (dum=max.g_w-25) ) f->wind.g_x = dum;
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
        if( f->exit && (*f->exit)( f->tree, num ) ) close_wind(&f->handle);
        else x_wdial_draw( f->handle, but, 8 );         /* just draw button */
      }
      return;
    }
}

/************************ Task list routines ***********************/

/* calculate a check sum */
char chksum( char *buf )
{
  char chk=0;

  while( *buf ) chk = (chk + *buf++)<<1;
  return chk;
}

/* compare one app's name to another; used with qsort() */
int qsc( struct App *ap1, struct App *ap2 )
{
  return( strcmp( ap1->name, ap2->name ) );
}

/* enable/disable menu entries based on what is selected */
void set_term_sleep(void)
{
  menu_ienable( menu, TERM, tasknum>0 );
  menu_ienable( menu, SLEEP, tasknum>0 );
  menu_ienable( menu, TMPFLAGS, tasknum>0 );
  menu_icheck( menu, SLEEP, tasknum>0 && (app[tasknum-1].asleep&1) );
}

/* update the task list */
int task_list(void)
{
  static char chk=0;            /* previous checksum */
  char nchk;                    /* new checksum */
  int i,                        /* index of current app */
      first,                    /* 0 for first app, 1 thereafter */
      type, sleep;
  struct App *a;

  /* Get the current list of apps and compute a checksum along the way. This
     way the list is only redrawn when something really changes */
  for( nchk=0, first=0, i=1, a=&app[0]; i<LISTAPPS; i++, a++ )
    if( appl_search( first, a->name+3, &type, &a->num ) )  /* find next name */
    {
      a->asleep = x_appl_sleep( a->num, -1 );              /* is it asleep? */
      /* if an app is single-tasking, its text is smaller, so put 3
         spaces before the name, instead of 2 */
      if( a->asleep&2 ) strcpy( a->name+2, a->name+3 );
      else a->name[2] = ' ';
      /* add to checksum */
      nchk += chksum(a->name+2) + type + a->asleep;
      a->name[0] = type!=4 ? ' ' : '�';                 /* is it a desk acc? */
      a->name[1] = ' ';
      first=1;
    }
    else break;
  i--;                                         /* i is now the count of apps */
  qsort( app, i, sizeof(app[0]), qsc );         /* sort them alphabetically */
  if( i != apps || chk != nchk )                /* list has changed */
  {
    apps = i;
    chk = nchk;                                /* set checksum for next time */
    applist[0].ob_head = applist[0].ob_tail = -1;
    /* modify the object tree, adding as many objects as needed */
    for( i=1, a=&app[0]; i<=apps; i++, a++ )
    {
      memcpy( applist+i, new, sizeof(OBJECT) );         /* copy from "new" */
      applist[i].ob_y = (i-1)*char_h;                   /* set y-coord */
      applist[i].ob_spec.free_string = a->name;         /* set name */
      if( a->asleep&1 ) applist[i].ob_flags |= X_ITALICS;      /* sleeping */
      if( !(a->asleep&2) ) applist[i].ob_state |= X_SMALLTEXT; /* single-task */
      if( i==tasknum ) applist[i].ob_state |= SELECTED; /* current selection */
      objc_add( applist, 0, i );
    }
    return 1;
  }
  return 0;
}

/*********************** Main window routines **********************/

/* set the height of the applications list */
void set_bl_height(void)
{
  int i, dum, h;

  applist[0].ob_height = (i=apps*char_h) > winner.g_h ? i : winner.g_h;
  x_wind_calc( WC_BORDER, WIN_TYPE, X_MENU, 0, 0, applist[0].ob_width,
      applist[0].ob_height, &dum, &dum, &dum, &h );
  /* readjust max window height */
  if( main_hand>0/*007*/ ) wind_set( main_hand, X_WF_MINMAX, -1, -1, -1, h );
}

/* calculate the size of the applications list */
void get_inner(void)
{
  int i;

  x_wind_calc( WC_WORK, WIN_TYPE, X_MENU, wsize.g_x, wsize.g_y, wsize.g_w,
      wsize.g_h, &winner.g_x, &winner.g_y, &winner.g_w, &winner.g_h );
  newtask[0].ob_x = 2;                /* new app names always 2 pixels over */
  newtask[0].ob_width = winner.g_w - 4;
  for( i=0; i<apps; i++ )
    applist[i+1].ob_width = newtask[0].ob_width;
  applist[0].ob_width = winner.g_w;
  set_bl_height();
}

/* calculate the size of the outer window */
void get_outer(void)
{
  /* round the overall height to the nearest character height */
  winner.g_h = (winner.g_h+(char_h>>1))/char_h*char_h;
  x_wind_calc( WC_BORDER, WIN_TYPE, X_MENU, winner.g_x, winner.g_y,
      winner.g_w, winner.g_h, &wsize.g_x, &wsize.g_y, &wsize.g_w,
      &wsize.g_h );
}

/* wait for the button to be released */
void but_up(void)
{
  int b, dum;

  do
    graf_mkstate( &dum, &dum, &b, &dum );
  while( b&1 );
}

/* activate/deactivate a particular entry in the apps list */
void tnum_onoff( int handle, int num, int state )
{
  x_wdial_change( handle, num, !state ? (applist[num].ob_state&~SELECTED) :
      (applist[num].ob_state|SELECTED) );
}

/* send an AC_OPEN message to an app; this either opens or tops the app
   (Geneva decides which) */
void opentask( int handle, int num )
{
  int msg[8];

  msg[0] = AC_OPEN;
  msg[2] = 0;
  msg[4] = -1;
  appl_write( msg[1] = app[num-1].num, 16, msg );
  tnum_onoff( handle, num, 0 );         /* turn the name off */
}

/*********************** Misc. main routines ***********************/

/* get an environmental variable and add it to the list of executable
   filename extensions to look for in the Open item selector */
int exec_ext( char *path, char *env, int i )
{
  char *ptr;

  if( shel_envrn( &ptr, env ) && ptr && *ptr )
  {
    /* found it! If this is the first time, add *.{, otherwise add a comma */
    strcat( path, i ? "," : "*.{" );
    strcat( path, ptr );
    return 1;
  }
  return i;
}

/* get the number of bitplanes and use this to decide which entries to use
   for several of the SETTINGS */
void get_color_mode(void)
{
  int work[56], vplanes, vdi_hand;

  vdi_hand = graf_handle( work, work, work, work );
  vq_extnd( vdi_hand, 1, work );
  vplanes = work[4];
  if( vplanes>=8 ) color_mode = 3;
  else if( vplanes>=4 ) color_mode = 2;
  else if( vplanes>=2 ) color_mode = 1;
  else color_mode = 0;
}

void quit(void)
{
  int i;

  for( i=0; form[i].init; i++ )
    close_wind( &form[i].handle );
  rsrc_free();
  appl_exit();
  exit(0);
}

/* get the position of a particular form's window */
void get_formxy( int i )
{
  FORM *f = &form[i];
  
  if( f->handle>0 )
    wind_get( f->handle, WF_CURRXYWH, &f->wind.g_x, &f->wind.g_y,
        &f->wind.g_w, &f->wind.g_h );
}

/* iconify the main Task Manager window and close all other open windows */
void do_iconify( int handle, int buf[] )
{
  int top, dum, wind, i, place;

  iconified = 1;
  /* close all open dialog windows, but don't delete */
  wind_update( BEG_UPDATE );
  /* get the next window up from the desktop */
  wind_get( 0, WF_OWNER, &dum, &dum, &wind, &dum );
  for( place=0; wind; ) /* wind=0 when past top window */
  {
    for( i=0; form[i].init; i++ )
      if( form[i].handle==wind )        /* it's one of my windows */
      {
        get_formxy(i);
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
}

/* uniconify the main Task Manager window and/or dialogs */
void do_uniconify( int handle, int buf[] )
{
  int i, place, count;

  iconified = 0;
  /* briefly select the icon */
  x_wdial_change( handle, 1, icon[1].ob_state|SELECTED );
  icon[1].ob_state &= ~SELECTED;
  /* turn dialog off so that icon's size and position will not change */
  wind_set( handle, X_WF_DIALOG, 0L );
  wind_set( handle, WF_UNICONIFY, buf[4], buf[5], buf[6], buf[7] );
  /* restore old dialog */
  wind_set( handle, X_WF_DIALOG, applist );
  /* reopen all dialog windows, bottom first, ending with top */
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
}

/* actually change a window's position */
void currxy( int hand, GRECT *gnew )
{
  wind_set( hand, WF_CURRXYWH, gnew->g_x, gnew->g_y, gnew->g_w, gnew->g_h );
}

/* actually change a window's position */
void resize_wind( int hand, GRECT *gnew )
{
  GRECT old;
  FORM *f;

  if( hand==main_hand && !iconified )          /* main window? */
  {
    old = wsize;
    wsize = *gnew;
    void get_inner();
    if( winner.g_h > applist[0].ob_height ) winner.g_h =
        applist[0].ob_height;
    void get_outer();
    void get_inner();
    currxy( hand, &wsize );
    if( !iconified && hand==main_hand && wsize.g_w <= old.g_w && wsize.g_h
        <= old.g_h && (wsize.g_w != old.g_w || wsize.g_h != old.g_h) )
        wind_set( hand, X_WF_DIALOG, applist );
    /* force a redraw of main window if it wouldn't otherwise happen */
  }
  /* if it's a dialog, set the "wind" element to reflect gnew position */
  else
  {
    for( f=form; f->init; f++ )
      if( f->handle==hand )
      {
        f->wind = *gnew;
        break;
      }
    currxy( hand, gnew );
  }
}

/* move a window that does not have a functional mover bar */
int drag_wind( int hand )
{
  GRECT g;

  wind_get( hand, WF_CURRXYWH, &g.g_x, &g.g_y, &g.g_w, &g.g_h );
  if( graf_dragbox( g.g_w, g.g_h, g.g_x, g.g_y, max.g_x-g.g_w,
      max.g_y, max.g_w<<1, max.g_h<<1, &g.g_x, &g.g_y ) )
  {
    resize_wind( hand, &g );
    return 1;
  }
  return 0;
}

/* write a full GRECT or just the x and y */
int put_grect( GRECT *g, int wh, int ok )
{
  char temp[50];

  if( ok>0 )
  {
    x_sprintf(temp, (char *) (wh ? "%d %d %d %d %d %d" : "%d %d"), g->g_x,
        g->g_y<max.g_y ? -99 : g->g_y-max.g_y, g->g_w/char_w,
        g->g_w%char_w, g->g_h/char_h, g->g_h%char_h );
    ok = x_shel_put( X_SHACCESS, temp );
  }
  return ok;
}

/* save window and dialog positions */
void save_settings(void)
{
  int ok, i;

  if( x_shel_put( X_SHOPEN, "Task Manager" ) )
  {
    ok = x_shel_put( X_SHACCESS, TASKMAN_VER );
    ok = put_grect( &wsize, 1, ok );
    for( i=0; i<sizeof(form)/sizeof(FORM)-1; i++ )
    {
      get_formxy(i);
      ok = put_grect( &form[i].wind, 0, ok );
    }
    if( ok>0 ) x_shel_put( X_SHCLOSE, 0L );
  }
}

/* read a full GRECT or just the x and y */
int get_grect( GRECT *g, int wh, int ok )
{
  char temp[50];
  int w, h, wr, hr;

  if( ok>0 && (ok = x_shel_get( X_SHACCESS, sizeof(temp), temp )) > 0 )
  {
    x_sscanf(temp, (char *) (wh ? "%d %d %d %d %d %d" : "%d %d"),
             &g->g_x, &g->g_y, &w, &wr, &h, &hr );
    if( g->g_x >= max.g_w ) g->g_x = max.g_w - (char_w<<1);
    if( g->g_y >= max.g_h ) g->g_y = max.g_h - (char_h<<1);
    g->g_y += max.g_y;
    if( wh )
    {
      w = w*char_w + wr;
      h = h*char_h + hr;
      if( w && h )
      {
        g->g_w = w;
        g->g_h = h;
      }
    }
  }
  return ok;
}

/* load window and dialog positions */
void load_settings(void)
{
  char temp[10];
  int ok, i;

  /* wait for CNF file to be released by another app */
  while( (i=x_shel_get( X_SHOPEN, 0, "Task Manager" )) == -1 );
  if( i>0 )	/* file could be opened and my section found */
  {
    ok = x_shel_get( X_SHACCESS, sizeof(temp), temp );
    if( !strcmp( temp, TASKMAN_VER ) )
    {
      ok = get_grect( &wsize, 1, ok );
      for( i=0; i<sizeof(form)/sizeof(FORM)-1; i++ )
        ok = get_grect( &form[i].wind, 0, ok );
    }
    if( ok>0 ) x_shel_get( X_SHCLOSE, 0, 0L );
  }
}

int run_program( char *path )
{ 
  SHWRCMD shwrcmd;              /* shel_write() structure */
  char temp[120];

  shwrcmd.name = path;
  strcpy( temp, path );
  *(strrchr(temp,'\\')+1) = 0;	/* remove filename from default dir */
  shwrcmd.dflt_dir = temp;
  /* run the task, letting Geneva decide how */
  return shel_write( SHD_DFLTDIR|SHW_RUNANY, 1, 0, (char *)&shwrcmd, "\0" );
}

int main(void)
{
  char **title,                 /* main window title */
       **fstitle,               /* title in item selector */
       **term_msg,              /* termination alert string */
       name[13*5+1],            /* room for 5 items */
       temp[150], temp2[80],
       *ptr, *ptr2, **ptr3,
       was_open=0,              /* was window already opened? */
       fulled;                  /* is window fulled now? */
  int buf[8],                   /* message buffer */
      i,
      but,
      dum,
      event,                    /* event returned by evnt_multi() */
      etype=MU_MESAG|X_MU_DIALOG, /* type of events to look for */
      timer,                    /* evnt_multi() time to wait */
      keys,                     /* which Control/Shift/Alt keys held */
      min_ht,                   /* minimum window height */
      min_wid,                  /* window width */
      max_wid;                  /* max width */
  GRECT prev;                   /* old position of fulled window */
  long cook;
  G_COOKIE *cookie;

  apid = appl_init();
  /* Tell the AES (or Geneva) that we understand AP_TERM message */
  if( _GemParBlk.global[0] >= 0x400 ) shel_write( SHW_MSGTYPE, 1, 0, 0L, 0L );
  if( rsrc_load("taskman.rsc") )
  {
    /* Check to make sure Geneva rel 003 or newer is active */
    if( CJar( 0, GENEVA_COOKIE, &cookie ) == CJar_OK && cookie &&
        cookie->ver >= 0x104 )
    {
      get_color_mode();
      /* default item selector path */
      path[0] = Dgetdrv()+'A';
      path[1] = ':';
      Dgetpath( path+2, 0 );
      strcat( path, "\\" );
      /* read extensions from GEM env vars for use in item selector */
      if( !exec_ext( path, "ACCEXT=", exec_ext( path, "TOSEXT=",
          exec_ext( path, "GEMEXT=", 0 ) ) ) ) strcat( path, "*.*" );
      else strcat( path, "}" );
      name[0] = 0;
      /* get main window menu tree and initialize root for keybd equivs */
      rsrc_gaddr( 0, TMENU, &menu );
      menu[0].ob_state |= X_MAGIC;
      /* get the popup menus */
      rsrc_gaddr( 0, PCOLORS, &colors );
      rsrc_gaddr( 0, PFILLS, &patterns );
      rsrc_gaddr( 0, PSEPS, &stylepop );
      rsrc_gaddr( 0, POTHKEYS, &kpop );
      /* get iconify icon */
      rsrc_gaddr( 0, TASKICON, &icon );
      /* get other strings used */
      rsrc_gaddr( 15, TITLE, &title );
      rsrc_gaddr( 15, FSTITLE, &fstitle );
      rsrc_gaddr( 15, NEW, &new_flag );
      rsrc_gaddr( 15, SSYSTEM, &sys_font );
      /* fix the newtask object tree used for entries in the task list */
      rsrc_obfix( newtask, 0 );
      char_h = newtask[0].ob_height;
      char_w = (newtask[0].ob_width-6)/10;
      /* fix positions for ST Medium rez */
      for( i=2; i<17; i++ )
        stylepop[i].ob_y = (i-1)*char_h + (char_h>>1);
      /* set default window width to same size as menu */
      wsize.g_w = menu[2].ob_width;
      get_inner();
      wind_get( 0, WF_WORKXYWH, &max.g_x, &max.g_y, &max.g_w, &max.g_h );
      load_settings();
      /* set the correct name in the menu */
      menu_register( apid, *title );
      if( _app )          /* running as PRG */
      {
        graf_mouse( ARROW, 0L );
        goto open;
      }
      timer = 500;      /* wait for 5 seconds, normally */
      for(;;)
      {
        event = evnt_multi( etype,  0,0,0,  0,0,0,0,0,
            0,0,0,0,0,  buf,  timer,0,  &dum, &dum, &dum, &keys, &dum, &dum );
        timer = 500;
        if( event&MU_MESAG ) switch( buf[0] )
        {
          case AC_OPEN:
open:       if( main_hand<=0 )
            {
              if( !was_open )
              {
                get_inner();	/* recalculate based on saved position */
                wind_update( BEG_UPDATE );
                task_list();
                set_term_sleep();
                wind_update( END_UPDATE );
                /* list must be at least 16 long */
                i = apps<16 ? apps*char_h : char_h*16;
                if( wsize.g_h < i ) winner.g_h = i;
                get_outer();
              }
              if( (main_hand=x_wind_create( WIN_TYPE, X_MENU,
                  wsize.g_x, wsize.g_y, wsize.g_w, wsize.g_h )) > 0 )
              {
                if( !was_open )
                {
                  /* get current minimum sizes */
                  wind_get( main_hand, X_WF_MINMAX, &min_wid, &min_ht,
                      &max_wid, &dum );
                  /* take either the width of the Options title in the
                     menu bar or the length of a name in the list,
                     whichever is greater */
                  x_wind_calc( WC_BORDER, WIN_TYPE, X_MENU, 0, 0,
                      i=menu[4].ob_width, 0, &dum, &dum, &buf[0], &dum );
                  /* 2*gadget width because of menu arrows */
                  buf[0] += buf[0] - i;
                  if( min_wid < buf[0] ) min_wid = buf[0];
                  x_wind_calc( WC_BORDER, WIN_TYPE, X_MENU, 0, 0,
                      10*char_w+6, 0, &dum, &dum, &buf[0], &dum );
                  if( min_wid < buf[0] ) min_wid = buf[0];
                  if( wsize.g_w < min_wid ) wsize.g_w = min_wid;
                  if( wsize.g_h < min_ht )
                  {
                    /* find the size of the smallest area that can fit into
                       the minimum height; start by calculating the inner
                       height using the outer min_ht */
                    x_wind_calc( WC_WORK, WIN_TYPE, X_MENU, 0, 0, 0, min_ht,
                        &dum, &dum, &dum, &buf[0] );
                    /* buf[0] now has the inner height, so round this to the
                       nearest character height and calculate the outer height
                       again */
                    x_wind_calc( WC_BORDER, WIN_TYPE, X_MENU, 0, 0, 0,
                       (buf[0]+char_h-1)/char_h*char_h, &dum, &dum, &dum,
                       &min_ht );
                    wsize.g_h = min_ht;
                  }
                  if( max_wid > menu[2].ob_width ) max_wid=menu[2].ob_width;
                }
                /* set the window's min/max sizes */
                wind_set( main_hand, X_WF_MINMAX, min_wid, min_ht, max_wid, -1 );
                get_inner();
                /* position the app list to the start */
                applist[0].ob_x = winner.g_x;
                applist[0].ob_y = winner.g_y;
                /* tell Geneva it's a windowed dialog */
                wind_set( main_hand, X_WF_DIALOG, applist );
                /* scroll vertically in increments of the character height */
                wind_set( main_hand, X_WF_DIALHT, char_h );
                /* put in the menu */
                wind_set( main_hand, X_WF_MENU, menu );
                wind_set( main_hand, WF_NAME, *title + 2 );
                wind_open( main_hand, wsize.g_x, wsize.g_y, wsize.g_w, wsize.g_h );
                etype |= MU_TIMER;
                timer = 0;                /* force task list update */
                was_open = 1;
                fulled = 0;
              }
              else alert( NOWIND );
              break;
            }
            buf[3] = main_hand;      /* fall through if already open */
          case WM_TOPPED:
            /* Don't top flags dialog if Find is open */
            if( form[8].handle<=0 || buf[3]!=form[5].handle )
                wind_set( buf[3], WF_TOP );
            break;
          case WM_FULLED:
            if( buf[3] != main_hand ) break;
            if( (fulled^=1) != 0 )      /* full it */
            {
              prev = wsize;             /* save for later */
              buf[4] = wsize.g_x;
              buf[5] = wsize.g_y;
              /* get max size and set it to that */
              wind_get( main_hand, X_WF_MINMAX, &dum, &dum, &buf[6], &buf[7] );
            }
            else *(GRECT *)&buf[4] = prev;      /* restore old position */
            /* make sure it won't go off the screen */
            if( buf[5]+buf[7] > max.g_y+max.g_h ) buf[7] =
                max.g_y+max.g_h-buf[5];
            /* fall through */
          case WM_MOVED:
          case WM_SIZED:
            resize_wind( buf[3], (GRECT *)&buf[4] );
            break;
          case WM_CLOSED:
            if( buf[3]==main_hand )        /* main window? */
            {
close:        close_wind(&main_hand);
              etype &= ~MU_TIMER;
              if( _app ) quit();        /* running as a PRG, so quit */
            }
            break;
          case AC_CLOSE:
            main_hand=0;
            free_gadgets();
            close_vdi();                /* close VDI workstation if open */
            for( i=0; form[i].init; i++ )
              form[i].handle = 0;
            iconified = 0;              /* just in case it was */
            /* don't bother waiting for timer events anymore */
            etype = MU_MESAG|X_MU_DIALOG;
            break;
          case X_WM_ARROWED2:
            if( form[9].handle>0 && buf[3]==form[9].handle )
                wg_new( wg_conv( buf[4], 1 ) );
            break;
          case WM_ARROWED:
            /* user clicked on an arrow in the Keyboard or Gadgets dialog */
            if( form[0].handle>0 && buf[3]==form[0].handle )
                new_key( buf[4], 0 );
            else if( form[9].handle>0 && buf[3]==form[9].handle )
                wg_new( wg_conv( buf[4], 0 ) );
            break;
          case AP_TERM:
            /* I'm being told to quit by the AES */
            quit();
          case SHUT_COMPLETED:
            /* a shutdown I started is complete */
            if( buf[3]==0 ) alert( ALNOSHUT );  /* it failed */
            else quit();
            break;
          case RESCH_COMPLETED:
            /* a rez change I started is complete */
            if( buf[3]==0 ) alert( ALREZ );     /* it failed */
            else quit();
            break;
          case CH_EXIT:
            /* a process I started has terminated */
            if( buf[4] )                        /* return code was non-zero */
            {
              /* ask Geneva for the name and ID of the app that just term'd */
              appl_search( -1, temp2, &dum, &but );
              /* get some strings */
              rsrc_gaddr( 15, ALTERM, &term_msg );
              rsrc_gaddr( 15, SUNKNOWN, &ptr3 );
              /* if this is the same app Geneva reported, then use the name
                 Geneva returned, otherwise <Unknown> */
              x_sprintf( temp, *term_msg, but==buf[3] ? temp2 : *ptr3, buf[4] );
              /* if it's <0, then Geneva can give it real text */
              if( buf[4]<0 ) x_form_error( temp, buf[4] );
              else
              {
                /* otherwise, undefined error */
                rsrc_gaddr( 15, SUNDEF, &ptr3 );
                spf_alert( temp, *ptr3 );
              }
            }
            timer = 0;
            break;
          case WM_ICONIFY:
          case WM_ALLICONIFY:
            if( buf[3]==main_hand && !iconified ) /* main window */
            {
              do_iconify( main_hand, buf );
              etype &= ~(MU_TIMER|X_MU_DIALOG);
            }
            break;
          case WM_UNICONIFY:
            if( buf[3]==main_hand && iconified ) /* main window */
            {
              do_uniconify( main_hand, buf);
              etype |= MU_TIMER|X_MU_DIALOG;
              event |= MU_TIMER;          /* force update */
            }
            break;
          case X_WM_SELECTED:
            /* user selected an object in the Keyboard or Gadgets dialog */
            if( form[0].handle>0 && buf[3]==form[0].handle )
              /* if it's the mover bar, move the window */
              if( buf[4]==WGMOVE ) drag_wind( buf[3] );
              else
              {
                /* if it's the iconize (SMALLER) gadget and Control held,
                   then just go to the right key */
                if( buf[4] == WGICONIZ && keys&4 ) new_key( XS_ALLICON, 0 );
                /* if it's the cycle gadget and Shift held,
                   then just go to the right key */
                else if( buf[4] == WGBACK && keys&3 ) new_key( XS_CYCLE2, 0 );
                else new_key( buf[4], 1 );        /* else, process it */
              }
            else if( form[9].handle>0 && buf[3]==form[9].handle )
                wg_new( buf[4] );
            break;
          case X_MN_SELECTED:
            /* window menu item selected */
            if( !iconified ) switch( buf[4] )
            {
              case OPEN:                        /* open a task */
                if( tasknum>0 )                 /* top a task that is open */
                {
                  opentask( main_hand, tasknum );
                  tasknum=0;
                  set_term_sleep();
                }
                else if( x_fsel_input( path, sizeof(path), name, 5,
                    &but, *fstitle ) && but )   /* ask for task names */
                {
                  strcpy( temp, path );
                  if( (ptr=strrchr(temp,'\\')) == 0 ) ptr=temp;
                  else ptr++;
                  /* go through the list of names, opening up to 5 */
                  for( ptr2=name; *ptr2; ptr2+=strlen(ptr2)+1 )
                  {
                    strcpy( ptr, ptr2 );
                    if( !run_program(temp) )
                    {
                      /* task could not be opened */
                      rsrc_gaddr( 15, ALOPEN, &ptr3 );
                      spf_alert( *ptr3, ptr2 );
                    }
                  }
                }
                break;
              case QUIT:
                if( main_hand ) close_wind(&main_hand);
                for( i=0; form[i].init; i++ )
                  close_wind(&form[i].handle);
                if( _app ) quit();
                /* don't bother waiting for timer events anymore */
                etype = MU_MESAG|X_MU_DIALOG;
                break;
              case TERM:
                if( tasknum>0 )
                  if( x_appl_term( app[tasknum-1].num, 0, 1 ) )
                      timer = 0;        /* force list to update */
                break;
              case SLEEP:
                if( tasknum>0 )
                {
                  x_appl_sleep( app[tasknum-1].num,
                      (app[tasknum-1].asleep&1)^1 );    /* toggle sleep */
                  timer = 0;
                }
                break;
              case MHELP:
                /* get main help */
                rsrc_gaddr( 15, HELPMAIN, &ptr3 );
                do_help( *ptr3 );
                break;
              case KEYBOARD:
                start_form( 0, KEYOPTS, CLOSER|FULLER|MOVER|INFO|UPARROW|\
                    DNARROW|VSLIDE|LFARROW|RTARROW|HSLIDE|SMALLER, 0 );
                break;
              case MISC:
                start_form( 1, MISCOPTS, NAME|MOVER, 0 );
                break;
              case DIALOGS:
                start_form( 2, DIALOPTS, NAME|MOVER, 0 );
                break;
              case TWINDOWS:
                start_form( 3, WINDOPTS, NAME|MOVER, 0 );
                break;
              case WCOLORS:
                start_form( 9, WINDGADS, CLOSER|FULLER|MOVER|INFO|UPARROW|\
                    DNARROW|VSLIDE|LFARROW|RTARROW|HSLIDE|SIZER|SMALLER,
                    X_MENU|X_HSPLIT|X_VSPLIT );
                break;
              case VIDEO:
                /* Check the _VDO cookie to find out which type of video */
                if( CJar( 0, _VDO_COOKIE, &cook ) == CJar_OK &&
                    (int)(cook>>16)==3 ) start_form( 7, FALCVID, NAME|MOVER, 0 );
                else start_form( 4, VIDOPTS, NAME|MOVER, 0 );
                break;
              case PERFLAGS:
                start_flags(1);
                break;
              case TMPFLAGS:
                start_flags(0);
                break;
              case SAVE:
                graf_mouse( BUSYBEE, 0L );
                x_shel_put( X_SHLOADSAVE, 0L );
                graf_mouse( ARROW, 0L );
                break;
              case RELOAD:
                graf_mouse( BUSYBEE, 0L );
                x_shel_get( X_SHLOADSAVE, 0, 0L );
                graf_mouse( ARROW, 0L );
                break;
              case TMSAVE:
                graf_mouse( BUSYBEE, 0L );
                save_settings();
                graf_mouse( ARROW, 0L );
                break;
            }
            menu_tnormal( menu, buf[3], 1 );
            break;
        }
        if( event&X_MU_DIALOG )         /* user clicked in a dialog window */
        {
          but = buf[2]&0x7fff;          /* isolate just the object # */
          if( buf[3]==main_hand )          /* main window */
            if( buf[2]&0x8000 )         /* double-click */
            {
              if( but ) opentask( main_hand, but );        /* on an item */
              else                      /* turn current off */
                  if( tasknum>0 ) tnum_onoff( main_hand, tasknum, 0 );
              tasknum=0;
              set_term_sleep();
            }
            else
            {
              if( but==tasknum || but<1 )       /* turn current off */
              {
                tnum_onoff( main_hand, tasknum, 0 );
                tasknum = -1;
              }
              else tasknum = but;
              but_up();
              set_term_sleep();
            }
          else use_form( buf[3], buf[2] );      /* process another dialog */
        }
        if( event&MU_TIMER && main_hand>0 && !iconified )  /* timer event */
        {
          wind_update( BEG_UPDATE );
          if( task_list() )                     /* has list changed? */
          {
            set_bl_height();
            if( tasknum>0 )                     /* turn current off */
                applist[tasknum].ob_state &= ~SELECTED;
            tasknum = 0;
            set_term_sleep();
            /* tell Geneva to redraw the whole dialog */
            wind_set( main_hand, X_WF_DIALOG, applist );
          }
          wind_update( END_UPDATE );
        }
      }
    }
    else if( _app ) alert( ALNOGEN );   /* Geneva not present */
  }
  else form_alert( 1, "[1][|TASKMAN.RSC not found!][Ok]" );
  /* An error occurred. If either running as an app or under Geneva or
     MultiTOS, get out */
  if( _app || _GemParBlk.global[1]==-1 ) quit();
  /* run as a desk accessory, but an error occurred, so just go into an
     infinite loop looking for AP_TERM messages */
  for(;;)
  {
    evnt_mesag(buf);
    if( buf[0]==AP_TERM ) quit();
  }
}

