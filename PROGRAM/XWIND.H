/* C declarations for Geneva. Copyright � 1994, Gribnif Software */

#ifndef __XWIND__
#define __XWIND__

#include <vdi.h>

#define GENEVA_COOKIE   0x476E7661      /* "Gnva" */
#define GENEVA_VER      0x0106          /* current Geneva version */

typedef struct
{
  int ver;
  char *process_name;
  int apid;
  int (**aes_funcs)();
  int (**xaes_funcs)();
  struct G_vectors *vectors;    /* rel 004 */
} G_COOKIE;

typedef struct G_vectors        /* rel 004 */
{
  int used;
  int (*keypress)( long *key );
  int (*app_switch)( char *process_name, int apid );
  int (*gen_event)(void);
} G_VECTORS;

/************************ appl_search **************************/
#define X_APS_CHILD0    0x7100
#define X_APS_CHILD     0x7101
#define X_APS_CHEXIT    -1

/********************* wind_get, wind_set **********************/
#define X_WF_MENU       0x1100
#define X_WF_DIALOG     0x1200
#define X_WF_DIALWID    0x1300
#define X_WF_DIALHT     0x1400
#define X_WF_DFLTDESK   0x1500          /* wind_set only */
#define X_WF_MINMAX     0x1600
#define X_WF_HSPLIT     0x1700
#define X_WF_VSPLIT     0x1800
#define X_WF_SPLMIN     0x1900
#define X_WF_HSLIDE2    0x1A00
#define X_WF_VSLIDE2    0x1B00
#define X_WF_HSLSIZE2   0x1C00
#define X_WF_VSLSIZE2   0x1D00
#define X_WF_DIALFLGS   0x1E00
  #define X_WD_ACTIVE   1       /* Mouse/keyboard events processed */
  #define X_WD_BLITSCRL 2       /* Use blit for realtime scroll */
  #define X_WD_KEYS     4       /* 004: Pass through unused keyclicks */
#define X_WF_OBJHAND    0x1F00
#define X_WF_DIALEDIT   0x2000
#define X_WF_DCOLSTAT   0x2100  /* rel 004 */

/******************* evnt_multi, evnt_mesag ********************/
#define X_MN_SELECTED   0xE000
#define X_WM_SELECTED   0xE100          /* Only if X_WTFL_CLICKS is off */
#define X_GET_HELP      0xE200
#define X_WM_HSPLIT     0xE400
#define X_WM_VSPLIT     0xE500
#define X_WM_ARROWED2   0xE600
#define X_WM_HSLID2     0xE700
#define X_WM_VSLID2     0xE800
#define X_WM_OBJECT     0xE900
#define X_WM_VECKEY     0xEA00          /* keypress vector event (rel 004) */
#define X_WM_VECSW      0xEA01          /* switch vector event (rel 004) */
#define X_WM_VECEVNT    0xEA02          /* user-defined event (rel 004) */
#define X_WM_RECSTOP    0xEA10          /* x_appl_trecord finished */
#define X_MU_DIALOG     0x4000          /* evnt_multi (only) type */

/************************** objc_draw **************************/
#define X_MAGMASK       0xF000 /* ob_state: Mask for X_MAGIC                  */
#define X_MAGIC         0x9000 /*           Must be set this way              */
#define X_PREFER        0x0040 /*           User-defined fill                 */
#define X_DRAW3D        0x0080 /*           3D                                */
#define X_ROUNDED       0x0100 /*           Rounded                           */
#define X_KBD_EQUIV     0x0200 /*           Scan for ['s; Root: no auto equivs*/
#define X_SMALLTEXT     0x0400 /*           Small font                        */
#define X_SHADOWTEXT    0x0800 /*           Shadowed text (rel 004)           */
#define X_BOLD          0x4000 /* ob_flags: With X_MAGIC, bold text           */
#define X_ITALICS       0x8000 /*           With X_MAGIC, italic text         */
/* Extended ob_types */
#define X_MOVER         17     /* Dialog mover box                            */
#define X_RADCHKUND     18     /* Radio/check/Undo                            */
#define X_UNDERLINE     19     /* Title (G_STRING)                            */
#define X_GROUP         20     /* Group (G_BUTTON)                            */
#define X_HELP          21     /* Activated with Help key                     */
#define X_UNDO          31     /* Activated with Undo key                     */
#define X_USRDEFPRE     90     /* With X_MAGIC, call USERBLK before drawing   */
#define X_USRDEFPOST    91     /* With X_MAGIC, call USERBLK after drawing    */
#define X_GRAYMENU      92     /* root object: draw DISABLED G_STRING children in gray (rel 004) */

/************************** form_dial **************************/
#define X_FMD_START     1000    /* Save area under dialog    */
#define X_FMD_FINISH    1003    /* Restore area under dialog */

/********************** form_do/objc_edit **********************/
/* Value for long edits into TEDINFO->te_tmplen */
#define X_LONGEDIT      -71     /* rel 004 */

/************************** graf_mouse *************************/
#define X_LFTRT         8       /* Left-right arrow */
#define X_UPDOWN        9       /* Up-down arrow    */
#define X_MRESET        1000    /* Reset to on once */
#define X_MGET          1001    /* Get mouse shape  */

typedef struct
{
  int frames, delay;
  MFORM form[32];
} ANI_MOUSE;
#define X_SET_SHAPE     1100    /* Add to mouse shape index to change shape */

/************************ rsrc_load *****************************/
typedef struct rshdr2
{
        unsigned int    rsh_vrsn;       /* Should be 3 */
        unsigned int    rsh_extvrsn;    /* Initialized to "IN" for Interface */
        unsigned long   rsh_object;
        unsigned long   rsh_tedinfo;
        unsigned long   rsh_iconblk;
        unsigned long   rsh_bitblk;
        unsigned long   rsh_frstr;
        unsigned long   rsh_string;
        unsigned long   rsh_imdata;     /* Image data */
        unsigned long   rsh_frimg;
        unsigned long   rsh_trindex;
        unsigned long   rsh_nobs;       /* Counts of various structs */
        unsigned long   rsh_ntree;
        unsigned long   rsh_nted;
        unsigned long   rsh_nib;
        unsigned long   rsh_nbb;
        unsigned long   rsh_nstring;
        unsigned long   rsh_nimages;
        unsigned long   rsh_rssize;     /* Total bytes in resource */
} RSHDR2;

#define X_LONGRSC       0x494EL /* "IN" */

/************************** shel_write *************************/
#define XSHW_RUNANY     0x00E0  /* rel 004 */
#define XSHW_RUNAPP     0x00E1  /* rel 004 */
#define XSHW_RUNACC     0x00E3  /* rel 004 */

#define XSHD_FLAGS      (1<<15) /* APFLG in long[9] of SHWRCMD */

/************************** x_settings *************************/
#define SET_VER         0x0106   /* the last time SETTINGS changed */
typedef struct
{
  unsigned char shift, scan, ascii;
} KEYCODE;
typedef union
{
  struct
  {
    unsigned outlined   :1;
    unsigned shadowed   :1;
    unsigned draw_3D    :1;
    unsigned rounded    :1;
    unsigned atari_3D   :1;     /* rel 004 */
    unsigned shadow_text:1;     /* rel 004 */
    unsigned bold_shadow:1;     /* rel 004 */
    unsigned reserved   :9;
    unsigned framecol   :4;
    unsigned textcol    :4;
    unsigned textmode   :1;
    unsigned fillpattern:3;
    unsigned interiorcol:4;
  } s;
  unsigned long l;
} OB_PREFER;
typedef struct Settings
{
  int version;
  int struct_len;
  int boot_rez, falcon_rez;
  union
  {
    struct
    {
      unsigned pulldown          :1;
      unsigned insert_mode       :1;
      unsigned long_titles       :1;
      unsigned alerts_under_mouse:1;
      unsigned fsel_1col         :1;
      unsigned grow_shrink       :1;
      unsigned tear_aways_topped :1;
      unsigned auto_update_shell :1;
      unsigned alert_mode_change :1;
      unsigned ignore_video_mode :1;
      unsigned no_alt_modal_equiv:1;    /* rel 004 */
      unsigned no_alt_modeless_eq:1;    /* rel 004 */
      unsigned preserve_palette  :1;    /* rel 004 */
      unsigned mouse_on_off      :1;    /* rel 004 */
      unsigned top_all_at_once   :1;    /* rel 005 */
      unsigned child_pexec_single:1;    /* rel 006: undocumented */
    } s;
    unsigned int i;
  } flags;
  int gadget_pause;             /* 50 Hz timer tics */
  KEYCODE menu_start, app_switch, app_sleep, ascii_table, redraw_all,
      wind_keys[13];
  OB_PREFER color_3D[4], color_root[4], color_exit[4], color_other[4];
  char sort_type, find_file[26], fsel_path[10][35], fsel_ext[10][6];
  KEYCODE cycle_in_app,		/* rel 004 */
          iconify,		/* rel 004 */
          alliconify,		/* rel 004 */
          procman,		/* rel 006 */
          unused[4];
  char graymenu, reserved;	/* rel 004 */
  union				/* rel 006 */
  {
    struct
    {
      unsigned procman_details  :1;
      unsigned reserved         :31;
    } s;
    unsigned long l;
  } flags2;
} SETTINGS;

#define XS_UPPAGE WA_UPPAGE
#define XS_DNPAGE WA_DNPAGE
#define XS_UPLINE WA_UPLINE
#define XS_DNLINE WA_DNLINE
#define XS_LFPAGE WA_LFPAGE
#define XS_RTPAGE WA_RTPAGE
#define XS_LFLINE WA_LFLINE
#define XS_RTLINE WA_RTLINE
#define XS_CLOSE  8
#define XS_CYCLE  9
#define XS_FULL   10
#define XS_LFINFO 11
#define XS_RTINFO 12

int x_settings( int getset, int length, SETTINGS *user );

/************************ x_shel_get/put ************************/
#define X_SHLOADSAVE    -1      /* Load/save SETTINGS */
#define X_SHOPEN        0       /* Start read/write   */
#define X_SHACCESS      1       /* Read/write         */
#define X_SHCLOSE       2       /* Close              */

int x_shel_get( int mode, int length, char *buf );
int x_shel_put( int mode, char *buf );

/***************** x_wind_create, x_wind_calc *******************/
#define X_MENU          0x0001
#define X_HSPLIT        0x0002
#define X_VSPLIT        0x0004

int x_wind_create( int kind, int xkind, int wx, int wy, int ww,
    int wh );
int x_wind_calc( int type, int kind, int xkind, int inx, int iny,
    int inw, int inh, int *outx, int *outy, int *outw, int *outh );

/************************** x_wind_tree *************************/
typedef struct WindTree
{
  int handle, count, flag;
  OBJECT *tree;
} WIND_TREE;

#define X_WT_GETCNT     0       /* Get count and flag */
#define X_WT_READ       1       /* Copy window tree   */
#define X_WT_SET        2       /* Set new tree       */

#define X_WTFL_RESIZE   1       /* Flags bit 0: Auto resize                  */
#define X_WTFL_CLICKS   2       /*           1: Process clicks               */
#define X_WTFL_SLIDERS  4       /*           2: Resize sliders, info         */

/* window gadgets */
#define WGCLOSE   1   /* BOXCHAR */
#define WGMOVE    2   /* BOXTEXT */
#define WGICONIZ  3   /* BOXCHAR */
#define WGBACK    4   /* BOXCHAR */
#define WGFULL    5   /* BOXCHAR */
#define WGILEFT   6   /* BOXCHAR */
#define WGINFO    7   /* BOXTEXT */
#define WGIRT     8   /* BOXCHAR */
#define WGTOOLBOX 9   /* IBOX */
#define WGMNLEFT  10  /* BOXCHAR */
#define WGMENU    11  /* BOX */
#define WGMNRT    12  /* BOXCHAR */
#define WGUP      13  /* BOXCHAR */
#define WGVBIGSL  14  /* BOX */
#define WGVSMLSL  15  /* BOX */
#define WGDOWN    16  /* BOXCHAR */
#define WGVSPLIT  17  /* BOX */
#define WGUP2     18  /* BOXCHAR */
#define WGVBIGSL2 19  /* BOX */
#define WGVSMLSL2 20  /* BOX */
#define WGDOWN2   21  /* BOXCHAR */
#define WGLEFT    22  /* BOXCHAR */
#define WGHBIGSL  23  /* BOX */
#define WGHSMLSL  24  /* BOX */
#define WGRT      25  /* BOXCHAR */
#define WGHSPLIT  26  /* BOX */
#define WGLEFT2   27  /* BOXCHAR */
#define WGHBIGSL2 28  /* BOX */
#define WGHSMLSL2 29  /* BOX */
#define WGRT2     30  /* BOXCHAR */
#define WGSIZE    31  /* BOXCHAR */

int x_wind_tree( int mode, WIND_TREE *wt );

/************************* x_appl_flags *************************/
typedef union
{
  struct
  {
    unsigned multitask    :1;
    unsigned special_types:1;
    unsigned round_buttons:1;
    unsigned kbd_equivs   :1;
    unsigned undo_equivs  :1;
    unsigned off_left     :1;
    unsigned exit_redraw  :1;
    unsigned AES40_msgs   :1;
    unsigned limit_handles:1;
    unsigned limit_memory :1;
    unsigned keep_deskmenu:1;
    unsigned clear_memory :1;
    unsigned maximize_wind:1;
    unsigned optim_redraws:1;   /* rel 004 */
    unsigned unused       :2;   /* Reserved for future use */
    unsigned mem_limit    :16;  /* Kb to limit memory allocation */
  } s;
  unsigned long l;
} APFLG;

typedef struct
{
  char name[13];
  char desc[17];
  APFLG flags;
  KEYCODE open_key, reserve_key[3];
} APPFLAGS;

#define X_APF_GET_INDEX 0
#define X_APF_SET_INDEX 1
#define X_APF_DEL_INDEX 2
#define X_APF_GET_ID    3
#define X_APF_SET_ID    4
#define X_APF_SEARCH    5       /* rel 004 */

int x_appl_flags( int getset, int index, APPFLAGS *flags );

/*********************** x_appl_font ****************************/
typedef struct
{
  int font_id;
  int point_size;
  int gadget_wid;
  int gadget_ht;
} XFONTINFO;

int x_appl_font( int getset, int zero, XFONTINFO *info );

/********************** Miscellaneous ***************************/
void appl_yield(void);
int  x_appl_term( int apid, int retrn, int set_me );
int  x_appl_trecord( void *mem, int count, KEYCODE *cancel, int mode );
int  x_appl_tplay( void *mem, int num, int scale, int mode );
int  x_appl_sleep( int id, int sleep );
int  x_form_center( OBJECT *tree, int *cx, int *cy, int *cw, int *ch );
int  x_form_error( char *fmt, int num );
int  x_form_filename( OBJECT *tree, int obj, int to_from, char *string );
int  x_form_mouse( OBJECT *tree, int mouse_x, int mouse_y, int clicks,
     int *edit_obj, int *next_obj, int *edit_idx );
int  x_fsel_input( char *inpath, int pathlen, char *insel, int sels,
     int *exbutton, char *label );
int  x_graf_blit( GRECT *r1, GRECT *r2 );
int  x_graf_rubberbox( GRECT *area, GRECT *outer, int minwidth,
     int minheight, int maxwidth, int maxheight, int snap, int lag );
void x_graf_rast2rez( unsigned int *src_data, long plane_len,
     int old_planes, MFDB *mfdb, int devspef );         /* rel 004 */
int  x_help( char *topic, char *helpfile, int sensitive );
void x_malloc( void **addr, long size );                /* rel 004 */
int  x_mfree( void *addr );                             /* rel 004 */
int  x_mshrink( void *addr, long new );                 /* rel 004 */
int  x_realloc( void **addr, long size );               /* rel 004 */
int  x_objc_edit( OBJECT *tree, int edit_obj, int key_press,
     int shift_state, int *edit_idx, int mode );
int  x_scrp_get( char *out, int delete );               /* rel 004 */
void x_sprintf( char *buf, char *fmt, ... );
int  x_sscanf( char *buf, char *fmt, ... );
int  x_wdial_draw( int handle, int start, int depth );
int  x_wdial_change( int handle, int object, int newstate );

#endif
