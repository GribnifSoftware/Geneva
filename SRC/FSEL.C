#include "tos.h"
#include "string.h"
#include "aes.h"
#include "xwind.h"
#define _FSELECT
#include "win_var.h"
#include "win_inc.h"
#include "lerrno.h"
#include "fsel.h"
#define rs_tedinfo fselted
#define rs_object  fselobj
#define rs_trindex fselind
#include "stdlib.h"
#ifdef GERMAN
  #include "german\fsel.rsh"
  #include "german\wind_str.h"
#else
  #include "fsel.rsh"
  #include "wind_str.h"
#endif

#define FNAME_INC       20
#define SNAME 		0
#define SSIZE 		1
#define SDATE 		2
#define STIME 		3
#define COPYSIZE	10240L
#define LASTS		15

typedef struct
{
  char name[12];
  unsigned int att;
  unsigned long date_time;
  long size;
} FNAME;

#define NIL      -1
#define edit_max 26
#define TOPLEN  200

#ifdef WINDOWS
  #define objc_draw(a,b,c,d,e,f,g) _objc_draw(a,0L,b,c,d,e,f,g)
  #define fsel_1col settings.flags.s.fsel_1col
  #define sort_type settings.sort_type
  #define form_alert _form_alert
  #define get_but() (g_mb&1)
  #define get_msey() g_my
  #define objc_offset objc_off
  #define graf_mouse(a,b) _graf_mouse(a,b,0)
  #define form_keybd(a,b,c,d,e,f) __form_keybd((long)a,b,d,e,f)
  #define x_objc_edit(a,b,c,d,e,f) _xobjc_edit(a,0L,b,c|((long)d<<16),e,f)
  #define x_form_mouse(a,b,c,d,e,f,g) _x_form_mouse(a,0L,b,c,d,e,f,g)
  #define _wcolors dwcolors[wcolor_mode][1]
#endif

char toppath[TOPLEN], auxpath[TOPLEN], toptitl[13], dev_err, in_copy,
    *copy_names, *copy_buf, info_ok=1;
TEDINFO *ttl_ptr;
int mappos, maxpath;
long drvmap;
DTA dta;
int files, folders, files0, folders0, topfile, topfold, otype,
    ftype, curfile, cf_top, last_fold[LASTS];
int fsclose, fsffile, fsfile, fsfsmall, fsfbig, fsfouter;
long bytes;
static int edit_obj, edit_idx, next_obj;
static int last_i;
static long last_b;
int max_fname=FNAME_INC;
FNAME *fnames;
int (*match_func)( char *s, char *pat );

int add_title( char ch );
void curfile_off( int i );

void draw_oth_obj( OBJECT *tree, int ind )
{
#ifndef WINDOWS
  objc_draw( tree, ind, 8, fs_rect.x, fs_rect.y, fs_rect.w, fs_rect.h );
#else
  objc_draw( (OBJECT2 *)tree, ind, 8, fs_rect.x, fs_rect.y, fs_rect.w, fs_rect.h );
#endif
}

void draw_obj( int ind )
{
  draw_oth_obj( fsel, ind );
}

void set_state( int istrue, int ind, int draw )
{
  int newval, old;
  unsigned int *o;

  old = *(o = &fsel[ind].ob_state);
  newval = istrue ? (old|SELECTED) : (old&~SELECTED);
  if( old != newval )
    if( !draw ) *o = newval;
    else change_objc( fsel, 0L, ind, &fs_rect, newval, draw );
}

void redo_map( int draw, char ltr, int *drv, int inc, int range )
{
  int i, b, a, old, ret;

  ltr-='A';
  for( i=b=a=0; i<26; i++ )
    if( drvmap&(1L<<i) )
      if( i<ltr ) b++;
      else a++;
  old = mappos;
  mappos += inc;
  if( range )
  {
    if( mappos>b ) mappos=b;
    if( mappos+9<b ) mappos=b-8;
  }
  if( mappos<0 ) mappos=0;
  if( mappos>(i=a+b-9) ) mappos=i;
  if( !draw )
  {
    hide_if( fsel, i>0, FSDDOWN );
    hide_if( fsel, i>0, FSDUP );
  }
  if( mappos<0 ) mappos=0;
  for( i=0, b=-1, a=FSDA; i<26; i++ )
    if( drvmap&(1L<<i) )
      if( ++b >= mappos && b < mappos+9 )
      {
        if( !draw || old != mappos )
        {
          u_object(fsel,a)->ob_spec.obspec.character = i+'A';
          if( i==ltr ) *drv=a;
          set_state( i==ltr, a, 0 );
          if( draw ) draw_obj(a);
        }
        a++;
      }
  if( !draw )
    while( a<FSDA+9 )
      hide_if( fsel, 0, a++ );
}

void arrow(void)
{
  graf_mouse( ARROW, 0L );
}

void bee(void)
{
  graf_mouse( BUSYBEE, 0L );
}

void toggle(void)
{
  if( edit_obj ) objc_edit( fsel, edit_obj, 0, &edit_idx, ED_END );
}

int end_path( TEDINFO *top, int flag )
{
  char *ptr;
  int i;

  /* show at least first 10 chars after last \  */
  if( strlen(toppath)>=edit_max )
  {
    if( flag ) toggle();
    ptr = pathend(toppath);
    if( (i=strlen(ptr)) <= 10 ) ptr += i;
    else if( ptr-toppath>=edit_max-10 ) ptr += 10;
    else ptr = toppath+edit_max;
    top->te_ptext = ptr-edit_max;
    top->te_txtlen = TOPLEN-edit_max;
    if( edit_obj==FSPATH ) edit_idx = edit_max;
    draw_obj(FSPATH);
    if( flag ) toggle();
    return 1;
  }
  return 0;
}

int check_path( char *p, int i )
{
  i += strlen(p);
  if( i < TOPLEN && i < maxpath ) return 1;
  form_alert( 1, FSPLONG );
  return 0;
}

void clear_last( int i )
{
  memset( &last_fold[i], 0, (LASTS-i)<<1 );
}

int resolve( char *path, char *toppath, TEDINFO *top, int *drv, int draw, int ret )
{
  char newval[TOPLEN], *pstart, *p, *p2, temp[]="x:\\*.*";
  int i;

  if( draw ) bee();
  pstart = toppath;
  newval[0] = toppath[0];
  if( toppath[0]<'A' || toppath[0]>'Z' || toppath[1] != ':' ) newval[0] = Dgetdrv()+'A';
  else
  {
    pstart += 2;
    if( !(drvmap&(1L<<toppath[0]-'A')) ) newval[0] = Dgetdrv()+'A';
    else Dsetdrv( toppath[0]-'A' );
  }
  temp[0] = newval[0];
  if( !ret ) ret = Mediach(temp[0]-'A');
  if( ret<0 && ret != AEUNDEV || (i=Fsfirst( temp, 0x37 )) != 0 && i!=AEFILNF && i!=AENMFIL )
      dev_err=1;
  else dev_err=0;
  ret = ret>0;
  newval[1] = ':';
  if( check_path( pstart, 2 ) )
  {
    strcpy( newval+2, pstart );
    *pathend(newval+2) = '\0';
    if( !draw || newval[0] != u_object(fsel,*drv)->ob_spec.obspec.character )
    {
      mappos--;
      redo_map(draw,newval[0],drv,0,1);
    }
    while( (dev_err || Dsetpath(newval+2) == AEPTHNF) && (p=pathend(newval)) > newval+3 )
    {
      *(p-1) = '\0';
      strcpy( pathend(newval), p );
      if( draw ) draw_obj( FSPATH );
    }
  }
  else *(newval+2) = 0;
  if( dev_err ) *(newval+2) = 0;
  else Dgetpath( newval+2, 0 );
  strcat( newval+2, "\\" );
  p = pathend(pstart);
  if( check_path( p, strlen(newval) ) )
  {
    strcpy( pathend(newval), p );
    if( !*pathend(newval) ) strcat(newval,"*.*");
  }
  if( ret ) i=0;
  else
  {
    for( i=0, p=newval, p2=path; *p && *p==*p2; p++, p2++ )
      if( *p == '\\' ) i++;
    if(i) i--;
  }
  clear_last(i);
  ret = ret || strcmp( newval, path );
  strcpy( path, newval );
  strcpy( toppath, newval );
  if( ret )
  {
    top->te_ptext = toppath;
    top->te_txtlen = TOPLEN;
    end_path( top, 0 );
/*    if( edit_obj==FSPATH )
    {
      if( (edit_idx = edit_max) > (i=strlen(toppath)) ) edit_idx=i;
    }
    else end_path( top, 0 ); */
  }
  if( draw )
  {
    draw_obj( FSPATH );
    if( !ret ) arrow();
  }
  return(ret);
}

void put_text( int type, char *ptr, FNAME *fn )
{
  char *str, sep;
  int a, b, c, i;

  switch(type)
  {
    case SNAME:
      if( fsel_1col ) *ptr++ = fn->att&0x10 ? '' :
          (fn->att&FA_READONLY ? '\10' : ' ');		/* 004: check mark */
      for( str=fn->name, i=0; i<11; i++ )
      {
        *ptr++ = *str ? *str++ : ' ';
        if( i==7 ) *ptr++ = *str ? '.' : ' ';
      }
      if( !fsel_1col ) break;
    case SSIZE:
      spf( ptr, (char *)(fsel_1col ? " %7D" : "%12D"), fn->size );
      if( !fsel_1col ) break;
      else ptr += 8;
    case SDATE:
      a = (*(unsigned int *)&fn->date_time>>5)&0xf;
      b = *(unsigned int *)&fn->date_time&0x1f;
      if( (c=((*(unsigned int *)&fn->date_time>>9)&0x7f)+80) >= 100 ) c-=100;	/* 007: was > 100 */
      if( (sep = (char)idt_fmt) == 0 ) sep = '/';
      str = (char *)(fsel_1col ? " %02d%c%02d%c%02d" : "    %02d%c%02d%c%02d");
      switch( (int)idt_fmt&0xf00 )
      {
        case 0x000:
          spf( ptr, str, a, sep, b, sep, c );
          break;
        case 0x100:
          spf( ptr, str, b, sep, a, sep, c );
          break;
        default:
          spf( ptr, str, c, sep, a, sep, b );
          break;
        case 0x300:
          spf( ptr, str, c, sep, b, sep, a );
      }
      break;
    case STIME:
      a = (unsigned int)fn->date_time;
      spf( ptr, (char *)"    %2d:%02d:%02d", (a>>11)&0x1f, (a>>5)&0x3f,
          (a&0x1f)<<1 );
      break;
  }
}

void set_text( int i, FNAME *fn )
{
  char *ptr;

  ptr = u_ptext(fsel,i);
  if( fn ) u_object(fsel,i)->ob_flags |= SELECTABLE|TOUCHEXIT;
  else u_object(fsel,i)->ob_flags &= ~(SELECTABLE|TOUCHEXIT);
  set_state( fn && fn->att&0x8000, i, 0 );
  if( fn ) put_text( fsel_1col ? SNAME : (i>=fsffile ? ftype : otype), ptr, fn );
  else strcpy( ptr, fsel_1col ? "                              " :
      "            " );
}

int changed( int *i, int num )
{
  if( *i==num ) return(0);
  *i=num;
  return(1);
}

void cur_copy( char *s )
{
  strcpy( u_ptext(fsel,fsfile), s );
  draw_obj(fsfile);
  if( edit_obj==fsfile ) edit_idx = strlen(u_ptext(fsel,fsfile));
}

int fobj2ind( int num )
{
  return topfile+num-fsffile;
}

int oobj2ind( int num )
{
  return topfold+num-FSFFOLD1;
}

int ind2fobj( int num )
{
  return num+fsffile-topfile;
}

int ind2oobj( int num )
{
  return num+FSFFOLD1-topfold;
}

void sel_item( int num, int truth, int draw, int isfile )
{
  unsigned int new_att, *old;

  new_att = *(old=&fnames[num].att) & 0x7fff;
  if( truth ) new_att |= 0x8000;
  if( new_att != *old )
  {
    if( isfile )
    {
      num -= folders;
      *old = new_att;
      if( num >= topfile && num <= topfile+9 ) set_state( truth,
          ind2fobj(num), draw );
    }
    else
    {
      if( folders || cf_top==FSFFOLD1 ) *old = new_att;
      if( num >= topfold && num <= topfold+9 ) set_state( truth,
          ind2oobj(num), draw );
    }
  }
}

void item_state( int draw )
{
  int i, max;

  if( !fsel_1col )
    for( i=0, max=!folders?10:folders; i<max; i++ )
      sel_item( i, 0, draw, 0 );
  for( i=folders, max=(!files?10:files)+folders; i<max; i++ )
    sel_item( i, 0, draw, 1 );
}

void total_info(void)
{
  int i, items;
  long b;

  if( !info_ok ) return;
  if( !curfile || cf_top==fsffile && !files || cf_top==FSFFOLD1 && !folders )
  {
    items = files+folders;
    b = bytes;
  }
  else if( curfile>0 )
  {
    items = 1;
    b = fnames[ cf_top==FSFFOLD1 ? oobj2ind(curfile) :
        fobj2ind(curfile)+folders ].size;
  }
  else
  {
    for( b=0L, i=folders; i<files+folders; i++ )
      if( fnames[i].att&0x8000 ) b += fnames[i].size;
    items = -curfile;
  }
  if( items != last_i )
  {
    spf( fsel[FSITEMS].ob_spec.tedinfo->te_ptext,
        (char *) (items == 1 ? FSINFO1 : FSINFO1P), last_i=items );
    draw_obj(FSITEMS);
  }
  if( b != last_b )
  {
    spf( fsel[FSBYTES].ob_spec.tedinfo->te_ptext,
        (char *) (b == 1L ? FSINFO2 : FSINFO2P), last_b=b );
    draw_obj(FSBYTES);
  }
}

void set_curfile( int ind, int flag )
{	/* flag:  bit 0: set name in edit   bit 1: multiple selection */
  int i;
  char isfile = cf_top==fsffile;

  if( !ind )
  {
    if( curfile )
    {
      item_state(1);
      curfile = 0;
      total_info();
    }
  }
  else
  {
    if( ind<0 ) ind=0;
    if( isfile )
    {
      if( !files ) ind = fsffile;
      else if( ind>=ind2fobj(files) ) ind = ind2fobj(files)-1;
      i=fobj2ind(folders+ind);
    }
    else
    {
      if( !folders ) ind = FSFFOLD1;
      else if( ind>=ind2oobj(folders) ) ind = ind2oobj(folders)-1;
      i=oobj2ind(ind);
    }
    if( flag&2 && fnames[i].att&0x8000 )
    {
      if( curfile>0 ) curfile_off(0);
      else
      {
        sel_item( i, 0, 1, isfile );
        if( ++curfile == -1 )
          for( curfile=0, i=0; i<files+folders; i++ )
            if( fnames[i].att&0x8000 )
              if( i>=folders )
              {
                curfile = ind2fobj(i-folders);
                cf_top = fsffile;
                break;
              }
              else
              {
                curfile = ind2oobj(i);
                cf_top = FSFFOLD1;
                break;
              }
        edit_obj = 0;
      }
      total_info();
      return;
    }
    if( (flag&1) && isfile && !(fnames[i].att&0x10) &&
        i>=folders && i<files+folders ) cur_copy( fnames[i].name );
    if( curfile != ind )
    {
      if( curfile )
        if( !(flag&2) )
        {
          item_state(1);
          curfile = ind;
        }
        else curfile = curfile>0 ? -2 : curfile-1;
      else curfile = ind;
      sel_item( i, 1, 1, isfile );
    }
    edit_obj = 0;
    total_info();
  }
}

void set_names( int which, int inc, int draw )
{
  int i, cnt, ch;
  FNAME *fn;

  if( which&2 && !fsel_1col )
  {
    if( (cnt = topfold+inc)+9 >= folders ) cnt = folders-10;
    if( cnt < 0 ) cnt=0;
    if( which<0 || cnt!=topfold )
    {
      if( draw&4 )
      {
        ch = curfile;
        info_ok=0;
        set_curfile( 0, 0 );
      }
      for( i=FSFFOLD1, fn=&fnames[topfold=cnt]; i<FSFFOLD1+10; i++, cnt++, fn++ )
      {
        set_state( 0, i, draw&1 );
        set_text( i, cnt<folders ? fn : 0L );
        if( draw&1 ) draw_obj(i);
      }
      if( draw&4 )
      {
        info_ok = 1;
        set_curfile( ch, 1 );
      }
      cnt = i = fsel[FSOBIG].ob_height;
      if( folders>10 )
        if( (i = i*10/folders) < char_h ) i = char_h;
      /* make sure changed gets called both times */
      if( draw&2 )
      {
        ch = changed( &fsel[FSOSMALL].ob_height, i );
        ch = changed( &fsel[FSOSMALL].ob_y, folders<=10 ? 0 :
            ((cnt-i)*(long)topfold+folders-11)/(folders-10) ) || ch;
        if( ch ) draw_obj(FSOBIG);
      }
    }
  }
  if( which&1 )
  {
    if( (cnt = topfile+inc)+9 >= files ) cnt = files-10;
    if( cnt < 0 ) cnt=0;
    if( which<0 || cnt!=topfile )
    {
      if( draw&4 )
      {
        ch = curfile;
        info_ok = 0;
        set_curfile( 0, 0 );
      }
      for( i=fsffile, fn=&fnames[(topfile=cnt)+folders]; i<fsffile+10; i++, cnt++, fn++ )
      {
        set_state( 0, i, draw&1 );
        set_text( i, cnt<files ? fn : 0L );
        if( draw&1 ) draw_obj(i);
      }
      if( draw&4 )
      {
        info_ok = 1;
        set_curfile( ch, 1 );
      }
      cnt = i = u_object(fsel,fsfbig)->ob_height;
      if( files>10 )
        if( (i = i*10/files) < char_h ) i = char_h;
      if( draw&2 )
      {
        ch = changed( &u_object(fsel,fsfsmall)->ob_height, i );
        ch = changed( &u_object(fsel,fsfsmall)->ob_y, files<=10 ? 0 : ((cnt-i)*(long)topfile+files-11)/(files-10) ) || ch;
        if( ch ) draw_obj(fsfbig);
      }
    }
  }
}

int match( char *str, char *pat, int len )
{
  char s, p, per=0, invert;

  for(;;)
  {
    if( !len-- ) return 1;
    if( (p = *pat++) == '\0' )
      if( *str ) return 0;
      else return 1;
    s = *str++;
    if( p == '*' )
    {
      if( !s ) return 1;
      str--;
      do
      {
        if( *str == '.' ) per=1;
        if( match( str, pat, len ) ) return 1;
      }
      while( *str++ && --len );
      if( *pat++ != '.' ) return 0;
      if( *pat == '*' )
        if( !*(pat+1) ) return 1;
        else return 0;
      else if( *pat || per ) return 0;
      return 1;
    }
    else if( p == '?' )
    {
      if( !s ) return 0;
    }
    else if( p == '!' )
    {
      invert = *pat++;
      if( match( --str, pat, invert ) ) return 0;
      if( !*(pat += invert) ) return 1;
      str += invert;
    }
    else if( p!=s ) return 0;
  }
}

int pnmatch( char *str, char *pat )
{
  char buf[40], *ptr, *ptr2, *ptr3, invert=0;
  int i, j;

  for( ptr=buf, j=0; *pat && j<sizeof(buf)-1; pat++ )
    if( *pat == '[' )
    {
      ptr2 = ++pat;
      while( *pat && *pat != ']' ) pat++;
/*      if( !*pat )
      {
        ferrs( SYNTAX, miss, ']' );
        return(-1);
      } */
      if( invert )
      {
        *ptr++ = 1;
        j++;
      }
      ptr3 = ptr;	/* char to replace */
      while( (*++ptr = *++pat) != 0 && ++j<sizeof(buf)-1 );	/* copy rest */
      if( *ptr ) *++ptr = '\0';	/* end if too long */
/*      if( invert && !*(ptr3+1) ) *(ptr3-1) = 2;*/  /* inc len if null after char */
      while( *ptr2 != ']' )
      {
        *ptr3 = *ptr2++;
        if( pnmatch( str, buf ) ) { if( !invert ) return 1; }
        else if( invert ) return 0;
        if( *ptr2 == '-' )
        {
          ptr2++;
          while( ++(*ptr3) <= *ptr2 )
            if( pnmatch( str, buf ) ) { if( !invert ) return 1; }
            else if( invert ) return 0;
          ptr2++;
        }
      }
      return(invert);
    }
    else if( *pat == '{' )
    {
      if( invert )
      {
        *ptr++ = 1;	/* temporary length */
        j++;
      }
      for(;;)
      {
        ptr3 = ptr;
        i = j;
        while( *++pat && *pat!=',' && *pat != '}' && ++i<sizeof(buf)-1 )
          *ptr3++ = *pat;
/*        if( !*pat )
        {
no_brace: ferrs( SYNTAX, miss, '}' );
          return(-1);
        } */
        if( invert ) *(ptr-1) = i-j;	/* set length of invert */
        ptr2 = pat;			/* skip to end of regexp */
        while( *ptr2 && *ptr2 != '}' ) ptr2++;
/*        if( !*ptr2 ) goto no_brace; */
        while( (*ptr3++ = *++ptr2) != 0 && ++i<sizeof(buf)-1 ); /* copy rest */
        if( *ptr3 ) *++ptr3 = '\0';	/* string too large: end it */
/*        if( invert && !*(ptr + *(ptr-1)) ) *(ptr-1) += 1;*/ /* ! for null at end */
        if( pnmatch( str, buf ) ) { if( !invert ) return 1; }
        else if( invert ) return 0;
        if( *pat == '}' || !*pat ) return(invert);
      }
    }
    else
    {
      if( invert )
      {
        *ptr++ = 1;
        j++;
        invert=0;
        if( *pat=='!' ) continue;
      }
      else if( *pat=='!' ) invert=1;	/* copy ! in next line */
      *ptr++ = *pat>='a' && *pat<='z' ? (*pat&0x5f) : *pat;
      if( invert && *(pat+1)<13 )	/* already know length */
      {
        *ptr++ = *++pat;
        j++;
        invert=0;
      }
      j++;
    }
  *ptr = '\0';
  return( match( str, buf, -1 ) );
}

int simp_match( char *str, char *pat )
{
  return match( str, pat, -1 );
}

void ini_match( char *s )	/* 004 */
{
  match_func = strpbrk(s,"[]{}!") ? pnmatch : simp_match;
}

int cmp_name( FNAME *a, FNAME *b )
{
  return( strcmp( a->name, b->name ) );
}
int cmp_date( FNAME *a, FNAME *b )
{
  /* unsigned, so can't use subtraction */
  if( b->date_time > a->date_time ) return 1;
  if( b->date_time < a->date_time ) return -1;
  return 0;
}
int cmp_size( FNAME *a, FNAME *b )
{
  long l = b->size - a->size;

  if( l>0L ) return 1;
  if( l<0L ) return -1;
  return 0;
}
int cmp_type( FNAME *a, FNAME *b )
{
  int i, as, bs;

  as = strlen(a->name)<=8;
  bs = strlen(b->name)<=8;
  if( as )
    if( !bs ) return -1;
    else return cmp_name( a, b );
  if( bs ) return 1;
  if( (i=strcmp(a->name+8,b->name+8)) == 0 )
      return cmp_name( a, b );
  return i;
}

void sort_it(void)
{
  static int (*scmp[])(FNAME *a, FNAME *b) = { cmp_name, cmp_size, cmp_date, cmp_type };

  if( sort_type<4 )
  {
    /* if by size, sort folders by name */
    qsort( fnames, folders0, sizeof(FNAME), sort_type!=1 ? (int (*)())scmp[sort_type] : (int (*)())cmp_name);
    qsort( &fnames[folders0], files0, sizeof(FNAME), (int (*)())scmp[sort_type] );
  }
}

int slashes( char *p )
{
  int s;

  for( s=0; (p=strchr(p,'\\'))!=0 && ++s<LASTS-1; p++ );
  return s ? s-1 : 0;
}

void wupdate( char *path, int noreset )
{
  int count, next;
  char *ptr1, copy[TOPLEN], *pat;
  FNAME *fn, temp;

  bee();
  strcpy( copy, path );
  strcpy( pathend(copy), "*.*" );
  pat = pathend(path);
start:
  files = folders = curfile = 0;
  if( !noreset )
  {
    count = last_fold[slashes(path)];
    if( fsel_1col )
    {
      topfile = count;
      topfold = 0;
    }
    else
    {
      topfile = 0;
      topfold = count;
    }
  }
  count = 0;
  bytes = 0L;
  fn=fnames;
  ini_match(pat);	/* 004 */
  if( !dev_err && !Fsfirst(copy,0x31) )
    do
      if( (((fn->att = dta.d_attrib) & 0x31) &&
          !(fn->att&(FA_HIDDEN|FA_SYSTEM)) || !fn->att) &&
          (fn->att&0x10 || (*match_func)( dta.d_fname, pat )) )
      {
        if( *(ptr1 = dta.d_fname) == '.' && (!*(ptr1+1) || *(ptr1+1)=='.') )
            continue;
        x_form_filename( (OBJECT *)fn->name, -1, 0, ptr1 );
        fn->date_time = ((unsigned long)dta.d_date<<16) | (unsigned long)dta.d_time;
        bytes += fn->size = dta.d_length;
        if( fn->att&0x10 && sort_type!=4 )
        {
          if( count != folders )
          {
            memcpy( &temp, fn, sizeof(FNAME) );
            memcpy( fn, &fnames[folders], sizeof(FNAME) );
            memcpy( &fnames[folders], &temp, sizeof(FNAME) );
          }
          folders++;
        }
        else files++;
        fn++;
        count++;
      }
    while( (next=Fsnext())==0 && count < max_fname );
  if( count==max_fname && !next )       /* expand buffer */
    if( !lrealloc((void **)&fnames,(count=max_fname+FNAME_INC)*(long)sizeof(FNAME)) != 0 )	/* 004: was lalloc */
    {
      max_fname = count;
      goto start;
    }
    else
    {
      ring_bell();
      form_alert( 1, FSNOMEM2 );
    }
  files0 = files;
  folders0 = folders;
  total_info();
  sort_it();
  if( fsel_1col )
  {
    files += folders;
    folders=0;
  }
  if( !fsel_1col ) hide_if( fsel, 1, FSOSMALL );
  hide_if( fsel, 1, fsfsmall );
  set_names( -1, 0, 3 );
  arrow();
}

void set_type( int flag, int inc )
{
  static char types[] = FS_NSDT;
  int i;

  if( flag&1 )
  {
    if( (ftype += inc) > STIME ) ftype = SNAME;
    fsel[FSFNSDT].ob_spec.obspec.character = types[ftype];
    if( flag>0 )
    {
      draw_obj(FSFNSDT);
      topfile--;
      set_names( 1, 1, 1 );
    }
  }
  if( flag&2 )
  {
    if( (otype += inc) > STIME ) otype = SNAME;
    else if( otype == SSIZE ) otype++;
    fsel[FSONSDT].ob_spec.obspec.character = types[otype];
    if( flag>0 )
    {
      draw_obj(FSONSDT);
      topfold--;
      set_names( 2, 1, 1 );
    }
  }
}

void curfile_off( int i )
{
  if( curfile && !(i&3) )
  {
    set_curfile(0,0);
    add_title(-1);
    if( !next_obj ) next_obj = fsfile;
  }
}

int has_match(void)
{
  FNAME *fn;
  int i, j, l;
  char temp[13];

  if( toptitl[0] )
  {
    x_form_filename( (OBJECT *)temp, -1, 0, toptitl );
    l = strlen(temp);
    for( fn = cf_top==FSFFOLD1 ? fnames : &fnames[folders], i=0;
        i<(cf_top==FSFFOLD1 ? folders : files); i++, fn++ )
      if( (j=strncmp(fn->name,temp,l))==0 ) return i;
      else if( j>0 && !fsel_1col && !sort_type ) return -1;
  }
  return -1;
}

int fnmatch( char *name1, char *name2 )
{
  int i;

  for( i=0; *name1++==*name2++ && i<12; i++ );
  return i;
}

void test_exmatch(void)
{
  FNAME *fn;
  int i, l, maxi, maxl;
  char *file;

  file = u_ptext(fsel,fsfile);
  if( file[0] )
  {
    for( maxl=0, fn=&fnames[folders], i=0; i<files; i++, fn++ )
      if( !(fn->att&0x10) && (l=fnmatch(fn->name,file)) >= maxl )
      {
        maxl = l;
        maxi = i;
      }
    if( maxi ) set_names( 1, maxi-topfile-1, 3 );
  }
}

void new_filenm(void)
{
  int i;

  if( (i=has_match()) >= 0 )
  {
    edit_obj = 0;
    if( cf_top==FSFFOLD1 )
    {
      if( i<topfold || i>topfold+9 )
      {
        info_ok = 0;
        set_curfile( 0, 0 );
        set_names( 2, i-topfold, 3 );
        info_ok = 1;
      }
    }
    else if( i<topfile || i>topfile+9 )
    {
      info_ok = 0;
      set_curfile( 0, 0 );
      set_names( 1, i-topfile, 3 );
      info_ok = 1;
    }
    set_curfile( cf_top+i-(cf_top==FSFFOLD1?topfold:topfile), 1 );
  }
  else curfile_off(0);
}

static void draw_title( char *s )
{
  ttl_ptr->te_ptext = s;
  ttl_ptr->te_just = TE_CNTR;
  draw_obj(FSTITLE);
}

int add_title( char ch )
{
  static char *old;
  char *ptr;
  int len, per, olen;

  if( ch >= 'A' && ch <= 'Z' || ch >= '0' && ch <= '9' || ch=='.' ||
      strchr(fvalid,ch) || !ch )
  {
    if( in_copy ) return 0;
    per = (ptr = strchr(toptitl,'.')) != 0 ? ptr-toptitl : -1;
    olen = len = strlen(toptitl);
    if( per>=0 && len-per>3 ) return 0;
    if( ch=='.' )
      if( per>=0 ) return 0;
      else toptitl[len++] = '.';
    else
    {
      toptitl[len++] = ch;
      if( len==8 && per<0 ) toptitl[len++] = '.';
    }
    toptitl[len]=0;
    if( has_match() >= 0 )
    {
      draw_title(toptitl);
      return 1;
    }
    toptitl[olen]=0;
  }
  else if ( ch==-1 )
  {
    if( !in_copy )
    {
      toptitl[0] = 0;
      if( ttl_ptr->te_ptext != old )
      {
        ttl_ptr->te_ptext = old;
        ttl_ptr->te_just = TE_LEFT;
        draw_obj(FSTITLE);
      }
    }
  }
  else if( ch==-2 )
  {
    toptitl[0] = 0;
    old = ttl_ptr->te_ptext;
    ttl_ptr->te_just = TE_LEFT;
  }
  return 0;
}

void page_it( int which, int obj, int all )
{
  int y, ox, oy, inc;

  y = get_msey();
  objc_offset( fsel, obj+1, &ox, &oy );
  inc = y < oy ? (all ? -1000 : -10) : (all ? 1000 : 10);
  do
    set_names( which, inc, 3 );
  while( get_but()&1 );
}

void move_path( TEDINFO *top, int dir, int togl )
{
  if( togl ) toggle();
  if( dir<0 )
  {
    if( top->te_ptext != toppath )
    {
      --(top->te_ptext);
      ++(top->te_txtlen);
      if( edit_obj==FSPATH )
        if( ++edit_idx>edit_max-1 ) edit_idx = edit_max-1;
      draw_obj( FSPATH );
    }
  }
  else
  {
    if( strlen(top->te_ptext) > edit_max )
    {
      ++(top->te_ptext);
      --(top->te_txtlen);
      if( edit_obj==FSPATH )
        if( --edit_idx < 1 ) edit_idx = 1;
      draw_obj( FSPATH );
    }
  }
  if( togl ) toggle();
}

void dgetpath( char *s )
{
  Dgetpath( s, 0 );
  strcat( s, "\\" );
}

int dsetpath( char *s )
{
  int ret;
  DTA dta, *old;

  old = Fgetdta();
  Fsetdta(&dta);
  ret = Fsfirst("\\*.*",0x37);
  Fsetdta(old);
  if( !ret ) return Dsetpath(s);
  return ret==AENMFIL||ret==AEFILNF ? AEPTHNF : ret;
}

void tree_set( int flag )
{
  static char old[TOPLEN];

  if( !flag )
  {
    bee();
    dgetpath( old );
  }
  else
  {
    dsetpath( old );
    arrow();
  }
}

#define OTH_OK    99
#define OTH_CANC  98
#define OTH_FOUND 97
#define OTH_SKIP  96

void bad( char *msg, char *s, int num )
{
  char temp[100];

  if( num<0 )
  {
    spf( temp, msg, s );
    x_form_error( temp, num );
  }
}

void gen_bad( int num )
{
  bad( FSGENERR, 0L, num );
}

int tree_it( OBJECT *tree, int func( OBJECT *tree, DTA *fn, int isfile ), char *oldfold )
{ /* func: isfile:  1:file  0:folder  -1:pop out of folder  -2:return to parent */
  DTA *old, new_dta;
  int ret=0, i;

  old = Fgetdta();
  Fsetdta( &new_dta );
  if( (i=Fsfirst( "*.*", 0x37 )) == 0 )
  {
    do
      if( new_dta.d_attrib & 0x10 )
      {
        if( new_dta.d_fname[0] != '.' || (new_dta.d_fname[1] != '.' &&
            new_dta.d_fname[1] != '\0') )
        {
          if( (ret=(*func)( tree, &new_dta, 0 )) == 0 )
            if( (ret = dsetpath(new_dta.d_fname)) == 0 &&
                (ret = tree_it( tree, func, new_dta.d_fname )) == 0 &&
                (ret = dsetpath("..")) == 0 &&
                (ret = (*func)( tree, &new_dta, -1 )) == 0 )
                ret = (*func)( tree, (DTA *)oldfold, -2 );
            else gen_bad(ret);
        }
      }
      else ret = (*func)( tree, &new_dta, 1 );
    while( ret>=0 && ret!=OTH_CANC && ret!=OTH_FOUND && (ret=Fsnext()) == 0 );
    if( ret == AENMFIL ) ret = 0;
  }
  else if( i != AEFILNF ) gen_bad(ret=i);
  Fsetdta( old );
  return ret;
}

int tool_parm1, tool_parm2, infofiles, infofolds;
char root[13];
long infobytes;

void get_root(void)
{
  char *ptr, *ptr0;

  if( (ptr=pathend(toppath)) != toppath+3 )
  {
    ptr0 = --ptr;
    while( ptr!=toppath && *--ptr != '\\' );
    strncpy( root, ptr+1, 12 );
    root[ptr0-ptr-1] = 0;
  }
  else root[0]=0;
}

int fold_inf( OBJECT *tree, DTA *fn, int isfile )
{
  if( isfile==1 )
  {
    infobytes += fn->d_length;
    infofiles++;
  }
  else if( !isfile ) infofolds++;
  tree++;
  return 0;
}

void info_bytes( OBJECT *tree, long infobytes )		/* 007: sub */
{
  spf( tree[FSRSIZE].ob_spec.tedinfo->te_ptext, FSRBYTES, infobytes );
}

void show_info( OBJECT *tree, int flag )
{
  spf( tree[FSRFOLD].ob_spec.tedinfo->te_ptext, "%d", infofolds );
  info_bytes( tree, infobytes );
  spf( tree[FSRFILES].ob_spec.tedinfo->te_ptext, "%d", infofiles );
  if( flag&1 ) draw_oth_obj( tree, FSRFOLD );
  if( flag&2 ) draw_oth_obj( tree, FSRSIZE );
  if( flag&4 ) draw_oth_obj( tree, FSRFILES );
}

int do_fold_inf( OBJECT *tree, char *s )
{
  int ret;

  infofiles=infofolds=0;
  infobytes=0L;
  tree_set(0);
  if( (ret=dsetpath(s)) == 0 ) ret = tree_it( tree, fold_inf, s );
  else gen_bad(ret);
  tree_set(1);
  if( !ret ) show_info( tree, 0 );
  return ret;
}

int oth_reni( OBJECT *tree )
{
  char temp[20], *ptr, c, hid, is_fold;
  FNAME *fn = &fnames[tool_parm1];

  info_bytes( tree, fn->size );
  put_text( STIME, temp, fn );
  ptr = tree[FSRDATE].ob_spec.free_string;
  strcpy( ptr, temp+4 );
  ptr += strlen(ptr);
  c = fsel_1col;
  fsel_1col = 1;
  put_text( SDATE, ptr, fn );
  fsel_1col = c;
  tree[FSROK].ob_spec.free_string = (c = tool_parm2==OPDEL)!=0 ? (char *)FSDEL : (char *)FSOKS;
  is_fold = (char)(fn->att & 0x10);
  if( tool_parm2==OPDEL || is_fold && Sversion()<0x1500 )
      tree[FSROLD].ob_flags &= ~EDITABLE;
  else tree[FSROLD].ob_flags |= EDITABLE;
  hid = (tree[FSRDELMSG].ob_flags & HIDETREE) != 0;
  if( c == hid ) tree[0].ob_height +=
      (hide_if( tree, c, FSRDELMSG ) ? 3 : -3) * char_h;
  tree[1].ob_spec.free_string =
      hide_if( tree, is_fold, FSRFOLDMSG ) ?
      (char *)(tool_parm2==OPDEL ? FSFODEL : FSFOINFO) :
      (char *)(tool_parm2==OPDEL ? FSFIDEL : FSFINFO);
  if( hide_if( tree, !is_fold && tool_parm2!=OPDEL, FSRRO ) )	/* 004 */
      sel_if( tree, FSRRO, fn->att&FA_READONLY );
  if( is_fold )
  {
    x_form_filename( (OBJECT *)fn->name, -1, 1, temp );
    return !do_fold_inf( tree, temp );
  }
  return 1;
}

int oth_ren( OBJECT *tree, int ind )
{
  char old[13], new_fname[13];
  int ret;
  unsigned int *i, c;

  if( ind==FSROK )
  {
    x_form_filename( (OBJECT *)fnames[tool_parm1].name, -1, 1, old );
    x_form_filename( tree, FSROLD, 1, new_fname );
    c = *(i=&fnames[tool_parm1].att)&~FA_READONLY;		/* 004 */
    if( tree[FSRRO].ob_state & SELECTED ) c |= FA_READONLY;	/* 004 */
    ret = 0;
    if( c != *i )				/* 004 */
    {
      Fattrib( old, 1, (*i=c)&0xFF );
      ret = OTH_OK;
    }
    if( new_fname[0] && strcmp(new_fname,old) )
    {
      if( (ret=Frename( 0, old, new_fname )) == 0 ) return OTH_OK;
      bad( FSBADREN, new_fname, ret );
    }
    return ret;
  }
  else if( ind==FSRCANCEL ) return OTH_CANC;
  return 0;
}

void find_names( OBJECT *tree, char *file, char *fold )
{
  if( file ) strcpy( tree[FFIFILE].ob_spec.tedinfo->te_ptext, file );
  if( fold ) strcpy( tree[FFIFOLD].ob_spec.tedinfo->te_ptext, fold );
}

int del_it( OBJECT *tree, DTA *fn, int isfile )
{
  int ret;

  if( isfile==1 )
  {
    if( (ret=Fdelete(fn->d_fname)) < 0 )
    {
      bad( FSBADRM, fn->d_fname, ret );
      return OTH_CANC;
    }
    if( tree )
    {
      infobytes -= fn->d_length;
      infofiles--;
      show_info( tree, 2|4 );
    }
  }
  else if( isfile==-1 )
  {
    if( (ret=Ddelete(fn->d_fname)) < 0 )
    {
      bad( FSBADRM, fn->d_fname, ret );
      return OTH_CANC;
    }
    if( tree )
    {
      infofolds--;
      show_info( tree, 1 );
    }
  }
  return 0;
}

int oth_del( OBJECT *tree, int ind )
{
  char temp[13];
  int ret;

  if( ind==FSROK )
  {
    x_form_filename( tree, FSROLD, 1, temp );
    if( fnames[tool_parm1].att&0x10 )
    {
      tree_set(0);
      if( (ret = dsetpath(temp)) == 0 )
          ret = tree_it( tree, del_it, temp );
      else gen_bad(ret);
      tree_set(1);
      if( ret ) return ret;
      if( (ret=Ddelete(temp))==0 ) return OTH_OK;
    }
    else if( (ret=Fdelete(temp))==0 ) return OTH_OK;
    bad( FSBADRM, temp, ret );
    return ret;
  }
  else if( ind==FSRCANCEL ) return OTH_CANC;
  return 0;
}

int oth_new( OBJECT *tree, int ind )
{
  char fold[13];
  int ret;

  if( ind==FFOK )
  {
    x_form_filename( tree, FFNAME, 1, fold );
    if( (ret=Dcreate(fold))==0 ) return OTH_OK;
    bad( FSBADNEW, fold, ret );
    return ret;
  }
  return 0;
}

int oth_findi( OBJECT *tree )
{
  hide_if( tree, 0, FFISHOW );
  get_root();
  find_names( tree, "", root );
  return 1;
}

int find_it( OBJECT *tree, DTA *fn, int isfile )
{
  int ind;

  if( isfile==1 )
  {
    find_names( tree, fn->d_fname, 0L );
    draw_oth_obj( tree, FFIFILE );
    if( (*match_func)( fn->d_fname, tree[FFINAME].ob_spec.tedinfo->te_ptext ) )
    {
      hide_if( tree, 1, FFISHOW );
      draw_oth_obj( tree, FFISHOW );
      change_objc( tree, 0L, FFINEXT, &fs_rect,
          tree[FFINEXT].ob_state&~SELECTED, 1 );
      arrow();
      ind = _form_do( tree, 0 );
      if( ind != FFINEXT ) sel_if(tree,ind,0);
      if( tree[FFINAME].ob_spec.tedinfo->te_ptext[0] )
      {
        if( ind==FFINEXT ) return 0;
        if( ind==FFISHOW )
        {
          auxpath[0] = toppath[0];
          auxpath[1] = ':';
          dgetpath( auxpath+2 );
          strcpy( root, fn->d_fname );
          return OTH_FOUND;
        }
      }
      bee();
      return OTH_CANC;
    }
  }
  else if( !isfile )
  {
    find_names( tree, 0L, fn->d_fname );
    draw_oth_obj( tree, FFIFOLD );
  }
  else if( isfile==-2 )
  {
    find_names( tree, 0L, (char *)fn );
    draw_oth_obj( tree, FFIFOLD );
  }
  return 0;
}

int oth_find( OBJECT *tree, int ind )
{
  int ret;

  if( ind==FFINEXT )
  {
    tree_set(0);
    ini_match( tree[FFINAME].ob_spec.tedinfo->te_ptext );	/* 004 */
    ret = tree_it( tree, find_it, root );
    tree_set(1);
    if( !ret ) form_alert( 1, FSMATCH );
    tree[FFINEXT].ob_state &= ~SELECTED;
    return ret;
  }
  return 0;
}

int oth_form( int index, int in1ind, char *in1str,
    int ini( OBJECT *tree ), int func( OBJECT *tree, int ind ) )
{
  OBJECT *tree;
  int ret, ind;
  Rect r;

  tree = fselind[index];
  if( ini )
    if( !(*ini)(tree) ) return OTH_CANC;
  if( in1ind ) strcpy( u_ptext(tree,in1ind), in1str );
  form_center( tree, &r.x, &r.y, &r.w, &r.h );
#ifndef WINDOWS
  objc_draw( tree, 0, 8, r.x, r.y, r.w, r.h );
#else
  objc_draw( (OBJECT2 *)tree, 0, 8, r.x, r.y, r.w, r.h );
#endif
  if( index != FCOPY ) ind = _form_do( tree, in1ind );
  if( index != FFIND || tree[FFINAME].ob_spec.tedinfo->te_ptext[0] )
    if( func ) ret = (*func)(tree,ind);
  if( index != FCOPY ) sel_if(tree,ind&0x7fff,0);	/* 005: mask touchexit */
#ifndef WINDOWS
  objc_draw( fsel, 0, 8, r.x, r.y, r.w, r.h );
#else
  objc_draw( (OBJECT2 *)fsel, 0, 8, r.x, r.y, r.w, r.h );
#endif
  return(ret);
}

void set_title( char *s )
{
  fsel[FSTITLE].ob_flags |= X_BOLD;
  draw_title(s);
  fsel[FSTITLE].ob_flags &= ~X_BOLD;
}

int mn_popup( int pop, int parent )
{
  int x, y, i;
  MENU menu, out;

  menu.mn_tree = fselind[pop];
  menu.mn_menu = 0;
  menu.mn_item = 1;
  menu.mn_scroll = 1;
  if( pop==EXTLIST )
  {
    menu.mn_item = 2;	/* 004 */
    set_title( FSEXTMSG );
  }
  else if( pop==PATHLIST )
  {
    set_title( FSPTHMSG );
    menu.mn_item = PATH1;
  }
  else if( pop==SORTLIST )
  {
    for( i=6; --i; )
      u_object(menu.mn_tree,i)->ob_state = i==sort_type+1 ? CHECKED : 0;
    if( !fsel_1col ) menu.mn_tree[5].ob_state = DISABLED;
    menu.mn_item = sort_type+1;
  }
  else
    for( i=OPINFO; i<=OPFREE; i++ )
    {
      if( in_copy ) x=0;
      else switch(i)
      {
        case OPMOVE:
        case OPCOPY:
        case OPINFO:
        case OPDEL:
          x = curfile>0 || curfile<=-2;
          break;
        case OPFIND:
        case OPFOLD:
        case OPFREE:
          x = !dev_err;
          break;
        default:
          x = 0;
      }
      u_object(menu.mn_tree,i)->ob_state = x ? 0 : DISABLED;
    }
  objc_offset( fsel, parent, &x, &y );
  i = menu_popup( &menu, x, y, &out );
  if( i || pop==EXTLIST || pop==PATHLIST ) add_title(-1);
  if( i )
  {
    ks = out.mn_keystate;
    return out.mn_item;
  }
  return 0;
}

void fsel_type(void)
{
  clear_last(0);
  hide_if( fsel, !fsel_1col, FSFOUTER );
  hide_if( fsel, !fsel_1col, FSOOUTER );
  hide_if( fsel, fsel_1col, FSFOUTER2 );
  if( fsel_1col )
  {
    fsel[FS1COL].ob_spec.tedinfo->te_ptext = FSCOL2;
    fsel[FSFOUTER2].ob_y = fsel[FSOOUTER].ob_y;
    fsclose = FSCLOSE2;
    fsffile = FSFFILE2;
    fsfile = FSNAME2;
    fsfsmall = FSFSMALL2;
    fsfbig = FSFBIG2;
    fsfouter = FSFOUTER2;
  }
  else
  {
    fsel[FS1COL].ob_spec.tedinfo->te_ptext = FSCOL1;
    fsclose = FSCLOSE;
    fsffile = FSFFILE1;
    fsfile = FSFILE;
    fsfsmall = FSFSMALL;
    fsfbig = FSFBIG;
    fsfouter = FSFOUTER;
    if( sort_type==4 ) sort_type = 0;
  }
}

int key_2asc( KEYTAB *key )
{
  int i;

  i = (unsigned)kr>>8;
  if( i>=0x78 && i<=0x83 ) i-=0x78-0x2;
  i = (ks&3) ? key->shift[i] : key->unshift[i];
  if( i>='a' && i<='z' ) i &= 0xdf;
  return i;
}

void slider( int total, int big, int small, int *top, int draw )
{
  int i;

  if( graf_slidebox( 0L, total, 10, 0x101 ) >= 0 )
  {
    set_state( 1, small, 1 );
    i = graf_slidebox( fsel, big, small, 0x201 );
    while( i>=0 )
    {
      set_names( draw, i - *top, 1 );
      i = graf_slidebox( fsel, big, small, 0x301 );
    }
    set_state( 0, small, 1 );
  }
}

void dflt_curfile(void)
{
  add_title(-1);
  if( edit_obj )
  {
    toggle();
    cf_top = fsffile;
    edit_obj = 0;
  }
}

void add_ext( char *s )
{
  char *ptr, *ptr2, temp[20];
  int l;

  ptr = pathend(toppath);
  if( ks&4 )
  {
    l = strlen(toppath) + strlen(s+2) + 2;  /* brace comma */
    if( !check_path( "", l ) ) return;
    if( (ptr2 = strchr(ptr,'.')) != 0 ) ptr = ptr2+1;
    if( (ptr2 = strchr(ptr,'{')) == 0 || (ptr2 = strchr(ptr2,'}')) == 0 )
    {
      if( !check_path( "", l+1 /* one more brace */ ) ) return;
      strcpy( temp, ptr );
      strcpy( ptr+1, temp );
      *ptr = '{';
      ptr2 = ptr+strlen(ptr);
    }
    strcpy( ptr2, "," );
    strcat( ptr2, s+2 );
    strcat( ptr2, "}" );
  }
  else strcpy( ptr, s );
}

int get_sel( int flag )
{
  static int ind;

  if( flag<=0 )
  {
    ind = flag<0 ? 0 : folders;
    return 0;
  }
  else
    for(;;)
    {
      if( ind >= folders+files ) return -1;
      if( fnames[ind++].att&0x8000 ) return ind-1;
    }
}

void sel_all(void)
{
  int i, j;

  if( files+folders )
  {
    for( i=j=0; i<files+folders; i++ )
      if( fnames[i].att&0x8000 ) j++;
    j = j!=files+folders;
    for( i=0; i<files+folders; i++ )
    {
      cf_top = i>=folders ? fsffile : FSFFOLD1;
      sel_item( i, j, 1, i>=folders );
    }
    if( !j ) curfile = 0;
    else if( (curfile = -files-folders) == -1 ) curfile = cf_top;
    next_obj = j ? 0 : fsfile;
    total_info();
  }
}

void update_path( char *path, TEDINFO *top, int *drv, int flag, int noreset, int last )
{
  char had_curfile=0;
  int i;

  if( !flag && curfile )
  {
    info_ok = 0;
    had_curfile++;
  }
  if( last ) i = last_fold[slashes(toppath)];
  if( resolve( path, toppath, top, drv, 1, flag ) )
  {
    curfile_off(0);
    if( last ) last_fold[slashes(path)] = i;
    wupdate( path, noreset );
    if( had_curfile ) next_obj = cf_top;
  }
  add_title(-1);
  info_ok = 1;
}

void copy_info( OBJECT *tree, int flag )
{
  spf( tree[FCOPFOGO].ob_spec.tedinfo->te_ptext, "%d", infofolds );
  spf( tree[FCOPFIGO].ob_spec.tedinfo->te_ptext, "%d", infofiles );
  if( flag&1 ) draw_oth_obj( tree, FCOPFIGO );
  if( flag&2 ) draw_oth_obj( tree, FCOPFOGO );
}

void c_names( OBJECT *tree, char *file, char *fold, int draw )
{
  if( file ) strcpy( tree[FCOPFILE].ob_spec.tedinfo->te_ptext, file );
  if( fold ) strcpy( tree[FCOPFOLD].ob_spec.tedinfo->te_ptext, fold );
  if( draw&1 ) draw_oth_obj( tree, FCOPFILE );
  if( draw&2 ) draw_oth_obj( tree, FCOPFOLD );
}

void draw_part( OBJECT *tree, int part )
{
#ifdef WINDOWS
  menu_owner = 0L;
#endif
  part_draw( tree, 0, part, part );
}

void copy_hide( OBJECT *tree, int flag, int draw )
{
  hide_if( tree, !flag, FCOPHID1 );
  hide_if( tree, flag, FCOPHID2 );
  if( draw ) draw_part( tree, FCOPHID2 );
}

int copy_ini( OBJECT *tree )
{
  c_names( tree, (char *)"", (char *)"", 0 );
  copy_hide( tree, 0, 0 );
  copy_info( tree, 0 );
  tree[FCOPTITL].ob_spec.free_string = (char *)(in_copy>0 ? FSCOPYING : FSMOVING);
  return 1;
}

int exists( OBJECT *tree, DTA *old )
{
  int ret, ind;
  char *ptr, temp[13];
  DTA new_dta;

  Fsetdta(&new_dta);
  while( (ret=Fsfirst(auxpath,0x37)) == 0 )
  {
    x_form_filename( tree, FCOPNAME, 0, ptr=pathend(auxpath) );
    copy_hide( tree, 1, 1 );
    arrow();
    sel_if(tree,ind=_form_do(tree,FCOPNAME),0);
    bee();
    copy_hide( tree, 0, 1 );
    if( ind==FCOPSKIP )
    {
      ret = OTH_SKIP;
      break;
    }
    if( ind!=FCOPCOP )
    {
      ret = OTH_CANC;
      break;
    }
    x_form_filename( tree, FCOPNAME, 1, temp );
    if( !strcmp(temp,ptr) )
    {
      if( !((old->d_attrib|new_dta.d_attrib)&FA_SUBDIR) )
      { /* both files */
        ret = 0;
        break;
      }
      else if( old->d_attrib&new_dta.d_attrib&FA_SUBDIR )
      { /* both folders */
        ret = OTH_SKIP;
        break;
      }
    }
    strcpy( ptr, temp );
  }
  Fsetdta(old);
  return ret==AEFILNF || ret==AENMFIL ? 0 : ret;
}

int copy( OBJECT *tree, DTA *fn, int isfile )
{
  int ret=0, in, out;
  long l, m;
  DOSTIME date;
  char same_drive=0, *end;

  end = pathend(auxpath);
  if( isfile>=0 ) strcpy( end, fn->d_fname );
  if( isfile==1 )
  {
    c_names( tree, fn->d_fname, 0L, 1 );
    if( (ret=exists( tree, fn )) == OTH_SKIP )
    {
      ret = 0;
      infofiles--;
      copy_info( tree, 1 );
      return ret;
    }
    if( ret != 0 ) return ret;
    if( in_copy<0 && (same_drive = auxpath[0]==toppath[0]) == 1 )
    {
      Fdelete(auxpath);		/* 005: remove existing file */
      if( (ret = Frename( 0, fn->d_fname, auxpath )) < 0 )
      {
        bad( FSMOVE, fn->d_fname, ret );
        return OTH_CANC;
      }
    }
    else if( (ret = Fopen( fn->d_fname, 0 )) > 0 )
    {
      in = ret;
      Fdatime( (DOSTIME *)&fn->d_time, in, 0 );		/* 004 */
      if( (ret = Fcreate( auxpath, 0 )) > 0 )
      {
        out = ret;
        ret = 0;
        while( !ret && (l=Fread(in,COPYSIZE,copy_buf)) > 0 )
          if( (m=Fwrite(out,l,copy_buf)) != l )
          {
            if( m<0 ) bad( FSWRITE, end, (int)m );
            else form_alert( 1, FSFULL );
            ret = OTH_CANC;
          }
        if( l<0 )
        {
          bad( FSREAD, fn->d_fname, (int)l );
          ret = OTH_CANC;
        }
        Fclose(out);
        if( ret ) Fdelete(auxpath);
        else if( (out = Fopen( auxpath, 2 )) > 0 )	/* 004: set date */
        {
          Fdatime( (DOSTIME *)&fn->d_time, out, 1 );
          Fclose(out);
        }
      }
      else
      {
        bad( FSCREAT, end, ret );
        ret = OTH_CANC;
      }
      Fclose(in);
    }
    else
    {
      bad( FSOPEN, fn->d_fname, ret );
      ret = OTH_CANC;
    }
    if( ret ) return ret;
    infofiles--;
    copy_info( tree, 1 );
  }
  else if( !isfile )
  {
    c_names( tree, 0L, fn->d_fname, 2 );
    if( (ret=exists( tree, fn )) != 0 && ret != OTH_SKIP ) return ret;
    if( ret == OTH_SKIP ) ret = 0;
    else if( (ret = Dcreate(auxpath)) != 0 )
    {
      bad( FSBADNEW, end, ret );
      return OTH_CANC;
    }
    strcat( auxpath, "\\" );
    if( in_copy > 0 )
    {
      infofolds--;
      copy_info( tree, 2 );
    }
  }
  else if( isfile==-1 )	/* exit folder */
  {
    *(end-1) = 0;
  }
  else if( isfile==-2 )  /* return to parent */
  {
    c_names( tree, 0L, (char *)fn, 2 );
    if( in_copy < 0 )
    {
      infofolds--;
      copy_info( tree, 2 );
    }
  }
  if( in_copy<0 && !same_drive ) ret = del_it( 0L, fn, isfile );
  return ret;
}

int copy_func( OBJECT *tree, int func( OBJECT *tree, DTA *fn, int isfile ) )
{
  char *ptr;
  int ret=0, type, len;
  DTA root, *old;

  tree_set(0);
  old = Fgetdta();
  Fsetdta(&root);
  for( ptr=copy_names; *ptr && !ret; ptr+=len )
  {
    len = abs(type=*ptr++);
    strncpy( root.d_fname, ptr, len );
    root.d_fname[len]=0;
    root.d_attrib = type<0 ? FA_SUBDIR : 0;
    if( type>0 ) ret = (*func)( tree, &root, 1 );
    else if( (ret=dsetpath(root.d_fname)) == 0 &&
        (ret = (*func)( tree, &root, 0 )) == 0 &&
        (ret = tree_it( tree, func, root.d_fname )) == 0 &&
        (ret = dsetpath("..")) == 0 &&
        (ret = (*func)( tree, &root, -1 )) == 0 )
        ret = (*func)( tree, (DTA *)"", -2 );
    gen_bad(ret);
  }
  Fsetdta(old);
  tree_set(1);
  return ret;
}

int copy_it( OBJECT *tree, int ind )
{
  ind++;
  return copy_func( tree, copy );
}

void end_copy( char *path, TEDINFO *top, int *drv, int ok )
{
  char temp[TOPLEN];

  curfile_off(0);
  if( ok )
  {
/* 005: return to dest path when done    strcpy( temp, auxpath );
    strcpy( auxpath, toppath );
    strcpy( toppath, temp ); */
    strcpy( temp, toppath );
    strcpy( toppath, auxpath );
    strcpy( auxpath, temp );
    update_path( path, top, drv, 0, 0, 0 );
    infofiles=infofolds=0;
    if( !copy_func( 0L, fold_inf ) )
        oth_form( FCOPY, 0, 0L, copy_ini, copy_it );
    strcpy( toppath, temp );
  }
  else strcpy( toppath, auxpath );
  in_copy = 0;
  lfree( copy_names );
  lfree( copy_buf );
  update_path( path, top, drv, 1, ok, 1 );
}

void newpath( char *ptr )
{
  char temp[100];

  strcpy( temp, pathend(toppath) );
  strcpy( toppath, ptr );
  strcat( toppath, temp );
}

#ifdef WINDOWS
void ob_att( int tree, int obj )
{
  OBJECT *o;

  o = u_object(fselind[tree],obj);
  if( settings.color_root[color_mode].s.atari_3D )
    if( *(char *)&o->ob_type == X_UNDERLINE ||
        *(char *)&o->ob_type == X_GROUP ) o->ob_flags |= 0x0600;
    else o->ob_flags |= 0x0400;
  else o->ob_flags &= ~0x0600;
}
void ob_ted( int tree, int obj )
{
  TEDINFO *t;

  t = u_object(fselind[tree],obj)->ob_spec.tedinfo;
  t->te_font = pfont_mode;
  t->te_junk1 = pfont_id;
  t->te_junk2 = ptsiz;
}
#endif

int __x_fsel_input( char *path, int pathlen, char *file, int sels,
    int *but, char *title )
{
  DTA *old;
  int ret, drv=0, dclick, which, i, j;
  char quit=0, *ptr, *ptr2, cont,
      temp[80] /*big enough for delete alert*/, temp2[13], old_templ[50];
  TEDINFO *top;
  KEYTAB *key = Keytbl( (void *)-1L, (void *)-1L, (void *)-1L );
  DISKINFO diskinfo;
  BASPAG *proc;
  static char cols[][2] = { FSDUP, WGUP, FSDDOWN, WGDOWN,
      FSCLOSE, WGCLOSE, FSROOT, WGCLOSE, FSOUP, WGUP, FSOBIG,
      WGVBIGSL, FSOSMALL, WGVSMLSL, FSODOWN, WGDOWN, FSFUP, WGUP,
      FSFBIG, WGVBIGSL, FSFSMALL, WGVSMLSL, FSFDOWN, WGDOWN,
      FSCLOSE2, WGCLOSE, FSROOT2, WGCLOSE, FSFUP2, WGUP,
      FSFBIG2, WGVBIGSL, FSFSMALL2, WGVSMLSL, FSFDOWN2, WGDOWN,
      FSONSDT, WGFULL, FSFNSDT, WGFULL, FSPLEFT, WGLEFT,
      FSPRIGHT, WGRT };
  static char atts[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, FSDA, 14, 15,
      16, 17, 18, 19, 20, 21, FSDDOWN, FSDUP, FSALL1, FSALL2, FSFILE, FSNAME2, FSITEMS,
      FSBYTES, -FFOLD, 1, 2, -1, -FINFO, 1, FSROLD, FSRSIZE, FSRFOLD,
      FSRFILES, -PATHLIST, 0, -EXTLIST, 0, -FSHELPTXT, 1, 9, 28, -OPLIST,
      0, -SORTLIST, 0, -FFIND, 1, FFINAME, FFIFOLD, FFIFILE, -FCOPY, 1,
      FCOPFOLD, FCOPFILE, FCOPHID2, FCOPFOGO, FCOPFIGO, FCOPNAME },
      teds[] = { FSTITLE, -FFOLD, FFNAME, -FINFO, FSROLD, FSRSIZEO, FSRSIZE, FSRDATEO,
      FSRFOLDO, FSRFOLD, FSRFILEO, FSRFILES, FSRMSG1, FSRMSG2,
      -FFIND, FFINAME, FFIFOO, FFIFOF, FFIFOLD, FFIFILE, -FCOPY, FCOPNAME,
      FCOPFOLD, FCOPFOLDO, FCOPFILE, FCOPFILEO, FCOPFOGO, FCOPFIGO, FCOPFOO, FCOPFIO };

  if( (fnames=(FNAME *)lalloc(max_fname*(long)sizeof(FNAME),-1)) == 0 )
  {
    form_alert( 1, FSNOMEM1 );
    return 0;
  }
  for( i=0; i<sizeof(cols)/2; i++ )
  {
    *((int *)&u_object(fsel,cols[i][0])->ob_spec.index+1) = _wcolors [cols[i][1]];
#ifdef WINDOWS
    u_object(fsel,cols[i][0])->ob_state = wstates[wcolor_mode][cols[i][1]];
#endif
  }
#ifdef WINDOWS
  for( i=sizeof(atts), j=0, ptr=atts; --i>=0; ptr++ )
    if( *ptr<0 ) j = -*ptr;
    else ob_att( j, *ptr );
  if( pfont_id!=1 )		/* 007 */
    for( i=sizeof(teds), j=0, ptr=teds; --i>=0; ptr++ )
      if( *ptr<0 ) j = -*ptr;
      else ob_ted( j, *ptr );
  proc = shel_context(0L);
  i = Dgetdrv();
  dgetpath( auxpath );
  if( !mint_preem )	/* 006: conditional */
  {
    shel_context(proc);
    Dsetdrv(i);
    Dsetpath( auxpath );
  }
  CJar( 0, IDT_cookie, &idt_fmt );	/* 004: reread IDT */
  dial_pall(0);		/* 004 */
#endif
  form_center( fsel, &fs_rect.x, &fs_rect.y, &fs_rect.w, &fs_rect.h );
  /* 004 if( pathlen==32767 ) */
  strncpy( old_templ, pathend(path), sizeof(old_templ) );
  old_templ[sizeof(old_templ)-1] = 0;
  maxpath = pathlen;
  if( !check_path( path, 0 ) ) return 0;
  fsel_type();
  next_obj=fsfile;
  for( i=FSDA; i<FSDA+9; )
    hide_if( fsel, 1, i++ );
  for( i=2; i<12; i++ )
    if( (ptr=strchr(u_obspec(fselind[EXTLIST],i)->free_string,' ')) != 0 )
        *ptr=0;
  otype = ftype = SNAME;
  curfile=files=folders=0;
  edit_obj=-1;
  old = Fgetdta();
  Fsetdta( &dta );
  drvmap = Drvmap();
  (ttl_ptr = fsel[FSTITLE].ob_spec.tedinfo)->te_ptext = title;
  add_title(-2);
  (top = fsel[FSPATH].ob_spec.tedinfo)->te_ptext = toppath;
  top->te_txtlen = TOPLEN;
  x_form_filename( fsel, fsfile, 0, file );
  strcpy( toppath, path );
  path[0] = '\0';
  resolve( path, toppath, top, &drv, 0, 1 );
  set_names( -1, 0, 0 );
  set_type( -1, 0 );
  fsel[FSOSMALL].ob_height = fsel[FSFSMALL].ob_height = fsel[FSFSMALL2].ob_height = 0;
  hide_if( fsel, 0, FSFSMALL );
  hide_if( fsel, 0, FSOSMALL );
  hide_if( fsel, 0, FSFSMALL2 );
  fsel[FSITEMS].ob_spec.tedinfo->te_ptext[0] =
      fsel[FSBYTES].ob_spec.tedinfo->te_ptext[0] = 0;
  last_i = -1;
  last_b = -1L;
  draw_obj(0);
  wupdate( path, 0 );
  test_exmatch();
  new_filenm();
  do
  {
    cont = 1;
    do
    {
      if (edit_obj != next_obj)
        if (next_obj != 0)
          if( next_obj==fsffile || next_obj==FSFFOLD1 )
          {
            if( curfile )
            {
              add_title(-1);
              info_ok=0;
              set_curfile( 0, 0 );
              info_ok=1;
            }
            set_curfile( cf_top=next_obj, 1 );
            edit_obj = next_obj = 0;
          }
          else
          {
            if( !edit_obj ) curfile_off(0);
            i = edit_obj;
            edit_obj = next_obj;
            next_obj = 0;
            objc_edit( fsel, edit_obj, 0, &edit_idx, ED_INIT );
            if( i == FSPATH )     /* was editing path */
            {
              ret = FSUPDATE;
              cont = 0;
              goto bottom;
            }
          }
      which = _multi((long)fsel);
      if (which & MU_BUTTON)
      {
        if( m_obj == NIL) ring_bell();
        else if( !m_obj || m_obj==FSITEMS || m_obj==FSBYTES ) curfile_off(0);
        else if( br==2 && m_obj == FSPATH )
        {
          i = (mx - fsel[FSPATH].ob_x - fsel[0].ob_x -
              ((fsel[FSPATH].ob_width-26*char_w)>>1)) / char_w +
              top->te_ptext - toppath;
          if( i<strlen(toppath) && (ptr=strchr( toppath+i+1, '\\' )) != 0 )
          {
            strcpy( ptr+1, pathend(toppath) );
            ret = FSUPDATE;
            cont = 0;
          }
        }
        else
        {
          i = edit_obj;
          fsel[FSPATH].ob_flags &= ~TOUCHEXIT;  /* enable cursor mvmnt */
          cont = x_form_mouse( fsel, mx, my, br, &edit_obj, &m_obj, &edit_idx );
          fsel[FSPATH].ob_flags |= TOUCHEXIT;   /* reset for dclick */
          if( edit_obj!=i )
            if( i==FSPATH  )        /* was editing path */
            {
              next_obj = edit_obj;
              ret = FSUPDATE;
              cont = 0;
              goto bottom;
            }
            else if( !i ) curfile_off(0);
          if( !cont )
          {
            if( (ret = m_obj) == FSOK && curfile>0 ) curfile_off(0);
          }
          else next_obj = m_obj;
        }
      }
      else if (which & MU_KEYBD)
      {
        cont = 1;
        i = kr&0xff00;
        if( ks&8 )
          if( i==0x0100 )	/* Alt+Esc */
          {
            cont=0;
            ret = fsclose;
          }
          else                  /* Alt+drive */
          {
            i = key_2asc(key);
            if( i>='A' && i<='Z' && drvmap&(1L<<(i-'A')) )
            {
              cont=0;
              toppath[0] = i;
              toppath[1] = ':';
              strcpy( toppath+2, pathend(path+2) );
              ret = FSUPDATE;
            }
            else if( i=='\\' )
            {
              ret = FSROOT;
              cont = 0;
            }
#ifdef WINDOWS
            else if( (ret=scan_alts( fsel, i, 1 )) > 0 ) cont=0;
#endif
          }
        else if( (ks&4) || !edit_obj )
          		/* Control held or in search mode already */
        {
          switch(i)
          {
            case 0x0100:  /* Esc */
              add_title(-1);
              break;
            case 0x0E00:  /* Bksp */
              if( (i = strlen(toptitl)) == 1 ) add_title(-1);
              else if( i>0 )
              {
                toptitl[--i]=0;
                add_title(0);
              }
              break;
            case 0x4700:
            case 0x7700:  /* Clr/Home */
              info_ok = 0;
              dflt_curfile();
              set_curfile( 0, 0 );
              set_names( (cf_top==FSFFOLD1)+1, (ks&3) ? 9999 : -9999, 3 );
              info_ok = 1;
              set_curfile( (ks&3) ? cf_top+9 : cf_top, 1 );
              break;
            case 0x4800:  /* Up arrow */
              if( (cf_top==FSFFOLD1 ? folders : files) == 0 ) goto bottom;
              dflt_curfile();
              if( ks&3 )                  /* page up */
              {
                if( curfile==cf_top )    /* already at top */
                {
                  info_ok = 0;
                  set_curfile( 0, 0 );
                  set_names( (cf_top==FSFFOLD1)+1, -10, 3 ); /* page up */
                  info_ok = 1;
                }
                set_curfile( cf_top, 1 );        /* set to top line */
              }
              else if( curfile<=0 ) set_curfile( cf_top+9, 1 );   /* no current, go to bottm */
              else if( curfile>cf_top ) set_curfile( curfile-1, 1 ); /* up one */
              else if( cf_top==FSFFOLD1 ? topfold : topfile )     /* go up one, if possible */
                  set_names( (cf_top==FSFFOLD1)+1, -1, 1|2|4 );
              toggle();
              break;
            case 0x5000:  /* Down arrow */
              if( (cf_top==FSFFOLD1 ? folders : files) == 0 ) goto bottom;
              dflt_curfile();
              if( ks&3 )
              {
                if( curfile==cf_top+9 )
                {
                  info_ok = 0;
                  set_curfile( 0, 0 );
                  set_names( (cf_top==FSFFOLD1)+1, 10, 3 );
                  info_ok = 1;
                }
                set_curfile( cf_top+9, 1 );
              }
              else if( curfile<=0 ) set_curfile( cf_top, 1 );
              else if( curfile<cf_top+9 ) set_curfile( curfile+1, 1 );
              else if( cf_top==FSFFOLD1 ? oobj2ind(curfile)<folders-1 :
                  fobj2ind(curfile)<files-1 )
                  set_names( (cf_top==FSFFOLD1)+1, 1, 1|2|4 );
              toggle();
              break;
            case 0x1c00:
            case 0x7200:     /* ^Ret or ^Enter */
              ret = FSOK;
              cont=0;
              break;
            default:
              toggle();
              if( edit_obj ) cf_top = !fsel_1col ? FSFFOLD1 : fsffile;
              if( (ret = key_2asc(key)) != 0 && add_title(ret) )
              {
                new_filenm();
                next_obj = 0;
              }
              else if( !edit_obj ) goto nextch;
              else toggle();
          }
        }
        else
nextch:     if( i==0x6100 )          /* Undo */
        {
          ret = FSCANCEL;
          cont = 0;
        }
        else if( i==0x0F00 )	/* Tab */
        {
          i = ks&3;
          switch( edit_obj )
          {
            case 0:
              if( fsel_1col ) next_obj = i ? fsfile : FSPATH;
              else if( cf_top==FSFFOLD1 ) next_obj = i ? fsfile : FSFFILE1;
              else next_obj = i ? FSFFOLD1 : FSPATH;
              break;
            case FSPATH:
              next_obj = i ? fsffile : fsfile;
              break;
            case FSFILE:
            case FSNAME2:
              next_obj = i ? FSPATH : (!fsel_1col ? FSFFOLD1 : fsffile);
          }
        }
        else if( i>=0x3b00 && i<=0x4400 )	/* Fkey */
        {
          i = ((i-0x3b00)>>8) + PATH1;
          toggle();
          goto path_pop;
        }
        else if( i>=0x5400 && i<=0x5d00 )	/* Shift-Fkey */
        {
          ks |= 2;
          i = ((i-0x5400)>>8) + PATH1;
          toggle();
          goto path_pop;
        }
        else
        {
          m_obj = edit_obj;
          if( edit_obj==FSPATH && !(ks&0xc) )
              switch(i)
              {
                case 0x0100:    /* Esc */
                  toppath[0] = '\0';
shl:              toggle();
                  top->te_ptext = toppath;
                  top->te_txtlen = TOPLEN;
                  draw_obj(FSPATH);
                  edit_idx = 0;
                  toggle();
                  goto bottom;
                case 0x1c00:
                case 0x7200:    /* CR */
                  next_obj = fsfile;    /* causes update */
                  goto bottom;
                case 0x0e00:    /* Bksp */
                  if( ks&3 )
                  {
                    strcpy( toppath, top->te_ptext+edit_idx );
                    goto shl;
                  }
                case 0x4b00:    /* left */
                  if( ks&3 ) goto shl;
                  if( edit_idx<=1 ) move_path( top, -1, 1 );
                  break;
                case 0x4d00:    /* right */
                  if( ks&3 )
                  {
                    if( end_path( top, 1 ) ) goto bottom;
                  }
                  else if( edit_idx>=edit_max ) move_path( top, 1, 1 );
                  break;
              }
          cont = form_keybd( fsel, m_obj, m_obj, kr, &m_obj, &kr );
          if( cont && kr )
          {
            x_objc_edit( fsel, edit_obj, kr, ks, &edit_idx, ED_CHAR );
            if( edit_obj==FSPATH && edit_idx>=edit_max ) move_path( top, 1, 1 );
          }
          else if( !cont ) ret = m_obj;
          else next_obj = m_obj;
        }
      }
bottom:
      if (!cont || (next_obj != edit_obj && next_obj != 0) ) toggle();
    }
    while( cont );
    dclick = ret<0;
    ret &= 0x7fff;
again:
    if( u_object(fsel,ret)->ob_flags&(1<<13) &&
        !is_sel(fsel,ret) ) set_state( 1, ret, 1 );
    switch(ret)
    {
      case FSPLEFT:
        move_path( top, -1, 0 );
        break;
      case FSPRIGHT:
        move_path( top, 1, 0 );
        break;
      case FSFBIG:
      case FSFBIG2:
        curfile_off(ks);
        page_it( 1, fsfbig, dclick );
        break;
      case FSFSMALL:
      case FSFSMALL2:
        curfile_off(ks);
        slider( files, fsfbig, fsfsmall, &topfile, 1 );
        break;
      case FSOBIG:
        curfile_off(ks);
        page_it( 2, FSOBIG, dclick );
        break;
      case FSOSMALL:
        curfile_off(ks);
        slider( folders, FSOBIG, FSOSMALL, &topfold, 2 );
        arrow();
        break;
      case FSFUP:
      case FSFUP2:
        curfile_off(ks);
        set_names( 1, dclick ? -1000 : -1, 3 );
        break;
      case FSFDOWN:
      case FSFDOWN2:
        curfile_off(ks);
        set_names( 1, dclick ? 1000 : 1, 3 );
        break;
      case FSOUP:
        curfile_off(ks);
        set_names( 2, dclick ? -1000 : -1, 3 );
        break;
      case FSODOWN:
        curfile_off(ks);
        set_names( 2, dclick ? 1000 : 1, 3 );
        break;
      case FSDUP:
        redo_map( 1, *path, &drv, dclick ? -100 : -1, 0 );
        break;
      case FSDDOWN:
        redo_map( 1, *path, &drv, dclick ? 100 : 1, 0 );
        break;
      case FSFNSDT:
        set_type( 1, 1 );
        break;
      case FSONSDT:
        set_type( 2, 1 );
        break;
      case FSOK:
        if( in_copy )
        {
          end_copy( path, top, &drv, 1 );
          break;
        }
        else if( curfile>0 )
          if( !folders && cf_top==FSFFOLD1 ) break;
          else if( fsel_1col && fnames[i=fobj2ind(curfile)].att&0x10 )
          {
            set_state( 0, FSOK, 1 );
            ret = curfile;
            goto openfold;
          }
          else if( !edit_obj && cf_top==FSFFOLD1 )
          {
            i = oobj2ind(curfile);
            set_state( 0, FSOK, 1 );
            ret = curfile;
            goto openfold;
          }
      case FSCANCEL:
        if( !in_copy ) quit++;
        else end_copy( path, top, &drv, 0 );
        break;
      case FSHELP:
        oth_form( FSHELPTXT, 0, 0L, 0L, 0L );
        break;
      case FSROOT:
      case FSROOT2:
        add_title(-1);
        if( (ptr=pathend(path)) != path+3 )
        {
          toppath[0] = path[0];
          toppath[1] = ':';
          toppath[2] = '\\';
          strcpy( toppath+3, pathend(path+2) );
        }
        update_path( path, top, &drv, 0, 0, 1 );
        break;
      case FSCLOSE:
      case FSCLOSE2:
        add_title(-1);
        if( pathend(path) != path+3 )
        {
          strcpy( toppath, path );
          ptr = pathend(toppath);
          --ptr;
          while( *--ptr != '\\' );
          strcpy( ptr+1, pathend(path) );
          update_path( path, top, &drv, 0, 0, 1 );
          break;
        }
      case FSUPDATE:
        update_path( path, top, &drv, 0, 0, 0 );
        break;
      case FSTOOLS:
        if( (i=mn_popup( OPLIST, FSTOOLS )) != 0 ) switch(i)
        {
          case OPMOVE:
            in_copy = -1;
            goto copy;
          case OPCOPY:
            in_copy = 1;
copy:	      strcpy( auxpath, toppath );
            j = curfile<0 ? -curfile : 1;
            if( (copy_names = (char *)lalloc( j*13+1, -1 )) == 0 )
            {
              form_alert( 1, FSNOMEM3 );
              in_copy = 0;
            }
            else if( (copy_buf = (char *)lalloc( COPYSIZE, -1 )) == 0 )
            {
              lfree(copy_names);
              form_alert( 1, FSNOMEM3 );
              in_copy = 0;
            }
            else
            {
              set_title( FSCOPY );
              for( i=0, ptr=copy_names; i<files+folders; i++ )
                if( fnames[i].att&0x8000 )
                {
                  x_form_filename( (OBJECT *)fnames[i].name, -1, 1, ptr+1 );
                  j = strlen(ptr+1);
                  *ptr = fnames[i].att&FA_SUBDIR ? -j : j;
                  ptr += j+1;
                }
              *ptr = 0;
              curfile_off(0);
            }
            break;
          case OPFIND:
            if( oth_form( FFIND, 0, 0L, oth_findi, oth_find ) == OTH_FOUND )
              if( check_path( auxpath, 0 ) )
              {
                newpath(auxpath);
                update_path( path, top, &drv, 0, 0, 0 );
                add_title(-1);
                cf_top = fsffile;
                for( ptr=root; *ptr && add_title(*ptr); ptr++ );
                if( !*ptr )
                {
                  new_filenm();
                  next_obj = 0;
                }
                else
                {
                  add_title(-1);
                  curfile_off(0);
                  x_form_filename( fsel, fsfile, 0, root );
                  draw_obj(fsfile);
                  test_exmatch();
                }
              }
            break;
          case OPFREE:
            bee();
            i = Dfree( &diskinfo, toppath[0]-'A'+1 );
            arrow();
            if( !i )
            {
              spf( temp, FSFR, diskinfo.b_total*diskinfo.b_clsiz*diskinfo.b_secsiz,
                  diskinfo.b_free*diskinfo.b_clsiz*diskinfo.b_secsiz );
              form_alert( 1, temp );
            }
            break;
          case OPFOLD:
            if( oth_form( FFOLD, FFNAME, "", 0L, oth_new ) == OTH_OK )
                update_path( path, top, &drv, 1, 1, 1 );
            break;
          case OPINFO:
opinfo:     get_sel(-1);
            i = 0;
            tool_parm2 = OPINFO;
            while( (j=get_sel(1)) >= 0 )
              if( (j=oth_form( FINFO, FSROLD, fnames[tool_parm1=j].name,
                  oth_reni, oth_ren )) == OTH_OK ) i++;
              else if(j) break;
            if(i) update_path( path, top, &drv, 1, 1, 1 );
            break;
          case OPDEL:
            get_sel(-1);
            i = 0;
            tool_parm2 = OPDEL;
            while( (j=get_sel(1)) >= 0 )
              if( (j=oth_form( FINFO, FSROLD, fnames[tool_parm1=j].name,
                  oth_reni, oth_del )) == OTH_OK ) i++;
              else if(j) break;
            if(i) update_path( path, top, &drv, 1, 1, 1 );
            break;
        }
        break;
      case FSSORT:
        if( (i=mn_popup( SORTLIST, FSSORT )) != 0 )
          if( --i != sort_type )
          {
            clear_last(0);
            j = sort_type;
            if( (sort_type=i)!=4 && j!=4 )
            {
              curfile_off(0);
              sort_it();
              set_names( -1, 0, 3 );
            }
            else update_path( path, top, &drv, 1, 1, 0 );
          }
        break;
      case FSPTHB:
        if( (i=mn_popup( PATHLIST, FSPTHB )) != 0 )
        {
path_pop: ptr=u_obspec(fselind[PATHLIST],i)->free_string;
          if( ks&3 )
          {
            strncpy( ptr, toppath, 34 );
            *pathend(ptr) = 0;
          }
          else
          {
            newpath(ptr);
            update_path( path, top, &drv, 0, 0, 0 );
          }
        }
        break;
      case FSFEXT:
        if( (i=mn_popup( EXTLIST, ret ) ) != 0 )
        {
          ptr=u_obspec(fselind[EXTLIST],i)->free_string;
          if( ks&3 )
            if( i==1 ) ring_bell();	/* 004 */
            else
            {
              if( (ptr2 = strchr(pathend(toppath),'.')) == 0 ) ptr2="";
              if( strlen(ptr2) > 4 ) form_alert( 1, FSELONG );
              else strcpy( ptr+1, ptr2 );
            }
          else
          {
            if( i==1 ) ptr = old_templ;	/* 004 */
            add_ext(ptr);
            update_path( path, top, &drv, 0, 0, 1 );
          }
        }
        break;
      case FS1COL:
        fsel_1col ^= 1;
        curfile_off(0);
        fsel_type();
        set_names( -1, 0, 0 );
        draw_part( fsel, FSFOUTER2 );
        set_state( 0, FS1COL, 0 );
        draw_obj(FS1COL);	/* always draw */
        ret = next_obj = 0;
        if( edit_obj!=FSPATH ) edit_obj = fsfile;
        update_path( path, top, &drv, 1, 0, 0 );
        break;
      case FSALL1:
      case FSALL2:
        sel_all();
        break;
      default:
        if( ret >= FSDA && ret <= FSDA+8 )
        {
          if( drv!=ret )
          {
            toppath[0] = u_obspec(fsel,ret)->obspec.character;
            toppath[1] = ':';
            strcpy( toppath+2, pathend(path+2) );
          }
          ret = FSUPDATE;
          goto again;
        }
        else if( ret >= fsffile && ret <= fsffile+9 )
        {
          if( fsel_1col && fnames[i=fobj2ind(ret)].att&0x10 )
              goto clickfold;
          if( !(ks&3) && ret==curfile ) curfile_off(0);
          else
          {
            if( curfile && cf_top==FSFFOLD1 )
            {
              if( !folders ) set_curfile( 0, 0 );
              if( !(ks&3) ) add_title(-1);
            }
            cf_top = fsffile;
            set_curfile( ret, (ks&3) ? 3 : 1 );
          }
          if( dclick )
          {
            ret = FSOK;
            quit++;
          }
          else
          {
            while( get_but() );
            if( curfile ) next_obj = 0;
            if( ks&4 ) goto opinfo;	/* 004 */
          }
        }
        else if( ret >= FSFFOLD1 && ret <= FSFFOLD1+9 )
        {
          i = oobj2ind(ret);
clickfold:
          if( !files && cf_top==fsffile ) set_curfile( 0, 0 );
          while( get_but() );
          if( !dclick && (ks&7) )	/* 004: was ks&3 */
          {
            cf_top = fsel_1col ? fsffile : FSFFOLD1;
            set_curfile( ret, 2 );
            if( curfile ) next_obj = 0;
            if( ks&4 ) goto opinfo;	/* 004 */
          }
          else
          {
            curfile_off(0);
openfold:   set_state( 0, ret, 1 );
            x_form_filename( (OBJECT *)fnames[i].name, -1, 1, temp );
            if( check_path( path, strlen(temp) ) )
            {
              /* these two values get changed during update */
              i = !fsel_1col ? topfold : topfile;
              j = slashes(path);
              strcpy( toppath, path );
              strcpy( pathend(toppath), temp );
              strcat( toppath, pathend(path)-1 );
              update_path( path, top, &drv, 0, 0, 0 );
              last_fold[j] = i;
            }
          }
        }
    }
    if( u_object(fsel,ret)->ob_flags & (1<<13) )
      if( !get_but() ) set_state( 0, ret, 1 );
      else goto again;
    else if( (u_object(fsel,ret)->ob_flags & (EXIT|RBUTTON)) == EXIT )
        set_state( 0, ret, !quit );
    if( !quit ) toggle();
  }
  while( !quit );
  if( (*but = ret==FSOK) != 0 )
    if( sels<=1 || curfile>=0 || cf_top==FSFFOLD1 )
    {
      x_form_filename( fsel, fsfile, 1, file );
      if( sels>1 ) *(file+strlen(file)+1) = 0;
    }
    else
    {
      get_sel(0);
      ptr = file;
      for( i=sels; --i>=0 && (j=get_sel(1)) >= 0; )
      {
        x_form_filename( (OBJECT *)fnames[j].name, -1, 1, ptr );
        ptr += strlen(ptr) + 1;
      }
      *ptr = 0;
    }
  else if( sels>1 ) *(file+strlen(file)+1) = 0;
  curfile_off(0);
  /* restore original template if new type was used with old fsel function */
  if( pathlen==32767 && strpbrk(pathend(path),"[]{}") )
      strcpy( old_templ, pathend(path) );
#ifdef WINDOWS
  dial_pall(1);		/* 004 */
  redraw_all( &fs_rect );
  while( get_but()&1 );
  reset_butq();
  shel_rpath();
#else
  form_dial( FMD_FINISH, 0, 0, 0, 0, Xrect(fs_rect) );
#endif
  Fsetdta(old);
  if( mint_preem ) shel_context(proc);		/* 006: moved here */
  lfree(fnames);
  return(1);
}

int _fsel_input( char *path, char *file, long but )
{
#ifdef WINDOWS
  if( test_update( (void *)_fsel_input ) ) return 0;
#endif
  return( __x_fsel_input( path, 32767, file, 1, (int *)but, FSDTITL ) );
}

int _fsel_exinput( char *path, char *file, long but, long title )
{
#ifdef WINDOWS
  if( test_update( (void *)_fsel_exinput ) ) return 0;
#endif
  return( __x_fsel_input( path, 32767, file, 1, (int *)but, (char *)title ) );
}

typedef struct { char *path, *file, *title; } XFSI;
int _x_fsel_input( int pathlen, int sels, XFSI *x, int *but )
{
#ifdef WINDOWS
  if( test_update( (void *)_x_fsel_input ) ) return 0;
#endif
  return( __x_fsel_input( x->path, pathlen, x->file, sels, but, x->title ) );
}
