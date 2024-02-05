/*#define MINT_DEBUG*/

#include "stdio.h"
#include "new_aes.h"
#include "xwind.h"
#include "stdlib.h"
#include "string.h"
#include "tos.h"
#include "ierrno.h"
#include "vdi.h"
#include "linea.h"
#include "multevnt.h"
#define _WINDOWS
#include "win_var.h"
#include "win_inc.h"
#include "windows.h"
#include "fsel.h"
#include "debugger.h"

/* for now, the wind_set( X_WF_DIALFLGS ) map to bits in treeflag */
#define X_WTFL_ACTIVE	8
#define X_WTFL_BLITSCRL	16
#define X_WTFL_KEYS     32
#define X_WTFL_MAXIMIZE 64

#define UPDT_SEM 0x476E5300L

void test_msg( APP *ap, char *msg );

Rect norect = { 0, 0, 0, 0 };

unsigned int handles[MAX_HANDLE/16];
int last_gadget, last_gad_m, last_gad_w, gadget_ok, max_redraw=1;
int nobuf[8];
int dc_rate;
unsigned long last_tic, gadget_tic, ngad_tic;
int new_un, new_ptr, cur_last, not_taken, old_un;
char no_set, repeat_func;
void *last_func;
APP *wind_app;
Rect wclip_rec, wclip_big;
Window *wclip_win;
static int last_handle = -1;	/* 004: global */
int get_window( int handle, int type, int *i );
void dial_sliders( Window *w, int draw );
char wait_curapp, block_loop;

typedef struct Rect_arr
{
  struct Rect_arr *next;
  unsigned int map;
} RECT_ARR;

static RECT_ARR *rect_arr;

int test_update( void *func )
{
  extern char block_app;

  if( !in_menu && (!update.i || curapp==has_update) || func!=last_func ) return 0;
  curapp->blocked = block_app = 1;
#ifdef MINT_DEBUG
  test_msg( curapp, "blocked" );
#endif
  return 1;
}

MSG pmsg;

void unblock_loop(void)
{
  if( block_loop<0 )
  {
    block_loop = 0;
    if( preempt ) Pmsg( 0x8001, 0x476EFFFEL, &pmsg );	/* 005 */
  }
  else block_loop = 0;	/* intentionally redundant */
}

void unblock(void)
{
  APP *ap;

  for( ap=app0; ap; ap=ap->next )
    if( ap->blocked && !ap->asleep )
    {
#ifdef MINT_DEBUG
      test_msg( ap, "unblocking" );
#endif
      ap->blocked = 0;
      ap->was_blocked = 1;
      if( preempt ) Pmsg( 0x8001, 0x476E0000L|ap->id, &pmsg );
    }
  unblock_loop();
}

void close_del( Window *w )
{
  APP *ap=curapp, *ap2;	/* 006 */

  wind_app = 0L;		/* don't consider old windows */
  if( (ap2 = find_ap(w->apid)) != 0 ) curapp = ap2;	/* 006 */
  /* w->apid = curapp->id;         /* so that no error occurs */ /* 006: now set curapp instead */
  if( w->place > 0 ) close_window( w->handle );
  delete_window( w->handle );
  curapp = ap;		/* 006 */
}

void _wind_new(void)
{
  Window *w, *w2;
  APP *ap;

  update.i = cnt_update.i = 0;
  has_update = 0L;
  for( ap=app0; ap; ap=ap->next )
  {
    if( !multitask ) ap->has_wind = 0;
/*%    ap->update.i = 0;
    ap->old_update = 0L; */
  }
  for( w2=desktop->next; (w=w2)!=0; )
  {
    w2 = w->next;
    if( w->apid==curapp->id ) close_del(w);
  }
  if( !multitask )
  {
    no_top = 0;
    has_desk = app0;
    top_wind = desktop;
    desktop->tree = dflt_desk;
    desktop->treecnt = dtree_cnt;
    desktop->menu = 0L;
    desk_obj = 0;
  }
}

APP *find_ap( int id )
{
  APP *ap;

  for( ap=app0; ap && ap->id!=id; ap=ap->next );
  return ap;
}

APP *key_owner(void)
{
  if( update.i ) return has_update;
  else if( no_top || top_wind==ascii_w || has_menu && top_wind==desktop )
      return has_menu;
  else return find_ap( top_wind->apid );
}

int owns_key( APP *ap )
{
  if( !update.i )
    if( no_top || top_wind==ascii_w || has_menu && top_wind==desktop )
        return ap==has_menu;
    else return ap->id==top_wind->apid;
  else return ap==has_update;
}

int _is_key( KEYCODE *k, unsigned char shift, unsigned int key )
{
  if( shift&3 ) shift |= 3;     /* 1 shift key becomes both */
  if( k->shift!=shift ) return 0;
  if( k->ascii ) return (unsigned char)key == k->ascii;
  return (unsigned char)(key>>8) == k->scan;
}

int is_key( KEYCODE *k, unsigned char shift, unsigned int key )
{
  APP *ap;
  int i;

  if( _is_key( k, shift, key ) )
  {
    if( (ap=key_owner()) != 0 )
      for( i=0; i<3; i++ )
        if( _is_key( &ap->flags.reserve_key[i], shift, key ) ) return 0;
    return 1;
  }
  return 0;
}

int find_next_handle(void)
{
  int i;

  for( i=0; i<MAX_HANDLE; i++ )
    if( !(handles[i>>4]&(1<<(i&0xf))) ) return(i+1);
  return(-1);
}

void set_handle( int hand, int val )
{
  int i, j;

  i = (--hand)>>4;
  j = 1<<(hand&0xf);
  if( val ) handles[i] |= j;
  else handles[i] &= ~j;
}

void set_dc( int r )
{
  static int clicks[]={ 113*5, 83*5, 69*5, 55*5, 42*5 };

  if( r>=0 && r<=4 ) dc_pause = clicks[dc_rate=r]/ticcal;
}

int evnt_dclick( int newval, int getset )
{
  int ret;

  ret = dc_rate;
  if( getset ) set_dc( newval );
  return(ret);
}

void newtop( int type, int hand, int id )
{
  int buf[8];

  if( id>=0 )
  {
    buf[0] = type;
    buf[2] = 0;
    buf[3] = hand;
    _appl_write( buf[1]=id, 16, buf, 1 );
  }
}

void no_memory(void)
{
  form_error( IENSMEM );
}

void draw_desk(void)
{
  redraw_window( 0, &desktop->working, 0, 1 );
}

int conv_handle( int handle, int is_old )
{
  int i;

  if( !wind_app || !handle || !wind_app->flags.flags.s.limit_handles ) return handle;
  if( is_old ) return handle>=1 && handle<=7 ? wind_app->old_handles[handle-1] : -1;
  for( i=0; i<7; i++ )
    if( wind_app->old_handles[i]==handle ) return i+1;
  return 0;	/* definitely not one of mine */
}

Window *find_window( int handle )
{
  Window *wind;

  wind=desktop;
  do
    if( wind->handle == handle ) return(wind);
  while( (wind=wind->next) != 0 );
  return(0L);
}


void get_min( Window *wind )
{
  int type=wind->type, xtype=wind->xtype, w=cel_w-1, h=cel_h-1, has_v, has_h;
  Rect r = { 0, 0, 0, 0 }, out;

  /* min inner size, considering just scroll bars first */
  has_v = type & (VSLIDE|UPARROW|DNARROW) || type&SIZER && xtype&X_HSPLIT;
  has_h = type & (HSLIDE|LFARROW|RTARROW) || type&SIZER && xtype&X_VSPLIT;
  if( !(wind->treeflag&X_WTFL_MAXIMIZE) )
  {
    if( type&SIZER || has_h || has_v )
    {
      r.w += w;
      r.h += h;
    }
  }
  else if( type&SIZER )
    if( !has_v && has_h ) r.w += w;
    else if( has_v && !has_h ) r.h += h;
  if( (type&RTARROW) || type&(NAME|MOVER|CLOSER|FULLER|SMALLER) && type&FULLER ) r.w += w;
  if( type & (CLOSER|LFARROW) ) r.w += w;
  if( type & HSLIDE ) r.w += w;
  if( type & UPARROW ) r.h += h;
  if( type & VSLIDE ) r.h += h;
  if( type & DNARROW ) r.h += h;
  if( xtype & X_VSPLIT )
  {
    wind->vsp_min1 = wind->vsp_min2 = r.h;
    r.h += dflt_wind[WVSPLIT].ob_height-1;
  }
  if( xtype & X_HSPLIT )
  {
    wind->hsp_min1 = wind->hsp_min2 = r.w;
    r.w += dflt_wind[WHSPLIT].ob_width-1;
  }
  recalc_outer( r, &out, type, xtype );
  wind->min_w = out.w;
  wind->min_h = out.h;
}

int check_split( int *split, int val, int min1, int min2, int max, int siz )
{
  int m1, m2;

  m1 = min1>>1;
  m2 = min2>>1;
  if( val!=-1 )
    if( max-siz+2 <= min1+min2 )
    {
      if( val ) val = -1;	/* don't change if already at 0 */
    }
    else if( val < m1 ) val = 0;
    else if( val < min1 ) val = min1;
    else if( val > max-siz-m2+2 ) val = -1;
    else if( val > max-siz-min2+2 ) val = max-siz-min2+2;
  if( val != *split )
  {
    *split = val;
    return 1;
  }
  return 0;
}

int both_splits( Window *wind )
{
  /* use logical OR, so both happen */
  return check_split( &wind->hsplit, wind->hsplit, wind->hsp_min1,
      wind->hsp_min2, wind->working.w, wind->tree[WHSPLIT].ob_width ) |
      check_split( &wind->vsplit, wind->vsplit, wind->vsp_min1,
      wind->vsp_min2, wind->working.h, wind->tree[WVSPLIT].ob_height );
}

void iconify_it( Window *wind, int set )	/* 005 */
{
  int next_iconify( int *out );

  if( wind->icon_index ) return;
  wind->icon_index = next_iconify( set ? (int *)&wind->working : 0L );
  wind->old_type = wind->type;
  wind->type = NAME|MOVER;
  wind->old_xtype = wind->xtype;
  wind->xtype = 0;
  wind->iconify = wind->outer;
  if( set ) recalc_outer( wind->working, &wind->outer, NAME|MOVER, 0 );
}

int create_window( int type, int xtype, Rect *r )
{
  Window *wind, *wnew;
  int h, *old=0L;

  wind = desktop;
  while( wind->next ) wind = wind->next;
  if( wind_app && wind_app->flags.flags.s.limit_handles )
  {
    for( old=wind_app->old_handles, h=0; *old && ++h<=7; old++ );
    if( h>7 ) return -1;
  }
  if( (h = find_next_handle()) >= 0 )
    if( (wnew = (Window *)lalloc( sizeof(Window), -1 )) == 0 )
    {
      no_memory();
      return(-1);
    }
    else
    {
      if( old ) *old = h;
      memset( wnew, 0, sizeof(Window) );
      wind->next = wnew;
      set_handle( wnew->handle=h, 1 );
      wnew->tree = dflt_wind;
      wnew->treecnt = TREECNT;
      wnew->apid = curapp->id;
      if( *(long *)r == -1L && *((long *)r+1) == -1L )	/* 005 */
      {
        iconify_it( wnew, 1 );
        wnew->full = wnew->outer;
      }
      else wnew->outer = wnew->full = *r;
      /* wnew->dirty = norect;	004 */
      wnew->prev = wnew->outer;
      wnew->type = type;
      wnew->xtype = xtype;
      wnew->top_bar = (char *) "";
      wnew->place = wnew->hslide = wnew->hslidesz = wnew->vslide =
          wnew->vslidesz = wnew->treeflag = wnew->hsplit = wnew->vsplit = -1;
      if( !curapp->flags.flags.s.maximize_wind )
          wnew->treeflag &= ~X_WTFL_MAXIMIZE;
      wnew->dial_swid = wnew->dial_sht = 1;
      memcpy( wnew->colors, dwcolors[wcolor_mode], sizeof(wnew->colors) );
      recalc_window( wnew->handle, wnew, -1L );
      recalc_inner( wnew->outer, &wnew->working, type, xtype );
      get_min( wnew );
      both_splits(wnew);
      wnew->max_w = desktop->outer.w;
      wnew->max_h = desktop->outer.h;
    }
  else DEBUGGER(WNCREATE,NOHAND,MAX_HANDLE);
  return conv_handle( h, 0 );  /* -1 if none */
}

void do_wclip(void)
{
  _vs_clip( 1, &wclip_rec );
}

#pragma warn -par
void wded_clip_ini( Rect *r )
{
  _v_mouse(0);	/* 004 */
}
#pragma warn +par

int wded_clip(void)
{
  if( wclip_rec.w )
  {
    do_wclip();
    wclip_rec.w = 0;
    return 1;
  }
  return 0;
}

void wd_edit( Window *w, Rect *r )
{
  if( w && w->dialog && w->place == place && w->treeflag&X_WTFL_ACTIVE )
  {
    if( r )
    {
      wclip_rec = *r;
      clip_ini = wded_clip_ini;
      clip_it = wded_clip;
    }
    if( w->dial_edit>0 ) objc_edit( w->dialog, w->dial_edit, 0, &w->dial_edind, ED_END );
    else if( w->dial_edit<0 && w->dial_obj != 0 )
    {
      objc_edit( w->dialog, w->dial_edit=w->dial_obj, 0, &w->dial_edind, ED_INIT );
      w->dial_obj = 0;
    }
    if( r ) reset_clip();
  }
}

/*typedef struct{ int handle; Rect r; } OWT;*/
int open_window( OWT *t )
{
  Window *wind;
  int handle;

  if( test_update( (void *)open_window ) ) return 0;
  handle = conv_handle( t->handle, 1 );
  if( (wind = find_window(handle)) == 0 )
  {
    DEBUGGER(WINDOP,INVHAND,handle);
    return 0;
  }
  if( wind==desktop )
  {
    DEBUGGER(WINDOP,OPDESK,0);
    return 0;
  }
  if( wind->place >= 0 )
  {
    DEBUGGER(WINDOP,ALROPEN,handle);
    return(0);
  }
  if( wind->apid>=0 && wind->apid != curapp->id && wind->dial_obj != IS_TEAR )
  {
    DEBUGGER(WINDOP,NOTOWN,handle);
    return(0);
  }
  if( *(long *)&t->r == -1L && *((long *)&t->r+1) == -1L )	/* 005 */
      iconify_it( wind, 1 );
  else wind->outer = t->r;
  opn_wind(wind);
  return(1);
}

void free_all_rects( Window *wind )
{
  Rect w1, w2, r;
  Window *w;

  *(long *)&w1.x = *(long *)&wind->outer.x;
  w1.w = wind->outer.w+2;
  w1.h = wind->outer.h+2;
  for( w=desktop; w; w=w->next )
    if( w->place < wind->place )
    {
      *(long *)&w2.x = *(long *)&w->outer.x;
      w2.w = w->outer.w+2;
      w2.h = w->outer.h+2;
      if( intersect( w1, w2, &r ) ) free_rects( w, 1 );
    }
}

void opn_wind( Window *wind )
{
  Window *top, *otop;
  APP *ap;

  top = top_wind;
  otop = top && !no_top && top->handle ? top : 0L;	/* 004: put it off */
  no_top = 0;
  top_wind = wind;
  wd_edit(top,0L);
  wind->place = ++place;
  if( (ap=find_ap(wind->apid)) != 0 )
  {
    ap->has_wind = 1;
    if( ap->menu ) switch_menu(ap);
    else switch_mouse( ap, 0 );
  }
  free_all_rects( wind );	/* 004 */
  all_gadgets( place-1 );
  recalc_window( wind->handle, wind, -1L );
  recalc_inner( wind->outer, &wind->working, wind->type, wind->xtype );
  dial_sliders( wind, 0 );
  redraw_window( wind->handle, &desktop->working, 0, 1 );
  if( otop ) newtop( WM_UNTOPPED, otop->handle, otop->apid );	/* 004: do it now */
}

/*void msg_redraw( Window *w, int *buf )
{
  buf[0] = WM_REDRAW;
  buf[1] = w->apid;
  buf[2] = 0;
  buf[3] = w->handle;
  *(Rect *)&buf[4] = w->dirty;
  w->dirty = norect;
}  004 */

void close_tears( Window *wind )
{
  Window *w2, *w;

  for( w=desktop->next; w; )
    if( w->tear_parent==wind )
    {
      w2 = w;
      w = w->next;
      close_del(w2);
    }
    else w = w->next;
}

int close_window( int handle )
{
  Window *wind, *w, *top=0L;
  int pl, was_top;
  APP *ap, *ap2;

  if( test_update( (void *)close_window ) ) return 0;
  handle = conv_handle( handle, 1 );
  if( (wind = find_window(handle)) == 0 )
  {
    DEBUGGER(WNCLOSE,INVHAND,handle);
    return 0;
  }
  if( wind==desktop )
  {
    DEBUGGER(WNCLOSE,CLDESK,0);
    return 0;
  }
  if( (pl=wind->place) < 0 )
  {
    DEBUGGER(WNCLOSE,NOTOPEN,handle);
    return(0);
  }
  if( wind->apid>=0 && wind->apid != curapp->id )
  {
    DEBUGGER(WNCLOSE,NOTOWN,handle);
    return(0);
  }
  free_rects( wind, 1 );
  ap2 = curapp;         /* only current app can own window */
  free_all_rects( wind );	/* 004 */
  wind->place = -1;     /* so that redraw_wind() in appl_write()
                           will fail below */
  if( wind->apid<0 && wind->hslide<0 ) acc_tear--;        /* ACC list tear-away */
/*  if( wind->dirty.w )  004
  {                      MultiDesk compatibility
    int buf[16];
    msg_redraw( wind, buf );
    _appl_write( wind->apid, 16, buf, 0 );
  } */
  w = desktop;
  ap2->has_wind=0;
  was_top = top_wind==wind;
  do
    if( w!=wind )
    {
      if( was_top && w->place == place-1 )
      {
        top = w;
        if( (ap = find_ap(w->apid)) != 0 )
          if( (no_top = ap->menu && has_menu != ap) != 0 )  /* 004 */
          {
            if( has_menu->id != wind->apid ) switch_mouse(has_menu,0);
          }
          else
          {
            if( w->place>0 ) newtop( WM_ONTOP, w->handle, ap->id );
            if( w->place>=0/*004*/ && ap->id != wind->apid ) switch_mouse(ap,0);
          }
      }
      if( ap2->id == w->apid ) ap2->has_wind=1;
      if( w->place > pl ) w->place--;   /* above window being closed */
      else redraw_window( w->handle, &wind->outer, 1, 1 );
    }
  while( (w = w->next) != 0 );
  if( --place < 0 ) place = 0;	/* just in case; should never happen */
  if( was_top )
  {
    if( !no_top ) all_gadgets( place );
    if( top ) wd_edit(top_wind=top,0L);
    else top_wind = desktop;
  }
  close_tears(wind);
  return(1);
}

void redraw_all( Rect *r )
{
  Window *w;

  w = desktop;
  do
    redraw_window( w->handle, r, 1, 1 );
  while( (w = w->next) != 0 );
}

int delete_window( int handle )
{
  Window *wind, *w;
  int h;

  handle = conv_handle( handle, 1 );
  if( (wind = find_window(handle)) == 0 )
  {
    DEBUGGER(WNDEL,INVHAND,handle);
    return 0;
  }
  if( wind==desktop )
  {
    DEBUGGER(WNDEL,DELDESK,0);
    return 0;
  }
  if( wind->place>=0 )
  {
    DEBUGGER(WNDEL,DECLOSE,handle);
    wind_app = 0L;
    close_window(handle);       /* del w/o close */
  }
  if( wind->apid>=0 && wind->apid != curapp->id )
  {
    DEBUGGER(WNDEL,NOTOWN,handle);
    return(0);
  }
  w = desktop;
  do
    if( w->next == wind )
    {
      w->next = wind->next;
      break;
    }
  while( (w = w->next) != 0 );
  set_handle( handle, 0 );
  if( wind_app && wind_app->flags.flags.s.limit_handles )
    for( h=0; h<7; h++ )
      if( wind_app->old_handles[h]==handle ) wind_app->old_handles[h]=0;
  free_rects( wind, 1 );
  lfree(wind);
  return(1);
}

void set_update(void)
{
/*  if( preempt ) Psemaphore( 2, UPDT_SEM, -1L );*/
  update.c[0] = cnt_update.c[0] > 0;
  update.c[1] = cnt_update.c[1] > 0;
  if( !update.i ) has_update = 0L;
  else if( !block_loop ) block_loop=1;	/* 005 */
/*  if( preempt ) Psemaphore( 3, UPDT_SEM, 0 ); */
}

int _wind_update( int flag )
{
  /* intentionally ignore flag&0x100 */
  if( (flag==BEG_UPDATE || flag==BEG_MCTRL) &&
      test_update( (void *)_wind_update ) ) return 0;
  return __wind_update( flag, 0L );
}

int __wind_update( int flag, APP *ap )
{
  if( !ap ) ap = curapp;
  if( flag&0x100 && (in_menu || update.i && has_update!=ap) ) return 0;
  switch( (char)flag )
  {
    case BEG_UPDATE:
      cnt_update.c[1]++;
      has_update = ap;
      break;
    case END_UPDATE:
      --cnt_update.c[1];
      if( !cnt_update.i ) unblock();		/* 004 */
      break;
    case BEG_MCTRL:
      cnt_update.c[0]++;
      has_update = ap;
      break;
    case END_MCTRL:
      --cnt_update.c[0];
      if( !cnt_update.i ) unblock();		/* 004 */
      break;
    default:
      DEBUGGER(WUPDATE,UNKTYPE,flag);
      return(0);
  }
  set_update();
  return(1);
}

void free_1rect( Rect_list *r )
{
  RECT_ARR *ra, *prev;

  for( ra=rect_arr, prev=0L; ra; ra=(prev=ra)->next )
    if( r >= (Rect_list *)(ra+1) && r < (Rect_list *)(ra+1)+16 )
    {
      if( (ra->map ^= 1<<(r - (Rect_list *)(ra+1))) == 0 )
      {
        if( !prev ) rect_arr = ra->next;
        else prev->next = ra->next;
        lfree(ra);
      }
      return;
    }
}

void free_rects( Window *wind, int always )
{
  Rect_list *r, *rn;
  APP *ap;

  if( (r = wind->rects) == 0L ) return;		/* 004: reworked */
  if( !always )
    if( update.i || (ap = find_ap(wind->apid)) != 0 &&
        ap->flags.flags.s.optim_redraws ) return;
  do
  {
    rn = r->next;
    free_1rect(r);
  }
  while( (r=rn) != 0 );
  wind->rects = wind->rectptr = 0L;
}

void wrecalc( Rect *r, int type, int xtype, int sign )
{
  int h, has_v, has_h;
  char max;

  h = sign*(cel_h-1);
  max = curapp->flags.flags.s.maximize_wind;
  if( !max ) has_h = has_v =
      type & (VSLIDE|UPARROW|DNARROW|HSLIDE|LFARROW|RTARROW|SIZER);
  else
  {
    has_v = type & (VSLIDE|UPARROW|DNARROW);
    has_h = type & (HSLIDE|LFARROW|RTARROW);
  }
  if( has_v || type&SIZER && (!has_h || xtype&X_HSPLIT) ) r->w += sign*(cel_w-1);
  if( has_h || type&SIZER && (!has_v || xtype&X_VSPLIT) ) r->h += h;
  if( type & (NAME|MOVER|CLOSER|FULLER|SMALLER) )
  {
    r->y -= h;
    r->h += h;
  }
  if( type & INFO )
  {
    r->y -= h;
    r->h += h;
  }
  if( xtype & X_MENU )
  {
    r->y -= (h=sign*(menu_h-1));
    r->h += h;
  }
/*  if( xtype & X_VSPLIT )
  {
    r->y -= (h=sign*(dflt_wind[WVSPLIT].ob_height-1));
    r->h += h;
  }
  if( xtype & X_HSPLIT )
  {
    r->x -= (h=sign*(dflt_wind[WHSPLIT].ob_width-1));
    r->w += h;
  } */
}

void recalc_outer( Rect inner, Rect *outer, int type, int xtype )
{
  *outer = inner;
  outer->x--;
  outer->w += 2;
  outer->y--;
  outer->h += 2;
  wrecalc( outer, type, xtype, 1 );
}

void recalc_inner( Rect outer, Rect *inner, int type, int xtype )
{
  *inner = outer;
  inner->x++;
  inner->w -= 2;
  inner->y++;
  inner->h -= 2;
  wrecalc( inner, type, xtype, -1 );
}

int calc_wind( int type, int kind, int kindx, Rect *out, Rect *r )
{
  if( type ) recalc_inner( *r, out, kind, kindx );
  else recalc_outer( *r, out, kind, kindx );
  return(1);
}

void cond_redraw( Window *w, int num, unsigned int rnum, int flag )
{
  Rect r;
  int i, j, nx, ny;

  if( w->place > 0 )
  {
    regenerate_rects( w, 0 );
    if( w->tree!=dflt_wind ) flag = 0;
    if( flag ) objc_off( w->tree, ++num, &r.x, &r.y ); /* old slider coords */
    recalc_window( w->handle, w, (long)rnum );
    /* avoid redraw problem with Neo 4 and real-time scrolling */
    if( flag )
    {
      objc_off( w->tree, num, &nx, &ny );       /* new coords */
      if( nx != r.x || ny != r.y )
      {
        r.w = u_object(w->tree,num)->ob_width+1;
        r.h = u_object(w->tree,num)->ob_height+1;
        if( num==WHSMLSL ) r.w += abs(nx-r.x);
        else r.h += abs(ny-r.y);
        if( flag<0 )
        {
          r.x = nx;
          r.y = ny;
        }
        redraw_obj( w, num-1, &r );
      }
    }
    else redraw_obj( w, num, 0L );
    free_rects( w, 0 );
  }
}

void redraw_info( Window *w )
{
  recalc_window( w->handle, w, INFO );
  _v_mouse(0);	/* 004 */
  redraw_obj( w, WILEFT, 0L );
  redraw_obj( w, WINFO, 0L );
  redraw_obj( w, WIRT, 0L );
  _v_mouse(1);	/* 004 */
}

void draw_wmenu( Window *w )
{
  if( w!=desktop && w->menu )
  {
    recalc_window( w->handle, w, (long)X_MENU<<8 );
    _v_mouse(0);	/* 004 */
    redraw_obj( w, WMNLEFT, 0L );
    redraw_obj( w, WMENU, 0L );
    redraw_obj( w, WMNRT, 0L );
    _v_mouse(1);	/* 004 */
  }
}

int check_slider( Window *w, int *i, int val )
{
  int r;

  recalc_window( w->handle, w, 0L );
  if( val < -1 ) val = -1;
  else if( val>1000 ) val = 1000;
  if( *i != val )
  {
    r = *i<val ? 1 : -1;
    *i = val;
    return(r);
  }
  return(0);
}

void _wind_set( Window *w, int item, int num )
{
  _set_window( w->handle, item, num, 0, 0, 0 );
}

void dial_sliders( Window *w, int draw )
{
  int i;

  if( w->dialog && w->treeflag&X_WTFL_SLIDERS )
  {
    if( w->type&HSLIDE )
    {
      _wind_set( w, WF_HSLSIZE, w->working.w*1000L/w->dialog[0].ob_width );
      if( (i = w->dialog[0].ob_x+w->dialog[0].ob_width - (w->working.x+w->working.w)) < 0 )
        if( (w->dialog[0].ob_x -= i) < w->working.x ) w->dialog[0].ob_x = w->working.x;
      if( (i=w->dialog[0].ob_width-w->working.w) <= 0 ) i = 0;
      else i = (w->working.x-w->dialog[0].ob_x)*1000L / i;
      if( draw ) _wind_set( w, WF_HSLIDE, i );
      else w->hslide = i;
    }
    if( w->type&VSLIDE )
    {
      _wind_set( w, WF_VSLSIZE, w->working.h*1000L/w->dialog[0].ob_height );
      if( (i = w->dialog[0].ob_y+w->dialog[0].ob_height - (w->working.y+w->working.h)) < 0 )
        if( (w->dialog[0].ob_y -= i) < w->working.y ) w->dialog[0].ob_y = w->working.y;
      if( (i=w->dialog[0].ob_height-w->working.h) <= 0 ) i = 0;
      else i = (w->working.y-w->dialog[0].ob_y)*1000L / i;
      if( draw ) _wind_set( w, WF_VSLIDE, i );
      else w->vslide = i;
    }
  }
}

void set_desk( APP *ap )
{
  desktop->apid = ap->id;
  desktop->tree = ap->desk;
  desk_obj = ap->desk_obj;
  if( !place )
  {
    if( has_desk ) has_desk->has_wind = 0;
    ap->has_wind = 1;
  }
  has_desk = ap;
}

void no_desk_own( int do_tree )
{
  if( !place && has_desk ) has_desk->has_wind = 0;
  has_desk = 0L;
  if( do_tree )		/* 004 */
  {
    desktop->tree = dflt_desk;
    desktop->apid = app0->id ? 1 : 0/*005*/;	/* 004 */
    desk_obj = 0;
  }
}

void new_desk( int mode, APP *ap )
{       /* mode:  -1: draw if different  0: no draw  1: draw if new is 0L */
  OBJECT *o;
  APP *ap2, *ap3;
  int flag;

  o = desktop->tree;
  if( ap && ap->desk ) set_desk(ap);
  else if( has_menu && has_menu->desk ) set_desk(has_menu);
  else if( !has_desk || !has_desk->desk || has_desk->asleep )
  {
    if( has_desk && !has_desk->asleep ) set_desk( ap2=has_desk );	/* 004: added sleep checks */
    else if( has_menu && !has_menu->asleep ) set_desk( ap2=has_menu );
    else
    {
      no_desk_own(1);
      if( (ap2 = app0) == 0 ) return;
      if( !ap2->id )
          while( (ap=ap2->next) != 0 ) ap2=ap;	/* 006: give single-tasker first chance */
    }
    desktop->tree = dflt_desk;
    desk_obj = 0;
    for( ap3=0L, ap=ap2->next; ap!=ap2; )
      if( !ap ) ap=app0;
      else if( ap->desk && !ap->asleep )
      {
        set_desk(ap);
        break;
      }
      else
      {
        if( !ap3 && !ap->asleep ) ap3=ap;
        ap=ap->next;
      }
    if( (!has_desk || has_desk->asleep/*004*/) && ap3 ) set_desk(ap3);
  }
  if( mode>0 && desktop->tree==dflt_desk ||
      mode<0 && o != desktop->tree )
  {
    flag=0;
    if( !curapp )
    {
      curapp = app0;
      flag++;
    }
    draw_desk();
    if( flag ) curapp = 0L;
  }
}

void set_dfltdesk( OBJECT *tree, int count )
{
  char was_mine;

  was_mine = desktop->tree == dflt_desk;
  if( (dflt_desk = tree) == 0 )
  {
    dflt_desk = dflt_desk0;
    dtree_cnt = DTREECNT;
  }
  else dtree_cnt = count;
  if( was_mine ) desktop->tree = dflt_desk;
}

void change_if( int *i, int num )
{
  if( num!=-1 ) *i = num;
}

char wcol_xref[] = { W_BOX, W_CLOSER, W_NAME, W_FULLER, W_FULLER, W_FULLER,
      W_INFO, W_INFO, W_INFO, -1/*tool*/, W_INFO, W_INFO/*menu*/, W_INFO, W_UPARROW,
      W_VSLIDE, W_VELEV, W_DNARROW, -1/*split*/, W_UPARROW, W_VSLIDE, W_VELEV,
      W_DNARROW, W_LFARROW, W_HSLIDE, W_HELEV, W_RTARROW, -1,
      W_LFARROW, W_HSLIDE, W_HELEV, W_RTARROW, W_SIZER };

void change_oldcol( int colors[2][WGSIZE+1], int num, int i1, int i0 )
{
  int i;

  for( i=0; i<=WGSIZE; i++ )
    if( wcol_xref[i] == num )
    {
      change_if( &colors[1][i], i1 );
      change_if( &colors[0][i], i0 );
      last_handle = -1;	/* 004 */
    }
}

int next_iconify( int *out )
{
  Window *w;
  unsigned int i, j;
  int x, y;

  for( i=1; i; i++ )
  {
    for( w=desktop; (w=w->next) != 0; )
      if( w->icon_index == i ) break;
    if( !w ) break;
  }
  if( out )
  {
    j = i-1;	/* 004 */
    x = desktop->working.w/(out[2]=ICON_WID);
    y = desktop->working.h/(out[3]=ICON_HT);
    out[0] = (j%x)*ICON_WID;	/* 004: was /x */
    out[1] = desktop->working.y+desktop->working.h - ICON_HT -
       (j%(x*y))/x*ICON_HT;	/* 004: added *ICON_HT */
  }
  return i;
}

void drw_win_menu( Window *w, Rect *r, int alts )
{
  int i, end;
  OBJECT *m = w->menu;

  if( !w->menu || !(w->xtype&X_MENU) ) return;
  if( (i=w->menu_tA) == 0 ) i = m[2].ob_head;
  if( (end=w->menu_tZ) == 0 ) end = m[2].ob_tail;
  else if( end<0 ) return;
  for(;;)
  {
    _objc_draw( (OBJECT2 *)m, find_ap(w->apid)/*007: was menu_owner*/,
        i, 0, Xrect(*r) );
    if( alts ) drw_alt( m, -i, 0 );
    if( i==end ) break;
    i = m[i].ob_next;
  }
}

int do_redraw( Window *wind, Rect *rect, int obj, int dial_rect )
{
  _objc_draw( (OBJECT2 *)wind->tree, 0L, obj, 8, Xrect(*rect) );
  drw_win_menu( wind, rect, 0 );
  if( wind != desktop )
    if( intersect( wind->working, *rect, rect ) )
      if( !wind->dialog ) return 0;
      else
      {
        _objc_draw( (OBJECT2 *)wind->dialog, 0L, 0, 8, Xrect(*rect) );
        wd_edit( wind, dial_rect ? rect : 0L );
      }
  return 1;
}

MSGQ *find_redraw( int h, MSGQ *q )
{
  for( ; q; q=q->next )
    if( ((int *)q->buf)[0]==WM_REDRAW && ((int *)q->buf)[3]==h ) return q;
  return 0L;
}

MSGQ *gad_msg;
int gad_hand;
Rect gad_rect;

void gad_clip_ini( Rect *r )
{
  if(r) gad_rect = *r;
  else gad_rect.w = -1;		/* edit cursor */
  gad_msg = find_redraw( gad_hand, msg_q );
  _v_mouse(0);
}

int gad_clip(void)
{
  Rect r;
  int ret=0;

  while( gad_msg && !ret )
  {
    if( gad_rect.w==-1 )	/* edit cursor */
    {
      _vs_clip( 1, (Rect *)&((int *)gad_msg->buf)[4] );
      ret = 1;
    }
    else if( intersect( gad_rect, *(Rect *)&((int *)gad_msg->buf)[4], &r ) )
    {
      _vs_clip( 1, &r );
      ret = 1;
    }
    gad_msg = find_redraw( gad_hand, gad_msg->next );
  }
  return ret;
}

Rect add2rect( Rect *in )
{
  Rect r;

  r = *in;
  r.w += 2;
  r.h += 2;
  return r;
}

int gad_redraws( Window *w, int draw )
{
  MSGQ *q, *q2;
  int h;
  Rect *r, r2;

  q = find_redraw( h=w->handle, msg_q );
  if( !q ) return 0;	/* 007: return 0 */
  if( draw )
  {
    gad_hand = h;
    clip_ini = gad_clip_ini;
    clip_it = gad_clip;
    r2 = add2rect( &w->outer );
    do_redraw( w, &r2, 0, draw<0 );
    reset_clip();
  }
  while(q)
  {
    r = (Rect *)&((int *)q->buf)[4];
    q2 = q->next;
    if( draw<0 || !intersect( *r, w->working, r ) )	/* limit redraw to working area */
        del_msg(q);				/* nothing in working area */
    q = find_redraw( h, q2 );
  }
  return 1;	/* 007 */
}

void optim_draw( Window *w, Rect *r, int gads )
{
  Rect_list *l;

  regenerate_rects( w, 1 );
  gen_rect( r, &w->rects );
  for( l=w->rects; l; l=l->next )
    redraw_window( w->handle, &l->r, 0, 0 );
  gad_redraws( w, gads );
  free_rects( w, 1 );
}

void blit( Window *w )
{
  int i;
  Rect rnew, old;

  if( intersect( desktop->working, add2rect(&w->outer), &rnew ) &&
      intersect( desktop->working, add2rect(&w->prev), &old ) )	/* 004 */
  {
    i = w->prev.x-w->outer.x;
    if( old.w > rnew.w )
    {
      if( i > 0 ) old.x += old.w-rnew.w;
      old.w = rnew.w;
    }
    else
    {
      if( i < 0 ) rnew.x -= old.w-rnew.w;
      rnew.w = old.w;
    }
    if( old.h > rnew.h ) old.h = rnew.h;
    else rnew.h = old.h;
    x_graf_blit( (GRECT *)&old, (GRECT *)&rnew );
    optim_draw( w, &rnew, 1 );
  }
}

int has_kgad( Window *w, int k )
{
  static int trans[] = { CLOSER, MOVER, SMALLER, CLOSER|MOVER|NAME|FULLER|SMALLER, FULLER,
      INFO, INFO, INFO,
      0,
      0, 0, 0,	/* can't fit X_MENU here */
      UPARROW, VSLIDE, VSLIDE, DNARROW,
      0,
      UPARROW, VSLIDE, VSLIDE, DNARROW,
      LFARROW, HSLIDE, HSLIDE, RTARROW,
      0,
      LFARROW, HSLIDE, HSLIDE, RTARROW,
      SIZER };

  return (w->type & trans[k-WCLOSE])!=0 || (w->xtype&X_MENU) &&
      k >= WMNLEFT && k<= WMNRT || (w->xtype&X_VSPLIT) && k==WVSPLIT ||
      (w->xtype&X_HSPLIT) && k==WHSPLIT;
}

int cycle( Window *w, int in_app )
{
  Window *w2;
  int h;

  if( w->place!=1 )
    for( h=1; h<place; h++ )	/* 004 */
      for( w2=desktop->next; w2; w2=w2->next )
        if( w2->place == h && has_kgad( w2, WBACK )/*004*/ &&
            (!in_app || !mouse_last || w2->apid==mouse_last->id) )
            return w2->handle;
  return 0;
}

int mv_bounds( Rect *r )
{
  if( r->x < 0 ) return -1;
  if( r->x+r->w+2 > desktop->working.w ) return 1;
  return 0;
}

#pragma warn -par
/*typedef struct{ int handle, change, i1, i2, i3, i4; } SWT;*/
int cdecl _set_window( int h, int c, int i1, int i2, int i3, int i4 )
{
  int set_window( SWT *t );

  wind_app = 0L;
  return set_window( (SWT *)&h );
}
#pragma warn +par

int set_window( SWT *t )
{
  Window *wind, *w, *w2;
  int pl, handle, i;
  Rect r, r2, r3;
  APP *ap;

  if( test_update( (void *)set_window ) ) return 0;
  handle = conv_handle( t->handle, 1 );
  wind = find_window(handle);
  if( t->change!=WF_NEWDESK && t->change!=WF_DCOLOR &&
      t->change!=X_WF_DFLTDESK/*004*/ && t->change!=X_WF_DCOLSTAT/*004*/ )
  {
    if( wind==desktop )
    {
      DEBUGGER(WNSET,SETDESK,t->change);
      return 0;
    }
    if( !wind )
    {
      DEBUGGER(WNSET,INVHAND,handle);
      return 0;
    }
  }
  switch( t->change )
  {
    case WF_CURRXYWH:
new_xywh:
      wind->prev = wind->outer;
      r = *(Rect *)&t->i1;
      if( wind->dialog )
      {
        wind->dialog[0].ob_x += r.x - wind->outer.x;
        wind->dialog[0].ob_y += r.y - wind->outer.y;
      }
      if( t->change!=WF_CURRXYWH ||
          *(long *)&r != *(long *)&wind->outer ||
          *((long *)&r+1) != *((long *)&wind->outer+1) )
      {
        /* free rects overlapped by old window position */
        if( wind->place>=0 ) free_all_rects( wind );	/* 004 */
        wind->outer = r;
        r3 = wind->working;	/* 004: save for optim */
        recalc_inner( r, &wind->working, wind->type, wind->xtype );
        if( wind->place>=0 )
        {
          ap = find_ap(wind->apid);
          i = ap && ap->flags.flags.s.optim_redraws && t->change==WF_CURRXYWH;
          dial_sliders( wind, 1 );
          both_splits(wind);
          recalc_window( handle, wind, -1L );
          pl=0;
          r = add2rect(&wind->prev);
          free_rects( wind, 1 );	/* 004 */
          max_redraw = 2;	/* 004 */
          /* always redraw whole thing if WM_(UN)ICONIFY */
          if( t->change==WF_CURRXYWH && *(long *)&wind->prev.w == *(long *)&wind->outer.w &&
              wind->place==place && intersect( desktop->working, r, &r2 ) &&
              (i && (mv_bounds(&wind->outer) ^ mv_bounds(&wind->prev)) != -2 ||
              !i && *(long *)&r == *(long *)&r2 &&
              *((long *)&r+1) == *((long *)&r2+1) && wind->outer.x>=0) )
          {
            blit(wind);
            pl=1;
          }
          else if( *(long *)&r == *(long *)&wind->outer )
            if( wind->outer.w <= wind->prev.w && wind->outer.h <=
                wind->prev.h )
            {
              regenerate_rects( wind, 0 );	/* 0 because of free above */
              redraw_obj( wind, 0, 0L );
              free_rects( wind, 0 );
              pl=1;
            }
            else if(i)
            {
              optim_draw( wind, &r3, wind->dialog ? -1 : 0 );
              all_gadgets( wind->place );
              free_rects( wind, 1 );
              pl=1;
            }
          max_redraw = 1;	/* 004 */
          /* free rects overlapped by new window position */
          free_all_rects( wind );
          for( w=desktop; w; w=w->next )	/* 004: started at desktop->next */
            if( w->place <= wind->place-pl ) redraw_window( w->handle,
                w==wind ? &wind->outer : &wind->prev, 1, 1 );
        }
      }
      break;
    case WF_FULLXYWH:
      wind->full = *(Rect *)&t->i1;
      break;
    case WF_WORKXYWH:
      return 0;
    case WF_NAME:
      wind->top_bar = *(char **)&t->i1 ? *(char **)&t->i1 : 0L;
      cond_redraw( wind, WMOVE, NAME, 0 );
      break;
    case WF_INFO:
      if( *(char **)&t->i1 )
      {
        strncpy( wind->info_bar, *(char **)&t->i1, INFO_LEN );
        wind->info_bar[INFO_LEN] = '\0';
      }
      else wind->info_bar[1] = wind->info_bar[0] = '\0';
      wind->info_pos = 0;
      if( wind->place > 0 )
      {
        regenerate_rects( wind, 0 );
        redraw_info(wind);
      }
      break;
    case WF_HSLIDE:
      if( (pl=check_slider( wind, &wind->hslide, t->i1 )) != 0 )
          cond_redraw( wind, WHBIGSL, HSLIDE, pl );
      break;
    case X_WF_HSLIDE2:
      if( (pl=check_slider( wind, &wind->hslide2, t->i1 )) != 0 )
          cond_redraw( wind, WHBIGSL2, HSLIDE, pl );
      break;
    case WF_VSLIDE:
      if( (pl=check_slider( wind, &wind->vslide, t->i1 )) != 0 )
          cond_redraw( wind, WVBIGSL, VSLIDE, pl );
      break;
    case X_WF_VSLIDE2:
      if( (pl=check_slider( wind, &wind->vslide2, t->i1 )) != 0 )
          cond_redraw( wind, WVBIGSL2, VSLIDE, pl );
      break;
    case WF_HSLSIZE:
      if( check_slider( wind, &wind->hslidesz, t->i1 ) )
          cond_redraw( wind, WHBIGSL, HSLIDE, 0 );
      break;
    case X_WF_HSLSIZE2:
      if( check_slider( wind, &wind->hslidesz2, t->i1 ) )
          cond_redraw( wind, WHBIGSL2, HSLIDE, 0 );
      break;
    case WF_VSLSIZE:
      if( check_slider( wind, &wind->vslidesz, t->i1 ) )
          cond_redraw( wind, WVBIGSL, VSLIDE, 0 );
      break;
    case X_WF_VSLSIZE2:
      if( check_slider( wind, &wind->vslidesz2, t->i1 ) )
          cond_redraw( wind, WVBIGSL2, VSLIDE, 0 );
      break;
    case WF_TOP:
      if( (wind->place != place || no_top) && wind->place>0 )
      {
        w2 = top_wind && !no_top && top_wind->handle ? top_wind : 0L;	/* 004: don't q the message yet */
        top_wind = wind;
        no_top = 0;
        pl = -1;
        for( w=desktop; (w=w->next) != 0; )
          if( w->place == place )
          {
            wd_edit(w,0L);
            pl = w==wind /* no_top was set */ ? w->place : --w->place;
            free_rects( w, 1 );
          }
          else if( w->place > wind->place )
          {
            --w->place;
            free_rects( w, 1 );
          }
        i = wind->place;
        free_rects( wind, 1 );
        wind->place = place;
        all_gadgets( pl );
        ap = find_ap(wind->apid);
        if( ap && ap->flags.flags.s.optim_redraws )
        {
          for( w=desktop; (w=w->next) != 0; )
            if( w != wind && w->place >= i )
              redraw_window( handle, &w->outer, 1, 0 );
          if( !gad_redraws( wind, wind->dialog!=0 ? -1 : 0 ) )
              wd_edit(wind,0L);	/* 007 */
          all_gadgets( place );
        }
        else redraw_window( handle, &desktop->working, 0, 1 );
        if( ap )
          if( ap->menu ) switch_menu(ap);
          else switch_mouse( ap, 0 );
        if( w2 ) newtop( WM_UNTOPPED, w2->handle, w2->apid );	/* 004: do msg now, so that app gets WM_REDRAWs first */
      }
      break;
    case WF_NEWDESK:
      curapp->desk_obj = t->i3;
      if( (curapp->desk = *(OBJECT **)&t->i1) == 0 )
      {		/* 004 */
        no_desk_own(0);
        new_desk( -1, curapp );
      }
      else new_desk( 1, curapp );
      break;
    case WF_COLOR:
      if( wind!=desktop )
      {
        change_oldcol( wind->colors, t->i1, t->i2, t->i3 );
        break;
      }
    case WF_DCOLOR:
      change_oldcol( dwcolors[wcolor_mode], t->i1, t->i2, t->i3 );
      break;
    case X_WF_DCOLSTAT:	/* 004 */
      change_if( &dwcolors[wcolor_mode][1][t->i1], t->i2 );
      change_if( &dwcolors[wcolor_mode][0][t->i1], t->i3 );
      change_if( (int *)&u_object(dflt_wind,t->i1)->ob_state, t->i4 );
      change_if( &wstates[wcolor_mode][t->i1], t->i4 );
      last_handle = -1;	/* 004 */
      break;
    case WF_BEVENT:
      wind->bevent = t->i1;
      break;
    case WF_BOTTOM:
      if( wind->place>0 && place>1 )
      {
        pl = wind->place;
        for( w=desktop->next; w; w=w->next )
        {
          if( w->place<pl && w->place>0 )
          {
            w->place++;
            free_rects( w, 1 );	/* 004 */
          }
          if( w->place==place ) w2=w;
        }
        free_rects( wind, 1 );	/* 004 */
        all_gadgets( wind->place=1 );
        if( pl==w2->place ) all_gadgets( pl );
        for( w=desktop->next; w; w=w->next )
          if( w->place<=pl && w!=wind ) redraw_window( w->handle,
              &wind->outer, 1, 1 );
        if( (ap = find_ap(w2->apid)) != 0 )
        {
          newtop( WM_ONTOP, w2->handle, ap->id );
          /* need to set no_top here, too */
        }
        top_wind = w2;
      }
      break;
    case WF_ICONIFY:
      if( wind->icon_index )
      {
        DEBUGGER(WNSET,ALRIC,wind->handle);
        return 0;
      }
      if( *(long *)&t->i1==-1L ) 	/* 006 */
      {
        iconify_it( wind, 1 );		/* 005 */
        *(Rect *)&t->i1 = wind->working;
      }
      else iconify_it( wind, 0 );		/* 005 */
      if( wind->place==place )	/* 004 */
        for( i=place-1; i>0; i-- )
          for( w=desktop->next; w; w=w->next )
            if( w->place == i && !w->icon_index )
            {
              newtop( WM_TOPPED, w->handle, w->apid );
              goto new_xywh;
            }
      goto new_xywh;
    case WF_UNICONIFY:
      if( !wind->icon_index )
      {
        DEBUGGER(WNSET,NOTIC,wind->handle);
        return 0;
      }
      wind->icon_index = 0;
      wind->type = wind->old_type;
      wind->xtype = wind->old_xtype;
      goto new_xywh;
    case WF_UNICONIFYXYWH:
      wind->iconify = *(Rect *)&t->i1;
      break;
    case X_WF_MENU:
      if( (wind->menu = *(OBJECT **)&t->i1) != 0 )
      {
        resize_menu(wind->menu);	/* 007 */
        set_equivs(wind);
      }
      wind->menu_tA = wind->menu_tZ = 0;
      if( wind->place >= 0 ) draw_wmenu(wind);
      break;
    case X_WF_DIALOG:
      wind->dial_obj = wind->dial_edit = 0;
      if( (wind->dialog = *(OBJECT **)&t->i1) != 0 )
      {
        if( !(wind->type&(HSLIDE|VSLIDE)) )
            *(Rect *)&(wind->dialog[0].ob_x) = *(Rect *)&wind->working;
        dial_sliders( wind, 1 );
        next_obj = 0;
        form_init( wind->dialog );
        wind->dial_obj = next_obj;
        wind->dial_edit = edit_obj;
        redraw_window( wind->handle, &wind->working, 0, 1 );
      }
      break;
    case X_WF_DIALWID:
      wind->dial_swid = t->i1;
      break;
    case X_WF_DIALHT:
      wind->dial_sht = t->i1;
      break;
    case X_WF_DIALFLGS:
      i = wind->treeflag & ~(X_WTFL_ACTIVE|X_WTFL_BLITSCRL) |
          ((t->i1&3)<<3);
      if( (i&X_WTFL_ACTIVE) != (wind->treeflag&X_WTFL_ACTIVE) )
      {
        wind->treeflag |= X_WTFL_ACTIVE;
        wd_edit( wind, 0L );
      }
      wind->treeflag = i;
      break;
    case X_WF_DIALEDIT:
      if( wind->dialog )
      {
        if( t->i1>=0 )
        {
          init_win_dial( wind );
          if( edit_obj>0 ) objc_edit( wind->dialog, edit_obj, 0, &edit_idx, ED_END );
          edit_obj = t->i1;
          edit_idx = t->i2;
          if( edit_obj>0 ) objc_edit( wind->dialog, edit_obj, 0, &edit_idx,
              edit_idx>=0 ? ED_END : ED_INIT );
          next_obj = 0;
          exit_win_dial( wind );
        }
        break;
      }
      DEBUGGER(WNSET,NOTDIAL,wind->handle);
      return 0;
    case X_WF_DFLTDESK:
      set_dfltdesk( *(OBJECT **)&t->i1, t->i3 );
      ddesk_app = *(OBJECT **)&t->i1!=0 ? curapp : 0L;
      break;
    case X_WF_MINMAX:
      change_if( &wind->min_w, t->i1 );
      change_if( &wind->min_h, t->i2 );
      change_if( &wind->max_w, t->i3 );
      change_if( &wind->max_h, t->i4 );
      break;
    case X_WF_SPLMIN:
      change_if( &wind->hsp_min1, t->i1 );
      change_if( &wind->hsp_min2, t->i2 );
      change_if( &wind->vsp_min1, t->i3 );
      change_if( &wind->vsp_min2, t->i4 );
      if( both_splits(wind) )
      {
        r = wind->working;
        r.h += cel_h;
        r.w += cel_w;
        goto draw;
      }
      break;
    case X_WF_HSPLIT:
      if( check_split( &wind->hsplit, t->i1, wind->hsp_min1,
          wind->hsp_min2, wind->working.w, wind->tree[WHSPLIT].ob_width ) )
      {
        r = wind->working;
        r.h += cel_h;
        goto draw;
      }
      break;
    case X_WF_VSPLIT:
      if( check_split( &wind->vsplit, t->i1, wind->vsp_min1,
          wind->vsp_min2, wind->working.h, wind->tree[WVSPLIT].ob_height ) )
      {
        r = wind->working;
        r.w += cel_w;
draw:   recalc_window( handle, wind, -1L );
        redraw_window( wind->handle, &r, 0, 1 );
      }
      break;
    case X_WF_OBJHAND:
      wind->objhand = *(int cdecl(**)(int hand, int obj))&t->i1;
      break;
    default:
      DEBUGGER(WNSET,UNKTYPE,t->change);
      return 0;
  }
  return(1);
}

int hide_if( OBJECT *tree, int truth, int idx )
{
  unsigned int *i;

  i = &u_object(tree,idx)->ob_flags;
  if( !truth ) *i |= HIDETREE;
  else *i &= ~HIDETREE;
  return truth;
}

void position_info( Window *w )
{
  int i, wid, t, cw, ch;
  char *ptr;
  TEDINFO *ted;

  if( !(w->treeflag&X_WTFL_SLIDERS) ) w->info_pos = w->info_end = 1;
  else
  {
    i = w->outer.w-2;
    if( w->info_pos ) i -= cel_w;
/*    w->info_end = strlen(w->info_bar+w->info_pos)*char_w > i;  003 */
    ptr = w->info_bar+w->info_pos;
    if( (t = (ted=u_tedinfo(w->tree,WINFO))->te_font) != SMALL && t != IBM )
    {
      ted_font( t, ted->te_junk1, ted->te_junk2, &cw, &ch );
      wid = prop_extent( t, ptr, &cw );
    }
    else wid = strlen(ptr)*char_w;
    w->info_end = wid > i;
  }
}

void position_menu( Window *w )
{
  int mx, i, wid, tail;
  OBJECT *m=w->menu;

  wid = w->outer.w;
  if( (i = w->menu_tA) <= 0 ) i = m[2].ob_head;
  else wid -= cel_w;
  tail = m[2].ob_tail;
  w->menu_tZ=-1;
  for(;;)
  {
    wid -= u_object(m,i)->ob_width;
    if( wid < cel_w )
      if( i!=tail || wid < 0 ) break;
    w->menu_tZ=i;
    if( i==tail ) break;
    i = u_object(m,i)->ob_next;
  }
  if( w->menu_tZ == tail ) w->menu_tZ=0;
  else if( w->menu_tZ<0 ) w->menu_tA = -1;
  else if( w->menu_tA<0 ) w->menu_tA = 0;
}

void calc_slid( int i, OBJECT *tree, int smlsl, int siz, int cel, int off, int pos )
{
  int *ip;

  if( (*(ip=&u_object(tree,smlsl)->ob_width+off) = siz <= 0 ? cel :
      (long)siz*i/1000) < cel ) *ip=cel;
  *(ip-2) = (long)(i - *ip) * pos / 1000;
}

int *gad_color( OBJECT *tree, int i )
{
  int t, u;

  tree = u_object(tree,i);
  u = is_xusrdef( 0L, tree );
  if( (t=tree->ob_type) == G_BOX || t==G_IBOX || t==G_BOXCHAR ||
      t==G_BUTTON )
    if(u) return (int *)&tree->ob_spec.userblk->ub_parm+1;  /* was ub_code in 003 */
    else return (int *)&tree->ob_spec+1;
  if( t==G_FTEXT || t==G_FBOXTEXT || t==G_TEXT || t==G_BOXTEXT )
    if(u) return &((TEDINFO *)(tree->ob_spec.userblk->ub_code))->te_color;
    else return &u_tedinfo(tree,0)->te_color;
  return 0L;
}
void recalc_window( int handle, Window *wind, long flag )
{
  int type=wind->type, typex=wind->xtype, w, w_1, w_2, h, h_1, h_2, i,
      top, has_h, has_v, ch_1, cw_1, ybase, mx, mh_1, wsp_ht, wsp_wd, *iptr;
  char max;
  OBJECT *tree;
  int dummy;
#if 0
  static int dummy;
  static int *cols[] = { (int *)&rs_object[1].ob_spec+1,        /* box */
                       (int *)&dummy,                           /* title */
                       (int *)&rs_object[2].ob_spec+1,          /* closer */
                       &rs_tedinfo[0].te_color,                 /* name */
                       (int *)&rs_object[6].ob_spec+1,          /* fuller */
                       &rs_tedinfo[1].te_color,                 /* info */
                       &dummy,                                  /* data */
                       &dummy,                                  /* work */
                       (int *)&rs_object[32].ob_spec+1,         /* sizer */
                       &dummy,                                  /* vbar */
                       (int *)&rs_object[14].ob_spec+1,         /* uparr */
                       (int *)&rs_object[17].ob_spec+1,         /* dnarr */
                       (int *)&rs_object[15].ob_spec+1,         /* vslide */
                       (int *)&rs_object[16].ob_spec+1,         /* velev */
                       &dummy,                                  /* hbar */
                       (int *)&rs_object[23].ob_spec+1,         /* lfarrow */
                       (int *)&rs_object[26].ob_spec+1,         /* rtarrow */
                       (int *)&rs_object[24].ob_spec+1,         /* hslide */
                       (int *)&rs_object[25].ob_spec+1 };       /* helev */
  static int *cols2[] = {
                       (int *)&rs_object[5].ob_spec+1,          /* toback */
                       (int *)&rs_object[7].ob_spec+1,          /* lfinfo */
                       (int *)&rs_object[9].ob_spec+1,          /* rtinfo */
                       (int *)&rs_object[4].ob_spec+1,          /* smaller */
                       (int *)&rs_object[19].ob_spec+1,         /* uparr2 */
                       (int *)&rs_object[20].ob_spec+1,         /* vslide2 */
                       (int *)&rs_object[21].ob_spec+1,         /* velev2 */
                       (int *)&rs_object[22].ob_spec+1,         /* dwnarr2 */
                       (int *)&rs_object[28].ob_spec+1,         /* lfarrow2 */
                       (int *)&rs_object[29].ob_spec+1,         /* hslide2 */
                       (int *)&rs_object[30].ob_spec+1,         /* helev2 */
                       (int *)&rs_object[31].ob_spec+1 };       /* rtarrow2 */
  static char col2x[] = { 2, 5, 5, 2, 10, 12, 13, 11, 15, 17, 18, 16 };
#endif

  if( wind==desktop ) return;
  if( !(wind->treeflag&X_WTFL_RESIZE) ) return;
  max = wind->treeflag&X_WTFL_MAXIMIZE;
  tree = wind->tree;
  if( last_handle != handle || flag<0L ) /* new window or flag says force it */
  {
    flag = -1L;
    last_handle = handle;
    *(Rect *)&tree[0].ob_x = wind->outer;
  }
  else if( !flag ) return;
  top = wind->place == place && !no_top;
  if( flag==-1L )
    for( i=0; i<=WGSIZE/*004*/; i++ )
      if( i!=WGMOVE )
        if( (iptr = gad_color( tree, i )) != 0 ) *iptr = wind->colors[top][i];
/*  if( (iptr = gad_color( tree, WGINFO )) != 0 ) *iptr = wind->colors[top][W_INFO];*/
  if( (iptr = gad_color( tree, WGMOVE )) != 0 )
  {
    *iptr = wind->colors[top][WGMOVE];
    iptr = &u_tedinfo(tree,WGMOVE)->te_font;
    if( *iptr==SMALL || *iptr==IBM ) *iptr = wind->icon_index ? SMALL : IBM;
  }
  if( !(type&VSLIDE) ) tree[WVBIGSL].ob_spec.obspec.fillpattern =
      tree[WVBIGSL2].ob_spec.obspec.fillpattern = 0;
  if( !(type&HSLIDE) ) tree[WHBIGSL].ob_spec.obspec.fillpattern =
      tree[WHBIGSL2].ob_spec.obspec.fillpattern = 0;
  w_2 = (w_1 = (w = wind->outer.w) - cel_w) - cel_w + 1;
  h_2 = (h_1 = (h = wind->outer.h) - cel_h) - cel_h + 1;
  top = type & (NAME|MOVER|CLOSER|FULLER|SMALLER);
  if( !max ) has_h = has_v = type&(HSLIDE|LFARROW|RTARROW|VSLIDE|UPARROW|DNARROW|SIZER);
  else
  {
    has_v = type&(VSLIDE|UPARROW|DNARROW);
    has_h = type&(HSLIDE|LFARROW|RTARROW);
  }
  if( type&SIZER )
  {
    if( typex&X_HSPLIT ) has_v = 1;
    if( typex&X_VSPLIT ) has_h = 1;
  }
  ch_1 = cel_h-1;
  cw_1 = cel_w-1;
  mh_1 = menu_h-1;
  ybase=0;
  if( top ) ybase = ch_1;
  if( type&INFO ) ybase += ch_1;
  if( typex&X_MENU ) ybase += mh_1;
  if( wind->tool ) ybase += wind->tool[0].ob_height;
  wsp_ht = typex&X_VSPLIT ? wind->tree[WVSPLIT].ob_height-1 : 0;
  wsp_wd = typex&X_HSPLIT ? wind->tree[WHSPLIT].ob_width-1 : 0;

  if( flag & (NAME|MOVER|CLOSER|FULLER|SMALLER) )	/* show send to back */
    if( hide_if( tree, top, WBACK ) ) tree[WBACK].ob_x = (type&FULLER) ? w_2 : w_1;

  if( flag & CLOSER ) hide_if( tree, type&CLOSER, WCLOSE );

  if( flag & SMALLER )
    if( hide_if( tree, type&SMALLER, WICONIZ ) )
        tree[WICONIZ].ob_x = ((type&FULLER) ? w_2-cel_w+1 : w_2);

  if( flag & FULLER )
    if( hide_if( tree, type&FULLER, WFULL ) ) tree[WFULL].ob_x = w_1;

  if( flag & (MOVER|NAME) )
    if( hide_if( tree, top, WMOVE ) )
    {
      tree[WMOVE].ob_x = (type&CLOSER) ? cw_1 : 0;
      tree[WMOVE].ob_width = w_1 - ((type&CLOSER) ? cw_1 : 0) -
          ((type&FULLER) ? cw_1 : 0) - ((type&SMALLER) ? cw_1 : 0) + 1;
    }

  if( flag & NAME ) u_tedinfo(tree,WMOVE)->te_ptext = wind->top_bar;

  if( flag & INFO )
  {
    if( (i = type&INFO) != 0 ) position_info(wind);
    else wind->info_pos=wind->info_end=0;
    hide_if( tree, i && wind->info_pos>0, WILEFT );
    hide_if( tree, i && wind->info_end>0, WIRT );
    if( hide_if( tree, i, WINFO ) )
    {
      tree[WIRT].ob_y = tree[WILEFT].ob_y = tree[WINFO].ob_y =
          top ? ch_1 : 0;
      tree[WIRT].ob_x = w_1;
      tree[WINFO].ob_x = 0;
      i = w;
      if( wind->info_pos>0 )
      {
        i -= cel_w-1;
        tree[WINFO].ob_x += cel_w-1;
      }
      if( wind->info_end>0 ) i -= cel_w-1;
      tree[WINFO].ob_width = i;
      u_tedinfo(tree,WINFO)->te_ptext = wind->info_bar + wind->info_pos;
    }
  }

  if( flag & ((long)X_MENU<<8) )
  {
    if( (i = (typex&X_MENU) && wind->menu) != 0 &&
        wind->menu[2].ob_width > w ) position_menu(wind);
    else wind->menu_tA=wind->menu_tZ=0;
    hide_if( tree, i && wind->menu_tA>0, WMNLEFT );
    hide_if( tree, i && wind->menu_tZ>0, WMNRT );
    if( hide_if( tree, i, WMENU ) )
    {
      tree[WMNRT].ob_y = tree[WMNLEFT].ob_y = tree[WMENU].ob_y = ybase-mh_1;
      tree[WMNRT].ob_x = w_1;
      tree[WMENU].ob_x = 0;
      i = w;
      if( wind->menu_tA>0 )
      {
        i -= cel_w-1;
        tree[WMENU].ob_x += cel_w-1;
      }
      if( wind->menu_tZ>0 ) i -= cel_w-1;
      tree[WMENU].ob_width = i;
      if( wind->menu )
      {
        wind->menu[0].ob_x = 0;
        if( (i = wind->menu_tA) <= 0 ) i = wind->menu[2].ob_head;
        objc_off( wind->menu, i, &mx, &dummy );
        wind->menu[0].ob_x = wind->outer.x+tree[WMENU].ob_x-mx;
        wind->menu[0].ob_y = wind->outer.y+ybase-mh_1;
      }
    }
  }

  if( flag & SIZER )
  {
    if( !(type&SIZER) && (!max && has_h || (type&(LFARROW|HSLIDE|RTARROW)) &&
        (type&(UPARROW|VSLIDE|DNARROW))) )
    {
      hide_if( tree, 1, WSIZE );
      i = ' ';
    }
    else
    {
      hide_if( tree, type&SIZER, WSIZE );
      i = '';
    }
    tree[WSIZE].ob_spec.obspec.character = i;
    tree[WSIZE].ob_x = w_1;
    tree[WSIZE].ob_y = h_1;
  }

  if( flag & UPARROW )
  {
    hide_if( tree, (type&UPARROW)!=0 && wind->vsplit, WUP );
    tree[WUP2].ob_x = tree[WUP].ob_x = w_1;
    tree[WUP].ob_y = ybase + (!wind->vsplit?wsp_ht:0);
    if( hide_if( tree, (type&UPARROW) && (typex&X_VSPLIT) && wind->vsplit>=0, WUP2 ) )
        tree[WUP2].ob_y = ybase + wind->vsplit + wsp_ht;
  }

  if( flag & DNARROW )
  {
    hide_if( tree, (type&DNARROW)!=0 && wind->vsplit, WDOWN );
    tree[WDOWN2].ob_x = tree[WDOWN].ob_x = w_1;
    tree[WDOWN].ob_y = (max && !(type & (HSLIDE|RTARROW|LFARROW|SIZER)) ? h_1 :
        h_2) - ((typex&X_VSPLIT)!=0 && wind->vsplit<0 ? wsp_ht : 0);
    if( hide_if( tree, (type&DNARROW) && (typex&X_VSPLIT) && wind->vsplit>=0, WDOWN2 ) )
    {
      tree[WDOWN2].ob_y = tree[WDOWN].ob_y;
      tree[WDOWN].ob_y = ybase + wind->vsplit - ch_1;
    }
  }

  if( flag & VSLIDE )
  {
    hide_if( tree, type&VSLIDE && wind->vsplit, WVSMLSL );
    mx = wind->vsplit>=0 ? wind->vsplit : h-ybase-(has_h||type&SIZER ? cel_h :1)-wsp_ht;
    if( hide_if( tree, wind->vsplit!=0 && (has_v || type&SIZER && !has_h), WVBIGSL ) )
    {
      tree[WVBIGSL].ob_x = w_1;
      tree[WVBIGSL].ob_y = ybase + ((type&UPARROW) ? ch_1 : 0) + (!wind->vsplit ? wsp_ht : 0);
      calc_slid( tree[WVBIGSL].ob_height = mx + 1 -
          ((type&UPARROW) ? ch_1 : 0) -
          ((type&DNARROW) ? ch_1 : 0),
          tree, WVSMLSL, wind->vslidesz, cel_h, 1, wind->vslide );
    }
    hide_if( tree, i=(type&VSLIDE) && (typex&X_VSPLIT) && wind->vsplit>=0, WVSMLSL2 );
    if( hide_if( tree, i || has_v || type&SIZER && !has_h, WVBIGSL2 ) )
    {
      tree[WVBIGSL2].ob_x = w_1;
      tree[WVBIGSL2].ob_y = ybase + mx + wsp_ht + ((type&UPARROW) ? ch_1 : 0);
      calc_slid( tree[WVBIGSL2].ob_height = h - ybase -
          wsp_ht -
          mx -
          ((type&UPARROW) ? ch_1 : 0) -
          (!max || (type&(SIZER|LFARROW|RTARROW|HSLIDE)) ? ch_1 : 0) -
          ((type&DNARROW) ? ch_1 : 0),
          tree, WVSMLSL2, wind->vslidesz2, cel_h, 1, wind->vslide2 );
    }
  }

  if( flag & LFARROW )
  {
    hide_if( tree, (type&LFARROW)!=0 && wind->hsplit, WLEFT );
    tree[WLEFT2].ob_y = tree[WLEFT].ob_y = h_1;
    tree[WLEFT].ob_x = !wind->hsplit ? wsp_wd : 0;
    if( hide_if( tree, (type&LFARROW) && (typex&X_HSPLIT) && wind->hsplit>=0, WLEFT2 ) )
        tree[WLEFT2].ob_x = wsp_wd + wind->hsplit;
  }

  if( flag & RTARROW )
  {
    hide_if( tree, (type&RTARROW)!=0 && wind->hsplit, WRT );
    tree[WRT2].ob_y = tree[WRT].ob_y = h_1;
    tree[WRT].ob_x = (!max || (type&(SIZER|UPARROW|DNARROW|VSLIDE)) ? w_2 :
        w_1) - ((typex&X_HSPLIT)!=0 && wind->hsplit<0 ? wsp_wd : 0);
    if( hide_if( tree, (type&RTARROW) &&(typex&X_HSPLIT) && wind->hsplit>=0, WRT2 ) )
    {
      tree[WRT2].ob_x = tree[WRT].ob_x;
      tree[WRT].ob_x = wind->hsplit - cw_1;
    }
  }

  if( flag & HSLIDE )
  {
    hide_if( tree, type&HSLIDE && wind->hsplit, WHSMLSL );
    mx = wind->hsplit>=0 ? wind->hsplit : w-(has_v||type&SIZER ? cel_w :1)-wsp_wd;
    if( hide_if( tree, wind->hsplit!=0 && (has_h || type&SIZER && !has_v), WHBIGSL ) )
    {
      tree[WHBIGSL].ob_x = (!wind->hsplit ? wsp_wd : 0) + ((type&LFARROW) ? cw_1 : 0);
      tree[WHBIGSL].ob_y = h_1;
      calc_slid( tree[WHBIGSL].ob_width = mx + 1 -
          ((type&LFARROW) ? cw_1 : 0) -
          ((type&RTARROW) ? cw_1 : 0),
          tree, WHSMLSL, wind->hslidesz, cel_w, 0, wind->hslide );
    }
    hide_if( tree, i=(type&HSLIDE) && (typex&X_HSPLIT) && wind->hsplit>=0, WHSMLSL2 );
    if( hide_if( tree, i || has_h || type&SIZER && !has_v, WHBIGSL2 ) )
    {
      tree[WHBIGSL2].ob_y = h_1;
      tree[WHBIGSL2].ob_x = mx + wsp_wd + ((type&LFARROW) ? cw_1 : 0);
      calc_slid( tree[WHBIGSL2].ob_width =
          ((type&LFARROW) ? w_1+1 : w) -
          wsp_wd -
          mx -
          (!max || (type&(SIZER|UPARROW|DNARROW|VSLIDE)) ? cw_1 : 0) -
          ((type&RTARROW) ? cw_1 : 0),
          tree, WHSMLSL2, wind->hslidesz2, cel_w, 0, wind->hslide2 );
    }
  }

  if( flag & ((long)X_HSPLIT<<8) )
    if( hide_if( tree, typex&X_HSPLIT, WHSPLIT ) )
    {
      tree[WHSPLIT].ob_x = wind->hsplit>=0 ? wind->hsplit :
          (type&SIZER || has_v ? w_1 : w) - wsp_wd;
      tree[WHSPLIT].ob_height = h-(tree[WHSPLIT].ob_y=ybase);
    }

  if( flag & ((long)X_VSPLIT<<8) )
    if( hide_if( tree, typex&X_VSPLIT, WVSPLIT ) )
    {
      tree[WVSPLIT].ob_y = wind->vsplit>=0 ? ybase + wind->vsplit :
          (type&SIZER || has_h ? h_1 : h) - wsp_ht;
      tree[WVSPLIT].ob_width = w;
    }
}

void wm_redraw( Window *w, Rect *add )
{
  int buf[8];

  buf[0] = WM_REDRAW;
  buf[1] = w->apid;
  buf[2] = 0;
  buf[3] = w->handle;
  *(Rect *)&buf[4] = *add;
  __appl_write( w->apid, 16, buf );
}

void add_r( Rect *curr, Rect *add )
{
  int i;

  if( (i=curr->x-add->x) > 0 )
  {
    curr->w += i;
    curr->x -= i;
  }
  if( (i=curr->y-add->y) > 0 )
  {
    curr->h += i;
    curr->y -= i;
  }
  if( (i=add->x+add->w-(curr->x+curr->w)) > 0 ) curr->w += i;
  if( (i=add->y+add->h-(curr->y+curr->h)) > 0 ) curr->h += i;
}

void add_redraw( Window *w, Rect *add )
{
  MSGQ *q, *mq;	/* 004 */
  Rect *curr, r, r2;
  APP *ap;
  int h, i, count;
  unsigned long min, m;

  if( add->w <= 0 || add->h <= 0 ) return;
  if( (q = find_redraw( h=w->handle, msg_q )) == 0 ) wm_redraw( w, add );
  else
  {
    curr = (Rect *)&((int *)q->buf)[4];
    min = (unsigned long)-1L;
    mq = 0L;
    count = 0;
    while(q)
    {
      curr = (Rect *)&((int *)q->buf)[4];
      if( intersect( *curr, *add, &r ) )
      {
        if( *(long *)&r.w == *(long *)&curr->w )
        {
          *curr = *add;	/* new completely overlaps old */
          return;
        }
        if( r.h == add->h )   /* horizontal cross */
        {
          if( r.w == add->w ) return;	/* new is completely inside old */
          r2.y = r.y;
          r2.h = r.h;
          r2.w = curr->x - (r2.x=add->x);
          add_redraw( w, &r2 );	/* left part */
          r2.w = add->x+add->w - (r2.x = r.x+r.w);	/* can't rely on curr */
          add_redraw( w, &r2 );	/* right part */
          return;
        }
        if( r.w == add->w )   /* vertical cross */
        {
          r2.x = r.x;
          r2.w = r.w;
          r2.h = curr->y - (r2.y=add->y);
          add_redraw( w, &r2 );	/* top part */
          r2.h = add->y+add->h - (r2.y = r.y+r.h);
          add_redraw( w, &r2 );	/* bottom part */
          return;
        }
        if( r.h == curr->h )
          if( curr->x < r.x ) curr->w = r.x - curr->x;
          else
          {
            curr->x += r.w;
            curr->w -= r.w;
          }
        else if( r.w == curr->w )
          if( curr->y < r.y ) curr->h = r.y - curr->y;
          else
          {
            curr->y += r.h;
            curr->h -= r.h;
          }
      }
      r = *curr;
      add_r( &r, add );		/* temporarily merge the two */
      m = (long)r.w * r.h;	/* area of merge */
      if( m == (long)curr->w*curr->h + (long)add->w*add->h )
      {
        *curr = r;		/* new is flush with old */
        return;
      }
      if( m < min )	/* find smallest merge */
      {
        min = m;
        mq = q;
      }
      count++;
      q = find_redraw( h, q->next );
    }
    if( count>=max_redraw ) add_r( (Rect *)&((int *)mq->buf)[4], add );
    else wm_redraw( w, add );
  }
}

int redraw_window( int handle, Rect *ar, int add, int now )
{
  Window *wind, *w;
  Rect rect, area, wo;
  Rect_list *r;
  int obj;

  if( (wind = find_window(handle)) == 0 || wind->place<0 ) return(0);
  area = *ar;
  if(add)
  {
    area.w += 2;
    area.h += 2;
  }
  wo = add2rect( &wind->outer );	/* 004 */
  if( wind->tree && intersect( desktop->outer, area, &area ) &&
      intersect( wo, area, &area ) )	/* 004 */
  {
    regenerate_rects( wind, 0 );	/* 004: moved inside if */
    recalc_window( handle, wind, 0L );
    if( now ) _v_mouse(0);	/* 004 */
    obj = wind==desktop ? desk_obj : 0;
    r = wind->rects;
    while( r )
    {
      if( intersect( area, r->r, &rect ) )
        if( !now ) add_redraw( wind, &rect );
        else if( !do_redraw( wind, &rect, obj, 1 ) ) add_redraw( wind, &rect );
      r = r->next;
    }
    if( now ) _v_mouse(1);	/* 004 */
    free_rects( wind, 0 );		/* 004: moved */
  }
  return(1);
}

char add_err;

Rect_list *add_rect( Rect_list *r, Rect_list **root )
{
  Rect_list *ret;
  RECT_ARR *ra;
  unsigned int map, mask;
  int i;

  if( add_err ) return(0);
  for( ra=rect_arr; ra; ra=ra->next )
    if( ra->map != 0xFFFF ) break;
  if( !ra )
    if( (ra = (RECT_ARR *)lalloc( sizeof(RECT_ARR) + 16*sizeof(Rect_list), -1 )) == 0L )
    {
      add_err++;
      no_memory();
      return 0L;
    }
    else
    {
      ra->next = rect_arr;
      rect_arr = ra;
      ra->map = 0;
    }
  for( ret=(Rect_list *)(ra+1), i=16, map=ra->map, mask=1; --i>=0;
      ret++, map>>=1, mask<<=1 )
    if( !(map&1) )
    {
      ra->map |= mask;
      if(r)
      {
        ret->next = r->next;
        r->next = ret;
      }
      else
      {
        *root = ret;
        ret->next = 0L;
      }
      return ret;
    }
  return 0L;
/*%  if( (ret = (Rect_list *)lalloc( sizeof(Rect_list), -1 )) == 0L )
  {
    add_err++;
    no_memory();
  }
  else if(r)
  {
    ret->next = r->next;
    r->next = ret;
  }
  else
  {
    *root = ret;
    ret->next = 0L;
  }
  return(ret); */
}

int make_rect( int i, Rect_list **r, int x, int y, int w, int h,
    Rect_list **root )
{
  Rect rect;
  Rect_list *l;

  rect.x = x;
  rect.y = y;
  if( (rect.w = w) == 0 || (rect.h = h) == 0 ) return(i);
  if( !i ) (*r)->r = rect;
  else if( (l = add_rect( *r, root )) != 0 ) (*r=l)->r = rect;
  return(++i);
}

int gen_rect( Rect *in, Rect_list **root )
{
  Rect rect1, rect2;
  Rect_list *r, *prev, *rn;
  int i;

      r = *root;
      prev = 0L;
      do
      {
        if( intersect( *in, r->r, &rect2 ) )
          if( *(long *)&r->r == *(long *)&rect2 &&
              *((long *)&r->r+1) == *((long *)&rect2+1) )
          {
            rn = r->next;
            if( prev ) prev->next = rn;
            else *root = rn;
            free_1rect(r);
            r = rn;
          }
          else
          {
            rect1 = r->r;
            i = 0;
            if( rect2.x > rect1.x ) i = make_rect( i, &r, rect1.x, rect1.y,
                rect2.x-rect1.x, rect1.h, root );
            if( rect2.x+rect2.w < rect1.x+rect1.w ) i = make_rect( i, &r,
                rect2.x+rect2.w, rect1.y, rect1.x+rect1.w-rect2.x-rect2.w,
                rect1.h, root );
            if( rect2.y > rect1.y ) i = make_rect( i, &r, rect2.x, rect1.y, rect2.w,
                rect2.y-rect1.y, root );
            if( rect2.y+rect2.h < rect1.y+rect1.h ) i = make_rect( i, &r, rect2.x,
                rect2.y+rect2.h, rect2.w, rect1.y+rect1.h-rect2.y-rect2.h,
                root );
            prev = r;
            r = r->next;
            if( add_err )
            {
              add_err = 0;
              return 0;
            }
          }
        else
        {
          prev = r;
          r = r->next;
        }
      }
      while( r );
  return 1;
}

void regenerate_rects( Window *wind, int always )
{
  Window *w;
  Rect_list **root, *r;
  Rect rect1;
  APP *ap;

  if( *(root = &wind->rects) != 0 && !always )	/* 004: reworked */
    if( update.i || (ap = find_ap(wind->apid)) != 0 &&
        ap->flags.flags.s.optim_redraws ) return;
  free_rects( wind, 1 );
  if( (r=add_rect( 0L, root )) == 0 )
  {
    add_err = 0;
    return;
  }
  r->r = wind->outer;
  (*root)->r.w += 2;
  (*root)->r.h += 2;
  if( wind==desktop ) (*root)->r = wind->working;
  w = desktop;
  while( (w = w->next) != 0 && *root )
    if( w->place > wind->place )
    {
      rect1 = add2rect(&w->outer);
      if( !gen_rect( &rect1, root ) ) return;
    }
}

int intersect( Rect r1, Rect r2, Rect *res )
{
  res->x = r1.x < r2.x ? r2.x : r1.x;
  res->y = r1.y < r2.y ? r2.y : r1.y;
  res->w = r1.x+r1.w < r2.x+r2.w ? r1.x+r1.w-res->x : r2.x+r2.w-res->x;
  res->h = r1.y+r1.h < r2.y+r2.h ? r1.y+r1.h-res->y : r2.y+r2.h-res->y;
  return( res->w > 0 && res->h > 0 );
}

int _find_wind( int x, int y )
{
  Window *w;
  int pl=-1, h=-1;

  w = desktop;
  do
    if( x >= w->outer.x && x < w->outer.x+w->outer.w &&
        y >= w->outer.y && y < w->outer.y+w->outer.h &&
        w->place > pl )
    {
      pl = w->place;
      h = w->handle;
    }
  while( (w=w->next) != 0 );
  return conv_handle( h, 0 );
}

int x_wind_tree( int mode, WIND_TREE *wt )
{
  Window *w;
  int handle;

  if( test_update( (void *)x_wind_tree ) ) return 0;
  handle = conv_handle( wt->handle, 1 );
  if( (w = find_window(handle)) == 0 )
  {
    DEBUGGER(XWTRE,INVHAND,handle);
    return 0;
  }
  switch( mode )
  {
    case X_WT_GETCNT:
      wt->count = w->treecnt;
      wt->flag = w->treeflag;
      break;
    case X_WT_READ:
      memcpy( wt->tree, w->tree, w->treecnt*sizeof(OBJECT) );
      if( w->tree == dflt_wind ) wt->tree[WINFO].ob_spec.tedinfo =
          wt->tree[WMOVE].ob_spec.tedinfo = 0L;
      break;
    case X_WT_SET:
      if( wt->count>=0 )
      {
        w->tree = wt->tree;
        w->treecnt = wt->count;
      }
      w->treeflag = wt->flag;
      break;
    default:
      DEBUGGER(XWTRE,UNKTYPE,mode);
      return 0;
  }
  return 1;
}

void get_split( Window *wind, int *i, int dir )
{
  int h, h1;

  h1 = (dir ? wind->working.h : wind->working.w) -
      (dir ? wind->tree[WVSPLIT].ob_height :
      wind->tree[WHSPLIT].ob_width) + 2;
  *i++ = (h = dir ? wind->vsplit : wind->hsplit);
  *i++ = !h ? 0 : (h>0 ? h : h1)-1;	/* added -1 for rel 3 */
  *i   = h>=0 ? h1-h-1 : 0;
}

void get_oldcol( int colors[2][WGSIZE+1], int num, int *out ) /* 004 */
{
  int i;

  for( i=0; i<=WGSIZE; i++ )
    if( wcol_xref[i] == num )
    {
      *out++ = colors[1][i];
      *out = colors[0][i];
      return;
    }
}

int get_window( int handle, int type, int *i )
{
  Window *wind, *w;
  int h, h1, pl, pl2, id;
  Rect *r, rect1;

  handle = conv_handle( handle, 1 );
  if( (wind = find_window(handle)) == 0 && type!=WF_SCREEN &&
      type!=WF_TOP && type!=WF_NEWDESK && type!=WF_DCOLOR &&
      type!=X_WF_DCOLSTAT/*004*/ )
  {
    if( type==WF_FIRSTXYWH || type==WF_NEXTXYWH ) *(Rect *)i = norect;
    DEBUGGER(WNGET,INVHAND,handle);
    return(0);
  }
  switch( type )
  {
    case WF_CURRXYWH:
      r = &wind->outer;
      if( wind->place<0 ) goto norec;
      goto rec;
    case WF_PREVXYWH:
      r = &wind->prev;
      goto rec;
    case WF_FULLXYWH:
      r = &wind->full;
      goto rec;
    case WF_WORKXYWH:
      r = &wind->working;
rec:  *(Rect *)i = *r;
      break;
    case WF_FIRSTXYWH:
firstxy:
      if( wind->place>=0 && (wind!=desktop || curapp==has_desk || curapp==ddesk_app) )
      {
        regenerate_rects( wind, 1 );	/* 004: use 1 */
        recalc_window( handle, wind, -1L&~((long)X_MENU<<8) );
        if( wind->rects )
        {
          if( wind->xtype&X_VSPLIT )
          {
            objc_xywh( (long)wind->tree, WVSPLIT, &rect1 );
            if( !gen_rect( &rect1, &wind->rects ) ) goto norec;
          }
          if( wind->xtype&X_HSPLIT )
          {
            objc_xywh( (long)wind->tree, WHSPLIT, &rect1 );
            if( !gen_rect( &rect1, &wind->rects ) ) goto norec;
          }
        }
        wind->rectptr = wind->rects;
      }
      else
      {
norec:  r = &norect;
        goto rec;
      }
    case WF_NEXTXYWH:
      for(;;)
        if( !wind->rectptr ) goto norec;
        else if( !wind->rects ) goto firstxy;	/* 004 */
        else
        {
          r = &(wind->rectptr->r);
          if( (wind->rectptr = wind->rectptr->next) == 0L )
              free_rects( wind, 1 );		/* 004: because of gen_rects */
          if( intersect( wind->working, *r, &rect1 ) )
          {
            r = &rect1;		/* 004 */
            goto rec;
          }
        }
    case WF_HSLIDE:
      *i = wind->hslide;
      break;
    case WF_VSLIDE:
      *i = wind->vslide;
      break;
    case WF_HSLSIZE:
      *i = wind->hslidesz;
      break;
    case WF_VSLSIZE:
      *i = wind->vslidesz;
      break;
    case X_WF_HSPLIT:
      get_split( wind, i, 0 );
      break;
    case X_WF_VSPLIT:
      get_split( wind, i, 1 );
      break;
    case X_WF_HSLIDE2:
      *i = wind->hslide2;
      break;
    case X_WF_VSLIDE2:
      *i = wind->vslide2;
      break;
    case X_WF_HSLSIZE2:
      *i = wind->hslidesz2;
      break;
    case X_WF_VSLSIZE2:
      *i = wind->vslidesz2;
      break;
    case X_WF_SPLMIN:
      *i++ = wind->hsp_min1;
      *i++ = wind->hsp_min2;
      *i++ = wind->vsp_min1;
      *i   = wind->vsp_min2;
      break;
    case WF_TOP:
      pl = pl2 = -1;
      h = h1 = 0;
      if( !no_top )
      {
        w = desktop;
        do
          if( w->place > pl )
          {
            pl = w->place;
            h = w->handle;
            id = w->apid;
          }
        while( (w=w->next) != 0 );
        w = desktop;
        do
          if( w->place > pl2 && w->place < pl )
          {
            pl2 = w->place;
            h1 = w->handle;
          }
        while( (w=w->next) != 0 );
      }
      *i++ = conv_handle( h, 0 );
      *i++ = id;
      *i = conv_handle( h1, 0 );
      break;
    case WF_NEWDESK:
      *(OBJECT **)i = &curapp->desk[curapp->desk_obj];
      break;
    case WF_SCREEN:
      *((long *)i)++ = pull_buf.l;
      *(long *)i = pull_siz.l;
      break;
    case WF_COLOR:
      get_oldcol( wind->colors, *i, i+1 );
      break;
    case WF_DCOLOR:
      get_oldcol( dwcolors[wcolor_mode], *i, i+1 );
      break;
    case X_WF_DCOLSTAT:	/* 004 */
      h = *i++;
      *i++ = dwcolors[wcolor_mode][1][h];
      *i++ = dwcolors[wcolor_mode][0][h];
      *i = u_object(dflt_wind,h)->ob_state;
      break;
    case WF_OWNER:
      *i++ = wind->apid;
      *i++ = wind->place>=0;
      w = desktop;
      *i = *(i+1) = 0;
      do
        if( w->place == wind->place+1 ) *i = conv_handle( w->handle, 0 );
        else if( w->place == wind->place-1 ) *(i+1) = conv_handle( w->handle, 0 );
      while( (w=w->next) != 0 );
      break;
    case WF_BEVENT:
      *i = wind->bevent;
      break;
    case WF_BOTTOM:
      w = desktop;
      *i = 0;
      do
        if( w->place == 1 ) *i = conv_handle( wind->handle, 0 );
      while( (w=w->next) != 0 );
      break;
    case WF_ICONIFY:
      *i++ = wind->icon_index!=0;
      *i++ = ICON_WID;
      *i   = ICON_HT;
      break;
    case WF_UNICONIFYXYWH:		/* 005: possibly not needed */
    case WF_UNICONIFY:
      r = &wind->iconify;
      goto rec;
    case X_WF_MENU:
      *(OBJECT **)i = wind->menu;
      break;
    case X_WF_DIALOG:
      *(OBJECT **)i = wind->dialog;
      break;
    case X_WF_DIALWID:
      *i = wind->dial_swid;
      break;
    case X_WF_DIALHT:
      *i = wind->dial_sht;
      break;
    case X_WF_DIALFLGS:
      *i = (wind->treeflag & (X_WTFL_ACTIVE|X_WTFL_BLITSCRL)) >> 3;
      break;
    case X_WF_MINMAX:
      *i++ = wind->min_w;
      *i++ = wind->min_h;
      *i++ = wind->max_w;
      *i   = wind->max_h;
      break;
    case X_WF_OBJHAND:
      *(int cdecl (**)(int hand, int obj))i = wind->objhand;
      break;
    case X_WF_DIALEDIT:
      *i++ = wind->dial_edit;
      *i   = wind->dial_edind;
      break;
    default:
      DEBUGGER(WNGET,UNKTYPE,type);
      return 0;
  }
  return(1);
}

void redraw_obj( Window *w, int num, Rect *ir )
{
  Rect r, r2;
  Rect_list *p;
  int n;

  if( (n=num)<0 ) num = 0;	/* 005 */
  if( is_hid(w->tree,num) ) return;
  if( !ir )
  {
    objc_xywh( (long)w->tree, num, &r );
    r.w += 2;
    r.h += 2;
    ir = &r;
  }
  p = w->rects;
  _v_mouse(0);	/* 004 */
  while( p )
  {
    if( intersect( *ir, p->r, &r2 ) )
    {
      _objc_draw( (OBJECT2 *)w->tree, 0L, num, 8, Xrect(r2) );
      if( !n || n==WMENU/*005:use n*/ ) drw_win_menu( w, &r2, draw_menu_alts );
    }
    p = p->next;
  }
  _v_mouse(1);	/* 004 */
}

int still_pressed( Window *w, int num )
{
  Mouse m;
  register int i, last, first;
  OBJECT *o = u_object(w->tree,num);

  if( (i=o->ob_flags)&SELECTABLE )
    if( i&TOUCHEXIT )
    {
      if( !(o->ob_state&SELECTED) )
      {
        o->ob_state |= SELECTED;
        redraw_obj( w, num, 0L );
      }
    }
    else for( last=num, first=1;;)
    {
      mks_graf( &m, 1 );
      if( !(m.b&1) )
      {
        o->ob_state &= ~SELECTED;
        if( last==num ) redraw_obj( w, num, 0L );
        return(last==num);
      }
      i = objc_find( w->tree, 0, 8, m.x, m.y );
      if( i!=last && (i==num || last==num) || first )
      {
        first=0;
        if( i != num ) o->ob_state &= ~SELECTED;
        else o->ob_state |= SELECTED;
        redraw_obj( w, num, 0L );
        last = i;
      }
    }
  return(1);
}

void all_gadgets( int pl )
{
  Window *w;

  for( w=desktop->next; w; w=w->next )
    if( w->place == pl )
    {
      recalc_window( w->handle, w, -1L );
      regenerate_rects( w, 0 );
      redraw_obj( w, 0, 0L );
      return;
    }
}

void reset_butq(void)
{
  but_q[bq_last].x = g_mx;
  but_q[bq_last].y = g_my;
  but_q[bq_last].k = *kbshift;
  bq_ptr=bq_last;
  unclick=not_taken=0;
}

char norecord;		/* 004 */

void _record( int num, long val )
{
  char *buf = (char *)recbuf;
  union
  {
    unsigned char c[4];
    long l;
  } u;

  if( norecord ) return;
  if( !recmode ) *buf++ = *buf++ = 0;		/* treat as long is correct, contrary to docs! */
  *buf++ = 0;
  *buf++ = num;
  u.l = val;
  *buf++ = u.c[0];
  *buf++ = u.c[1];
  *buf++ = u.c[2];
  *buf++ = u.c[3];
  recbuf = (void *)buf;
  if( !--recnum )
    if( recmode ) norecord++;
    else recorder = 0L;
}

void record_event( int num, long val )
{
  char k;

  if( tic>rec_tic+1 )		/* 004: added 1 */
  {
    _record( 0, (tic-rec_tic-1)*ticcal );
    if( !recnum ) return;
    rec_tic = tic;
  }
  if( num != 3 && (k=*kbshift) != rec_shift )
  {
    _record( 3, (long)k<<16 );
    if( !recnum ) return;
    rec_shift = k;
  }
  _record( num, val );
}

void clicked( int mb )
{
  int i;

  if( g_mb == mb ) return;
  if( recorder ) record_event( 1, ((long)mb<<16)+1 );
  i = bq_last;
  if( ++i == MAX_BUTQ ) i = 0;
  if( i == bq_ptr )
    if( ++bq_ptr == MAX_BUTQ ) bq_ptr = 0;
  but_q[i].x = g_mx;
  but_q[i].y = g_my;
  but_q[i].b = g_mb = mb;
  but_q[i].k = *kbshift;
  but_q[bq_last=i].time = tic;
  if( !(mb&1) ) gadget_ok=0;
}

unsigned int get_int( unsigned char *buf )	/* 005: unsigned */
{
  return (*buf<<8) | *(buf+1);
}

long get_long( unsigned char *buf )
{
  int i;

  i = get_int(buf);
  return ((long)i<<16) | get_int(buf+2);
}

void ticked(void)
{
  APP *ap;
  long l;
  int *mouse, *form, i, j, dum;
  char *p;
  static char ctran[] = { 0, 15, 1, 2, 4, 6, 3, 5, 7, 8, 9, 10,
      12, 14, 11, 13 };
  static long play_pause;

  if( !no_interrupt )
  {
    retrap_vdi();		/* 004: check on trap #2 */
    for( ap=app0; ap; ap=ap->next )
      if( ap->type&MU_TIMER )
      {
        l = ((long)ap->high<<16)|(unsigned int)(ap->low);
        if( (l -= ticcal) < 0 ) l = 0;
        ap->low = l;
        ap->high = l>>16;
      }
    if( recorder && (g_mx!=rec_mx || g_my!=rec_my) )
        record_event( 2, ((long)(rec_mx=g_mx)<<16L)|(rec_my=g_my) );
    if( player )
    {
      /* first field as long is correct, contrary to docs! */
      p = (char *)playbuf;
      if( !playmode ) p+=2;
      if( !play_pause ) switch( *(p+1) )
      {
        case 0:
          if( tic-play_tic < get_long((unsigned char *) (p + 2)) / ticcal ) break;
          goto next;
        case 1:
          clicked( *(p+3) );
          goto next;
        case 2:
          /*move_mouse( i=get_int(p+2), j=get_int(p+4) );
          vrq_locator( vdi_hand, i, j, &dum, &dum, &dum ); */
          (*play_motv)(i=get_int((unsigned char *) (p + 2)), j=get_int((unsigned char *) (p + 4)) );
          (*play_curv)( i, j );
          Vdiesc->cur_x = Vdiesc->gcurx = i;
          Vdiesc->cur_y = Vdiesc->gcury = j;
          goto next;
        case 3:
          if( play_key ) break;
          else
          {
            play_key = get_long((unsigned char *) (p + 2));
            *kbshift = (*kbshift&0x10) | (int)(play_key>>16L) | ((g_mb^3)<<5);
            if( !(int)play_key ) play_key = 0L;
          }
        default:	/* 004 */
next:     if( !--playnum )
            if( playmode ) norecord++;
            else no_player();
          else
          {
            play_tic = tic;
            (char *)playbuf = p+6;
            play_pause = playrate;
          }
          break;
      }
      else play_pause--;
    }
  }
  if( !sleeping && ani_mouse && Vdiesc && Vdiesc->m_hid_ct<=0 )
    if( !ani_delay-- )
    {
      mouse = &Vdiesc->m_pos_hx;
      form = (int *)(&ani_mouse->form[ani_frame]);
      *((long *)mouse)++ = *((long *)form)++;
      *mouse++ = *form++;
      *mouse++ = ctran[*form++];
      *mouse++ = ctran[*form++];
      for( i=16; --i>=0; )
      {
        *mouse++ = *form;
        *mouse++ = *(form+16);
        form++;
      }
      Vdiesc->cur_flag = 1;
      ani_delay = ani_mouse->delay;
      if( ++ani_frame >= ani_mouse->frames ) ani_frame = 0;
    }
    else if( ani_delay<0 ) ani_delay=0;
  tic++;
}

void set_butq(void)
{
  unclick = new_un;
  bq_ptr = new_ptr;
  cur_last = bq_last;
  not_taken=0;
}

void gadget_off(void)
{
  Window *w;
  unsigned int *i;

  if( last_gadget )
  {
    if( (w = find_window( last_gad_w ))->place >= 0 &&
        *(i=&u_object(w->tree,last_gadget)->ob_state)&SELECTED )
    {
      recalc_window( last_gad_w, w, 0L );
      regenerate_rects( w, 0 );
      *i &= ~SELECTED;
      redraw_obj( w, last_gadget, 0L );
    }
    last_gadget = 0;
  }
}

void set_gadget(void)
{
  if( last_gadget )
    if( !(g_mb&1) ) gadget_off();
    else
    {
      if( !gadget_ok ) gadget_tic = ngad_tic;
      gadget_ok = 1;
    }
}

int get_butq( int mask, int state, int clicks, int flag )
{
  int h1, h3, h4, times, pause, first=1, both=0;
  long l, m;

  if( !mask && !state ) flag = 1;
  if( clicks&0xFF00 )
  {
    both++;
    clicks &= 0xff;
  }
  pause = clicks>1 ? dc_pause : 0;
  if( clicks<1 ) clicks=1;
  if( !mask ) unclick = 0;
  else if( not_taken ) unclick = old_un;
  else old_un = unclick;
  for( times=h4=0, l=but_q[h1=bq_ptr].time;; )
  {
    h3 = but_q[h1].b&mask;
    if( both ? (!state ? h3==state : ((h3&state) != 0)) :
        (h3 != state) )
    {
      if( unclick )
      {
        unclick=0;
        times=0;
        if( bq_ptr != h1 )
        {
          bq_ptr = h1;
          old_un = 0;
        }
      }
      h4=0;
    }
    else if( !h4 )
    {
      if( !unclick )
      {
        if( first ) ngad_tic = but_q[h1].time;
        first=0;
      }
      h4 = 1;
      ++times;
    }
    m = but_q[h1].time;
    if( h1!=bq_last )
    {
      h3 = h1;
      if( ++h3 == MAX_BUTQ ) h3=0;
      if( unclick ) times = 0;
      if( times >= clicks )
      { /* take the unclick now, if it's there */
        if( !both )
          if( (but_q[h3].b&mask) != state || !mask && !state ) h1 = h3;
        break;
      }
      if( first ) l=but_q[h3].time;
      h1 = h3;
      m = but_q[h1].time;
    }
    else
    {
      if( flag || times && clicks<2 ) break;
      if( tic-l <= pause )
      {
        times = -1;
        break;
      }
      m = tic;
    }
    if( m - l > pause ) break;
  }
  if( times )
  {
    h3 = but_q[h1].b&mask;
    new_un = (both ? (!state ? h3!=state : ((h3&state)==0)) :
        (h3==state)) && mask;
  }
  else new_un = unclick;
  new_ptr = h1;
  not_taken = 1;
  return(times);
}

void multi( int *buf, EMULTI *emulti )
{
  memcpy( &curapp->type, emulti, 16*sizeof(int) );
/*  if( curapp->type & X_MU_DIALOG ) curapp->type &= ~(MU_KEYBD|MU_BUTTON); */
  curapp->buf = buf;
  curapp->apread_cnt = 16;
  curapp->apread_id = curapp->id;
  curapp->state &= curapp->mask;
  if( curapp->blocked )
  {
    curapp->blocked = curapp->type;
    curapp->type = curapp->type&MU_MESAG;
  }
}

void recstop( APP **ap )
{
  if( *ap )
  {
    newtop( X_WM_RECSTOP, ap==&recorder ?
        ((char *)recbuf-(char *)recbuf0)/(recmode?6:8): 0, (*ap)->id );
    if( ap==&player ) no_player();
    else *ap = 0L;
    norecord = 0;
  }
}

int has_key(void)
{
  if( norecord )         	/* 004 */
  {
    recstop( &recorder );
    recstop( &player );
  }
  return play_key || Bconstat(2);
}

long getkey(void)
{
  char sh, *ptr;
  long k;

  if( play_key )
  {
    k = ((play_key&0x000FFF00)<<8) | (play_key&0xFF);
    play_key = 0L;
  }
  else
  {
    k = Bconin(2)&0xFFFFFFL;
    if( (sh = *(ptr=kbbuf+(kbio->ibufhd>>2))) < 0 ) sh=*kbshift&0xf;	/* 004: added & */
    else *ptr = -1;
    k |= (long)sh<<24L;
  }
  newtop( X_WM_VECKEY, 0, (*g_vectors.keypress)(&k) );	/* 004 */
  if( recorder )
    if( is_key( &rec_stop, k>>24L, (int)((k&0xFF0000L)>>8)|(unsigned char)k ) )
    {      	/* 004 */
      recstop( &recorder );
      k = 0L;
    }
    else if( k/*004*/ ) record_event( 3, ((k&0xFFFF0000L)>>8)|(unsigned char)k );
  return k;
}

int prev_obj( OBJECT *o, int num, int i )
{
  if( (i = u_object(o,i)->ob_head) == num ) return num;
  while( u_object(o,i)->ob_next != num ) i = u_object(o,i)->ob_next;
  return i;
}

void eat_clicks( APP *ap )
{
  while( bq_ptr != bq_last && !(but_q[bq_ptr].b&ap->mask) )
    if( ++bq_ptr == MAX_BUTQ ) bq_ptr = 0;
}

int wind_menu(void)
{
  Window *w, *d=desktop;

  w = top_wind;
  if( w->dial_obj==IS_TEAR || !w->menu || w==d ) w=0;
  if( !d->menu ) d=0;
  if( cur_menu!=d ) next_menu=d;
  else next_menu=w;
  return next_menu!=0;
}

void menu_right( Window *w )
{
  if( w->menu && w->menu_tZ>0 )
  {
    if( !w->menu_tA ) w->menu_tA = w->menu[2].ob_head;
    w->menu_tA = u_object(w->menu,w->menu_tA)->ob_next;
    if( w->menu_tZ )
      if( (w->menu_tZ = u_object(w->menu,w->menu_tZ)->ob_next) ==
          w->menu[2].ob_tail ) w->menu_tZ = 0;
    draw_wmenu(w);
  }
}

void menu_left( Window *w )
{
  if( w->menu && w->menu_tA>0 )
  {
    if( (w->menu_tA=prev_obj(w->menu,w->menu_tA,2)) ==
        w->menu[2].ob_head ) w->menu_tA=0;
    if( w->menu_tZ ) w->menu_tZ = prev_obj(w->menu,w->menu_tZ,2);
    draw_wmenu(w);
  }
}

int mouse_ok( APP *ap, int use_ap, Window **w )
{
  int id;

  *w = find_window( _find_wind( ap->mouse_x, ap->mouse_y ) );
  if( !use_ap )
  {
    if( !(*w)->dialog && !(*w)->icon_index || (*w)->place<0 ) return 0;
  }
  else if( !update.i && (*w)->dialog && ap->type&X_MU_DIALOG ) return 0;	/* added for 004 */
  if( (!update.i ?  (!ap->state/*004*/ && !(ap->clicks&0xff00)/*004*/ ||
      (id=(*w)->apid) == ap->id || !use_ap && id<0)  : has_update==ap) &&
      (use_ap && (char)(ap->clicks)<2 ||
      but_q[bq_last].time-but_q[bq_ptr].time >= dc_pause ||
      tic-but_q[bq_ptr].time >= dc_pause) && cur_last==bq_last )
    if( (ap->times = use_ap ? get_butq( ap->mask, ap->state, ap->clicks, 0 ) :
        get_butq( 1, 1, 2, 0 )) == 0 )
    {
      if( !(*w)->icon_index ) eat_clicks(ap);
    }
    else if( ap->times==-1 ) return 0;
    else if( cur_last==bq_last ) return 1;
  return 0;
}

void win_clip_ini( Rect *r )
{
  if( !r || (wclip_big.w = r->w)==0 || (wclip_big.h = r->h)==0 )
      wclip_big = desktop->outer;
  else
  {
    wclip_big.x = r->x;
    wclip_big.y = r->y;
  }
  get_window( wclip_win->handle, WF_FIRSTXYWH, (int *)&wclip_rec );
  _v_mouse(0);
}

int win_clip(void)
{
  int ok=0;

  for(;;)
  {
    if( !wclip_rec.w ) return 0;
    if( intersect( wclip_big, wclip_rec, &wclip_rec ) )
    {
      do_wclip();
      ok++;
    }
    get_window( wclip_win->handle, WF_NEXTXYWH, (int *)&wclip_rec );
    if( ok ) return 1;
  }
}

int _x_wdial( int hand, int start, int parm, int flag )
{
  Window *w;
  int i;

  hand = conv_handle( hand, 1 );
  wind_app = 0L;
  w = find_window(hand);
  if( w && w->apid == curapp->id && w->dialog && w->place>=0 )
  {
    wclip_win = w;
    clip_ini = win_clip_ini;
    clip_it = win_clip;
    if( !flag ) i = _objc_draw( (OBJECT2 *)w->dialog, 0L, start, parm, 0, 0, 0, 0 );
    else i = change_objc( w->dialog, 0L, start, &norect, parm, 1 );
    reset_clip();
    return i;
  }
  DEBUGGER(flag?XWDIALC:XWDIALD,INVHAND,hand);
  return 0;
}

int x_wdial_draw( int hand, int start, int depth )
{
  if( test_update( (void *)x_wdial_draw ) ) return 0;
  return _x_wdial( hand, start, depth, 0 );
}

int x_wdial_change( int hand, int start, int newstate )
{
  if( test_update( (void *)x_wdial_change ) ) return 0;
  return _x_wdial( hand, start, newstate, 1 );
}

void init_win_dial( Window *w )
{
  wclip_win = w;
  clip_ini = win_clip_ini;
  clip_it = win_clip;
  form_reinit( w->dial_obj, w->dial_edit, w->dial_edind, w->place==place );
}

void exit_win_dial( Window *w )
{
  w->dial_obj = next_obj;
  w->dial_edit = edit_obj;
  w->dial_edind = edit_idx;
  reset_clip();
}

int win_dial( Window *w, int *buf, APP *ap )
{
  int i;

  if( w && w->apid == ap->id && w->dialog && w->place>=0 )
  {
    if( !(w->treeflag&X_WTFL_ACTIVE) )
    {
      ring_bell();
      return 0;
    }
    init_win_dial( w );
    edit_curs( w->dialog, 0, 0 );
    if( (i = form_event( (long)w->dialog, wind_dial, 0 )) != 0 )
    {
      edit_curs( w->dialog, 1, 1 );
      edit_curs( w->dialog, 0, 0 );
    }
    exit_win_dial( w );
    if( !i )
    {
      ap->event |= X_MU_DIALOG;
      *(OBJECT **)&buf[0] = w->dialog;
      buf[2] = next_obj;
      buf[3] = w->handle;
      w->dial_obj = 0;
      return 1;
    }
    return i==2/*004*/ ? -2 : -1;
  }
  return 0;
}

int test_wdial( Window *w )
{
  return w->dialog &&
      (w->treeflag&(X_WTFL_CLICKS|X_WTFL_ACTIVE)) == (X_WTFL_CLICKS|X_WTFL_ACTIVE);
}

int scroll_wdial( Window *w, int flag, int val, int realtime )
{
  int i, j, imax, jmax, a;
  Rect r1, r2, r3, redraw;

  if( test_wdial(w) )
  {
    i = w->dialog[0].ob_x;
    j = w->dialog[0].ob_y;
    if( (imax=w->dialog[0].ob_width-w->working.w) <= 0 ) imax = 0;
    if( (jmax=w->dialog[0].ob_height-w->working.h) <= 0 ) jmax = 0;
    redraw = r1 = r2 = w->working;
    switch( flag )
    {
      case WA_LFPAGE:
        i += w->working.w;
        goto hor;
      case WA_RTPAGE:
        i -= w->working.w;
        goto hor;
      case WA_LFLINE:
        i += w->dial_swid;
        goto hor;
      case WA_RTLINE:
        i -= w->dial_swid;
        goto hor;
      case WM_HSLID:
        i = w->working.x - (!realtime ? (long)imax*val/1000L : val);
hor:    i = (i-w->working.x)/w->dial_swid*w->dial_swid + w->working.x;
        if( i<w->working.x-imax ) i = w->working.x - imax;
        else if( i>w->working.x ) i = w->working.x;
        if( w->dialog[0].ob_x==i ) return 1;
        intersect( desktop->outer, w->working, &r3 );
        if( w->treeflag&X_WTFL_BLITSCRL && w->place==place &&
            (a=abs(j=i-w->dialog[0].ob_x)) < w->working.w &&
            *(long *)&r3.x == *(long *)&w->working.x &&
            *(long *)&r3.w == *(long *)&w->working.w )
        {
          r1.w = r2.w -= (redraw.w=a);
          if( j>0 ) r2.x += j;
          else
          {
            r1.x -= j;
            redraw.x = r2.x+r2.w;
          }
          x_graf_blit( (GRECT *)&r1, (GRECT *)&r2 );
        }
        w->dialog[0].ob_x = i;
        break;
      case WA_UPPAGE:
        j += w->working.h;
        goto vert;
      case WA_DNPAGE:
        j -= w->working.h;
        goto vert;
      case WA_UPLINE:
        j += w->dial_sht;
        goto vert;
      case WA_DNLINE:
        j -= w->dial_sht;
        goto vert;
      case WM_VSLID:
        j = w->working.y - (!realtime ? (long)jmax*val/1000L : val);
vert:   j = (j-w->working.y)/w->dial_sht*w->dial_sht + w->working.y;
        if( j<w->working.y-jmax ) j = w->working.y - jmax;
        else if( j>w->working.y ) j = w->working.y;
        if( w->dialog[0].ob_y==j ) return 1;
        intersect( desktop->outer, w->working, &r3 );
        if( w->treeflag&X_WTFL_BLITSCRL && w->place==place &&
            (a=abs(i=j-w->dialog[0].ob_y)) < w->working.h &&
            *(long *)&r3.x == *(long *)&w->working.x &&
            *(long *)&r3.w == *(long *)&w->working.w )
        {
          r1.h = r2.h -= (redraw.h=a);
          if( i>0 ) r2.y += i;
          else
          {
            r1.y -= i;
            redraw.y = r2.y+r2.h;
          }
          x_graf_blit( (GRECT *)&r1, (GRECT *)&r2 );
        }
        w->dialog[0].ob_y = j;
        break;
    }
    dial_sliders( w, !realtime );
    redraw_window( w->handle, &redraw, 0, 1 );
    return 1;
  }
  return 0;
}

int wslider( Window *w, int dir )
{
  int i, big, small, mult;

  if( !test_wdial(w) || !(w->type&(HSLIDE|VSLIDE)) ) return 0;
  mult = !dir ? w->dial_swid : w->dial_sht;
  if( graf_slidebox( 0L, !dir ? w->dialog[0].ob_width/mult :
      w->dialog[0].ob_height/mult, !dir ? w->working.w/mult :
      w->working.h/mult, 0x100|dir ) >= 0 )
  {
    small = !dir ? WHSMLSL : WVSMLSL;
    sel_if(w->tree,small,1);
    redraw_obj( w, small, 0L );
    big = !dir ? WHBIGSL : WVBIGSL;
    i = graf_slidebox( w->tree, big, small, 0x200|dir );
    while( i>=0 )
    {
      scroll_wdial( w, !dir ? WM_HSLID : WM_VSLID, i*mult, 1 );
      i = graf_slidebox( w->tree, big, small, 0x300|dir );
    }
    sel_if(w->tree,small,0);
    regenerate_rects( w, 0 );
    redraw_obj( w, small, 0L );
  }
  return 1;
}

void fix_msg( APP *ap, int *buf, int is_old/*004*/ )
{
  int *f;
  static int fix[] = { WM_REDRAW, WM_TOPPED, WM_CLOSED, WM_FULLED,
      WM_ARROWED, WM_HSLID, WM_VSLID, WM_SIZED, WM_MOVED, WM_UNTOPPED,
      WM_ONTOP, X_WM_SELECTED/*004*/, X_WM_HSPLIT, X_WM_VSPLIT, X_WM_ARROWED2,
      X_WM_HSLID2, X_WM_VSLID2, X_WM_OBJECT, WM_ICONIFY/*004*/,
      WM_ALLICONIFY/*004*/, WM_UNICONIFY/*004*/, WM_TOOLBAR/*004*/, 0 };

  if( ap->flags.flags.s.limit_handles && !buf[2]/*004*/ )
    if( ap->event&X_MU_DIALOG ) goto fix_it;
    else if( buf[0]==(signed)X_MN_SELECTED )	/* 004 */
    {
      wind_app = ap;
      buf[7] = conv_handle( buf[7], is_old );
    }
    else
      for( f=fix; *f; f++ )
        if( *f==buf[0] )
        {
fix_it:   wind_app = ap;
          buf[3] = conv_handle( buf[3], is_old );	/* let set_stack clear wind_app */
          return;
        }
  if( ap->neo_le005 && buf[0]==CH_EXIT ) buf[0] = 80;	/* 007: old CH_EXIT */
}

int cycle_wind(void)
{
  APP *ap, *ap2;

  if( place && !no_top && top_wind->apid>=0 )
      for( ap=app0; ap && ap->id!=top_wind->apid; ap=ap->next );
  else if( (ap=has_menu)==0L ) return 0;
  if( ap ) for( ap2=ap->next; ap2!=ap; )
    if( !ap2 ) ap2 = app0;
    else if( !ap2->asleep && ap2->menu && ap2!=has_menu )
    {
      switch_menu(ap2);
      return 1;
    }
    else if( !ap2->asleep && ap2->has_wind &&
        (no_top || top_wind->apid!=ap2->id) && cycle_top(ap2) ) return 1;
    else ap2 = ap2->next;
  return 0;
}

int sw_next_app( APP *curr )
{
  APP *ap2;

  if( !has_menu || !multitask ) return 0;
  if( curr && curr->parent_id != 1 )	/* added parent search for 004 */
    if( (ap2 = find_ap(curr->parent_id)) != 0 )
      if( ap2->menu && !ap2->asleep )
      {
        switch_menu(ap2);
        return 1;
      }
  for( ap2=has_menu->next; ap2!=has_menu; )
    if( !ap2 ) ap2 = app0;
    else if( ap2->menu && !ap2->asleep )
    {
      switch_menu(ap2);
      return 1;
    }
    else ap2 = ap2->next;
  return 0;
}

APP *dis_init( int set, int no_evnt, long stack )
{
  APP *ap;

  if( curapp )
  {
    if( set )
    {
      curapp->stack = stack;
      if( (curapp->no_evnt = no_evnt) != 0 ) curapp->type = 0;
    }
    if( !(curapp->type&MU_MESAG) ) curapp->buf = nobuf;
    if( !no_evnt && apps_initial && curapp->start_end ) /* is DA initializing? */
    {
      curapp->start_end = 0;
      apps_initial--;
    }
    if( !preempt )
    {
      if( loading ) loading = load_acc(0);  /* only returns when out of DA's */
      if( !apps_initial ) shel_exec();
    }
    ap=curapp->next;
  }
  else ap = 0L;
  move_kbbuf();
  _x_appl_free();
  return ap;
}

void wait_up( APP *ap )
{
  do
    get_mks();
  while( ap->mouse_b&1 );
}

void top_all( APP *ap, Window *w, int *buf, int skip )
{
  int p, n;
  Window *w2;

  wait_up(ap);
  buf[0] = WM_TOPPED;
  buf[1] = w->apid;
  buf[2] = 0;
  if( w->dial_obj == IS_TEAR )
  {
    buf[3] = w->handle;
    ap->event |= MU_MESAG;
    menu_evnt( ap, buf );
    return;
  }
  for( n=0, w2=desktop; (w2=w2->next) != 0; )	/* 005 */
    if( w2->apid==w->apid ) n++;
  for( p=1; p<=place; p++ )	/* 005: used to start at 0 */
    for( w2=desktop; (w2=w2->next) != 0; )
      if( w2->place==p )
      {
        if( w2->apid==w->apid )
        {
          n--;					/* changed lots for 005 */
          if( skip && w2==w ) n--;
          else if( p!=place-n || !skip && no_top && !n )
          {
            buf[3] = w2->handle;
            _appl_write( buf[1], 16, buf, 1 );
          }
        }
        break;
      }
  if( skip ) newtop( WM_TOPPED, w->handle, w->apid );	/* 005 */
}

int gad_zone( int gadget )	/* 005 */
{
  return gadget>=WUP2 && gadget<=WDOWN2 || gadget>=WRT2 && gadget<=WLEFT2 ?
      X_WM_ARROWED2 : WM_ARROWED;
}

long _dispatch( APP *ap )
{
  int h1, h2, h3, h4, x, y, buf[8];
  Window *w, *w2;
  static unsigned char eq_tran[] = { WVBIGSL, WVBIGSL, WUP, WDOWN, WHBIGSL,
      WHBIGSL, WLEFT, WRT, WCLOSE, WBACK, WFULL, WILEFT, WIRT };
  Rect r;
  APP *ap2;
  long l;
  MSGQ *msg, *msg2;
  char no_copy=0;
  static KEYCODE mouse_reset = { 0xF, 0x71, 0 };

  buf[2] = 0;	/* 004 */
  for( ;;ap=ap->next )
  {
#ifdef MINT_DEBUG
/*    Cconws( "\033j\033H\033CD\033k" );*/
#endif
    if( preempt )
    {
/*      Psemaphore( 3, UPDT_SEM, 0 );*/
      while( loading )
      {
        for( ap2=app0; ap2; ap2=ap2->next )
          if( ap2->waiting<0 ) break;
        if( ap2 ) Syield();
        else loading = load_acc(0);
      }
      if( block_loop && update.i )	/* 005: wait for wind_update to end */
      {
        block_loop = -1;
        Pmsg( 0, 0x476EFFFEL, &pmsg );
        ap = app0;
      }
      if( !apps_initial && sw_flag>0 /*005:redundant, but faster*/ )
      {
        grab_curapp();		/* 005 */
        ap2 = curapp;		/* 005 */
        set_curapp(sw_app);	/* 005 */
        shel_exec();
        set_curapp(ap2);	/* 005 */
        lock_curapp = 0;	/* 005 */
      }
      _x_appl_free();
      fix_pfork();		/* 006 */
    }
    test_unload();
    if( !ap )
      if( (ap = app0) == 0 ) return 0L;	/* only if preempted */
    if( ap->asleep || ap==recorder&&!recmode || ap==player&&!playmode )   /* 004: added modes */
        continue;
    if( preempt )
    {
      if( ap->waiting<=0 ) continue;
      if( ap->mint_id<=0 ) fix_mintid(ap);	/* 005 */
      while( no_interrupt )
      {
#ifdef MINT_DEBUG
/*      Cconws( "\033j\033H\033CL\033k" );*/
#endif
        Syield();
      }
      grab_curapp();
    }
    curapp=ap;
    ap->event = 0;
    newtop( X_WM_VECEVNT, 0, (*g_vectors.gen_event)() );	/* 004 */
    /* 004: moved here and added mouse_k for STFAX */
    mks_graf( (Mouse *)&ap->mouse_x, -1 );
    ap->mouse_k = *kbshift;
    if( last_gadget && !gadget_ok )
      if( last_gadget==99 ) last_gadget=0;
      else
      {
        reset_butq();
        gadget_off();
      }
/*    if( preempt ) Psemaphore( 2, UPDT_SEM, -1 );*/
    /* 004: queued message check was here */
    if( !lastkey && has_key() )
    {
      lastkey = ((l=getkey())&0xFF) | ((l>>8)&0xFF00);
      lastsh = (l>>24)&0xF;
      for( ap2=app0; ap2; ap2=ap2->next )
        if( is_key( &ap2->flags.open_key, lastsh, lastkey ) )
        {
          ac_open( 0L, ap2->id );
          lastkey=0;
        }
    }
    if( !update.i && lastkey )
      if( in_menu )
      {
        if( ap==has_menu )
        {
          choose_menu( w=in_menu );
          menu_event( ap, buf );
          goto msg_test;
        }
      }
      else if( is_key( &settings.menu_start, lastsh, lastkey ) )
      {
        wind_menu();
        lastkey=0;
      }
      else if( is_key( &settings.ascii_table, lastsh, lastkey ) )
      {
        buf[0] = MN_SELECTED;
        buf[4] = MAGASCII;
        _appl_write( 1, 16, buf, 0 );
        lastkey=0;
      }
      else if( is_key( &settings.redraw_all, lastsh, lastkey ) )
      {
        draw_menu();
        for( w=desktop; w; w=w->next )	/* 004 */
          free_rects( w, 1 );
        redraw_all( &desktop->outer );
        lastkey=0;
      }
      else if( is_key( &settings.app_switch, lastsh, lastkey ) )
      {
        if( cycle_wind() ) lastkey = 0;
      }
      else if( is_key( &settings.app_sleep, lastsh, lastkey ) )
      {
        if( owns_key(ap) )
        {
          x_appl_sleep( ap->id, 1 );
          lastkey = 0;
        }
      }
      else if( is_key( &mouse_reset, lastsh, lastkey ) )
      {
        if( owns_key(ap) )
        {
          _graf_mouse( X_MRESET, 0L, 1 );
          lastkey = 0;
        }
      }
      else if( ap->type&MU_MESAG && !ap->blocked && owns_key(ap) && menu_equiv( buf, lastsh, lastkey ) )
      {
        ap->event |= MU_MESAG;
        lastkey=0;
      }
    if( lastkey && is_key( &settings.procman, lastsh, lastkey ) )	/* 005 (006: make settings.procman) */
    {
      procman();
      lastkey = 0;
    }
    if( !update.i && ap->type&MU_MESAG && next_menu &&
        (next_menu==desktop ? ap==has_menu : next_menu->apid==ap->id) )
    {
      choose_menu( cur_menu=next_menu );
      next_menu=0L;
      domenu( ap, buf, 1 );
/*      cur_menu=0L;  004: move to finish_menu */
    }
    if( !update.i && place && lastkey && (top_wind->apid<0 || ap->type&MU_MESAG && !ap->blocked
        && ap->id==top_wind->apid) && !no_top && !in_menu )    /* window kbd equivs */
    {
      h1 = (w=top_wind)->handle;
      recalc_window( h1, w, 0L );
      regenerate_rects( w, 0 );
      if( is_key( &settings.cycle_in_app, lastsh, lastkey ) )	/* 004 */
      {
        lastkey = 0;
        if( (h1=cycle( w, 1 )) > 0 ) goto topped;
      }
      else if( is_key( &settings.iconify, lastsh, lastkey ) )	/* 004 */
      {
        if( !w->icon_index )
        {
          h4 = WM_ICONIFY;
          next_iconify( buf+4 );
        }
        else
        {
          h4 = WM_UNICONIFY;
          *(Rect *)&buf[4] = w->iconify;
        }
        h3 = buf[4];
        lastkey = 0;
        goto msg;
      }
      else if( is_key( &settings.alliconify, lastsh, lastkey ) )	/* 004 */
      {
        if( !w->icon_index )
        {
          h4 = WM_ALLICONIFY;
          next_iconify( buf+4 );
        }
        else
        {
          h4 = WM_UNICONIFY;
          *(Rect *)&buf[4] = w->iconify;
        }
        h3 = buf[4];
        lastkey = 0;
        goto msg;
      }
      for( h2=0; h2<sizeof(settings.wind_keys)/sizeof(KEYCODE); h2++ )
        if( is_key( &settings.wind_keys[h2], lastsh, lastkey ) )
        {
          h3 = eq_tran[h2];
          if( !w->vsplit && (h3==WUP || h3==WDOWN || h3==WVBIGSL) ) h3+=WUP2-WUP;	/* 005 */
          else if( !w->hsplit && (h3==WLEFT || h3==WRT || h3==WHBIGSL) ) h3+=WLEFT2-WLEFT;	/* 005 */
          if( h3==WBACK )	/* 004: separated */
          {
            lastkey=0;
            if( (h1=cycle( w, 0 )) > 0 )
            {
              free_rects( w, 0 );
              goto topped;
            }
          }
          else if( has_kgad( w, h3 ) )
          {
            lastkey=0;
            if( h2<XS_CLOSE )
            {
              h4 = gad_zone(h3);		/* 005 */
              h3 = h2;
              goto armsg2;			/* 005: was armsg */
            }
            goto sw;
          }
        }
      free_rects( w, 0 );
    }
    /* 004: mks_graf used to be here */
    if( !update.i && !settings.flags.s.pulldown && desktop->menu && !in_menu )
    {
      choose_menu( desktop );
      if( (h1=objc_find( menu, 2, 1, ap->mouse_x, ap->mouse_y )) >=
          menu[2].ob_head && h1<=menu[2].ob_tail ) goto menu_evnt;
    }
    cur_last = bq_last;
    if( !update.i && in_menu )
    {
      if( in_menu==desktop ? ap == has_menu : ap->id == in_menu->apid )
      {
        choose_menu( w=in_menu );
        menu_event( ap, buf );
        if( !in_menu ) eat_clicks(ap);  /* eat clicks on desktop if app */
        else reset_butq();
        goto msg_test;
      }
    }
    else if( !update.i && get_butq( 1, 1, 1, 1 ) )  /* window ops */
    {
      if( gadget_ok )
      {
        if( last_gadget!=99 && tic-gadget_tic < settings.gadget_pause ) goto butn;
        h1 = last_gad_w;
      }
      else
      {
        if( last_gadget )
          if( last_gadget==99 ) last_gadget=0;
          else
          {
            gadget_off();
            set_butq();
            goto butn;
          }
        *(Mouse *)&ap->mouse_x = *(Mouse *)&but_q[new_ptr].x;
        h1 = _find_wind( x=ap->mouse_x, y=ap->mouse_y );
      }
      h2 = no_top ? 0 : top_wind->handle;
      w = find_window( h1 );
      if( !h1 )
      {
menu_evnt:
        choose_menu( desktop );
        ap->event |= MU_BUTTON;
        domenu( ap, buf, 0 );
        w = desktop;
        if( ap->event&MU_BUTTON )
        {
          ap->event &= ~MU_BUTTON;
          if( ap == has_desk && !ap->no_evnt &&
              !(ap->type&MU_BUTTON) ) eat_clicks(ap);  /* eat clicks on desktop if app */
        }                                              /* doesn't get button events */
        else set_butq();
      }
      else if( h1 == h2 || last_gadget || (ap->mouse_b&2) || w->bevent&1 )
      {
        if( w->apid>=0 )
        {
          if( w->apid != ap->id ) goto thru;
          if( !(ap->type&MU_MESAG) || ap->blocked ) goto butn;
        }
        last_gad_w = h1;
        recalc_window( h1, w, 0L );
        regenerate_rects( w, 0 );
        if( (h3=last_gadget ? last_gadget : objc_find( w->tree, 0, 8, x, y )) >= 0
            && (!h3 || has_kgad( w, h3 )) )
        {
          if( h3 && h3!=99 ) set_butq();
sw:       if( !(w->treeflag&X_WTFL_CLICKS) && h3 && h3!=99 && h3!=WUP && h3!=WVBIGSL &&
              h3!=WDOWN && h3!=WLEFT && h3!=WHBIGSL && h3!=WRT )
          {
            h4 = X_WM_SELECTED;
            goto msg;
          }
          if( h3==last_gadget || !(w->treeflag&X_WTFL_CLICKS)/*004*/ || still_pressed( w, h3 ) )
          {
            if( w->objhand && !objhand( w->objhand, h1, h3 ) )
            {
              h4 = X_WM_OBJECT;
              goto msg;		/* leave h3 */
            }
            switch(h3)
            {
              case WCLOSE:
                h4 = WM_CLOSED;
                goto msg;
              case WMOVE:
                if( dragbox_graf( 0, &buf[4], &buf[5],
                    buf[6]=w->outer.w, buf[7]=w->outer.h,
                    w->outer.x, w->outer.y, desktop->working.x-
                    (ap->flags.flags.s.off_left ? w->outer.w : 0),
                    desktop->working.y, desktop->working.x+desktop->working.w+
                    (ap->flags.flags.s.off_left ? (w->outer.w<<1) : w->outer.w),
                    desktop->working.y+desktop->working.h+w->outer.h ) )
                {
                  if( tic-ngad_tic < dc_pause ) goto topped;
                  h4 = WM_MOVED;
                  h3 = buf[4];
                  goto msg;
                }
                goto topped;	/* clicked just once on name */
              case WBACK:
                if( (h1 = cycle( w, ap->mouse_k&3 )) > 0 )
                {
                  free_rects( w, 0 );
                  goto topped;
                }
                break;
              case WICONIZ:
                h4 = (ap->mouse_k&4) ? WM_ALLICONIFY : WM_ICONIFY;
                next_iconify( buf+4 );
                h3 = buf[4];
                goto msg;
              case WFULL:  /* backward compat.: return current size */
                h4 = WM_FULLED;
                *(Rect *)&buf[4] = w->outer;
                h3 = buf[4];
                goto msg;
              case WIRT:
                if( w->info_end && w->treeflag&X_WTFL_SLIDERS )
                {
                  w->info_pos += !w->info_pos ? cel_w/char_w+1 : 1;
                  redraw_info(w);
                }
                last_gadget = h3;
                set_gadget();
                break;
              case WILEFT:
                if( w->info_pos > 0 && w->treeflag&X_WTFL_SLIDERS )
                {
                  if( w->info_pos <= cel_w/char_w+1 ) w->info_pos=0;
                  else w->info_pos--;
                  redraw_info(w);
                }
                last_gadget = h3;
                set_gadget();
                break;
              case WMNRT:
                menu_right(w);
                last_gadget = h3;
                set_gadget();
                break;
              case WMNLEFT:
                menu_left(w);
                last_gadget = h3;
                set_gadget();
                break;
              case WMENU:
                if( w->menu )
                {
                  /* wait for mouse to maybe come up */
                  while( tic-ngad_tic < settings.gadget_pause );
                  choose_menu( w );
                  get_mks();
                  domenu( ap, buf, -1 );
                }
                break;
              case WUP:
              case WUP2:
                last_gadget = h3;
                h3 = WA_UPLINE;
                goto armsg;
              case WVBIGSL:
              case WVBIGSL2:
                if( last_gadget == h3 ) h3 = last_gad_m;
                else
                {
                  objc_off( w->tree, h3+1, &x, &y );
                  last_gadget = h3;
                  h3 = last_gad_m = ap->mouse_y < y ? WA_UPPAGE : WA_DNPAGE;
                }
                goto armsg;
              case WVSMLSL:
                if( w==top_wind && wslider(w,1) ) goto thru;
                h4 = WM_VSLID;
                goto vslid;
              case WVSMLSL2:
                h4 = X_WM_VSLID2;
vslid:          h3 = graf_slidebox( w->tree, h3-1, h3, 1 );
                goto msg;
              case WDOWN:
              case WDOWN2:
                last_gadget = h3;
                h3 = WA_DNLINE;
                goto armsg;
              case WVSPLIT:
                objc_xywh( (long)w->tree, WVSPLIT, &r );
                if( w->working.h-r.h <= w->vsp_min1+w->vsp_min2 )
                    dragbox_graf( 0, &h4, &h3, r.w, r.h, r.x, r.y,
                    r.x, r.y, r.w, r.h );
                else if( dragbox_graf( 0, &h4, &h3, r.w, r.h,
                    w->outer.x, r.y, w->working.x, h2=w->working.y-1,
                    r.w, w->working.h+2 ) )
                {
                  h4 = w->vsplit;
                  if( check_split( &w->vsplit, h3-h2, w->vsp_min1,
                      w->vsp_min2, w->working.h, r.h ) )
                  {
                    get_split( w, buf+4, 1 );
                    h3 = w->vsplit;
                    w->vsplit = h4;
                    h4 = X_WM_VSPLIT;
                    goto msg;
                  }
                }
                break;
              case WLEFT:
              case WLEFT2:
                last_gadget = h3;
                h3 = WA_LFLINE;
                goto armsg;
              case WHBIGSL:
              case WHBIGSL2:
                if( last_gadget == h3 ) h3 = last_gad_m;
                else
                {
                  objc_off( w->tree, h3+1, &x, &y );
                  last_gadget = h3;
                  h3 = last_gad_m = ap->mouse_x < x ? WA_LFPAGE : WA_RTPAGE;
                }
                goto armsg;
              case WHSMLSL:
                if( w==top_wind && wslider(w,0) ) goto thru;
                h4 = WM_HSLID;
                goto hslid;
              case WHSMLSL2:
                h4 = X_WM_HSLID2;
hslid:          h3 = graf_slidebox( w->tree, h3-1, h3, 0 );
                goto msg;
              case WRT:
              case WRT2:
                last_gadget = h3;
                h3 = WA_RTLINE;
armsg:          h4 = gad_zone(last_gadget);
armsg2:         free_rects( w, 0 );
                goto msg;
              case WHSPLIT:
                objc_xywh( (long)w->tree, WHSPLIT, &r );
                if( w->working.w-r.w <= w->hsp_min1+w->hsp_min2 )
                    dragbox_graf( 0, &h4, &h3, r.w, r.h, r.x, r.y,
                    r.x, r.y, r.w, r.h );
                else if( dragbox_graf( 0, &h4, &h3, r.w, r.h, r.x,
                    w->working.y, w->outer.x, w->working.y,
                    w->working.w+2, r.h ) )
                {
                  h3 = h4-w->outer.x;
                  h4 = w->hsplit;
                  if( check_split( &w->hsplit, h3,
                      w->hsp_min1, w->hsp_min2, w->working.w, r.w ) )
                  {
                    h3 = w->hsplit;
                    get_split( w, buf+4, 0 );
                    w->hsplit = h4;
                    h4 = X_WM_HSPLIT;
                    goto msg;
                  }
                }
                break;
              case WSIZE:
                last_gadget = h3;
                *(Rect *)&buf[4] = w->outer;
                if( x_graf_rubberbox( (GRECT *)&buf[4], (GRECT *)&desktop->working,
                    w->min_w, w->min_h, w->max_w, w->max_h, 1, 1 ) )
                {
                  h4 = WM_SIZED;
                  h3 = buf[4];
                  gadget_off();
                  goto msg;
                }
                gadget_off();
                break;
              case WTOOLBOX:
                last_gadget = h3;
                break;
              case 0:
                free_rects( w, 0 );
                if( w->icon_index && (h4=mouse_ok( ap, 0, &w2 )) != 0 &&
                    ap->times==2 )
                {
                  set_butq();
                  h4 = WM_UNICONIFY;
                  *(Rect *)&buf[4] = w->iconify;
                  h3 = buf[4];
                  goto msg;
                }
                else if( w->dial_obj == IS_TEAR )
                {
                  set_butq();
                  buf[3] = h1;
                  ap->event |= MU_BUTTON;
                  menu_evnt( ap, buf );
                }       /* eat clicks if not looking for this event */
                else if( !ap->no_evnt && !(ap->type&MU_BUTTON) &&
                    (!w->icon_index || h4 && ap->times) &&
                    (w->apid>=0 && !(ap->type&X_MU_DIALOG) ||
                    !w->dialog || !(w->treeflag&X_WTFL_ACTIVE)) ) set_butq();
                else if( ap->mouse_b&1 && cur_last==bq_last )
                {
                  last_gadget = 99;
                  gadget_ok = 1;
                }
            }
          }
        }
        free_rects( w, 0 );
      }
      else
      {
        if( ap->mouse_k&3 ||
            top_wind->apid!=w->apid && settings.flags.s.top_all_at_once/*005*/ )	/* 004 */
        {
          top_all( ap, w, buf, 1 );
          goto thru;
        }
topped: h4 = WM_TOPPED;
    wait_up(ap);
msg:    set_gadget();
        if( h4==WM_ARROWED )
        {
          *(long *)&buf[5] = 0L;	/* 004 */
          buf[7] = 0;			/* 004 */
          if( scroll_wdial( w, h3, 0, 0 ) ) goto thru;
        }
        else if( h4==WM_HSLID || h4==WM_VSLID )
          if( scroll_wdial( w, h4, h3, 0 ) ) goto thru;
        buf[0] = h4;
        w = find_window( h1 );
        buf[1] = w->apid;
        buf[2] = 0;
        buf[3] = h1;
        buf[4] = h3;
        ap->event |= MU_MESAG;
        if( w->dial_obj == IS_TEAR )
        {
          menu_evnt( ap, buf );
          w = desktop;
        }
      }
msg_test:
      if( ap->event&MU_MESAG )
      {
        if( (w==desktop ? ap!=has_menu : w->apid!=ap->id) ||
           !(ap->type&MU_MESAG) || ap->blocked )
        {
          _appl_write( w==desktop ? has_menu->id : w->apid, 16, buf, 1 );
          ap->event &= ~MU_MESAG;
        }
        goto thru;
      }
    }
butn:
    if( (!gadget_ok || last_gadget==99) && !in_menu )
    {
      if( ap->type&X_MU_DIALOG )
      {
        if( mouse_ok( ap, 0, &w2 ) && win_dial( w2, buf, ap ) )
        {
          set_butq();
          mks_graf( (Mouse *)&ap->mouse_x, -1 );
          goto thru;
        }
        if( lastkey && owns_key(ap) &&
            ((h1=win_dial( top_wind, buf, ap )) > 0 || h1==-2) )	/* 004 */
        {
          mks_graf( (Mouse *)&ap->mouse_x, -1 );
/*          if( h1>0 || edit_obj>=0 || !(top_wind->treeflag&X_WTFL_KEYS) ) 004
          {  */
          lastkey=0;
          goto thru;
/*          }
          goto keybd; */
        }
      }
      if( ap->type&MU_BUTTON )
        if( mouse_ok( ap, 1, &w2 ) && (!w2->icon_index || ap->times<2) )
        {
          ap->event |= MU_BUTTON;
          set_butq();
          mks_graf( (Mouse *)&ap->mouse_x, -1 );
          if( ap->clicks <= 0 ) ap->times = 0;
          else if( /* 004 !unclick &&*/  ap->mask )
          {
            h1 = bq_ptr;	/* 004 */
            if( !unclick && (--h1) < 0 ) h1 = MAX_BUTQ-1;	/* 004 */
            ap->mouse_b = but_q[h1].b;	 /* 004: was just ap->state; */
          }
        }
    }
keybd:
    if( lastkey && owns_key(ap) && !in_menu )
    {
      ap->key = lastkey;
      h1 = ap->mouse_k;
      ap->mouse_k = lastsh;
      ap->event |= MU_KEYBD;
      if( !update.i && !no_top && top_wind->dial_obj == IS_TEAR )
      {
        buf[3] = top_wind->handle;
        menu_evnt( ap, buf );
        if( !(ap->event&MU_KEYBD) )
        {
          ap->mouse_k = h1;
          lastkey = 0;  /* tear ate key */
        }
      }
      if( !ap->no_evnt )
      {
        if( !(ap->type&MU_KEYBD) )
        {
          ap->event &= ~MU_KEYBD;
          ap->mouse_k = h1;
          if( ap->type&MU_MESAG ) lastkey = 0;
        }
        else lastkey = 0;
      }
      else ap->event &= ~MU_KEYBD;
    }
thru:
    if( (!update.i || has_update==ap) && ap->type & MU_MESAG && !ap->blocked &&
        !(ap->event&X_MU_DIALOG)/* 004: because of move */ )
    {
      for( msg=msg_q; msg; msg=msg2 )           /* get pending msg */
      {
        msg2 = msg->next;               /* get next before unlinking */
        if( ap->apread_id>=0 && ap->apread_id == msg->app ||
            ap->apread_id==-1 && ap->id == msg->app || msg->app==-1/*004*/ )
        {
          if( ap->event&MU_MESAG )	/* 004: because of moving this whole block */
              _appl_write( ap->id, 16, buf, 1 );
          ap->event |= MU_MESAG;
          if( (h1 = ap->apread_cnt) > msg->len ) h1 = msg->len;
          if( h1!=16 || ((int *)msg->buf)[2] ||
              !menu_evnt( ap, (int *)msg->buf ) )	/* 004 */
          {
            memcpy( ap->buf, msg->buf, h1 );
            if( h1==16 && !((char *)ap->buf)[0] && ((char *)ap->buf)[1]==AP_TERM ) term_time = tic;
            if( (msg->len -= h1) == 0 ) del_msg(msg);
            else memcpy( msg->buf, (char *)(msg->buf)+h1, msg->len );
            no_copy = 1;
            if( ap->apread_id==-1 )	/* 004 */
            {
              *(int *)ap->stack = 1;	/* appl_read return 1 */
              ap->no_evnt=1;
            }
/* 004 move            goto thru; */
            goto timer;
          }
          del_msg(msg);			/* 004 */
          ap->event &= ~MU_MESAG;	/* 004 */
        }
      }
      if( ap->apread_id==-1 )   /* appl_read( -1, ... ) */
          ap->no_evnt=1;
/*      for( w=desktop->next; w; w=w->next )
        if( w->dirty.w && (ap->id==w->apid || w->dial_obj==IS_TEAR) &&
            w->place>=0 )
        {
          msg_redraw( w, buf );
          ap->event |= MU_MESAG;
          if( w->dial_obj==IS_TEAR ) menu_evnt( ap, buf );
          else break;
        } 004 */
    }
timer:
    if( (ap->type & MU_TIMER) && !ap->low && !ap->high &&
        !(ap->event&MU_MESAG) ) ap->event |= MU_TIMER;  /* 004: skip timer if msg */
    if( (ap->type & MU_M1) && (ap->has_wind || ap==has_update) &&
        in_rect( g_mx, g_my, (Rect *)&ap->m1x ) != (ap->m1flags!=0) )
    {
      ap->event |= MU_M1;
      /* evnt_mouse() take unclick */
      if( !(ap->type&MU_BUTTON) && unclick && bq_ptr != bq_last )
      {
        if( ++bq_ptr == MAX_BUTQ ) bq_ptr=0;
        ap->mouse_b = but_q[bq_ptr].b;
        unclick=0;
      }
    }
    if( (ap->type & MU_M2) && (ap->has_wind || ap==has_update) &&
        in_rect( g_mx, g_my, (Rect *)&ap->m2x ) != (ap->m2flags!=0) )
        ap->event |= MU_M2;
    if( !ap->asleep )	/* might have been set during a menu_evnt */
    {
      if( ap->event )
      {
        if( has_update==ap && !(ap->type&MU_BUTTON) ) bq_ptr=bq_last;
        memcpy( emult_out, &ap->event, 7*sizeof(int) );
        if( ap->event&(X_MU_DIALOG|MU_MESAG) )
        {
          if( no_copy ) memcpy( buf, ap->buf, 8*sizeof(int) );
          fix_msg( ap, buf, 0 );
          memcpy( ap->buf, buf, 8*sizeof(int) );
        }
#ifdef MINT_DEBUG
        test_msg( ap, "locked in dispatch" );
#endif
#if 0
        if( preempt )
        {
/*          Psemaphore( 3, UPDT_SEM, 0 );*/
          grab_curapp();
        }
#endif
        lock_curapp = 0x8000|ap->mint_id;	/* 005 */
        set_curapp(ap);
        return(ap->stack);
      }
      if( ap->no_evnt && (!update.i || has_update==ap) && !ap->blocked )
      {
        if( ap->was_blocked )	/* 004 */
        {
          ap->was_blocked = 0;
          repeat_func = 1;	/**ap->id+2;**/
        }
#ifdef MINT_DEBUG
        test_msg( ap, "locked in dispatch" );
#endif
#if 0
        if( preempt )
        {
/*          Psemaphore( 3, UPDT_SEM, 0 );*/
          grab_curapp();
        }
#endif
        lock_curapp = 0x8000|ap->mint_id;	/* 005 */
        set_curapp(ap);
        return(ap->stack);
      }
      if( preempt )
      {
        lock_curapp = 0;
        if( wait_curapp ) Syield();
      }
    }
  }
}

long dispatch( int no_evnt, long stack )
{
  APP *ap;

  ap = dis_init( 1, no_evnt, stack );
  if( !preempt ) return _dispatch(ap);
  else
  {
    ap = curapp;
    if( ap->blocked )
    {
#ifdef MINT_DEBUG
      test_msg( ap, "blocking in dispatch" );
#endif
      ap->waiting = 1;
      Pmsg( 0, 0x476E0000L|ap->id, &pmsg );
      /* lock_curapp = 0;	005 */
      ap->waiting = ap->was_blocked = 0;
#ifdef MINT_DEBUG
      test_msg( ap, "unblocking in dispatch" );
#endif
      repeat_func = 1;  /** ap->id+2; **/
      return stack;
    }
#ifdef MINT_DEBUG
    test_msg( ap, "entering dispatch" );
#endif
    if( old_app ) set_oldapp();
    ap->waiting = 1;
    unblock_loop();	/* 005: release dispatch */
    Pmsg( 0, 0x476E0000L|ap->id, &pmsg );
    if( ap==has_update && !block_loop ) block_loop=1;	/* 005: make it resume sleep */
#ifdef MINT_DEBUG
    test_msg( ap, "returning from dispatch" );
#endif
    return stack;
  }
}

#if 0
void dispatch2(void)	005: not used (was appl_yield)
{
  curapp->no_evnt = 1;
  curapp->type = 0;
  curapp->buf = nobuf;
  if( old_app ) set_oldapp();
  curapp->waiting = 1;
  unblock_loop();	/* 005: release dispatch */
  Pmsg( 0, 0x476E0000L|curapp->id, &pmsg );
}
#endif

void buserr(void)
{
  char temp[100];
  
  lock_curapp = 1;
  spf( temp, "%s: memory violation. Terminating.\r\n",
      curapp ? curapp->dflt_acc : "" );
  Cconws(temp);
  if( curapp )
  {
    curapp->ap_type = -1;
    curapp->asleep = ASL_USER;
    Pkill( curapp->mint_id, SIGKILL );
  }
  curapp = 0L;
  Psigreturn();
  lock_curapp = 0;
  pdisp_init();	/* restart loop */
}

int preempt_loop(void)
{
  APP *ap=app0;
  MSG msg;
  int id;

  all_sigs();
  Psignal(SIGBUS, buserr);
  dis_init( 0, 0, 0L );
/*  Psemaphore( 0, UPDT_SEM, 0 );*/
  for(;;)
  {
#ifdef MINT_DEBUG
/*    Cconws( "\033j\033HP\033k" );*/
#endif
    _dispatch( ap );
    if( curapp )
    {
      ap = curapp->next;
      id = curapp->id;
      curapp->waiting = 0;
      Pmsg( 0x8001, 0x476E0000L|id, &pmsg );
    }
    else break;
  }
/*  in_disp_loop = 0;*/
/*  Psemaphore( 1, UPDT_SEM, 0 );*/
  return 0;
}
