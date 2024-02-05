#ifndef _WIN_VAR_
  #define _WIN_VAR_

#include "xwind.h"
#include "tos.h"
#include "aes.h"
#include "vdi.h"
#define Xrect(r) (r).x, (r).y, (r).w, (r).h

#define STKSIZE         40      /* stack for APP struct */
#define MAX_HANDLE      256     /* max no. of windows */
#define INFO_LEN        100     /* length of info bar string */
#define IS_TEAR         -23     /* magic number in info_pos meaning tear-away */
#define SHLINELEN 	120	/* max length of line in GENEVA.CNF */
#define MAX_KEYS	20	/* number of kbd equivs/dialog */
#define CNF_VER		0x100	/* oldset CNF I can read */

#define DTREECNT	1
#define TREECNT 	WSIZE+1

#define ICON_WID	72
#define ICON_HT		72

#define PATHLEN 120
#define TAILLEN 128

#define SH_RUNHELP	0x7101	/* run Help program */
#define SH_RUNSHELL	0x7102	/* run shell program */
#define SH_RUNSLEEP	0x7103	/* run program asleep */
#define SH_RUNACC	0x7104	/* run DA without open */
#define SH_REGHELP	0x7108	/* 005: I am the Help program; intentionally overlaps with SHW_ENVIRON */

#define ASL_PEXEC	4	/* parent asleep from Pexec */
#define ASL_USER	2	/* asleep by user */
#define ASL_SINGLE	1	/* asleep from singletasking app */

#define CJar_cookie     0x434A6172L     /* "CJar" */
#define CJar_xbios      0x434A          /* "CJ" */
#define CJar_OK         0x6172          /* "ar" */
#define CJar(mode,cookie,value)         xbios(CJar_xbios,mode,cookie,value)
#define _VDO_COOKIE 0x5F56444FL
#define MiNT_COOKIE 0x4d694e54L
#define SBL_COOKIE  0x4F53424CL
#define IDT_cookie      0x5F494454L     /* "_IDT" */

typedef struct { int x, y, w, h; } Rect;
typedef struct rect_list
{
  struct rect_list *next;
  Rect r;
} Rect_list;

typedef struct windoW
{
  struct windoW *next, *tear_parent;
  int handle, place, sleep_pl;
  Rect outer, working, full, prev;
  Rect_list *rects, *rectptr;
  char *top_bar, info_bar[INFO_LEN+1], info_end;
  int apid;
  int hslide,           /* also used for title # of a menu */
      vslide,           /* also for pull # */
      hslidesz, vslidesz,
      hslide2, vslide2,
      hslidesz2, vslidesz2;
  int hsplit, vsplit, hsp_min1, vsp_min1, hsp_min2, vsp_min2;
  int type, xtype, bevent;
  int info_pos;         /* may also have IS_TEAR */
  int min_w, min_h, max_w, max_h;
  OBJECT *tree;
  int treecnt, treeflag;
  OBJECT *menu, *dialog, *tool;
  int menu_tA, menu_tZ;
  int dial_obj, dial_edit, dial_edind,
      dial_swid, dial_sht;
  int colors[2][WGSIZE+1];	/* 004 */
  int cdecl (*objhand)( int hand, int obj );
  Rect iconify;
  int icon_index, old_type, old_xtype;
} Window;

typedef struct
{
        int             ob_next;        /* -> object's next sibling     */
        int             ob_head;        /* -> head of object's children */
        int             ob_tail;        /* -> tail of object's children */
        unsigned char   ob_typex;
        unsigned char   ob_type;        /* object type: BOX, CHAR,...   */
        unsigned int    ob_flags;       /* object flags                 */
        unsigned char   ob_statex;
        unsigned char   ob_state;       /* state: SELECTED, OPEN, ...   */
        OBSPEC          ob_spec;        /* "out": -> anything else      */
        int             ob_x;           /* upper left corner of object  */
        int             ob_y;           /* upper left corner of object  */
        int             ob_width;       /* object width                 */
        int             ob_height;      /* object height                */
} OBJECT2;

typedef struct
{
  int state, magic, effects;
  union
  {
    unsigned long l;
    bfobspec b;
  } color;
  unsigned int atari_col;
  char atari3d, atari_move;
} OBDESC;

typedef struct
{
  MENU menu;
  OBJECT2 *parent;
} INT_MENU;

typedef struct mdesc
{
  struct mdesc *next, *prev;
  void *addr;
  long len;
} MDESC;

typedef struct{ int handle, change, i1, i2, i3, i4; } SWT;

typedef struct App
{
  struct App *next, *prev, *child, *parent, *old_update;
  int type, clicks, mask, state, m1flags, m1x, m1y, m1w, m1h,
      m2flags, m2x, m2y, m2w, m2h, low, high;
  int event, mouse_x, mouse_y, mouse_b, mouse_k, key, times;
  int id, parent_id, *buf;
  int ap_type, ap_msgs, mint_id, apread_cnt, apread_id;
  APPFLAGS flags;
  BASPAG *basepage;
  OBJECT *menu, *desk;
  MFORM *mouse, *save_mouse;
  int mouse_on;
  int desk_obj;
  union
  {
    unsigned char c[2];
    unsigned int i;
  } update;
  long stack, mem_used;
  MDESC *mem_first;
  char dflt_acc[11], no_evnt, has_wind, start_end, asleep, waiting,
       neo_le005;	/* 007 */
  char path[PATHLEN], tail[TAILLEN], *env, *app_desc;
  struct App *ap_srch;
  int old_handles[7];
  int attaches;
  INT_MENU *menu_att;
  void *old_sigchild;	/* 005 */
  long vex[9];		/* 004 */
  int palette[16][3];	/* 004 */
  int blocked, was_blocked;		/* 004 */
  int dummy[STKSIZE/2];         /* should be near the end */
  int stktop;
} APP;

#define MAX_BUTQ 8
typedef struct
{
  int x, y, b, k;
  unsigned long time;
} BUTQ;

typedef struct Msg_q
{
  struct Msg_q *next;
  int app, len;
  void *buf;
  unsigned long tic;
} MSGQ;

typedef union
{
  long l;
  int i[2];
} PULL;

typedef struct Acc_list
{
  struct Acc_list *next, *next_sort;
  char *name;
  int apid, index;
  char is_acc, is_dflt;
} ACC_LIST;

typedef struct _appflags
{
  struct _appflags *next;
  APPFLAGS *flags;
} _APPFLAGS;

typedef union
{
  char c[2];
  unsigned int i;
} UPDATE;

typedef struct
{
  Rect r;
  OBJECT *tree, *old;
  long old_len;
  int pull, entry, up, down, first, last,
      sfirst, sprev,/* last non-scrollable obj */ stop, sbot;
  int itemoff, root, ret, downscroll;
  MENU *mdata, *in;
  Rect r2, r3;
  char done, plev;
} SUB_DAT;

#define X_PROPSTR	79
typedef struct
{
  char *str, *equiv;
  int eq_x;
} PROPSTR;

/* from neocommn.h */
#define LOAD_COOKIE 0x4e4c6472L          /* "NLdr" */
typedef struct
{
  int ver;
  void cdecl (*dopf)( char *a, char *b, ... );
  void cdecl (*bconws)( char *string );
  int *bad_media;
  
  char *top;
  int *outhand;
  char ***bufptr;         /* set the POINTER */
  int *count;		/* for backward compatibility */
  
  int *savcnt;
  char *scrsav, saveon, ampm, clockon, savmins, **kbshift;
  long (*pall)[];           /* use the POINTER */
  char *clock_cnt;
  
  long *open_wind;      /* set the POINTER to a function */
  
  char *t2table;
  void *mas;
  char *do_malloc;
  long *baspag, **pgmsize;
  char *inittab;
  
  void (**saver)( int onoff );  /* 1: on   0: off   other: interrupt */
  void (*shrink)( BASPAG *bp, long stack );

  int (**fselect)( int apid, char *path, char *file, int *button, char *title );
  
  void (**vclock)(void);
  void (*clock_on)( void vclock(void), long *ptr );
  void (*clock_off)(void);
  
  int *mdwa;
  char *ldpath,
      *clock_temp;	/* 0 if clock is temporarily off */
  int (*w_colors)[][2];
  char (*lock_drive)[4];

  long *pr_bufsiz, *pr_count;
  char **pr_bufmax, **pr_bufstart, **pr_buftail, *no_bcon;
  int pr_valid;
} LoadCookie;

/* contained in WINS.S */
extern IOREC *kbio;
extern char *kbbuf, *kbshift;
extern int kbsize;	/* can never be > 65535/4 */
extern char in_t1, in_t2, preempt, sleeping;
extern int emult_out[7];
extern int *curstack;
extern BASPAG **bp_addr;
extern void (*old_kbd)(), (*old_200)();
extern void (*old_t13)(), (*old_t1)(), (*old_t2)(), (*old_vdi)(), (*old_crit)(), (*old_term)();
extern void my_t1(), my_vdi(), my_t13(), my_term();
extern int my_butv(), my_motv(), my_timv(),
    (*old_butv)(), (*old_motv)(), (*old_timv)();
extern void new_exec();
extern long old_exec;
extern BASPAG *db_bp;
extern int *old_stack;
extern void (*old_t13_term)();
extern void term_t13();
extern long set_t2(), get_t2(), set_frame();
extern void my_kbd(), my_200();
extern int my_motv( int x, int y );
extern int (*t2atbl)(), (*t2xtbl)();
void start_magic( BASPAG *bp );
void acc_start(), magic_init(), pdisp_init(void);
int null_mouse( int x, int y );
void install_key( int remove );
void retrap_vdi(void);
long undo_linea(void);
void retrap_la(void);
void lock_mem(void);
void unlock_mem(void);
int redo_func( int type, int *parms, void *func );

#ifdef _APPLIC
  #define EXTERN
#else
  #define EXTERN extern
  extern SETTINGS settings, dflt_settings;
  extern OBJECT *fselind[], ascii_tbl[];
  extern _APPFLAGS *flags_list;
  extern APPFLAGS noflags, dflt_flags;
  extern OBJECT rs_object[];
  extern TEDINFO rs_tedinfo[];
  extern unsigned char alt[], alt_off[], alt_flag[];
  extern char lastapp_name[];
  extern char ind_move, ind_change, act_move, act_change;
  extern int alt_obj[], ind_col, act_col, bkgrnd_col, add3d_h, add3d_v;
  extern XFONTINFO fontinfo[9];
  extern int dwcolors[3][2][WGSIZE+1];
  extern int wstates[3][WGSIZE+1];
  extern G_VECTORS g_vectors;
  extern char fvalid[];
  extern KEYCODE pman_key;	/* 005 */
#endif

#ifndef _FSELECT
  extern OBJECT *fselind[], fselobj[];
#endif

EXTERN void (*clip_ini)( Rect *r );
EXTERN int (*clip_it)(void);
EXTERN APP *app0, *curapp, *has_menu, *has_desk, *has_mouse, *shell_app,
    *unloader, *has_update, *ddesk_app, *menu_owner, *form_app, *txt_app,
    *free_ap, *help_app, *mouse_last, *has_fmd_update, *last_curapp, *sw_app;
EXTERN char loadpath[PATHLEN];
EXTERN OBJECT *menu;
EXTERN Window *desktop, *next_menu, *cur_menu, *top_wind, *ascii_w, *menu_wind, *in_menu;
EXTERN Rect min;
EXTERN int next_obj, edit_idx, edit_obj;
EXTERN int fn_dir, fn_last;
EXTERN int dc_pause, dtree_cnt;
EXTERN int place;
EXTERN char sw_flag;
EXTERN char scrap_dir[PATHLEN];
EXTERN OBJECT *fsel, *pman;
EXTERN Rect fs_rect;
EXTERN int shgp_app, load_err;
EXTERN PULL pull_buf, pull_siz;
EXTERN int vdi_hand, vplanes, v_bpp, ptsiz, font_id, font_mode,
    cel_w, menu_h, cel_h, char_w, char_h, color_mode, wcolor_mode, desk_obj;
EXTERN unsigned long tic;
EXTERN OBJECT *dflt_wind, *dflt_desk, *dflt_desk0, *acc_tree;
EXTERN int ticcal, loop_id;
EXTERN int g_mx, g_mb, g_my;
EXTERN int kr, ks, br, mx, my, mb, m_obj;
EXTERN BUTQ but_q[MAX_BUTQ];
EXTERN int bq_ptr, bq_last, unclick;
EXTERN int oarray[14];
EXTERN int lastkey, lastsh;
EXTERN MFDB fdb0, fdb2;
EXTERN char loading, no_interrupt, has_mint, mint_preem;
EXTERN char no_top, multitask, draw_menu_alts, font_scalable, playmode, recmode;
EXTERN int cur_rez, new_rez, new_vid, unload, unload_type, apps_initial;
EXTERN BASPAG *curr_bp;
EXTERN int acc_tear;
EXTERN MSGQ *msg_q;
EXTERN ACC_LIST *acc_list;
EXTERN UPDATE update, cnt_update;
EXTERN unsigned long term_time;
EXTERN int num_keys;
EXTERN int lastapp_type, lastapp_id;
EXTERN char *environ;
EXTERN ANI_MOUSE *ani_mouse;
EXTERN int ani_frame, ani_delay;
EXTERN unsigned long farbtbl2[256];
EXTERN int farbtbl[256][32];
EXTERN DTA shel_dta;
EXTERN char sw_dflt[120];
EXTERN unsigned long idt_fmt;
EXTERN Rect clip_rect;
EXTERN void *playbuf, *recbuf, *recbuf0;
EXTERN int recnum, playnum, playrate;
EXTERN APP *recorder, *player;
EXTERN unsigned long rec_tic, play_tic, play_key;
EXTERN BASPAG magic_bp;
EXTERN int lock_curapp;		/* 005: int */
EXTERN APP *old_app;
EXTERN char **proc_name, rec_shift, best_malloc/*005*/;
EXTERN int *proc_id;
EXTERN KEYCODE rec_stop;
EXTERN int rec_mx, rec_my;
EXTERN int (*play_motv)( int x, int y );
EXTERN int (*play_curv)( int x, int y );
EXTERN MFORM *last_mouse;
EXTERN SUB_DAT sub_dat[5]; /*004: go to 5*/
EXTERN int in_sub;	/* 004: in_sub to int */

#endif
