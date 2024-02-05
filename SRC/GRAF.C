#include "new_aes.h"
#include "win_var.h"
#include "win_inc.h"
#include "xwind.h"
#include "vdi.h"
#include "string.h"
#include "stdlib.h"
#include "debugger.h"

/* 0x0000, 0x0000, 0x0001, 0x0000, 0x0001, 0xC000, 0xE000, 0xF000,
0xF800, 0xFC00, 0xFE00, 0xFF00, 0xFF80, 0xFFC0, 0xFFE0, 0xFE00,
0xEF00, 0xCF00, 0x8780, 0x0780, 0x0380, 0x0000, 0x4000, 0x6000,
0x7000, 0x7800, 0x7C00, 0x7E00, 0x7F00, 0x7F80, 0x7C00, 0x6C00,
0x4600, 0x0600, 0x0300, 0x0300, 0x0000,  Atari arrow */
/* 0x0008, 0x0008, 0x0001, 0x0000, 0x0001, 0x1C7E, 0x1CFF, 0x1CFF,
0xEFFF, 0xFFFF, 0xFFFF, 0x3FFE, 0x3FFC, 0x7FFE, 0xFFFE, 0xFFFF,
0xFFFF, 0xFFFF, 0xFFFF, 0xFEFF, 0x7C3E, 0x0800, 0x083C, 0x0062,
0x06C2, 0xC684, 0x198A, 0x1B54, 0x06E0, 0x1D58, 0x33FC, 0x6160,
0x42DE, 0x44D8, 0x4A56, 0x3414, 0x0000,  Atari bee */

MFORM mice[10] = {
0x0000, 0x0000, 0x0002, 0x0000, 0x0001, 0x0000, 0x4000, 0x6000,   /* arrow */
0x7000, 0x7800, 0x7C00, 0x7E00, 0x7F00, 0x7F80, 0x7C00, 0x7000,
0x6000, 0x4000, 0x0000, 0x0000, 0x0000, 0xC000, 0xA600, 0x9700,
0x8F80, 0x87C0, 0x83E0, 0x81F0, 0x80F8, 0x807C, 0x83FE, 0x8FFF,
0x97E0, 0xA780, 0xC700, 0x8600, 0x0400,
0x0007, 0x0007, 0x0002, 0x0000, 0x0001, 0x3C3C, 0x4242, 0x399C,   /* line */
0x0420, 0x0240, 0x0240, 0x0240, 0x0240, 0x0240, 0x0240, 0x0240,
0x0240, 0x0420, 0x399C, 0x4242, 0x3C3C, 0x0000, 0x3C3C, 0x0660,
0x03C0, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180,
0x0180, 0x03C0, 0x0660, 0x3C3C, 0x0000,
0x0007, 0x0007, 0x0002, 0x0000, 0x0001, 0xFFFF, 0x3FFC, 0x3FFC,   /* bee */
0x3FFC, 0x1FF8, 0x0FF0, 0x07E0, 0x03C0, 0x03C0, 0x07E0, 0x0FF0,
0x1FF8, 0x3FFC, 0x3FFC, 0x3FFC, 0xFFFF, 0x7FFE, 0x1008, 0x1008,
0x1008, 0x0A30, 0x0560, 0x02C0, 0x0180, 0x0180, 0x0240, 0x0420,
0x0810, 0x1088, 0x1558, 0x1AA8, 0x7FFE,
0x0000, 0x0000, 0x0002, 0x0000, 0x0001, 0x3000, 0x7C00, 0x7E00,   /* point */
0x1F80, 0x0FC0, 0x3FF8, 0x3FFC, 0x7FFC, 0xFFFE, 0xFFFE, 0x7FFF,
0x3FFF, 0x1FFF, 0x0FFF, 0x03FF, 0x00FF, 0x3000, 0x4C00, 0x6200,
0x1980, 0x0C40, 0x32F8, 0x2904, 0x6624, 0x93C2, 0xCF42, 0x7C43,
0x2021, 0x1001, 0x0C41, 0x0380, 0x00C0,
0x0008, 0x0008, 0x0001, 0x0000, 0x0001, 0x0300, 0x1FB0, 0x3FF8,   /* grab */
0x3FFC, 0x7FFE, 0xFFFE, 0xFFFE, 0x7FFF, 0x7FFF, 0xFFFF, 0xFFFF,
0x7FFF, 0x3FFF, 0x0FFF, 0x01FF, 0x003F, 0x0300, 0x1CB0, 0x2448,
0x2224, 0x7112, 0x9882, 0x8402, 0x4201, 0x7001, 0x9801, 0x8401,
0x4000, 0x3000, 0x0E00, 0x01C0, 0x0030,
0x0007, 0x0007, 0x0001, 0x0000, 0x0001, 0x0380, 0x0380, 0x0380,   /* thin */
0x0380, 0x0380, 0x0380, 0xFFFE, 0xFFFE, 0xFFFE, 0x0380, 0x0380,
0x0380, 0x0380, 0x0380, 0x0380, 0x0000, 0x0000, 0x0100, 0x0100,
0x0100, 0x0100, 0x0100, 0x0100, 0x7FFC, 0x0100, 0x0100, 0x0100,
0x0100, 0x0100, 0x0100, 0x0000, 0x0000,
0x0007, 0x0007, 0x0001, 0x0000, 0x0001, 0x07C0, 0x07C0, 0x07C0,   /* thick */
0x07C0,	0x07C0, 0xFFFE, 0xFFFE, 0xFFFE,	0xFFFE, 0xFFFE, 0x07C0,
0x07C0,	0x07C0, 0x07C0, 0x07C0, 0x0000, 0x0380, 0x0380, 0x0380,
0x0380,	0x0380, 0x0380, 0xFFFE, 0xFFFE,	0xFFFE, 0x0380, 0x0380,
0x0380,	0x0380, 0x0380, 0x0380, 0x0000,
0x0007, 0x0007, 0x0001, 0x0000, 0x0001, 0x06C0, 0x06C0, 0x06C0,  /* outline */
0x06C0, 0x06C0, 0xFEFE, 0xFEFE, 0x0000, 0xFEFE, 0xFEFE, 0x06C0,
0x06C0, 0x06C0, 0x06C0, 0x06C0, 0x0000, 0x0280, 0x0280, 0x0280,
0x0280, 0x0280, 0x0280, 0xFEFE, 0x0000, 0xFEFE, 0x0280, 0x0280,
0x0280, 0x0280, 0x0280, 0x0280, 0x0000,
0x0007, 0x0007, 0x0002, 0x0000, 0x0001, 0x0000, 0x0000, 0x0000,  /* left-rt */
0x0C30, 0x1C38, 0x3C3C, 0x7FFE, 0xFFFF, 0xFFFF, 0x7FFE, 0x3C3C,
0x1C38, 0x0C30, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0810, 0x1818, 0x300C, 0x7FFE, 0x7FFE, 0x300C, 0x1818,
0x0810, 0x0000, 0x0000, 0x0000, 0x0000,
0x0007, 0x0007, 0x0002, 0x0000, 0x0001, 0x0180, 0x03C0, 0x07E0,  /* up-down */
0x0FF0, 0x1FF8, 0x1FF8, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x1FF8,
0x1FF8, 0x0FF0, 0x07E0, 0x03C0, 0x0180, 0x0000, 0x0180, 0x03C0,
0x07E0, 0x0DB0, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180,
0x0DB0, 0x07E0, 0x03C0, 0x0180, 0x0000 };

MFORM *mouse_prev=&mice[0];

void init_xor(void)
{
  _vsl_color( 1 );
  _vsl_type( 7 );
  _vswr_mode( 3 );
  _vsl_udsty( 0x5555 );
}

int _mks_graf( Mouse *out )
{
  out->x = g_mx;
  out->y = g_my;
  out->b = g_mb;
  out->k = *kbshift&0xf;	/* 004 */
  return(1);
}

int mks_graf( Mouse *out, int flag )
{
  void reset_butq(void);

  if( flag>0 ) reset_butq();
  *out = *(Mouse *)&but_q[bq_ptr].x;
  if( flag<=0 )
  {
    if( !flag && bq_ptr != bq_last )
    {
      if( ++bq_ptr == MAX_BUTQ ) bq_ptr=0;
      unclick=0;
    }
    but_q[bq_ptr].x = g_mx;
    but_q[bq_ptr].y = g_my;
  }
  return(1);
}

void objc_xywh( long tree, int obj, Rect *p )
{
  objc_off((OBJECT *)tree, obj, &p->x, &p->y);
  *(long *)&(p->w) = *(long *)&u_object((OBJECT *)tree,obj)->ob_width;
}

void clip_desk(void)
{
  _vs_clip( 1, &desktop->outer );
}

void draw_xor( Rect *r )
{
  to_larr( Xrect(*r) );
  _v_mouse(0);
  pline_5();
  _v_mouse(1);
}

OBJECT2 *gr_tree;
static int gr_par, gr_obj, gr_all;
static Mouse m;
static Rect obr, par;

void move_slid( Rect *r, int nx, int ny )
{
  Rect s;

  s = *r;
  s.x--;
  s.y--;
  s.w+=2;
  s.h+=2;
  if( !ny )
    if( nx<0 ) s.w = -nx+2;
    else
    {
      s.x = r->x+r->w-nx;
      s.w = nx+2;
    }
  else if( !nx )
    if( ny<0 ) s.h = -ny+2;
    else
    {
      s.y = r->y+r->h-ny;
      s.h = ny+2;
    }
  _objc_draw( gr_tree, 0L, gr_par, 0, Xrect(s) );
  s = *r;
  s.x -= nx;
  s.y -= ny;
  mblit( 0x103/*005*/, &s );
  u_object((OBJECT *)gr_tree,gr_obj)->ob_x -= nx;
  u_object((OBJECT *)gr_tree,gr_obj)->ob_y -= ny;
}

int _graf_dragbox( GDBO *go, GDB *g, int mode )
{
  int nx, ny, bw, bh;
  Rect r;

  if( test_update( (void *)_graf_dragbox ) ) return 0;
  go->x = r.x = g->x;
  go->y = r.y = g->y;
  r.w = g->w;
  r.h = g->h;
  bw = g->bx+g->bw;
  bh = g->by+g->bh;
  if( !mode ) mks_graf( &m, 1 );
  if( r.x < g->bx ) r.x = g->bx;
  else if( r.x+r.w > bw ) r.x = bw-r.w;
  if( r.y < g->by ) r.y = g->by;
  else if( r.y+r.h > bh ) r.y = bh-r.h;
/*  if( mode ) mblit( 0x101, &r );
  if( !(m.b&1) ) return(0);		005 */
  if( !(m.b&1) )
  {
    if( mode ) mblit( -1, &r );		/* 005: moved to here */
    return(0);
  }
  if( !mode )
  {
    init_xor();
    clip_desk();
  }
  do
  {
    if( !mode ) draw_xor( &r );
    else mblit( 0x103/*005*/, &r );
    do
    {
      ny = m.y;
      nx = m.x;
      mks_graf( &m, 1 );
      if( m.x < g->bx ) m.x = g->bx;
      else if( m.x > bw ) m.x = bw;
      if( m.y < g->by ) m.y = g->by;
      else if( m.y > bh ) m.y = bh;
      ny -= m.y;
      nx -= m.x;
      if( r.x-nx < g->bx ) nx = r.x-g->bx;
      else if( r.x+r.w-nx > bw ) nx = -(bw-r.w-r.x);
      if( r.y-ny < g->by ) ny = r.y-g->by;
      else if( r.y+r.h-ny > bh ) ny = -(bh-r.h-r.y);
    }
    while( (m.b&1) && !nx && !ny );
    if( !mode ) draw_xor( &r );
    else move_slid( &r, nx, ny );
    r.x -= nx;
    r.y -= ny;
  }
  while( m.b&1 && !mode );
  go->x = r.x;
  go->y = r.y;
  return(1);
}

int cdecl dragbox_graf( int blit, int *ox, int *oy, int gr_dwidth, ... )
{
  int i;
  GDBO o;

  i = _graf_dragbox( &o, (GDB *)&gr_dwidth, blit );
  *ox = o.x;
  *oy = o.y;
  return(i);
}

int sl_pos( int slvh, int x, int y )
{
  int i;

  if( !(slvh&0xff) )
    if( (i = par.w-obr.w)<=0 ) return(0);
    else return( (long)((x-par.x)*1000L+i-1)/i );
  else if( (i=par.h-obr.h)<=0 ) return(0);
    else return( (long)((y-par.y)*1000L+i-1)/i );
}

int conv_pos( int pos )
{
  return (long)pos * gr_all / 1000L;
}

int graf_slidebox( OBJECT *tree, int parent, int object, int slvh )
{
  int oldpos=0, newpos, x, y, mode;

  if( test_update( (void *)graf_slidebox ) ) return 0;
  mode = slvh>>8;
  if( mode==1 )
  {
    mks_graf( &m, 1 );
    if( !(m.b&1) ) return -1;
    _graf_mouse( (char)slvh+8, 0L, 0 );
    if( (gr_all = parent - object + 1) < 0 ) gr_all = 0;
    return 0;
  }
  objc_xywh( (long)tree, object, &obr );
  objc_xywh( (long)tree, parent, &par );
  form_app = curapp;
  adjust_rect( &tree[object], &obr, 1 );
  adjust_rect( &tree[parent], &par, 1 );
  if( mode==2 )
  {
    if( !mblit( 0x100, &obr ) ) return 0;
    _objc_draw( gr_tree=(OBJECT2 *)tree, 0L, gr_par=parent, 0, par.x, par.y, par.w, par.h );
    gr_obj = object;
  }
  if( mode ) oldpos = conv_pos( sl_pos( slvh, obr.x, obr.y ) );
  x = obr.x;
  y = obr.y;
  if( !mode ) _graf_mouse( slvh+8, 0L, 0 );
  do
  {
    if( !dragbox_graf( mode, &x, &y, obr.w, obr.h, x, y, par.x, par.y, par.w, par.h ) && mode )
    {
      _graf_mouse( M_PREVIOUS, 0L, 0 );
      return -1;
    }
    newpos = sl_pos( slvh, x, y );
    if( mode ) newpos = conv_pos(newpos);
    else break;
  }
  while( newpos==oldpos );
  if( !mode ) _graf_mouse( M_PREVIOUS, 0L, 0 );
  return newpos;
}

void get_mpos( Mouse *m, GRECT *area, int snap )
{
  mks_graf( m, 1 );
  if( snap>1 )
  {
    m->x = (m->x-area->g_x) / snap * snap + area->g_x;
    m->y = (m->y-area->g_y) / snap * snap + area->g_y;
  }
}

typedef struct { int minwidth, minheight, maxwidth, maxheight, snap, lag; } XGRU;

int _x_graf_rubberbox( GRECT *r, GRECT *outer, long in )
{
  XGRU *x = (XGRU *)in;

  if( test_update( (void *)_x_graf_rubberbox ) ) return 0;
  return x_graf_rubberbox( r, outer, x->minwidth, x->minheight, x->maxwidth,
      x->maxheight, x->snap, x->lag );
}

int x_graf_rubberbox( GRECT *r, GRECT *outer, int minwidth, int minheight,
    int maxwidth, int maxheight, int snap, int lag )
{
  int x, y, xo, yo, dx, dy, i, negx=0, negy=0, lagx, lagy, no_neg;
  register int state=0;
  Mouse m;
  GRECT r0;

  if( snap<1 ) snap = 1;
  if( !outer ) outer = (GRECT *)&desktop->working;
  get_mpos( &m, r, snap );
  no_neg = lag&0xff00;
  lag = (unsigned char)lag;
  if( !lag )
  {
    r->g_w = m.x-r->g_x;
    r->g_h = m.y-r->g_y;
    lagx=lagy=0;
  }
  else
  {
    lagx = r->g_x+r->g_w-m.x;
    lagy = r->g_y+r->g_h-m.y;
  }
  r0 = *r;
  clip_desk();
  init_xor();
  while( m.b & 1 )
  {
    xo = negx ? r->g_x : r->g_x+r->g_w;
    yo = negy ? r->g_y : r->g_y+r->g_h;
    draw_xor( (Rect *)r );
    do
    {
      get_mpos( &m, r, snap );
      dx = m.x + lagx - xo;
      dy = m.y + lagy - yo;
      if( negx )
      {
        dx = -dx;
        if( (i=r->g_w+dx) < minwidth )
        {
          if( i<0 ) negx = -1;
          dx = minwidth-r->g_w;
        }
        else if( i > maxwidth ) dx = maxwidth-r->g_w;
        if( r->g_x-dx < outer->g_x ) dx = r->g_x-outer->g_x;
      }
      else
      {
        if( (i=r->g_w+dx) < minwidth )
          if( i<0 && minwidth>0 && !no_neg )
          {
            negx = 2;
            dx = -i;
            if( r0.g_w+dx > maxwidth ) dx = maxwidth-r0.g_w;
          }
          else dx = minwidth-r->g_w;
        else if( i > maxwidth ) dx = maxwidth-r->g_w;
        if( r->g_x+r->g_w+dx > outer->g_x+outer->g_w )
            dx = outer->g_x+outer->g_w-r->g_x-r->g_w;
      }
      if( negy )
      {
        dy = -dy;
        if( (i=r->g_h+dy) < minheight )
        {
          if( i<0 ) negy = -1;
          dy = minheight-r->g_h;
        }
        else if( i > maxheight ) dy = maxheight-r->g_h;
        if( r->g_y-dy < outer->g_y ) dy = r->g_y-outer->g_y;
      }
      else
      {
        if( (i=r->g_h+dy) < minheight )
          if( i<0 && minheight>0 && !no_neg )
          {
            negy = 2;
            dy = -i;
            if( r0.g_h+dy > maxheight ) dy = maxheight-r0.g_h;
          }
          else dy = minheight-r->g_h;
        else if( i > maxheight ) dy = maxheight-r->g_h;
        if( r->g_y+r->g_h+dy > outer->g_y+outer->g_h )
            dy = outer->g_y+outer->g_h-r->g_y-r->g_h;
      }
    }
    while( (m.b&1) && !dx && !dy && (!negx || negx==1) && (!negy || negy==1) );
    draw_xor( (Rect *)r );
    if( negx==2 )
    {
      r->g_w = r0.g_w;
      negx=1;
    }
    if( negx ) r->g_x -= dx;
    r->g_w += dx;
    if( negy==2 )
    {
      r->g_h = r0.g_h;
      negy=1;
    }
    if( negy ) r->g_y -= dy;
    r->g_h += dy;
    if( negx<0 )
    {
      r->g_x = r0.g_x;
      negx=0;
    }
    if( negy<0 )
    {
      r->g_y = r0.g_y;
      negy=0;
    }
    state = 1;
  }
  return( state );
}
int rubberbox_graf( Rect *ri, RGBO *ro )
{
  GRECT r;
  int i;

  if( test_update( (void *)rubberbox_graf ) ) return 0;
  r.g_x = ri->x;
  r.g_y = ri->y;  /* g_w, g_h don't matter */
  i = x_graf_rubberbox( &r, 0L, ri->w, ri->h, 32767, 32767, 1, 0x100 );
  ro->w = r.g_w;
  ro->h = r.g_h;
  return(i);
}

static Rect norect = { 0, 0, 0, 0 };
APP *gw_app;

int gwatch( Rect *g, OBJECT2 *tree, int obj, int in, int out,
    unsigned int ex )
{
  int state, x, y, i;

  x = g_mx - g->x;
  y = g_my - g->y;
  i = (state = x >= 0 && x < g->w && y >= 0 && y < g->h) != 0 ? in : out;
  if( u_object((OBJECT *)tree,obj)->ob_state != i )
      change_objc( (OBJECT *)tree, gw_app, obj, &norect, ex|i, 1 );
  return(state);
}

int graf_watchbox( OBJECT *tree, int obj, int in, int out )
{
  if( test_update( (void *)graf_watchbox ) ) return 0;
  return _graf_watchbox( tree, curapp, obj, in, out );
}

int _graf_watchbox( OBJECT *tree, APP *ap, int obj, int in, int out )
{
  unsigned int ex;
  Rect g;
  int state;
  OBJECT *tree2;

  gw_app = ap;
  objc_xywh( (long)tree, obj, &g );
  tree2 = &tree[obj];
  ex = tree2->ob_state&0xFF00;
  if( !(g_mb&1) )
  {
    state = 1;
    if( tree2->ob_state != in ) change_objc( tree, gw_app, obj, &norect,
        ex|in, 1 );
  }
  else
  {
    while( g_mb&1 ) state = gwatch( &g, (OBJECT2 *)tree, obj, in, out, ex );
    state = gwatch( &g, (OBJECT2 *)tree, obj, in, out, ex );
  }
  return(state);
}

typedef struct { int wch, hch, wbox, hbox; } GHS;
int _graf_handle( GHS *g )
{
  g->wch = char_w;
  g->hch = char_h;
  g->wbox = cel_w;
  g->hbox = cel_h;
  return(vdi_hand);
}

ANI_MOUSE *other_mouse[X_UPDOWN+1];

void new_mouse( ANI_MOUSE *in, int num )
{
  ANI_MOUSE **ani;
  MFORM *mf = 0L;

  if( *(ani=&other_mouse[num]) )
  {
    mf = (*ani)->form;
    lfree(*ani);  /* new size might not be same, so always free */
  }
  *ani = in ? (ANI_MOUSE *)lalloc( in->frames*sizeof(MFORM)+2+2, -1 ) : 0L;
  if( *ani ) memcpy( *ani, in, 2+2+in->frames*sizeof(MFORM) );
  else
  {
    if( ani_mouse == *ani ) ani_mouse = 0L;
    if( last_mouse == mf ) _graf_mouse( ARROW, 0L, 0 );
  }
}

int get_curapp(void);
void set_oldapp(void);

int _mouse_onoff( int i )
{
  if( !i ) curapp->mouse_on = 0;
  else curapp->mouse_on += i;
  if( !settings.flags.s.mouse_on_off || mint_preem )
  {
    if( !i ) mouse_hide = 0;
    return 1;
  }
  else if( curapp==mouse_last ) return -1;
  if( !i ) mouse_hide = 0;
  else mouse_hide += i;
  return 1;
}

int mouse_onoff( int i )
{
  int ret=0;

  if( !get_curapp() ) return 1;
  if( (ret = _mouse_onoff(i)) < 0 ) mouse_hide = curapp->mouse_on;
  set_oldapp();
  return ret;
}

int _graf_mouse( int num, MFORM *m, int set4app )
{
  int num0=num, i, is_arr;
  char is_ani = 0;
  APP *ap;

  switch( num&=0x7FFF )		/* 004: was 0xEfff */
  {
    case M_ON:
      if( set4app && !_mouse_onoff(-1) ) break;
      _v_mouse(1);
      break;
    case X_MRESET:
      if( set4app )
      {
        for( ap=app0; ap; ap=ap->next )
          ap->mouse_on = 0;
        reset_butq();
      }
      _v_mouse(2);
      break;
    case M_OFF:
      if( set4app && !_mouse_onoff(1) ) break;
      _v_mouse(0);
      break;
    case X_MGET:
      if( m ) memcpy( m, last_mouse, sizeof(MFORM) );
      return(mouse_hide);
    case USER_DEF:
form: if( !is_ani )
        for( i=0; i<sizeof(other_mouse)/sizeof(other_mouse[0]); i++ )
          if( other_mouse[i]->form == m )
          {
            if( other_mouse[i]->frames > 1 )
            {
              m = (MFORM *)other_mouse[i];
              is_ani++;
            }
            break;
          }
      is_arr = is_ani ? m==(MFORM *)other_mouse[0] : (m==&mice[0] ||
          m==other_mouse[0]->form);
      if( set4app ) curapp->mouse = is_arr ? 0L :
          (is_ani ? ((ANI_MOUSE *)m)->form : m);
      if( !set4app || !has_mouse || curapp->id==has_mouse->id || num0&0x8000 )
      {
        mouse_prev = last_mouse;
        if( is_ani )
        {
          if( last_mouse != ((ANI_MOUSE *)m)->form )
          {
            ani_mouse = 0L;
            ani_frame = ani_delay = 0;
            last_mouse = (ani_mouse = (ANI_MOUSE *)m)->form;
          }
        }
        else
        {
          ani_mouse = 0L;
          _v_mouse(0);
          vsc_form( vdi_hand, (int *)(last_mouse=m) );
          _v_mouse(1);
        }
        if( set4app )
        {
          if( !has_mouse && !is_arr ) has_mouse = curapp;
          else if( is_arr ) has_mouse = 0L;
        }
      }
      break;
    case M_SAVE:
      curapp->save_mouse = last_mouse;
      break;
    case M_RESTORE:
      m = curapp->save_mouse;
      goto form;
    case M_PREVIOUS:
      m = mouse_prev;
      goto form;
    default:
      if( num>=0 && num<=X_UPDOWN )
      {
        if( other_mouse[num] )
          if( other_mouse[num]->frames > 1 )
          {
            is_ani++;
            (ANI_MOUSE *)m = other_mouse[num];
          }
          else m = other_mouse[num]->form;
        else m = &mice[num];
        goto form;
      }
      else if( num>=X_SET_SHAPE && num<=X_SET_SHAPE+X_UPDOWN )
      {
        new_mouse( (ANI_MOUSE *)m, num-X_SET_SHAPE );
        return 1;
      }
      DEBUGGER(GRFMO,UNKTYPE,num);
  }
  return(1);
}

int x_graf_blit( GRECT *r1, GRECT *r2 )
{
  MFDB mfdb = { 0L };
  int px[8];

  if( test_update( (void *)x_graf_blit ) ) return 0;
  if( !r1 ) mblit( 0x101, (Rect *)r2 );
  else if( !r2 ) return mblit( 0x100, (Rect *)r1 );
  else
  {
    _vs_clip( 0, 0L );
    px[2] = (px[0] = r1->g_x) + r1->g_w - 1;
    px[3] = (px[1] = r1->g_y) + r1->g_h - 1;
    px[6] = (px[4] = r2->g_x) + r2->g_w - 1;
    px[7] = (px[5] = r2->g_y) + r2->g_h - 1;
    _v_mouse(0);
    vro_cpyfm( vdi_hand, 3, px, &mfdb, &mfdb );
    _v_mouse(1);
  }
  return 1;
}

void waittic(void)
{
  int i;

  for( i=4000; --i>=0; );
}

int flourish( Rect *start, Rect *end )
{
  int d[10], arr[10], i, j, k;

  if( !settings.flags.s.grow_shrink ) return 1;
  to_larr( Xrect(*end) );
  memcpy( arr, oarray, 20 );
  to_larr( Xrect(*start) );
  for( i=0; i<10; i++ )
    d[i] = (arr[i] - oarray[i]) / 8;
  clip_desk();
  init_xor();
  _v_mouse(0);
  for( k=0; k<2; k++ )
  {
    for( i=0; i<8; i++ )
    {
      waittic();
      pline_5();
      for( j=0; j<10; j++ )
        oarray[j] += d[j];
    }
    to_larr( Xrect(*start) );
  }
  _v_mouse(1);
  return 1;
}

int movebox( int *in )
{
  Rect r1, r2;

  if( test_update( (void *)movebox ) ) return 0;
  *(long *)&r1.x = *(long *)(in+2);
  *(long *)&r2.x = *(long *)(in+4);
  *(long *)&r1.w = *(long *)&r2.w = *(long *)in;
  return flourish( &r1, &r2 );
}

void fl_rect( Rect *newrect, Rect *start, Rect *end )
{
  newrect->x = end->x+(end->w-start->w)/2;
  newrect->y = end->y+(end->h-start->h)/2;
  *(long *)&newrect->w = *(long *)&start->w;
}

int growbox( Rect *start, Rect *end )
{
  Rect newrect;

  if( settings.flags.s.grow_shrink )
  {
    fl_rect( &newrect, start, end );
    if( start->w && start->h ) flourish( start, &newrect );
    flourish( &newrect, end );
  }
  return 1;
}

int shrinkbox( Rect *end, Rect *start )
{
  Rect newrect;

  if( settings.flags.s.grow_shrink )
  {
    fl_rect( &newrect, start, end );
    flourish( end, &newrect );
    if( start->w && start->h ) flourish( &newrect, start );
  }
  return 1;
}

