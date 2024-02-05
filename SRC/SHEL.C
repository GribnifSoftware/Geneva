#include "new_aes.h"
#include "string.h"
#include "win_var.h"
#include "win_inc.h"
#include "tos.h"
#include "ierrno.h"
#include "xwind.h"
#include "debugger.h"
#define _SHELL
#ifdef GERMAN
  #include "german\wind_str.h"
#else
  #include "wind_str.h"
#endif
#include "dragdrop.h"

#define ENVBUFSIZ	1024

/*#define X_DRAGDROP	0xE300
/********************* shel_write (mode 9) *********************/
#define X_SHW_DRAGDROP	0x4000		/* Can accept X_DRAGDROP message */ */

unsigned int env_buflen;
TEDINFO blankted = { 0L, "", "", IBM, 0, TE_CNTR, 0x1180, 0, -1, 12, 1 };
OBJECT blankdesk[] = { { -1, 1, 1, G_BOX, 0, 0, (long)((4<<4)|3) },
                       { 0, -1, -1, G_BOXTEXT, 32, 0, (long)&blankted } };
char sw_path[120], _sw_tail[128+120], *sw_tail=&_sw_tail[0],
    *sw_env="\0", sw_acc, *sw_hlptopic, *sw_hlpfile;
int sw_gr, sw_ex, sw_argv, sw_hlpsens;
SHWRCMD shwrcmd;

#if 0   /* Unused code */
static char pipename[] = "U:\\PIPE\\DRAGDROP.AA";
static long oldpipesig;

/* Code for originator */

/*
 * create a pipe for doing the drag & drop,
 * and send an AES message to the receipient
 * application telling it about the drag & drop
 * operation.
 *
 * Input Parameters:
 * apid:	AES id of the window owner
 * winid:	target window (0 for background)
 * msx, msy:	mouse X and Y position
 *		(or -1, -1 if a fake drag & drop)
 * kstate:	shift key state at time of event
 *
 * Output Parameters:
 * exts:	A 32 byte buffer into which the
 *		receipient's 8 favorite
 *		extensions will be copied.
 *
 * Returns:
 * A positive file descriptor (of the opened
 * drag & drop pipe) on success.
 * -1 if the receipient doesn't respond or
 *    returns DD_NAK
 * -2 if appl_write fails
 */

int ddcreate( int msg[8] )
{
	int fd, i;
	long fd_mask;
	char c;
	char exts[32];

	pipename[17] = pipename[18] = 'A';
	fd = -1;
	do {
		pipename[18]++;
		if (pipename[18] > 'Z') {
			pipename[17]++;
			if (pipename[17] > 'Z')
				break;
		}
/* FA_HIDDEN means "get EOF if nobody has pipe open for reading" */
		fd = Fcreate(pipename, FA_HIDDEN);
	} while (fd == IEACCDN);

	if (fd < 0) return 0;

/* construct and send the AES message */
	msg[7] = (pipename[17] << 8) | pipename[18];
	i = _appl_write(msg[2], 16, msg, 0);
	if (i == 0) return 0;

/* now wait for a response */
	fd_mask = 1L << fd;
	i = Fselect(DD_TIMEOUT, &fd_mask, 0L, 0L);
	if (!i || !fd_mask) {	/* timeout happened */
abort_dd:
		Fclose(fd);
		return 0;
	}

/* read the 1 byte response */
	i = Fread(fd, 1L, &c);
	if (i != 1 || c != DD_OK) {
		goto abort_dd;
	}

/* now read the "preferred extensions" */
	i = Fread(fd, DD_EXTSIZE, exts);
	if (i != DD_EXTSIZE) {
		goto abort_dd;
	}

	oldpipesig = (long)Psignal(SIGPIPE, (void *)SIG_IGN);
	return 1;
}

/*
 * see if the receipient is willing to accept a certain
 * type of data (as indicated by "ext")
 *
 * Input parameters:
 * fd		file descriptor returned from ddcreate()
 * ext		pointer to the 4 byte file type
 * name		pointer to the name of the data
 * size		number of bytes of data that will be sent
 *
 * Output parameters: none
 *
 * Returns:
 * DD_OK	if the receiver will accept the data
 * DD_EXT	if the receiver doesn't like the data type
 * DD_LEN	if the receiver doesn't like the data size
 * DD_NAK	if the receiver aborts
 */

int ddstry( int fd, char *ext, char *name, long size )
{
	Word hdrlen, i;
	char c;

/* 4 bytes for extension, 4 bytes for size, 1 byte for
 * trailing 0
 */
	hdrlen = 9 + strlen(name);
	i = Fwrite(fd, 2L, &hdrlen);

/* now send the header */
	if (i != 2) return DD_NAK;
	i = Fwrite(fd, 4L, ext);
	i += Fwrite(fd, 4L, &size);
	i += Fwrite(fd, (long)strlen(name)+1, name);
	if (i != hdrlen) return DD_NAK;

/* wait for a reply */
	i = Fread(fd, 1L, &c);
	if (i != 1) return DD_NAK;
	return c;
}

/*
 * close a drag & drop operation
 */
void ddclose( int fd )
{
  Psignal(SIGPIPE, (void *)oldpipesig);
  Fclose(fd);
}
#endif    /* Unused code */

char *pathend( char *path )
{
  char *ptr;

  if( (ptr = strrchr(path,'\\')) != 0 ) return(++ptr);
  return(path);
}

#define SHSTLEN 1024L

int shel_setup(void)
{
  BASPAG *old;
  char *stack, *ptr, *ptr2, *env=0L;

  if( (stack = lalloc(SHSTLEN,curapp->id)) == 0 )
  {
    load_err = IENSMEM;
    return 0;
  }
  if( sw_argv )
  {
    sw_argv=0;
    for( ptr=sw_env; *ptr; ptr+=strlen(ptr)+1 );
    if( !*sw_tail ) *(sw_tail+1) = 0;
    if( (env = lalloc( strlen(sw_tail+1)+ptr-sw_env+strlen(sw_path)+9, -1 )) != 0 )
    {
      memcpy( env, sw_env, ptr-sw_env );
      ptr = ptr-sw_env+env;
      strcpy( ptr, "ARGV=" );
      strcpy( ptr+=6, sw_path );
      ptr += strlen(sw_path)+1;
      for( ptr2=sw_tail+1; *ptr2; )
        if( *ptr2==' ' )
        {
          *ptr++ = 0;
          while( *++ptr2==' ' );
        }
        else *ptr++ = *ptr2++;
      *ptr = *ptr++ = 0;
      *sw_tail = '\x7f';
      sw_env = env;
    }
  }
  old = (BASPAG *)Pexec( 5, 0L, "", sw_env );
  if( env ) lfree(env);
  if( (long)old > 0 )
  {
    old->p_bbase = stack;
    old->p_blen = SHSTLEN;
    *(char **)old->p_cmdlin = sw_path;
    *(char **)(old->p_cmdlin+4) = sw_tail;
    *(int *)(old->p_cmdlin+8) = curapp->id;
    *(int **)(old->p_cmdlin+14) = &curapp->mint_id;
    *(long *)(old->p_cmdlin+18) = -1L;  /* Psetlimit default */
    *(int *)(old->p_cmdlin+22) = 0L;	/* Prenice default */
    curapp->basepage = old;
    return 1;
  }
  load_err = (int)old;
  return 0;
}

void pad_acc_name( char *s2 )
{
  char *s;

  if( (s=strchr(s2,'.')) == 0 ) s = s2+strlen(s2);
  while( s-s2<8 ) *s++ = ' ';
}

void set_exec( char *path, char *tail, char *env )
{
  char *s, *s2;

  strncpy( curapp->path, path, PATHLEN );
  strncpy( curapp->tail, tail, TAILLEN );
  curapp->tail[tail[0]+1] = '\0';
  curapp->env = env ? env : "\0";
  s2 = curapp->dflt_acc;
  *s2++ = *s2++ = ' ';
  s = path;
  if( *(s+2)==':' ) s+=2;
  strncpy( s2, pathend(s), 8 );
  *(s2+8) = '\0';
  pad_acc_name(s2);
}

/* 006: copy APP as a result of P*fork() */
void fix_pfork(void)
{
  extern char do_frk, frk_mode;
  extern int frk_id;
  APP *ap, *c;
  
  if( do_frk )
  {
    for( ap=app0; ap; ap=ap->next )
      if( ap->mint_id == frk_id )
      {
        c = curapp;
        curapp = ap;
        if( init_aph( 1, 0 ) )
        {
          memcpy( &curapp->flags, &ap->flags,
              (long)&((APP *)0L)->vex - (long)&((APP *)0L)->flags );
          curapp->waiting = 0;
          curapp->mint_id = 0;
          lock_curapp = 0;
          curapp->basepage = 0L;
        }
        curapp = c;
        break;
      }
    do_frk = 0;
  }
}

/* initialize an app header for a program started by Pexec */
void sh_exec( char *path, char *tail, char *env )
{
  int i, j, id;
  APP *ap;

  if( sleeping ) return;
  id = (ap=curapp)->mint_id;	/* 005: prevent toggle_multi from changing sleep of curapp */
  ap->mint_id = -1;	/* 005 */
  noflags.flags.s.multitask = !settings.flags.s.child_pexec_single && multitask;	/* 006 */
  if( init_aph( 0/*006: used to use noflags...multitask*/, 0 ) )
  {
    memcpy( &curapp->flags, &noflags, sizeof(APPFLAGS) );
    to_sleep( ASL_SINGLE, ASL_SINGLE, curapp );	/* just in case we are already single-tasking */
    i = noflags.flags.s.multitask;
    j = i ? ASL_PEXEC : ASL_SINGLE|ASL_PEXEC;	/* 006: added test */
    to_sleep( j, j, curapp->parent );
    set_multi(i);
    set_exec( path, tail, env ? env : curapp->parent->env );
    new_flags();
    if( (curapp->flags.flags.s.multitask = i) != 0 )
    	top_menu();		/* 006 */
    curapp->basepage = 0L;
    lock_curapp = 0;
  }
  ap->mint_id = id;	/* 005 */
}

APP *last_parent;	/* 004 */

void terminate( int ret )
{
  APP *parent;

  if( sleeping || !curapp || curapp==last_parent/*004*/ ) return;
  if( !preempt )
    if( *bp_addr==&magic_bp )
    {
      *bp_addr = *(BASPAG **)(curapp->stack+4);  /* basepage from stack */
      ring_bell();
      in_t2 = 0;
    }
  if( sw_flag<0 && curapp==sw_app/*004*/ ) sw_flag = 1;
  if( preempt ) _x_appl_term( curapp->id, ret, 0, 1 );
  else if( !curapp->basepage )       /* only do it now if not run by shell */
  {
    parent = curapp->parent;
    _x_appl_term( curapp->id, ret, 0, 1 );
    for( ; parent->child; parent=parent->child );
    set_curapp(last_parent/*004*/=parent);
  }
  else if( !(*bp_addr)->p_parent ) (*bp_addr)->p_parent = curapp->basepage;
  /* hack to fix up cleared-out parent pointer for DA's */
}

int shel_read( char *path, char *tail )
{
  strcpy( path, curapp->path );
  strcpy( tail, curapp->tail );
  return(1);
}

int _shel_envrn( char *env, char **ptr, char *s )
{
  char *p;
  int i;

  *ptr = 0L;	/* 004: was "" */
  if( !env ) return 0;
  i = strlen(s);
  while( *env )
  {
    if( !strncmp( env, s, i ) )
    {
      *ptr = env+i;
      return(1);
    }
    env += strlen(env)+1;
  }
  return(0);
}

int shel_envrn( char **ptr, char *s )
{
  _shel_envrn( curapp->env, ptr, s );
  return 1;
}

char *env_match( char *env_name, char *cmp, char *dflt )
{
  char *ptr, *p;

  _shel_envrn( environ, &ptr, env_name );
  if( !ptr/*004*/ || !*ptr ) ptr = dflt;
  p = ptr;
  for(;;)
  {
    if( *ptr == ',' || *ptr == ';' || !*ptr )
    {
      if( !strncmp( p, cmp, ptr-p ) && strlen(cmp)==ptr-p ) return p;
      if( *(p = ptr) == 0 ) return 0L;
      p++;
    }
    ptr++;
  }
}

void shel_path( char *path )
{
  char *ptr, c;

  if( !*path ) return;		/* 005 */
  if( path[1]==':' )
  {
    Dsetdrv((path[0]&0x5f)-'A');
    path+=2;
  }
  c = *(ptr=pathend(path));
  *ptr = '\0';
  Dsetpath(path);
  *ptr = c;
}

void shel_rpath(void)
{
  shel_path(loadpath);
}

void term_msg( int type, int why, int id )
{
  int buf[8];

  buf[0] = type;
  buf[2] = 0;
  if( type==AC_CLOSE ) find_menu_id( buf, id );
  buf[5] = why;
  _appl_write( buf[1]=id, 16, buf, 1 );
}

void send_term( APP *ap, int why, int alw_term )
{
  del_all_msg( -1, ap );
  if( ap->ap_type==4 ) term_msg( AC_CLOSE, why, ap->id );
  if( ap->ap_msgs&1 || alw_term ) term_msg( AP_TERM, why, ap->id );
}

void bad_unload(void)
{
  APP *ap;

  for( ap=app0; ap; ap=ap->next )
  {
    del_all_msg( AP_TERM, ap );
    del_all_msg( AC_CLOSE, ap );
  }
  unload=0;
  unloader=0L;
}

int child_apid(void)	/* 004: return apid if multitasking */
{
  APPFLAGS *apf;
  int i;

  if( sw_flag<0 ) return 1;
  if( (apf=find_flags(sw_path)) == 0L ) apf = &dflt_flags;
  if( apf->flags.s.multitask )
  {
    find_free_apno( 1, &i );
    return i;
  }
  return 32767;
}

char swrite_wait( int doex )
{
  extern char block_app;
/*  void dispatch2(void);*/

  if( mint_preem && loop_id && ((unsigned char)doex >= XSHW_RUNANY ||
      (unsigned char)doex <= SHW_RUNACC ||	/* includes SH_RUNHELP, etc. */
      doex==SH_RUNACC/*006*/) && sw_flag>0 )
      return block_app = curapp->was_blocked = 1;
/* 005: simplified to just redo the func until sw_flag<=0 */
/*****    while( sw_flag>0 && wait_id( Pgetpid() ) )
    {
        if( !curapp ) break;
#ifdef MINT_DEBUG
        Crawio( Pgetpid()+'0' );
        test_msg( curapp, "wait in shel_write" );
#endif
        dispatch2();
#ifdef MINT_DEBUG
        test_msg( curapp, "returned from wait" );
#endif
        lock_curapp = 0;
    } *******/
  return 0;
}

int _shel_write( int doex, int gr, int gem, char *name, char *tail )
{
  BASPAG *bp, *old;
  char *ptr, *ptr2, *ptr3, temp[20];
  APP *ap;
  int i, buf[8];
  MSGQ *msg, *msg2;
  Window *w, *w2;

  if( swrite_wait(doex) ) return 0;		/* 005 */
  if( (sw_ex = doex) == SH_RUNHELP )
    if( _shel_envrn( environ, &ptr2, "SHOWHELP=" ) )
    {
      sw_hlpfile = name;
      sw_hlptopic = tail;
      sw_hlpsens = gem;
      name = (char *)&shwrcmd;
      shwrcmd.name = pathend(ptr2);
      strcpy( shwrcmd.dflt_dir=sw_dflt, ptr2 );		/* 004: copy into sw_dflt */
      *pathend(sw_dflt) = 0;				/* 004: and end the path */
      tail = "\0";
      doex = SHD_DFLTDIR;
    }
    else return 0;
  else if( (unsigned char)doex >= XSHW_RUNANY && (unsigned char)doex <= XSHW_RUNACC )
      doex -= XSHW_RUNANY-SHW_RUNANY;
  else if( doex == SH_RUNSHELL || doex == SH_RUNSLEEP ) doex = SHD_DFLTDIR;
  else if( doex == SH_RUNACC ) doex = SHD_DFLTDIR | SHW_RUNACC;
  else if( doex == SH_REGHELP ) 	/* 005 */
  {
    help_app = curapp;
    return 1;
  }
  if( (unsigned char)doex <= SHW_RUNAPP || (unsigned char)doex==SHW_RUNACC )
  {
    if( unload ) return 0;
    if( sw_tail != _sw_tail ) lfree(sw_tail);
    if( tail[0]!='\xff' )
    {
      strcpy( sw_tail = _sw_tail, tail );
      if( !tail[0] ) *(sw_tail+1) = 0;
    }
    else if( (sw_tail = lalloc(strlen(tail+1)+129,-1)) == 0 )
    {
      sw_tail = _sw_tail;
      return 0;
    }
    else
    {
      *sw_tail = '\xff';
      strcpy( sw_tail+1, tail+1 );
    }
    if( doex&0xff00 )
    {
      memcpy( &shwrcmd, name, sizeof(shwrcmd) );
      strcpy( sw_path, shwrcmd.name );
    }
    else strcpy( sw_path, name );
    if( !sw_path[0] )
    {
      sw_flag = 0;	/* 004 */
      return 0;
    }
    if( sw_path[1] != ':' && !strchr( sw_path,'\\' ) ||
        !strchr( pathend(sw_path), '.' ) )
    {
      _shel_envrn( environ, &ptr2, "GEMEXT=" );
      ptr3 = doex&SHD_DFLTDIR ? shwrcmd.dflt_dir : 0L;
      if( !_shel_find( ptr3, sw_path, 0L, ptr2 ) )
      {
        _shel_envrn( environ, &ptr2, "TOSEXT=" );
        if( !_shel_find( ptr3, sw_path, 0L, ptr2 ) )
        {
          _shel_envrn( environ, &ptr2, "ACCEXT=" );
          _shel_find( ptr3, sw_path, 0L, ptr2 );
        }
      }
    }
    strupr( sw_path );
    if( doex&SHD_DFLTDIR && shwrcmd.dflt_dir[0]/*005*/ )
    {
      strcpy( sw_dflt, shwrcmd.dflt_dir );
      if( *pathend(sw_dflt) ) strcat( sw_dflt, "\\" );	/* 004 */
    }
    else
    {
      old = shel_context(0L);
      sw_dflt[0] = Dgetdrv()+'A';
      sw_dflt[1] = ':';
      Dgetpath( sw_dflt+2, 0 );
      shel_context(old);
      strcat( sw_dflt, "\\" );
    }
    sw_env = (doex&SHD_ENVIRON) && shwrcmd.environ ? shwrcmd.environ : environ;
    sw_acc = 1;
  }
  switch( (unsigned char)doex )
  {
    case SHW_RUNANY:
      if( (ptr=strrchr(sw_path,'.'))!=0 && ptr>=pathend(sw_path) )
        if( env_match( "ACCEXT=", ++ptr, "ACC,ACX" ) ) goto acc;
        else if( env_match( "TOSEXT=", ptr, "TOS,TTP" ) ) gr = 0;	/* 006: used to contain moved block below */
        else if( env_match( "GEMEXT=", ptr, "PRG,APP,GTP" ) ) gr = 1;
        else gr = 1;
    case SHW_RUNAPP:
      if( !gr && ((unsigned char)doex != SHW_RUNANY ||
          ptr && curapp->flags.flags.s.multitask) &&
          _shel_envrn( environ, &ptr2, "TOSRUN=" ) )	/* 006: moved this block here */
      {
        strcpy( sw_tail+1, sw_path );
        if( tail[0] )
        {
          strcat( sw_tail+1, " " );
          strcat( sw_tail+1, tail+1 );
        }
        *sw_tail = (i=strlen(sw_tail+1)) < 126 ? i : '\xff';
        strcpy( sw_path, ptr2 );
        sw_gr = sw_argv = 1;
        sw_acc = 0;
        sw_app = curapp;		/* 004; 005: sw_app global */
        sw_flag = curapp->flags.flags.s.multitask ? 1 : -1;
        return child_apid();	/* 004 */
      }
      sw_acc = 0;
acc:  sw_gr = gr;
      sw_app = curapp;		/* 004 */
      sw_argv = (sw_flag = curapp->flags.flags.s.multitask ? 1 : -1) < 0 ? 0 : gem;	/* 005: argv used to always be on for s-task */
      return child_apid();	/* 004 */
    case SHW_RUNACC:
      goto acc;
    case SHW_SHUTDOWN:		/* shut down */
      if( !gr )
      {
        bad_unload();
        return 1;
      }
      unload_type = SHUT_COMPLETED;
unld: if( gr!=1 && gr!=2 && gr!=-1 || !multitask ) return 0;
      unload = gr<0 ? -1 : 2;
      for( i=0, ap=app0; ap; ap=ap->next )
        if( ap!=curapp && (unload<0 || ap->ap_type==unload) )
          if( !(ap->ap_msgs&1) ) i++;
      if( i && _form_alert( 2, SHUTDOWN ) == 2 )
      {
        unload = 0;
        return 0;
      }
      for( ap=app0; ap; ap=ap->next )
      {
        if( ap->asleep&ASL_USER ) to_sleep( ASL_USER, 0, ap );
        if( ap!=curapp )
        {
          if( unload<0 || ap->ap_type==unload ) send_term( ap, AP_TERM, 1 );
          if( ap->type==4 )
            for( w2=desktop->next; (w=w2)!=0; )
            {
              w2 = w->next;
              close_del(w);
            }
        }
      }
      unloader = curapp;
      return 1;
    case SHW_NEWREZ:
      if( !gem )
      {
        if( gr==cur_rez ) return 0;
        new_rez = gr;
      }
      else if( gem!=1 ) return 0;
      else
      {
        if( Vsetmode(-1)==gr ) return 0;
        new_vid = gr;
        new_rez = 3;
      }
      unload_type = RESCH_COMPLETED;
      gr = -1;
      goto unld;
    case 6:     	    /* broadcast (for backward compatibility with <= 005) */
    case SHW_BROADCAST:     /* broadcast (7 is the correct value!) */
      for( ap=app0; ap; ap=ap->next )
        if( ap!=curapp && ap->ap_type!=1 && 
            (!ap->neo_le005 || *(int *)name!=80)/*007*/ ) 
          _appl_write( ap->id, 16, (int *)name, 0 );
      return 1;
    case SHW_ENVIRON:     /* env */
      switch(gr)
      {
        case 0:
          return env_buflen;
        case 1:
          for( ptr=name, ptr2=temp; *ptr; )
            if( (*ptr2++ = *ptr++) == '=' ) break;
          *ptr2++ = 0;
          if( _shel_envrn( environ, &ptr2, temp ) )
          {
            ptr3 = ptr2+strlen(ptr2)+1;
            memcpy( ptr2-strlen(temp), ptr3, environ+env_buflen-ptr3 );
          }
          if( !*ptr ) return 1;		/* just delete */
          if( env_buflen )
          {
            for( ptr=environ; *ptr; ptr+=strlen(ptr)+1 ); /* find end */
            if( ptr+strlen(name)+2 > environ+env_buflen )
            {
              ptr2 = environ;
              if( lrealloc( (void **)&ptr2, env_buflen+ENVBUFSIZ ) ) return 0;
              ptr = ptr-environ+ptr2;
              environ = ptr2;
              env_buflen += ENVBUFSIZ;
            }
          }
          else if( (ptr = lalloc(ENVBUFSIZ,-1)) != 0 )
          {
            environ = ptr;
            env_buflen += ENVBUFSIZ;
          }
          else return 0;
          strcpy( ptr, name );
          *(ptr+strlen(ptr)+1) = 0;
          return 1;
        case 2:
          if( !env_buflen )
          {
            *name = *name++ = 0;
            return gem-2;
          }
          memcpy( name, environ, gem>env_buflen ? env_buflen : gem );
          return gem>=env_buflen ? 0 : env_buflen-gem;
      }
      return 0;
    case SHW_MSGTYPE:
      curapp->ap_msgs = gr;
      return 1;
    case SHW_SENDTOAES:
      switch( *(int *)name )
      {
/*        case X_DRAGDROP:
          for( ap=app0; ap; ap=ap->next )
            if( ap->id == *((int *)name+1) )
              if( ap->ap_msgs&X_SHW_DRAGDROP )
                  _appl_write( ap->id, 16, (int *)name, 0 );
              else if( has_mint ) ddcreate( (int *)name );
              else return 0;
          return 0; */
        case AP_TFAIL:
          if( unloader )
          {
            newtop( unload_type, 0, unloader->id );
            bad_unload();
          }
          return 1;
        case SH_WDRAW:
          if( shell_app )
          {
            memcpy( buf, name, 8*2 );
            buf[4] = -1;
            _appl_write( buf[1]=shell_app->id, 16, buf, 0 );
          }
          else return 0;
          break;
        case WM_ARROWED:
          memcpy( buf, name, 8*2 );
          if( (w = find_window( buf[3] )) != 0 )
            if( scroll_wdial( w, buf[4], 0, 0 ) ) return 1;
          return 0;
      }
      return 1;
    default:
      DEBUGGER(SHWR,UNKTYPE,(unsigned char)doex);
      return 1;
  }
}

void shel_disp( char *title, int num )
{
  char *p;
  long *l, *m;

  if( !has_menu || !multitask )
  {
    /* 004: so X_WF_DFLTDESK works here: copy type, flags, state, spec */
    l = (long *)&blankdesk[0].ob_type;
    m = (long *)&dflt_desk[0].ob_type;
    *l++ = *m++;
    *l++ = *m++;
    *(int *)l = *(int *)m;
    blankdesk[0].ob_width = blankdesk[1].ob_width = desktop->outer.w;
    blankdesk[0].ob_height = desktop->outer.h;
    blankdesk[1].ob_height = menu_h-1;
    for( blankted.te_ptext=p=pathend(title); *p; p++ )
      if( *p>='a' && *p<='z' ) *p &= 0x5f;
    desktop->tree = blankdesk;
    desk_obj = 0;
    _objc_draw( (OBJECT2 *)blankdesk, 0L, num, 1, Xrect(desktop->outer) );
  }
}

BASPAG *shel_context( BASPAG *bp )
{
  BASPAG *out;
  static int old_dom;
  static char in_sc;

  if( preempt )
  {
    if( mint_preem )
      if( !bp )
      {
        if( !in_sc++ )
        {
          old_dom = Pdomain(-1);
          Pdomain(0);
        }
        return (BASPAG *)1L;
      }
      else if( !--in_sc )
        if( old_dom ) Pdomain(old_dom);
    return 0L;
  }
  if( has_mint ) return 0L;
  if( !bp ) bp = curr_bp;
  out = *bp_addr;
  *bp_addr = bp;
  return out;
}

void odd_ex(void)
{
  if( sw_ex==SH_RUNHELP )
  {
    help_app = curapp;
    _x_help( sw_hlptopic, sw_hlpfile, sw_hlpsens );
  }
  else if( sw_ex==SH_RUNSHELL ) shell_app = curapp;
  else if( sw_ex==SH_RUNSLEEP ) curapp->asleep |= ASL_USER;
  else
  {
    if( sw_ex&SHD_PSETLIM )
      if( has_mint ) *(long *)(curapp->basepage->p_cmdlin+18) =
          shwrcmd.psetlimit;
      else
      {
        curapp->flags.flags.s.mem_limit = shwrcmd.psetlimit/1024L;
        curapp->flags.flags.s.limit_memory = 1;
      }
    if( sw_ex&SHD_PRENICE )
        *(int *)(curapp->basepage->p_cmdlin+22) = shwrcmd.prenice;
  }
}

long _trap2(long val) 0x4e42;

void fix_warp9(void)
{
  static char w9_fixed;
  int i;
  long l;

  if( !w9_fixed )
  {
    w9_fixed=1;
    if( (i=_trap2(0x18dbaL)) > 0x300 && i < 0x382 &&
        (l=_trap2(0x5142414BL)) != 0 && l!=0x5142414BL )
        *(int *)(l+2) = 1;	/* force zoom boxes on */
  }
}

int shel_exec(void)
{
  BASPAG *bp;
  int id, ret, bad;
  APPFLAGS *apf, af;
  APP *ap;

  if( sw_flag<=0 ) return(0);
  fix_warp9();
  if( (apf=find_flags(sw_path)) == 0L ) apf = &dflt_flags;
  if( (unsigned char)sw_ex >= XSHW_RUNANY && (unsigned char)sw_ex <= XSHW_RUNACC &&
      sw_ex&XSHD_FLAGS )
  {
    memcpy( &af, apf, sizeof(af) );
    af.flags.l = shwrcmd.app_flags;
    apf = &af;
  }
  sw_flag = 0;
  shel_path(sw_dflt);
  if( sw_acc )
  {
    ap = curapp;
    do_sig();
    if( (bp=acc_run(sw_path)) != 0 )
    {
      /* odd_ex(); 004: moved into go_acc */
      id = curapp->id;
      if( sw_ex != SH_RUNACC/*004*/ ) ac_open( 0L, id );
      if( go_acc( ap, bp ) )
      {
        shel_rpath();
        return 1;
      }
      exit_msg( id, ap->id, load_err );
    }
    else exit_msg( -1, ap->id, load_err );
    shel_rpath();
    return 0;
  }
  if( init_aph( apf->flags.s.multitask, 0 ) )
  {
    memcpy( &curapp->flags, apf, sizeof(APPFLAGS) );
/*    curapp->asleep |= ASL_PEXEC;*/	/* 005: prevent toggle_multi from changing sleep of curapp; rmv for 006 because it messes up PMJ and probably isn't needed due to change in noflags.multi */
    if( !multitask )  /* put the only ST app to sleep */
        toggle_multi( 0L, ASL_USER, 0 );
    bad=0;
    if( settings.flags.s.alert_mode_change && multitask &&
        !apf->flags.s.multitask )
    {
      if( sw_ex==SH_RUNSLEEP/*004*/ ) curapp->asleep |= ASL_USER;
      else switch( _form_alert( 1, ENTERST ) )	/* 005: add Cancel button */
      {
        case 1:
          set_multi( apf->flags.s.multitask );
          break;
        case 2:
          curapp->asleep |= ASL_USER;
          break;
        case 3:
          bad = 1;
          load_err = IEINVFN;
          break;
      }
    }
    else set_multi( apf->flags.s.multitask );
/*    curapp->asleep &= ~ASL_PEXEC;  006 */	/* 005 */
    if( !bad ) /*005*/
      if( !multitask )
        if( sw_gr )
        {
          Cconws( "\033f" );
          shel_disp( sw_path, 0 );
        }
        else
        {
          _graf_mouse( M_OFF, 0L, 0 );
          Cconws( "\033E\033e" );
        }
      else _graf_mouse( X_MRESET, 0L, 0 );
    id = curapp->id;
    ap = curapp;
    if( !bad/*005*/ && shel_setup() )
    {
      /* 1.8  shel_rpath(); */
      set_exec( sw_path, sw_tail, ap->basepage->p_env );
      _menu_register( id, 0L );
      odd_ex();
      in_t2 = 0;
      do_sig();
      ret = my_pexec( ap->basepage );	/* only returns if preempt */
      if( ret<0 ) load_err = (int)ret;
      else
      {
        shel_rpath();
        return 1;
      }
    }
    __x_appl_term( id, load_err, 0 );        /* catches errors */
    if( !curapp ) curapp = app0;	/* 005 */
  }
  else exit_msg( -1, curapp->id, load_err );
  lock_curapp = 0;
  shel_rpath();
  return(1);
}

int shel_get( char *buf, int len )
{
  char temp[120];
  int h;
  long l;

  strcpy( temp, loadpath );
  strcat( temp, "NEWDESK.INF" );
  if( (h = Fopen(temp,0)) < 0 )
  {
    strcpy( pathend(temp), "DESKTOP.INF" );
    if( (h = Fopen(temp,0)) < 0 )
    {
      DEBUGGER(SHGE,SHGEFILE,0);
      return 0;
    }
  }
  if( len>0 ) Fread( h, len, buf );
  l = Fseek( 0L, h, 2 );
  Fclose(h);
  return (int)l;
}

int shel_put( char *buf, int len )
{
/*  char temp[120];
  int h;
*/
  DEBUGGER(SHPU,NOTDONE,0);
  len = *buf;
  *buf=len;
/*  strcpy( temp, loadpath );
  strcat( temp, "DESKTOP.INF" );
  if( (h = Fcreate(temp,0)) > 0 )
  {
    Fwrite( h, len, buf );
    Fclose(h);
    return(1);
  }*/
  return(0);
}

char *shel_splitlist( char *in, char *out )
{
  int i;
  char *in0=in;

  for(;;)
  {
    if( *in == ',' || *in == ';' || !*in )
    {
      i = in-in0;
      strncpy( out, in0, i );
      out[i] = 0;
      return !*in ? 0L : in+1;
    }
    in++;
  }
}

void fix_path( char *path )	/* 004: avoid prob w/ . paths in some prg's */
{
  char temp[120], *p, *t;

  strcpy( t=temp, p=path );
  if( t[1] != ':' )		/* no drive letter */
  {
    *p++ = Dgetdrv()+'A';
    *p++ = ':';
  }
  else			/* copy drive letter: */
  {
    *p++ = *t++;
    *p++ = *t++;
  }
  if( *t != '\\' )		/* not an absolute path */
  {
    Dgetpath( p, 0 );		/* get my directory */
    p += strlen(p);
    *p++ = '\\';
  }
  if( !strncmp( t, ".", 2 ) ) t++;
  else if( !strncmp( t, ".\\", 2 ) ) t += 2;
  strcpy( p, t );
}

int _shel_find( char *dflt, char *buf, char **paths, char *ext )
{
  DTA *old;
  int ret=0, i;
  char *ptr, temp[120], *ex, *end, old_path[120], old_drv,
      chk_home=0, home=0;
  BASPAG *old_bp;

  chk_home = !dflt && !paths && !ext;
  old_bp = shel_context(0L);
  if( dflt )
  {
    old_drv = Dgetdrv();
    Dgetpath( old_path, 0 );
    shel_path(dflt);
  }
  old = Fgetdta();
  Fsetdta(&shel_dta);
  if( strchr(buf,'\\') || buf[1]==':' )
  {
    ptr = "";
    paths = &ptr;
  }
  if( !paths )
  {
    if( !_shel_envrn( environ, &ptr, "PATH=" ) )
        shel_envrn( &ptr, "PATH=" );
    if( !ptr ) ptr = "";	/* 004 */
    paths = &ptr;
  }
  if( ext )
    if( strchr(pathend(buf),'.') ) ext = 0L;
  while( !ret )
  {
    if( !home/*006*/ ) *paths = shel_splitlist( *paths, temp );
/*    fix_path(temp); considered in 004 */
    if( (i=strlen(temp)) != 0 && temp[i-1]!='\\' ) strcat( temp, "\\" );
    if( !strncmp( temp, ".\\", 2 ) ) strcpy( temp, temp+2 );	/* 004 added, 005 swapped with above */
    strcat( temp, buf );
    if( ext ) strcat( temp, "." );
    end = temp+strlen(temp);
    ex = ext ? ext : "";
    while( !ret )
    {
      ex = shel_splitlist( ex, end );
      if( !Fsfirst( temp, 0x27 ) )	/* 004: was 0x37 */
      {
        strcpy( buf, temp );
        ret = 1;
      }
      if( !ex ) break;
    }
    if( !*paths )
      if( home || !chk_home ) break;	/* 006: check app's "home" dir */
      else
      {
        home = 1;
        strcpy( temp, curapp->path );
        *pathend(temp) = 0;
      }
  }
  Fsetdta(old);
  if( dflt )
  {
    Dsetdrv(old_drv);
    Dsetpath(old_path);
  }
  shel_context(old_bp);
  return(ret);
}

int shel_find( char *buf )
{
  return _shel_find( 0L, buf, 0L, 0L );
}


