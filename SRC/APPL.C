/*#define MINT_DEBUG*/

#include "stdio.h"
#include "new_aes.h"
#include "xwind.h"
#include "stdlib.h"
#include "string.h"
#include "ctype.h"
#include "tos.h"
#include "ierrno.h"
#include "multevnt.h"
#include "linea.h"
#include "windows.h"
#define _APPLIC
#ifdef GERMAN
  #include "german\wind_str.h"
  #include "german\windows.rsh"
#else
  #include "wind_str.h"
  #include "windows.rsh"
#endif
#define _APPLIC
#include "win_var.h"
#include "win_inc.h"
#include "fsel.h"
#include "debugger.h"

void test_msg( APP *ap, char *msg );

#define NUM_OBS   223	/* taken from fsel.rh */
#define INIT_VERSION    0x400		/* appl_init return */
#define ABOUT_VER_MAJOR	1		/* in About dialog */
#define ABOUT_VER_MINOR	8		/* in About dialog */
#define UNLOAD_TIME	2		/* wait 2 secs for apps to quit */

#include "initial.h"

char ind_move=1, ind_change=1, act_move=1, act_change=0;
int ind_col, act_col, bkgrnd_col, add3d_h=2, add3d_v=2;
OBJECT *mag_menu, *about;
int ticsec;
int iargc, sversion, alt_obj[MAX_KEYS]/* 004: moved to int */;
char **iargv, **ienvp;
char did_unload, is_auto, no_load;
char lastapp_name[9], is_falc;
long setexc( int number, void (*newfunc)(), void (**oldfunc)(), int force );
void warmboot(void);
char fvalid[] = "_!@#$%^&()+-=~`;\'\",<>|[]{}";
int work_in[] = { 1, 7, 1, 1, 1, 1, 1, 1, 1, 1, 2 }, work_out[57];
static char exec_path[120], exec_name[13*5+1];
unsigned char alt[MAX_KEYS], alt_off[MAX_KEYS];
long vid;
int my_keyvec( long *key );
int my_switch( char *process_name, int apid );
int my_genevent(void);
G_VECTORS g_vectors = { 7, my_keyvec, my_switch, my_genevent };
int ROM_ver, neo_ver;
extern char is_PD, wait_curapp;
/*extern BASPAG *PD_bp; 006*/

void unencode( char *str, char *out )
{
  int i;

  for( i=0; i<25; i++ )
    *out++ = *str++ ^ (i+2);
}

void alloc_kbbuf(void)
{
  unsigned int i;

  if( (kbbuf = lalloc(i=(unsigned)(kbio->ibufsiz)>>2,-1)) != 0 )	/* 004: unsigned, added if */
      memset( kbbuf, -1, kbsize=i );
}

void set_kbd(void)
{
  KBDVBASE *kb;
  extern void my_midi();
  extern void (*old_midi)();
  extern IOREC *midi_io;

  if( !kbio )
  {
    kbio = Iorec(1);
    alloc_kbbuf();
    if( kbbuf )		/* 004: now it checks */
    {
      old_kbd = (kb=Kbdvbase())->kb_kbdsys;
      kb->kb_kbdsys = my_kbd;
      midi_io = Iorec(2);		/* 005 */
      old_midi = kb->kb_midisys;	/* 005 */
      kb->kb_midisys = my_midi;		/* 005 */
      setexc( 0x114, my_200, &old_200, 0 );
    }
  }
  else
  {
    if( kbbuf )
    {
      (kb=Kbdvbase())->kb_kbdsys = old_kbd;
      kb->kb_midisys = old_midi;	/* 005 */
      setexc( 0x114, old_200, 0L, 1 );
      lfree(kbbuf);
    }
    kbio=0;
  }
}

void move_kbbuf(void)
{
  unsigned int i;
  char *k;

  if( (unsigned)kbio->ibufsiz > (unsigned)(kbsize<<2) )
  {
    k = kbbuf;
    alloc_kbbuf();
    if( !kbbuf ) kbbuf = k;
    else lfree(k);
  }
}

int ofixw, ofixh;

void fixx2( int *i )
{
  *i = ((*i&0xFF)*ofixw) + (*i>>8);
}

void fixy2( int *i )
{
  *i = ((*i&0xFF)*ofixh) + (*i>>8);
}

int odd_obfix( OBJECT *tree, int ind )
{
  int *i = &(tree=u_object(tree,ind))->ob_x;

  fixx2( i++ );
  fixy2( i++ );
  fixx2( i++ );
  fixy2( i );
  if( ofixw==6 && (char)tree->ob_type==G_STRING )
      tree->ob_state |= (X_MAGIC|X_SMALLTEXT);
  return(1);
}

void ufixx( int *i )
{
  *i = *i/ofixw + ((*i%ofixw)<<8);
}

void ufixy( int *i )
{
  *i = *i/ofixh + ((*i%ofixh)<<8);
}

int unfix( OBJECT *tree, int ind )
{
  int *i = &u_object(tree,ind)->ob_x;

  ufixx( i++ );
  ufixy( i++ );
  ufixx( i++ );
  ufixy( i );
  return(1);
}

void shorten( int flag )
{
  int j, *ptr, *ptr2, wb;
  BITBLK *bb = about[COPLOGO].ob_spec.bitblk;
  static int *buf, *old_data;

  if( !flag )
  {
    /* copy every 2nd line into a new buffer */
    if( (buf = (int *)lalloc( (bb->bi_hl>>1)*(wb=bb->bi_wb), -1 )) != 0 )
    {
      (char *)ptr=(char *)(old_data=bb->bi_pdata)+wb;
      for( j=bb->bi_hl>>=1, ptr2=buf; --j>=0; ptr+=wb, (char *)ptr2+=wb )
        memcpy( ptr2, ptr, wb );
      bb->bi_pdata = buf;
    }
  }
  else if( buf )
  {
    bb->bi_pdata = old_data;
    bb->bi_hl<<=1;
    lfree(buf);
    buf = 0L;
  }
}

void fix_trees( int flag )
{
  int i, j;

  if( desktop->outer.w > char_w*51 )
  {
    ofixw = char_w;
    ofixh = char_h;
  }
  else ofixw=ofixh=6;
  for( i=0; i<sizeof(ascii_tbl)/sizeof(OBJECT); i++ )
    if( !flag ) odd_obfix( ascii_tbl, i );
    else unfix( ascii_tbl, i );
  ofixw = char_w;
  ofixh = char_h;
  about = rs_trindex[COPYRT];
  if( !flag )
  {
    ascii_tbl[0].ob_x = (desktop->outer.w-ascii_tbl[0].ob_width)>>1;
    ascii_tbl[0].ob_y = desktop->working.y + menu_h + 2;
    j = about[COPLOGO].ob_height;
    fixy2(&j);
    if( j < about[COPLOGO].ob_spec.bitblk->bi_hl ) shorten(flag);
  }
  else shorten(flag);
  /* fix about dialog, mag_menu, and procman */
  for( i=rs_trindex[COPYRT]-rs_trindex[0];
       i<sizeof(rs_object)/sizeof(OBJECT); i++ )
    if( !flag ) obfix( rs_object, i );
    else unfix( rs_object, i );
  for( i=NUM_OBS; --i>=0; )
    if( !flag ) obfix( fselobj, i );
    else unfix( fselobj, i );
  if( !flag )
  {
    fsel = fselind[0];
    for( i=2; i<12; i++ )
      u_obspec(fselind[EXTLIST],i)->free_string = settings.fsel_ext[i-2];
    for( i=PATH1; i<PATH1+10; i++ )
      u_obspec(fselind[PATHLIST],i)->free_string = settings.fsel_path[i-PATH1];
    (fselind[FFIND])[FFINAME].ob_spec.tedinfo->te_ptext = settings.find_file;
    fsel[FSFBIG].ob_height = fsel[FSOBIG].ob_height = fsel[FSFBIG2].ob_height =
        fsel[FSOOUTER].ob_height - 3*(fsel[FSCLOSE].ob_height+1);
    fsel_type();
    ob_fixspec();
    /* center all lines of About box */
    j = (about[COPLOGO].ob_width = about[COPLOGO].ob_spec.bitblk->bi_wb<<3) + 8;
    if( (i=about[0].ob_width) > j ) j = i;
    if( (i=about[4].ob_width) > j ) j = i+8;
    about[0].ob_width = j;
    for( i=2; i<=about[0].ob_tail; i++ )
      u_object(about,i)->ob_x = (j-u_object(about,i)->ob_width)>>1;
    about[1].ob_x = j-(about[1].ob_width=16);
  }
}

void set_vex( int flag )
{
  if( !flag )
  {
    Supexec( set_frame );
    setexc( 0x84, my_t1, &old_t1, 0 );
    if( !is_auto ) setexc( 0xb4, my_t13, &old_t13, 0 );
    if( !preempt ) setexc( 0x408, my_term, &old_term, 0 );
    Supexec( set_t2 );
  }
  else
  {
    setexc( 0x404, old_crit, 0L, 1 );
    setexc( 0x84, old_t1, 0L, 1 );
    if( !is_auto ) setexc( 0xb4, old_t13, 0L, 1 );
    else
    {
      Supexec( get_t2 );
      setexc( 0xb4, term_t13, &old_t13_term, 0 );
    }
    if( !preempt ) setexc( 0x408, old_term, 0L, 1 );
    if( !is_auto )
    {
      if( old_vdi ) setexc( 0x88, old_vdi, 0L, 1 );	/* 004 */
      setexc( 0x88, old_t2, 0L, 1 );
      Supexec( undo_linea );
    }
  }
}

int set_asc_cur( int x, int y )
{
  int ret, w;

  y--;
  ret = (x-ascii_tbl[0].ob_x)/(w=ascii_tbl[0].ob_width/51);
  ascii_tbl[6].ob_x = ret*w;
  ascii_tbl[6].ob_y = y*ascii_tbl[1].ob_height;
  ascii_tbl[6].ob_flags &= ~HIDETREE;
  return ret + y*51 + 1;
}

int x_key( unsigned char num, char *tbl )
{
  int i;

  for( i=128; --i>=0; )
    if( *tbl++==num )
    {
      lastkey = ((127-i)<<8) | num;
      return 1;
    }
  return 0;
}

void trans_key( unsigned char num )
{
  KEYTAB *kt=Keytbl((void *)-1L, (void *)-1L, (void *)-1L);

  lastsh = 0;
  if( x_key( num, kt->unshift ) ) return;
  if( x_key( num, kt->shift ) || x_key( num, kt->capslock ) )
  {
    lastsh = 1;
    return;
  }
  lastkey = num;
}

int exec_ext( char *env, int i )
{
  char *ptr;

  if( _shel_envrn( curapp->env, &ptr, env ) && *ptr )  /* 004: was shel_envrn */
  {
    strcat( exec_path, i ? "," : "*.{" );
    strcat( exec_path, ptr );
    return 1;
  }
  return i;
}

char *skip_space( char *p2 )
{
  while( *p2 && isspace(*p2) ) p2++;
  return p2;
}

int do_shwrit( int is_shell, char *name, char *path, char *tail )
{
  SHWRCMD shwrcmd;
  char temp[150], temp2[150];

  strcpy( temp, path );
  strcpy( pathend(temp), pathend(name) );
  shwrcmd.name = temp;
  strcpy( temp2, path );	/* 004 */
  *pathend(temp2) = 0;		/* 004: used to use temp for both */
  shwrcmd.dflt_dir = temp2;
  if( !shel_write( is_shell, 0, 0, (char *)&shwrcmd, tail ) ) return 0;
  return 1;
}

char *scan_cmd( char *buf, char *cmd )
{
  char *p2, *p3;

  for( p2=buf; *p2 && !isspace(*p2); p2++ );
  if( !*p2 ) return 0L;
  *p2++ = 0;
  if( !strcmpi( buf, cmd ) )
  {
    p2 = skip_space(p2);
    for( p3=p2; *p3 && !isspace(*p3); p3++ );
    if( !*p3 ) *(p3+1) = 0;
    else *p3++ = 0;
    if( p3 != p2 ) return p2;
  }
  else *(p2-1) = ' ';
  return 0L;
}

void cdecl sigchilddisp( long sig )
{
  MSG msg;

  if( (int)(Pwait3( 3, 0L )>>16L)==loop_id && sig!=SIGSTOP )		/* 005: test pid & sig */
      Pmsg( 0x8001, 0x476E8000L|loop_id, &msg );	/* return control to initialize() */
}

void do_about( int flag )
{
  Rect r;
  int i, j;

  spf( about[COPDATE].ob_spec.tedinfo->te_ptext, "%d.%02d %s", ABOUT_VER_MAJOR, ABOUT_VER_MINOR, __DATE__ );
  _form_center( about, &r, 1 );
  if( flag ) i = form_dial( X_FMD_START, 0, 0, 0, 0, Xrect(r) );
  if( !flag || !i ) about[COPMOVE].ob_flags |= HIDETREE;
  else about[COPMOVE].ob_flags &= ~HIDETREE;
  _objc_draw( (OBJECT2 *)about, 0L, 0, 8, Xrect(r) );
  if( flag )
  {
    if( (j=form_do( about, 0 )) > 0 ) sel_if(about,j,0);
    objc_xywh( (long)about, 0, &r );
    adjust_rect( about, &r, 1 );
    form_dial( i ? X_FMD_FINISH : FMD_FINISH, 0, 0, 0, 0, Xrect(r) );
  }
}

char *fix_tail( char *tail, char *temp )
{
  strcpy( temp+1, tail );
  *temp = strlen(tail);
  return temp;
}

void magic_shell(void)
{	/* must not use any direct calls that rely on curapp! */
  APP *ap;
  int msgbuf[8], but, asc_hand=0, i, apid;
  long exit_time;
  char temp[130], temp2[130], *p2;
  Window *w;
  Rect r;
  EMULTI emulti;
  static char cmds[4][9] = { "RUN", "SHELL", "RUNSLEEP", "RUNACC" };
  static int modes[] = { SHD_DFLTDIR, SH_RUNSHELL, SH_RUNSLEEP, SH_RUNACC };

  emulti.type = MU_MESAG;
  new_flags();
  apid = appl_init();
  shel_write( 9, 1, 0, 0L, 0L );
  if( !no_load )
  {
    loading=1;
    do
    {
      appl_yield();          /* start the DA's loading */
/***      if( Bconstat(2) )
      {
        Bconin(2);
        for( ap=app0; ap; ap=ap->next )
          if( ap->start_end ) Cconws(ap->dflt_acc);
        Bconin(2);
      }	005: seems to be testing code, so del it */
    }
    while( loading || apps_initial );
  }
  if( is_auto ) mag_menu[MAGQUIT].ob_state |= DISABLED;	/* 004 */
  menu_bar( mag_menu, 1 );
  graf_mouse( ARROW, 0L );
  if( !no_load )
  {
    gem_cnf( 0, 0L );
    while( gem_cnf(1,temp) > 0 )
      for( i=0; i<4; i++ )	/* 004 */
        if( (p2 = scan_cmd( temp, cmds[i] )) != 0 )
        {
          do_shwrit( modes[i], p2, p2, fix_tail(skip_space(p2+strlen(p2)+1),temp2) );
          break;
        }
    gem_cnf( 2, 0L );
  }
  if( exec_name[0] ) do_shwrit( SHD_DFLTDIR, exec_name, exec_path, "\0" );
  *pathend(exec_path)=0;
  if( !exec_ext( "ACCEXT=", exec_ext( "TOSEXT=", exec_ext( "GEMEXT=", 0 ) ) ) )
      strcat( exec_path, "*.*" );
  else strcat( exec_path, "}" );
  /* 004: GNVADESK crashes       draw_desk();  */
  msgbuf[0] = WM_REDRAW;
  msgbuf[2] = msgbuf[3] = 0;
  *(Rect *)&msgbuf[4] = desktop->working;
  appl_write( msgbuf[1] = apid, 16, msgbuf );
  for(;;)
  {
    multi_evnt( &emulti, msgbuf );
    if( emulti.event&MU_MESAG )
      switch( msgbuf[0] )
      {
        case MN_SELECTED:
          switch( msgbuf[4] )
          {
            case MAGQUIT:
              shel_write( 4, -1, 0, 0L, 0L );	/* shut down */
              break;
            case MAGHELP:
              if( !x_help( SHHELP, 0L, 0 ) ) form_alert( 1, SHNOHELP );
              break;
            case MAGABOUT:
              do_about(1);
              break;
            case MAGASCII:
              if( asc_hand>0 ) wind_set( asc_hand, WF_TOP, asc_hand );
              else
              {
                r.x = ascii_tbl[0].ob_x-1;
                r.y = ascii_tbl[0].ob_y-cel_h;
                r.w = ascii_tbl[0].ob_width+2;
                r.h = ascii_tbl[0].ob_height+cel_h+1;
                if( (asc_hand = wind_create( NAME|MOVER|CLOSER,
                    Xrect(r) )) > 0 )
                {
                  w = ascii_w = find_window(asc_hand);
                  w->top_bar = ASCII_TITLE;
                  w->bevent |= 1;
                  wind_set( asc_hand, X_WF_DIALOG, ascii_tbl );
                  wind_open( asc_hand, Xrect(r) );
                  emulti.type |= X_MU_DIALOG;
                }
              }
              break;
            case MAGOPEN:
              if( !x_fsel_input( exec_path, sizeof(exec_path), exec_name,
                  5, &but, SHWOPSTR ) || but==0 ) break;
              for( p2=exec_name; *p2; p2+=strlen(p2)+1 )
                if( !do_shwrit( SHD_DFLTDIR, p2, exec_path, "\0" ) )
                {
                  sspf_alert( SHWOPEN, p2 );
                  break;
                }
              break;
          }
          menu_tnormal( mag_menu, msgbuf[3], 1 );
          break;
        case SHUT_COMPLETED:
          if( !msgbuf[3] ) break;
          unloader = 0L;	/* should have been me */
        case AP_TERM:
          goto quit;
        case CH_EXIT:
          if( msgbuf[4] )
          {
            spf( temp, SHWEXIT, lastapp_id==msgbuf[3] ? lastapp_name : SHWUNKN,
                msgbuf[4] );
            if( msgbuf[4]<0 ) x_form_error( temp, msgbuf[4] );
            else sspf_alert( temp, SHWUNDF );
          }
          break;
        case WM_TOPPED:
          wind_set( asc_hand, WF_TOP, asc_hand );
          break;
        case WM_CLOSED:
          wind_close( asc_hand );
          wind_delete( asc_hand );
          emulti.type &= ~X_MU_DIALOG;
          asc_hand=0;
          break;
        case WM_MOVED:
          wind_set( asc_hand, WF_CURRXYWH, msgbuf[4], msgbuf[5],
              msgbuf[6], msgbuf[7] );
          break;
      }
    if( emulti.event&X_MU_DIALOG )
      if( asc_hand>0 && msgbuf[3]==asc_hand )
      {
        i = set_asc_cur( curapp->mouse_x, msgbuf[2] );
        spf( temp, "ASCII %d ($%x)", i, i );
        wind_set( asc_hand, WF_NAME, temp );
        x_wdial_change( asc_hand, 6, SELECTED );
        while( curapp->mouse_b&1 ) get_mks();
        x_wdial_change( asc_hand, 6, 0 );
        ascii_tbl[6].ob_flags |= HIDETREE;
        wind_set( asc_hand, WF_NAME, ASCII_TITLE );
        trans_key(i);
      }
  }
quit: ;
  menu_bar( 0L, 0 );
  exit_time = 0L;
  for(;;)
  {
    appl_read( -1, 16, msgbuf );	/* dummy */
    if( unloader || msg_q ) continue;
    for( ap=app0, i=0; ap; ap=ap->next )
      if( ap->ap_type==2 )
      {
        i++;
        break;
      }
    if( !i )
    {
      if( !app0->next || !did_unload ) break;
      if( !exit_time ) exit_time = tic;
      else if( tic-exit_time > UNLOAD_TIME*ticsec ) break;      /* wait x secs */
    }
  }
  if( is_auto )
  {
    unload = -1;          /* terminate everything */
    appl_yield();         /* unload */
  }
  /* in_disp_loop = 0; */
  if( has_mint ) sigchilddisp(SIGKILL);
}

int test_rez (void)
{
  int     i, np, color, pxy[8], rgb[3], bpp = 0;
	unsigned int    backup[32], test[32];
	int     black[3] = {0, 0, 0};
	int     white[3] = {1000, 1000, 1000};
	int     (*rgb_palette)[256][4];
	MFDB    screen;
	MFDB    pixel = {0L, 16, 1, 1, 0, 1, 0, 0, 0};
	MFDB    stdfm = {0L, 16, 1, 1, 1, 1, 0, 0, 0};
	int     pixtbl[16] = {0, 2, 3, 6, 4, 7, 5, 8, 9, 10, 11, 14, 12, 15, 13, 16};

	if (vplanes >= 8)
	{
		for (color = 0; color < 256; color++)
			*(unsigned char *)&farbtbl2[color] = color;

		if (vplanes == 8)
		{
			color = 0xff;
			memset (test, 0, vplanes * sizeof (int));
			for (np = 0; np < vplanes; np++)
				test[np] = (color & (1 << np)) << (15 - np);

			pixel.fd_addr = stdfm.fd_addr = test;
			vr_trnfm (vdi_hand, &stdfm, &pixel);

			for (i = 1; i < vplanes; i++)
				if (test[i])	break;

			if (i >= vplanes && !(test[0] & 0x00ff))
				bpp = 1;
		}
		else
		{
			_vs_clip (0, 0L);
			_graf_mouse (M_OFF, 0L, 0);
			screen.fd_addr = 0L;
			_vswr_mode (MD_REPLACE);
			_vsl_type (1);
			memset (pxy, 0, sizeof (pxy));
			pixel.fd_addr = backup;	/* Punkt retten */

			memset (backup, 0, sizeof (backup));

			vro_cpyfm (vdi_hand, S_ONLY, pxy, &screen, &pixel);

			/* Alte Farbe retten */
			vq_color (vdi_hand, 15, 1, rgb);

			/* Ger�teabh�ngiges Format testen */
			pixel.fd_addr = test;
			_vsl_color (15);
			vs_color (vdi_hand, 15, white);
			v_pline (vdi_hand, 2, pxy);

			memset (test, 0, vplanes * sizeof (int));
			vro_cpyfm (vdi_hand, S_ONLY, pxy, &screen, &pixel);

			for (i = (vplanes + 15) / 16 * 2; i < vplanes; i++)
				if (test[i])	break;

			if (i >= vplanes)
			{
				vs_color (vdi_hand, 15, black);
				v_pline (vdi_hand, 2, pxy);

				memset (test, 0, vplanes * sizeof (int));
				vro_cpyfm (vdi_hand, S_ONLY, pxy, &screen, &pixel);

				for (i = (vplanes + 15) / 16 * 2; i < vplanes; i++)
					if (test[i])	break;

				if (i >= vplanes)
					bpp = (vplanes + 7) / 8;
			}

			/* Alte Farbe restaurieren */
			vs_color (vdi_hand, 15, rgb);

			pixel.fd_addr = backup;	/* Punkt restaurieren */
			vro_cpyfm (vdi_hand, S_ONLY, pxy, &pixel, &screen);

			/* Read the color palette */
		 	if( (rgb_palette = (int (*)[256][4])lalloc(256*4*2,-1)) != 0 )
			{
			  for (color = 0; color < 255; color++)
			  {	if (color < 16)
				{	vq_color (vdi_hand, pixtbl[color], 1, (*rgb_palette)[color]);
					(*rgb_palette)[color][3] = pixtbl[color];
				}
				else
				{	vq_color (vdi_hand, color + 1, 1, (*rgb_palette)[color]);
					(*rgb_palette)[color][3] = color + 1;
				}
			  }
			  vq_color (vdi_hand, 1, 1, (*rgb_palette)[255]);
			  (*rgb_palette)[255][3] = 1;

			  memset (backup, 0, sizeof (backup));
	 		  memset (farbtbl, 0, 32 * 256 * sizeof (WORD));
			  stdfm.fd_nplanes = pixel.fd_nplanes = vplanes;

			  vro_cpyfm (vdi_hand, S_ONLY, pxy, &screen, &pixel);

			  /* Alte Farbe retten */
			  vq_color (vdi_hand, 15, 1, rgb);

			  for (color = 0; color < 256; color++)
			  {
				vs_color (vdi_hand, 15, (*rgb_palette)[color]);
				vsl_color (vdi_hand, 15);
				v_pline (vdi_hand, 2, pxy);

				stdfm.fd_addr = pixel.fd_addr = farbtbl[color];

				/* vro_cpyfm, weil v_get_pixel nicht mit TrueColor (>=24 Planes) funktioniert */
				vro_cpyfm (vdi_hand, S_ONLY, pxy, &screen, &pixel);

				if (farbtbl2 != NULL && bpp)
				{	farbtbl2[color] = 0L;
					memcpy (&farbtbl2[color], pixel.fd_addr, bpp);
				}

				vr_trnfm (vdi_hand, &pixel, &stdfm);
				for (np = 0; np < vplanes; np++)
					if (farbtbl[color][np])
						farbtbl[color][np] = 0xffff;
			  }

			  /* Alte Farbe restaurieren */
			  vs_color (vdi_hand, 15, rgb);

			  pixel.fd_addr = backup;	/* Punkt restaurieren */
			  vro_cpyfm (vdi_hand, S_ONLY, pxy, &pixel, &screen);
  			  lfree( rgb_palette );
			}

			_graf_mouse (M_ON, 0L, 0);
		}
	}

	return (bpp);
}

long get_idt_fmt(void)
{
  static unsigned int vals[] = { (0<<12) | (0<<8) | '/',  /* USA 004: was 0 */
  				 (1<<12) | (1<<8) | '.',  /* Germany */
  				 (1<<12) | (1<<8) | 0,	  /* France */
  				 (1<<12) | (1<<8) | '.',  /* UK */
  				 (1<<12) | (1<<8) | 0,	  /* Spain */
  				 (1<<12) | (1<<8) | 0 };  /* Italy */
  unsigned int mode = (*(SYSHDR **)0x4f2)->os_base->os_palmode>>1;	/* 004: added os_base */

  /* added /2 for 004 */
  return mode>=sizeof(vals)/2 ? ((1<<12) | (2<<8) | '-') : vals[mode];
}

int add_cookie( long val, void *cook )
{
  if( CJar( 1, val, cook )!=CJar_OK )
  {
    Cconws( JARROOM );
    set_vex(1);
    Crawcin();
    return 0;
  }
  return 1;
}

void clear_all_mem(void)
{
  struct Memblk { struct Memblk *prev; long len; } *prev, *newmem;
  long l;

  prev = 0L;
  while( (l=(long)Malloc(-1L)) >= sizeof(struct Memblk) )
  {
    if( (newmem=(struct Memblk *)Malloc(l)) == 0 ) break;
    newmem->prev = prev;
    newmem->len = l;
    prev = newmem;
  }
  while( prev )
  {
    newmem = prev->prev;
    memset( prev, 0, prev->len );
    Mfree(prev);
    prev = newmem;
  }
}

BASPAG *process( void func() )
{
  BASPAG *bp, *b;

  bp = (BASPAG *)Pexec( 5, 0L, (void *)"", environ );
  if( (long)bp > 0 )
  {
    Mshrink( 0, bp, sizeof(BASPAG) );
    bp->p_hitpa = (char *)bp->p_lowtpa + sizeof(BASPAG);
    if( shel_setup() )
    {
      curapp->env = environ;
/*      bp->p_tbase = magic_shell;  redundant */
      b = curapp->basepage;
      *(char **)b->p_cmdlin = "GManager";	/* 004 */
      *(BASPAG **)(b->p_cmdlin+4) = bp;
      *(void (**)())(b->p_cmdlin+10) = func;
      b->p_tbase = (void *)run_acc;
      return bp;
    }
    else Mfree(bp);
  }
  return 0;
}

void get_falcon( int rez )
{
  switch( rez )
  {
    case 0+2:
      settings.falcon_rez = COL40|BPS4|STMODES;
      break;
    case 1+2:
      settings.falcon_rez = COL80|BPS2|STMODES;
      break;
    case 6+2:  	/* TT-high: go to ST-high */
    case 2+2:
      settings.falcon_rez = COL80|BPS1|STMODES;
      break;
    case 4+2:
      settings.falcon_rez = COL80|BPS4;
      break;
    case 7+2:
      settings.falcon_rez = COL40|BPS16;
      break;
  }
}

void fix_rez(void)
{
  int vga=0, rez=settings.boot_rez, d;

  if( vid<0x30000L )
  {
    d = Getrez()+2;
    if( vid==0x20000L ) vga = VGA_FALCON;
    if( d==2+2 )
    {
      if( !vga || rez==6+2 ) rez = 2+2;
    }
    else if( d==6+2 ) rez = 6+2;
    else if( !vga && rez!=0+2 && rez!=1+2 || rez==6+2 ) rez=1+2;
    get_falcon(rez);
  }
  else if( is_falc )
  {
    settings.falcon_rez = settings.falcon_rez&~(PAL|OVERSCAN|VGA_FALCON) |
        Vsetmode(-1)&(PAL|OVERSCAN|VGA_FALCON);
    switch( mon_type() )
    {
      case 0:
        rez=2+2;
        get_falcon(rez);
        break;
      case 1:
/*        if( rez != 0+2 && rez != 1+2 )	removed for 004
        {
          rez = 1+2;
          get_falcon(rez);
        } */
        break;
      case 2:
        vga = VGA_FALCON;
        break;
    }
    if( (settings.falcon_rez&(NUMCOLS|COL80))==(COL40|BPS1) )
        settings.falcon_rez |= COL80;
    if( vga && (settings.falcon_rez&(NUMCOLS|COL80))==(COL80|BPS16) )
        settings.falcon_rez &= ~COL80;
    d = settings.falcon_rez & NUMCOLS;
    if( (settings.falcon_rez&COL80) == COL40 )
      if( d==BPS8 ) rez = 7+2;
      else rez = 0+2;
    else switch(d)
    {
      case BPS1:
        rez = 2+2;
        break;
      case BPS2:
        rez = 1+2;
        break;
      case BPS4:
      case BPS8:
      case BPS16:
        rez = 4+2;
        break;
    }
  }
  settings.falcon_rez = settings.falcon_rez & ~VGA_FALCON | vga;
  settings.boot_rez = rez;
}

void set_screen(void)
{
  /* don't set new rez if using a Moniterm or other card */
  if( (unsigned long)Logbase() < 0xC00000L )
    if( is_falc ) xbios( 5, 0L, 0L, 3, settings.falcon_rez );
    else if( settings.boot_rez!=Getrez()+2 )  /* avoid wrap bug */
        Setscreen( (void *)-1L, (void *)-1L, settings.boot_rez-2 );
}

void stand_handles(void)
{
  int i, d;
  static char devs[6][5] = { "CON:", "CON:", "AUX:", "PRN:", "CON:", "CON:" };

  if( sversion==0x3000 )
    /* Initialize standard file handles */
    for( i=0; i<6; i++ )
    {
      Fforce( i, d=Fopen( devs[i], 0 ) );
      Fclose(d);
    }
}

void no_fonts(void)
{
  if( have_fonts )
  {
    _vst_font( 1, 0 );		/* added whole routine for this in 004 */
    vst_unload_fonts( vdi_hand, 0 );
    have_fonts = 0;
  }
}

void cdecl sigchild( long sig )
{
  int id;
  long l;
  APP *ap2;
  void terminate( int ret );
  static unsigned long ptic;

#ifdef MINT_DEBUG
  char buf[50];
/*  if( (int)sig!=SIGCHLD )*/
  {
    spf( buf, "%d: signal %lX\r\n", Pgetpid(), sig );
    Cconws(buf);
  }
#endif
  if( (int)sig==SIGCHLD && mint_preem )
  {
    if( (l=Pwait3( 3/*005: was 1*/, 0L )) != 0 && l != IEFILNF && (char)l != 0x7f/*005: stopped*/ )
    {
#ifdef MINT_DEBUG
      spf( buf, "Signal %lX\r\n", l );
      Cconws(buf);
#endif
      id = l>>16L;
      if( (int)l == (SIGKILL<<8) || (int)l == (SIGTERM<<8) ||
          (int)l == (SIGINT<<8) || (int)l == (SIGQUIT<<8) ) l = 0L;
      grab_curapp();
      for( ap2=app0; ap2; ap2=ap2->next )
        if( ap2->mint_id == id )
        {
          old_app = curapp;
          set_curapp(ap2);
          terminate( (int)l );
          if( old_app != ap2 ) curapp = old_app;
          lock_curapp = 0;
          if( ap2->parent && ap2->parent->old_sigchild ) (*(void cdecl (*)(long))ap2->parent->old_sigchild)(sig);	/* 005 */
          return;
        }
      __x_appl_term( curapp->id, (int)l, 0 );
      lock_curapp = 0;
    }
  }
  else if( !play_key && ptic<tic/*005*/ )
  {
    ptic = tic+20;		/* 005: all apps get sig, so prevent multiple ^Z's in play_key */
    l = *kbshift&0xf;
    if( !(l&0x8) ) l = 0xC;
    l <<= 16L;
    switch( (int)sig )
    {
      case SIGTSTP:	/* process as a ^~Z keypress */
        play_key = l | 0x2C1A;
        break;
      case SIGINT:	/* process as a ^~C keypress */
        play_key = l | 0x2E03;
        break;
      case SIGQUIT:	/* process as a ^~\ keypress */
        play_key = l | 0x2B1C;
        break;
    }
  }
}

void do_sig(void)
{
  SIGACTION act = { (void(*)( long sig ))&sigchild, 0L, SA_NOCLDSTOP };

  if( has_mint ) Psigaction( SIGCHLD, &act, 0L );
}

void fix_neo(void)
{
  LoadCookie *lc;

  if( CJar( 0, LOAD_COOKIE, &lc )==CJar_OK && (neo_ver=lc->ver)>=0x300 )
      lc->t2table[110-10] = -1;
}

/* 004: install cookies at start if !is_auto */
long old_cookie=0L;
G_COOKIE cookie = { GENEVA_VER, 0L, 0, &t2atbl, &t2xtbl, &g_vectors };
char *pdisp_temp;

void test_mint(void)	/* 004 */
{
  preempt = mint_preem = 	/* can't work right in coopr mode, so use preem whenever MiNT */
      has_mint = CJar( 0, MiNT_COOKIE, 0L ) == CJar_OK;
}

void open_vdi(void)
{
  if( is_auto ) {
    v_opnwk( work_in, &vdi_hand, work_out );
  }
  else {
    v_opnvwk( work_in, &vdi_hand, work_out );
  }
}

void initialize(void)
{
  int d, i, shge, work[57], desk_vdi, bad_get, dum, vdi0;
  static char mod[] = { 3, 2, 3, 3, 3,
                        11, 14, 11,
                        0,
                        11, 14, 11,
                        3, 1, 1, 3,
                        0,
                        3, 1, 1, 3,
                        3, 6, 2, 3,  0,  3, 6, 2, 3,  3 },
              vidmod[] = { -1, 0+2, 1+2, 2+2, 4+2, 6+2, 7+2 };
  char *m;
  OBJECT *o;
  char *p, *p2, *name;
  APP *ap, *ap2;
  _APPFLAGS *af;
  BASPAG *bp;
  G_COOKIE *new_cookie=&cookie;
  int (*func)();
  MSG msg;
  void buserr(void);
  extern int mall_mode;	/* memory.s */

  sversion = Sversion();
  test_mint();
  if( has_mint && sversion>=0x1900 ) mall_mode = (2<<4)|(1<<3);
  best_malloc = !has_mint;	/* 005 */
  stand_handles();
  proc_name = &cookie.process_name;
  proc_id = &cookie.apid;
  clip_ini = dflt_clip_ini;
  clip_it = dflt_clip;
  if( !is_auto )
  {
    if( CJar( 0, GENEVA_COOKIE, &old_cookie )==CJar_OK && old_cookie!=0 )
         x_appl_term( -1, 0, 0 );  /* make any resident copy go to sleep */
    vdi_hand = graf_handle( &char_w, &char_h, &cel_w, &cel_h );  /* trap to old AES */
  }
  vdi0 = vdi_hand;
  linea_init();
  set_vex(0);	/* must come before set_kbd */
  loadpath[0] = Dgetdrv()+'A';
  loadpath[1] = ':';
  Dgetpath(loadpath+2,0);
  strcat( loadpath, "\\" );
  environ = "\0";
  /* strcpy( scrap_dir, loadpath ); 004 */
  name = p = 0L;
  no_load = load_acc(1) < 0;
  x_settings( -1, -1, 0 );
  /* 004: used to install IDT cookie here; added !is_auto */
  if( !is_auto && !add_cookie( GENEVA_COOKIE, &new_cookie ) )
  {
    set_vex(1);
    return;
  }
  if( CJar( 0, _VDO_COOKIE, &vid ) != CJar_OK ) vid = 0L;
  else vid &= 0x00FF0000L;
  is_falc = vid==0x30000L;
  fix_neo();
  if( preempt )		/* 004 */
  {
    do_sig();
    if( (pdisp_temp = lalloc( 1500, -1 )) == 0 )
    {
      set_vex(1);
      return;
    }
    else pdisp_temp += 1500;
  }
  if( !no_load )
  {
    if( (p = (char *)lalloc(1024,-1)) != 0 ) shge=shel_get( p, 1024 );
    if( is_auto && p && shge && (p2=strstr(p,"#E"))!=0 )
    {
      i = atoi(p2+6);
      xscan( p2, "#E %x %x %x %x %h %h", &dum, &settings.boot_rez,
          &dum, &dum, (char *)&settings.falcon_rez, (char *)&settings.falcon_rez+1 );
      i = settings.boot_rez & 0xf;
      settings.boot_rez = i>=1 && i<=6 ? vidmod[i] : 1+2;
      if( !isdigit(*(p2+9)) ) settings.falcon_rez = Vsetmode(-1);
    }
    if( iargc > 1 ) name = iargv[1];
    else if( is_auto && p && shge && (name=strstr(p,"#Z"))!=0 )
    {
      if( (m=strchr( name+=6, '@' )) != 0 ) *m = 0;	/* 004: if */
    }
  }
  else
  {
    settings.boot_rez = Getrez()+2;
    settings.falcon_rez = Vsetmode(-1);
  }
  if( !name )
  {
    strcpy( exec_path, loadpath );
    strcat( exec_path, "*.*" );
    exec_name[0] = 0;	/* don't do shel_write() in magic_shell */
  }
  else
  {
    strcpy( exec_path, name );
    strcpy( exec_name, pathend(name) );
  }
  if( ienvp )		/* 005 */
      while( *ienvp ) _shel_write( 8, 1, 0, *ienvp++, 0L );
  if(p)
  {
    gem_cnf( 0, 0L );
    while( gem_cnf(1,p) > 0 )
      if( (p2=scan_cmd( p, "SETENV" )) != 0 ) _shel_write( 8, 1, 0, p2, 0L );
    gem_cnf( 2, 0L );
    lfree(p);
  }
  reset_butq();
  memcpy( fontinfo, dfontinfo, sizeof(fontinfo) );
  wcolor_mode = -1;	/* 004: prevent x_shel_get from setting colors */
  bad_get = x_shel_get( X_SHLOADSAVE, 0, 0L )<=-2;
  for( af=&flag0; af; af=af->next )
    if( !strcmp( af->flags->name, magic_flags.name ) ) break;
  if( !af ) x_appl_flags( 1, -1, &magic_flags );
  fix_rez();
  if( is_auto ) set_screen();
open_vdi:
  Supexec( (long (*)())set_kbd );
  if( !kbbuf )
  {
    Cconws( ANOMEM );
    Crawcin();
    set_vex(1);
    return;
  }
  cur_rez = Getrez()+2;
  work_in[0] = is_falc ? 5 : cur_rez;
  work_out[45] = settings.falcon_rez;
  Supexec( (long (*)())open_vdi );	/* 005: always in Super mode */
  if( !vdi_hand )
  {
    Cconws( NOVDI );
    Crawcin();
    set_vex(1);
    return;
  }
  vdi_reset();
  if( !old_vdi ) setexc( 0x88, my_vdi, &old_vdi, 0 );	/* 004 */
  work_out[0]++;
  work_out[1]++;
  vq_extnd( vdi_hand, 1, work );
  vplanes = work[4];
  v_bpp = test_rez();
  do_pall( app0, 0 );	/* 004: do it now because init_aph didn't. Depends on vplanes */
  if( vplanes>=8 ) color_mode = 3;
  else if( vplanes>=4 ) color_mode = 2;
  else if( vplanes>=2 ) color_mode = 1;
  else color_mode = 0;
  ind_col = act_col = bkgrnd_col = color_mode>=2 ? 0x1178 : 0x1170;
  wcolor_mode = color_mode>2 ? 2 : color_mode;
  dflt_wind = rs_trindex[WIND];		/* 004: dflt_wstates() needs this */
  dflt_wstates();
  vq_mouse( vdi_hand, &g_mb, &g_mx, &g_my );
  vex_butv( vdi_hand, my_butv, &old_butv );
  vex_motv( vdi_hand, my_motv, &old_motv );
  vex_timv( vdi_hand, my_timv, &old_timv, &ticcal );
  ticsec = 1000/ticcal;
  char_w = 8;
  char_h = work_out[1]>=400 ? 16 : 8;
  if( !new_rez ) set_dc(3);
  /* shel_get was here */
  if( (font_mode = cur_rez-2) > 8 ) font_mode = 8;	/* max was 9 before 004 */
  font_id = fontinfo[font_mode].font_id;
  font_scalable = 0;
  if( font_id != 1 && vq_gdos() )
  {
    char tempname[33];

    i = work_out[10] + vst_load_fonts( vdi_hand, 0 );
    have_fonts = 1;
    dum = 0;	/* 007 */
    while(i)
    {
      tempname[32] = 0;
      if( (d=/*007*/vqt_name( vdi_hand, i, tempname )) == font_id )
      {
        pfont_id =/*007*/ font_id = _vst_font( font_id, font_scalable=tempname[32] );
        pfont_mode = font_scalable ? GDOS_PROP : GDOS_BITM;	/* 007 */
        break;
      }
      else i--;	/* don't decrement if it succeeds! */
      if( d==X_SYSFONT ) dum = 1;	/* 007 */
    }
    if( pfont_id<0 && dum ) {	/* 007 */
      pfont_id = X_SYSFONT;
      pfont_mode = font_scalable ? GDOS_PROP : GDOS_BITM;
    }
    else if(!i)			/* 007 */
    {
      font_id = 1;
      no_fonts();
    }
  }
  /* 004: loop until >=25 rows */
  ptsiz = fontinfo[font_mode].point_size;
  for(;;)
  {
    ptsiz = _vst_point( ptsiz, work, work, &char_w, &char_h );
    if( ptsiz<=8 || work_out[1]/char_h >= 25 ) break;
    if( font_id != 1 )
    {
      font_id = 1;
      no_fonts();
      ptsiz = 10;
    }
    else ptsiz--;
  }
  cel_w = fontinfo[font_mode].gadget_wid + char_w;
  cel_h = fontinfo[font_mode].gadget_ht + char_h;
  if( pfont_id<0 ) _vst_font( X_SYSFONT, 0 );		/* 007: load proportional system font */
  if( pfont_id!=1 ) {		/* 007 */
    dflt_wind[WMOVE].ob_spec.tedinfo->te_font = dflt_wind[WINFO].ob_spec.tedinfo->te_font = pfont_mode;
    dflt_wind[WMOVE].ob_spec.tedinfo->te_junk1 = dflt_wind[WINFO].ob_spec.tedinfo->te_junk1 = pfont_id;
    dflt_wind[WMOVE].ob_spec.tedinfo->te_junk2 = dflt_wind[WINFO].ob_spec.tedinfo->te_junk2 = ptsiz;
  }
  menu_h = char_h + 3;
  user_menu();		/* always re-fix menu height */
  dflt_desk = dflt_desk0 = rs_trindex[DESK];
  dtree_cnt = DTREECNT;
  mag_menu = rs_trindex[MAGMENU];
  pman = rs_trindex[PROCMAN];	/* 005 */
  for( o=&dflt_wind[WCLOSE], m=mod, i=0; i<sizeof(mod); i++, o++ )
  {
    if( *m&1 ) o->ob_width = cel_w;
    if( *m&2 ) o->ob_height = i>=9 && i<=11 ? menu_h : cel_h;
    if( *m&4 ) o->ob_x = cel_w-1;
    if( *m++&8 ) o->ob_y = cel_h-1;
  }
  dflt_wind[WHSPLIT].ob_width = 5 * work_out[4] / work_out[3];
  dflt_wind[WVSPLIT].ob_height = 5 * work_out[3] / work_out[4];
  pull_siz.l = (long)work_out[0]*work_out[1]/8/2 * vplanes;
  if( (pull_buf.l = (long)lalloc( pull_siz.l, -1 )) == 0L ||
      (desktop = (Window *)lalloc( sizeof(Window), -1 )) == 0L )
  {
    Cconws( ANOMEM );
    Crawcin();
    set_vex(1);
    return;
  }
  memset( top_wind=desktop, 0, sizeof(Window) );
  desktop->outer.w = work_out[0];
  desktop->outer.h = work_out[1];
  desktop->working.y = menu_h;
  desktop->working.w = work_out[0];
  desktop->working.h = work_out[1]-menu_h;
  desktop->full = desktop->working;
  desktop->prev = desktop->outer;
  desktop->tree = dflt_desk;
  desktop->treeflag = -1;
  desktop->treecnt = DTREECNT;
  *(Rect *)&desktop->tree->ob_x = desktop->outer;
  desk_obj = 0;
  has_mouse = 0L;
  _graf_mouse( BUSYBEE, 0L, 0 );
  _graf_mouse( X_MRESET, 0L, 0 );
  draw_desk();
  fix_trees(0);
  do_about(0);
  new_rez = 0;
  did_unload = 0;
  if( bad_get )
  {
    x_shel_get( X_SHLOADSAVE, 0, 0L );	/* repeat the alert */
    bad_get = 0;
  }
  /* fix a bug in Falcon TOS which does not set the DEF_FONT correctly */
  if( ROM_ver>=0x400/*004*/ && Vdiesc->def_font != Vdiesc->cur_font )
  {
    if( Vdiesc->v_cel_ht==8 && Vdiesc->def_font->size!=9 )
    {
      i = 9;
      goto point;
    }
    if( Vdiesc->v_cel_ht==16 && Vdiesc->def_font->size!=10 )
    {
      i = 10;
point:for( d=0; d<4 && Vdiesc->font_ring[d]; d++ )
        if( Vdiesc->font_ring[d]->size==i )
        {
          Vdiesc->def_font = Vdiesc->font_ring[d];
          break;
        }
    }
  }
/*  in_disp_loop = 1;*/
  if( preempt )
  {
    all_sigs();
    Psignal( SIGCHLD, (void *)sigchilddisp );		/* catch loop termination */
    if( process( pdisp_init )!=0 )
    {
      *(int *)(curapp->basepage->p_cmdlin+22) = -10; /* Prenice default */
      if( go_proc() != 0 )
      {
        loop_id = curapp->mint_id;
        if( process( magic_init ) )
          if( go_proc() )
          {
            Psignal( SIGBUS, (void *)buserr );
            Psignal( SIGSEGV, (void *)buserr );			/* 005 */
            Pmsg( 0, 0x476E8000L|loop_id, &msg );	/* wait for loop to release control */
            Psignal( SIGSEGV, 0L );
            Pkill( loop_id, SIGKILL );			/* kill it */
            loop_id = Pgetpid();
            lock_curapp = 0;
          }
      }
    }
  }
  else if( process( magic_init ) ) start_magic( curapp->basepage );
  vex_butv( vdi_hand, old_butv, &func );
  vex_motv( vdi_hand, old_motv, &func );
  vex_timv( vdi_hand, old_timv, &func, &ticcal );
  for( i=X_SET_SHAPE+ARROW; i<=X_SET_SHAPE+X_UPDOWN; i++ )
    _graf_mouse( i, 0L, 0 );	/* free up mouse shapes */
  _graf_mouse( ARROW, 0L, 0 );
  del_acc_name(-2);
  no_fonts();
  if( !is_auto ) v_clsvwk( vdi_hand );
  else v_clswk( vdi_hand );
  Supexec( (long (*)())set_kbd );
  for( ap=app0; ap; )
  {
    i = ap->id;
    ap = (ap2=ap)->next;        /* multi-tasking friendly */
    lfreeall(i);
    lfree(ap2);
  }
  app0 = last_curapp = 0L;
  if( new_rez ) fix_trees(1);
  lfree(desktop);
  desktop = 0L;
  lfree((void *)pull_buf.l);
  has_desk = has_menu = has_mouse = 0L;
  if( new_rez )
  {
    exec_name[0] = 0;	/* don't do shel_write() in magic_shell */
    settings.boot_rez = new_rez;
    settings.falcon_rez = new_vid;
    fix_rez();
    set_screen();
    shel_rpath();
    no_load = load_acc(1) < 0;
    vdi_hand = vdi0;
    goto open_vdi;
  }
  lfreeall(-1);
  set_vex(1);
  CJar( 1, GENEVA_COOKIE, &old_cookie );	/* replace old cookie, or zero it out */
  if( old_cookie && !is_auto ) x_appl_term( -1, 0, 0 ); /* make any resident copy wake up */
  if( is_auto ) clear_all_mem();
}

APP *find_free_apno( int multi, int *apno )	/* moved for 004 */
{
  APP *aph2, *aph, *prev;

  if( (aph2=app0) != 0 )
  {
    *apno=0;
    /* apno=0 is reserved for first single-tasking app */
    if( !multi && app0->id!=0 ) aph2 = 0L;  /* only use ID 0 if avail */
    else
    {
      /* find first empty slot */
      for( prev=aph2, *apno=aph2->id; aph2 && *apno==aph2->id; (*apno)++, prev=aph2,
          aph2=aph2->next );
check:
      /* make sure there is not an app pending de-allocation with this id */
      for( aph=free_ap; aph; aph=aph->next )
        if( aph->id==*apno )
        {
          (*apno)++;
          for( ; aph2 && *apno==aph2->id; (*apno)++, prev=aph2, aph2=aph2->next );
          goto check;
        }
      aph2 = prev;
    }
  }
  else *apno = 1;
  return aph2;
}

int init_aph( int multi, int is_acc )
{
  APP *aph, *aph2;
  int apno;
  long sem;
  void get_vex( APP *ap );

  is_acc++;
  no_interrupt++;
  aph2 = find_free_apno( multi, &apno );
  if( (aph = (APP *)lalloc(sizeof(APP),-1)) == 0 )
  {
    no_memory();
    no_interrupt--;
    load_err = IENSMEM;
    return 0;
  }
  else
  {
    memset( aph, 0, sizeof(APP) );
    aph->id = apno;
    aph->parent_id = curapp ? curapp->id : -1;
    memcpy( &aph->flags, /* 004: never happens apno==-1 ? &magic_flags :*/
        &dflt_flags, sizeof(APPFLAGS) );
    if( !aph2 )
    {					/* first app in list */
      if( (aph->next = app0) != 0 )	/* if there was a first */
          app0->prev = aph;		/* point it back to me */
      app0 = aph;			/* set first to me */
    }
    else if( (aph->next = aph2->next) != 0 )	/* insert into list */
        aph->next->prev = aph;			/* if next, point to me */
    if( !multi )
      if( (aph->parent = curapp) != 0 )		/* if there is a parent */
      {
        aph->child = curapp->child;		/* just in case */
        curapp->child = aph;			/* I'm its child */
        if( update.i && curapp==has_update )	/* inherit update flags */
        {
          has_update = aph;
/*%          (has_update = aph)->update = curapp->update;	004
          aph->old_update = curapp->old_update; */
        }
      }
    if( (aph->prev = aph2) != 0 ) aph2->next = aph;
    if( apno==1 ) aph->has_wind = aph->ap_type = 1; /* screen manager */
    else
    {
      if( !multi && curapp/*005*/ ) aph->has_wind = curapp->has_wind;	/* 005: was aph2 */
      aph->ap_type = 2;
    }
    aph->waiting = -1;	/* initial value */
    if( vdi_hand>0 ) do_pall( aph, 0 );	/* 004 */
    if( !preempt ) get_vex(aph);	/* 004 */
#ifdef MINT_DEBUG
    test_msg( aph, "locked in init_aph" );
#endif
    if( aph2 ) lock_curapp = (apno^0xff) | 0x8000/*005*/;
    set_curapp(aph);
    no_interrupt--;
    return 1;
  }
}

BASPAG *acc_run( char *path )
{
  BASPAG *bp, *old;
  int id;
  long len;

  bp = (BASPAG *)Pexec( 3, path, (void *)"", environ );	/* 004: env was 0L */
  if( (long)bp > 0 )
  {
    Mshrink( 0, bp, len=bp->p_tlen+bp->p_dlen+bp->p_blen+sizeof(BASPAG) );
    bp->p_hitpa = (char *)bp->p_lowtpa + len;
    id = curapp->id;
    if( !init_aph( 1, 1 ) )
    {
      Mfree(bp);
      return 0L;
    }
    else
    {
      curapp->parent_id = id;
      curapp->ap_type = 4;
      set_exec( path, (char *)"", environ );
      new_flags();
      curapp->flags.flags.s.multitask = 1;	/* 005: always true */
      return bp;
    }
  }
  else load_err = (long)bp;
  return 0L;
}

int go_proc(void)
{
  long ret;

  if( mint_preem )
  {
    ret = (*(int cdecl (*)(BASPAG *bp))curapp->basepage->p_tbase)( curapp->basepage );
/**    ret = Pexec( 104, 0, curapp->basepage, 0 ); **/
    if( ret<0 ) load_err = (int)ret;
    else return ret;
  }
  else
  {
    in_t2 = 0;
/*    load_err = Pexec( 4, 0, curapp->basepage, 0 ); *//* only returns if err */
    load_err = (*(int cdecl (*)(BASPAG *bp))curapp->basepage->p_tbase)( curapp->basepage );
    in_t2 = 1;
  }
  return 0;
}

int go_acc( APP *old, BASPAG *bp )
{
  BASPAG *b;

  if( shel_setup() )
  {
    odd_ex();
    b = curapp->basepage;
    *(char **)b->p_cmdlin = curapp->path;	/* 004: so MiNT gets fname from Pexec */
    *(BASPAG **)(b->p_cmdlin+4) = bp;
    *(void (**)())(b->p_cmdlin+10) = acc_start;
    b->p_tbase = &run_acc;
    if( !go_proc() )
    {
      __x_appl_term( curapp->id, load_err, 0 );
      if( old ) set_curapp(old);
      else curapp = 0L;
      return 0;
    }
    return 1;
  }
  return 0;
}

void ac_close( int id )
{
  int buf[8];

  buf[0] = AC_CLOSE;
  buf[2] = 0;
  find_menu_id( buf, id );
  _appl_write( buf[1]=id, 16, buf, 0 );
}

void reopen_win( int id )
{
  int pl, min;
  Window *w;
  APP *ap;

  for( w=desktop->next, min=-1; w; w=w->next )
    if( w->place < min ) min = w->place;    /* find topmost */
  for( pl=-2; pl>=min; pl-- )
    for( w=desktop->next; w; w=w->next )
      if( w->place==pl )    /* find window(s) with this location */
        if( id<0 || w->apid==id )
        {
          for( ap=app0; ap && ap->id != w->apid; ap=ap->next );
          if( ap && !ap->asleep ) opn_wind(w);	/* reopen */
        }
}

void to_sleep( int mask, int sleep, APP *ap )
{
  Window *w;
  int newsleep;

  if( ap && (mask&ASL_USER || ap != curapp) )
  {
    newsleep = (ap->asleep&~mask) | sleep;
    if( ap->asleep!=newsleep )
    {
      if( ap->has_wind && newsleep && (multitask || !(mask&sleep&ASL_PEXEC)) )
      {
        for( w=desktop; (w=w->next)!=0; )
        {
          w->sleep_pl = 0;
          if( w->place!=-1 && w->apid==ap->id && (w->place>0 || !newsleep) )
            if( newsleep&ASL_USER || ap->ap_type!=4 )
            {
              w->sleep_pl = -w->place-1;
              free_rects( desktop, 1 );	/* 004 */
            }
            else close_del(w);
        }
        /* second pass, because close_wind() modifies place of all winds */
        for( w=desktop; (w=w->next)!=0; )
          if( w->sleep_pl && w->apid==ap->id )
          {
            if( (newsleep&ASL_USER) && (mask&ASL_USER) )
            {
              w->apid = curapp->id;	/* so no error */
              close_window( w->handle );
              w->apid = ap->id;
            }
            w->place = w->sleep_pl;
          }
      }
      if( ap->ap_type!=4 || mask&ASL_USER )
      {
        /* 005: moved Pkill into here so DA's won't be put to sleep */
        if( mint_preem && (ap->asleep!=0) != (newsleep!=0) && ap->mint_id>0/*005*/ )
            Pkill( ap->mint_id, newsleep ? SIGSTOP : SIGCONT );
        ap->asleep = newsleep;
        if( (mask&ASL_USER) )
          if( !(newsleep&ASL_USER) )
          {
            reopen_win(ap->id);
            switch_mouse( ap, 1 );
          }
          else
          {
            if( ap==has_mouse )
            {
              has_mouse = 0L;
              _graf_mouse( ARROW, 0L, 0 );
            }
            if( ap==has_desk )
            {
              /* no_desk_own();   removed for 004 */
              new_desk( -1, 0L );
            }
          }
      }
      else if( sleep ) ac_close(ap->id);
      unblock();	/* 004 */
    }
  }
}

void set_multi( int multi )
{
  APP *ap;
  Window *w, *w2;

  if( multi != multitask )
  {
    for( ap=app0; ap; ap=ap->next )
      if( !(ap->asleep&ASL_PEXEC) ) to_sleep( ASL_SINGLE, !multi, ap );
    if( !multi )
    {
      to_sleep( ASL_SINGLE, ASL_SINGLE, curapp->parent );
      for( w2=desktop->next; (w=w2)!=0; )		/* 005: close Apps tears */
      {
        w2 = w->next;
        if( w->dial_obj==IS_TEAR ) close_del(w);
      }
      place = 0;
      no_top = 0;
      top_wind = desktop;
    }
/*    redraw_all( &desktop->outer ); */
    multitask = multi;
    if( multi )
    {
      reopen_win(-1);
      new_desk( -1, 0L );
      top_menu();
    }
    else
    {
      no_desk_own(0);	/* 004: was equiv of 1 */
      switch_menu(curapp);
    }
  }
  else if( !multi )
  {
    no_desk_own(1);
    has_menu=0L;
  }
}

BASPAG *shel_context2( BASPAG *bp )
{
  BASPAG *out;

  if( preempt || has_mint ) return 0L;
  if( !bp ) bp = curr_bp;
  out = *bp_addr;
  *bp_addr = bp;
  return out;
}

int load_acc( int first )
{
  static int ret;
  static char start;
  BASPAG *bp, *old;
  APP *prev;
  static char temp[120], *ptr;
  static DTA acc_dta;
  long stack;

  if( first )
  {
    start = 1;
    last_curapp = curapp = 0L;	/* 004 */
    stack = Super(0L);	/* 004: set_curapp must be in Supervisor mode */
    init_aph( 0, 0 );
    Super( (void *)stack );	/* 004 */
    multitask = 1;
    strcpy( temp, loadpath );
    strcat( temp, "GENEVA" );
    set_exec( temp, "", _BasPag->p_env );
    if( (Kbshift(-1)&0xf) == 4 ) ret=1;   /* Control held */
    else ret = 0;
  }
  else if( !ret || ret==IENMFIL )
  {
    old = curr_bp;
    shel_context2( curr_bp = &magic_bp );
    shel_rpath();
    if( start )
    {
      start = 0;
      shel_envrn( &ptr, "ACCPATH=" );
      if( !ptr ) ptr="";	/* 004 */
      ret = 1;
    }
    if( ret )
    {
      strcpy( temp, "*.ACC" );
      if( !ptr || !_shel_find( 0L, temp, &ptr, 0L ) )
      {
        shel_context2( curr_bp = old );
        return 0;
      }
      strupr(temp);
      memcpy( &acc_dta, &shel_dta, sizeof(DTA) );
    }
    strcpy( pathend(temp), acc_dta.d_fname );
    shel_path(temp);
    shel_disp( acc_dta.d_fname, 1 );
    prev = curapp;
    bp = acc_run( temp );
    Fsetdta(&acc_dta);
    ret = Fsnext();
    shel_context2( curr_bp = old );
    if( bp )
    {
      curapp->start_end = 1;
      apps_initial++;
      if( !go_acc( prev, bp ) )  /* won't return if successful (unless MiNT) */
      {
        apps_initial--;
        return 0;
      }
    }
  }
  return( ret==1 ? -1 : (ret==0 || ret==IENMFIL) );
}

#define exec_os (long **)0x4fe

void get_kbs(void)
{
  SYSHDR *sys = *(SYSHDR **)0x4f2;
  long **exec;

  if( is_auto && !mint_preem )
  {
    /* search along the XBRA chain */
    exec = exec_os;
    while( *(*exec-3) == 0x58425241L )
    {
      if( *(*exec-2) == GENEVA_COOKIE )
      {
        Cconws( TWICEMSG );
        Crawcin();
        Pterm0();
      }
      exec = (long **)(*exec-1);
    }
    if( (long)*exec >= (long)sys->os_start &&
        ((long)*exec <= (long)sys->os_magic || (long)sys->os_magic < (long)sys->os_start ) )
    {
      old_exec = (long)*exec;
      (long)*exec = (long)&new_exec;
    }
    else
    {
      Cconws( OSMSG );
      Crawcin();
      Pterm0();
    }
  }
  if( (ROM_ver=sys->os_version)>0x0100 )
  {
    kbshift = (char *)sys->kbshift;
    bp_addr = sys->_run;
  }
  else
  {
    kbshift = (char *)0xE1BL;
    bp_addr = (BASPAG **)((sys->os_palmode>>1)==4 ? 0x873c : 0x602c);
  }
  if( _BasPag != *bp_addr )	/* running from PD.PRG */
  {
    is_PD = 1;   /* 006: removed PD_bp=*bp_addr */
    memcpy( &(_BasPag->p_resrvd0), &((*bp_addr)->p_resrvd0), 16+18 );
  }
}

int main( int argc, char *argv[], char *envp[] )
{
  int i;
  char temp[150];
  G_COOKIE *new_cookie=&cookie;

  appl_init();
  is_auto = _GemParBlk.global[0] == 0;
  if( (Kbshift(-1)&0xf) == 8 )
  {
    if( !is_auto ) Cconws( "\033E" );
    Cconws( NOINST );
    if( !is_auto ) Crawcin();
    return 0;
  }
  old_stack = &i;
  if( CJar( 0, CJar_cookie, 0L ) != CJar_OK )
  {
    DTA dta;
    char no_jar = 1;
    
    Fsetdta(&dta);
    has_mint = 1;	/* so Pexec works right, gets reset below */
    if( !Fsfirst("\\AUTO\\JAR*.PRG", 0x27) )
    {
      strcpy(temp, "\\AUTO\\");
      strcat(temp, dta.d_fname);
      if (!Pexec(0, temp, (void *)"\0", 0L) && CJar(0, CJar_cookie, 0L) == CJar_OK)
      {
        no_jar = 0;
      }
    }
    if (no_jar)
    {
      Cconws( NOJAR );
      Crawcin();
      return 0;
    }
  }
  test_mint();
/*  preempt = mint_preem = has_mint; dupl in test_mint */	/* Change me for PowerDOS */
  Supexec( (long (*)())get_kbs );
  memcpy( &magic_bp, _BasPag, sizeof(BASPAG) );
  /* 004: moved here. Must be after get_kbs */
  if( CJar( 0, IDT_cookie, &idt_fmt )!=CJar_OK )
  {
    idt_fmt = Supexec( get_idt_fmt );
    if( !add_cookie( IDT_cookie, (long *)&idt_fmt ) ) return 0;
  }
  if( is_auto && !add_cookie( GENEVA_COOKIE, &new_cookie ) ) return 0;
  if( is_auto && !mint_preem )
  {
    iargc=0;
    spf( temp, COPYRTM, ABOUT_VER_MAJOR, ABOUT_VER_MINOR, __DATE__ );
    Cconws( temp );
    Ptermres( _BasPag->p_tlen+_BasPag->p_dlen+_BasPag->p_blen+sizeof(BASPAG), 0 );
  }
  else
  {
    iargc = argc;
    iargv = argv;
    ienvp = envp;	/* 005 */
    initialize();
  }
  return(0);
}

int __appl_write( int ap_wid, int ap_wlength, int *ap_wpbuff )
{
  MSGQ *msg, *ptr;

    if( (msg = (struct Msg_q *)lalloc(sizeof(MSGQ),-1)) != 0 )
      if( (msg->buf = lalloc(ap_wlength,-1)) != 0 )
      {
        memcpy( msg->buf, ap_wpbuff, msg->len=ap_wlength );
        msg->app = ap_wid;
        msg->next = 0;
        msg->tic = tic;
        if( (ptr=msg_q) != 0 )
        {
          while( ptr->next ) ptr = ptr->next;
          ptr->next = msg;
        }
        else msg_q = msg;
        return(1);
      }
      else
      {
        lfree(msg);
        no_memory();
      }
    else no_memory();
  return 0;
}

int uappl_write( int ap_wid, int ap_wlength, void *ap_wpbuff )   /* 004 */
{
  fix_msg( curapp, (int *)ap_wpbuff, 1 );
  return _appl_write( ap_wid, ap_wlength, (int *)ap_wpbuff, 1 );
}

int _appl_write( int ap_wid, int ap_wlength, int *ap_wpbuff, int check )
{
  APP *ap;
  static unsigned char msg_list[] = { WM_UNTOPPED, WM_ONTOP, AP_TERM, AP_TFAIL,
      AP_RESCHG, SHUT_COMPLETED, RESCH_COMPLETED, AP_DRAGDROP,
      SH_WDRAW, CH_EXIT, 0 };	/* all these must be >0, <256 */

  if( ap_wpbuff[0] == AC_OPEN && ap_wpbuff[4]==-1 )
      return ac_open( 0L, ap_wid );     /* recursive! */
  for( ap=app0; ap; ap=ap->next )
  { /* write to id 0 in ST mode goes to actual id,
       or, in MT mode, when there is no app with id 0:
         to self when app is a PRG
         to app w/ top wind when src is a DA  (004) */
    if( !ap_wid && (!multitask || ap==curapp && app0->id/*no ST app*/) )
      if( curapp->ap_type==4 )
      {
        ap_wid = has_menu ? has_menu->id : app0->id;
        break;
      }
      else if( !ap->asleep && ap->ap_type==2 )
      {
        ap_wid = ap->id;
        break;
      }
    if( ap->id==ap_wid ) break;
  }
  if( !ap ) return 0;
  if( check && !ap->flags.flags.s.AES40_msgs &&
      strchr((char *)msg_list,ap_wpbuff[0]) ) return 0;
  if( ap->asleep && ap_wpbuff[0]!=SH_WDRAW && ap_wpbuff[0]!=CH_EXIT/*005*/ ) return 0;
  if( ap_wpbuff[0] == WM_REDRAW && redraw_window(	/* 004: used to return 0 if WM_REDRAW */
      ap_wpbuff[3], (Rect *)&ap_wpbuff[4], 0, 1 ) ) return 1;
  return __appl_write( ap_wid, ap_wlength, ap_wpbuff );
}

void search_data( char *name, int data[2] )
{
  data[0] = curapp->ap_srch->ap_type;
  data[1] = curapp->ap_srch->id;
  if( name ) strcpy( name, curapp->ap_srch->dflt_acc+2 );
}

int _appl_search( int mode, char *name, int data[2] )
{
  switch(mode)
  {
    case X_APS_CHEXIT:
      strcpy( name, lastapp_name );
      data[0] = lastapp_type;
      data[1] = lastapp_id;
      return 1;
    case 0:
      curapp->ap_srch = app0;
      goto next;
    case 2:
      curapp->ap_srch = shell_app ? shell_app : app0;
    case 1:
next: if( !curapp->ap_srch ) return 0;
      if( mode != 2 )
        while( curapp->ap_srch->asleep&ASL_PEXEC )
          if( (curapp->ap_srch = curapp->ap_srch->next) == 0 ) return 0;
      search_data( name, data );
      curapp->ap_srch = mode==2 ? 0 : curapp->ap_srch->next;
      return 1;
    case X_APS_CHILD0:
      curapp->ap_srch = app0;
    case X_APS_CHILD:
      if( !curapp->ap_srch ) return 0;
      while( curapp->ap_srch->asleep&ASL_PEXEC || curapp->ap_srch->parent_id!=curapp->id )
        if( (curapp->ap_srch = curapp->ap_srch->next) == 0 ) return 0;
      search_data( name, data );
      curapp->ap_srch = curapp->ap_srch->next;
      return 1;
  }
  return 0;
}

void del_msg( MSGQ *msg )
{
  MSGQ *m;

  lfree( msg->buf );
  if( msg_q==msg ) msg_q = msg->next;
  else
  {
    for( m=msg_q; m->next!=msg; m=m->next );
    m->next = msg->next;
  }
  lfree(msg);
}

void del_all_msg( int type, APP *ap )
{
  MSGQ *msg, *msg2;

  for( msg=msg_q; msg; msg=msg2 )               /* clear old msgs */
  {
    msg2 = msg->next;                   /* get next before unlinking */
    if( ap->id == msg->app && (type<0 || *(int *)(msg->buf)==type) ) del_msg(msg);
  }
}

void close_all( APP *ap, int all )
{
  Window *w, *w2;	/* added look ahead for 004 */

  for( w2=desktop->next; (w=w2)!=0; )
  {
    w2 = w->next;
    if( !ap && w->place>=-1 || ap && w->apid==ap->id )  /* 004: added ap && */
      if( all || w->place>=0 ) close_del(w);	/* 004: was >= -1 */
  }
}

int appl_exit(void)
{
  APP *ap;
  int x;

  if( curapp->no_evnt != -1 )
  {
    curapp->no_evnt = -1;
    if( !multitask )
    {
      for( ap=app0; ap; ap=ap->next )
        if( ap!=curapp && ap->ap_type==4 ) ac_close(ap->id);
      close_all( 0L, 0 );
    }
    else close_all( curapp, 0 );
    if( has_mouse == curapp ) _graf_mouse( ARROW, 0L, 1 );
    del_all_msg( -1, curapp );
  }
  if( curapp==ddesk_app )
  {
    ddesk_app = 0L;
    set_dfltdesk( dflt_desk0, DTREECNT );
  }
  if( curapp->id==shgp_app ) close_sh(1);
  if( curapp==recorder ) recorder=0L;
  if( curapp==player ) no_player();
  if( curapp==has_fmd_update )
  {
    _wind_update( END_UPDATE );
    has_fmd_update = 0L; 	/* 004 */
  }
  return(1);
}

int ap_ptsiz( int id, char scale, int pt )	/* 004 */
{
  int temp[10];

  _vst_font( id, scale );
  _vst_point( pt, temp, temp, temp, temp );
  vqt_attributes( vdi_hand, temp );
  return temp[7];
}

int _appl_getinfo( int num, int *out )
{
  char name[33];

  memset( out, 0, 4*2 );
  switch(num)
  {
    case 0:	/* default font info */
      *out++ = ap_ptsiz( font_id, font_scalable, ptsiz );	/* 004 */
      *out++ = font_id;
      *out++ = font_scalable;
      break;
    case 1:	/* small font info */
      *out++ = ap_ptsiz( 1, 0, 8 );	/* 004 */
      *out = vqt_name( vdi_hand, 1, name );
      break;
    case 2:	/* graphics info */
      *out++ = cur_rez;
      *out++ = 16;	/* AES object colors */
      *out++ = 1;	/* color icons? */
      *out = 1;		/* extended RSC files? */
      break;
    case 3:		/* language=0 */
      *out = AES_LANG;
      break;
    case 4:
      *out++ = preempt;	/* preemptive multitasking */
      *out++ = has_mint;/* can convert mint id's in appl_find() */
      *out++ = 1;	/* appl_search() OK */
      *out = 1;		/* rsrc_rcfix() OK */
      break;
    case 5:
      /* objc_xfind() not OK */
      /* next word is reserved */
      /* menu_click() not OK */
      /* shel_rdef/wdef not OK */
      break;
    case 6:
      *out++ = 1;	/* -1 is OK for appl_read() */
      *out++ = 1;	/* -1 to shel_get() is OK */
      *out = 1;		/* -1 to menu_bar() is OK */
      /* no MENU_INSTL during menu_bar() */
      break;
    case 7:	/* reserved */
      break;
    case 8:
      *out++ = 1;	/* graf_mouse modes 258-260 OK */
      *out = 1;		/* mouse form per application */
      break;
    case 9:
      *out++ = 1;	/* submenus OK */
      *out++ = 1;	/* popup menus OK */
      /* no scrollable menus */
      *++out = 1;	/* MN_SELECTED returns tree info */
      break;
    case 10:		/* shel_write */
      *out++ = (SHD_ENVIRON|SHD_DFLTDIR|SHD_PRENICE|SHD_PSETLIM) /* fixed for 004 */|	/* bits supported */
      	       SHW_SENDTOAES;	/* max mode */
      *out++ = !multitask;	/* mode 0:  0=launch, 1=cancels */
      *out++ = !multitask;	/* mode 1:  0=immediate, 1=on quit */
      *out = 1;		/* ARGV supported */
      break;
    case 11:
      *out = 0x1FF;	/* wind_get/set types */
      /* reserved */
      *(out+2) = 3;	/* Bottomer|Iconifier */
      *(out+3) = 1;	/* wind_update check and set */
      break;
    case 12:
      *out = 0x3ff;	/* messages supported */
      *(out+2) = 1;	/* WM_ICONIFY gives coords */
      break;
    case 13:		/* objects */
      *out++ = 1;	/* 3D objects */
      *out++ = 1;	/* MTOS 1.01 objc_sysvar supported */
      *out = 1;		/* can use GDOS fonts in TEDINFOs */
      break;
    case 14:
      *out = 1;		/* flying dialogs */
      /* no keyboard tables */
      /* last cursor position not returned */
      /* reserved */
      break;
    default:
      return 0;
  }
  return 1;
}

void setfont( int val, int *i )
{
  if( val!=-1 ) *i = val==-2 ?
      *(int *)((long)i-(long)&fontinfo[0]+(long)&dfontinfo) : val;
}

int _x_appl_font( int getset, XFONTINFO *info )
{
  if( getset==0 ) memcpy( info, &fontinfo[font_mode], sizeof(XFONTINFO) );
  else if( getset==1 )
  {
    setfont( info->font_id, &fontinfo[font_mode].font_id );
    setfont( info->point_size, &fontinfo[font_mode].point_size );
    setfont( info->gadget_wid, &fontinfo[font_mode].gadget_wid );
    setfont( info->gadget_ht, &fontinfo[font_mode].gadget_ht );
  }
  return 1;
}

void all_sigs(void)
{
  Psignal( SIGTSTP, (void *)sigchild );
  Psignal( SIGINT, (void *)sigchild );
  Psignal( SIGQUIT, (void *)sigchild );
}

int app_init( int *globl )
{
  int i;
  unsigned long in, out;
  APP *ap = curapp;
  void (*dum)();
  extern void otrts();

  if( !strcmp( ap->dflt_acc+2, "NEODESK " ) && neo_ver < 0x406 ||
      !strcmp( ap->dflt_acc+2, "NEO_CLI " ) ) ap->neo_le005 = 1;	/* 007 */
  if( has_mint )
  {
    all_sigs();
    if( !curapp->basepage )		/* 005 */
    {
      old_term = otrts;
      setexc( 0x408, my_term, &dum, 0 );
    }
/*    if( !ap->old_sigchild )	use my_term instead
      if( (ap->old_sigchild = Psignal( SIGCHLD, sigchild )) == (void *)sigchild )
          ap->old_sigchild = 0L;  */
  }
  retrap_la();
  /* hack to fix up cleared-out parent pointer for DA's */
/*  if( !bp->p_parent ) bp->p_parent = ap->basepage;  terminate only */
  globl[0] = INIT_VERSION;
  globl[1] = ap->flags.flags.s.multitask ? -1 : 1;
  globl[2] = ap->id;
  globl[10] = vplanes;
  for( out=0, i=33, in=Drvmap(); --i; )
  {
    out <<= 1;
    out |= in&1;
    in >>= 1;
  }
  *(long *)(globl+13) = out;
  _menu_register( -2, 0L );
  return(globl[2]);
}

int _appl_read( int id, int len, int *buf )
{
  curapp->apread_cnt = len;
  curapp->apread_id = id;
  curapp->buf = buf;
  curapp->type = MU_MESAG;
  return id==-1 ? 0 : 1;	/* 004: default to 0 if id==-1 */
}

#pragma warn -aus
int app_find( char *name )
{
  APP *ap;
  int foo;

  if( !name ) return curapp->id;
  if( (int)((long)name>>16) == -1 )
  {
    if( !has_mint ) return -1;
    for( ap=app0; ap; ap=ap->next )
      if( ap->mint_id == (foo=(int)name) ) return(ap->id);	/* 005: avoid cmpa.w compiler bug by storing in foo */
    return -1;		/* 005 */
  }
  if( (int)((long)name>>16) == -2 )
  {
    if( !has_mint ) return -1;
    for( ap=app0; ap; ap=ap->next )
      if( ap->id == (foo=(int)name) ) return(ap->mint_id);	/* 005: avoid cmpa.w compiler bug */
    return -1;		/* 005 */
  }
  if( !strcmp(name,"?AGI") ) return 0;		/* 006 */
  for( ap=app0; ap; ap=ap->next )
    if( !strncmp(ap->dflt_acc+2,name,8) && !ap->asleep ) return(ap->id);
  return(-1);
}
#pragma warn +aus

int _appl_yield(void)
{
  extern char block_app;

  block_app = 1;	/* 005: make sure dispatch is called */
  return(1);
}

int x_appl_trecord( void *mem, int count, KEYCODE *cancel, int mode )
{
  if( count )
  {
    if( recorder ) return 0;
    recnum = count;
    recbuf = recbuf0 = mem;
    rec_tic = tic;
    if( cancel ) rec_stop = *cancel;
    else *(long *)&rec_stop = 0L;
    recmode = (char) mode;
    rec_mx = rec_my = -1;
    recorder = curapp;
    _record( 3, (long)(rec_shift = *kbshift)<<16 );
  }
  else recorder = 0L;
  return !mode ? count : 1;
}

int x_set_curapp( int id )
{
  static APP *old;
  APP *ap;

  if( id>=0 )
  {
    if( (ap=find_ap(id)) == 0 ) return 0;
    lock_curapp = (lock_curapp+0x1000)|0x8000;	/* 005: used to be ++ */
    old = curapp;
    set_curapp(ap);
  }
  else
  {
    set_curapp(old);
    if( (unsigned)(lock_curapp -= 0x1000) == (unsigned)0x8000 ) lock_curapp=0;	/* 005: used to be -- */
  }
  return 1;
}

void no_player(void)
{
  int (*dum)();

  if( player )
  {
    player = 0L;
    install_key(1);
    vex_curv( vdi_hand, (int (*)())play_curv, &dum );
    vex_motv( vdi_hand, (int (*)())play_motv, &dum );
  }
}

int x_appl_tplay( void *mem, int num, int scale, int mode )
{
  if( num )
  {
    if( player ) return 0;
    playnum = num;
    playbuf = mem;
/*    playrate = scale>=20 ? 0 : (20-scale-1)/2; */
    playrate = !scale ? 50 : 50/scale;
    playmode = (char) mode;
    vex_curv( vdi_hand, (int (*)())null_mouse, (int (**)())&play_curv );
    vex_motv( vdi_hand, (int (*)())null_mouse, (int (**)())&play_motv );
    install_key(0);
    play_tic = tic;
    player = curapp;
  }
  else no_player();
  return 1;
}

void ring_bell(void)
{
  Bconout(2,7);
}

void test_unload(void)
{
  APP *ap2;
  MSGQ *msg, *msg2;

  if( unload )
  {
    for( ap2=app0; ap2; ap2=ap2->next )
      if( (unload<0 && ap2->ap_type!=1 || ap2->ap_type == unload) && ap2!=unloader && !ap2->child &&
          !(ap2->ap_msgs&1) )	/* 004: no timeout if understands AP_TERM */
      {
        for( msg2=msg_q; (msg=msg2)!=0; )
        {
          msg2 = msg2->next;
          if( msg->app == ap2->id )
            if( tic-msg->tic > ticsec*3 )
            {
              del_msg(msg);  /* delete after 3 sec */
              ring_bell();
            }
            else return;        /* make sure no pending msgs */
        }
        if( tic-term_time < ticsec*UNLOAD_TIME ) return;  /* x secs after last term */
        term_time = 0L;
        if( mint_preem )
        {
          ap2->ap_type = -1;
          ap2->asleep = ASL_USER;
          Pkill( ap2->mint_id, SIGTERM );
          if( (lock_curapp&0x7ff) == ap2->mint_id ) lock_curapp = 0;	/* 005 */
/*          set_curapp(ap2);
          _x_appl_term( ap2->id, 0, 0, 1 ); */
          return;
        }
        else
        {
          set_curapp(ap2);
          *bp_addr = *(BASPAG **)(ap2->stack+4);  /* basepage from stack */
          Dsetpath("\\");	/* maybe free up memory pool blocks? */
          in_t2 = 0;
          Pterm0();
        }
      }
    if( unloader ) newtop( unload_type, 1, unloader->id );
    unload = 0;
    did_unload++;
  }
}

MDESC *find_block( void *addr )
{
  MDESC *m;

  if( (m=curapp->mem_first)!=0 )
  {
    while( m && m->addr != addr ) m = m->next;
    return m;
  }
  return 0;
}

long memory( int mode, void *ptr )
{
  MDESC *m;
  void *addr;
  long len, ret, lim;
  char limit, clear;

  clear=curapp->flags.flags.s.clear_memory;
  if( (limit=curapp->flags.flags.s.limit_memory)==0 && !clear ) return 1L; /* remove if small_malloc is ever fixed */
  ret = 1L;
  switch( mode )
  {
    case 0:	/* Mxalloc */
      mode = *((int *)ptr+2) & 3;	/* 007: &3 */
      goto alloc;
    case 1:	/* Malloc */
      mode = ((*bp_addr)->p_resrvd0 & 4) ? 3 : 0;
alloc:len = *(long *)ptr;
      lim = curapp->flags.flags.s.mem_limit*1024L - curapp->mem_used;
      if( len<0 )
        if( (ret = (long)Mxalloc(len,mode)) > lim && limit ) return lim;
        else return ret;
      else if( limit && len > lim ) return 0L;
/*      else if( (ret = (long)small_malloc(len,mode)) != 0 )  doesn't work */
      else if( (ret = (long)Mxalloc(len,mode)) != 0 )
      {
        if( limit && (m=(MDESC *)lalloc(sizeof(MDESC),curapp->id)) != 0 )
        {
          m->next = curapp->mem_first;
          m->prev = 0L;
          m->len = len;
          m->addr = (void *)ret;
          curapp->mem_used += len;
          curapp->mem_first = m;
        }
        if( clear ) memset( (void *)ret, 0, len );
      }
      break;
    case 2:	/* Mfree */
      if( !limit ) return 1L;
      addr = *(void **)ptr;
      if( (ret=Mfree(addr))==0L && (m=find_block(addr))!=0 )
      {
        if( m->prev ) m->prev->next = m->next;
        else curapp->mem_first = m->next;
        if( m->next ) m->next->prev = m->prev;
        if( (curapp->mem_used -= m->len) < 0L ) curapp->mem_used = 0L;
        lfree(m);
      }
      break;
    case 3:	/* Mshrink */
      if( !limit ) return 1L;
      addr = *(void **)ptr;
      len = *((long *)ptr+1);
      if( (ret=Mshrink( 0, addr, len ))==0L && (m=find_block(addr))!=0 )
      {
        if( (curapp->mem_used -= m->len-len) < 0L ) curapp->mem_used = 0L;
        m->len = len;
      }
  }
  return ret;
}

#ifdef MINT_DEBUG
void test_msg( APP *ap, char *msg )
{
  char buf[100], l;
  static int row, off=1;

/*  if( strcmp( ap->dflt_acc+2, "NEODESK " ) ) return;*/
  if( (l=Kbshift(-1)&0xF) == 0xF ) off = 1;
  else if( l==0xD ) off = 0;
  if( off ) return;
  if( (l=lock_curapp)==0 ) lock_curapp = -1;
  if( !row ) Cconws( "\033H\r\n\r\n" );
  spf( buf, "\033f%s: %s. noe=%d st=%X sp=%X\033K\033e\r\n",
      ap->dflt_acc+2, msg, ap->no_evnt, ap->stack, buf );
  Cconws( buf );
  if( ++row==18 ) row=0;
  lock_curapp = l;
}
#endif

void fix_mintid( APP *ap )	/* 005 */
{
  DTA *old, new_dta;
  char *p;
  int id2, i;
  static char n[] = "U:\\PROC\\XXXXXXXX.Y";

  old = Fgetdta();
  Fsetdta(&new_dta);
  strcpy( n+8, ap->dflt_acc+2 );
  if( (p=strchr(n,' ')) != 0 ) strcpy( p, ".*" );
  else strcat( n, ".*" );
  id2 = -1;
  if( !Fsfirst( n, 0x27 ) ) {
    do {
      if ((p = strchr(new_dta.d_fname, '.')) != 0 && (i = atoi(p + 1)) > id2 &&
          app_find((char *) (0xFFFF0000L | i)) < 0)
        id2 = i;
    }
    while( !Fsnext() );
  }
  if( id2>0 )
  {
    ap->mint_id = id2;
    if( ap->asleep ) Pkill( id2, SIGSTOP );	/* 006 */
  }
  Fsetdta(old);
}

int wait_id( int id )
{
  APP *ap;

  for( ap=app0; ap; ap=ap->next )
  {
    if( ap->mint_id <= 0 ) fix_mintid(ap);	/* 005 */
    if( ap->mint_id == id )
    {
#ifdef MINT_DEBUG
      if( lock_curapp ) test_msg( curapp, "locked in get" );
#endif
/*      wait_curapp++;*/
/**      while( lock_curapp ) Syield();	005: grab */
      grab_curapp();
/*      wait_curapp--; */
      old_app = curapp;
      set_curapp(ap);
      lock_curapp = 0;	/* 005: always called in supervisor mode, so this is OK */
#ifdef MINT_DEBUG
      test_msg( ap, "switched" );
#endif
      return 1;
    }
  }
  return 0;
}

int get_curapp(void)
{
  int id;

  if( !preempt ) return 1;
  if( !app0 || app0->id==1/*005*/ && app0->waiting<0 || lock_curapp && curapp->waiting<0 ) return 0;
  id = Pgetpid();
  if( id==loop_id ) return 0;
  return wait_id(id);
/*  for(;;)
  {
    if( wait_id(id) ) return 1;
#ifdef MINT_DEBUG
    Crawio(id+'0');
    test_msg( curapp, "yield in get_curapp" );
    if( Bconstat(2) ) breakpoint();
#endif
    Syield();
  } */
}

void get_vex( APP *ap )
{
  long *l, *v;

  v = (long *)0x8;
  l = ap->vex;
  *l++ = *v++;
  *l++ = *v++;
  *l++ = *v++;
  *l++ = *v++;
  *l++ = *v++;
  *l++ = *v++;
  *l++ = *v;
  v = (long *)0x404;
  *l++ = *v++;
  *l = *v;
}

void copy_vex( APP *ap )
{
  long *l, *v;

  if( last_curapp ) get_vex( last_curapp );
  v = (long *)0x8;
  l = ap->vex;
  *v++ = *l++;
  *v++ = *l++;
  *v++ = *l++;
  *v++ = *l++;
  *v++ = *l++;
  *v++ = *l++;
  *v = *l++;
  v = (long *)0x404;
  *v++ = *l++;
  *v = *l;
}

void set_proc( APP *ap )
{
  *proc_name = (char *) (ap ? ap->dflt_acc + 2 : "");
  *proc_id = ap ? ap->id : -1;
/*  if( !ap ) breakpoint();*/
}

void set_curapp( APP *ap )
{
  if( !ap ) last_curapp = 0L;		/* 004 */
  else
  {
    if( !preempt ) copy_vex(ap);	/* 004 */
    curstack = &(last_curapp=curapp=ap)->stktop;
  }
  set_proc(ap);
}

void set_oldapp(void)
{
  if( preempt ) set_curapp(old_app);
}

#ifdef DEBUG_ON
void debug_alert( char *name, char *msg, int i )
{
  char temp1[150], temp2[150];

  if( *name )
  {
    spf( temp1, msg, i );
    spf( temp2, DEBUG_STR, name, temp1, curapp->dflt_acc+2, curapp->id );
    if( _form_alert( 2, temp2 ) == 1 ) *name = 0;
  }
}
#endif
