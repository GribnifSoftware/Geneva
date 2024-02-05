/* version in GENEVA.CNF */
#define GNVAMAC_VER "2.0"

/* window type for the main window */
#define WIN_TYPE SMALLER|FULLER|NAME|MOVER|INFO|SIZER|CLOSER|UPARROW|DNARROW|VSLIDE

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

#define TYPE_TIMER	0
#define TYPE_BUTTN	1
#define TYPE_MOUSE	2
#define TYPE_KEY	3
#define TYPE_SOUND	0x7000
#define TYPE_DATE	0x7001
#define TYPE_TIME	0x7002
#define TYPE_MSG	0x7003

typedef struct
{
  int type;
  union
  {
    long timer;
    struct
    {
      int state, clicks;
    } button;
    struct
    {
      int x, y;
    } mouse;
    struct
    {
      int shift;
      unsigned char scan, ascii;
    } key;
    struct
    {
      int index, unused;
    } sound;
    char (*msg)[5][31];
  } u;
} MACBUF;

typedef struct
{
  char name[21];
  KEYCODE key;
  int len;
  int auto_on, automode;
  unsigned int date, time;	/* nothing before here can move! (load, chksum) */
  int obj;
  char kstr[11], afrom[5], adate[7], is_global;
  TEDINFO ted, fromted, dateted;
  MACBUF *mb;
} MACDESC;

typedef struct Editdesc
{
  struct Editdesc *next;
  int hand;
  int nobj, obj_rem;
  int last_on;
  OBJECT *o;
  MACDESC *m;
  int *ref;
  int refs, ref_rem;
  GRECT inner, outer;
  char info[60];
  TEDINFO *ted;
  int nted, ted_rem;
  char *str;
  int nstr, str_rem;
  OBJECT *icon;
  char iconified, fulled;
  GRECT full;
} EDITDESC;

#ifndef EXTERN
  #define EXTERN extern
  extern FORM form[1];
  extern int last_mobj,
             last_on,
             macsize,
             next_msg,
             d_sound, d_date, d_time;
  extern char etypes, aes_ok;
  extern KEYCODE start_end, in_read;
  extern GRECT wsize;
#else
  int last_mobj=1,
      last_on=-1,
      macsize = 2000,
      next_msg=-1,
      d_sound=0, d_date=5, d_time=5;
  char etypes=-1, aes_ok=1;
  KEYCODE start_end = { 0xF, 1, 0 },
          in_read = { 0xff, 0xff, 0 };
  GRECT wsize = { 50, 60, 250, 100 };
#endif

EXTERN EDITDESC *elist, *tim_edesc;
EXTERN GRECT winner, max;

EXTERN int apid,       /* my application ID */
    nmacs, num_macs, macs_rem,
    num_mobj, mobj_rem,
    char_w, char_h,
    panel_h, main_h,
    min_ht,                   /* minimum window height */
    min_wid,                  /* window width */
    main_hand,	/* window handle of main window */
    last_chk,
    iglobl,
    rec_pos;

EXTERN char iconified, /* true if the window is iconified */
     gma_path[120],
     main_path[120],
     auto_last[9],
     mlist_edit,
     maintitle[50],
     **minfo, **selected, **einfo, **evedit, **title,
     main_info[60],
     *date_fmt[8], *time_fmt[8],
     mac_err,
     gma_auto,
     is_global,
     edate_fmt[7],
     *months[12],
     **smonths;

EXTERN char (*msg_mac)[5][31], (*msg_ptr[10])[5][31];

EXTERN OBJECT *maclist,
       *menu,           /* points to the main window menu tree */
       *emenu,
       *date_pop, *time_pop, *mac_pop,
       *events;
       
EXTERN G_COOKIE *cookie;

EXTERN MACDESC *mdesc, *rec_mac;

EXTERN EDITDESC *rec_desc, *msg_edit;

EXTERN MACBUF *mac_buf;
EXTERN int mac_end, mac_count, msg_ind;

EXTERN long idt;

void close_sh(void);
int open_sh( char *name, int create );
char flush_sh( char noerr );
char _fwrite( char noerr, long len, void *addr );
char _fread( char noerr, int h, long len, void *addr );
int sh_getc( char *buf );
int get_shline( char *buf );
void set_if( OBJECT *tree, int num, int true );
void start_form( int fnum, int tnum, int type, int xtype );
void set_bl_height(void);
void get_inner(void);
void get_outer(void);
int free_macs( int root, int save_glob );
void free_most_macs(void);
void edits_on( char *state, OBJECT *o, int hand, int on );
void mac_off( int num, int draw );
void macs_off( int draw );
void edit_mac( int m );
int mac_list( int mode );
int add_tree( OBJECT **list, int *count, int *total, int *rem, OBJECT *add, int items, int h );
int set_mobj( MACDESC *m );
void main_dial(void);
void reset_maclist(void);
int add_mlist(void);
int shift_maclist( int num, int inc );
int alert( int num );
void *xmalloc( long size );
void xmfree( void *addr );
int xrealloc( void **start, long size );
int add_thing( void **start, int *total, int *remain, void *add,
    long size );
int add_obj( OBJECT **list, int *total, int *remain, OBJECT *o );
void close_wind( int *hand );
void do_help( char *topic );
void set_if( OBJECT *tree, int num, int true );
int selfunc( int func(int num), int backward );
int mo_exit( OBJECT *o, int num );
int mo_init( OBJECT *o );
void mo_touch( OBJECT *o, int num );
int ms_init( OBJECT *o );
void ms_touch( OBJECT *o, int num );
void ms_update(void);
int d_init( OBJECT *o );
void d_touch( OBJECT *o, int num );
int d_exit( OBJECT *o, int num );
int a_init( OBJECT *o );
void a_touch( OBJECT *o, int num );
int a_exit( OBJECT *o, int num );
int at_init( OBJECT *o );
int at_exit( OBJECT *o, int num );
int msg_init( OBJECT *o );
int msg_exit( OBJECT *o, int num );
void calc_bord( int type, int xtype, OBJECT *tree, GRECT *g );
void start_form( int fnum, int tnum, int type, int xtype );
void use_form( int hand, int num );
char *spathend( char *p );
int fselect( char *path, char *templ, int title );
void load_clip( EDITDESC *e );
int save_clip( EDITDESC *e );
void delete(void);
void edelete( EDITDESC *e );
int msaveclip( int num );
int load_gma( char *path, int nf );
int save_gma( char *path, int as );
int load_settings(void);
void save_settings(void);
void update_msg(void);
void get_etypes(void);
int get_mac_end( int *end );
void read_key( OBJECT *o, int num, KEYCODE *k, int hand, char *title, int ascii );
void key_str( OBJECT *o, int num, KEYCODE *key, int ascii );
void do_info(void);
void quit(void);
void main_sel( int dclick, int but, int keys );
EDITDESC *find_edesc( int hand );
void edit_sel( EDITDESC *e, int dclick, int but, int keys );
void but_up(void);
void gget_outer( GRECT *wsize, GRECT *winner, int h );
void gget_inner( GRECT *wsize, GRECT *winner, OBJECT *o );
void free_edits( int ac_close );
void close_edit( EDITDESC *e );
void arrow(void);
void bee(void);
void do_emenu( EDITDESC *e, int num );
int eselfunc( int func( EDITDESC *e, int num ), EDITDESC *e, int backward );
void evnts_off( EDITDESC *e, int draw );
void edit_dial( EDITDESC *e );
int shift_evnt( EDITDESC *e, int num, int inc, MACBUF *mb );
int evnt_list( EDITDESC *e, int mode );
void set_elist_ht( EDITDESC *e );
int evnt_tree( EDITDESC *e );
void finish_evnts( EDITDESC *e, int i, int len );
void new_mac( EDITDESC *e, int pos );
int set_ap( int id );
void build_mlist(void);
void do_einfo( EDITDESC *e );
void main_title(void);
int eedelete( EDITDESC *e, int num );
void end_record(void);
int do_save( int as );
int test_close(void);
int chksum(void);
void all_timers( EDITDESC *e, long val );
void check_rec(void);
void some_events( MACBUF *m, int total );
void cancel_rec(void);
int play_mac( MACDESC *m, int aes );
int keypress( int *key );
void spf_date( unsigned int dt, char *ptr, int yr );
void _strftime( char *buf, char *fmt, int lim, unsigned int dt, unsigned int time );
void updt_timepop(void);
void updt_datepop(void);
unsigned int do_popup( OBJECT *o, int obj, OBJECT *pop, unsigned int val );
long current_time(void);
void setup_mac( MACDESC *m );
void auto_play(void);
void set_global( MACDESC *m );
int test_eglob( EDITDESC *e );
int test_mglob( MACDESC *m );
void cmfree( void **p );
void free_msg( MACBUF *mb );
void free_msgs( MACDESC *m );
void msg_str( EDITDESC *e, int r, int draw );
long count_msgs( MACDESC *m );
int create_mac(void);
