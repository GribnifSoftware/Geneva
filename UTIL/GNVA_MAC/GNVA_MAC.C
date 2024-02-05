#include "new_aes.h"
#include "xwind.h"              /* Geneva extensions */
#include "tos.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "gnva_mac.h"            /* resource file constants */
#include "lerrno.h"
#define EXTERN
#include "common.h"
#include "multevnt.h"

/* definitions for seeing if Geneva is present, by way of JARxxx */
#define IDT_cookie      0x5F494454L     /* "_IDT" */
#define CJar_cookie     0x434A6172L     /* "CJar" */
#define CJar_xbios      0x434A          /* "CJ" */
#define CJar_OK         0x6172          /* "ar" */
#define CJar(mode,cookie,value)         (int)xbios(CJar_xbios,mode,cookie,value)

FORM form[] =
	      { { 0, 0L, mo_init, mo_touch, mo_exit },  /* Misc options */
		{ 0, 0L, ms_init, ms_touch, 0L },	/* Recording msg */
		{ 0, 0L, a_init, a_touch, a_exit },	/* About */
		{ 0, 0L, at_init, 0L, at_exit },	/* All timers */
		{ 0, 0L, d_init, d_touch, d_exit },	/* Defaults */
		{ 0, 0L, msg_init, 0L, msg_exit },	/* Message */
                { 0, 0L, 0L } };                        /* end of list */

int my_keyvec( long *key );
extern int (*old_keyvec)( long *key );
int my_appvec( char *process_name, int apid );
extern int (*old_appvec)( char *process_name, int apid );
int my_genvec(void);
extern int (*old_genvec)(void);

char have_vex;
int old_val, *patch_base;

void install_vex( G_VECTORS *vex )
{
  int *i;
  long **start, stack;
  extern long norecord;
  extern char *kbshift;
  extern int ROM_ver;
  void install_key( int remove );
  SYSHDR *sys;

  old_keyvec = vex->keypress;
  vex->keypress = my_keyvec;
  old_appvec = vex->app_switch;
  vex->app_switch = my_appvec;
  old_genvec = vex->gen_event;
  vex->gen_event = my_genvec;
  have_vex = 1;
  if( cookie->ver == 0x104 )
  {
    stack = Super(0L);
    start = (long **)&vex->keypress;
    while( *(*start-3) == 0x58425241L/*"XBRA"*/ )
      start = (long **)(*start-1);
    i = ((int *)*start) + 0x28/2;
    if( *(long *)i == 0x3f004eb9 )
    {
      patch_base = i;
      norecord = *((long *)(i+0x116/2));
      sys = *(SYSHDR **)0x4f2;
      kbshift = (ROM_ver=sys->os_version)>0x0100 ?
          (char *)sys->kbshift : (char *)0xE1BL;
      old_val = *(i+2);
      *i = 0x4ef9;	/* jmp */
      *(long *)(i+1) = (long)install_key;
    }
    Super((void *)stack);
  }
}

void remove_one( void *value, long **start )
{
  /* search along the XBRA chain */
  while( *(*start-3) == 0x58425241L/*"XBRA"*/ )
  {
    if( (long)*start == (long)value )
    {
      *(long *)start = *(*start-1);	/* found it! remove from list */
      return;
    }
    start = (long **)(*start-1);	/* continue along chain */
  }
}

void remove_vex( G_VECTORS *vex )
{
  remove_one( my_keyvec, (long **)&vex->keypress );
  remove_one( my_appvec, (long **)&vex->app_switch );
  remove_one( my_genvec, (long **)&vex->gen_event );
  if( patch_base )
  {
    *(long *)patch_base = 0x3f004eb9;	/* old values */
    *(patch_base+2) = old_val;
  }
}

void quit(void)
{
  int i;

  if( mac_buf ) xmfree( mac_buf );
  free_macs( 0, 0 );
  if( have_vex ) remove_vex( cookie->vectors );
  for( i=0; form[i].init; i++ )
    close_wind( &form[i].handle );
  rsrc_free();
  appl_exit();
  exit(0);
}

void iconify1( int handle, EDITDESC *e, int buf[] )
{
  OBJECT *icon;

  rsrc_gaddr( 0, e ? EDITICON : MACICON, &icon );
  if(e)
  {
    if( (e->icon = xmalloc(2*sizeof(OBJECT))) == 0 ) return;
    memcpy( e->icon, icon, 2*sizeof(OBJECT) );
    icon = e->icon;
  }
  if( !e ) iconified = 1;
  else e->iconified = 1;
  /* turn dialog off so that maclist's size and position will not change */
  wind_set( handle, X_WF_DIALOG, 0L );
  wind_set( handle, WF_ICONIFY, buf[4], buf[5], buf[6], buf[7] );
  if( (buf[4] += buf[6]) + buf[6] >= max.g_w )
  {
    buf[4] = 0;
    if( (buf[5] -= buf[7]) < max.g_y ) buf[5] = max.g_y;
  }
  /* get working area of iconified window */
  wind_get( handle, WF_WORKXYWH, &icon[0].ob_x, &icon[0].ob_y,
      &icon[0].ob_width, &icon[0].ob_height );
  /* center the icon within the form */
  icon[1].ob_x = (icon[0].ob_width - icon[1].ob_width) >> 1;
  icon[1].ob_y = (icon[0].ob_height - icon[1].ob_height) >> 1;
  /* new (buttonless) dialog in main window */
  wind_set( handle, X_WF_DIALOG, icon );
}

void do_iconify( int buf[] )
{
  int top, dum, wind, i, place, handle;
  EDITDESC *e;

  e = find_edesc(handle = buf[3]);
  if( !e || buf[0]==WM_ALLICONIFY )
  {
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
  }
  wind_update( END_UPDATE );
  iconify1( handle, e, buf );
  if( buf[0]==WM_ALLICONIFY )
  {
    if(e) iconify1( main_hand, 0L, buf );
    for( e=elist; e; e=e->next )
      iconify1( e->hand, e, buf );
  }
}

void do_uniconify( int buf[] )
{
  int i, place, count, handle;
  EDITDESC *e;

  e = find_edesc(handle = buf[3]);
  if( !e ) iconified = 0;
  else e->iconified = 0;
  /* turn dialog off so that icon's size and position will not change */
  wind_set( handle, X_WF_DIALOG, 0L );
  wind_set( handle, WF_UNICONIFY, buf[4], buf[5], buf[6], buf[7] );
  /* restore old dialog */
  wind_set( handle, X_WF_DIALOG, e ? e->o : maclist );
  if( !e )
  {
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
}

/* wait for the button to be released */
void but_up(void)
{
  int b, dum;

  do
    graf_mkstate( &dum, &dum, &b, &dum );
  while( b&1 );
}

void currxy( int hand, GRECT *new )
{
  wind_set( hand, WF_CURRXYWH, new->g_x, new->g_y, new->g_w, new->g_h );
}

void resize_wind( int hand, GRECT *new )
{
  GRECT old;
  FORM *f;
  EDITDESC *e;

  if( hand==main_hand && !iconified )          /* main window? */
  {
/*    old = wsize;*/
    wsize = *new;
    void get_inner();
    if( winner.g_h > maclist[0].ob_height ) winner.g_h =
        maclist[0].ob_height;
    void get_outer();
    void get_inner();
    currxy( hand, &wsize );
/*    if( !iconified && hand==main_hand && wsize.g_w <= old.g_w && wsize.g_h
        <= old.g_h && (wsize.g_w != old.g_w || wsize.g_h != old.g_h) )
        wind_set( hand, X_WF_DIALOG, maclist ); */
    /* force a redraw of main window if it wouldn't otherwise happen */
  }
  else if( (e = find_edesc(hand)) != 0 && !e->iconified )
  {
    e->outer = *new;
    gget_inner( &e->outer, &e->inner, e->o );
    if( e->inner.g_h > e->o[0].ob_height ) e->inner.g_h =
        e->o[0].ob_height;
    gget_outer( &e->outer, &e->inner, panel_h );
    gget_inner( &e->outer, &e->inner, e->o );
    currxy( hand, &e->outer );
  }
  else
  {
    for( f=form; f->init; f++ )
      if( f->handle==hand )
      {
        f->wind = *new;
        break;
      }
    currxy( hand, new );
  }
}

long count_msgs( MACDESC *m )
{
  MACBUF *mb;
  int i;
  long b=0L;

  for( mb=m->mb, i=m->len; --i>=0; mb++ )
    if( mb->type==TYPE_MSG ) b+=5*31;
  return b;
}

void do_info(void)
{
  int i, count, l;
  long bytes, events;
  char temp[sizeof(main_info)];

  if( main_hand<=0 ) return;
  count = 0;
  bytes = events = 0L;
  i = mac_list(0);
  if( i>=0 )
  {
    do
    {
      count++;
      events += (l=mdesc[i].len);
      bytes += l*sizeof(MACBUF) + count_msgs(&mdesc[i]);
    }
    while( (i = mac_list(1)) >= 0 );
    x_sprintf( temp, *minfo, count, *selected, events, bytes );
  }
  else if( !nmacs ) temp[0] = 0;
  else
  {
    for( i=nmacs; --i>=0; )
    {
      count++;
      events += (l=mdesc[i].len);
      bytes += l*sizeof(MACBUF) + count_msgs(&mdesc[i]);
    }
    x_sprintf( temp, *minfo, count, "", events, bytes );
  }
  if( strcmp( temp, main_info ) )
  {
    strcpy( main_info, temp );
    wind_set( main_hand, WF_INFO, main_info );
  }
}

int need_timer(void)
{
  MACDESC *m = mdesc;
  int i;
  
  for( i=nmacs; --i>=0; m++ )
    if( m->auto_on ) return 1;
  return 0;
}

extern char do_auto;

int auto_load(void)
{
  char temp[120], *p;
  int ret=-1;

  do_auto = 0;
  strcpy( temp, gma_path );
  strcpy( spathend(temp), auto_last );
  if( (p = strchr(temp,' ')) != 0 ) strcpy( p, ".GMA" );
  else strcat( temp, ".GMA" );
  aes_ok = 0;
  if( !load_gma( temp, 0 ) && strcmp( spathend(main_path),"DEFAULT.GMA" ) )
  {
    strcpy( spathend(temp), "DEFAULT.GMA" );
    load_gma( temp, 0 );
  }
  if( need_timer() ) ret = apid;
  aes_ok = 1;
  return ret;
}

void auto_test( char *name )
{
  if( gma_auto && main_hand<=0 && !elist && !rec_mac &&
      strcmp( auto_last, name ) && strcmp( name, "GNVA_MAC" ) )
  {
    strcpy( auto_last, name );
    do_auto = 1;
  }
}

void main_title(void)
{
  char *p, *p2;

  strcpy( maintitle, *title+2 );
  if( (p = spathend(main_path)) != 0 )
    if( (p2 = strchr(p,'.')) != 0 )
    {
      strcat( maintitle, ": " );
      strncat( maintitle, p, p2-p );
    }
  if( main_hand>0 ) wind_set( main_hand, WF_NAME, maintitle );
}

void center_panels(void)
{
  static char obs[] = { EVTIMER, EVLEFT, EVRIGHT,
      EVMX, EVMY, EVMREAD, EVKKEY, EVDPOP, EVTPOP, EVBELL, EVUP, EVDOWN,
      EVMSAM, EVMEDIT };
  char *p;
  int y, i;

  y = (panel_h - char_h) >> 1;
  for( i=sizeof(obs), p=obs; --i>=0; p++ )
    events[*p].ob_y = y;
  y = (main_h + events[EVMACNAM].ob_y + events[EVMACNAM].ob_height - events[EVAS1].ob_height)>>1;
  events[EVAON].ob_y = events[EVAS1].ob_y = events[EVAFROM].ob_y =
      events[EVADATE].ob_y = y;
}

void check_rec(void)
{
  int i;

  menu_ienable( menu, MTIMER, i=rec_mac==0 );
  menu_ienable( menu, MMOUSE, i );
  menu_ienable( menu, MBUTTON, i );
  menu_ienable( menu, MKEYBD, i );
  menu_ienable( menu, NEWMAC, i );
  menu_ienable( emenu, ERECORD, i );
}

int cmdline_play( int argc, char *argv[] )
{
  char *p;
  int i, buf[8];
  MACDESC *m;

  for( i=2; i<argc; i++ )
    for( p=argv[i-1], p+=strlen(p); p<argv[i]; )
      *p++ = ' ';
  if( argc>1 && (m=mdesc) != 0 )
    for( i=0; i<num_macs; i++, m++ )
      if( !strcmpi( m->name, argv[1] ) )
        if( play_mac( m, -1 ) ) return 1;
        else
          for(;;)
          {
            evnt_mesag(buf);
            if( buf[0] == AP_TERM || buf[0] == (int)X_WM_RECSTOP ) return 0;
          }
  return 0;
}

char *dflt_date( int i )
{
  static char yfmts[4][11] = { "%m%/%d%/%y", "%d%/%m%/%y", "%y%/%m%/%d", "%y%/%d%/%m" };
  int d;
  
  d = ((unsigned int)idt>>8)&0xf;
  if( d > 3 ) d = 2;
  if(i) return yfmts[d];
  return d&1 ? "%d%/%m" : "%m%/%d";
}

void top_app(void)
{
  int id, first=0, type, num;
  char name[10];

  if( !gma_auto ) return;
  id = menu_bar( 0L, -1 );
  while( appl_search( first, name, &type, &num ) )
    if( num==id )
    {
      auto_test( name );
      return;
    }
    else first=1;
}

EMULTI emulti = { MU_MESAG|X_MU_DIALOG, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  500, 0 };

int main( int argc, char *argv[] )
{
  char *ptr, **ptr3,
       temp[120],
       fulled=0,
       was_open=0;              /* was window already opened? */
  int buf[8],                   /* message buffer */
      i,
      but,
      dum;
  EDITDESC *ed;
  GRECT full;
  OBJECT *o;

  apid = appl_init();
  /* Tell the AES (or Geneva) that we understand AP_TERM message */
  if( _GemParBlk.global[0] >= 0x400 ) shel_write( SHW_MSGTYPE, 1, 0, 0L, 0L );
  if( rsrc_load("gnva_mac.rsc") )
  {
    /* Check to make sure Geneva rel 004 or newer is active */
    if( CJar( 0, GENEVA_COOKIE, &cookie ) == CJar_OK && cookie &&
        cookie->ver >= 0x104 )
    {
      (void)CJar( 0, IDT_cookie, &idt );
      install_vex( cookie->vectors );
      /* default item selector path */
      gma_path[0] = Dgetdrv()+'A';
      gma_path[1] = ':';
      Dgetpath( gma_path+2, 0 );
      strcat( gma_path, "\\" );
      rsrc_gaddr( 15, SMONTHS, &smonths );
      rsrc_gaddr( 15, MONTHS, &ptr3 );
      months[0] = ptr = *ptr3;
      for( i=1; i<12; i++ )
      {
        ptr = strchr(ptr,'|');
        *ptr++ = 0;
        months[i] = ptr;
      }
      /* get main window menu tree and initialize root for keybd equivs */
      rsrc_gaddr( 0, MMENU, &menu );
      menu[0].ob_state |= X_MAGIC;
      rsrc_gaddr( 0, EMENU, &emenu );
      emenu[0].ob_state |= X_MAGIC;
      rsrc_gaddr( 0, EVENTS, &events );
      rsrc_gaddr( 0, DATEPOP, &date_pop );
      rsrc_gaddr( 0, TIMEPOP, &time_pop );
      rsrc_gaddr( 0, AUTOPOP, &mac_pop );
      rsrc_gaddr( 0, DEFAULTS, &o );
      for( i=0; i<8; i++ )
      {
        strcpy( date_fmt[i] = o[DFDATE0+i].ob_spec.tedinfo->te_ptext,
            i<2 ? dflt_date(i) : date_pop[i+1].ob_spec.free_string );
        strcpy( time_fmt[i] = o[DFTIME0+i].ob_spec.tedinfo->te_ptext,
            time_pop[i+1].ob_spec.free_string );
      }
      updt_datepop();
      updt_timepop();
      ptr = dflt_date(1);
      strncpy( edate_fmt, ptr, 2 );
      strncpy( edate_fmt+2, ptr+4, 2 );
      strncpy( edate_fmt+4, ptr+8, 2 );
      if( (i = (char)idt) == 0 ) i = '/';
      events[EVADATE].ob_spec.tedinfo->te_ptmplt[2] = 
        events[EVADATE].ob_spec.tedinfo->te_ptmplt[5] = i;
      if( (ptr = strchr(edate_fmt,'m')) != 0 ) *ptr = 'q';
      if( (ptr = strchr(edate_fmt,'d')) != 0 ) *ptr = 'r';
      /* get other strings used */
      rsrc_gaddr( 15, TITLE, &title );
      rsrc_gaddr( 15, MINFO, &minfo );
      rsrc_gaddr( 15, EINFO, &einfo );
      rsrc_gaddr( 15, MSELECT, &selected );
      rsrc_gaddr( 15, EVEDIT, &evedit );
      main_h = events[1].ob_height;
      panel_h = events[ETIMER].ob_height;
      char_w = events[EVMACNAM].ob_width/20;
      char_h = events[EVMACNAM].ob_height;
      center_panels();
      if( !free_macs( 1, 0 ) )
      {
        form_error(8);
        quit();
      }
      get_inner();
      wind_get( 0, WF_WORKXYWH, &max.g_x, &max.g_y, &max.g_w, &max.g_h );
      load_settings();
      if( (mac_buf = xmalloc((long)macsize*sizeof(MACBUF))) == 0L )
      {
        form_error(8);
        quit();
      }
      /* set the correct name in the menu */
      menu_register( apid, *title );
      strcpy( main_path, gma_path );	/* gma_path may have been set in load_settings */
      strcpy( spathend(main_path), "GLOBAL.GMA" );
      load_gma( main_path, 0 );
      i = 0;
      if( argc>1 )
      {
        strcpy( main_path, argv[1] );
        if( shel_find(main_path) )
          for( i=1, --argc; i<argc; i++ )
            argv[i] = argv[i+1];
        i = 1;
      }
      if(!i)
      {
        strcpy( main_path, gma_path );	/* gma_path may have been set in load_settings */
        strcpy( spathend(main_path), "DEFAULT.GMA" );
      }
      i = load_gma( main_path, 0 );
      if( _app )          /* running as PRG */
      {
        arrow();
        if( i && cmdline_play( argc, argv ) )
           if( !(Kbshift(-1)&4) ) quit();
        goto open;
      }
      for(;;)
      {
        auto_play();
        if( form[1].handle>0 || need_timer() ) emulti.type |= MU_TIMER;
        else emulti.type &= ~MU_TIMER;
        multi_evnt( &emulti, buf );
        if( next_msg>=0 )
          if( msg_ptr[next_msg] )
          {
            msg_edit = 0L;
            msg_mac = msg_ptr[next_msg];
            msg_ind = 0;
            next_msg = -1;
            start_form( 5, NEWMSG, NAME|MOVER|CLOSER, 0 );
          }
          else next_msg = -1;
        if( emulti.event&MU_MESAG ) switch( buf[0] )
        {
          case AC_OPEN:
open:       if( main_hand<=0 )
            {
              if( !was_open )
              {
                winner.g_w = events[1].ob_width;
                dum = wsize.g_h;
                /* list must be at least 2 long */
                winner.g_h = 2*main_h;
                get_outer();
                min_wid = wsize.g_w;
                min_ht = wsize.g_h;
                if( dum > wsize.g_h )
                {
                  if( (wsize.g_h = dum) > max.g_h ) wsize.g_h = max.g_h;
                  get_inner();
                  get_outer();
                }
              }
              if( (main_hand=x_wind_create( WIN_TYPE, X_MENU,
                  wsize.g_x, wsize.g_y, wsize.g_w, wsize.g_h )) > 0 )
              {
                /* set the window's min/max sizes */
                wind_set( main_hand, X_WF_MINMAX, min_wid, min_ht, min_wid, -1 );
                get_inner();
                build_mlist();
                /* position the app list to the start */
                maclist[0].ob_x = winner.g_x;
                maclist[0].ob_y = winner.g_y;
                /* tell Geneva it's a windowed dialog */
                main_dial();
                wind_set( main_hand, X_WF_DIALHT, main_h );
                for( i=0; i<4; i++ )
                  menu_icheck( menu, MTIMER+i, etypes&(1<<i) );
                /* put in the menu */
                wind_set( main_hand, X_WF_MENU, menu );
                main_title();
                main_info[0] = 1;	/* so that it will get set first time */
                do_info();
                wind_open( main_hand, wsize.g_x, wsize.g_y, wsize.g_w, wsize.g_h );
                was_open=1;
              }
              else alert( NOWIND );
              break;
            }
            buf[3] = main_hand;      /* fall through if already open */
          case WM_TOPPED:
            wind_set( buf[3], WF_TOP );
            break;
          case WM_FULLED:
            wind_get( buf[3], X_WF_MINMAX, &dum, &dum, &buf[6], &buf[7] );
            if( buf[3] == main_hand )
              if( (fulled^=1) != 0 )
              {
                full = wsize;
                buf[4] = wsize.g_x;
                buf[5] = max.g_y;
              }
              else *(GRECT *)&buf[4] = full;
            else if( (ed = find_edesc(buf[3])) == 0 ) break;
            else if( (ed->fulled^=1) != 0 )
            {
              ed->full = ed->outer;
              buf[4] = ed->outer.g_x;
              buf[5] = max.g_y;
            }
            else *(GRECT *)&buf[4] = ed->full;
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
close:        if( !test_close() ) break;
	      close_wind(&main_hand);
	      auto_last[0] = 1;
              if( _app ) quit();        /* running as a PRG, so quit */
              top_app();
/****auto_test( "PC      " );
buf[0] = AC_OPEN;
appl_write( apid, 16, buf ); for testing auto GMA ****/
            }
            else if( (ed = find_edesc(buf[3])) != 0 ) close_edit(ed);
            else for( i=0; form[i].init; i++ )
              if( form[i].handle==buf[3] ) close_wind( &form[i].handle );
            break;
          case AC_CLOSE:
            main_hand=0;
            auto_last[0] = 1;
            for( i=0; form[i].init; i++ )
              form[i].handle = 0;
            tim_edesc = 0L;
            iconified = 0;		/* just in case it was */
            free_edits(1);
            break;
          case AP_TERM:
            /* I'm being told to quit by the AES */
            quit();
          case WM_ICONIFY:
          case WM_ALLICONIFY:
            if( buf[3]==main_hand && !iconified ||
                (ed=find_edesc(buf[3]))!=0 && !ed->iconified )
                do_iconify(buf);
            break;
          case WM_UNICONIFY:
            if( buf[3]==main_hand && iconified ||
                (ed=find_edesc(buf[3]))!=0 && ed->iconified )
                do_uniconify(buf);
            break;
          case X_WM_RECSTOP:
            if( rec_mac ) end_record();
          case X_WM_VECKEY:
            if( form[1].handle>0 )
            {
              update_msg();
              close_wind( &form[1].handle );
            }
            if( rec_desc )
            {
              if( (i = get_mac_end(buf)) != 0 )
              {
                some_events( mac_buf, buf[0] );
                finish_evnts( rec_desc, rec_pos, i );
                do_einfo( rec_desc );
              }
              rec_desc = 0L;
            }
            check_rec();
            break;
          case X_MN_SELECTED:
            /* window menu item selected */
            if( buf[7]==main_hand && !iconified ) switch( buf[4] )
            {
              case OPEN:
                strcpy( temp, main_path );
                if( fselect( temp, "*.GMA", FSTITLE ) &&
                    *spathend(temp) != 0 ) load_gma( temp, 1 );
                break;
              case SAVE:
                do_save(0);
                break;
              case SAVEAS:
                do_save(1);
                break;
              case NEW:
                if( main_hand )
                  if( !test_close() ) break;
                if( iglobl<nmacs ) free_most_macs();
                else free_macs( 1, 0 );
                *spathend(main_path) = 0;
                main_title();
	        do_info();
                get_inner();
        	main_dial();
	        last_chk = chksum();
                break;
              case QUIT:
                if( main_hand )
                {
                  if( !test_close() ) break;
                  close_wind(&main_hand);
                }
                for( i=0; form[i].init; i++ )
                  close_wind(&form[i].handle);
                if( _app ) quit();
                break;
              case NEWMAC:
                macs_off(1);
                new_mac( 0L, 0 );
                break;
              case MGLOBL:
                if( iglobl==nmacs ) break;
                wind_update( BEG_UPDATE );
                for( i=iglobl; i<nmacs; i++ )
                {
                  mdesc[i].is_global = 0;
                  set_if( maclist, mdesc[i].obj, 1 );
                }
                iglobl = nmacs;
                delete();
                wind_update( END_UPDATE );
                break;
              case EDNEW:
                if( create_mac() ) edit_mac(iglobl-1);
                break;
              case EDMAC:
                i = mac_list(0);
                if( i>=0 )
                  do edit_mac(i);
                  while( (i = mac_list(1)) >= 0 );
                macs_off(1);
                break;
              case MCUT:
                if( save_clip(0L) ) delete();
                break;
              case MCOPY:
                save_clip(0L);
                macs_off(1);
                break;
              case MPASTE:
                load_clip(0L);
                break;
              case MDEL:
                delete();
                break;
              case MABOUT:
                start_form( 2, ABOUT, NAME|MOVER, 0 );
                break;
              case MHELP:
                /* get main help */
                rsrc_gaddr( 15, HELPMAIN, &ptr3 );
                do_help( *ptr3 );
                break;
              case MISC:
                start_form( 0, MISCOPTS, NAME|MOVER, 0 );
                break;
              case MDFLT:
                start_form( 4, DEFAULTS, NAME|MOVER, 0 );
                break;
              case SSAVE:
                bee();
                save_settings();
                arrow();
                break;
              case MTIMER:
              case MMOUSE:
              case MBUTTON:
              case MKEYBD:
                get_etypes();
                menu_icheck( menu, buf[4], (etypes^-1)&(1<<(buf[4]-MTIMER)) );
                break;
            }
            else if( (ed = find_edesc( buf[7] )) != 0 && !ed->iconified )
                do_emenu( ed, buf[4] );
            menu_tnormal( *(OBJECT **)&buf[5], buf[3], 1 );
            break;
        }
        if( emulti.event&X_MU_DIALOG )         /* user clicked in a dialog window */
        {
          but = buf[2]&0x7fff;          /* isolate just the object # */
          i = buf[2]&0x8000;
          if( buf[3]==main_hand )          /* main window */
              main_sel( i, but, emulti.mouse_k );
          else if( (ed=find_edesc( buf[3] )) != 0 ) edit_sel( ed, i, but, emulti.mouse_k );
          else use_form( buf[3], buf[2] );      /* process another dialog */
        }
        if( main_hand>0 && emulti.event&(MU_MESAG|X_MU_DIALOG) ) do_info();
        if( emulti.event&MU_TIMER && form[1].handle>0 ) update_msg();
      }
    }
    else if( _app ) alert( ALNOGEN );   /* Geneva not present */
  }
  else form_alert( 1, "[1][|GNVA_MAC.RSC not found!][Ok]" );
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

