#include "new_aes.h"
#include "xwind.h"              /* Geneva extensions */
#include "tos.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "lerrno.h"
#include "gnva_mac.h"
#include "common.h"

EDITDESC *find_edesc( int hand )
{
  EDITDESC *e;

  for( e=elist; e; e=e->next )
    if( e->hand==hand ) return e;
  return 0L;
}

int evnt_list( EDITDESC *e, int mode )
{
  static int prev;

  if( !mode ) prev = 0;
  for( ; prev<e->refs; prev++ )
    if( e->o[e->ref[prev]].ob_state&SELECTED ) return prev++;
  return -1;
}

void evnt_off( EDITDESC *e, int num, int draw )
{
  unsigned int *i;

  if( *(i=&e->o[num].ob_state)&SELECTED )
  {
    *i &= ~SELECTED;
    if(draw) x_wdial_draw( e->hand, num, 8 );
  }
}

void evnts_off( EDITDESC *e, int draw )
{
  int i, *r;

  for( r=e->ref, i=e->refs; --i>=0; )
    evnt_off( e, *r++, draw );
/*  edits_on( &e->edits_on, e->o, e->hand, 0 );*/
  e->last_on = -1;
}

MACBUF *get_mb( EDITDESC *e, int num )
{
  return &e->m->mb[num];
}

void do_einfo( EDITDESC *e )
{
  int i, count, l;
  long events, b;
  MACBUF *mb;
  char temp[sizeof(main_info)];

  events = b = 0L;
  i = evnt_list( e, 0 );
  if( i>=0 )
  {
    do
    {
      if( get_mb(e,i)->type==TYPE_MSG ) b++;
      events++;
    }
    while( (i = evnt_list( e, 1 )) >= 0 );
    x_sprintf( temp, *einfo, events, *selected, events*sizeof(MACBUF)+b*5*31 );
  }
  else if( e->m->len == 0 ) temp[0] = 0;
  else x_sprintf( temp, *einfo, (long)e->m->len, "", (long)e->m->len*sizeof(MACBUF)+count_msgs(e->m) );
  if( strcmp( temp, e->info ) )
  {
    strcpy( e->info, temp );
    wind_set( e->hand, WF_INFO, e->info );
  }
}

int find_evnt( EDITDESC *e, int but )
{
  int i, *r;

  for( r=e->ref, i=0; i<e->refs; i++, r++ )
    if( but >= *r && but <= e->o[*r].ob_tail ) return i;
  return -1;
}

void read_xy( EDITDESC *e, int but, MACBUF *mb )
{
  int x, y, b, k, draw;
  char **t;

  rsrc_gaddr( 15, GETXY, &t );
  wind_set( e->hand, WF_NAME, *t );
  wind_update( BEG_UPDATE );
  draw = but;
  for(;;)
  {
    evnt_button( 0x101, 3, 0, &x, &y, &b, &k );
    if( b&2 ) break;
    if( b&1 )
    {
      draw = but-EVMREAD+EMOUSE;
      x_sprintf( e->o[draw+EVMX-EMOUSE].ob_spec.tedinfo->te_ptext,
          "%d", mb->u.mouse.x=x );
      x_sprintf( e->o[draw+EVMY-EMOUSE].ob_spec.tedinfo->te_ptext,
          "%d", mb->u.mouse.y=y );
      break;
    }
  }
  but_up();
  wind_update( END_UPDATE );
  set_if( e->o, but, 0 );
  x_wdial_draw( e->hand, draw, 8 );
  wind_set( e->hand, WF_NAME, e->m->name );
}

void obj_date_str( OBJECT *o, int i, MACBUF *mb )
{
  o[i+EVDPOP-EDATE].ob_spec.free_string = date_pop[mb->u.sound.index+1].ob_spec.free_string+2;
}

void obj_time_str( OBJECT *o, int i, MACBUF *mb )
{
  o[i+EVTPOP-ETIME].ob_spec.free_string = time_pop[mb->u.sound.index+1].ob_spec.free_string+2;
}

unsigned int do_popup( OBJECT *o, int obj, OBJECT *pop, unsigned int val )
{
  MENU m, out;
  int x, y;

  m.mn_tree = pop;
  m.mn_menu = 0;
  m.mn_item = val+1;
  m.mn_scroll = 1;
  for( x=1; x<=pop[0].ob_tail; x++ )
    if( x==val+1 ) pop[x].ob_state |= CHECKED;
    else pop[x].ob_state &= ~CHECKED;
  objc_offset( o, obj, &x, &y );
  if( menu_popup( &m, x, y, &out ) ) return out.mn_item-1;
  return val;
}

#pragma warn -par
void edit_sel( EDITDESC *e, int dclick, int but, int keys )
{
  OBJECT *o = e->o;
  MACDESC *m = e->m;
  int i, dum;
  long l;
  MACBUF *mb;
  KEYCODE k;

  if( o[but].ob_head<0 && o[but-1].ob_head==but ) but--;	/* panel type */
  else if( o[but].ob_flags&(1<<11) ) but -= EVMSAM-EMSG;
  if( o[but].ob_head<0 )  /* not panel */
  {
    if( o[but].ob_type==G_BUTTON )	/* int compare is right! */
    {
      if( !test_eglob(e) ) return;
      if( (i = find_evnt(e,but)) >= 0 )
        if( (mb=&m->mb[i])->type == TYPE_MOUSE ) read_xy( e, but, mb );
        else if( mb->type == TYPE_DATE )
        {
          if( (dum = do_popup( o, but, date_pop, mb->u.sound.index )) != mb->u.sound.index )
          {
            mb->u.sound.index = dum;
            obj_date_str( o, dum=e->ref[i], mb );
            x_wdial_draw( e->hand, but, 0 );
          }
        }
        else if( mb->type == TYPE_TIME )
        {
          if( (dum = do_popup( o, but, time_pop, mb->u.sound.index )) != mb->u.sound.index )
          {
            mb->u.sound.index = dum;
            obj_time_str( o, dum=e->ref[i], mb );
            x_wdial_draw( e->hand, but, 0 );
          }
        }
        else if( mb->type == TYPE_MSG )
        {
          msg_edit = e;
          msg_mac = mb->u.msg;
          msg_ind = i;
          start_form( 5, NEWMSG, NAME|MOVER, 0 );
          set_if( o, but, 0 );
          x_wdial_draw( e->hand, but, 0 );
        }
        else
        {
          *(long *)&k = mb->u.timer<<8L;
          read_key( o, but, &k, e->hand, m->name, 1 );
          mb->u.timer = *(long *)&k>>8L;
        }
    }
/*    else if( !e->edits_on )
    {
      edits_on( &e->edits_on, o, e->hand, 1 );
      wind_set( e->hand, X_WF_DIALEDIT, but, -1 );
    } */
    else if( o[but].ob_flags&EXIT )
    {
      wind_set( e->hand, WF_NAME, *evedit );
      o[but].ob_flags |= DEFAULT|EDITABLE;
      form_do( o, but );
      o[but].ob_flags &= ~(DEFAULT|EDITABLE);
      wind_set( e->hand, WF_NAME, e->m->name );
      set_if( o, but, 0 );
      x_wdial_draw( e->hand, but, 0 );
      l = atol( o[but].ob_spec.tedinfo->te_ptext );
      if( (i = find_evnt(e,but)) >= 0 )
        if( (dum=(mb=get_mb(e,i))->type) == 0 ) mb->u.timer = l;
        else if( dum==TYPE_MOUSE )
          if( but-e->ref[i] == EVMX-EMOUSE ) mb->u.mouse.x = l;
          else mb->u.mouse.y = l;
    }
    else	/* right/left button */
    {
      if( !test_eglob(e) ) return;
      if( (dum = find_evnt(e,but)) >= 0 )
      {
        i = e->ref[dum];
        if( (mb=get_mb(e,dum))->type==TYPE_BUTTN )
        {
          dum = 0;
          if( o[i+EVLEFT-EBUTTON].ob_state&SELECTED ) dum |= 1;
          if( o[i+EVRIGHT-EBUTTON].ob_state&SELECTED ) dum |= 2;
          mb->u.button.state = dum;
        }
        else if( mb->type == TYPE_SOUND )
        {
          for( dum=0; dum<3; dum++, i++ )
            if( o[i+EVBELL-ESOUND].ob_state&SELECTED ) mb->u.sound.index = dum;
        }
      }
    }
  }
/*  else if( dclick )         /* double-click */
  {
    if( but )
    {
      i = evnt_list( e, 0 );
      if( i>=0 )
        do edit_mac(i);
        while( (i = mac_list(1)) >= 0 );
    }
    macs_off(1);
  } */
  else
  {
    if( keys&4 && e->last_on>=0 )
    {
      if( but < e->last_on )
      {
        i = but;
        dum = e->last_on-1;
      }
      else
      {
        i = e->last_on+1;
        dum = but;
      }
      for( ; i<=dum; i++ )
        if( o[i].ob_head >= 0 )
        {
          set_if( o, i, 1 );
          x_wdial_draw( e->hand, i, 8 );
        }
      e->last_on = but;
    }
    else if( !(o[but].ob_state&SELECTED) )
    {
      if( !(keys&3) ) evnts_off( e, 1 );
      set_if( o, but, 1 );
      x_wdial_draw( e->hand, but, 8 );
      e->last_on = but;
    }
    else if( !(keys&3) ) evnts_off( e, 1 );
    else
    {
      evnt_off( e, but, 1 );
      e->last_on = -1;
    }
    but_up();
  }
  do_einfo(e);
}
#pragma warn +par

void edit_dial( EDITDESC *e )
{
  wind_set( e->hand, X_WF_DIALOG, e->o );
}

void cmfree( void **p )
{
  if( *p )
  {
    xmfree( *p );
    *p = 0L;
  }
}

void free_edit( EDITDESC *e )
{
  MACBUF *mb;
  int i;

  if( e->o && e->ref )
    for( mb=e->m->mb, i=0; i<e->m->len; i++, mb++ )
      if( mb->type==TYPE_MSG ) xmfree( e->o[e->ref[i]+EVMSAM-EMSG].ob_spec.free_string );
  cmfree( (void **)&e->o );
  cmfree( (void **)&e->ref );
  e->refs = 0;
  cmfree( (void **)&e->ted );
  cmfree( (void **)&e->str );
}

void cancel_rec(void)
{
  if( mac_end >= 0 ) x_appl_trecord( 0L, 0, 0L, 1 );
  rec_desc = 0L;
  rec_mac = 0L;
  check_rec();
  close_wind( &form[1].handle );
}

void close_edit( EDITDESC *e )
{
  EDITDESC *e2;

  if( aes_ok )
  {
    close_wind( &e->hand );
    if( tim_edesc==e )
    {
      tim_edesc = 0L;
      close_wind( &form[3].handle );
    }
    if( msg_edit==e )
    {
      msg_mac = 0L;
      msg_edit = 0L;
      close_wind( &form[5].handle );
    }
  }
  free_edit(e);
  if( e==elist ) elist = e->next;
  else
  {
    for( e2=elist; ; )
      if( e2->next == e )
      {
        e2->next = e->next;
        break;
      }
      else if( (e2=e2->next) == 0 ) return;
  }
  if( e==rec_desc ) cancel_rec();
  xmfree(e);
}

void free_edits( int ac_close )
{
  EDITDESC *e, *e2;

  e2 = elist;
  while( (e=e2) != 0 )
  {
    e2 = e->next;
    if( ac_close ) e->hand = 0;
    close_edit(e);
  }
}

int add_ref( EDITDESC *e )
{
  return add_thing( (void **)&e->ref, &e->refs, &e->ref_rem, 0L, sizeof(int) );
}

void set_elist_ht( EDITDESC *e )
{
  int i;

  e->o[0].ob_height = (i=e->m->len*panel_h) > e->inner.g_h ? i : e->inner.g_h;
  e->o[0].ob_width = e->inner.g_w;
}

char *add_str( EDITDESC *e )
{
  if( add_thing( (void **)&e->str, &e->nstr, &e->str_rem, 0L, 11 ) )
      return e->str + 11*(e->nstr-1);
  return 0L;
}

TEDINFO *add_ted( EDITDESC *e, int copy )
{
  char *s;
  TEDINFO *t;

  if( (s = add_str(e)) == 0 ) return 0L;
  if( add_thing( (void **)&e->ted, &e->nted, &e->ted_rem,
      copy ? events[copy].ob_spec.tedinfo : 0L, sizeof(TEDINFO) ) )
  {
    t = &e->ted[e->nted-1];
    t->te_ptext = s;
    return t;
  }
  return 0L;
}

void msg_str( EDITDESC *e, int r, int draw )
{
  char *s = e->o[e->ref[r]+EVMSAM-EMSG].ob_spec.free_string;

  strncpy( s, (*e->m->mb[r].u.msg)[0], 26 );
  s[26] = 0;
  if( draw ) x_wdial_draw( e->hand, e->ref[r], 8 );
}

int evnt_tree( EDITDESC *e )
{
  int i, c, count;
  long oc;
  OBJECT *o;
  TEDINFO *ted;
  MACBUF *mb;
  char *s;
  static OBJECT obj = { -1, -1, -1, G_BOX, 0, X_MAGIC|X_KBD_EQUIV, 0 };
  static char xref[] = { ETIMER, EBUTTON, EMOUSE, EKEYBD, ESOUND, EDATE, ETIME, EMSG };

  e->last_on = -1;
  bee();
  oc = e->o ? *(long *)&e->o[0].ob_x : -1L;
  free_edit(e);
  if( (long)e->m->len*panel_h > 32767 )
  {
    alert(OBJBIG);
    close_edit(e);
    arrow();
    return 0;
  }
  if( !add_obj( &e->o, &e->nobj, &e->obj_rem, &obj ) )
  {
    close_edit(e);
    arrow();
    return 0;
  }
  *(long *)&e->o[0].ob_x = oc!=-1L ? oc : *(long *)&e->inner.g_x;
  count = 1;
  for( i=0; i<e->m->len; i++ )
  {
    if( (c=(mb=get_mb(e,i))->type) >= TYPE_SOUND ) c -= TYPE_SOUND-TYPE_KEY-1;
    o = &events[xref[c]];
    c = e->nobj;
    if( !add_tree( &e->o, &count, &e->nobj, &e->obj_rem, o, i, panel_h ) ||
        !add_ref(e) )
    {
      close_edit(e);
      arrow();
      return 0;
    }
    e->ref[e->refs-1] = c;
    o = &e->o[c];
    switch( mb->type )
    {
      case TYPE_TIMER:
        if( !add_ted( e, EVTIMER ) )
        {
          close_edit(e);
          arrow();
          return 0;
        }
        break;
      case TYPE_BUTTN:
        break;
      case TYPE_MOUSE:
        if( !add_ted( e, EVMX ) || !add_ted( e, EVMY ) )
        {
          close_edit(e);
          arrow();
          return 0;
        }
        break;
      case TYPE_KEY:
        if( !add_str(e) )
        {
          close_edit(e);
          arrow();
          return 0;
        }
        break;
      case TYPE_SOUND:
      case TYPE_DATE:
      case TYPE_TIME:
        break;
      case TYPE_MSG:
        if( (s=o[EVMSAM-EMSG].ob_spec.free_string = xmalloc( 27 )) == 0 )
        {
          close_edit(e);
          arrow();
          return 0;
        }
        msg_str( e, e->refs-1, 0 );
        break;
    }
    if( e->m->is_global )
      for( c=e->nobj-c-1, o+=2; --c>0; o++ )
        if( o->ob_flags & (SELECTABLE|EXIT) )
            o->ob_flags = o->ob_flags & ~(SELECTABLE|EXIT) | TOUCHEXIT;
  }
  for( i=e->m->len, mb=get_mb(e,0), ted=e->ted, s=e->str, o=e->o+1; --i>=0; mb++ )
  {
    switch( mb->type )
    {
      case TYPE_TIMER:
        x_sprintf( ted->te_ptext=s, "%D", mb->u.timer );
        o[EVTIMER-ETIMER].ob_spec.tedinfo = ted++;
        s += 11;
        break;
      case TYPE_BUTTN:
        set_if( o, EVLEFT-EBUTTON, mb->u.button.state&1 );
        set_if( o, EVRIGHT-EBUTTON, mb->u.button.state&2 );
        break;
      case TYPE_MOUSE:
        x_sprintf( ted->te_ptext=s, "%d", mb->u.mouse.x );
        o[EVMX-EMOUSE].ob_spec.tedinfo = ted++;
        x_sprintf( ted->te_ptext = (s+=11), "%d", mb->u.mouse.y );
        o[EVMY-EMOUSE].ob_spec.tedinfo = ted++;
        s += 11;
        break;
      case TYPE_KEY:
        o[EVKKEY-EKEYBD].ob_spec.free_string = s;
        key_str( o, EVKKEY-EKEYBD, (KEYCODE *)((long)&mb->u.timer+1), 1 );
        s += 11;
        break;
      case TYPE_SOUND:
        for( c=0; c<3; c++ )
          set_if( o, c+EVBELL-ESOUND, c==mb->u.sound.index );
        break;
      case TYPE_DATE:
        obj_date_str( o, 0, mb );
        break;
      case TYPE_TIME:
        obj_time_str( o, 0, mb );
        break;
    }
    o += o->ob_tail - o->ob_head + 2;
  }
  set_elist_ht(e);
  arrow();
  return 1;
}

int shift_evnt( EDITDESC *e, int num, int inc, MACBUF *copy )
{
  MACDESC *m;
  MACBUF *mb;
  int i;

  m = e->m;
  if( e==msg_edit && num>=msg_ind ) msg_ind += num;
  if( inc>0 )
  {
    if( m->len+inc > macsize )
    {
      alert(MACLONG);
      return 0;
    }
    if( xrealloc( (void **)&m->mb, (m->len+inc)*sizeof(MACBUF) ) ) return 0;
    memcpy( &m->mb[num+inc], &m->mb[num], (m->len-num)*sizeof(MACBUF) );
    m->len += inc;
    if( copy ) memcpy( &m->mb[num], copy, inc*sizeof(MACBUF) );
  }
  else
  {
    m->len += inc;
    for( mb=&m->mb[num], i=inc; ++i<=0; mb++ )
      free_msg(mb);
    memcpy( &m->mb[num], &m->mb[num-inc], (m->len-num)*sizeof(MACBUF) );
    if( xrealloc( (void **)&m->mb, m->len*sizeof(MACBUF) ) ) return 0;
  }
  return 1;
}
/*    if( copy )
  int i, j, nted, nstr, nobj;
  OBJECT *o;
  MACDESC *m;
  MACBUF *mb;
  char *p;
  static int ited[] = { 1, 0, 2, 0 }, istr[] = { 0, 0, 0, 1 },
      iobj[] = { EVTIMER-ETIMER+1, EVRIGHT-EBUTTON+1, EVMY-EMOUSE+1, EVKKEY-EKEYBD+1 },
      from = { ETIMER, EBUTTON, EMOUSE, EKEYBD };

  /* find out distances to adjust */
  if( (mb = copy) == 0 ) mb = &m->mb[num];
  for( nted=nstr=nobj=0; i=abs(inc); --i>=0; mb++ )
  {
    j = mb->type;
    nted += ited[j];
    nobj += iobj[j];
    nstr += istr[j];
  }
  m = e->m;
  if( inc>0 )
  {
    if( xrealloc( (void **)&m->mb, (m->len+inc)*sizeof(MACBUF) ) return 0;
    for( i=nobj; --i>=0; )
      if( !add_obj( &e->o, &e->nobj, &e->obj_rem, 0L ) ) return 0;
    for( i=nstr; --i>=0; )
      if( !add_str(e) ) return 0;
    for( i=nted; --i>=0; )
      if( !add_ted( e, 0 ) ) return 0;
    for( i=inc; --i>=0; )
      if( !add_ref(e) ) return 0;
  }
  else
  {
    e->nobj -= nobj;
    e->obj_rem += nobj;
    o = &e->o[e->ref[num]];
    memcpy( o, o+nobj, (e->nobj-e->ref[num])*sizeof(OBJECT) );
    e->nstr -= nstr;
    e->str_rem += nstr;
    memcpy( s, s+nstr*11, &e->str[(long)e->nstr*11]-s );
    e->nted -= nted;
    e->ted_rem += nted;
    memcpy( ted, ted+nted, &e->ted[e->nted]-ted );
    e->refs += inc;
    e->ref_rem -= inc;
    memcpy( &e->ref[num], &e->ref[num-inc], (e->refs-num)<<1 );
  }
  while( (j = ited[mb->type]) == 0 ) mb++;
  while( --j>=0 )
  mb = &m->mb[num];
  e->nobj += dif;
  m->len += inc;
  if( inc>0 )
  {
    num += inc;
    mb += inc;
  }
  else
  {
    macs_rem -= inc;
    num_mobj += dif;
    mobj_rem -= dif;
  }
  i=m->obj;
  memcpy( o=&maclist[i], &maclist[i-dif], (last_mobj-i)*sizeof(OBJECT) );
  memcpy( m, m-inc, (nmacs-num)*sizeof(MACDESC) );
  for( ; i<last_mobj; i++, o++ )
  {
    mov_obj( &o->ob_next, dif );
    mov_obj( &o->ob_head, dif );
    mov_obj( &o->ob_tail, dif );
  }
  if( !nmacs ) *(long *)&maclist[0].ob_head = -1L;
  else
  {
    i = dif;
    if( inc>0 ) i = -i;
    o[i].ob_next = 0;
    maclist[0].ob_tail = last_mobj+i;
  }
  reset_maclist();
  set_bl_height();
  return 1;
} */

void edit_mac( int m )
{
  EDITDESC *e;
  int h, mn;

  /* recording this macro from New Macro? */
  if( rec_mac == &mdesc[m] ) return;
  /* already open? */
  for( e=elist; e; e=e->next )
    if( e->m == &mdesc[m] )
    {
      wind_set( e->hand, WF_TOP );
      return;
    }
  if( (e = xmalloc(sizeof(EDITDESC))) == 0 ) return;
  memset( e, 0, sizeof(EDITDESC) );
  e->m = &mdesc[m];
  if( !evnt_tree(e) ) return;
  e->inner.g_y = winner.g_y;
  e->inner.g_w = events[ETIMER].ob_width;
  e->inner.g_h = 3*panel_h;	/* temporary */
  if( (e->inner.g_x = winner.g_x + winner.g_w) + e->inner.g_w >
      max.g_x+max.g_w-8 ) e->inner.g_x = max.g_x+max.g_w-8 - e->inner.g_w;
  gget_outer( &e->outer, &e->inner, panel_h );
  mn = e->outer.g_h;
  e->inner.g_h = winner.g_h;
  gget_outer( &e->outer, &e->inner, panel_h );
  gget_inner( &e->outer, &e->inner, e->o );
  if( (h=x_wind_create( WIN_TYPE, X_MENU,
      e->outer.g_x, e->outer.g_y, e->outer.g_w, e->outer.g_h )) > 0 )
  {
    wind_set( e->hand=h, X_WF_MINMAX, e->outer.g_w, mn, e->outer.g_w, -1 );
    set_elist_ht(e);
    wind_set( h, X_WF_DIALHT, panel_h );
    e->o[0].ob_x = e->inner.g_x;
    e->o[0].ob_y = e->inner.g_y;
    edit_dial(e);
    wind_set( h, X_WF_MENU, emenu );
    wind_set( h, WF_NAME, e->m->name );
    e->info[0] = 1;	/* so that it will get set first time */
    do_einfo(e);
    wind_open( h, e->outer.g_x, e->outer.g_y, e->outer.g_w, e->outer.g_h );
  }
  else
  {
    alert( NOWIND );
    xmfree(e->o);
    xmfree(e);
    return;
  }
  e->next = elist;
  elist = e;
}

void temp_sel( EDITDESC *e, int i )
{
  set_if( e->o, i, 1 );
  x_wdial_draw( e->hand, i, 8 );
  edit_sel( e, 0, i, 0 );
}

void insert_event( EDITDESC *e, int n )
{
  int i, j, dum;
  MACBUF new;

  if( !test_eglob(e) ) return;
  new.u.timer = 0L;
  switch( new.type=n )
  {
    case TYPE_MOUSE:
      graf_mkstate( &new.u.mouse.x, &new.u.mouse.y, &dum, &dum );
      break;
    case TYPE_SOUND:
      new.u.sound.index = d_sound;
      break;
    case TYPE_DATE:
      new.u.sound.index = d_date;
      break;
    case TYPE_TIME:
      new.u.sound.index = d_time;
      break;
    case TYPE_MSG:
      if( (new.u.msg = xmalloc(5*31)) == 0 ) return;
      memset( *new.u.msg, 0, 5*31 );
  }
  if( (i = evnt_list(e,0)) < 0 ) i = e->m->len;
  if( shift_evnt( e, i, 1, &new ) )
  {
    evnt_tree(e);
    set_elist_ht(e);
    i = e->ref[i];
    if( (j=e->o[i].ob_y+e->o[0].ob_y) < e->inner.g_y ) e->o[0].ob_y =
        e->inner.g_y - e->o[i].ob_y;
    else if( j >= e->inner.g_y+e->inner.g_h ) e->o[0].ob_y =
        e->inner.g_y + e->inner.g_h - panel_h - e->o[i].ob_y;
    edit_dial(e);
    if( n==TYPE_TIMER ) temp_sel( e, i+EVTIMER-ETIMER );
    else if( n==TYPE_KEY ) temp_sel( e, i+EVKKEY-EKEYBD );
    else if( n==TYPE_MSG ) temp_sel( e, i+EVMEDIT-EMSG );
  }
  else evnts_off(e,1);
}

int edit_type( EDITDESC *e, int type, int func( EDITDESC *e, int num ), int backward )
{
  int i, c, sel, *r;
  MACBUF *mb;

  sel = evnt_list(e,0) >= 0;
  c = 0;
  if( backward ) for( i=e->refs, r=&e->ref[i], mb=get_mb(e,i); --i>=0; )
  {
    mb--;
    r--;
    if( (!sel || e->o[*r].ob_state&SELECTED) && mb->type==type )
      if( !(*func)(e,i) ) return c;
      else c++;
  }
  else for( i=-1, r=e->ref, mb=get_mb(e,0); ++i<e->refs; mb++, r++ )
    if( (!sel || e->o[*r].ob_state&SELECTED) && mb->type==type )
      if( !(*func)(e,i) ) return c;
      else c++;
  return c;
}

void edit_updt( EDITDESC *e, int c )
{
  if(c)
  {
    evnt_tree(e);
    set_elist_ht(e);
    edit_dial(e);
  }
}

void remove_all( EDITDESC *e, int type )
{
  if( !test_eglob(e) ) return;
  edit_updt( e, edit_type( e, type, eedelete, 1 ) );
}

long tim_val;
#pragma warn -par
int etimcount( EDITDESC *e, int num )
{
  return 1;
}
#pragma warn +par

int ealltim( EDITDESC *e, int num )
{
  x_sprintf( e->o[e->ref[num]+EVTIMER-ETIMER].ob_spec.tedinfo->te_ptext,
       "%D", get_mb(e,num)->u.timer=tim_val );
  return 1;
}

void all_timers( EDITDESC *e, long val )
{
  tim_val = val;
  if( edit_type( e, 0, ealltim, 0 ) ) edit_dial(e);
}

MACBUF *oltimer, *olbutton, *olmouse, *olkeybd;
int cevents[4];

MACBUF *oback( MACBUF *mb, MACBUF *old )
{
  for(;;)
  {
    if( ++old==mb ) return mb;
    if( old->type != -1 ) return 0L;
  }
}

int otimer( EDITDESC *e, int num )
{
  MACBUF *o;

  o = get_mb(e,num);
  if( !o->u.timer )
  {
    o->type = -1;
    cevents[0]++;
  }
  else if( !oltimer ) oltimer = o;
  else if( oback( o, oltimer ) )
  {
    o->u.timer += oltimer->u.timer;
    oltimer->type = -1;
    oltimer = o;
    cevents[0]++;
  }
  else oltimer = o;
  return 1;
}

int obutton( EDITDESC *e, int num )
{
  MACBUF *o;

  o = get_mb(e,num);
  if( olbutton && o->u.timer == olbutton->u.timer )
  {
    o->type = -1;
    cevents[1]++;
  }
  else olbutton = o;
  return 1;
}

int omouse( EDITDESC *e, int num )
{
  MACBUF *o;

  o = get_mb(e,num);
  if( olmouse && oback( o, olmouse ) != 0 )
  {
    olmouse->type = -1;
    cevents[2]++;
  }
  olmouse = o;
  return 1;
}

int okeybd( EDITDESC *e, int num )
{
  MACBUF *o;

  o = get_mb(e,num);
  if( olkeybd && oback( o, olkeybd ) != 0 )
  {
    olkeybd->type = -1;
    cevents[3]++;
  }
  if( !o->u.button.clicks ) olkeybd = o;
  else olkeybd = 0;
  return 1;
}

int iso_timers( EDITDESC *e )
{	/* remove timers surrounded by mouse mvmnt */
  int i, *r, c=0;
  char sel;
  MACBUF *pm=0L, *pt=0L, *m;

  sel = evnt_list(e,0) >= 0;
  for( m=get_mb(e,0), r=e->ref, i=e->refs; --i>=0; m++, r++ )
    if( !sel || e->o[*r].ob_state&SELECTED )
      if( pt && m->type==2 )
      {
        c = 1;
        pt->type = -1;
        pm = m;
        pt = 0L;
      }
      else if( pm && m->type==0 && !pt ) pt = m;
      else if( m->type==2 )
      {
        pm = m;
        pt = 0L;
      }
      else pm = pt = 0L;
  return c;
}

void optimize( EDITDESC *e, int type )
{
  int i, j, c=0, type0=type;
  static int (*funcs[])( EDITDESC *e, int num ) = { otimer, obutton,
      omouse, okeybd };

  if( !test_eglob(e) ) return;
  while( type )
  {
    oltimer = olbutton = olmouse = olkeybd = 0L;
    *(long *)cevents = *(long *)&cevents[2] = 0L;
    if( type&1 )
      if( iso_timers(e) ) c = 1;
    for( i=4; --i>=0; )
      if( type&(j=1<<i) )
      {
        edit_type( e, i, funcs[i], 0 );
        if( cevents[i] )
        {
          type = type0 & ~j;	/* repeat all others */
          c = 1;
        }
        else type &= ~j;
      }
    /* remove last keypress if no char */
    if( olkeybd )
    {
      olkeybd->type = -1;
      type = type0;
      c = 1;
    }
    else if( oltimer && oback( get_mb(e,e->refs), oltimer ) )	/* remove timer if last */
    {
      oltimer->type = -1;
      type = type0;
      c = 1;
    }
  }
  if(c) remove_all( e, -1 );
}

void do_emenu( EDITDESC *e, int num )
{
  char **ptr;
  int i;

  switch(num)
  {
    case ECUT:
      if( save_clip(e) ) edelete(e);
      break;
    case ECOPY:
      save_clip(e);
      evnts_off(e,1);
      break;
    case EPASTE:
      load_clip(e);
      break;
    case EDEL:
      edelete(e);
      break;
    case EHELP:
      rsrc_gaddr( 15, HELPEDIT, &ptr );
      do_help( *ptr );
      break;
    case ECLOSE:
      close_edit(e);
      break;
    case EMTIMER:
    case EMBUTN:
    case EMMOUSE:
    case EMKEYBD:
      insert_event( e, num-EMTIMER );
      break;
    case EMSOUND:
    case EMDATE:
    case EMTIME:
    case EMMSG:
      insert_event( e, num-EMSOUND+TYPE_SOUND );
      break;
    case EALLTIM:
      if( !test_eglob(e) ) break;
      if( edit_type( e, 0, etimcount, 0 ) )
      {
        tim_edesc = e;
        start_form( 3, ALLTIM, NAME|MOVER, 0 );
      }
      break;
    case EOTIMER:
    case EOBUTN:
    case EOMOUSE:
    case EOKEYBD:
      optimize( e, 1<<(num-EOTIMER) );
      break;
    case EOALL:
      optimize( e, 15 );
      break;
    case ERTIMER:
    case ERBUTN:
    case ERMOUSE:
    case ERKEYBD:
      remove_all( e, num-ERTIMER );
      break;
    case ERECORD:
      if( !test_eglob(e) ) break;
      if( (i = evnt_list(e,0)) < 0 ) i = e->m->len;
      new_mac( e, i );
      break;
    case ETEST:
/*      x_appl_tplay( get_mb(e,0), e->m->len, 100, 1 );*/
      play_mac( e->m, 1 );
      break;
  }
  do_einfo(e);
}

void updt_datepop(void)
{
  int i;
  char *p;
  static char datebuf[8][21];

  for( i=8; --i>=0; )
  {
    p = date_pop[i+1].ob_spec.free_string = datebuf[i];
    *p++ = ' ';
    *p++ = ' ';
    _strftime( p, date_fmt[i], 18, 0x1f9f, 0xbf7d );
  }
}

void updt_timepop(void)
{
  int i;
  char *p;
  static char timebuf[8][21];

  for( i=8; --i>=0; )
  {
    p = time_pop[i+1].ob_spec.free_string = timebuf[i];
    *p++ = ' ';
    *p++ = ' ';
    _strftime( p, time_fmt[i], 18, 0x1f9f, 0xbf7d );
  }
}
