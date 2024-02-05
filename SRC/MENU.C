#include "new_aes.h"
#include "vdi.h"
#include "win_var.h"
#include "win_inc.h"
#define _MENUS
#ifdef GERMAN
  #include "german\wind_str.h"
#else
  #include "wind_str.h"
#endif
#include "string.h"
#include "stdlib.h"
#include "xwind.h"
#include "windows.h"
#include "debugger.h"

int new_pull( int x );
void pop_event( APP *ap );

Rect pull_rect, max;
OBJECT blank[] = { -1, -1, -1, G_BOX, 32, 0, 0xFF1070L };
int title, bar_h, m_head, title_alts, next_pull;
int old_mx, old_my;
static int keybd, state;
char is_acc, neg_alts, is_menu;
MN_SET user_mset = { 200, 10000, 250, 0, 16 }, menu_set;
unsigned char mnuk, mnush;
int mnuk_out, *menu_buf;
int m_first, m_last;
int acc_count;
OBJECT acc_root = { -1, -1, -1, (X_GRAYMENU<<8)|G_BOX, 0, 0, 0xff1000 },	/* 004 */
       acc_item = { -1, -1, -1, G_STRING, X_ITALICS, X_MAGIC|X_SMALLTEXT };
MENU acc_pop, sub_pop;
unsigned long entry_tic, scroltic1, scroltic2, pop_tic;
static ACC_LIST *sorted_acc;
static int m_obj;
static OBJECT *acc_obj, *m_start, *menu_root;
SUB_DAT *sd=&sub_dat[0];
int prop_menu( OBJECT *o, int root );

int find_pull( int title )
{
  int x, y;

  y = u_object(menu,u_object(menu,menu[0].ob_head)->ob_next)->ob_head;
  for( x=m_head; x!=title; y=u_object(menu,y)->ob_next, x++  /* 004 x=menu[x].ob_next*/ );
  return y;
}

unsigned char *keys;

int find_equiv(OBJECT *ob, int obj)
{
  char *s, *e, *s0, k, shift=0;
  int ted, i;
  static char knames[][7]= { "ESC", "TAB", "RET", "RETURN", "BKSP", "BACKSP",
      "DEL", "DELETE", "HELP", "UNDO", "INS", "INSERT", "CLR", "HOME", "F1",
      "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "ENT", "ENTER",
      "KP(", "KP)", "KP/", "KP*", "KP7", "KP8", "KP9", "KP-", "KP4", "KP5",
      "KP6", "KP+", "KP1", "KP2", "KP3", "KP0", "KP.", "",  "",  "" },
              kscans[]   = { 1,     0XF,   0X1C,  0X1C,     0XE,    0XE,
      0X53,  0X53,     0X62,   0X61,   0X52,  0X52,     0X47,  0X47,   0X3B,
      0X3C, 0X3D, 0X3E, 0X3F, 0X40, 0X41, 0X42, 0X43, 0X44,  0X72,   0X72,
      0X63,  0X64,  0X65,  0X66,  0X67,  0X68,  0X69,  0X4A,  0X6A,  0X6B,
      0X6C,  0X4E,  0X6D,  0X6E,  0X6F,  0X70,  0X71,  0x50, 0x4d, 0x4b };

  if( !obj ) return 1;
  ob = u_object(ob,obj);
  if( ob->ob_flags&HIDETREE ) return 0;
  if( (char)(ob->ob_type)==G_TITLE ) return 1;
  if( (ob->ob_state&X_MAGMASK) == X_MAGIC ) return 1;
  ob->ob_state &= 0xff;
  if( (s=s0=get_butstr((long)ob,0,&ted,0))!=0 && (e = s+strlen(s))!=s0 )
  {
    while( *--e==' ' )
      if( e==s0 ) return 1;
    s=e;
    while( *--s!=' ' )
      if( s==s0 ) return 1;
    if( s==s0+1 ) return 1;
    ob->ob_flags &= ~(7<<13);
    while( s<e ) switch( *++s )
    {
      case ' ':
      case 0:
        return 1;
      case '':
        shift |= 1;
        break;
      case '^':
        shift |= 2;
        break;
      case '':
      case '~':
        shift |= 4;
        break;
      default:
        if( s==e )
        {
          if( (k = *s)>='a' && k<='z' ) k&=0xdf;
          for( i=0; i<128; i++ )
            if( keys[i]==k )
            {
              ob->ob_state |= i<<8;
              ob->ob_flags |= shift<<13;
              return 1;
            }
        }
        else
        {
          k = *(e+1);
          *(e+1) = 0;
          for( i=sizeof(kscans); --i>=0; )
            if( !strcmpi( s, knames[i] ) ) break;
          *(e+1) = k;
          if( i>=0 )
          {
            i = kscans[i];
            if( i>=2 && i<=0xd && shift&4 ) i += 0x78-2;
            else if( i>=0x3b && i<=0x44 && shift&1 ) i += 0x54-0x3b;
            else if( shift&2 )	/* 004 */
              if( i==0x47 ) i = 0x77;
              else if( i==0x4b ) i = 0x73;
              else if( i==0x4d ) i = 0x74;
            ob->ob_state |= i<<8;
            ob->ob_flags |= shift<<13;
          }
          return 1;
        }
    }
  }
  return 1;
}

void set_equivs( Window *w )
{
  choose_menu( w );
  if( menu && (menu[0].ob_state&X_MAGMASK)==X_MAGIC )
  {
    keys = (unsigned char *) Keytbl((void *)-1L, (void *)-1L, (void *)-1L )->shift;
    fn_dir = 0;
    map_tree( menu, 0, -1, find_equiv );
  }
}

void draw_menu(void)
{
  int flag=0;

  if( !curapp )
  {
    curapp = app0;
    flag++;
  }
  set_equivs(desktop);
  blank->ob_x = 0;
  blank->ob_y = 0;
  blank->ob_width = desktop->outer.w;
  blank->ob_height = menu_h-1;
  if( menu ) accs_obj(1);
  _objc_draw( (OBJECT2 *)blank, 0L, 0, 1, Xrect(max) );
  if( menu ) _objc_draw( (OBJECT2 *)menu, menu_owner, 2, 1, Xrect(max) );
  if( flag ) curapp = 0L;
}

char my_color;

int usrcolor( int vdi, int num, int getset, int *intin )
{
  int ret, *pal;

  if( my_color || vdi>20 || num>15 || !get_curapp() ) return 0;
  if( (ret = !loading && settings.flags.s.preserve_palette &&
      curapp!=mouse_last) != 0 )
  {
    pal = curapp->palette[15-num];
    if( getset ) memcpy( pal, intin, 6 );
    else memcpy( intin, pal, 6 );
  }
  set_oldapp();
  return ret;
}

void dial_pall( int startend )		/* 004 */
{
  APP *ap;

  if( settings.flags.s.preserve_palette && (ap=find_ap(1)) != 0 )
    if( !startend )
    {
      do_pall( curapp, 0 );
      do_pall( ap, 1 );
    }
    else do_pall( curapp, 1 );
}

void do_pall( APP *ap, int getset )	/* 004 */
{
  int i, *arr;
  static char vq_mode=1;

  my_color++;
  i = vplanes>=4 ? 16 : 1<<vplanes;
  for( arr=ap->palette[0]; --i>=0; arr+=3 )
    if( getset && settings.flags.s.preserve_palette && !loading ) vs_color( vdi_hand, i, arr );
    else
      for(;;)
      {
        vq_color( vdi_hand, i, vq_mode, arr );
        if( vq_mode && (arr[0]>1000 || arr[1]>1000 || arr[2]>1000) ) vq_mode=0;
        else break;
      }
  my_color--;
}

void switch_mouse( APP *ap, int nonz )
{
  newtop( X_WM_VECSW, 0, ap ?
      (*g_vectors.app_switch)( ap->dflt_acc+2, ap->id ) :
      (*g_vectors.app_switch)( "", -1 ) );		/* 004 */
  if( mouse_last!=ap )					/* 004 */
  {
    if( mouse_last ) do_pall( mouse_last, 0 );
    if( (mouse_last=ap)!=0 )
    {
      if( settings.flags.s.mouse_on_off && !mint_preem )
      {
        while( mouse_hide < ap->mouse_on ) _v_mouse(0);
        while( mouse_hide > ap->mouse_on ) _v_mouse(1);
      }
      do_pall( ap, 1 );
    }
  }
  if( !nonz || ap->mouse )
    if( ap && ap->mouse ) _graf_mouse( USER_DEF, (has_mouse=ap)->mouse, 0 );
    else
    {
      _graf_mouse( ARROW, 0L, 0 );
      has_mouse = 0L;
    }
}

int cycle_top( APP *ap )
{
  Window *w, *top, *top2;	/* 004: added top2: don't top tear if another avail */
  int buf[8];

  if( ap && ap->has_wind )
  {
    for( top2=0L, w=top=desktop; w; w=w->next )
      if( w->apid==ap->id && w->place > top->place )
        if( w->dial_obj != IS_TEAR ) top=w;
        else top2=w;
    if( top==desktop && top2 ) top = top2;	/* 004 */
    if( top->handle && top->apid>=0 )
    {
      if( top != top_wind || no_top )
        if( settings.flags.s.top_all_at_once ) top_all( curapp, top, buf, 0 ); /*005*/
        else newtop( WM_TOPPED, top->handle, top->apid );
      return 1;
    }
  }
  return 0;
}

void new_menu( APP *ap )
{
  int flag=0;

  switch_mouse( ap, 0 );
  if( !curapp )
  {
    curapp = app0;
    flag++;
  }
  if( ap!=has_menu )
  {
    next_menu = 0L;
    has_menu = ap;
    if( multitask )
    {
      if( cycle_top(ap) )
      {
        if( flag ) curapp = 0L;
        return;
      }
      if( top_wind && top_wind->handle != 0 && !no_top )
      {
        no_top = 1;
        all_gadgets(place);
        newtop( WM_UNTOPPED, top_wind->handle, top_wind->apid );
      }
    }
  }
  if( flag ) curapp = 0L;
}

void switch_menu( APP *ap )
{
  OBJECT *ob;

  if( !ap || !ap->asleep )
  {
    new_menu(ap);
    ob = desktop->menu;
    if( (desktop->menu = ap ? ap->menu : 0L) != ob ) draw_menu();
    new_desk(-1,ap);
  }
}

void top_menu(void)
{
  APP *ap;

  if( (!update.i || has_update==has_menu) && multitask &&
      (!has_menu || !has_menu->flags.flags.s.keep_deskmenu) )
  {
    if( has_menu && !has_menu->menu )
      for( ap=app0; ap; ap=ap->next )
        if( ap->menu && has_menu->parent_id==ap->id )
        {
          switch_menu(ap);
          return;
        }
    for( ap=app0; ap; ap=ap->next )
      if( ap->menu && top_wind->apid==ap->id )
      {
        switch_menu(ap);
        return;
      }
    if( (ap=shell_app)!=0 && ap->menu && !ap->asleep )	/* 005 */
    {
      switch_menu(ap);
      return;
    }
    for( ap=app0; ap; ap=ap->next )
      if( ap->menu && !ap->asleep )
      {
        switch_menu(ap);
        return;
      }
    for( ap=app0; ap; ap=ap->next )
      if( ap->id==1 && (ap->asleep&ASL_USER) )	/* wake up Geneva menu */
      {
        to_sleep( ASL_USER, 0, ap );
        switch_menu(ap);
        return;
      }
  }
  switch_menu(0L);
}

void close_mtear(void)
{
  Window *w2, *w;

  for( w=desktop->next; w; )
    if( w->tear_parent && w->menu==curapp->menu )
    {
      w2 = w;
      w = w->next;
      close_del(w2);
    }
    else w = w->next;
}

void resize_menu( OBJECT *o )		/* 007 */
{
  if( *(char *)&o[0].ob_type == X_GRAYMENURSZ ) return;
  *(char *)&o[0].ob_type = X_GRAYMENU;
  if( !curapp->flags.flags.s.prop_font_menus ) return;
  *(char *)&o[0].ob_type = X_GRAYMENURSZ;
  objc_propfont( o, 1, 8 );
}

int _menu_bar( OBJECT *tree, int flag )
{
  if( flag==-1 ) return has_menu ? has_menu->id : -1;
  if( test_update( (void *)_menu_bar ) ) return 0;
  if( flag )
  {
    if( curapp->menu && curapp->menu != tree ) close_mtear();	/* 004 */
    desktop->menu = curapp->menu = tree;
    new_menu(curapp);
    new_desk(-1,curapp);
    tree[0].ob_x = 0;
    tree[0].ob_y = 0;
    tree[0].ob_width = tree[1].ob_width = desktop->outer.w;
    resize_menu(tree);	/* 007 */
    draw_menu();
  }
  else
  {
    close_mtear();	/* 004 */
    curapp->menu = 0L;
    if( curapp==has_menu && !curapp->flags.flags.s.keep_deskmenu &&
        !update.i ) top_menu();
  }
  return 1;
}

void menu_rec( Window *w, int i, int title )
{
  Rect r1;

  if( title )
    if( w != desktop )
    {
      if( w->place > 0 )	/* 004: test */
      {
        recalc_window( w->handle, w, (X_MENU<<8L) );	/* 004: was 0L */
        objc_xywh( (long)menu, i, &r1 );			/* 004: moved here */
        regenerate_rects( w, 0 );
        redraw_obj( w, WMENU, &r1 );
      }
    }
    else _objc_draw( (OBJECT2 *)menu, menu_owner, i, 8, Xrect(max) );
  else
  {
    objc_xywh( (long)menu, i, &r1 );
    add_redraw( w, &r1 );
  }
}

void resize_tear( Window *w, int nw, int nh )		/* 007: moved here */
{
  int oh, ow;
  OBJECT *o;

  if( !nw && !nh ) {
    o = u_object(menu,w->vslide);
    nw = o->ob_width+2;
    nh = o->ob_height+1+cel_h;
  }
  ow = w->outer.w;
  oh = w->outer.h;
  if( nh != oh || nw != ow ) {
      _set_window( w->handle, WF_CURRXYWH, w->outer.x, w->outer.y, nw, nh );
      if( nh <= oh && nw <= ow )	/* 007: put inside outer if */
          redraw_window( w->handle, &w->working, 0, 1 );
  }
}

int change_menu( OBJECT *tree, int obj, int flag, int val )
{
  Window *w;
  int i, j, *p, ret=0;
  Rect_list *r;

  i = *(p=(int *)&u_object(tree,obj)->ob_state);
  if( flag ) *p |= val;
  else *p &= ~val;
  if( *p != i || flag<0 )
    for( w=desktop; w; w=w->next )
      if( w->dial_obj==IS_TEAR && w->menu == tree )
      {
        move_menu( 0, w );
        resize_tear( w, 0, 0 );
        i = j = u_object(menu,w->vslide)->ob_head;
        do
          if( i==obj )
          {
            menu_rec( w, i, 0 );
            break;
          }
        while( (i=u_object(menu,i)->ob_next) != j && i>0 );
        move_menu( 1, w );
      }
      else if( w->menu==tree )
      {
        choose_menu( w );
        if( (char)u_object(tree,obj)->ob_type == G_TITLE ) menu_rec( w, obj, 1 );
        else if( flag<0 )	/* 004: menu_tnorm on menu entry */
        {
          *p = i;	/* reset old state */
          ret = 1;
        }
        if( w==desktop && obj == u_object(menu,find_pull(tree[2].ob_head))->ob_head )
            accs_obj(0);
      }
  return ret;
}

int icheck_menu( OBJECT *tree, int obj, int check )
{
  if( !tree )
  {
    DEBUGGER(MNUICH,NULLTREE,0);
    return(0);
  }
  if( test_update( (void *)icheck_menu ) ) return 0;
  change_menu( tree, obj, check, CHECKED );
  return(1);
}

int ienable_menu( OBJECT *tree, int obj, int check )
{
  if( !tree )
  {
    DEBUGGER(MNUIEN,NULLTREE,0);
    return(0);
  }
  if( test_update( (void *)ienable_menu ) ) return 0;
  change_menu( tree, obj, !check, DISABLED );
  return(1);
}

int tnormal_menu( OBJECT *tree, int obj, int check )
{
  int st;

  if( !tree )
  {
    DEBUGGER(MNUTNO,NULLTREE,0);
    return(0);
  }
  if( test_update( (void *)tnormal_menu ) ) return 0;
  check = !check;
  if( ((st=u_object(tree,obj)->ob_state)&SELECTED) != check )
  {
    if( change_menu( tree, obj, -1, 0 ) )	/* 004: menu_tnormal of entry */
        change_objc( tree, curapp, obj, &desktop->outer, st^SELECTED, 1 );
    else change_menu( tree, obj, check, SELECTED );
  }
  return(1);
}

int text_menu( OBJECT *tree, int obj, char *text )
{
  if( !tree )
  {
    DEBUGGER(MNUTEX,NULLTREE,0);
    return(0);
  }
  if( test_update( (void *)text_menu ) ) return 0;
  strcpy( u_obspec(tree,obj)->free_string, text );
  change_menu( tree, obj, -1, 0 );
  return(1);
}

void find_menu_id( int *buf, int hand )
{
  ACC_LIST *acc;

  buf[3] = buf[4] = 0;
  for( acc=acc_list; acc; acc=acc->next )
    if( acc->apid==hand )
    {
      buf[3] = buf[4] = acc->index;
      return;
    }
}

int ac_open( ACC_LIST *acc, int hand )
{
  int buf[8], i;
  APP *ap;

  buf[0] = AC_OPEN;
  buf[1] = hand;
  buf[2] = 0;
  if( !acc ) find_menu_id( buf, hand );
  else
  {
    get_mks();
    if( curapp->mouse_k&4 )     /* if Control is held, kill it */
    {
      __x_appl_term( hand, 0, 1 );
      return 1;
    }
    buf[3] = buf[4] = acc->index;
  }
  if( (ap = find_ap(hand)) != 0 )
    {
      if( acc && curapp->mouse_k&3 || ap->asleep )     /* if Shift is held, put to sleep */
      {
        if( !(ap->asleep&ASL_PEXEC) &&
            (i=toggle_multi( ap, ap->asleep ? -1 : ASL_USER, 1 )) <= 0 )
          if( i<0 || ap!=has_menu || sw_next_app(ap) )
          {
            if( ap->asleep&ASL_USER || !i )
            {
              to_sleep( ASL_USER, ap->asleep^ASL_USER, ap );
              accs_obj(1);
            }
            if( !ap->asleep )
              if( ap->menu ) switch_menu(ap);
              else if( !ap->has_wind )	/* 004 */
                  return _appl_write( hand, 16, buf, 0 );
          }
          else ring_bell();
        return 1;
      }
      if( ap->asleep ) return 1;
      if( ap->menu )
      {
        switch_menu(ap);
        return 1;
      }
      else if( ap->ap_type==4 ) return _appl_write( hand, 16, buf, 0 );	/* 004 */
      else if( cycle_top(ap) ) return 1;
    }
  return _appl_write( hand, 16, buf, 0 );
}

void new_acc_obj( int wid, int not_name )
{
  m_obj++;
  acc_obj = acc_tree + m_obj;
  memcpy( acc_obj, &acc_item, sizeof(OBJECT) );
  if( not_name )
  {
    memcpy( &acc_obj->ob_type, m_obj==1 ?
        &m_start->ob_type : &u_object(menu,m_start->ob_next)->ob_type,
        sizeof(OBJECT)-3*sizeof(int) );
    if( m_obj!=1 ) acc_obj->ob_flags |= 1<<13;
  }
  objc_add( acc_tree, 0, m_obj );
  acc_obj->ob_y = (m_obj-1)*char_h;
  acc_obj->ob_width = wid;
  acc_obj->ob_height = char_h;
  acc_tree[0].ob_height += char_h;
}

void add_acc( int j, int wid )
{
  ACC_LIST *acc;
  APP *ap;

  new_acc_obj( wid, 0 );
  for( acc = sorted_acc ? sorted_acc : acc_list; acc; )
  {
    if( !j-- )
    {
      acc_obj->ob_spec.free_string = acc->name;
      if( has_menu && acc->apid==has_menu->id ) acc_obj->ob_state |= CHECKED;
      if( (ap = find_ap(acc->apid)) != 0 )
      {
        if( ap->flags.flags.s.multitask ) acc_obj->ob_state &= ~X_SMALLTEXT;
        if( !ap->asleep ) acc_obj->ob_flags &= ~X_ITALICS;
      }
      break;
    }
    acc = sorted_acc ? acc->next_sort : acc->next;
  }
}

int sort_acc( ACC_LIST **a, ACC_LIST **b )
{
  int ret;

  if( (ret = strcmp( (*a)->name, (*b)->name )) == 0 )
      ret = (*a)->apid < (*b)->apid ? -1 : 1;
  return ret;
}

int tprop_menu( OBJECT *o, int root )		/* 007 */
{
  if( (!menu_owner || menu_owner->flags.flags.s.prop_font_menus) &&
      *(char *)&o->ob_type >= X_GRAYMENU &&
      *(char *)&o->ob_type <= X_PROPFONTRSZ ) return prop_menu( o, root );
  return 0;
}

void accs_obj( int tears )
{
  int i, wid, mw, prgs, accs, nw;
  ACC_LIST *acc, **sort, **srt;
  Window *w;

  if( in_t1 || (acc=acc_list)==0 ) return;
  choose_menu( desktop );
  if( !menu ) return;
  if( acc_tree ) lfree(acc_tree);
  sorted_acc = 0L;
  if( !acc_count ) sort = 0L;
  else sort = (ACC_LIST **)lalloc(acc_count<<4,-1);
  for( wid=mw=prgs=0, accs=acc_count; acc; acc=acc->next )
  {
    if( (i=strlen(acc->name)) > wid ) wid = mw = i;
    if( sort )
      if( acc->is_acc ) sort[--accs] = acc;
      else sort[prgs++] = acc;
  }
  if( (wid=(wid+1)*char_w) <
      (i=u_object(menu,u_object(menu,find_pull(m_head))->ob_head)->ob_width) ) wid = i;
  if( (acc_tree = (OBJECT *)lalloc(sizeof(OBJECT)*
      ((acc_count?acc_count+1:0)+(prgs&&accs!=acc_count)+2),-1)) != 0 )
  {
    if( sort )
    {
      qsort( sort, prgs, sizeof(sort[0]), (int (*)())sort_acc );
      qsort( &sort[prgs], acc_count-prgs, sizeof(sort[0]), (int (*)())sort_acc );
      for( sorted_acc=*(srt=sort), i=acc_count; --i>=0; )
        (*srt++)->next_sort = i ? *(srt+1) : 0L;
    }
    m_start = u_object(menu,u_object(menu,find_pull(m_head))->ob_head);
    m_obj = 0;
    memcpy( acc_tree, &acc_root, sizeof(OBJECT) );
    acc_tree[0].ob_width = wid;
    acc_tree[0].ob_height = 0;
    new_acc_obj( wid, 1 );
    if( acc_count )
    {
      new_acc_obj( wid, 1 );
      for( i=0; i<prgs; i++ )
        add_acc( i, wid );
      if( i && i<acc_count ) new_acc_obj( wid, 1 );
      while( i<acc_count )
        add_acc( i++, wid );
    }
  }
  if( sort ) lfree(sort);
  if( tears ) {
    APP *a = menu_owner;	/* 007 */
    menu_owner = 0L;		/* 007 */
    for( w=desktop, i=acc_tear; i && w; w=w->next )
      if( w->apid<0 && w->hslide<0 )      /* ACC tear */
      {
        if( !acc_count || !acc_tree ) close_del(w);
        else	/* 007: changed a lot */
        {
          acc_tree[1].ob_flags |= HIDETREE;
          acc_tree[2].ob_flags |= HIDETREE;
          if( tprop_menu( w->menu = acc_tree, 0 ) ) {
            nw = acc_tree[0].ob_width + 2;
          }
          else {
            nw = (mw+1)*char_w + 2;
            if( nw < (wid=strlen(APPLIC)*char_w+cel_w+cel_w) ) nw = wid;
          }
          resize_tear( w, nw, acc_tree[0].ob_height-
              (acc_tree[1].ob_height<<1)+1+cel_h );
        }
        i--;
      }
    menu_owner = a;	/* 007 */
  }
}

void del_acc_name( int id )
{
  ACC_LIST *current, *prev, *n;

  if( id==-2 )
  {
    if( acc_tree )
    {
      lfree(acc_tree);
      acc_tree = 0L;
    }
    if( acc_list )
    {
      lfree(acc_list);
      acc_list = 0L;
    }
    acc_count = 0;
    return;
  }
  if( id<0 ) id = curapp->id;
  for( prev=0L, current=acc_list; current; )
    if( current->apid==id )
    {
      n = current->next;
      if( prev ) prev->next = n;
      else acc_list = n;
      lfree(current);
      current = n;
      acc_count--;
    }
    else
    {
      prev = current;
      current = current->next;
    }
  accs_obj(1);
}

char *new_desc( char *in, int len )
{
  int extra;

  extra = len + strlen(in) + 1;
  if( !curapp->app_desc || strlen(curapp->app_desc)+1<extra )
  {
    if( curapp->app_desc ) lfree( curapp->app_desc );
    if( (curapp->app_desc = (char *)lalloc( extra, curapp->id )) == 0L )
        return 0L;
  }
  strncpy( curapp->app_desc, "      ", len );
  strcpy( curapp->app_desc+len, in );
  return curapp->app_desc;
}

int _menu_register( int apid, char *str )
{
  ACC_LIST *current, *next, *prev;
  APP *ap;
  int i, id, spaces;
  char *ptr, is_dflt;

  /* apid==-1: replace dflt_acc   -2: insert current if none exists */
  /* str=0L: take flags->desc or dflt_acc */
  if( str && test_update( (void *)_menu_register ) ) return 0;
  if( apid==-1 )
  {
    strncpy( curapp->dflt_acc+2, str, 8 );
    pad_acc_name( curapp->dflt_acc+2 );
    return 1;
  }
  spaces = curapp->flags.flags.s.multitask ? 2 : (char_w*3)/6;
  if( !str )
  {
    is_dflt = 1;
    if( !curapp->flags.desc[0] || !strcmp( curapp->flags.desc, dflt_flags.desc ) ||
        (str = new_desc( curapp->flags.desc, spaces )) == 0 )
      if( (str = new_desc( curapp->dflt_acc+2, spaces )) == 0 )
          str = curapp->dflt_acc;
  }
  else
  {
    is_dflt = 0;
    if( spaces>2 && (ptr = new_desc( str, spaces-2 )) != 0 ) str = ptr;
  }
  if( (id=apid)==-2 ) id = curapp->id;
  ap = find_ap(id);
  if( !ap ) return -1;
  for( current=acc_list; current && current->apid!=id; current=current->next );
  if( apid==-2 && current ) return -2;
  if( current && current->name==str )	/* 004: just redraw if same str */
  {
    accs_obj(1);
    return current->index;
  }
  /* PRG only gets one name */
  if( current && (current->is_dflt || ap->ap_type==2) )
  {
    current->name=str;
    current->is_dflt = is_dflt;
    accs_obj(1);
    return current->index;
  }
  if( (current=(ACC_LIST *)lalloc(sizeof(ACC_LIST),-1)) == 0 ) return -1;
  /* find first empty index */
  for( i=0, prev=0L, next=acc_list; next && next->index==i; )
  {
    i++;
    prev = next;
    next = next->next;
  }
  if( !prev ) acc_list = current;
  else prev->next = current;
  current->next = next;
  current->name = str;
  current->apid = id;
  current->index = i;
  current->is_acc = ap->ap_type==4;
  current->is_dflt = is_dflt;
  acc_count++;
  accs_obj(1);
  return i;
}

void no_clip(void)
{
  _vs_clip( 0, 0L );
}

int mblit( int flag, Rect *r2 )
{
  int px[8];
  long size;
  Rect r = *r2;
  static PULL pb[4];
  static int blit_lev;
  static char first_too;	/* set if blit_lev=0 was too small */

  if( flag==-1 ) goto free;
  if( !(flag&0xff00) )
  {
    if(r.x) r.x--;
    r.y--;
    r.w+=2;
    r.h+=2;
  }
  else flag = (char)flag;
  intersect( desktop->outer, r, &r );
  fdb2.fd_h = r.h;
  fdb2.fd_wdwidth = (fdb2.fd_w = r.w+16) >> 4;
  fdb2.fd_nplanes = vplanes;
  fdb2.fd_r1 = fdb2.fd_r2 = fdb2.fd_r3 = 0;
  size = (long)r.h*(fdb2.fd_wdwidth<<1)*vplanes;
  if( !flag && !blit_lev && size>pull_siz.l )
  {
    first_too++;
    blit_lev++;
  }
  if( !flag && blit_lev++ )
  {
    pb[blit_lev-2].l = pull_buf.l;
    if( (pull_buf.l = (long)lalloc( size, -1 )) == 0 )
    {
      while( --blit_lev > 1 )
        lfree((void *)pb[blit_lev-1].l);
      pull_buf.l = pb[0].l;
      blit_lev = 0;
      no_memory();
      return 0;
    }
  }
  fdb2.fd_addr = (char *)pull_buf.l;
  no_clip();
  px[2] = (px[0] = !flag ? r.x : 0) + r.w - 1;
  px[3] = (px[1] = !flag ? r.y : 0) + r.h - 1;
  px[6] = (px[4] = !flag ? 0 : r.x) + r.w - 1;
  px[7] = (px[5] = !flag ? 0 : r.y) + r.h - 1;
  _v_mouse(0);
  vro_cpyfm( vdi_hand, 3, px, !flag ? &fdb0 : &fdb2, !flag ? &fdb2 : &fdb0 );
  _v_mouse(1);
free:
  if( flag && (flag<0 || !(flag&2))/*005*/ )
  {
    if( blit_lev>1 )
    {
      lfree((void *)pull_buf.l);
      pull_buf.l = pb[blit_lev-2].l;
    }
    if( blit_lev )
    {
      blit_lev--;
      if( first_too )
      {
        first_too--;
        blit_lev--;
      }
    }
  }
  return 1;
}

void drw_alt( OBJECT *tree, int num, int undraw )
{
  _v_mouse(0);
  alt_redraw( (long)tree, num, undraw );
  _v_mouse(1);
}

void draw_title(void)
{
  menu_rec( menu_wind, title, 1 );
  if( menu_wind != desktop )
  {
    no_clip();
    drw_alt( menu, -title, 0 );
  }
}

INT_MENU *find_sub( APP *ap, OBJECT2 *tree, int item, unsigned int *index )
{
  INT_MENU *im;
  unsigned char num;
  unsigned int ii;

  if( !ap || (im=ap->menu_att)==0 ) return 0L;
  tree = (OBJECT2 *)u_object((OBJECT *)tree,item);
  if( (num = tree->ob_typex)<=127 ) return 0L;
  im += (ii = num-128);
  if( index ) *index = ii;
  if( im->parent!=tree ) return 0L;
  return im;
}

int add_attach( OBJECT *tree, int num )
{
  unsigned int ii;

  if( (char)u_object(tree,num)->ob_type == G_STRING && find_sub( menu_owner,
      (OBJECT2 *)tree, num, &ii ) != 0 )
      u_obspec(tree,num)->free_string[strlen(u_obspec(tree,num)->free_string)-2] =
      fn_dir ? ' ' : '';
  return 1;
}

void set_attaches( OBJECT *tree, int undo, int pull )
{
  if( menu_owner && menu_owner->menu_att )
  {
    fn_dir = undo;
    map_tree( tree, pull, u_object(tree,pull)->ob_next, add_attach );
  }
}

void undraw( int title, int flag )
{
  if( flag )
  {
    draw_title();
    sel_if(menu,title,0);
    draw_title();
  }
  set_attaches( menu, 1, sd->pull );
  u_object(sd->tree,sd->pull)->ob_x = old_mx;
  u_object(sd->tree,sd->pull)->ob_y = old_my;
}

int in_rect( int x, int y, Rect *r )
{
  x -= r->x;
  y -= r->y;
  return( x>=0 && x<r->w && y>=0 && y<r->h );
}

void entry_off(void)
{
  if( sd->entry )
  {
    change_objc( sd->tree, menu_owner, sd->entry, &max,
        u_object(sd->tree,sd->entry)->ob_state&~SELECTED, 1 );
    sd->entry=0;
  }
}

void get_mks(void)
{
  mks_graf( (Mouse *)&curapp->mouse_x, 1 );
}

char *center_title( int title )
{
  char *p, *p2, *p3;
  int x;
  OBJECT2 *o;

  p2 = p = (x=(o=(OBJECT2 *)u_object(menu,title))->ob_type)==G_STRING ||	/* 005: check type, use get_spec */
      x==G_TITLE ? (char *)get_spec((OBJECT *)o) : (char *)"";
  x = 0;
  while( *p2 == ' ' )
  {
    p2++;
    x++;
  }
  p3 = p2;
  while( *p3 ) p3++;
  while( p3>p2 && *--p3==' ' && x>0 ) x--;
  return(p+x);
}

int check_acc(void)
{
  ACC_LIST *acc;
  int i;

  if( menu==acc_tree )
  {
    if( sd->entry<3 )
    {
      acc_pop.mn_tree = menu = desktop->menu;
      sd->entry = u_object(menu,find_pull(m_head))->ob_head;
      acc_pop.mn_menu = sd->pull;
      return 0;
    }
    for( i=3, acc = sorted_acc ? sorted_acc : acc_list; acc;
        acc = sorted_acc ? acc->next_sort : acc->next, i++ )
    {
      if( u_object(acc_tree,i)->ob_flags&(1<<13) ) sd->entry--;
      if( i==sd->entry )
      {
        move_menu( 1, menu_wind );	/* 004: 005: causes error in DB mode */
        ac_open( acc, acc->apid );
        choose_menu(desktop);
        menu = acc_tree;		/* 004 */
        return 1;
      }
    }
  }
  return 0;
}

void new_entry( int x, int force )
{
  if( x!=sd->entry )
  {
    entry_off();
    if( force || !is_disab(sd->tree,x) )
    {
      change_objc( sd->tree, menu_owner, sd->entry=x, &max,
          u_object(sd->tree,x)->ob_state|SELECTED, 1 );
      entry_tic = tic;
    }
  }
}

int check_title( int x )
{
  while( is_disab(menu,x) ) x = u_object(menu,x)->ob_next;
  if( x<m_head ) return 0;
  return x;
}

void get_menu_alts( OBJECT *tree, int start, int draw )
{
  int i, neg;

  if( keybd )
  {
    neg = num_keys;	/* this is the title bar, so negate them */
    no_clip();
    fn_dir = 0;		/* scan all items */
    fn_last = draw;
    form_app = menu_owner;
    map_tree( tree, start, u_object(tree,start)->ob_next, find_alt );
    if( !neg )
    {
      for( i=0; i<num_keys; i++ )
        alt_obj[i] = -alt_obj[i];
      neg_alts=1;
    }
    else neg_alts=0;
  }
}

void undraw_menu_alts(void)
{
  if( (num_keys=title_alts) != 0 )
  {
    no_clip();
    if( menu_wind==desktop ) form_redraw_all( (long)menu, 1 );
    else redraw_obj( menu_wind, WMENU, 0L );
  }
}

int prev_asd(void)
{
  int x;

  if( sd->done )
  {
    if( is_acc )
    {
      menu = acc_tree;
      is_acc = 0;
    }
    x = sd->ret;
    if( x<=0 ) acc_pop.mn_item = 0;
    sd->entry = acc_pop.mn_item;
    if( x<0 ) return -1;
    return 1;
  }
  return 0;
}

int search_down( int old, int current )	/* 005 */
{
  int l;
  OBJECT *o = u_object(sd->tree, current);

  if( (l=o->ob_y) > u_object(sd->tree, sd->entry)->ob_y &&
      l < u_object(sd->tree, old)->ob_y ) return current;
  return old;
}

int search_lowest( int old, int current )	/* 005 */
{
  if( u_object(sd->tree, current)->ob_y > u_object(sd->tree,old)->ob_y ) return current;
  return old;
}

int search_up( int old, int current )	/* 005 */
{
  int l;
  OBJECT *o = u_object(sd->tree, current);

  if( (l=o->ob_y) < u_object(sd->tree, sd->entry)->ob_y &&
      l > u_object(sd->tree, old)->ob_y ) return current;
  return old;
}

int search_highest( int old, int current )	/* 005 */
{
  if( u_object(sd->tree, current)->ob_y < u_object(sd->tree, old)->ob_y ) return current;
  return old;
}

int search_entry( int func( int old, int current ), int root, int dis )	/* 005 */
{
  int i, j, k;
  OBJECT *o;

  i = j = u_object(sd->tree,root)->ob_head;
  if( func==search_down ) i = search_entry( search_lowest, root, dis );
  else if( func==search_up ) i = search_entry( search_highest, root, dis );
  else
  {  /* find first non- hidden/disab entry */
    for( k=0x7fff; i!=root; i=u_object(sd->tree,i)->ob_next )
      if( (!dis || !((o=u_object(sd->tree,i))->ob_state&DISABLED)) &&
          !(o->ob_flags&HIDETREE) && o->ob_y < k )
      {
        j = i;
        k = o->ob_y;
      }
    i = j;
  }
  for( ; j!=root; j=u_object(sd->tree,j)->ob_next )
    if( !is_disab(sd->tree,j) && !is_hid(sd->tree,j) ) i = (*func)( i, j );
  return i;
}

void set_npull( int x )
{
  next_pull = !is_disab(menu,x) ? x : -1;
}

int new_pull( int x )
{
  int y, tx, ty;
  char round=1;

  if( title ) undraw( title, 1 );
  y = find_pull(title=x);
  if( menu_owner->flags.flags.s.prop_font_menus &&
      (*(char *)&menu[0].ob_type == X_GRAYMENURSZ ||
       *(char *)&menu[0].ob_type == X_PROPFONTRSZ) )	/* 007 */
  {
    objc_off( menu, x, &tx, &ty );
    u_object(menu,y)->ob_x = tx-menu[0].ob_x;
    round = 0;
  }
  objc_off( menu, y, &pull_rect.x, &pull_rect.y );
  old_mx = u_object(menu,y)->ob_x;
  old_my = u_object(menu,y)->ob_y;
  if( pull_rect.x < 0 )
  {
    u_object(menu,y)->ob_x -= pull_rect.x;
    pull_rect.x = 0;
  }
  if( pull_rect.x+(pull_rect.w=u_object(menu,y)->ob_width) > (x=max.x+max.w) )
  {
    u_object(menu,y)->ob_x -= (x=pull_rect.x+pull_rect.w-x+1);
    pull_rect.x -= x;
  }
  if( pull_rect.y+(pull_rect.h=u_object(menu,sd->pull=y)->ob_height) > (x=max.y+max.h) )
  {
    u_object(menu,y)->ob_y -= (x=pull_rect.y+pull_rect.h-x+1);
    pull_rect.y -= x;
  }
  if(round/*007*/) {
    u_object(menu,y)->ob_x -= pull_rect.x&7;	/* 004 */
    pull_rect.x &= 0xFFF8;		/* 004 */
  }
  if( !is_sel(menu,title) )
  {
    sel_if(menu,title,1);
    draw_title();
  }
  if( menu_wind!=desktop || title!=m_head )
  {
    acc_pop.mn_tree = menu;
    acc_pop.mn_menu = y;
    acc_pop.mn_item = search_entry( search_highest, y, 0 );	/* 005 was u_object(menu,y)->ob_head; */
    acc_pop.mn_scroll = 0;
  }
  else	/* Desk menu */
  {
    acc_pop.mn_tree = acc_tree;
    acc_pop.mn_menu = 0;
    acc_pop.mn_item = 1;
    acc_pop.mn_scroll = 1;
    is_acc++;
  }
  next_pull=0;
  if( in_sub>=4 ) return 0;
  in_sub++;
  sd++;
  tprop_menu( acc_pop.mn_tree, acc_pop.mn_menu );	/* 007 */
  menu_popup( &acc_pop, pull_rect.x, pull_rect.y, &acc_pop );
  return prev_asd();
}

/****int last_entry( int horiz )	005
{
  int i, l;

  for( l=0, i=u_object(sd->tree,sd->pull)->ob_head; i!=sd->pull; i=u_object(sd->tree,i)->ob_next )
    if( !is_hid(sd->tree,i) ) l=i;
  if( !horiz ) return l;
  if(l) new_entry(l,1);
  return 0;
}  ******/

char kbd_draw;

void pull_it( int i )
{
  int f, l;

  if( (i = check_title(i)) == 0 ) i = check_title(m_head);
  if( (f=menu_wind->menu_tA) == 0 ) f=m_head;
  if( (l=menu_wind->menu_tZ) == 0 ) l=menu[2].ob_tail;
  if( i!=title )
  {
    entry_off();
    if( i < f || i > l )
    {
      if( title ) undraw( title, 1 );
      menu_wind->menu_tA = i==m_head ? 0 : i;
      regenerate_rects( menu_wind, 0 );
      kbd_draw = 1;
    }
    set_npull(i);
  }
}

int test_sub( int flag )
{
  return menu_owner && menu_owner->menu_att &&
      ((OBJECT2 *)u_object(sd->tree,sd->entry))->ob_typex>=128 && (!flag ||
      tic-entry_tic > menu_set.Display) &&
      !is_disab(sd->tree,sd->entry);
}

int menu_keybd(void)
{  /* return:  0: disabled or err,  -2: menu start,  -3: new pull in sub
              -1: Esc (entry=0) CR or Space,  other: new entry */
  int i, j, l, sh;
  unsigned long k;
  unsigned int key;

  if( !sd->entry ) return 0;
  if( is_menu )
  {
    key = lastkey;
    sh = lastsh;
    lastkey = 0;
  }
  else
  {
    k = getkey();
    key = (k&0xff)|((k>>8L/*004: was 16*/)&0xff00);
    sh = k>>24;
  }
  if( is_key( &settings.menu_start, sh, key ) ) return -2;
  switch( (char)key )
  {
    case '\r':
    case ' ':
cr:   if( is_disab(sd->tree,sd->entry) ) return 0;
      return -1;
    case '\033':
      entry_off();
      return -1;
    default:
      i = key>>8;
      if( i>=0x78 && i<=0x81 ) i-=0x78-0x2;
      j = Keytbl((void *)-1L, (void *)-1L, (void *)-1L)->unshift[i];
      if( j>='a' && j<='z' ) j &= 0xdf;
      for( i=0; i<num_keys; i++ )
        if( alt[i]==j )
          if( !title_alts )
          {
            new_entry( -alt_obj[i], 1 );	/* 004: was char */
            return -1;
          }
          else if( i<title_alts )
          {
            if( (is_menu || in_sub) && -alt_obj[i] != title )	/* 004: was char */
            {
              entry_off();
              pull_it( -alt_obj[i] );	/* 004: was char */
              if( in_sub ) return -3;
              if( is_menu ) return -1;
            }
            return 0;
          }
          else
          {
            new_entry( alt_obj[i], 1 );
            return -1;
          }
      /* 005: search for true next/prev, even with badly ordered menus */
/** 005      i=u_object(sd->tree,sd->pull)->ob_head; */
      i = search_entry( search_highest, sd->pull, 1 );
      switch( (char)(key>>8) )
      {
        case 0x1C:
        case 0x72:
          goto cr;
        case 0x61:	/* Undo */
esc:      entry_off();
          return -1;
        case 0x48:    /* up */
          if( !(sh&3) ) i = search_entry( sd->entry!=i ? search_up :
              search_lowest, sd->pull, 1 );
          return i;
        case 0x50:    /* down */
          j = search_entry( search_lowest, sd->pull, 1 );
          if( sh&3 ) i = j;
          else if( sd->entry!=j ) i = search_entry( search_down, sd->pull, 1 );
          return i;
/** 005          if( sh&3 ) return last_entry(is_menu);
          else if( (j=u_object(sd->tree,sd->entry)->ob_next) != sd->pull &&
              !is_hid(sd->tree,j) )
          {
            return j;
          }
          else
          {
            return i;
          }
          break;		********/
        case 0x4b:    /* left */
          if( is_menu ? in_sub>1 : in_sub>0 ) goto esc;
          else if( !is_menu || in_sub>1/*005*/ ) break;
          for( l=0, i=m_head; i>=m_head; i=j )
          {
            j = u_object(menu,i)->ob_next;
            if( !is_disab(menu,i) ) l = i;
            if( j==title&&l>0 || j<m_head )
            {
              pull_it(l);
              break;
            }
          }
          return -3;
        case 0x4d:    /* right */
          if( test_sub(0) ) goto cr;
          else if( !is_menu || in_sub>1/*005*/ ) break;
          if( (i=u_object(menu,title)->ob_next) > m_head ) pull_it(i);
          else pull_it(m_head);
          return -3;
      }
  }
  return 0;
}

int test_pop(void)
{
  if( next_menu ) return 1;
  if( acc_pop.mn_item>=0 )
  {
    sd->entry = acc_pop.mn_item;
    menu = acc_tree;
    return 1;
  }
  return 0;
}

void menu_sel( APP *ap, OBJECT *tree, int ent, int pull )
{
  ap->event |= MU_MESAG;
  menu_buf[0] = menu_wind==desktop ? MN_SELECTED : X_MN_SELECTED;
  menu_buf[3] = title;
  menu_buf[4] = ent;
  *(OBJECT **)&menu_buf[5] = tree;
  menu_buf[7] = menu_wind==desktop ? pull : menu_wind->handle;
}

void drw_pop_al( int undraw, int top, int bot )
{
  int i, j;

  j=top;
  do
  {
    i = j;
    if( i != sd->up && i != sd->down ) drw_alt( sd->tree, neg_alts ? -i : i, undraw );
    j = u_object(sd->tree,i)->ob_next;
  } while( i!=bot && i>=0 );
}

void draw_pop_alts( int undraw )
{
  int i, j;

  if( num_keys )
  {
    if( sd->sprev>0 && sd->stop != sd->first ) drw_pop_al( undraw, sd->first, sd->sprev );
    drw_pop_al( undraw, sd->stop, sd->sbot );
  }
}

void alts_off(void)
{
  if( num_keys )
  {
    draw_pop_alts(1);
    undraw_menu_alts();
    num_keys = title_alts = 0;
  }
  keybd = 0;
}

int prev_psd(void)
{
  if( sd->done )
  {
    if( next_pull && !in_sub )
    {
      return 1;	/* continue if there is still a pull */
    }
    else if( sd->ret )
    {
      if( !is_menu ) return 0;	/* 005 */
      if( sd->mdata ) memcpy( sd->mdata, &sub_pop, sizeof(MENU) );
      entry_off();
      return 1;
    }
    else if( !in_sub ) return 0;	/* 005: pops already closed */
    prop_menu( 0L, 0 );		/* 007 */
    in_sub--;
    sd--;
    num_keys = title_alts;
    get_menu_alts( sd->tree, sd->pull, 1 );
  }
  return 0;
}

int submenu( Rect *r, MENU *out, int modal )
{ /* return:  1: continue,  0: get out */
  int x, y, i;
  MENU *sub;

  if( in_sub<4 )
  {
    pop_tic = tic;
    sub = &find_sub( menu_owner, (OBJECT2 *)sd->tree, sd->entry, 0L )->menu;
    x = (u_object(sd->tree,sd->pull)->ob_x + u_object(sd->tree,sd->pull)->ob_width - char_w) & 0xfff8;
    if( x + (i=u_object(sub->mn_tree,sub->mn_item)->ob_width) > max.x+max.w )
      if( (x = u_object(sd->tree,sd->pull)->ob_x - i) < 0 ) x = 0;
      else x &= 0xfff8;
    y = r->y + u_object(sd->tree,sd->entry)->ob_y;
    next_pull=0;
    draw_pop_alts(1);
    num_keys = title_alts;
    sd++;
    in_sub++;
    if( !out ) out = &sub_pop;
    tprop_menu( sub->mn_tree, sub->mn_menu );	/* 007 */
    menu_popup( sub, x, y, out );
    if( modal )
    {
      while( !sd->done )
      {
        get_mks();
        pop_event(curapp);
      }
      if( sd->ret>0 )
          menu_sel( curapp, sub->mn_tree, sub_pop.mn_item, sub->mn_menu );
      return 0;
    }
    return !prev_psd();
  }
  return 1;
}

void release_menu(void)
{
  long stack;

  if( preempt ) stack = Super(0L);
  in_menu = 0L;
  unblock();
  if( preempt ) Super((void *)stack);
}

void finish_menu( APP *ap )
{
  OBJECT *o;

  o = menu_wind->menu;
  if( sd->entry && title==m_head )
    if( check_acc() ) sd->entry = 0;
  menu = o;
  if( menu_wind->menu==o )
  {
    undraw( title, sd->entry==0 );
    undraw_menu_alts();
    title_alts=num_keys=0;	/* added for rel 003 */
  }
  else sel_if(menu,title,0);
  while( ap->mouse_b&1 ) get_mks();
  reset_butq();
  if( sd->entry ) menu_sel( ap, acc_pop.mn_tree, sd->entry, acc_pop.mn_menu );
  title=sd->pull=num_keys=0;
  force_mouse(1);
  is_menu = 0;
  next_pull = 0;
  cur_menu = 0L;
  release_menu();
  test_unload();                /* won't return if term worked */
}

int has_pkey(void)
{
  return in_menu ? lastkey && keybd : has_key();
}

void menu_event( APP *ap, int *buf )
{
  int x;
  OBJECT *o;

  menu_buf = buf;
  if( next_pull )
  {
    acc_pop.mn_item = -1;
    if( next_pull<0 || new_pull(next_pull) && next_pull<=0 ) finish_menu(ap);
  }
  else if( in_sub )
  {
    pop_event(ap);
    if( prev_asd() )
      if( next_pull>0 )
      {
        o = menu;
        menu = menu_wind->menu;
        undraw( title, 1 );
        menu = o;
      }
      else finish_menu(ap);
  }
  else if( (ap->mouse_b&1) == state )
  {
/*    if( has_pkey() )	 004: was has_key() 
    {
      keybd = 1;
      if( (x=menu_keybd()) == -2 )
      {
        if( wind_menu() )
        {
          undraw( title, sd->entry==0 );
          sd->entry=0;
          finish_menu(ap);
          return;
        }
      }
      else if( test_pop() || x )
      {
        finish_menu(ap);
        return;
      }
    }
    else*/ if( !keybd && (x=objc_find( menu, 2, 1, ap->mouse_x, ap->mouse_y )) >= m_first &&
        (!m_last /* 004 */ || x<=m_last) )
    {     /* in bar */
      alts_off();
      if( x != title )
        if( is_disab(menu,x) )
        {
/*          if( title )
          {
            undraw( title, 1 );
            sd->pull=title=sd->entry=0;	added pull for 004
          } */
          is_menu = 0;
          force_mouse(1);
          release_menu();
          return;
        }
        else set_npull(x);
      else entry_off();
    }
    else if( !keybd ) entry_off();
  }
  else if( keybd || !state && in_rect( ap->mouse_x, ap->mouse_y, &min ) )
  {
    alts_off();
    state=1;
  }
  else if( title )
  {
    if( keybd ) entry_off();
    finish_menu(ap);
  }
  else if( state )
  {
    undraw_menu_alts();
    title_alts=num_keys=0;
    is_menu = 0;
    force_mouse(1);
    release_menu();
  }
}

void draw_kmenu(void)
{
  if( kbd_draw )
  {
    draw_menu_alts = menu_wind!=desktop;
    draw_wmenu(menu_wind);
    draw_menu_alts = kbd_draw = 0;
  }
}

void domenu( APP *ap, int *buf, int keyybd )
{ /* keybd: 0=use settings.pulldown; <0=use current button; >0=keyboard */
  int x;
  char has_mouse=0;
  long stack;

  if( !menu || menu_wind->menu_tZ<0/*004*/ ) return;
  sd->entry=title=0;
  acc_pop.mn_item = -1;
  keybd = keyybd;
  num_keys = title_alts = 0;	/* zero-out kbd equivs */
  menu_buf = buf;
  if( keybd>0 )
  {
    if( (x=check_title(m_head))==0 ) return;
    get_menu_alts( menu, 2, menu_wind==desktop );  /* get title alts */
    title_alts = num_keys;
    if( menu_wind!=desktop )
    {
      regenerate_rects( menu_wind, 0 );
      draw_menu_alts = 1;
      undraw_menu_alts();
      draw_menu_alts = 0;
    }
    state=0;
    force_mouse(0);
    has_mouse++;
    pull_it(x);
    draw_kmenu();
    if( test_pop() )
    {
      finish_menu(ap);
      return;
    }
    get_mks();
  }
  else
  {
    sd->pull=0;
    state = ap->mouse_b&1;
    keybd=0;
  }
  if( (m_first=menu_wind->menu_tA) == 0 ) m_first = menu[2].ob_head;
  m_last=menu_wind->menu_tZ;
  /* 004 if( (last=menu_wind->menu_tZ) == 0 ) last = menu[2].ob_tail;*/
  if( keybd || in_rect( ap->mouse_x, ap->mouse_y, &min ) )
  {
    if( preempt )	/* 004 */
    {
      stack = Super(0L);
      while( update.i ) Syield();
      in_menu = menu_wind;	/* set in Super mode to prevent someone else from getting it */
      Super((void *)stack);
    }
    else in_menu = menu_wind;
    is_menu = 1;
    ap->event &= ~MU_BUTTON;
    /* 004 if( !has_mouse )*/  force_mouse(0);
    /* 004: move the rest into menu_event */
    menu_event( ap, buf );
  }
  else cur_menu = 0L;
}

void draw_entry( Rect *r )
{
  _objc_draw( (OBJECT2 *)sd->tree, menu_owner, sd->entry, 0, Xrect(*r) );
}

void undraw2( Rect_list *r )
{
  Rect r2;

  while( r )
  {
    if( intersect( r->r, pull_rect, &r2 ) )
    {
      sel_if(sd->tree,sd->entry,1);
      draw_entry( &r2 );
      sel_if(sd->tree,sd->entry,0);
      draw_entry( &r2 );
    }
    r = r->next;
  }
}

void move_menu( int flag, Window *w )
{
  static int x[3], y[3], opull[3], lev;
  static OBJECT *tree[3];
  int mx, my;
  OBJECT *o;

  if( !flag )
  {
/***#ifdef DEBUG_ON	005
    if( lev>2 ) form_alert( 1, "[1][move_menu:|lev too high][OK]" );
#endif ***/
    tree[lev] = sd->tree;
    choose_menu(w);
    opull[lev] = sd->pull;
    x[lev] = (o=u_object(sd->tree,sd->pull=w->vslide))->ob_x;
    y[lev] = o->ob_y;
    objc_off( sd->tree, sd->pull, &mx, &my );
    o->ob_x += w->working.x-mx;
    o->ob_y += w->working.y-my;
    if( sd->tree==acc_tree )
    {
      o->ob_y -= acc_tree[1].ob_height<<1;	/* 007: was char_h<<1 */
      acc_tree[1].ob_flags |= HIDETREE;
      acc_tree[2].ob_flags |= HIDETREE;
    }
    lev++;
    tprop_menu( sd->tree, sd->pull );	/* 007 */
  }
  else if( lev )
  {
    prop_menu( 0L, 0 );	/* 007 */
    u_object(sd->tree,sd->pull)->ob_x = x[--lev];
    u_object(sd->tree,sd->pull)->ob_y = y[lev];
    if( sd->tree==acc_tree )
    {
      acc_tree[1].ob_flags &= ~HIDETREE;
      acc_tree[2].ob_flags &= ~HIDETREE;
    }
    sd->tree = tree[lev];
    sd->pull = opull[lev];
  }
/****#ifdef DEBUG_ON 005
  else form_alert( 1, "[1][move_menu:|lev too low][OK]" );
#endif ****/
}

Window *menu_w( int *buf, int *opull )
{
  Window *w;

  if( (w = find_window( buf[3] )) == 0L || w->dial_obj!=IS_TEAR ) return 0L;
  else
  {
    recalc_window( buf[3], w, 0L );
    regenerate_rects( w, 0 );
    move_menu( 0, w );  /* sets sd->pull */
    *opull = sd->pull;
    return w;
  }
}

int menu_evnt( APP *ap, int *buf )
{
  Window *w, *parent;
  Rect_list *r;
  Rect r2;
  int i, j;

  w = 0L;	/* 004 */
  if( ap->event & MU_KEYBD )
  {
    if( (char)ap->key == '\033' )   /* Esc key */
    {
      *(Rect *)&buf[4] = desktop->working;
      ap->event &= ~MU_KEYBD;
      goto redraw;
    }
  }
  if( ap->event & MU_MESAG )
  {
    switch( buf[0] )
    {
      case WM_REDRAW:
redraw:
        if( (w=menu_w( buf, &i )) == 0 ) return 0;	/* 004 */
        set_attaches( menu, 0, i );	/* 004 */
        r = w->rects;
        while( r )
        {
          if( intersect( r->r, *(Rect *)&buf[4], &r2 ) )	/* 004: r2 was r->r */
              _objc_draw( (OBJECT2 *)menu, menu_owner, i, 8, Xrect(r2) );
          r = r->next;
        }
        set_attaches( menu, 1, i );	/* 004 */
        break;
      case WM_TOPPED:
        if( (w=menu_w( buf, &i )) == 0 ) return 0;	/* 004 */
        _wind_set( w, WF_TOP, buf[3] );
        break;
      case WM_CLOSED:
        if( (w=menu_w( buf, &i )) == 0 ) return 0;	/* 004 */
        close_del(w);
        break;
      case WM_MOVED:
        if( (w=menu_w( buf, &i )) == 0 ) return 0;	/* 004 */
        _set_window( buf[3], WF_CURRXYWH, buf[4], buf[5], buf[6], buf[7] );
        break;
      default:		/* 004 */
        return 0;
    }
    ap->event &= ~MU_MESAG;
  }
  else if( ap->event & MU_BUTTON )
  {
    if( (w=menu_w( buf, &i )) == 0 ) return 0;	/* 004 */
    set_attaches( menu, 0, i );	/* 004 */
    pull_rect = w->working;
    parent = w->tear_parent;
    sd->entry = 0;
    ap->event = 0;
    for(;;)
    {
      get_mks();
      if( ap->mouse_b&1 )
      {
        i = objc_find( menu, sd->pull, 8, ap->mouse_x, ap->mouse_y );
        if( i != sd->entry )
        {
          r = w->rects;
          if( sd->entry ) undraw2(r);
          sd->entry=0;
          if( i>0 )
            if( !is_disab(menu,i) )
            {
              sd->entry = i;
              while( r )
              {
                if( intersect( r->r, pull_rect, &r2 ) )
                {
                  sel_if(menu,i,1);
                  draw_entry(&r2);
                }
                r = r->next;
              }
              if( test_sub(0) )  /* 004 */
              {
                is_menu = 1;
                i = submenu( &pull_rect, 0L, 1 );
                is_menu = 0;
                if( !i )
                {
                  undraw2( w->rects );
                  if( ap->event )	/* was it set by submenu? */
                  {
                    memcpy( buf, menu_buf, 16 );
                    buf[7] = sub_pop.mn_menu;
                    goto finish;
                  }
                  else break;
                }
              }
            }
        }
      }
      else if( sd->entry )
      {
        undraw2( w->rects );
        if( w->hslide != m_head || !check_acc() )
        {
          ap->event = MU_MESAG;
          buf[1] = w->apid;
          buf[4] = sd->entry;
          *(OBJECT **)&buf[5] = menu;
          buf[7] = parent==desktop ? sd->pull : parent->handle;
finish:   buf[0] = parent==desktop ? MN_SELECTED : X_MN_SELECTED;
          tnormal_menu( menu, buf[3]=w->hslide, 0 );
        }
        set_attaches( menu, 1, i );	/* 004 */
        move_menu( 1, menu_wind/*004: was w */ );	/* just in case test_unload() doesn't return */
        test_unload();                /* won't return if term worked */
        move_menu( 0, menu_wind/*004: was w */ );
        break;
      }
      else break;
    }
    ap->event &= ~MU_BUTTON;
    reset_butq();
  }
  if(w)/*004*/ free_rects( w, 0 );
  set_attaches( menu, 1, i );	/* 004 */
  move_menu( 1, menu_wind/*004: was w */ );
  return 1;
}

void choose_menu( Window *w )
{
  int x;
  APP *ap;

  if( (menu_wind = w) == 0L )
  {
    menu = 0L;
    return;
  }
  menu = w->menu;
  if( !in_sub ) sd->tree = menu;
  max = desktop->outer;
  bar_h = menu_h - 1;
  if( w!=desktop )
  {
    offset_objc( w->tree, WMENU, &min.x );
    *(long *)&min.w = *(long *)&w->tree[WMENU].ob_width;
    menu_owner = find_ap(w->apid);
  }
  else
  {
    min.x = desktop->outer.x;
    min.y = desktop->outer.y;
    min.w = desktop->outer.w;
    min.h = bar_h;
    menu_owner = has_menu;
  }
  if( menu ) m_head = menu[2].ob_head;
}

int menu_k(OBJECT *ob, int obj)
{
  ob = u_object(ob,obj);
  if( ob->ob_flags&HIDETREE || mnuk_out ) return 0;
  if( ob->ob_state&DISABLED ) return 1;
  if( (ob->ob_state>>8) == mnuk && (unsigned)(ob->ob_flags)>>13 == mnush )
  {
    mnuk_out=obj;
    return 0;
  }
  return 1;
}

int _menu_equiv( Window *w, int *buf, int sh, int key )
{
  int parent, i, j, e;

  if( !w->menu || (w->menu[0].ob_state&X_MAGMASK)!=X_MAGIC || w->icon_index ||
      (mnuk = key>>8)==0/*005*/ ) return 0;
  if( w->dial_obj==IS_TEAR && w->menu==desktop->menu ) w=desktop;	/* 004 */
  choose_menu( w );
  mnush = ((sh&3) != 0) | ((sh&12)>>1);
  mnuk_out=0;
  map_tree( menu, 0, -1, menu_k );
  if( mnuk_out )
  {
    parent = find_parent( menu, mnuk_out );
    for( i=u_object(menu,e=u_object(menu,menu[0].ob_head)->ob_next)->ob_head, j=0; i!=e;
        i=u_object(menu,i)->ob_next, j++ )
      if( i==parent )
      {
/*        for( i=m_head; --j>=0; i=menu[i].ob_next );
        menu[title=i].ob_state |= SELECTED;   004 */
        sel_if(menu,title=i=m_head+j,1);
        draw_title();
        buf[0] = menu_wind==desktop ? MN_SELECTED : X_MN_SELECTED;
        buf[3] = i;
        buf[4] = mnuk_out;
        *(OBJECT **)&buf[5] = menu;
        buf[7] = menu_wind==desktop ? parent :
            (menu_wind->dial_obj==IS_TEAR ? menu_wind->tear_parent->handle :	/* 004 */
            menu_wind->handle);
        return 1;
      }
  }
  return 0;
}

int menu_equiv( int *buf, int sh, int key )
{
  if( curapp->id == top_wind->apid )
    if( _menu_equiv( top_wind, buf, sh, key ) ) return 1;
  if( has_menu == curapp )
      return _menu_equiv( desktop, buf, sh, key );
  return 0;
}

void user_menu(void)
{
  int i;
  long *l1, *l2;

  for( i=4, l1=&user_mset.Display, l2=&menu_set.Display; --i>=0; )
    *l2++ = *l1++ / ticcal;             /* convert to 50 Hz timer tics */
  menu_set.Height = user_mset.Height*char_h;
  if( desktop && menu_set.Height > desktop->working.h )
      menu_set.Height = desktop->working.h;	/* 004: added range check */
}

int menu_settings( int flag, MN_SET *values )
{
  if( flag==0 ) memcpy( values, &user_mset, sizeof(MN_SET) );
  else if( values )
  {
    memcpy( &user_mset, values, sizeof(MN_SET) );
    user_menu();
  }
  return 1;
}

TEDINFO upted =
{
  "", "", "", IBM, 0, 2, 1<<8, 0, 0, 2, 0
},
dwnted =
{
  "", "", "", IBM, 0, 2, 1<<8, 0, 0, 2, 0
};

void pop_text( OBJECT *tree, int num, int ind, int reset )
{
  static int old[2][5];

  tree = u_object(tree,num);	/* 004 */
  if( !reset )
  {
    /* save type, flags, state, spec */
    memcpy( old[ind], &tree->ob_type, 10 );
    tree->ob_type = G_BOXTEXT;
    tree->ob_state = 0;			/* 007   tree->ob_flags = */
    tree->ob_flags &= FL3DMASK;		/* 007: preserve Atari 3D */
    tree->ob_spec.tedinfo = ind ? &dwnted : &upted;
  }
  else memcpy( &tree->ob_type, old[ind], 10 );
}

void part_draw( OBJECT *tree, int root, int first, int last )
{
  Rect r;
  OBJECT *t;

  offset_objc( tree, first, &r.x );
  *(long *)&r.w = *(long *)&(t=u_object(tree,first))->ob_width;
  r.h += u_object(tree,last)->ob_y - t->ob_y;
  form_app = menu_owner;
  adjust_rect( t, &r, 1 );
  _objc_draw( (OBJECT2 *)tree, menu_owner, root, 8, Xrect(r) );
  draw_pop_alts(0);
}

void set_mpos( OBJECT *tree, int root, Rect *r,
    int downscroll, int *itemoff, int offset, int draw )
{
  int i, end, h;
  Rect r2, r3;

  h = u_object(tree,sd->sfirst)->ob_height;
  if( downscroll )
  {
    i = *itemoff;
    end = sd->last-downscroll;
    if( (i += offset) < 0 ) i = 0;
    else if( i > end ) i = end;
    if( draw )
    {
      if( sd->up ) pop_text( tree, sd->up, 0, 1 );
      if( sd->down ) pop_text( tree, sd->down, 1, 1 );
      sd->up=sd->down=0;
    }
    if( i ) pop_text( tree, sd->up=sd->sfirst+i, 0, 0 );
    if( i<end ) pop_text( tree, sd->down=downscroll+i, 1, 0 );
    u_object(tree,root)->ob_y += (*itemoff-i) * h;
    if( i==*itemoff ) return;   /* no draw */
    *itemoff = i;
  }
  if( draw )
    if( offset>1 || offset<-1 || !offset )
    {
      _objc_draw( (OBJECT2 *)tree, menu_owner, root, 8, Xrect(*r) );
      draw_pop_alts(0);
    }
    else
    {
      r2 = *r;
      r2.h -= h*3-1;
      r2.y += h;
      r3 = r2;
      r3.y += h;
      sd->first += *itemoff;
      sd->sbot = downscroll += *itemoff;
      sd->stop = sd->sfirst + *itemoff;
      if( offset>0 )
      {
        part_draw( tree, root, sd->stop, sd->stop );
        x_graf_blit( (GRECT *)&r3, (GRECT *)&r2 );
        part_draw( tree, root, downscroll-1, downscroll );
      }
      else
      {
        part_draw( tree, root, downscroll, downscroll );
        x_graf_blit( (GRECT *)&r2, (GRECT *)&r3 );
        part_draw( tree, root, sd->stop, sd->stop+1 );
      }
    }
}

int pop_pause(void)
{
  if( !scroltic1 ) scroltic1 = tic+menu_set.Delay;
  if( tic < scroltic1 ) return 0;
  if( !scroltic2 ) scroltic2 = tic+menu_set.Speed;
  if( tic < scroltic2 ) return 0;
  scroltic2 = 0;
  return 1;
}

int pop_obj( APP *ap, Rect *r, OBJECT *tree )
{
  int i;

  if( in_rect( ap->mouse_x, ap->mouse_y, r ) &&
      (i=objc_find( tree, sd->pull, 8, ap->mouse_x, ap->mouse_y )) > 0 &&
      !is_disab(tree,i) )
  {
    pop_tic = tic - menu_set.Display - 1;
    return i;
  }
  return 0;
}

int sub_obj( int x, int y )
{
  int j, i;

  if( tic-pop_tic > menu_set.Display )
    for( j=in_sub; --j>=is_menu; )		/* 004: changed order */	/* 005: is_menu was 1 */
      if( in_rect( x, y, &sub_dat[j].r ) &&
          (i=objc_find( sub_dat[j].tree, sub_dat[j].pull, 8, x, y )) > 0 )
        if( i==sub_dat[j].entry )
        { /* returned to a parent sub starter */
          pop_tic = tic;
          return 1;
        }
        else return 2;
  return 0;
}

void end_pop( APP *ap, int copy )
{
  if( in_sub || sd->ret>=-1/*005:was 0*/ )
  {
    mblit( 1, &sd->r2 );
    num_keys = title_alts;
  }
  else pull_rect = sd->r2;
  if( sd->downscroll )
  {
    if( sd->up ) pop_text( sd->tree, sd->up, 0, 1 );
    if( sd->down ) pop_text( sd->tree, sd->down, 1, 1 );
  }
  if( copy && sd->entry && !is_disab(sd->tree,sd->entry) )
  {
    memcpy( sd->mdata, sd->in, sizeof(MENU) );
    sd->mdata->mn_item = sd->entry>=sd->stop || sd->entry<sd->sfirst ? sd->entry : sd->entry-sd->itemoff;
    sd->mdata->mn_keystate = ap->mouse_k;
    sd->ret = 1;
  }
}

void quit_pop( APP *ap, int abort )
{
  int r, copy=1;

  set_attaches( sd->tree, 1, sd->root );
  num_keys=0;
  if( !abort )
    do
    {
      end_pop( ap, copy );
      draw_kmenu();
      copy = 0;
      if( sd->entry ) sel_if(sd->tree,sd->entry,0);
      if( in_sub )
      {
        prop_menu( 0L, 0 );		/* 007 */
        sd[-1].ret = sd->ret;
        in_sub--;
        sd--;
        if( !sd[1].ret )
        {
          if( !is_menu || in_sub ) get_menu_alts( sd->tree, sd->pull, 1 );
          break;	/* moving to an older pop */
        }
      }
      else		/* 005 */
      {
        sd->done = 1;
        return;
      }
      sd->done = 1;
    }
    while( is_menu ? in_sub>0 : in_sub>=0 );	/* 005: changed condition */
  else sd->done = 1;
/*  num_keys = title_alts;*/
}

void finish_pop( APP *ap )
{
  if( sd->entry && test_sub(0) )	/* clicked on sub -> */
  {
    if( submenu( &sd->r3, sd->mdata, 0 ) ) return;
    if( is_menu ) sd->ret = -2;
    else
    {
      sd->ret = 1;
      sd->mdata->mn_keystate = ap->mouse_k;
    }
  }
  if( sd->entry ) sel_if(sd->tree,sd->entry,0);
  if( !in_sub )
  {
    while( ap->mouse_b&1 ) get_mks();
    reset_butq();
  }
  quit_pop( ap, 0 );
}

int tear_away( APP *ap )
{
  Rect drag;
  Window *w;
  int y;

      if( ap->mouse_k&4 && ap->mouse_b&1 )
      {
        alts_off();
        entry_off();
        drag = sd->r;
        if( is_acc )
            drag.h = u_object(sd->tree,sd->root)->ob_height - (char_h<<1);
            /* 005: was drag.h -= char_h<<1; */
        quit_pop( ap, 0 );
        drag.w += 2;
        drag.h += 2;
        /* 005: if  used to be here */
        _graf_mouse( FLAT_HAND, 0L, 0 );
        if( dragbox_graf( 0, &drag.x, &drag.y, drag.w,
            drag.h, drag.x, drag.y, max.x-drag.w, menu_h,
            max.x+max.w+(drag.w<<1), max.y+max.h+drag.h ) )
        {
          drag.h += cel_h;
          if( (drag.y-=cel_h) < desktop->working.y )
              drag.y = desktop->working.y;
          if( (y=create_window( NAME|MOVER|CLOSER, 0, &drag )) > 0 )
          {
            w = find_window(y);
            w->dial_obj = IS_TEAR;
            w->bevent = settings.flags.s.tear_aways_topped;
            if( is_acc )
            {
              w->menu=acc_tree;
              acc_tear++;
              w->top_bar = APPLIC;
              w->apid = -1;
              w->hslide = -1;
              w->vslide = 0;
            }
            else
            {
              w->menu=menu;
              w->top_bar = center_title( title );
              w->apid = (menu_wind==desktop?has_menu->id:menu_wind->apid);
              w->hslide = title;
              w->vslide = sd->pull;
            }
            w->tear_parent = menu_wind;
            opn_wind(w);
          }
        }
        _graf_mouse( ARROW, 0L, 0 );
        return 1;
      }
  return 0;
}

void pop_event( APP *ap )
{
  char ok;
  int xy, i;

    if( !is_menu && has_pkey()/*004*/ || (ap->mouse_b&1) == state )
    {
      if( has_pkey() )
      {
        if( !keybd )	/* 004 */
        {
          keybd=1;
          get_menu_alts( sd->tree, sd->root, 0 );
          draw_pop_alts(0);
        }
        ok = 1;		/* 004 */
        if( !sd->entry )
        {
          new_entry( sd->sfirst+sd->itemoff, 1 );
          ok = 0;	/* 004: go to first item in list if dn arrow */
        }
        switch( i=menu_keybd() )
        {
          case -1:
            finish_pop(ap);
            return;
          case -2:
            if( is_menu )
              if( wind_menu() )
              {
                entry_off();
                finish_pop(ap);
                return;
              }
            break;
          case -3:
            sd->ret = -2;
            finish_pop(ap);
            return;
          case 0:
            break;
          default:
            if( sd->up && i<=sd->up || sd->down && i>=sd->down )
            {
              xy = i-sd->entry;
              entry_off();
              set_mpos( sd->tree, sd->root, &sd->r, sd->downscroll, &sd->itemoff, xy, 1 );
              new_entry(i,1);
            }
            else if( ok || i!=sd->entry+1 /* 004 */ ) new_entry(i,1);
        }
      }  /* over an entry in the same popup */
      else if( (!keybd || !is_menu/*004*/) && (i=pop_obj(ap,&sd->r3,sd->tree)) > 0 )
      {
        alts_off();
        if( i != sd->entry )
        {
          scroltic1 = scroltic2 = 0;
          new_entry(i,0);
        }
        else if( (i==sd->down||i==sd->up) && pop_pause() )
        {
          entry_off();
          i = i==sd->up ? -1 : 1;
          set_mpos( sd->tree, sd->root, &sd->r, sd->downscroll, &sd->itemoff, i, 1 );
          if( (i = i==-1?sd->up:sd->down) != 0 ) new_entry(i,0);
        }
        else if( test_sub(1) )  /* has a submenu */
          if( !submenu( &sd->r3, sd->mdata, 0 ) )
          {
            if( is_menu ) sd->ret = -2;
            else
            {
              sd->ret = 1;
              sd->mdata->mn_keystate = ap->mouse_k;
            }
            finish_pop(ap);
            return;
          }
      }
      else if( !keybd && is_menu /*(in_sub || is_acc)*/ && (i=objc_find( menu_root, 2, 1, ap->mouse_x,
          ap->mouse_y )) >= m_first && (!m_last || i<=m_last) )
        if( i==title && (acc_count || menu_wind!=desktop || i!=m_head) && tear_away(ap) )
        { /* drag a menu */
          return;
        }
        else if( i!=title )
        { /* new sub */
          sd->ret = -2;
          set_npull(i);
          alts_off();
          entry_off();
          quit_pop( ap, 0 );
          return;
        }
        else entry_off();
      else if( /* 004 !keybd &&*/ sub_obj( ap->mouse_x, ap->mouse_y ) == 2 )
      { /* on another entry in a parent submenu */
        alts_off();
        entry_off();
        finish_pop(ap);
      }
      else if( !keybd )
      { /* not on anything */
/*        keybd = state^1; */
        entry_off();
      }
    }
    else if( keybd || !state && in_rect( ap->mouse_x, ap->mouse_y, &min ) )
    { /* returned to menu bar */
      alts_off();
      state=1;
    }
    else if( sd->entry && (sd->entry==sd->up || sd->entry==sd->down) )	/* 004 */
    {	/* clicked on up/down arrow */
      state = keybd = 0;
    }
    else
    {
      if( sub_obj( ap->mouse_x, ap->mouse_y ) == 1 ) return;
      if( !pop_obj( ap, &sd->r3, sd->tree ) /* || keybd*/ ) entry_off();
      if( !sd->entry && in_sub ) sd->ret = -1;	/* 005: was = -2 */
      finish_pop(ap);
    }
}

int fmmax, fmmod, fmeq;
#pragma warn -par
int find_max( OBJECT *o, int num )
{
  if( num > fmmax ) fmmax = num;
  return 1;
}
#pragma warn +par

extern char ted_buf[];

int disab_prop( OBJECT2 *o2, int *font, char *s )
{
  int dum;
  char *d;

  if( (((OBJECT *)o2)->ob_state & (X_MAGMASK|X_SMALLTEXT)) == (X_MAGIC|X_SMALLTEXT) ) *font = SMALL;
  else *font = pfont_mode;
  ted_font( *font, X_SYSFONT, 0, &dum, &dum );
  if( !(o2->ob_state&DISABLED) || *s!='-' ) return 0;
  while( *s=='-' ) s++;
  if( !*s ) {
    ted_buf[0] = 0;
    return 1;
  }
  d = s+strlen(s);
  while( *--d=='-' );
  strncpy( ted_buf, s, d-s+1 );
  ted_buf[d-s+1] = 0;
  return 1;
}

int prop_equiv( OBJECT *o, int i )
{
  OBJECT2 *o2;
  int font, w;
  PROPSTR *p;
  char *s, *s0, *e, *e1, c;

  if( (o2=(OBJECT2 *)u_object(o,i))->ob_flags&HIDETREE ) return 0;
  if( i!=sd->root && o2->ob_type == G_STRING )
  {
    if( disab_prop( o2, &font, s0 = (char *)get_spec((OBJECT *)o2) ) ) {
      s0 = ted_buf;
      goto dflt;
    }
    if( (e = s0+strlen(s0))!=s0 ) {
      while( *--e==' ' )
        if( e==s0 ) goto dflt;
      s=e;
      while( *--s!=' ' )
        if( s==s0 ) goto dflt;
      e1 = s0;
      while( e1<s && *e1==' ' ) e1++;
      if( e1!=s && (*(s-1)==' ' || strchr( "^~", *(s+1) )) ) {
        s++;
        o2->ob_flags &= ~INDIRECT;
        o2->ob_spec.index = (long)(p=&((PROPSTR *)((char *)sd->old+sd->old_len))[i]);
        o2->ob_type = X_PROPSTR;
        if( (e = strchr(p->equiv=s,' ')) != 0 ) *e = 0;
        e1 = s;
        while( *e1 && strchr( "^~", *e1 ) ) e1++;
        c = *e1;
        *e1 = 0;
        p->eq_x = -(w = prop_extent( font, s, 0L ));
        if( w > fmmod ) fmmod = w;
        *e1 = c;
        w = char_w+prop_extent( font, e1, 0L );
        if( w > fmeq ) fmeq = w;
        if(e) *e = ' ';
        p->str = s0;
        while( *--s==' ' && s>s0 );
        *(s+1) = 0;
        w = char_w+char_w+prop_extent( font, s0, 0L );
      }
      else {
dflt:   s = s0+strlen(s0);
        while( *--s==' ' && s>s0 );
        c = *++s;
        *s = 0;
        w = char_w+char_w+char_w+prop_extent( font, s0, 0L );
        *s = c;
      }
      if( w > fmmax ) fmmax = w;
    }
  }
  return i==sd->root;	/* only continue past root */
}

int prop_resize( OBJECT *o, int i )
{
  OBJECT2 *o2;

  if( (o2=(OBJECT2 *)u_object(o,i))->ob_type == X_PROPSTR ) {
    ((PROPSTR *)((char *)sd->old+sd->old_len))[i].eq_x += fmmax+fmmod;
  }
  o2->ob_width = fmmax+fmmod+fmeq;
  return 1;
}

int prop_reset( OBJECT *o, int i )
{
  char *s;

  if( ((OBJECT2 *)u_object(o,i))->ob_type == X_PROPSTR ) {
    s = ((PROPSTR *)((char *)sd->old+sd->old_len))[i].str;
    s[strlen(s)] = ' ';
  }
  return i==sd->root;	/* only continue past root */
}

void mfixflags( OBJECT *oldobj, OBJECT *newobj, int i )
{
  while( --i>=0 )
    *(long *)&(oldobj++)->ob_flags = *(long *)&(newobj++)->ob_flags;
}

int prop_menu( OBJECT *o, int root )
{
  int i, dum;

  if(o) {
    if( sd->old ) {
      sd->plev++;
      return 1;
    }
    sd->plev = 0;
    fmmax = 0;	/* used in find_max */
    sd->root = root;	/* not set yet if called outside menu_popup */
    map_tree( o, 0, -1, find_max );
    if( (sd->old = (OBJECT *)lalloc((sd->old_len=(fmmax+1)*sizeof(OBJECT))+
        (fmmax+1)*sizeof(PROPSTR),-1)) == 0 ) return 0;
    memcpy( sd->old, o, sd->old_len );
    fmmax = fmmod = fmeq = 0;
    map_tree( o, root, -1, prop_equiv );
    if( fmmax ) map_tree( o, root, -1, prop_resize );
  }
  else if( sd->old ) {
    if( sd->plev ) {
      sd->plev--;
      return 1;
    }
    map_tree( o=sd->tree, sd->root, -1, prop_reset );
    mfixflags( sd->old, o, sd->old_len/sizeof(OBJECT) );
    memcpy( o, sd->old, sd->old_len );
    lfree( sd->old );
    sd->old = 0L;
  }
  return 1;
}

int menu_popup( MENU *mnu, int xpos, int ypos, MENU *mdata )
{  /* return: -1: drag Desk menu, -2: exit w/o entry (in_sub), 0: no entry, 1: OK */
  OBJECT *tree;
  int xy[2], pxy[2], top, i;
  APP *ap = curapp;
  char ok=0;

  if( !is_menu && test_update( (void *)menu_popup ) ) return 0;
  sd->tree = tree = mnu->mn_tree;
  sd->root = mnu->mn_menu;
  sd->itemoff=0;
  sd->ret=0;
  sd->downscroll=0;
  sd->mdata = mdata;
  sd->in = mnu;
  sd->done = 0;
  sd->entry = mnu->mn_item;		/* 005 */
  if( mnu->mn_item<0 || is_hid( tree, mnu->mn_item ) || is_disab( tree, mnu->mn_item ) )  /* 005 */
      mnu->mn_item = search_entry( search_highest, sd->root, 1 );
  if( !is_menu ) {
    menu_owner = curapp;
    sd->old = 0L;
    if( menu_owner->flags.flags.s.prop_font_menus &&
        *(char *)&tree[0].ob_type >= X_GRAYMENU &&
        *(char *)&tree[0].ob_type <= X_PROPFONTRSZ ) prop_menu( tree, sd->root );	/* 007 */
  }
  set_attaches( tree, 0, sd->root );
  sd->first = u_object(tree,sd->root)->ob_head;
  /* 004: avoid problem where tree order is weird */
  top = search_entry( search_highest, sd->root, 0 );
  if( (sd->sfirst = mnu->mn_scroll)==0 ) sd->sfirst=top; /* 005: was sd->first; */
  sd->r.h = is_menu && !is_acc ? u_object(tree,sd->root)->ob_height : 0;
  for( sd->last=i=sd->first; i!=sd->root; sd->last=i, i=u_object(tree,i)->ob_next )
    if( !is_hid(tree,i) )  /* 004: check hidden */
    {
      /* make sure sfirst is a child of root (avoid mn_scroll==1 falsely) */
      if( i==sd->sfirst ) ok=1;
      if( mnu->mn_scroll && sd->r.h >= menu_set.Height )
      {
        if( !sd->downscroll ) sd->downscroll = sd->last;
      }
      else if( (!is_menu || is_acc) &&
          (xy[0] = u_object(tree,i)->ob_y + u_object(tree,i)->ob_height) > sd->r.h ) sd->r.h = xy[0];  /* 004: find last item */
    }
  if( !ok ) sd->sfirst = sd->first;
  for( sd->sprev=-1, i=sd->first; i!=sd->root; i=u_object(tree,i)->ob_next )
    if( u_object(tree,i)->ob_next == sd->sfirst ) sd->sprev = i;	/* find last non-scrollable obj */
  sd->first = top;	/* 004 */
  if( sd->downscroll && mnu->mn_item > sd->downscroll )
    if( (sd->last-sd->sfirst+1) - (sd->itemoff = mnu->mn_item-sd->sfirst-1) < sd->downscroll-sd->sfirst+1 )
        sd->itemoff = sd->last-sd->downscroll;
  *(long *)&u_object(tree,sd->root)->ob_x = 0L;
  offset_objc( tree, sd->root, pxy );
  sd->r.x = xpos;
  u_object(tree,sd->root)->ob_x = xpos - pxy[0];
  offset_objc( tree, sd->entry/*005: was mnu->mn_item*/-sd->itemoff, xy );
  if( (i = ypos-xy[1])+pxy[1] < desktop->working.y ) i =
      desktop->working.y-pxy[1];
  else if( i+sd->r.h > desktop->working.y+desktop->working.h-4 ) i =
      desktop->working.y+desktop->working.h-4-sd->r.h;
  u_object(tree,sd->root)->ob_y = i;
  sd->r.y = i + pxy[1];
  sd->r.w = u_object(tree,sd->root)->ob_width;
  sd->r2=sd->r;
  sd->r2.x--; sd->r2.y--;
  sd->r2.w+=2; sd->r2.h+=2;
  form_app = menu_owner;
  if( !is_menu/*004*/ ) adjust_rect( u_object(tree,sd->root), &sd->r2, 1 );
  if( !mblit( 0, &sd->r2 ) )
  {
    quit_pop( ap, 1 );
    return sd->ret;
  }
  i = u_object(tree,sd->root)->ob_height;
  u_object(tree,sd->root)->ob_height = sd->r.h;
  _objc_draw( (OBJECT2 *)tree, menu_owner, sd->root, 0, Xrect(sd->r2) );
  u_object(tree,sd->root)->ob_height = i;
  if( (i=u_object(tree,sd->root)->ob_spec.obspec.framesize)>0 ) sd->r.h += i;
  sd->r3 = sd->r;
  sd->r.y += u_object(tree,sd->sfirst)->ob_y;
  sd->r.h -= u_object(tree,sd->sfirst)->ob_y;
  sd->up=sd->down=0;
  i = sd->itemoff;
  sd->itemoff = 0;
  get_mks();
  if( !is_menu && !in_sub/*005*/ )
  {
    state = ap->mouse_b&1;
    keybd = !state && !in_rect( ap->mouse_x, ap->mouse_y, &sd->r );		/* 005: changed */
  }
  num_keys = is_menu ? title_alts : 0;
  get_menu_alts( tree, sd->root, 0 );
  if( in_sub==1 ) menu_root = sd[-1].tree;
  sd->tree = tree;
  sd->stop = sd->sfirst+sd->itemoff;
  sd->sbot = sd->downscroll ? sd->downscroll+sd->itemoff : sd->last;
  if( sd->sfirst!=sd->first ) part_draw( tree, sd->root, sd->first, sd->sbot );
  set_mpos( tree, sd->root, &sd->r, sd->downscroll, &sd->itemoff, i, 0 );
  _objc_draw( (OBJECT2 *)tree, menu_owner, sd->root, 8, Xrect(sd->r) );
  draw_pop_alts(0);
  sd->entry=0;
  sd->pull = mnu->mn_menu;
  if( keybd ) new_entry(mnu->mn_item,1);
  for(;;)
  {
    pop_event(ap);
    if( is_menu || sd->done ) break;
    get_mks();
  }
  if( !is_menu ) prop_menu( 0L, 0 );		/* 007 */
  return sd->ret>0 ? sd->ret : 0;	/* 005 <0 -> 0 */
}

int menu_attach( int flag, OBJECT *tree, int item, MENU *mdata )
{
  INT_MENU *im;
  unsigned int ii;

  im = find_sub(curapp,(OBJECT2 *)tree,item,&ii);
  switch( flag )
  {
    case 0:     /* inquire */
      if( !im ) return 0;
      if( mdata ) memcpy( mdata, &im->menu, sizeof(MENU) );
      return 1;
    case 1:     /* new */
      if( mdata )
      {
        if( !im )
          if( !curapp->menu_att && (curapp->menu_att =
              (INT_MENU *)lalloc(64*sizeof(INT_MENU), curapp->id)) == 0 )
          {
            no_memory();
            return 0;
          }
          else if( curapp->attaches>=64 ) return 0;     /* limit attaches */
          else im = &curapp->menu_att[ii=curapp->attaches++];
        else im->parent->ob_typex = 0;
        memcpy( &im->menu, mdata, sizeof(MENU) );
        (im->parent = (OBJECT2 *)u_object(tree,item))->ob_typex = ii+128;
        return 1;
      }
    case 2:     /* delete */
      if( !im ) return 0;
      if( !--curapp->attaches )
      {
        lfree(curapp->menu_att);
        curapp->menu_att=0;
      }
      else
      {
        im->parent->ob_typex = 0;
        memcpy( im, im+1, (ii=curapp->attaches-ii)*sizeof(INT_MENU) );
        for( ; ii-- > 0; im++ )
          im->parent->ob_typex--;
      }
  }
  return 0;
}

int menu_istart( int flag, OBJECT *tree, int imenu, int item )
{
  INT_MENU *im;
  int i;

  if( (im=curapp->menu_att) == 0 ) return 0;
  for( i=curapp->attaches; --i>=0; im++ )
    if( im->menu.mn_tree==tree && im->menu.mn_menu==imenu )
    {
      if( flag ) im->menu.mn_item = item;
      return im->menu.mn_item;
    }
  return 0;
}

