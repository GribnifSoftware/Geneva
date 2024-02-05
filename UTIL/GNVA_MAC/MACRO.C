#include "new_aes.h"
#include "xwind.h"              /* Geneva extensions */
#include "tos.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "lerrno.h"
#include "gnva_mac.h"
#include "common.h"

char sound_up[] = { 0x9, 0x0, 0xa, 0x0, 0x0, 0x0, 0x8, 0xe, 0x7, 0x3e, 0x80,
    0xc, 0x81, 0x1, 0xff, 0x0, 0x7, 0x3f, 0xff, 0x0 },
     sound_dwn[] = { 0x9, 0x0, 0xa, 0x0, 0x0, 0x0, 0x8, 0xe, 0x7, 0x3e, 0x80, 0x0, 0x81, 1, 1,
    0xc, 0x7, 0x3f, 0xff, 0x0 };

int add_macro( MACDESC *m )
{
  char moved;
  int ret;
  EDITDESC *e;

  if( (moved = mdesc && !macs_rem) != 0 )
  {
    /* elist->m's might move! */
    for( e=elist; e; e=e->next )
      (long)e->m = e->m - mdesc;
  }
  ret = add_thing( (void **)&mdesc, &num_macs, &macs_rem, m, sizeof(MACDESC) );
  if( moved )
    for( e=elist; e; e=e->next )
      e->m = &mdesc[(long)e->m];
  return ret;
}

/*********************** Main window routines **********************/

/* set the height of the applications list */
void set_bl_height(void)
{
  int i;

  maclist[0].ob_height = (i=nmacs*main_h) > winner.g_h ? i : winner.g_h;
  maclist[0].ob_width = winner.g_w;
}

void gget_inner( GRECT *wsize, GRECT *winner, OBJECT *o )
{
  x_wind_calc( WC_WORK, WIN_TYPE, X_MENU, wsize->g_x, wsize->g_y, wsize->g_w,
      wsize->g_h, &winner->g_x, &winner->g_y, &winner->g_w, &winner->g_h );
  o[0].ob_width = winner->g_w;
  if( o==maclist ) set_bl_height();
}

void get_inner(void)
{
  gget_inner( &wsize, &winner, maclist );
}

void gget_outer( GRECT *wsize, GRECT *winner, int h )
{
  /* round the overall height to the nearest character height */
  winner->g_h = (winner->g_h+(h>>1))/h*h;
  x_wind_calc( WC_BORDER, WIN_TYPE, X_MENU, winner->g_x, winner->g_y,
      winner->g_w, winner->g_h, &wsize->g_x, &wsize->g_y, &wsize->g_w,
      &wsize->g_h );
}

void get_outer(void)
{
  gget_outer( &wsize, &winner, main_h );
}

void free_msg( MACBUF *mb )
{
  int i;

  if( mb->type == TYPE_MSG )
  {
    if( mb->u.msg==msg_mac )
    {
      close_wind( &form[5].handle );
      msg_mac = 0L;
    }
    for( i=sizeof(msg_ptr)/sizeof(msg_ptr[0]); --i>=0; )
      if( mb->u.msg==msg_ptr[i] ) msg_ptr[i] = 0L;
    cmfree( (void **)&mb->u.msg );
  }
}

void free_msgs( MACDESC *m )
{
  MACBUF *mb;
  int i;

  if( (mb=m->mb) != 0 ) for( i=m->len; --i>=0; )
    free_msg(mb++);
}

int free_macs( int root, int save_glob )
{
  int i;
  static OBJECT obj = { -1, -1, -1, G_BOX, 0, 0, 0 };

  mlist_edit = 0;
  if( mdesc )
  {
    for( i=save_glob ? iglobl : nmacs; --i>=0; )
    {
      free_msgs( &mdesc[i] );
      xmfree( mdesc[i].mb );
    }
    if( !save_glob || nmacs==iglobl )
    {
      cmfree( (void **)&mdesc );
      nmacs = num_macs = 0;
    }
    else
    {
      macs_rem += iglobl;
      memcpy( mdesc, mdesc+iglobl, (nmacs = num_macs = nmacs-iglobl)*sizeof(MACDESC) );
    }
    iglobl = 0;
  }
  cmfree((void **)&maclist);
  if( root )
  {
    last_mobj = 1;
    if( !add_obj( &maclist, &num_mobj, &mobj_rem, &obj ) ) return 0;
    *(long *)&maclist[0].ob_x = *(long *)&winner.g_x;
  }
  free_edits(0);
  return 1;
}

void edits_on( char *state, OBJECT *o, int hand, int on )
{
  if( *state != on )
  {
    if( !on ) wind_set( hand, X_WF_DIALEDIT, 0, 0 );
    do
      if( (char)(++o)->ob_type==G_FTEXT )
      {
        if( hand==main_hand )
        {
          if( o->ob_state&DISABLED ) break;	/* ignore globals */
          if( o[-1].ob_head < 0 ) continue;	/* only edit macro name */
        }
        if( on )
        {
          o->ob_flags |= EDITABLE;
          o->ob_flags &= ~TOUCHEXIT;
        }
        else
        {
          o->ob_flags &= ~EDITABLE;
          o->ob_flags |= TOUCHEXIT;
        }
      }
    while( o->ob_next >= 0 );
    *state = on;
  }
}

void mac_off( int num, int draw )
{
  unsigned int *i;

  if( *(i=&maclist[num].ob_state)&SELECTED )
  {
    *i &= ~SELECTED;
    if(draw) x_wdial_draw( main_hand, num, 8 );
  }
}

void macs_off( int draw )
{
  int i;

  for( i=0; i<nmacs; i++ )
    mac_off( mdesc[i].obj, draw );
  edits_on( &mlist_edit, maclist, main_hand, 0 );
  last_on = -1;
}

int mac_list( int mode )
{
  static int prev;

  if( !mode ) prev = 0;
  for( ; prev<nmacs; prev++ )
    if( maclist[mdesc[prev].obj].ob_state&SELECTED ) return prev++;
  return -1;
}

int add_tree( OBJECT **list, int *count, int *total, int *rem, OBJECT *add, int items, int h )
{
  int n, i, par;

  n = add->ob_tail - add->ob_head + 2;
  for( i=0; i<n; i++ )
    if( !add_obj( list, total, rem, add++ ) )
    {
      *rem += i;
      return 0;
    }
  (*list)[*count].ob_x = 0;
  (*list)[*count].ob_y = items*h;
  i = par = *count;
  *count += n;
  objc_add( *list, 0, par );
  *(long *)&(*list)[par].ob_head = -1L;
  while( ++i<*count )
    objc_add( *list, par, i );
  return 1;
}

int objc_add( OBJECT *tree, int parent, int child )
{
  int i;
  OBJECT *p = &tree[parent];

  i = p->ob_tail;
  p->ob_tail = child;
  if( p->ob_head < 0 ) p->ob_head = child;
  if( i >= 0 ) tree[i].ob_next = child;
  tree[child].ob_next = parent;
  return(1);
}

int set_mobj( MACDESC *m )
{
  int i;

  memcpy( maclist[m->obj+EVMACNAM-EMAC].ob_spec.tedinfo=&m->ted,
     events[EVMACNAM].ob_spec.tedinfo, sizeof(TEDINFO) );
  m->ted.te_ptext = m->name;
  memcpy( maclist[m->obj+EVAFROM-EMAC].ob_spec.tedinfo=&m->fromted,
     events[EVAFROM].ob_spec.tedinfo, sizeof(TEDINFO) );
  m->fromted.te_ptext = m->afrom;
  memcpy( maclist[m->obj+EVADATE-EMAC].ob_spec.tedinfo=&m->dateted,
     events[EVADATE].ob_spec.tedinfo, sizeof(TEDINFO) );
  m->dateted.te_ptext = m->adate;
  maclist[i=m->obj+EVMACKEY-EMAC].ob_spec.free_string = m->kstr;
  return i;
}

void main_dial(void)
{
  if( main_hand>0 ) wind_set( main_hand, X_WF_DIALOG, maclist );
}

void reset_maclist(void)
{
  int i, num;
  MACDESC *m;

  for( num=0, m=mdesc, i=1; num<nmacs; num++, m++ )
  {
    maclist[m->obj=i].ob_y = num*main_h;
    set_mobj( m );
    i = maclist[i].ob_next;
  }
}

void macpop_str( MACDESC *m )
{
  maclist[m->obj+EVAPOP-EMAC].ob_spec.free_string = mac_pop[m->automode+1].ob_spec.free_string+2;
}

void setup_time( MACDESC *m, int draw )
{
  _strftime( m->afrom, "%H%M", 4, 0, m->time );
  if( draw && main_hand>0 ) x_wdial_draw( main_hand, m->obj+EVAFROM-EMAC, 0 );
}

void setup_date( MACDESC *m, int draw )
{
  _strftime( m->adate, edate_fmt, 6, m->date, 0 );
  if( draw && main_hand>0 ) x_wdial_draw( main_hand, m->obj+EVADATE-EMAC, 0 );
}

void set_global( MACDESC *m )
{
  int i;
  unsigned int *f;
  OBJECT *o, *o0;

  if( !aes_ok ) return;  
  for( o=o0=&maclist[m->obj+EVMACNAM-EMAC], i=EVADATE-EVMACNAM+1; --i>=0; o++ )
  {
    f = &o->ob_flags;
    if( m->is_global )
/***      if( o->ob_state&DISABLED ) return;
      else ***/
      {
        if( (char)o->ob_type==G_FTEXT ) *f &= ~(1<<10);	/* avoid bug in G 004 */
        if( *f & TOUCHEXIT ) *f = (*f | (1<<11)) & ~TOUCHEXIT;
        o->ob_state |= DISABLED;
      }
    else
    {
      if( (char)o->ob_type==G_FTEXT ) *f |= (1<<10);
      if( *f & (1<<11) ) *f |= TOUCHEXIT;
      o->ob_state &= ~DISABLED;
    }
  }
  f = &o0[EVAON-EVMACNAM].ob_flags;
  if( m->is_global ) *f &= ~SELECTABLE;
  else *f |= SELECTABLE;
  o0[EVAON-EVMACNAM].ob_state &= ~DISABLED;
}

void setup_evaon( MACDESC *m, int draw )
{
  int i;

  set_if( maclist, i=m->obj+EVAON-EMAC, m->auto_on );
  if( draw ) x_wdial_draw( main_hand, i, 0 );
}

void setup_mac( MACDESC *m )
{
  set_mobj(m);
  key_str( maclist, m->obj+EVMACKEY-EMAC, &m->key, 0 );
  setup_evaon( m, 0 );
  macpop_str(m);
  setup_date( m, 0 );
  setup_time( m, 0 );
  set_global(m);
}

int add_1mac( MACDESC *m, int n )
{
  m->obj = last_mobj;
  if( !add_tree( &maclist, &last_mobj, &num_mobj, &mobj_rem, &events[EMAC], n, main_h ) )
  {
    num_macs--;
    macs_rem++;
    return 0;
  }
  setup_mac(m);
  return 1;
}

void build_mlist(void)
{
  int i;

  if( num_mobj==1 && nmacs>0 )
  {
    for( i=0; i<nmacs; i++ )
      add_1mac( &mdesc[i], i );
    reset_maclist();
  }
}

int add_mlist(void)
{
  MACDESC *m;
  int i, j;

  if( !add_macro(0L) ) return 0;
  memset( (m=&mdesc[nmacs]), 0, sizeof(MACDESC) );
  *(long *)&m->date = current_time();
  if( !aes_ok )
  {
    nmacs++;
    return 1;
  }
  if( !add_1mac( m, nmacs ) ) return 0;
  iglobl++;
  nmacs++;
  reset_maclist();
  set_bl_height();
  return 1;
}

int create_mac(void)
{
  int i, j;
  MACDESC *m;

  if( !shift_maclist(iglobl,1) ) return 0;
  m = &mdesc[iglobl-1];
  m->name[0] = m->is_global = 0;
  *(long *)&m->key = 0L;
  m->len = m->auto_on = m->automode = 0;
  m->mb = 0L;
  *(long *)&m->date = current_time();
  i = m->obj;
  if( (j=maclist[i].ob_y+maclist[0].ob_y) < winner.g_y ) maclist[0].ob_y =
      winner.g_y - maclist[i].ob_y;
  else if( j >= winner.g_y+winner.g_h ) maclist[0].ob_y =
      winner.g_y + winner.g_h - main_h - maclist[i].ob_y;
  setup_mac(m);
  macs_off(0);
  main_dial();
  return 1;
}


void new_mac( EDITDESC *e, int pos )
{
  if( !e ) create_mac();
  start_form( 1, MESSAGE, NAME|MOVER, 0 );
  if( form[1].handle>0 )
  {
    mac_err = 0;
    rec_desc = e;
    rec_pos = pos;
    rec_mac = e ? e->m : &mdesc[iglobl-1];
    check_rec();
  }
}

void some_events( MACBUF *m, int total )
{
  MACBUF *m2;

  for( m2=mac_buf; --total>=0; m2++ )
    if( etypes&(1<<m2->type) ) *m++ = *m2;
}

void end_record(void)
{
  MACBUF *m, *m2;
  int c, e;

  if( !rec_desc )
  {
    if( !rec_mac ) return;	/* user deleted macro before record ended */
    if( rec_mac->mb ) xmfree( rec_mac->mb );
    rec_mac->len = 0;
    rec_mac->mb = 0L;
  }
  if( !mac_err )
  {
    Dosound( sound_dwn );
    c = get_mac_end(&e);
    if(c)
      if( !rec_desc )
      {
        rec_mac->mb = xmalloc( c*sizeof(MACBUF) );
        if( rec_mac->mb )
        {
          rec_mac->len = c;
          some_events( rec_mac->mb, e );
        }
      }
  }
  rec_mac = 0L;
}

MACBUF *mac_str( MACBUF *o, char *s, int *len )
{
  int k;

  while(*s)
  {
    if( *len >= macsize ) return o;
    ++*len;
    k = *s++;
    o->type = TYPE_KEY;
    o->u.key.shift = keypress( &k );
    o->u.key.scan = *(char *)&k;
    o->u.key.ascii = k;
    o++;
  }
  return o;
}

int play_mac( MACDESC *m, int aes )
{
  MACBUF *o, *b;
  int len = m->len, i, msg_num=0;
  char temp[80];
  
  b = m->mb;
  o = mac_buf;
  for( i=0; i<m->len; i++ ) switch( b->type )
  {
    case TYPE_MSG:
      if( msg_num<(sizeof(msg_ptr)/sizeof(msg_ptr[0])) )
      {
        o->u.timer = 0x3700L | (msg_num+0x20);
        msg_ptr[msg_num++] = b->u.msg;
        o->type = TYPE_KEY;
        o++;
      }
      b++;
      break;
    case TYPE_SOUND:
      o->u.timer = 0x3700L | b->u.sound.index;
      o->type = TYPE_KEY;
      o++;
      b++;
      break;
    case TYPE_DATE:
      len--;
      _strftime( temp, date_fmt[b->u.sound.index], sizeof(temp)-1, Tgetdate(), 0 );
      o = mac_str( o, temp, &len );
      b++;
      break;
    case TYPE_TIME:
      len--;
      _strftime( temp, time_fmt[b->u.sound.index], sizeof(temp)-1, 0, Tgettime() );
      o = mac_str( o, temp, &len );
      b++;
      break;
    default:
      *o++ = *b++;
  }
  if( !aes )
  {
    if( !(*(int (*)( void *mem, int num, int scale, int mode ))
        (cookie->xaes_funcs[4]))( mac_buf, len, 100, 1 ) )
    {
      Bconout(2,7);
      return 0;
    }
  }
  else if( !x_appl_tplay( mac_buf, len, 100, 1 ) ) return 0;
  return 1;
}

int test_key( unsigned char *key )
{  /* return  0: no change,  1: took key, no event  2: took key, event */
  int i;
  MACDESC *m;
  char sh;

  if( *(key+1) == (unsigned char)0x37 )
  {
    switch( *((int *)key+1) )
    {
      case 0:
        Bconout(2,7);
        break;
      case 1:
        Dosound(sound_up);
        break;
      case 2:
        Dosound(sound_dwn);
        break;
      default:
        next_msg = *(key+3)-0x20;
        *(long *)key = 0L;
        return 2;
    }
    *(long *)key = 0L;
    return 1;
  }
  if( (sh = (*key&0xf))&3 ) sh |= 3;
  if( (rec_mac&&!mac_err) && start_end.shift==sh && start_end.scan==*(key+1) )
  {
    if( mac_end<0 )
    {
      if( set_ap(apid) )
      {
        *(long *)key = 0L;
        mac_end = 0;
        i = 1;
        if( !(*(int (*)( void *mem, int num, KEYCODE *key, int mode ))(cookie->xaes_funcs[5]))
            ( mac_buf, macsize, &start_end, 1 ) )
        {
          Bconout(2,7);
          mac_err++;
          rec_mac = 0L;
          i = 2;
        }
        else Dosound( sound_up );
        set_ap(-1);
        return i;
      }
    }
    else
    {
      if( set_ap(apid) )
      {
        (*(int (*)( void *mem, int num, KEYCODE *key, int mode ))(cookie->xaes_funcs[5]))
            ( 0L, 0, 0L, 1 );
        set_ap(-1);
      }
      *(long *)key = 0L;
      aes_ok = 0;
      end_record();
      aes_ok = 1;
      return 2;
    }
    return 0;
  }
  for( i=nmacs, m=mdesc; --i>=0; m++ )
    if( m->key.shift==sh && m->key.scan==*(key+1) )
    {
      if( in_read.scan!=(unsigned char)0xff && m->key.shift==in_read.shift &&
          m->key.scan==in_read.scan ) return 0;
      if( set_ap(apid) )
      {
        *(long *)key = 0L;
        if( in_read.scan!=(unsigned char)0xff ) Bconout(2,7);
        else play_mac( m, 0 );
        set_ap(-1);
        return 1;
      }
      return 0;
    }
  return 0;
}

void mov_obj( int *i, int dif )
{
  if( *i>0 ) *i+=dif;
}

int shift_maclist( int num, int inc )
{
  int i, dif;
  OBJECT *o;
  MACDESC *m;
  EDITDESC *e;

  if( inc>0 )
  {
    i = last_mobj;
    if( !add_mlist() ) return 0;
    if( num>=nmacs-1 ) return 1;
    nmacs--;
    last_mobj = i;
  }
  else iglobl += inc;
  m = &mdesc[num];
  for( e=elist; e; e=e->next )
    if( e->m >= m ) e->m += inc;
  if( num<nmacs-1 ) dif = m[1].obj - m->obj;
  else dif = last_mobj - m->obj;
  dif *= inc;	/* set sign correctly */
  last_mobj += dif;
  nmacs += inc;
  if( inc>0 )
  {
    num += inc;
    m += inc;
  }
  else
  {
    free_msgs(m);
    cmfree((void **)&m->mb);
    num_macs += inc;
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
}

void get_etypes(void)
{
  int i;

  etypes &= ~0xf;
  for( i=0; i<4; i++ )
    if( menu[MTIMER+i].ob_state & CHECKED ) etypes |= 1<<i;
}

int get_mac_end( int *end )
{
  int i, c;
  MACBUF *m;

  get_etypes();
  for( m=&mac_buf[i=mac_end], c=mac_count; i<macsize; i++, m++ )
  {
    if( m->type==-1 ) break;
    if( etypes&(1<<m->type) ) c++;
  }
  if(end) *end = i;
  return c;
}

int find_mac( int but )
{
  int i;

  for( i=0; i<nmacs; i++ )
    if( mdesc[i].obj==but ) return i;
  return -1;
}

int edch( char c )
{
  if( !c || c==' ' ) return -1;
  return c-'0';
}

void temp_edit( MACDESC *m, int but, int fr )
{
  static OBJECT edit[2] = { { -1, 1, 1, G_IBOX, DEFAULT|EXIT|SELECTABLE, 0 },
      { 0, -1, -1, 0, EDITABLE|(1<<10) } };
  OBJECT *o;
  int vals[3], i, j, x;
  char *ptr, err;

  if( !m ) return;
  edits_on( &mlist_edit, maclist, main_hand, 0 );
  o = &maclist[but];
  wind_set( main_hand, WF_NAME, *evedit );
  edit[1].ob_type = o->ob_type;
  edit[1].ob_state = o->ob_state;
  edit[1].ob_spec.tedinfo = fr ? &m->fromted : &m->dateted;
  objc_offset( maclist, but, &edit[0].ob_x, &edit[0].ob_y );
  *(long *)&edit[0].ob_width = *(long *)&edit[1].ob_width = *(long *)&o->ob_width;
  form_do( edit, 1 );
  main_title();
  set_if( maclist, but, 0 );
  for( i=0, ptr=edit[1].ob_spec.tedinfo->te_ptext; i<(fr?2:3); i++ )
  {
    vals[i] = (j = edch(*ptr)) >= 0 ? j : 0;
    if( *ptr ) ptr++;
    if( (j = edch(*ptr)) >= 0 ) vals[i] = vals[i] * 10 + j;
    if( *ptr ) ptr++;
  }
  if( fr )
  {
    if( vals[0]>23 || vals[1]>59 ) alert( BADTIME );
    else m->time = (vals[0]<<11) | (vals[1]<<5);
    setup_time(m,1);
  }
  else
  {
    for( x=0, err=0, i=0, ptr=edate_fmt; i<3 && *ptr; )
      switch(*ptr++)
      {
        case 'q':		/* month */
          if( (j=vals[i++]) > 12 || !j ) err++;
          x |= j<<5;
          break;
        case 'r':		/* day */
          if( (j=vals[i++]) > 31 || !j ) err++;
          x |= j;
          break;
        case 'y':		/* year */
          x |= ((j=vals[i++]-80)<0 ? j+100 : j)<<9;
      }
    if(!err) m->date = x;
    else alert( BADDATE );
    setup_date(m,1);
  }
}

void advance_time( MACDESC *m )
{
  unsigned long now;
  unsigned int date[2];
  int h, mn, mo, d, y, dpm;
  static char im[] = { 2, 5, 10, 15, 30 },
      dm[] = { 31, 0, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
  
  now = current_time();
  mo = (m->date>>5)&0xf;
  d = m->date&0x1f;
  y = (m->date>>9)+80;
  h = m->time>>11;
  mn = (m->time>>5)&0x3f;
  *(long *)date = *(long *)&m->date;
  while( *(unsigned long *)date <= now )
  {
    dpm = mo==2 ? 28+(y!=100 && !(y&3)) : dm[mo-1];
    switch( m->automode )
    {
      default:
        if( (mn += im[m->automode]) < 60 ) break;
        mn -= 60;
      case 5:
        if( ++h < 24 ) break;
        h = 0;
      case 6:
        if( ++d <= dpm ) break;
        goto fix_d;
      case 7:
        if( (d+=7) <= dpm ) break;
fix_d:  d -= dpm-1;
      case 8:
        if( ++mo < 13 ) break;
        mo = 1;
      case 9:
        y++;
    }
    date[0] = (mo<<5)|d|((y-80)<<9);
    date[1] = (h<<11)|(mn<<5);
  }
  if( date[1] != m->time )
  {
    m->time = date[1];
    setup_time(m,1);
  }
  if( date[0] != m->date )
  {
    m->date = date[0];
    setup_date(m,1);
  }
}

void auto_play(void)
{
  MACDESC *m = mdesc;
  int i, chk, chk2;
  unsigned long time = current_time();
  
  for( i=nmacs; --i>=0; m++ )
    if( m->auto_on && time >= *(unsigned long *)&m->date )
    {
      if( !m->automode )
      {
        chk = chksum();
        m->auto_on=0;
        chk2 = chksum();
        /* If I made the only change here, the file doesn't need to be saved */
        if( chk==last_chk ) last_chk = chk2;
        else if( last_chk==chk2 ) last_chk = chk2+99;	/* in case it was changed and we made it unchanged */
        if( maclist ) set_if( maclist, m->obj+EVAON-EMAC, 0 );
        if( main_hand>0 ) x_wdial_draw( main_hand, m->obj+EVAON-EMAC, 0 );
      }
      else advance_time(m);
      play_mac( m, 1 );
    }
}

void main_sel( int dclick, int but, int keys )
{
  int i, dum;
  MACDESC *m;

  if( maclist[but].ob_head<0 )  /* not panel */
  {
    i = ((but-EMAC)%(ETIMER-EMAC)) + EMAC;
    if( (dum = find_mac(but-i+1)) >= 0 ) m = &mdesc[dum];
    else m = 0L;
    if( (char)maclist[but].ob_type==G_BUTTON )
    {
      if(m)
        if( i==EVMACKEY ) read_key( maclist, but, &m->key, main_hand, maintitle, 0 );
        else if( i==EVAON )
        {
          if( (m->auto_on = maclist[but].ob_state&SELECTED) != 0 ) advance_time(m);
        }
        else if( i==EVAPOP )
           if( (i = do_popup( maclist, but, mac_pop, m->automode )) != m->automode )
           {
             m->automode = i;
             macpop_str(m);
             x_wdial_draw( main_hand, but, 0 );
           }
    }
    else if( i==EVAS1 )
    {
      if( (m->auto_on ^= 1) != 0 ) advance_time(m);
      setup_evaon( m, 1 );
      but_up();
    }
    else if( i==EVAFROM ) temp_edit( m, but, 1 );
    else if( i==EVADATE ) temp_edit( m, but, 0 );
    else if( !mlist_edit )
    {
      edits_on( &mlist_edit, maclist, main_hand, 1 );
      wind_set( main_hand, X_WF_DIALEDIT, but, -1 );
    }
  }
  else if( dclick )         /* double-click */
  {
    if( but )
    {
      i = mac_list(0);
      if( i>=0 )
        do edit_mac(i);
        while( (i = mac_list(1)) >= 0 );
      else if( (i = find_mac(but)) >= 0 ) edit_mac(i);
    }
    macs_off(1);
  }
  else
  {
    if( keys&4 && last_on>=0 )
    {
      if( but<last_on )
      {
        i = but;
        dum = last_on-1;
      }
      else
      {
        i = last_on+1;
        dum = but;
      }
      for( ; i<=dum; i++ )
        if( maclist[i].ob_head >= 0 )
        {
          set_if( maclist, i, 1 );
          x_wdial_draw( main_hand, i, 8 );
        }
      last_on = but;
    }
    else if( !(maclist[but].ob_state&SELECTED) )
    {
      if( !(keys&3) ) macs_off(1);
      set_if( maclist, but, 1 );
      x_wdial_draw( main_hand, but, 8 );
      last_on = but;
    }
    else if( !(keys&3) ) macs_off(1);
    else
    {
      mac_off( but, 1 );
      last_on = -1;
    }
    but_up();
  }
}
