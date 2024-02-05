#include "string.h"
#include "new_aes.h"
#include "xwind.h"
#include "win_var.h"
#include "win_inc.h"
#include "lerrno.h"
#include "debugger.h"
#include "ctype.h"
#include "stdlib.h"
#define _XTRAS
#ifdef GERMAN
  #include "german\wind_str.h"
#else
  #include "wind_str.h"
#endif

#define SHBUFSIZ	1024

int shgph, shgpph, shbuflen, shglen;
static int gc_ihand;
char shfile[120], tempfile[120], nums[]="0123456789ABCDEF", *shbptr, shbuf[16], sh_eof, sh_first;

/*********************** x_rsrc_load ****************************/
#define X_RLOPEN        0       /* Open resource file */
#define X_RLGADDR       1       /* Load and get tree address */
#define X_RLFREE        2       /* Free tree */
#define X_RLCLOSE       3       /* Close file */

int x_rsrc_load( int mode, int type, int index, void *addr );
/* THIS FUNCTION IS NOT YET IMPLEMENTED */

typedef struct Vdi_hand
{
  struct Vdi_hand *next;
  int handle;
  APP *app;
} VDI_HAND;
VDI_HAND *vdi_track;

void close_vdi( int close, VDI_HAND *prev, VDI_HAND *v )
{
  /* must remove from list first, because track_vdi() will be called
     during v_clsvwk() */
  if( !prev ) vdi_track = v->next;
  else prev->next = v->next;
  if( close )
    if( v->handle&0x8000 ) v_clswk( v->handle&0x7fff );
    else v_clsvwk( v->handle );
  lfree(v);
}

void track_vdi( int hand, int mode )
{
  VDI_HAND *v, *prev;

  if( mode&0xff00 ) hand |= 0x8000;
  if( !(char)mode )
  {
    /* make sure there is not a duplicate handle */
    for( v=vdi_track; v; v=v->next )
      if( v->handle==hand )
      {
        v->app = curapp;
        return;
      }
    if( (v = (VDI_HAND *)lalloc(sizeof(VDI_HAND),-1)) != 0 )
    {
      v->handle = hand;
      v->app = curapp;
      v->next = vdi_track;
      vdi_track = v;
    }
  }
  else
    for( prev=0L, v=vdi_track; v; prev=v, v=v->next )
      if( v->handle==hand )
      {
        close_vdi( 0, prev, v );
        return;
      }
}

char *skip_num( char *buf )
{
  while( strchr(nums,*buf) ) buf++;
  return buf;
}

typedef struct
{
  char *buf, *fmt;
  int **arg;
} SCAN_STR;

int sscnf_( SCAN_STR *s )
{
  char *buf = s->buf;
  char *fmt = s->fmt;
  int **arg = s->arg;
  register int cnt=0, base;
  register unsigned long i;
  register char *ptr, c, larg;
  char neg;
  unsigned int v1, v2;

  while( *fmt && *buf )
  {
    base = 10;
    larg = 2;
    neg=0;
    while( *fmt == *buf && *buf )
    {
      fmt++;
      buf++;
    }
    if( isspace(*fmt) ) while( isspace(*buf) ) buf++;
    else
      if( *fmt == '%' )
      {
        switch( *(++fmt) )
        {
          case 'b':
            *(char *)*arg++ = *buf++ == 'T' ? 1 : 0;
            break;
          case 'k':
            xscan( buf, "%h %h %h", &(*(KEYCODE **)arg)->shift,
               &(*(KEYCODE **)arg)->scan, &(*(KEYCODE **)arg)->ascii );
            arg++;
            buf = skip_num(skip_num(skip_num(buf)+1)+1);
            break;
          case 'v':
            xscan( buf, "%x.%x", &v1, &v2 );
            **arg++ = (v1<<8)+v2;
            buf = skip_num(skip_num(buf)+1);
            break;
          case 'h':
            larg = 0;
          case 'X':
            base = 16;
            goto get;
          case 'x':
            base = 16;
          case 'd':
            larg = 1;
          case 'D':
get:        i=0L;
            while( *buf && !isspace(*buf) )
            {
              c = *buf;
              if( !i && c=='-' ) neg++;
              else
              {
                if( c>='a' && c<='f' ) c &= 0x5F;
                if( (ptr=strchr(nums,c)) != 0 && (long)ptr-(long)nums < base )
                    i = i*base + (long)ptr - (long)nums;
                else break;
              }
              buf++;
            }
            if( neg ) i = -i;
            if( larg==2 ) **((long **)arg++) = i;
            else if( larg ) **arg++ = i;
            else **((char **)arg++) = i;
            break;
          case 's':
            ptr = (char *)*arg++;
            while( *buf && !isspace(*buf) ) *ptr++ = *buf++;
            *ptr++ = '\0';
            break;
          case 'S':
            if( !*buf ) break;
            ptr = (char *)*arg++;
            buf++;
            while( *buf && *buf!='}' ) *ptr++ = *buf++;
            if( *buf ) buf++;
            *ptr++ = '\0';
            break;
          case 'c':
            *(char *)*arg++ = *buf++;
            break;
        }
        cnt++;
      }
    fmt++;
  }
  return(cnt);
}

int xscan( char *buf, char *fmt, ... )
{
  SCAN_STR s;
  s.buf = buf;
  s.fmt = fmt;
  s.arg = (int **)&...;
  return( sscnf_(&s) );
}

char *pn( char *b, unsigned long n, int base, int w, int sign, int pad, int lj )
{
  int i;
  char nb[20];
  switch (base) {
  case 'o':
    base = 8;
    break;
  case 'x':
  case 'h':
  case 'X':
    base = 16;
    break;
  default:
    base = 10;
  }
  i = 0;
  if( !n ) nb[i++] = '0';
  else
  {
    if( sign && (n&0x80000000L) ){
      n = (unsigned long)(-(long)n);
    } else sign = 0;
    for (; n; n /= base)
      nb[i++] = "0123456789ABCDEF"[n%base];
    if( sign ) nb[i++] = '-';
  }
  w -= i;
  if( !lj ) while (w-- > 0) *b++ = pad;
  else while (w-- > 0) nb[i++] = pad;
  while (i--) *b++ = nb[i];
  return b;
}

typedef struct
{
  char *buf, *fmt;
  unsigned int *ap;
} SPF_STR;

void _spf( SPF_STR *s ) {
  char *buf=s->buf,
       *fmt=s->fmt;
  unsigned int *ap = s->ap;
  char **pp, *ps, pad, lj, sign, larg;
  unsigned long n, *lp;
  register int c, w;

  while( (c = *fmt++) != 0 ){
    if (c == '%') {
      if( (lj = ((c=*fmt++) == '-')) != 0 ) c = *fmt++;
      if( c == '0' ){
        pad = '0';
        c = *fmt++;
      }
      else pad = ' ';
      w = 0;
      sign = 1;
      while (c >= '0' && c <= '9'){
        w = w*10 + c-'0';
        c = *fmt++;
      }
      if( (larg = (c == 'l')) != 0 ) c = *fmt++;
      switch (c) {
        case 'c': *buf++ = *ap++;
            break;
        case 'b': *buf++ = *ap++ != 0 ? 'T' : 'F';
            break;
        case 'k':
            spf( buf, "%02x %02x %02x", (*(KEYCODE **)ap)->shift,
                (*(KEYCODE **)ap)->scan, (*(KEYCODE **)ap)->ascii );
            ((KEYCODE **)ap)++;
            buf += strlen(buf);
            break;
        case 'v':
            spf( buf, "%x.%02x", *ap>>8, *ap&0xff );
            ap++;
            buf += strlen(buf);
            break;
        case 'S': *buf++ = '{';
        case 's': pp = (char **) ap;
            ps = *pp++;
            w -= strlen(ps);
            if( !lj ) while (w-- > 0) *buf++ = pad;
            ap = (unsigned int *) pp;
            while (*ps) *buf++ = *ps++;
            if( lj ) while (w-- > 0) *buf++ = pad;
            if( c=='S' ) *buf++ = '}';
            break;
        case 'x':
            sign=0;
            n = (unsigned long) *ap++;
            goto do_pn;
        case 'h':
            sign=0;
            n = (unsigned long)(unsigned char)*ap++;
            goto do_pn;
        case 'X': sign=0;
        case 'D': larg++;
        case 'd': case 'o':
            if (larg) {
            lp = (unsigned long *)ap;
            n = *lp++;
            ap = (unsigned int *)lp;
            }
            else n = (long)(signed) *ap++;
do_pn:      buf = pn(buf, n, c, w, sign, pad, lj);
            break;
        default:  *buf++ = c;
            break;
      }
    }
    else  *buf++ = c;
  }
  *buf = 0;
}

void spf( char *buf, char *fmt, ... )
{
  SPF_STR s;
  s.buf = buf;
  s.fmt = fmt;
  s.ap = (unsigned int *)&...;
  _spf(&s);
}

int x_settings( int getset, int len, SETTINGS *user )
{
  if( (unsigned)len>(unsigned)sizeof(SETTINGS) ) len=sizeof(SETTINGS);
  if( getset==-1 ) memcpy( &settings, &dflt_settings, len );
  else if( getset )
  {
    if( len>4 ) memcpy( (int *)&settings.struct_len+1,
        (int *)&user->struct_len+1, len-4 );
  }
  else if( user ) memcpy( user, &settings, len );
  return 1;
}

int gem_cnf( int mode, char *buf )
{
  int tempi, tempo, ret=0;

  tempi = shgph;
  tempo = shgpph;
  shgph = gc_ihand;
  shgpph = 0;
  switch( mode )
  {
    case 0:
      shgph = 0;
      open_sh("GEM.CNF");
      break;
    case 1:
      ret = get_shline(buf);
      break;
    case 2:
      close_sh(0);
  }
  gc_ihand = shgph;
  shgph = tempi;
  shgpph = tempo;
  return ret;
}

void close_sh( int delete )
{
  if( shgph>0 )
  {
    Fclose(shgph);
    shgph = 0;
    if( shbptr != shbuf ) lfree(shbptr);
  }
  if( shgpph>0 )
  {
    Fclose(shgpph);
    if( delete ) Fdelete( tempfile );
    else
    {
      Fdelete( shfile );
      Frename( 0, tempfile, shfile );
    }
    shgpph = 0;
  }
  shgp_app = 0;
}

void open_sh( char *name )
{
  static char tried;

  do
  {
    if( !tried )
    {
      tried++;
      strcpy( shfile, loadpath );
    }
    else if( tried==1 )
    {
      tried++;
      strcpy( shfile+3, "GENEVA\\" );
    }
    strcpy( pathend(shfile), name );
    shgph = Fopen(shfile,2);
  }
  while( shgph<0 && tried<2 );
  shglen = 0;
  sh_eof = 0;
  sh_first = 1;		/* 004 */
  if( shgph<0 )		/* 004 */
  {
    tried = 0;
    return;
  }
  tried = 2;		/* 005? */
  if( (shbptr = lalloc(shbuflen=SHBUFSIZ,-1)) == 0 )
  {
    shbuflen = sizeof(shbuf);
    shbptr = shbuf;
  }
}

int sh_getc( char *buf )
{
  static char *shgptr;
  int ret;

  if( !shglen )
    if( (shglen = Fread( shgph, shbuflen, shgptr=shbptr )) < 0 )
    {
      ret = shglen;
      shglen = 0;
      return ret;
    }
    else if( !shglen )
      if( !sh_eof++ )	/* 004 */
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

int put_shline( char *buf )
{
  long l;

  if( shgpph>0 )
  {
    if( (l=strlen(buf)) != 0 && Fwrite( shgpph, l, buf ) != l ||
        Fwrite( shgpph, 2, "\r\n" ) != 2 ) return -2;
    return 1;
  }
  return 0;
}

int find_shline( char *buf, int write )
{
  int ret;
  char temp[SHLINELEN], cmp;

  while( (ret=_get_shline(temp))==1 )
  {
    cmp = buf && !strncmp( temp, buf, strlen(buf) );
    if( !temp[0] ) continue;	/* 004: skip blank lines */
    if( write>0 || write<0 && cmp )
    {
      if( temp[0]=='[' )	/* 004: cr before [name] */
        if( sh_first ) sh_first = 0;
        else if( (ret=put_shline("")) <= 0 ) return ret;
      if( (ret=put_shline(temp)) <= 0 ) return ret;
    }
    if( cmp ) return 1;
  }
  return ret;
}

int scan32( char *buf, int *arr, int ret )
{
  char *p;
  int i, j;

  for( j=2; --j>=0; )
    if( ret>0 && (ret = x_shel_get( X_SHACCESS, SHLINELEN, p=buf )) > 0 )
      for( i=16; --i>=0; )
      {
        xscan( p, "%x", arr++ );
        while( *p && !isspace(*p) ) p++;
        while( *p && isspace(*p) ) p++;
      }
  return ret;
}

int sh_get_line( int *ret, char *fmt, ... )
{
  char buf[SHLINELEN];
  SCAN_STR s;

  if( *ret<=0 || (*ret = x_shel_get( X_SHACCESS, SHLINELEN, buf )) <= 0 )
      return 0;
  s.buf = buf;
  s.fmt = fmt;
  s.arg = (int **)&...;
  sscnf_(&s);
  return 1;
}

int x_shel_get( int mode, int len, char *buf )
{  /* return  -3: bad version;  -2: disk error;  -1: already open;  0: not found;  1: OK */
  char temp[SHLINELEN];
  int ret, i, j, ver, nflags;
  APPFLAGS apf;

  switch( mode )
  {
    case X_SHLOADSAVE:
      if( (ret = x_shel_get( X_SHOPEN, 0, "Geneva" )) > 0 &&
          (ret = x_shel_get( X_SHACCESS, SHLINELEN, temp )) > 0 )
      {
        xscan( temp, "%v", &ver );
        if( ver < CNF_VER || ver > SET_VER )
        {
          if( desktop ) _form_alert( 1, BADSET );
          x_shel_get( X_SHCLOSE, 0, 0L );
          return -3;
        }
        i = settings.boot_rez;
        j = settings.falcon_rez;
        if( sh_get_line( &ret, "%x %x", &settings.boot_rez, &settings.falcon_rez ) )
        {
          sh_get_line( &ret, "%x %d", &settings.flags.i, &settings.gadget_pause );
          if( settings.boot_rez==-1 || settings.flags.s.ignore_video_mode )
          {
            settings.boot_rez = i;
            settings.falcon_rez = j;
          }
          for( i=-5; i<13; i++ )
            if( !sh_get_line( &ret, "%k", &settings.wind_keys[i] ) ) break;
          sh_get_line( &ret, "%X %X %X %X", &settings.color_3D[0].l, &settings.color_3D[1].l,
                &settings.color_3D[2].l, &settings.color_3D[3].l );
          sh_get_line( &ret, "%X %X %X %X", &settings.color_root[0].l, &settings.color_root[1].l,
                &settings.color_root[2].l, &settings.color_root[3].l );
          sh_get_line( &ret, "%X %X %X %X", &settings.color_exit[0].l, &settings.color_exit[1].l,
                &settings.color_exit[2].l, &settings.color_exit[3].l );
          sh_get_line( &ret, "%X %X %X %X", &settings.color_other[0].l, &settings.color_other[1].l,
                &settings.color_other[2].l, &settings.color_other[3].l );
          sh_get_line( &ret, "%h %S", &settings.sort_type, settings.find_file );
          if( ret>0 ) for( i=0; i<10 && (ret = x_shel_get( X_SHACCESS, sizeof(settings.fsel_path[i]),
              settings.fsel_path[i] )) > 0; i++ );
          if( ret>0 ) for( i=0; i<10 && (ret = x_shel_get( X_SHACCESS, sizeof(settings.fsel_ext[i]),
              settings.fsel_ext[i] )) > 0; i++ );
          for( i=0; i<9; i++ )
            if( !sh_get_line( &ret, "%d %d %d %d", &fontinfo[i].font_id, &fontinfo[i].point_size,
                &fontinfo[i].gadget_wid, &fontinfo[i].gadget_ht ) ) break;
          if( sh_get_line( &ret, "%d", &nflags ) )
          {
            while( x_appl_flags( 2, 1, 0L ) );  /* delete all current */
            for( j=0; --nflags>=0; j=-1 )
            {
              if( !sh_get_line( &ret, "%S %S %X %k %k %k %k", &apf.name, &apf.desc, &apf.flags.l, &apf.open_key,
                  &apf.reserve_key[0], &apf.reserve_key[1], &apf.reserve_key[2] ) ) break;
              x_appl_flags( 1, j, &apf );
            }
          }
          for( i=0; i<3; i++ )
          {
            ret = scan32( temp, dwcolors[i][0], ret );
            ret = scan32( temp, dwcolors[i][1], ret );
            ret = scan32( temp, wstates[i], ret );
          }
          dflt_wstates();
          sh_get_line( &ret, "%k %k %k", &settings.cycle_in_app, &settings.iconify,
              &settings.alliconify );
          sh_get_line( &ret, "%h", &settings.graymenu );
          sh_get_line( &ret, "%k", &settings.procman );		/* 006 */
          sh_get_line( &ret, "%X", &settings.flags2.l );	/* 006 */
        }
        if( ret>0 ) ret = x_shel_get( X_SHCLOSE, 0, 0L );
      }
      return ret;
    case X_SHOPEN:
      if( shgph > 0 || gc_ihand > 0/*006*/ )
      {
/* 004        DEBUGGER(XSHGE,SHUSED,shgp_app); */
        return -1;                                      /* already open */
      }
      open_sh("GENEVA.CNF");
      if( shgph < 0 ) return 0;                         /* not found */
      shgp_app = curapp->id;
      spf( temp, "[%s]", buf );
      ret = find_shline(temp,0);
      break;
    case X_SHACCESS:
      if( (ret=get_shline(temp)) < 0 ) break;           /* read error */
      if( temp[0]=='[' )
      {
        close_sh(0);	/* 004 */
        return 0;                      /* end of section */
      }
      strncpy( buf, temp, len-1 );
      buf[len-1] = 0;
      return 1;
    case X_SHCLOSE:
      close_sh(0);
      return 1;
    default:
      DEBUGGER(XSHGE,UNKTYPE,mode);
      return 0;
  }
#ifdef DEBUG_ON
  if( ret == -2 ) DEBUGGER(XSHGE,DISKERR,shgph);
#endif
  if( ret<=0 ) close_sh(0);
  return ret;
}

int x_put( char *fmt, ... )
{
  char temp[SHLINELEN];

  SPF_STR s;
  s.buf = temp;
  s.fmt = fmt;
  s.ap = (unsigned int *)&...;
  _spf(&s);
  return put_shline(temp);
}

void put_arr( int *arr, int num )
{
  char fmt[50];
  int i;

  while( num>0 )
  {
    fmt[0] = 0;
    for( i=0; i<16 && i<num; i++ )
      strcat( fmt, i ? " %x" : "%x" );
    arr += i;
    x_put( fmt, *--arr, *--arr, *--arr, *--arr,  *--arr, *--arr, *--arr, *--arr,
         *--arr, *--arr, *--arr, *--arr, *--arr, *--arr, *--arr, *--arr );
    arr += i;
    num -= i;
  }
}

int cmp_flag( _APPFLAGS **a, _APPFLAGS **b )
{
  return strcmp( (*a)->flags->name, (*b)->flags->name );
}

int x_shel_put( int mode, char *buf )
{
  char temp[SHLINELEN];
  int ret, i, j;
  _APPFLAGS *apfp, **flg;

  switch( mode )
  {
    case X_SHLOADSAVE:
      if( (ret = x_shel_put( X_SHOPEN, "Geneva" )) > 0 )
      {
        x_put( "%v", SET_VER );
        x_put( "%02x %04x", settings.boot_rez, settings.falcon_rez );
        x_put( "%x %d", settings.flags.i, settings.gadget_pause );
        for( i=-5; i<13; i++ )  /* under-index the array */
          x_put( "%k", &settings.wind_keys[i] );
        x_put( "%X %X %X %X", settings.color_3D[0].l, settings.color_3D[1].l,
            settings.color_3D[2].l, settings.color_3D[3].l );
        x_put( "%X %X %X %X", settings.color_root[0].l, settings.color_root[1].l,
            settings.color_root[2].l, settings.color_root[3].l );
        x_put( "%X %X %X %X", settings.color_exit[0].l, settings.color_exit[1].l,
            settings.color_exit[2].l, settings.color_exit[3].l );
        x_put( "%X %X %X %X", settings.color_other[0].l, settings.color_other[1].l,
            settings.color_other[2].l, settings.color_other[3].l );
        x_put( "%h %S", settings.sort_type, settings.find_file );
        for( i=0; i<10; i++ )
          put_shline(settings.fsel_path[i]);
        for( i=0; i<10 && (ret = put_shline(settings.fsel_ext[i])) > 0; i++ );
        for( i=0; i<9; i++ )
          x_put( "%d %d %d %d", fontinfo[i].font_id, fontinfo[i].point_size,
              fontinfo[i].gadget_wid, fontinfo[i].gadget_ht );
        for( i=0, apfp=flags_list; apfp; apfp=apfp->next, i++ );
        x_put( "%d", i );
        if( i>1 && (flg = (_APPFLAGS **)lalloc(i*sizeof(long),-1)) != 0 )  /* 004 */
        {
          for( j=0, apfp=flags_list; j<i-1; j++ )
            flg[j] = apfp = apfp->next;
          qsort( flg, i-1, sizeof(flg[0]), cmp_flag );
          apfp=flags_list;
          for( j=0; j<i-1; j++ )
            apfp = apfp->next = flg[j];
          apfp->next = 0L;
          lfree(flg);
        }
        for( apfp=flags_list; --i>=0; apfp=apfp->next )
          x_put( "%S %S %X %k %k %k %k", apfp->flags->name, apfp->flags->desc,
              apfp->flags->flags.l, &apfp->flags->open_key, &apfp->flags->reserve_key[0],
              &apfp->flags->reserve_key[1], &apfp->flags->reserve_key[2] );
        for( i=0; i<3; i++ )
        {
          put_arr( dwcolors[i][0], WGSIZE+1 );
          put_arr( dwcolors[i][1], WGSIZE+1 );
          put_arr( wstates[i], WGSIZE+1 );
        }
        x_put( "%k %k %k", &settings.cycle_in_app, &settings.iconify, &settings.alliconify );
        x_put( "%h", settings.graymenu );
        x_put( "%k", &settings.procman );	/* 006 */
        x_put( "%X", settings.flags2.l );	/* 006 */
        if( ret>0 ) ret = x_shel_put( X_SHCLOSE, 0L );
      }
      return ret;
    case X_SHOPEN:
      if( shgph > 0 || gc_ihand > 0/*006*/ )
      {
        DEBUGGER(XSHPU,SHUSED,shgp_app);
        return -1;                      /* already open */
      }
      open_sh("GENEVA.CNF");
      if( shgph<0 && shgph != AEFILNF )
      {
        ret = -2;       /* not found */
        break;
      }
      strcpy( tempfile, loadpath );
      strcat( tempfile, "GENEVA.$$$" );
      if( (shgpph = Fcreate(tempfile,0)) < 0 )
      {
        ret = -2;                                       /* can't create temp */
        break;
      }
      shgp_app = curapp->id;
      spf( temp, "[%s]", buf );
      if( (ret=find_shline(temp,1))==0 )
        if( sh_first || (ret=put_shline("")) > 0 )	/* 004 */
        {
          sh_first = 0;
          ret=put_shline(temp);
        }
      break;
    case X_SHACCESS:
      ret = put_shline(buf);
      break;
    case X_SHCLOSE:
      if( (ret = find_shline("[",-1)) == 1 ) ret = find_shline(0L,1);
      close_sh( ret<0 );
      break;
    default:
      DEBUGGER(XSHPU,UNKTYPE,mode);
      return 0;
  }
#ifdef DEBUG_ON
  if( ret == -2 ) DEBUGGER(XSHPU,DISKERR,shgph);
#endif
  if( ret <= 0 ) close_sh( shgpph>0 );
  return ret;
}

void free_vdi( APP *ap )
{
  VDI_HAND *v, *prev;

  for( prev=0L, v=vdi_track; v; )
    if( v->app==ap )
    {
      v = v->next;
      close_vdi( 1, prev, prev ? prev->next : vdi_track );
    }
    else
    {
      prev = v;
      v = v->next;
    }
}

void _x_appl_free(void)
{
  APP *ap, *ap2;

  while( free_ap )
  {
    ap2 = free_ap->next;
/*    if( free_ap->basepage )
    {
      Mfree(free_ap->basepage->p_env);
      Mfree(free_ap->basepage);
    } */
    ap = curapp;
    free_vdi( curapp = free_ap );
    if( multitask ) appl_exit();
    del_acc_name(-1);
    curapp = ap;
    lfreeall(free_ap->id);
    if( free_ap==mouse_last ) mouse_last = 0L;	/* 004 */
    if( free_ap==last_curapp ) last_curapp = 0L;	/* 004 */
    lfree(free_ap);
    free_ap = ap2;
  }
}

void exit_msg( int term_id, int to_id, int ret )
{
  int buf[8];

  if( to_id>=0 )
  {
    buf[0] = CH_EXIT;
    buf[2] = 0;
    buf[3] = term_id;
    buf[4] = ret;
    _appl_write( buf[1]=to_id, 16, buf, 1 );
  }
}

int __x_appl_term( int id, int ret, int alert )
{
  return _x_appl_term( id, ret, alert, 0 );
}

int _x_appl_term( int id, int ret, int alert, int in_trap1 )
{
  APP *ap, *ap2, *ap3, *ap4;
  int parent;
  char term_it=0;

  if( id == -1 ) return sleeping ^= 1;
  if( !in_trap1 ) _x_appl_free();
  if( (ap = find_ap(id)) != 0 )
      if( alert && !(ap->ap_msgs&1) && _form_alert( 1, XTERM ) == 2 )
          return 0;
      else if( alert && (ap->ap_msgs&1) && sspf_alert( XTERM2,
          ap->dflt_acc+2 ) == 2 ) return 0;
      else if( alert || ap != curapp )
      {
        to_sleep( ASL_USER, 0, ap );	/* wake it up */
        send_term( ap, AP_TERM, 1 );
        if( ap->ap_type==4 ) ap->ap_msgs &= ~1;	/* 004: if DA, throw out if ignores AP_TERM */
        ap->ap_type = unload = 99;
        return 1;               /* let dispatch() terminate it */
      }
      else
      {
/*        if( ap->flags.flags.s.exit_redraw )
        {
          draw_menu();
          redraw_all( &desktop->outer );
        } */
        parent = ap->parent_id;
        if( !unload )
        {
          strcpy( lastapp_name, ap->dflt_acc+2 );
          lastapp_type = ap->ap_type;
          exit_msg( lastapp_id=id, parent, ret );
        }
        ap2 = curapp;
        if( ap==shell_app ) shell_app=0L;
        if( !in_trap1 )
        {
          lfreeall(id);
          if( curapp->basepage )
          {
            Mfree(curapp->basepage->p_env);
            Mfree(curapp->basepage);
          }
          curapp = ap;
          appl_exit();
          del_acc_name(-1);
          free_vdi(ap);		/* 004: used to happen if in_trap1 */
        }
        if( update.i && ap==has_update )  /* 004: was ap->update.i */
        {
          curapp = ap;
/*%          cnt_update.i -= ap->update.i;
          ap->update.i = 0;
          find_oldup(ap);  004 */
          cnt_update.i = 0;
          set_update();
          unblock();	/* 004 */
        }
        no_interrupt++;
        if( ap->parent != 0 )
        {
          /* 004: was  ap->parent->child = ap->child; */
          for( ap3=ap->parent; ap3->child; ap3=ap3->child )
            if( ap3->child==ap )
            {
              ap3->child = ap->child;
              break;
            }
          /* ap->parent->has_wind = ap->has_wind;   rel 003 */
          if( (ap3=ap->parent)->asleep&ASL_PEXEC )
          {
            /* last child (probably ap->parent) is now awake */
            for( ; ap3->child; ap3=ap3->child );
          }
          to_sleep( ASL_SINGLE|ASL_PEXEC, 0, ap3 );
	  ap->flags.flags.s.keep_deskmenu = 0;		/* 005: fixes top_menu() */
          set_multi( ap3->flags.flags.s.multitask );
          if( multitask ) close_all( ap, 1 );  /* clear any winds left by app */
/*          ap4 = curapp;
          curapp = ap3;
          _menu_register( ap3->id, 0L );
          curapp = ap4; */
/*%           if( update.i && ap==has_update )
          {
            has_update = ap3;
            (has_update = ap3)->update = ap->update;
            ap3->old_update = ap->old_update;
          } 004 */
        }
        if( ap->prev ) ap->prev->next = ap->next;
        else app0 = ap->next;
        if( ap->next ) ap->next->prev = ap->prev;
        if( ap->child )
          if( (ap->child->parent = ap->parent) == 0L )
          { /* parent is dead, so give it to Geneva menu */
            ap->child->parent = find_ap(1);
          }
        for( ap3=app0; ap3; ap3=ap3->next )	/* 004: make sure someone is awake */
          if( !ap3->asleep ) break;
        if( !ap3 ) to_sleep( -1, 0, find_ap(parent) );	/* nobody awake, wake up parent */
        if( (!has_menu || has_menu->id != parent) && (ap3=find_ap(parent)) != 0 )		/* 005 */
            cycle_top(ap3);
        no_interrupt--;
        /* used to free ap here before 004 */
        if( ap!=ap2 ) set_curapp(ap2);
        else
        {
          if( multitask ) _graf_mouse( X_MRESET, 0L, 0 );
          curapp = 0L;
          if( preempt ) term_it++;
        }
        if( ap==has_desk ) no_desk_own(1);
        if( ap->flags.flags.s.exit_redraw )
        {
          draw_menu();
          redraw_all( &desktop->outer );
        }
        for( ap3=app0; ap3; ap3=ap3->next )
          /* parent is going away, so send term msgs to its parent */
          if( ap3->parent_id == id ) ap3->parent_id = ap->parent_id;
        if( multitask )
          if( ap==has_menu )
          {
            ap3 = find_ap(parent);
            if( ap3 && ap3->menu && !ap3->asleep ) switch_menu(ap3);
            else
            {
              has_menu = 0L;
              top_menu();
            }
          }
          else if( !has_desk ) new_desk(-1,0L);
        if( ap==unloader ) unloader=0L;
        if( ap==help_app ) help_app=0L;
        if( !in_trap1 )
        {
          if( ap==mouse_last ) mouse_last = 0L;
          if( last_curapp==ap ) last_curapp = 0L;	/* 004 */
          lfree(ap);
        }
        else
        {
          ap->next = free_ap;
          free_ap = ap;
        }
/*        if( term_it && !in_trap1 ) Pterm(ret);*/
        return 1;
      }
  return 0;
}

APPFLAGS *find_flags( char *name )
{
  _APPFLAGS *apfp;

  name = pathend(name);
  for( apfp=flags_list->next; apfp; apfp=apfp->next )
    if( pnmatch( name, apfp->flags->name ) ) return apfp->flags;
  return 0L;
}

void new_flags(void)
{
  APPFLAGS *apf;

  if( (apf=find_flags(curapp->path)) != 0 )
      memcpy( &curapp->flags, apf, sizeof(APPFLAGS) );
  _menu_register( curapp->id, 0L );
}

APPFLAGS *loc_flags( int by_id, int index )
{
  _APPFLAGS *out;
  APP *ap;

  if( by_id )
  {
    if( (ap = find_ap(index)) != 0 ) return &ap->flags;
    return 0L;
  }
  else
  {
    for( out=flags_list; out && (--index>=0); out=out->next );
    return out ? out->flags : 0L;
  }
}

int x_appl_flags( int getset, int index, APPFLAGS *flags )
{
  APPFLAGS *apf;
  _APPFLAGS *apfp, *last;

  switch( getset )
  {
    case X_APF_GET_INDEX:     /* get by index */
      if( (apf=loc_flags(0,index)) == 0 ) return 0;
      if( flags ) memcpy( flags, apf, sizeof(APPFLAGS) );
      break;
    case X_APF_SET_INDEX:     /* set by index */
      if( index<0 )
      {
        if( (apfp=(_APPFLAGS *)lalloc(sizeof(_APPFLAGS),-1)) == 0 ) return 0;
        if( (apf=(APPFLAGS *)lalloc(sizeof(APPFLAGS),-1)) == 0 )
        {
          lfree(apfp);
          return 0;
        }
        for( last=flags_list; last->next; last=last->next );
        last->next = apfp;
        apfp->next = 0L;
        apfp->flags = apf;
      }
      else if( (apf=loc_flags(0,index)) == 0 ) return 0;
      if( flags )
        if( index ) memcpy( apf, flags, sizeof(APPFLAGS) );
        else memcpy( &apf->flags, &flags->flags, sizeof(APPFLAGS)-
           sizeof(apf->name)-sizeof(apf->desc) );
      break;
    case X_APF_DEL_INDEX:     /* delete by index */
      if( index<=0 ) return 0;
      for( apfp=flags_list; apfp && (--index>0); apfp=apfp->next );
      if( apfp && apfp->next )
      {
        last = apfp->next->next;        /* access before lfree */
        lfree(apfp->next->flags);
        lfree(apfp->next);
        apfp->next = last;
      }
      else return 0;
      break;
    case X_APF_GET_ID:     /* get by ID */
      if( (apf=loc_flags(1,index)) == 0 ) return 0;
      if( flags ) memcpy( flags, apf, sizeof(APPFLAGS) );
      break;
    case X_APF_SET_ID:     /* set by ID */
      if( (apf=loc_flags(1,index)) == 0 ) return 0;
      if( flags ) memcpy( apf, flags, sizeof(APPFLAGS) );
      break;
    case X_APF_SEARCH:
      if( !flags ) return 0;
      if( (apf=find_flags(flags->name)) == 0L ) apf = &dflt_flags;
      memcpy( flags, apf, sizeof(APPFLAGS) );
      break;
    default:
      return 0;
  }
  return 1;
}

int toggle_multi( APP *ap, int sleep, int alert )
{
  APP *ap2;

  if( !multitask && (!ap || ap->ap_type!=4) )
  {  /* enter MT mode */
    if( alert && settings.flags.s.alert_mode_change &&
        _form_alert( 1, TOGL2MT ) == 2 ) return 1;
    for( ap2=app0; ap2; ap2=ap2->next )
      if( ap2->ap_type!=4 && !ap2->asleep )
      {
        to_sleep( ASL_SINGLE|ASL_USER, ASL_USER, ap2 );
        break;
      }
    if( !multitask && ap/*005*/ && !ap->flags.flags.s.multitask ) goto single;
    no_desk_own(1);
    has_menu = 0L;
    set_multi(1);
/*    if( !desktop->tree ) draw_desk();*/
    accs_obj(1);
    if( sleep<0 ) return -1;
    top_menu();
    return 1;
  }
  if( multitask && sleep<=0 && !ap->flags.flags.s.multitask )
  {  /* enter ST mode */
    if( alert && settings.flags.s.alert_mode_change &&
        _form_alert( 1, TOGL2ST ) == 2 ) return 1;
    ap2 = curapp;
    curapp = ap;
    set_multi(0);
    curapp = ap2;
single:
    to_sleep( ASL_SINGLE|ASL_USER, 0, ap );
    accs_obj(1);
    switch_menu( ap->menu ? ap : 0L );
    new_desk( -1, ap );
    return 1;
  }
  return 0;
}

int x_appl_sleep( int id, int sleep )
{
  APP *ap;
  int ret;

  ap = find_ap(id);
  if( !ap || !sleep && ap->asleep&4 ) return -1;
  ret = (ap->asleep!=0) | (ap->flags.flags.s.multitask<<1);
  if( (unsigned)sleep <= 1 && !toggle_multi(ap,sleep<<1,1) )
    if( /*(ap->ap_type==4 || multitask) &&*/ (ap!=has_menu || sw_next_app(ap)) )
    {
      to_sleep( ASL_USER, sleep<<1, ap );
      accs_obj(1);
    }
    else return -1;
  return ret;
}

int x_form_filename( OBJECT *tree, int obj, int to_from, char *string )
{
  int i=0, max;
  char *s, *s2;

  s = obj>=0 ? u_ptext(tree,obj) : (char *)tree;
  if( !to_from )
  {
    max = 12;	/* 004 */
    if( (s2=strchr( string, '.' ))==0 || s2-string > 8 ) max = 8;	/* 004 */
    for( ; i<max && *string; i++ )	/* 004: was 12 */
      if( *string != '.' ) *s++ = *string++;
      else if( i<8 ) *s++ = ' ';
      else string++;
    *s = '\0';
  }
  else
    do
    {
      if( ++i == 9 && *s ) *string++ = '.';
      if( *s != ' ' ) *string++ = *s;
    }
    while( *s++ );
  return 1;
}

#define AC_HELP     1025            /* message to Help DA */

int _x_help( char *topic, char *helpfile, int sensitive )
{
  MSGQ *ptr;
  int msg[8];
  char *ptr2;

  if( !help_app )
  {
    if( !_shel_write( SH_RUNHELP, 0, sensitive, helpfile, topic ) ) return 0;
    return 1;
  }
  if( _shel_envrn( environ, &ptr2, "SHOWHELP" ) &&
      strncmpi( pathend(ptr2), "GNVA", 4 ) )		/* 005 */
  {
    msg[0] = AC_HELP;
    msg[1] = 1;			/* sent by G mgr */
    msg[2] = 0;
    *(char **)&msg[3] = topic;
    return _appl_write( help_app->id, 16, msg, 0 );
  }
  if( !helpfile ) helpfile = "GENEVA.HLP";
  msg[0] = X_GET_HELP;
  msg[2] = (msg[3] = strlen(topic)+1) + (msg[4] = strlen(helpfile)+1);
  msg[5] = sensitive;
  if( _appl_write( msg[1] = help_app->id, msg[2]+16, msg, 0 ) )
  {
    for( ptr=msg_q; ptr->next; ptr=ptr->next );
    strcpy( (char *)ptr->buf+16, topic );
    strcpy( (char *)ptr->buf+16+msg[3], helpfile );
    return 1;
  }
  return 0;
}

void wdraw( int dev )
{
  int msg[8];
  MSGQ *m;

  msg[0] = SH_WDRAW;
  *(long *)&msg[1] = 0L;
  for( m=msg_q; m; m=m->next )
    if( *(int *)(m->buf) == SH_WDRAW && *((int *)(m->buf)+3) == dev ) return;  /* 004: was || */
  if( !m )
  {
    msg[3] = dev;
    msg[4] = curapp->id;
    _appl_write( msg[1] = shell_app->id, 16, msg, 1 );
  }
}

void pathtest( char *path )
{
  if( shell_app && settings.flags.s.auto_update_shell )
    if( *(path+1)==':' ) wdraw( (*path&0x5f)-'A' );
    else if( *(path+3)!=':' ) wdraw( Dgetdrv() );
}

void dflt_wstates(void)
{
  OBJECT *o;
  int i;

  if( wcolor_mode>=0 )
    for( i=0, o=dflt_wind; i<=WGSIZE; i++ )
      (o++)->ob_state = wstates[wcolor_mode][i];
}

void x_malloc( void **addr, long size )
{
  *addr = (void *)lalloc( size, curapp->id );
}
