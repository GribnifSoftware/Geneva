#include "stdio.h"
#include "new_aes.h"
#include "xwind.h"
#include "stdlib.h"
#include "string.h"
#include "tos.h"
#include "ierrno.h"
#include "vdi.h"
#include "linea.h"
#include "win_var.h"
#include "win_inc.h"
#include "windows.h"
#define _PROCMAN
#ifdef GERMAN
  #include "german\wind_str.h"
#else
  #include "wind_str.h"
#endif

#ifdef WINDOWS
  #define _wcolors dwcolors[wcolor_mode][1]
  #define COLOR_3D settings.color_root[color_mode].s.atari_3D
  #define get_msey() g_my
  #define objc_offset objc_off
  #define graf_mouse(a,b) _graf_mouse(a,b,0)
  #define objc_draw(a,b,c,d,e,f,g) _objc_draw((OBJECT2 *)a,0L,b,c,d,e,f,g)
  #define form_do _form_do
  #define x_appl_term __x_appl_term
  #define x_sprintf spf
  #define pman_big settings.flags2.s.procman_details	/* 006 */
  int _form_dial( int flag, Rect *small, Rect *big );
#endif

typedef struct
{
  char txt[50], sel, slpok;
  int apid, mint, sleep;
} PMENT;

static PMENT *list;
static int items, top;
static Rect pman_rect;

static void set_state( int truth, int ind, int draw )
{
  int newval, oldval;
  unsigned int *o;

  oldval = *(o = &(u_object(pman,ind)->ob_state));
  newval = truth ? (oldval|SELECTED) : (oldval&~SELECTED);
  if( oldval != newval )
    if( !draw ) *o = (unsigned int) newval;
    else change_objc( pman, 0L, ind, &pman_rect, newval, draw );
}

static PMENT *find_mint( char *n, int *mid )
{
  int i, x;
  APP *ap;

  *mid = -1;
  if( (n=strchr(n,'.')) == 0 ) return 0;
  if( (*mid = atoi(n+1)) == 0 ) return 0;
  for( x=0, ap=app0; ap; ap=ap->next, x++ )
    if( ap->mint_id==*mid ) return list ? &list[x] : (PMENT *)1L;
  return 0;
}

static void types( APP *ap, char *buf )
{
  int i, t=ap->type;
  static char y[] = "kb12mt";
  
  *buf++ = t&X_MU_DIALOG ? 'd' : '_';
  for( i=6; --i>=0; )
    *buf++ = t&(1<<i) ? y[i] : '_';
  *buf++ = 0;
}

static int sort( PMENT *a, PMENT *b )
{
  return strcmp( a->txt, b->txt );
}

static int get_list(void)
{
  APP *ap;
  int cnt, i, j, mid;
  char buf[20], *m;
  DTA *old, dta;
  PMENT *pm;
  static char atts[] = { 0, 1, 0x20, 0x21, 0x22, 2, 0x24 }, states[][7] =
      { "run   ", "ready ", "wait  ", "iowait", "zombie", "tsr   ", "stop  " };
  
  old = Fgetdta();
  Fsetdta(&dta);
  if( list ) lfree(list);
  list = 0L;
  for( cnt=0, ap=app0; ap; ap=ap->next )
    cnt++;
  if( !Fsfirst( "U:\\PROC\\*.*", 0x27 ) )
    do
      if( !find_mint(dta.d_fname,&mid) && mid>=0 ) cnt++;
    while( !Fsnext() );
  list = (PMENT *)lalloc( cnt*sizeof(PMENT), -1 );
  i=0;
  if( list )
  {
    for( ap=app0; ap && i<cnt; ap=ap->next, i++ )
    {
      list[i].mint = ap->mint_id;
      list[i].sel = 0;
      list[i].sleep = ap->asleep;
      list[i].slpok = !(ap->asleep&ASL_PEXEC);
      types( ap, buf );
      x_sprintf( list[i].txt, (char *)"%c%s %s  %c%c%c%c  %-3d  %s             ",
          ap->ap_type==4 ? '\xF8' : ' ',
          ap->dflt_acc+2,
          ap->asleep & ASL_PEXEC ? "child" :
            (ap->asleep & ASL_SINGLE ? "singl" :
            (ap->asleep & ASL_USER ? "sleep" : "     ")),
          ap->blocked ? 'b' : (ap==has_update ? 'U' : '_'),
          ap==has_mouse ? 'M' : '_',
          ap==has_menu ? 'E' : (ap->menu ? 'e' : '_'),
          top_wind->apid==ap->id ? 'T' : (ap->has_wind ? 'w' : '_'),
          list[i].apid=ap->id, buf );
    }
    if( i<cnt && !Fsfirst( "U:\\PROC\\*.*", 0x27 ) )
      do
      {
        pm = find_mint(dta.d_fname,&mid);
        if( mid<0 ) continue;
        m = (char *)"  ?   ";
        for( j=sizeof(atts); --j>=0; )
          if( atts[j]==dta.d_attrib )
          {
            m = states[j];
            break;
          }
        x_sprintf( buf, (char *)"%-3d  %s", mid, m );
        if( pm ) strcpy( pm->txt+37, buf );
        else if( i<cnt )
        {
          list[i].apid = -1;
          list[i].mint = mid;
          list[i].sel = 0;
          list[i].slpok = dta.d_attrib == 0x24 ? (char) -1 : (char) 1;
          *strchr(dta.d_fname,'.') = 0;
          if( pman_big ) x_sprintf( list[i].txt, (char *)"\xF8%-36s%s", dta.d_fname, buf );
          else x_sprintf( list[i].txt, (char *)"\xF8%-8s%s", dta.d_fname, buf+4 );
          i++;
        }
      }
      while( !Fsnext() );
  }
  Fsetdta(old);
  pman[PMSMALL].ob_height = (items=i)<=10 ? pman[PMBIG].ob_height :
      pman[PMBIG].ob_height*10/i;
  if(list) qsort( list, items, sizeof(PMENT), (int (*)())sort );
  return list!=0;
}

static int position_list( int t, int slider, int updt )
{
  int i, j, ret;

  if( t>items-10 ) t = items-10;
  if( t<0 ) t = 0;
  ret = t!=top;
  if( !updt ) return ret;	/* 006 */
  top = t;  		/* 006: used to ignore this block if no change, instead use updt */
  for( i=t-top+PMLINE1; t<top+10; t++, i++ )
  {
    j = hide_if( pman, t<items, i );
    pman[i].ob_spec.tedinfo->te_ptext = !j ? (char *)"" : list[t].txt;
    pman[i].ob_state = !j ? 0 : list[t].sel;
    if( j && !pman_big ) list[t].txt[16] = 0;
  }
  if( ret/*006*/ && slider ) pman[PMSMALL].ob_y = items<=10 ? 0 :
     (long)top*(pman[PMBIG].ob_height-pman[PMSMALL].ob_height)/(items-10);
  return ret;
}

static void others_off( int exc )
{
  int j, s;

  s = (int)Kbshift(-1)&3;		/* 006: just sample state now */
  if( s && exc ) return;	/* 006 */
  for( j=0; j<items; j++ )
    if( list[j].sel && j!=exc-PMLINE1+top )
    {
      if( !s/*006*/ ) list[j].sel = 0;
      if( j>=top && j<top+10 ) set_state( 0, j-top+PMLINE1, 1 );
    }
}

static void scroll( int inc, int slider )
{
  int i;

  if( position_list( top+inc, slider, 0 ) )	/* 006: just check */
  {
    others_off(0);				/* 006: moved inside if */
    position_list( top+inc, slider, 1 ); 	/* 006: now really do it */
    if( slider ) objc_draw( pman, PMBIG, 8, Xrect(pman_rect) );
    for( i=10; --i>=0; )
      objc_draw( pman, PMLINE1+i, 0, Xrect(pman_rect) );
  }
}

static void reset_but( int i, int draw )
{
  set_state( 0, i, draw );
}

static void ob_att( int obj )
{
  OBJECT *o;

  o = u_object(pman,obj);
  if( COLOR_3D ) o->ob_flags |= 0x0400;
  else o->ob_flags &= ~0x0600;
}

static void set_pmsize(void)
{
  int i;

#ifdef WINDOWS
  if( desktop->working.w/char_w < 54 ) hide_if( pman, pman_big=0, PMDETAIL );
#endif
  pman[PMTITL].ob_spec.free_string[16] = pman_big ? ' ' : (char)0;
  hide_if( pman, pman_big, PMTITL0 );
  hide_if( pman, pman_big, PMI0 );
/*  pman[PMI1].ob_x = pman[PMI0].ob_x; */
  if( pman_big )
  {
/*    pman[PMI1].ob_x += pman[PMI0].ob_width;*/
    pman[0].ob_width = 54*char_w;
    pman[PMLINE1-1].ob_width = 49*char_w;
    pman[PMDETAIL].ob_state |= SELECTED;
    pman[PMTITL].ob_x = char_w<<1;
  }
  else
  {
    pman[0].ob_width = pman[PMSHIFT].ob_width+(char_w<<1);
    pman[PMLINE1-1].ob_width = pman[PMSHIFT].ob_width-pman[PMI2].ob_width;
    pman[PMTITL].ob_x = ((pman[PMLINE1-1].ob_width-(char_w<<4))>>1)+pman[PMLINE1-1].ob_x;
    pman[PMDETAIL].ob_state &= ~SELECTED;
  }
  pman[PMI1].ob_x = pman[0].ob_width-pman[PMI1].ob_width;
  pman[PMI2].ob_x = pman[PMLINE1-1].ob_x + pman[PMLINE1-1].ob_width;
  pman[PMSHIFT].ob_x = (pman[0].ob_width-pman[PMSHIFT].ob_width)>>1;
  pman[PMDETAIL].ob_x = ((pman[PMI1].ob_x-pman[PMLINE1-1].ob_x-pman[PMDETAIL].ob_width)>>1)+
      pman[PMLINE1-1].ob_x;
  for( i=PMLINE1; i<=PMLINE1+9; i++ )
    pman[i].ob_width = pman[PMLINE1-1].ob_width;
  pman[PMBIG].ob_height = (pman[PMLINE1-1].ob_height-((pman[PMUP].ob_height+1)<<1));
  pman[PMDOWN].ob_y = pman[PMBIG].ob_y + pman[PMBIG].ob_height + 1;
}

void procman(void)
{
  int fd, i, j, done, k, dum;
  static char cols[][2] = { PMUP, WGUP, PMDOWN, WGDOWN, PMBIG, WGVBIGSL, PMSMALL, WGVSMLSL };
  static char obs[] = { 0, PMLINE1-1, PMLINE1, PMLINE1+1, PMLINE1+2, PMLINE1+3, PMLINE1+4,
      PMLINE1+5, PMLINE1+6, PMLINE1+7, PMLINE1+8, PMLINE1+9, PMUP, PMDOWN };

again:
  top = -1;
  set_pmsize();
  if( !get_list() ) return;
  for( i=0; i<sizeof(cols)/2; i++ )
  {
    *((int *)&u_object(pman,cols[i][0])->ob_spec.index+1) = _wcolors[cols[i][1]];
#ifdef WINDOWS
    u_object(pman, cols[i][0])->ob_state = (unsigned int) wstates[wcolor_mode][cols[i][1]];
#endif
  }
  for( i=sizeof(obs); --i>=0; )
    ob_att(obs[i]);
  position_list( 0, 1, 1 );
#ifdef WINDOWS
  hide_if( pman, has_mint && pman_big, PMIBOX );
  _form_center( pman, &pman_rect, 1 );
  fd = _form_dial( X_FMD_START, 0L, &pman_rect ) ? X_FMD_FINISH : FMD_FINISH;
#else
  x_form_center( pman, &pman_rect.x, &pman_rect.y, &pman_rect.w, &pman_rect.h );
  fd = form_dial( X_FMD_START, 0, 0, 0, 0, Xrect(pman_rect) ) ? X_FMD_FINISH : FMD_FINISH;
#endif
  objc_draw( pman, 0, 8, Xrect(pman_rect) );
#ifdef WINDOWS
  force_mouse(0);
#else
  graf_mouse( X_MRESET, 0L );
  graf_mouse( ARROW, 0L );
#endif
  for(done=0;!done;)
    switch( i=form_do( pman, 0 )&0x7fff )
    {
      default:
        if( i>=PMLINE1 && i<PMLINE1+10 )
        {
          others_off(i);
          list[i-PMLINE1+top].sel = (char) pman[i].ob_state;
        }
        break;
      case PMUPDT:
#ifdef WINDOWS
        cnt_update.i = 0;
        set_update();
        unblock();
#endif
        reset_but( PMUPDT, 1 );
        if( !get_list() ) done=1;
        else
        {
          others_off(0);
          position_list( top, 1, 1 );
          objc_draw( pman, PMLINE1-1, 8, Xrect(pman_rect) );
          objc_draw( pman, PMBIG, 8, Xrect(pman_rect) );
        }
        break;
      case PMUP:
        scroll( -1, 1 );
        break;
      case PMDOWN:
        scroll( 1, 1 );
        break;
      case PMSMALL:
        if( graf_slidebox( 0L, items, 10, 0x101 ) >= 0 )
        {
          set_state( 1, PMSMALL, 1 );
          i = graf_slidebox( pman, PMBIG, PMSMALL, 0x201 );
          while( i>=0 )
          {
            scroll( i - top, 0 );
            i = graf_slidebox( pman, PMBIG, PMSMALL, 0x301 );
          }
          set_state( 0, PMSMALL, 1 );
        }
        break;
      case PMBIG:
        objc_offset( pman, PMSMALL, &dum, &k );
        scroll( get_msey()<k ? -10 : 10, 1 );
        break;
      case PMSLEEP:
        for( j=0; j<items; j++ )
          if( list[j].sel && list[j].slpok ) done = 1;
        reset_but( i, !done );
        break;
      case PMTERM:
        for( j=0; j<items; j++ )
          if( list[j].sel ) done = 1;
        reset_but( i, !done );
        break;
      case PMCANCEL:
        done = 1;
        reset_but( i, !done );
        break;
      case PMDETAIL:
        pman_big = pman[PMDETAIL].ob_state&SELECTED;
#ifdef WINDOWS
        _form_dial( fd, 0L, &pman_rect );
#else
        form_dial( fd, 0, 0, 0, 0, Xrect(pman_rect) );
#endif
        goto again;
    }
#ifdef WINDOWS
  _form_dial( fd, 0L, &pman_rect );
  force_mouse(1);
#else
  form_dial( fd, 0, 0, 0, 0, Xrect(pman_rect) );
#endif
  if( i==PMSLEEP || i==PMTERM )
    for( j=0; j<items; j++ )
      if( list[j].sel )
        if( i==PMTERM )
          if( list[j].apid>=0 ) x_appl_term( list[j].apid, 0, 1 );
          else Pkill( list[j].mint, SIGTERM );
        else if( list[j].slpok )
          if( list[j].mint>=0 && list[j].apid<0 )
              Pkill( list[j].mint, list[j].slpok<0 ? SIGCONT : SIGSTOP );
          else x_appl_sleep( list[j].apid, (list[j].sleep>>1)^1 );
  if(list)
  {
    lfree(list);
    list = 0L;
  }
}
