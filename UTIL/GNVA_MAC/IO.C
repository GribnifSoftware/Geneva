#include "new_aes.h"
#include "xwind.h"              /* Geneva extensions */
#include "tos.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "lerrno.h"
#include "ctype.h"
#include "gnva_mac.h"
#include "common.h"

#define SHBUFSIZ 1024
#define SHLINELEN 80

int shgph, shgpph, shglen;
char *shbptr, sh_eof;
int mdelete( int num );

void close_sh(void)
{
  if( shgph>0 )
  {
    Fclose(shgph);
    shgph = 0;
    if( shbptr ) xmfree(shbptr);
  }
}

int open_sh( char *name, int create )
{
  if( create ) shgph = Fcreate( name, 0 );
  else shgph = Fopen(name,0);
  if( shgph < 0 ) return 0;
  if( (shbptr = xmalloc( SHBUFSIZ )) == 0 )
  {
    close_sh();
    return 0;
  }
  shglen = 0;
  sh_eof = 0;
  return shgph;
}

char flush_sh( char noerr )
{
  if( !noerr ) return 0;
  if( shglen && Fwrite( shgph, shglen, shbptr ) != shglen ) return 0;
  shglen = 0;
  return 1;
}

char _fwrite( char noerr, long len, void *addr )
{
  long l;

  while( len && noerr )
  {
    if( shglen+len < SHBUFSIZ ) l = len;
    else l = SHBUFSIZ-shglen;
    memcpy( shbptr+shglen, addr, l );
    (char *)addr += l;
    shglen += l;
    len -= l;
    if( shglen==SHBUFSIZ ) noerr = flush_sh(noerr);
  }
  return noerr;
}

char _fread( char noerr, int h, long len, void *addr )
{
  if( !noerr ) return 0;
  if( Fread( h, len, addr ) != len ) return 0;
  return 1;
}

int sh_getc( char *buf )
{
  static char *shgptr;
  int ret;

  if( !shglen )
    if( (shglen = Fread( shgph, SHBUFSIZ, shgptr=shbptr )) < 0 )
    {
      ret = shglen;
      shglen = 0;
      return ret;
    }
    else if( !shglen )
      if( !sh_eof++ )
      {
        *buf = '\n';
        return 1;
      }
      else return 0;
  *buf = *shgptr++;
  shglen--;
  return 1;
}

int _get_shline( char *buf )
{
  int ret, count;

  if( shgph>0 )
  {
    for( count=0; count<SHLINELEN-1 && (ret=sh_getc(buf)) == 1; )
      if( *buf=='\n' )
      {
        *buf = 0;
        break;
      }
      else if( *buf!='\r' )
      {
        buf++;
        count++;
      }
    if( ret<0 ) return -2;
    return ret;
  }
  return 0;
}

int get_shline( char *buf )
{
  int ret;

  while( (ret=_get_shline(buf)) == 1 && (buf[0]==';' || buf[0]=='#') );
  return ret;
}

/* write a full GRECT or just the x and y */
int put_grect( GRECT *g, int wh, int ok )
{
  char temp[50];

  if( ok>0 )
  {
    x_sprintf( temp, wh ? "%d %d %d %d %d %d" : "%d %d", g->g_x,
        g->g_y<max.g_y ? -99 : g->g_y-max.g_y, g->g_w/char_w,
        g->g_w%char_w, g->g_h/char_h, g->g_h%char_h );
    ok = x_shel_put( X_SHACCESS, temp );
  }
  return ok;
}

/* save window and dialog positions */
void save_settings(void)
{
  int ok, i;
  char temp[130];

  if( x_shel_put( X_SHOPEN, "Geneva Macro" ) )
  {
    ok = x_shel_put( X_SHACCESS, GNVAMAC_VER );
    ok = put_grect( &wsize, 1, ok );
    for( i=0; i<sizeof(form)/sizeof(FORM)-1; i++ )
      ok = put_grect( &form[i].wind, 0, ok );
    if( ok )
    {
      get_etypes();
      x_sprintf( temp, "%d %h %d %d %d %k", macsize, etypes, d_sound, d_date, d_time, &start_end );
      ok = x_shel_put( X_SHACCESS, temp );
      if( ok )
      {
        x_sprintf( temp, "%h %s", gma_auto, gma_path );
        ok = x_shel_put( X_SHACCESS, temp );
      }
      for( i=0; i<8 && ok; i++ )
        ok = x_shel_put( X_SHACCESS, date_fmt[i] );
      for( i=0; i<8 && ok; i++ )
        ok = x_shel_put( X_SHACCESS, time_fmt[i] );
    }
    if( ok>0 ) x_shel_put( X_SHCLOSE, 0L );
  }
}

/* read a full GRECT or just the x and y */
int get_grect( GRECT *g, int wh, int ok )
{
  char temp[50];
  int w, h, wr, hr;

  if( ok>0 && (ok = x_shel_get( X_SHACCESS, sizeof(temp), temp )) > 0 )
  {
    x_sscanf( temp, wh ? "%d %d %d %d %d %d" : "%d %d", &g->g_x,
        &g->g_y, &w, &wr, &h, &hr );
    if( g->g_x >= max.g_w ) g->g_x = max.g_w - (char_w<<1);
    if( g->g_y >= max.g_h ) g->g_y = max.g_h - (char_h<<1);
    g->g_y += max.g_y;
    if( wh )
    {
      w = w*char_w + wr;
      h = h*char_h + hr;
      if( w && h )
      {
        g->g_w = w;
        g->g_h = h;
      }
    }
  }
  return ok;
}

/* load window and dialog positions */
void load_settings(void)
{
  char temp[130];
  int ok, i;

  while( (i=x_shel_get( X_SHOPEN, 0, "Geneva Macro" )) == -1 );
  if( i>0 )
  {
    ok = x_shel_get( X_SHACCESS, sizeof(temp), temp );
    if( !strcmp( temp, GNVAMAC_VER ) )
    {
      ok = get_grect( &wsize, 1, ok );
      for( i=0; i<sizeof(form)/sizeof(FORM)-1; i++ )
        ok = get_grect( &form[i].wind, 0, ok );
      if( ok>0 && (ok = x_shel_get( X_SHACCESS, sizeof(temp), temp )) > 0 )
      {
        x_sscanf( temp, "%d %h %d %d %d %k", &macsize, &etypes, &d_sound, &d_date, &d_time, &start_end );
        if( ok>0 && (ok = x_shel_get( X_SHACCESS, sizeof(temp), temp )) > 0 )
            x_sscanf( temp, "%h %s", &gma_auto, gma_path );
      }
      for( i=0; i<8 && ok>0; i++ )
        ok = x_shel_get( X_SHACCESS, 16, date_fmt[i] );
      for( i=0; i<8 && ok>0; i++ )
        ok = x_shel_get( X_SHACCESS, 16, time_fmt[i] );
    }
    if( ok>0 ) x_shel_get( X_SHCLOSE, 0, 0L );
  }
}

#define GMA_VER	0x474D4101	/* GMA1 */

typedef struct
{
  long ver;
  unsigned int hdr_size,	/* sizeof(GMAHDR) */
 	       ent_size,	/* sizeof(GMAITEM) */
  	       entries;		/* number of items */
  unsigned int msgs;
  long msg_ptr;
} GMAHDR;

typedef struct
{
  char name[21];
  KEYCODE key;
  unsigned int len;
  int auto_on, automode;
  unsigned int date, time;
} GMAITEM;

int save_gma( char *path, int as )
{
  int i, j, ret=0;
  char temp[120], noerr=1;
  GMAHDR hdr;
  MACDESC *m;
  MACBUF *mb;

  if( as && !Fsfirst( path, 0x27 ) && alert(SAVEOVER)==2 ) return 0;
  bee();
  hdr.ver = GMA_VER;
  hdr.hdr_size = sizeof(GMAHDR);
  hdr.ent_size = sizeof(GMAITEM);
  hdr.entries = iglobl;
  strcpy( temp, path );
  strcpy( spathend(temp), "$MACRO$.$$$" );
  if( open_sh(temp,1) )
  {
    hdr.msgs = 0;
    hdr.msg_ptr = 0L;
    noerr = _fwrite( noerr, sizeof(GMAHDR), &hdr );
    for( i=iglobl, m=mdesc; --i>=0 && noerr; m++ )
    {
      noerr = _fwrite( noerr, sizeof(GMAITEM), m );
      noerr = _fwrite( noerr, sizeof(MACBUF)*m->len, m->mb );
    }
    hdr.msg_ptr = Fseek( 0L, shgph, 1 ) + shglen;
    for( i=iglobl, m=mdesc; --i>=0; m++ )
      for( j=m->len, mb=m->mb; --j>=0 && noerr; mb++ )
        if( mb->type==TYPE_MSG )
          if( !mb->u.msg )
          {
            Bconout(2,7);
            noerr = 0;
          }
          else
          {
            noerr = _fwrite( noerr, 5*31, mb->u.msg );
            hdr.msgs++;
          }
    flush_sh(noerr);
    if( hdr.msgs && noerr )
    {
      Fseek( 0L, shgph, 0 );
      Fwrite( shgph, sizeof(GMAHDR), &hdr );
    }
    close_sh();
    if( noerr ) if( Fdelete(path)==AEACCDN ) noerr=0;
    if( noerr ) if( Frename( 0, temp, path ) ) noerr=0;
    if( !noerr )
    {
      Fdelete(temp);
      alert(BADSAVE);
    }
    else
    {
      ret=1;
      last_chk = chksum();
    }
  }
  else alert(BADSAVE);
  arrow();
  return ret;
}

long current_time(void)
{
  return ((long)Tgetdate()<<16) | (Tgettime()&~0x1f/* 0 seconds */);
}

void free_most_macs(void)
{
  int i;
  
  for( i=iglobl; --i>=0; )
    mdelete(0);		/* delete all non-global */
}

int load_gma( char *path, int nf )
{
  int h, i, dif, hms, j;
  long l;
  char noerr=1, new_global;
  GMAHDR hdr;
  MACDESC *m2;
  GMAITEM m;
  MACBUF *mb, *mb2;

  if( !test_close() ) return 0;
  if( aes_ok ) bee();
  if( (h = Fopen( path, 0 )) > 0 )
  {
    noerr = _fread( noerr, h, sizeof(GMAHDR), &hdr );
    if( noerr && (hdr.ver == GMA_VER || hdr.ver == GMA_VER-1) )
    {
      if( (i = hdr.hdr_size-sizeof(GMAHDR)) < 0 )
      {
        Fseek( i, h, 1 );
        hdr.msgs = 0;
      }
      if( hdr.msgs )
        if( (hms = Fopen( path, 0 )) < 0 )
        {
          Fclose(h);
          goto bad_opn;
        }
        else if( Fseek( hdr.msg_ptr, hms, 0 ) != hdr.msg_ptr )
        {
          Fclose(hms);
          Fclose(h);
          goto bad_opn;
        }
      new_global = !strcmpi( spathend(path), "GLOBAL.GMA" );
      if( !new_global )
        if( !is_global )
        {
          if( aes_ok ) free_most_macs();
          else if( !free_macs( 1, 1 ) ) goto nofree;
        }
        else
          for( m2=mdesc, i=nmacs; --i>=0; m2++ )
          {
            m2->is_global = 1;	/* all are now global */
            set_global(m2);
          }
      else if( !free_macs( 1, 0 ) )	/* delete all */
      {
nofree: Fclose(h);
	if( hms ) Fclose(hms);
        if( !aes_ok ) return 0;
        form_error(8);
        quit();
        return 0;
      }
      dif = hdr.ent_size - sizeof(GMAITEM);
      for( i=0; i<hdr.entries && noerr; i++ )
      {
        noerr = _fread( noerr, h, dif>0 ? sizeof(GMAITEM) : hdr.ent_size, &m );
        if( noerr )
        {
          if(dif>0) Fseek( dif, h, 1 );
          else if(dif<0)
          {
            m.auto_on = m.automode = 0;
            *(long *)&m.date = current_time();
          }
          l = sizeof(MACBUF)*m.len;
          if(l)
          {
            if( (mb = xmalloc(l)) == 0 )
            {
              noerr = -1;
              break;
            }
            noerr = _fread( noerr, h, l, mb );
            for( mb2=mb, j=m.len; --j>=0 && noerr; mb2++ )
              if( mb2->type == TYPE_MSG )
                if( !hdr.msgs-- ) noerr++;
                else if( (mb2->u.msg=xmalloc(5*31)) == 0 ) noerr++;
                else if( Fread( hms, 5*31, mb2->u.msg ) != 5*31 )
                {
                  cmfree( (void **)&mb2->u.msg );
                  noerr++;
                }
          }
          else mb = 0L;
          if( !(aes_ok ? shift_maclist(i,1) : add_mlist()) )
          {
            if( mb ) xmfree(mb);
            break;
          }
          iglobl = i+1;
          m2=&mdesc[i];
          if( !aes_ok ) memcpy( m2+1, m2, (nmacs-i-1)*sizeof(MACDESC) );
          memcpy( m2, &m, sizeof(m) );
          m2->is_global = 0;
          m2->mb = mb;
          if( aes_ok ) setup_mac( m2 );
        }
      }
      is_global = new_global;
      if( aes_ok )
      {
        if( !noerr ) alert(BADLOAD);
        do_info();
        main_dial();
      }
    }
    else if( aes_ok ) alert(BADLOAD);
    Fclose(h);
    if( hms ) Fclose(hms);
  }
  else
  {
bad_opn:
    if( nf ) alert(BADOPEN);
    noerr = 0;
  }
  if( aes_ok ) arrow();
  i = noerr>0;
  if(i)
  {
    last_chk = chksum();
    strcpy( main_path, path );
    main_title();
  }
  return i;
}

int not_global(void)
{
  alert(NOTGLOB);
  return 0;
}

int test_eglob( EDITDESC *e )
{
  if( e->m->is_global ) return not_global();
  return 1;
}

int test_mglob( MACDESC *m )
{
  if( m->is_global ) return not_global();
  return 1;
}

int mdelete( int num )
{
  MACDESC *m;
  EDITDESC *e, *e2;

  m = &mdesc[num];
  if( !test_mglob(m) ) return 0;
  for( e2=elist; (e=e2) != 0; )
  {
    e2 = e->next;
    if( e->m == m ) close_edit(e);
  }
  if( rec_mac==m ) cancel_rec();
  return shift_maclist( num, -1 );
}

int eedelete( EDITDESC *e, int num )
{
  if( !test_eglob(e) ) return 0;
  return shift_evnt( e, num, -1, 0L );
}

int clip_s( char *s )
{
  return _fwrite( 1, strlen(s), s );
}

int clip_str( char *s )
{
  if( clip_s(s) ) return clip_s( "\r\n" );
  return 0;
}

int eesaveclip( MACBUF *mb )
{
  char temp[50], *ptr;
  int i;

  temp[0] = 0;
  switch( mb->type )
  {
    case TYPE_TIMER:
      x_sprintf( temp, "TIMER %D", mb->u.timer );
      break;
    case TYPE_BUTTN:
      x_sprintf( temp, "BUTTN %d %d", mb->u.button.state, mb->u.button.clicks );
      break;
    case TYPE_MOUSE:
      x_sprintf( temp, "MOUSE %d %d", mb->u.mouse.x, mb->u.mouse.y );
      break;
    case TYPE_KEY:
      x_sprintf( temp, "KEYBD %x %h %h", mb->u.key.shift, mb->u.key.scan, mb->u.key.ascii );
      break;
    case TYPE_SOUND:
      x_sprintf( temp, "SOUND %x", mb->u.sound.index );
      break;
    case TYPE_DATE:
      x_sprintf( temp, "DATE %x", mb->u.sound.index );
      break;
    case TYPE_TIME:
      x_sprintf( temp, "TIME %x", mb->u.sound.index );
      break;
    case TYPE_MSG:
      if( mb->u.msg == 0 ) break;
      if( !clip_str( "MSG" ) ) return 0;
      for( i=0; i<5; i++ )
        if( !clip_s( "+MSG " ) || !clip_str( (*mb->u.msg)[i] ) ) return 0;
      return 1;
  }
  if( temp[0] )
    return clip_str(temp);
  return 1;
}

int msaveclip( int num )
{
  MACDESC *m;
  MACBUF *mb;
  int i;
  char temp[50];

  m = &mdesc[num];
  x_sprintf( temp, "MACRO %S %k %b %d %X", m->name, &m->key, m->auto_on, m->automode, *(long *)&m->date );
  if( !clip_str(temp) ) return 0;
  for( i=m->len, mb=m->mb; --i>=0; mb++ )
    if( !eesaveclip(mb) ) return 0;
  return 1;
}

int esaveclip( EDITDESC *e, int num )
{
  return eesaveclip( &e->m->mb[num] );
}

void delete(void)
{
  if( mac_list(0)<0 ) return;
  selfunc( mdelete, 1 );
  macs_off(0);
  main_dial();
}

void edelete( EDITDESC *e )
{
  if( evnt_list(e,0)<0 ) return;
  eselfunc( eedelete, e, 1 );
/*  evnts_off( e, 0 );*/
  evnt_tree(e);
  set_elist_ht(e);
  edit_dial(e);
}

int save_clip( EDITDESC *e )
{
  int ret=0;
  char scrp[120];

  if( !e )
  {
    if( mac_list(0)<0 ) return 0;
  }
  else if( evnt_list(e,0)<0 ) return 0;
  bee();
  if( x_scrp_get( scrp, 1 ) )
  {
    strcpy( spathend(scrp), "SCRAP.TXT" );
    if( open_sh(scrp,1) )
    {
      ret = !e ? selfunc( msaveclip, 0 ) : eselfunc( esaveclip, e, 0 );
      flush_sh(ret);
      close_sh();
      if( !ret ) Fdelete(scrp);
    }
  }
  arrow();
  if( !ret ) alert(BADCLIP);
  return ret;
}

void finish_mac( int i, char *name, KEYCODE *key, int len, char auto_on, long date, int mode )
{
  MACDESC *m;
  long l;
  MACBUF *mb;

  if(len)
  {
    if( (mb = xmalloc( l=len*sizeof(MACBUF) )) == 0 ) return;
  }
  else mb = 0L;
  m = &mdesc[i];
  if( !shift_maclist( i, 1 ) )
  {
    if( mb )
    {
      m->mb = mac_buf;	/* just temporarily, for free */
      free_msgs(m);
      xmfree(mb);
      m->mb = 0L;
    }
    return;
  }
  strcpy( m->name, name );
  m->key = *key;
  m->len = len;
  m->auto_on = auto_on;
  m->automode = mode;
  m->is_global = 0;
  *(long *)&m->date = date;
  if( (m->mb=mb) != 0L ) memcpy( mb, mac_buf, l );
  setup_mac( m );
}

void finish_evnts( EDITDESC *e, int i, int len )
{
  if( shift_evnt( e, i, len, mac_buf ) )
  {
    evnt_tree(e);
    set_elist_ht(e);
    edit_dial(e);
  }
  else evnts_off(e,1);
}

char *skip_spaces( char *ptr )
{
  while( isspace(*ptr) ) ptr++;
  return ptr;
}

#pragma warn -def
void load_clip( EDITDESC *e )
{
  int i, j, len, mode, msnum;
  char scrp[120], draw=0, name[21], *ptr, *s, auto_on, (*msgs)[5][31];
  long date;
  KEYCODE key;
  static char mtype[8][6] = { "TIMER", "BUTTN", "MOUSE", "KEYBD", "SOUND", "DATE", "TIME", "MSG" };
  MACBUF *mb;

  if( !e )
  {
    if( (i=mac_list(0)) < 0 || i>=iglobl ) i = iglobl;
  }
  else if( !test_eglob(e) ) return;
  else if( (i=evnt_list(e,0)) < 0 ) i = e->m->len;
  bee();
  if( x_scrp_get( scrp, 0 ) )
  {
    strcpy( spathend(scrp), "SCRAP.TXT" );
    if( open_sh(scrp,0) )
    {
      name[0] = 0;
      *(long *)&key = 0L;
      mb = mac_buf;
      len = 0;
      auto_on = 0;
      mode = 0;
      date = current_time();
      while( get_shline(scrp) == 1 )
      {
        ptr = skip_spaces(scrp);
        if( !strncmp( ptr, "MACRO", 5 ) && isspace(ptr[5]) )
        {
          if(e) continue;	/* loading just events, so get all */
          if( len )
          {
            finish_mac( i++, name, &key, len, auto_on, date, mode );
            draw = 1;
          }
          mb = mac_buf;
          len = 0;
          auto_on = 0;
          mode = 0;
          date = current_time();
          x_sscanf( skip_spaces(ptr+5), "%S %k %b %d %X", name, &key, &auto_on, &mode, (long *)&date );
        }
        else
        {
          if( (s = strchr( ptr, ' ')) != 0 || (s = strchr( ptr, '\t' )) != 0 ) *s++ = 0;
          else s = ptr+strlen(ptr);
          for( j=0; j<8; j++ )
            if( !strcmp( ptr, mtype[j] ) )
            {
              ptr = skip_spaces(s);
              switch(j)
              {
                case TYPE_TIMER:
                  x_sscanf( ptr, "%D", &mb->u.timer );
                  break;
                case TYPE_BUTTN:
                  x_sscanf( ptr, "%d %d", &mb->u.button.state, &mb->u.button.clicks );
                  break;
                case TYPE_MOUSE:
                  x_sscanf( ptr, "%d %d", &mb->u.mouse.x, &mb->u.mouse.y );
                  break;
                case TYPE_KEY:
                  x_sscanf( ptr, "%x %h %h", &mb->u.key.shift, &mb->u.key.scan, &mb->u.key.ascii );
                  break;
    	        case TYPE_KEY+1:
                case TYPE_KEY+2:
                case TYPE_KEY+3:
                  x_sscanf( ptr, "%x", &mb->u.sound.index );
                  break;
                case TYPE_KEY+4:	/* message */
                  msnum = 0;
                  if( (msgs = xmalloc(5*31)) == 0 ) break;
                  (*msgs)[0][0] = (*msgs)[1][0] = (*msgs)[2][0] = (*msgs)[3][0] = (*msgs)[4][0] = 0;
                  while( get_shline(scrp) == 1 && msnum<5 )
                  {
                    ptr = skip_spaces(scrp);
                    if( strncmp( ptr, "+MSG", 4 ) ) break;
                    (*msgs)[msnum][30] = 0;
                    strncpy( (*msgs)[msnum++], skip_spaces(ptr+4), 30 );
                  }
                  mb->u.msg = msgs;
              }
              mb->type = j <= TYPE_KEY ? j : j+TYPE_SOUND-TYPE_KEY-1;
              mb++;
              len++;
              break;
            }
        }
      }
      if(e)
      {
        if( len ) finish_evnts( e, i, len );
      }
      else if( len )
      {
        finish_mac( i++, name, &key, len, auto_on, date, mode );
        draw = 1;
      }
      close_sh();
    }
  }
  arrow();
  if( draw )
  {
    macs_off(0);
    main_dial();
  }
}
#pragma warn +def

int do_save( int as )
{
  char temp[120];

  if( !as )
    if( *spathend(main_path) )
        return save_gma( main_path, 0 );
  strcpy( temp, main_path );
  if( fselect( temp, "*.GMA", FSSAVAS ) && *spathend(temp) )
    if( save_gma( temp, 1 ) )
    {
      strcpy( main_path, temp );
      is_global = !strcmpi( spathend(temp), "GLOBAL.GMA" );
      main_title();
      return 1;
    }
  return 0;
}
