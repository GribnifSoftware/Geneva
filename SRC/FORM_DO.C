#define NO_FDI  0x71

#include "new_aes.h"
#include "vdi.h"
#include "tos.h"
#include "win_var.h"
#include "win_inc.h"
#define _FORMS
#include "ierrno.h"
#ifdef GERMAN
  #include "german\wind_str.h"
#else
  #include "wind_str.h"
#endif
#include "string.h"
#include "stdlib.h"
#include "xwind.h"
#include "formcnst.h"

#define	ALSTRLEN	80
#define	ALBUTLEN	25
static char string[5][ALSTRLEN+1], buttons[3*ALBUTLEN+3];	/* 007: changed sizes */

static OBJECT2 alert[] = {
-1, 1, 10, X_PROPFONTRSZ, G_BOX, NONE, X_MAGIC>>8, X_PREFER, 0xFD1100L, 0, 0, 0, 0,
2, -1, -1, X_MOVER, G_BUTTON, TOUCHEXIT, 0, NORMAL, 0x0L, 0, 0, 16, 0,
3, -1, -1, 0, G_ICON, NONE, 0, NORMAL, 0x0L, 8, 0, 32, 31,
4, -1, -1, 0, G_STRING, NONE, 0, NORMAL, (long)&string[0][0], 6,0, 240,1,	/* 007 */
5, -1, -1, 0, G_STRING, NONE, 0, NORMAL, (long)&string[1][0], 6,1, 240,1,
6, -1, -1, 0, G_STRING, NONE, 0, NORMAL, (long)&string[2][0], 6,2, 240,1,
7, -1, -1, 0, G_STRING, NONE, 0, NORMAL, (long)&string[3][0], 6,3, 240,1,
8, -1, -1, 0, G_STRING, NONE, 0, NORMAL, (long)&string[4][0], 6,4, 240,1,
9, -1, -1, 0, G_BUTTON, 0x5, X_MAGIC>>8, X_PREFER, 0L, 1,6, 11,1,
10,-1, -1, 0, G_BUTTON, 0x5, X_MAGIC>>8, X_PREFER, 0L, 13,6, 11,1,
0, -1, -1, 0, G_BUTTON, 0x25, X_MAGIC>>8, X_PREFER, 0L, 25,6, 11,1};

/*BITBLK bb = { 0L, 4, 32, 0, 0, 1 };*/
ICONBLK icb = { 0L, 0L, "", 0x1000|0, 0, 0, 3, 0, 32, 31, 0, 31, 0, 0 };

#define do_alts   curapp->flags.flags.s.kbd_equivs
#define undo_alts curapp->flags.flags.s.undo_equivs

static char gchar[2]="x";	/* 007: moved */

void force_mouse( int flag )	/* 004: don't use curapp anymore */
{
  static int h;
  static MFORM *old;

  if( !flag )
  {
    old = last_mouse;
    h = _graf_mouse( X_MGET, 0L, 0 );
    _graf_mouse( 0, 0L, 0 );
    _graf_mouse( X_MRESET, 0L, 0 );
  }
  else
  {
    while( h-- > 0 ) _graf_mouse( M_OFF, 0L, 0 );
    if( flag==1 ) _graf_mouse( USER_DEF, old, 0 );
  }
}

int _form_alert( int dflt, char *str )
{	/* 007: various changes for proportional strings */
  int w, h, i, j, w2, w3, dif, dum;
  Rect r;
  static int icon_color[] = { (BLACK<<12)|(YELLOW<<8),
      (BLACK<<12)|(YELLOW<<8), (BLACK<<12)|(RED<<8) };
  OBJECT2 *a;

  if( !desktop ) return 1;
  if( test_update( (void *)_form_alert ) ) return 0;
  if( !*str++ )
  {
err: str=ALERR;
  }
  h = char_h;
  w = char_w;
  if( *str <= '0' || *str > '3' )
  {
    alert[2].ob_flags |= HIDETREE;
    w3 = h;
  }
  else
  {
    alert[2].ob_y = char_h;
    alert[2].ob_flags &= ~HIDETREE;
    alert[2].ob_spec.iconblk = &icb;
/*    bb.bi_pdata = image[*str-'1'];*/
    icb.ib_pdata = images[*str-'1'];
    icb.ib_pmask = masks[*str-'1'];
    icb.ib_char = vplanes >= 4 ? icon_color[*str-'1'] : (BLACK<<12);
    w += 40;
    w3 = 31+char_h;
  }
  if( !*++str || !*++str ) goto err;
  ted_font( pfont_mode, X_SYSFONT, 0, &dum, &dum );
  for( a=(OBJECT2 *)u_object((OBJECT *)alert,i=3), w2=0; i<8; i++, a++ )
    if( *str && *str!=']' )
    {
      a->ob_x = w;
      a->ob_y = h;
      a->ob_height = char_h;
      a->ob_flags &= ~HIDETREE;
      for( j=0; *++str && *str != ']' && *str != '|'; )
        if( j<ALSTRLEN ) string[i-3][j++] = *str;
      string[i-3][j] = '\0';
      a->ob_width = prop_extent( pfont_mode, string[i-3], &dum );
      if( a->ob_width>w2 ) w2 = a->ob_width;
      h+=char_h;
    }
    else a->ob_flags |= HIDETREE;
  if( w3>h ) h=w3;
  h += char_h;
  w += w2 + char_w;
  while( *str != ']' && *str ) str++;
  if( !*str || *++str!='[' ) goto err;
  strncpy( buttons, ++str, sizeof(buttons)-1 );
  str = buttons;
  for( w2=w3=0, a=(OBJECT2 *)u_object((OBJECT *)alert,i=8); i<11; i++, a++ )
    if( dflt!=-99 && *str && *str != ']' )
    {
      a->ob_y = h-2;
      a->ob_typex = 0;
      a->ob_flags &= ~HIDETREE;
      a->ob_spec.free_string = str;
      for( j=0; *str && *str != ']' && *str != '|'; str++ )
        if( *str!='[' ) j++;
        else a->ob_typex = X_RADCHKUND;
      if( *str=='|' ) *str++ = 0;
      else *str = 0;
      j = prop_extent( pfont_mode, a->ob_spec.free_string, &dum );
      if( j>w2 ) w2=j;
      w3++;
    }
    else
    {
      a->ob_flags |= HIDETREE;
      *str = 0;
    }
  if( dflt != -99 )
  {
    if( !w3 ) goto err;
    if( (j=w2+3*char_w)*w3 > w ) w=j*w3;
    if( dflt<1 || dflt>w3 ) dflt = w3;
    for( a=(OBJECT2 *)u_object((OBJECT *)alert,i=8), w2=(w-j*w3+char_w*2)>>1; w3--; i++, w2+=j, a++ )
    {
      a->ob_width = j-(char_w<<1);
      a->ob_height = char_h + 2;
      a->ob_x = w2;
      if( i==dflt+7 ) a->ob_flags |= DEFAULT;
      else a->ob_flags &= ~DEFAULT;
    }
  }
  alert[0].ob_width = w;
  alert[0].ob_height = h+char_h+(char_h>>1);
  alert[1].ob_x = w - 17;
  alert[1].ob_height = char_h<16 ? char_h : 16;
  form_center( (OBJECT *)alert, &asc.x, &asc.y, &asc.w, &asc.h );
  if( dflt==-99 )
  {
    alert[0].ob_height -= char_h<<1;
    _objc_draw( alert, 0L, 0, 8, 0, 0, 0, 0 );
    return 0;
  }
  force_mouse(0);
  if( settings.flags.s.alerts_under_mouse )
  {
    dif = alert[0].ob_x - asc.x;
    if( (asc.x = g_mx - asc.w/2) < (i=desktop->working.x) ) asc.x =
        desktop->working.x;
    else if( asc.x+asc.w > (i+=desktop->working.w) ) asc.x = i-asc.w;
    if( (asc.y = g_my - asc.h + char_h) < (i=desktop->working.y) ) asc.y =
        desktop->working.y;
    else if( asc.y+asc.h > (i+=desktop->working.h) ) asc.y = i-asc.h;
    alert[0].ob_x = asc.x+dif;
    alert[0].ob_y = asc.y+dif;
  }
  _form_dial( X_FMD_START, 0L, &asc );
  dial_pall(0);		/* 004 */
  _objc_draw( alert, 0L, 0, 8, 0, 0, 0, 0 );
  is_alert++;
  sel_if((OBJECT *)alert,i = _form_do( (OBJECT *)alert, 0 ),0);
  is_alert--;
  dial_pall(1);		/* 004 */
  _form_dial( X_FMD_FINISH, 0L, &asc );
  force_mouse(-1);
  return(i-7);
}

void fix_int( int *i, int cel )
{
  *i = (*i&0xFF)*cel + (*i>>8);
}

void line_init(int bp)
{
  _vsl_color( bp );
  _vsl_type( 1 );
  _vswr_mode( 1 );
}

void cdecl draw_line( int i, ... )
{
  v_pline( vdi_hand, 2, &i );
}

void adjust_rect( OBJECT *obj, Rect *r, int frame )
{
  int i, j;
  char round=0;
  OBDESC od;

  i = j = frame ? (char)(((char)obj->ob_type==G_BUTTON ? but_spec(obj) :
      get_spec(obj))>>16) : 0;
  get_obdesc( (OBJECT2 *)obj, r, &od );
  /* rounded, outlined is a special case */
  if( (!form_app || form_app->flags.flags.s.round_buttons) &&
      od.magic && (od.state&(X_ROUNDED|OUTLINED)) == (X_ROUNDED|OUTLINED) )
  {
    i = j-2;
    round++;
  }
  else if( i>-3 && od.state&OUTLINED ) i = -3;
  if( i<0 )
  {
    r->x += i;
    r->y += i;
    r->w -= (i<<1);
    r->h -= (i<<1);
  }
  if( od.state&SHADOWED )
  {
    i = (abs(j)<<1);
    if( j>1 ) i -= j-1;
    if( round )
      if( (i-=2) <= 0 ) return;
    r->w += i;
    r->h += i;
  }
}

int is_xusrdef( APP *ap, OBJECT *tree )
{
  int typ;

  return (tree->ob_state&X_MAGMASK) == X_MAGIC &&
      ((typ=get_typex(ap,(OBJECT2 *)tree)) == X_USRDEFPRE ||
      typ == X_USRDEFPOST);
}

long get_spec(OBJECT *tree )
{
  long ptr;

  ptr = tree->ob_spec.index;
  if( tree->ob_flags & INDIRECT ) return *(long *)ptr;
  if( is_xusrdef(form_app,tree) ) return ((USERBLK *)ptr)->ub_parm;
  return( ptr );
}

long but_spec( OBJECT *o )
{
  if( (o->ob_flags&(DEFAULT|EXIT) ) == (DEFAULT|EXIT) )
      return( 0x00FD1170L );
  if( o->ob_flags&(DEFAULT|EXIT) ) return( 0x00FE1170L );
  return( 0x00FF1170L );
}

void objc_toggle( long tree, int obj )
{
  Rect root;

  objc_xywh(tree, ROOT, &root);
  adjust_rect( (OBJECT *)tree, &root, 1 );
  change_objc((OBJECT *)tree, form_app, obj, &root, OB_STATE(obj)^SELECTED, 1);
}

void objc_sel( long tree, int obj )
{
  if ( !(OB_STATE(obj) & SELECTED) ) objc_toggle(tree, obj);
}

void objc_dsel( long tree, int obj )
{
  if (OB_STATE(obj) & SELECTED) objc_toggle(tree, obj);
}

/************* Keyboard manager and subroutines ***************/

int find_def( OBJECT *ob, int obj )
{
  ob = u_object(ob,obj);
/*  if (HIDETREE & ob->ob_flags) return (FALSE);	005 */
    if ( ob->ob_flags&DEFAULT )
      if ( !(DISABLED & ob->ob_state) )
            fn_obj = obj;   /* Record object number                 */
  return (TRUE);
}

int find_ndef( OBJECT *ob, int obj )
{
  ob = u_object(ob,obj);
/*  if (HIDETREE & ob->ob_flags) return (FALSE);	005 */
    if ( fn_obj==NIL && (ob->ob_flags&(DEFAULT|EXIT|SELECTABLE)) ==
        (EXIT|SELECTABLE) && !(DISABLED & ob->ob_state) )
        fn_obj = obj;   /* Record object number                 */
  return (TRUE);
}

int find_tab( OBJECT *ob, int obj ) /* Look for target of TAB operation.    */
{                       /* Check for hidden subtree.           */
  int i;

  ob = u_object(ob,obj);
                          /* If not EDITABLE, who cares?          */
  if ( !(EDITABLE & ob->ob_flags) /*||
      ((i=ob->ob_type) != G_FTEXT && i != G_FBOXTEXT)*/ )
  {
    if (HIDETREE & ob->ob_flags) return (FALSE);  /* 004: moved here */
    return (TRUE);
  }
  if (fn_dir)             /* Check for forward tab match          */
  {
    if(fn_prev == fn_last) fn_obj = obj;
  }
  else if (obj == fn_last)/* Check for backward tab match         */
       fn_obj = fn_prev;
  fn_prev = obj;          /* Record object for next call.         */
  return (TRUE);
}

int find_edit( OBJECT *ob, int obj )   /* Look for next editable field.*/
{                       /* Check for hidden subtree.           */
  int i;

  ob = u_object(ob,obj);
                          /* If not EDITABLE, who cares?          */
  if ( !(EDITABLE & ob->ob_flags) /*||
      ((i=ob->ob_type) != G_FTEXT && i != G_FBOXTEXT)*/ )
  {
    if (HIDETREE & ob->ob_flags) return (FALSE);  /* 004: moved here */
    return (TRUE);
  }
  if( fn_obj==NIL ) fn_obj = obj;
  return (TRUE);
}

char *get_butstr( long tree, int obj, int *is_ted, int disab )
{
  char *ptr=0L;		/* 007: gchar is global */
  OBJECT *ob = u_object((OBJECT *)tree,obj);

  *is_ted = 0;
  if ( !fn_dir || fn_dir & ob->ob_flags )
    if ( !disab || !(DISABLED & ob->ob_state) )
    {
      switch( (char)ob->ob_type )
      {
        case G_BOXCHAR:
          *(ptr = gchar) = get_spec(ob)>>24;
          break;
        case G_BOXTEXT:
        case G_TEXT:
          ptr=(char *)((TEDINFO *)get_spec(ob))->te_ptext;
          *is_ted = 1;
          break;
        case G_FTEXT:
        case G_FBOXTEXT:
          ptr=(char *)((TEDINFO *)get_spec(ob))->te_ptmplt;
          *is_ted = 1;
          break;
        case G_TITLE:	/* 007 */
          ptr=fix_title((char *)get_spec(ob));
          break;
        case G_BUTTON:
        case G_STRING:
          ptr=(char *)get_spec(ob);
          break;
        case X_PROPSTR:		/* 007 */
          ptr=((PROPSTR *)get_spec(ob))->str;
          break;
      }
    }
  return(ptr);
}

int strxcmp( char *ptr1, char *ptr2 )
{
  if( *ptr1=='[' ) ptr1++;
  while( (*ptr1&0xdf) == (*ptr2&0xdf) )
  {
    if( !*ptr1 ) return(1);
    ptr1++;
    ptr2++;
    if( *ptr1=='[' ) ptr1++;
  }
  if( !*ptr2 )
  {
    while( *ptr1 == ' ' ) ptr1++;
    if( !*ptr1 ) return(1);
  }
  return(0);
}

int find_exit( OBJECT *ob, int obj )
{
  char *ptr, *ptr2;
  int dum;
  long tree = (long)ob;

  ob = u_object(ob,obj);
  if (HIDETREE & ob->ob_flags)
          return (FALSE);
  if( fn_obj==NIL )
    if( get_typex(form_app,(OBJECT2 *)ob) == X_RADCHKUND && ob->ob_flags&(1<<11) ) fn_obj = obj;
    else if( (ptr=get_butstr( tree, obj, &dum, 1 )) != 0 )
    {
      while( *ptr==' ' ) ptr++;
      for( ptr2=undo_ptr; *ptr2; ptr2+=strlen(ptr2)+2 )
        if( strxcmp(ptr,ptr2) ) fn_obj = obj;
    }
  return (TRUE);
}

int fnd_hu( OBJECT *ob, int obj, int type )
{
  ob = u_object(ob,obj);
  if( ob->ob_flags&HIDETREE ) return (FALSE);
  if( fn_obj==NIL )
    if( get_typex(form_app,(OBJECT2 *)ob) == type ) fn_obj = obj;
  return (TRUE);
}

int find_help( OBJECT *ob, int obj )
{
  return fnd_hu( ob, obj, X_HELP );
}

int find_undo( OBJECT *ob, int obj )
{
  return fnd_hu( ob, obj, X_UNDO );
}

void draw_alt( long tree, int obj, char *ptr, int off, int is_ted,
    int undraw )
{
  TEDINFO *ted;
  int w, j, type, dum, font;
  unsigned long spec;
  Rect g;
  unsigned int c;
  OBJECT *ob = u_object((OBJECT *)tree,obj);
  OBDESC od;

  objc_xywh( tree, obj, &g );
  ted = (TEDINFO *)(spec=get_spec(ob));
  j = !is_ted || ted->te_font==IBM;
  get_obdesc( (OBJECT2 *)ob, 0L, &od );
  if( (od.state&(X_MAGMASK|X_SMALLTEXT)) == (X_MAGIC|X_SMALLTEXT) ) j=0;
	/* 007: lots of changes after here */
  if( !j ) font = SMALL;
  else if( (!form_app || form_app->flags.flags.s.prop_font_menus) &&
        ((OBJECT2 *)tree)->ob_typex >= X_GRAYMENU &&
        ((OBJECT2 *)tree)->ob_typex <= X_PROPFONTRSZ ) font=pfont_mode;
  else font = IBM;
  ted_font( font, X_SYSFONT, 0, &dum, &dum );
  gchar[0] = *ptr;
  switch( ((type = (char)ob->ob_type)==G_STRING || type==X_PROPSTR) ? TE_LEFT :
      (is_ted ? ted->te_just : TE_CNTR) )
  {
    case TE_CNTR:
      g.x += (g.w - prop_extent( font, ptr-off, 0L )+1)>>1;
      /* no break */
    case TE_LEFT:
      *ptr = 0;
      g.x += prop_extent( font, ptr-off, 0L );
      *ptr = gchar[0];
      break;
    case TE_RIGHT:
      g.x += g.w - prop_extent( font, ptr, 0L );
      break;
  }
  if( (w = prop_extent( font, gchar, 0L )) < 4 ) {
    g.x -= (4-w)>>1;
    w = 4;
  }
/*****  007
  g.x += off*(w=j?char_w:6);
  if( type != G_STRING && type!=G_TITLE )
      switch( is_ted ? ted->te_just : TE_CNTR )
  {
    case TE_RIGHT:
      g.x += g.w - (off+strlen(ptr))*w;
      break;
    case TE_CNTR:
      g.x += (g.w - (off+strlen(ptr))*w+1)>>1;
      break;
  } ******/
  g.y += (g.h>>1) + (!j ? 3 : (char_h-2)>>1 /* 004: was (char_h>10 ? (char_h-4)>>1 : (char_h-2)>>1) */ );
  txt_app = form_app;
  /*c = od.color.l; 004 */
  if( od.atari3d != 0 && od.atari_move && od.state&SELECTED ||
      od.magic && (od.state&(X_DRAW3D|SELECTED)) == (X_DRAW3D|SELECTED) )
  {
    g.x++;
    g.y++;
    od.state &= ~SELECTED;
  }
  txt_app = 0L;
  if( type==G_TITLE )
  {
    _vsl_color(1);
    _vsl_type(1);
    _vswr_mode(3);
  }
  else
  {
    if( !od.atari3d )
    {
      if( is_ted ) c = ted->te_color;
      else if( type==G_BOXCHAR ) c = spec;
      else c = (od.color.l&0xF0F0) | (undraw ? 0 : 0x101);  /* 004: added | */
      od.color.l = c;	/* 004 */
      if( od.magic && od.state&X_SHADOWTEXT && (!od.color.b.textcol ||
          od.color.b.interiorcol && od.color.b.fillpattern) ) c = c ? 0 : 0x101;  /* 004 */
      if( od.state&SELECTED ) xor_col(&c);
    }
    else c = od.atari_col;	/* 004 */
    if( !undraw ) c >>= 8;
    line_init( c&0xf );
  }
  draw_line( g.x, g.y, g.x+w-1, g.y );
}

int find_alt( OBJECT *ob, int obj )
{
  char *ptr, c;
  int j, i, is_ted;
  long tree = (long)ob;

  ob += obj;
  if (HIDETREE & ob->ob_flags) return (FALSE);
  if( /*(fn_dir && (ob->ob_type&fn_dir)==0) ||*/ (ob->ob_state&(X_MAGMASK|X_KBD_EQUIV)) ==
      (X_MAGIC|X_KBD_EQUIV) || ((i=get_typex(form_app,(OBJECT2 *)ob)) == X_RADCHKUND ||
      i == X_HELP) && ((char)ob->ob_type==G_STRING || (char)ob->ob_type==G_BUTTON) ) return(TRUE);
  for( j=0; j<num_keys; j++ )
    if( alt_obj[j] == obj ) return(TRUE);
  if( num_keys < MAX_KEYS && (ptr=get_butstr( tree, obj, &is_ted, 1 )) != 0 )
    for( i=0; *ptr; i++, ptr++ )
      if( (c=*ptr&0xdf)>='A' && c<='Z' ||
          (c=*ptr)>='0' && c<='9' )
      {
        for( j=0; j<num_keys; j++ )
          if( c == alt[j] ) break;
        if( j==num_keys )
        {
          alt[j] = c;
          alt_off[j] = i;
          alt_obj[num_keys++] = obj;
          if( fn_last ) draw_alt( tree, obj, ptr, i, is_ted, 0 );
          break;
        }
      }
  return (TRUE);
}

char fn_alt;

int find_nalt( OBJECT *ob, int obj )
{
  char *ptr, c;
  int is_ted;
  long tree = (long)ob;

  if( fn_obj > 0 ) return(FALSE);
  ob = ob+obj;
  if (HIDETREE & ob->ob_flags) return (FALSE);
  if( fn_alt && ((ob->ob_state&(X_MAGMASK|X_KBD_EQUIV)) !=
      (X_MAGIC|X_KBD_EQUIV) && (get_typex(form_app,(OBJECT2 *)ob) != X_RADCHKUND ||
      (char)ob->ob_type!=G_STRING && (char)ob->ob_type!=G_BUTTON)) ) return(TRUE);
  if( (ptr=get_butstr( tree, obj, &is_ted, 1 )) == 0 ) return TRUE;  /* 004: lots of changes */
  if( fn_alt )
  {
    if( (ptr=strchr(ptr,'[')) != 0 )
    {
      if( (c = *(ptr+1))>='a' && c<='z' ) c&=0xdf;
      if( c==fn_last )
      {
        fn_obj = obj;
        return(FALSE);
      }
    }
  }
  else if( *ptr==fn_last && !*(ptr+1) )
  {
    fn_obj = obj;
    return(FALSE);
  }
  return(TRUE);
}

int scan_alts( OBJECT *tree, char ch, int is_alt )
{
  fn_last = ch;
  fn_obj = 0;
  fn_dir = SELECTABLE|TOUCHEXIT|EXIT;
  fn_alt = is_alt;	/* 004 */
  map_tree(tree, ROOT, NIL, find_nalt);
  return( fn_obj );
}

void alt_redraw( long tree, int obj, int undraw )
{
  int i, is_ted, ob;
  char *ptr;

  fn_dir = 0;
  for( i=0; i<num_keys; i++ )
    if( alt_obj[i]==obj )	/* 004: was char */
    {
      ob = abs(obj);	/* 004: was char */
      if( (ptr=get_butstr(tree,ob,&is_ted,1))!=0 ) draw_alt( tree,
          ob, ptr+alt_off[i], alt_off[i], is_ted, undraw );
      return;
    }
}

void form_redraw_all( long tree, int undraw )
{
  int i, is_ted;
  char *ptr;

  fn_dir = 0;
  for( i=0; i<num_keys; i++ )
    if( (ptr=get_butstr(tree,abs(alt_obj[i]),&is_ted,1))!=0 )	/* 004: was char */
      draw_alt( tree, abs(alt_obj[i]), ptr+alt_off[i], alt_off[i], is_ted, undraw );
}

void form_search( OBJECT *tree, int func( OBJECT *tree, int tmp ) )
{
  fn_dir = EXIT|TOUCHEXIT;
  map_tree( tree, ROOT, fn_obj = NIL, func );
}

int __form_keybd( long tree, int edit_obj, int kr, int *out_obj, int *okr )
{
  if( test_update( (void *)__form_keybd ) ) return 0;
  *okr = kr;
  fn_dir = 0;             /* Default tab direction is backward.   */
  switch (kr&0xFF00) {
    case HELP:
      *okr = 0;
      form_search( (OBJECT *)tree, find_help );
      goto test;
    case UNDO:
      *okr = 0;
      form_search( (OBJECT *)tree, find_undo );
      if( fn_obj == NIL ) form_search( (OBJECT *)tree, find_exit );
test: if( fn_obj != NIL )
      {
sel:    objc_sel(tree, fn_obj);
        *out_obj = fn_obj;
        return (FALSE);
      }
      break;
    case KCR:
    case CR:        /* Zap character.                       */
      if((char)kr != '\r') break;
      *okr = 0;
              /* Look for a DEFAULT object.           */
      map_tree((OBJECT *)tree, ROOT, fn_obj = NIL, find_def);
              /* If found, SELECT and force exit.     */
      if (fn_obj != NIL) goto sel;
                                    /* Falls through to     */
    case TAB:                       /* tab if no default    */
    case DOWN:
      fn_dir = 1;             /* Set fwd direction    */
    case UP:
      *okr = 0;               /* Zap character        */
      fn_last = edit_obj;
      map_tree((OBJECT *)tree, ROOT, fn_prev = fn_obj = NIL, find_tab);
      if (fn_obj == NIL)      /* try to wrap around   */
          map_tree((OBJECT *)tree, ROOT, NIL, find_tab);
      if (fn_obj != NIL) *out_obj = fn_obj;
      break;
    default:                        /* Pass other chars     */
      return (TRUE);
  }
  return (TRUE);
}

/************* Mouse button manager and subroutines ***************/
void do_radio( long tree, int obj, int wait )
{
  int pobj, sobj, state;

  pobj = find_parent( (OBJECT *)tree, obj );  /* Get the object's parent */

  for (sobj = OB_HEAD(pobj); sobj != pobj; sobj = OB_NEXT(sobj) )
  {                               /* Deselect all but...     */
    if (sobj != obj && (OB_FLAGS(sobj)&RBUTTON) )
      objc_dsel(tree, sobj);
  }
  objc_sel(tree, obj);                    /* the one being SELECTED  */
  if( wait ) while( g_mb&1 );
}

int form_button( OBJECT *tree, int obj, int clicks, int *next_obj )
{
  if( test_update( (void *)form_button ) ) return 0;
  return _form_button( tree, curapp, obj, clicks, next_obj ) != 0;	/* 004: != 0 */
}

int _form_button( OBJECT *tree, APP *ap, int obj, int clicks, int *next_obj )
{
  unsigned char flags, state, texit, sble, dsbld;
  int hibit, ret=TRUE;

  flags = u_object(tree,obj)->ob_flags;           /* Get flags and states   */
  state = u_object(tree,obj)->ob_state;
  texit = flags & TOUCHEXIT;
  sble = flags & SELECTABLE;
  dsbld = state & DISABLED;

  if( state&SELECTED && !sble && (flags&(EXIT|TOUCHEXIT))==EXIT && !dsbld )
  {	/* 004: special case: fool into touchexit */
    texit = 1;
    clicks = clicks!=0;
  }
  
  if ( !(flags&(TOUCHEXIT|EDITABLE|EXIT)) && (!sble || dsbld) ) /* This is not an      */
  {                                /* interesting object  */
    *next_obj = 0;
    return (TRUE);
  }

  if (texit && clicks == 2)               /* Preset special flag  */
          hibit = 0x8000;
  else
          hibit = 0x0;

  if (sble && !dsbld)                     /* Hot stuff!           */
  {
    ret = 2;  /* 004: return a unique value so X_WTFL_KEYS works right */
    if (flags & RBUTTON)            /* Process radio buttons*/
    {
      do_radio((long)tree, obj, !sble||!texit);    /* immediately!         */
                  /* selectable, touchexit, radio button is special */
    }
    else if (!_graf_watchbox((OBJECT *)tree, ap, obj, state^SELECTED, state ))
    {                       /* He gave up...  */
      *next_obj = 0;
      return ret;	/* 004 */
    }
  }

  if (texit || flags&EXIT && sble&&!dsbld&&!(state&SELECTED) ) /* checks old state */
          /* Exit conditions.             */
  {
    *next_obj = obj | hibit;
    return (FALSE);         /* Time to leave!               */
  }
  else if (!(flags&EDITABLE) )    /* Clear object unless tabbing  */
          *next_obj = 0;
  else *next_obj = obj;           /* EDITABLE */

  return ret;
}

void move_curs( long tree, int edit_obj, int *idx, int nidx )
{
  if( nidx != *idx )
  {
    objc_edit((OBJECT *)tree, edit_obj, 0, idx, ED_END );
    *idx = nidx;
    objc_edit((OBJECT *)tree, edit_obj, 0, idx, ED_END );
  }
}

int last_but;

int _multi( long tree )
{
  long l;
  int up, pos=0;

  br=1;
  kr=0;
  for(;;)
  {
    if( g_mb & 1 )
    {
      mx = g_mx;
      my = g_my;
      ks = *kbshift&0xf;
      br = 1;
      if( (m_obj = objc_find((OBJECT *)tree, ROOT, MAX_DEPTH, mx, my)) >= 0
          && !(last_but&1) && ((up=OB_FLAGS(m_obj))&TOUCHEXIT ||
          up&EXIT && OB_STATE(m_obj)&SELECTED) )
      {
        l = tic+dc_pause;
        reset_butq();
        up = 0;
        while( tic < l )
          if( !(g_mb&1) ) up=1;
          else
          {
            if( up ) br=2;
            up=0;
          }
      }
      mb = last_but = g_mb;
      return(MU_BUTTON);
    }
    else last_but = 0;
    if( has_key() && (l = getkey()) != 0 )
    {
      if( (ks = *kbshift&0xf) == 8 )
        if( (char)l >= '0' && (char)l <= '9' )
        {
          kr = kr*10 + (char)l - '0';
          if( ++pos==3 && kr>255 ) kr=0;
        }
      if( !pos ) kr = (int)(l>>8)|(char)l;
      if( !pos || pos==3 )
kbd:      return(MU_KEYBD);
    }
    else if( pos && (ks = *kbshift&0xf) != 8 ) goto kbd;
  }
}

int wind_dial( long tree )
{
  if( lastkey )
  {
    kr = lastkey;
    ks = lastsh;
    /*lastkey = 0;	 added for 004, but causes WTFL_KEYS to fail */
    return MU_KEYBD;
  }
  m_obj = objc_find((OBJECT *)tree, ROOT, MAX_DEPTH,
      mx=curapp->mouse_x, my=curapp->mouse_y);
  br = curapp->times;
  return MU_BUTTON;
}

void reblit( long addr, int save )
{
  int px[8];

  fdb2.fd_addr = (char *)addr;
  _vs_clip( 0, 0L );
  if( !save )
  {
    px[0] = px[1] = 0;
    px[2] = (px[6]=asc_x2) - (px[4]=asc.x);
    px[3] = (px[7]=asc_y2) - (px[5]=asc.y);
    vro_cpyfm( vdi_hand, 3, px, &fdb2, &fdb0 );
  }
  else
  {
    px[4] = px[5] = 0;
    px[6] = (px[2]=asc_x2) - (px[0]=asc.x);
    px[7] = (px[3]=asc_y2) - (px[1]=asc.y);
    vro_cpyfm( vdi_hand, 3, px, &fdb0, &fdb2 );
  }
}

int fblit( int flag )
{
  return mblit( flag, &asc );
}

void blit_drag( OBJECT *tree, int clicks )
{
  int x, y, nx, ny, gx, dif;
  long under;

  hide_mouse();
  under = pull_buf.l;
  if( !fblit(0xff00) ) return;	/* save dialog */
  if( g_mb&2 )
  {
    reblit( under, 0 );		/* blank */
    while( g_mb&1 );
    reblit( pull_buf.l, 0 );	/* draw dialog */
  }
  else if( clicks==2 )
  {
    reblit( under, 0 );		/* blank */
    _form_center( tree, &asc, 1 );
    asc_x2 = asc.x + asc.w - 1;
    asc_y2 = asc.y + asc.h - 1;
    reblit( under, 1 );		/* save new blank */
    while( g_mb&1 );
    reblit( pull_buf.l, 0 );	/* draw dialog */
  }
  else
  {
    x = g_mx;
    y = g_my;
    dif = tree[0].ob_x - asc.x;
    while( g_mb&1 )
    {
      ny = g_my-y;
      y = g_my;
      gx = g_mx;
      nx = gx-x;
      x = gx;
      if( asc.x+nx < 0 ) nx = -asc.x;
      else if( asc_x2+nx >= desktop->outer.w ) nx = desktop->outer.w-asc_x2-1;
      if( asc.y+ny < desktop->working.y ) ny = desktop->working.y-asc.y;
      else if( asc_y2+ny >= desktop->outer.y+desktop->outer.h ) ny =
          desktop->outer.y+desktop->outer.h-asc_y2-1;
      if( nx || ny )
      {
        reblit( under, 0 );		/* blank */
        tree[0].ob_x = (asc.x += nx) + dif;
        tree[0].ob_y = (asc.y += ny) + dif;
        asc_x2 += nx;
        asc_y2 += ny;
        reblit( under, 1 );		/* save new blank */
        reblit( pull_buf.l, 0 );	/* draw dialog */
      }
    }
  }
  fblit(-1);				/* just free */
  show_mouse(1);
}

void draw_ascii( int flag )
{
  static int ox, oy;
  static Rect old;

  hide_mouse();
  if( !flag )
  {
    ox = ascii_tbl[0].ob_x;
    oy = ascii_tbl[0].ob_y;
    old = asc;
    asc.w = ascii_tbl[0].ob_width;
    asc_x2 = (asc.x = ascii_tbl[0].ob_x = (desktop->outer.w-asc.w)>>1) +
        asc.w;
    asc_y2 = (asc.y = ascii_tbl[0].ob_y = char_h+3) +
        (asc.h=ascii_tbl[0].ob_height);
    fblit(0);
    _objc_draw( (OBJECT2 *)ascii_tbl, 0L, 0, 8, 0, 0, 0, 0 );
  }
  else
  {
    ascii_tbl[0].ob_x = ox;
    ascii_tbl[0].ob_y = oy;
    fblit(1);
    asc = old;
    asc_x2 = asc.x+asc.w-1;
    asc_y2 = asc.y+asc.h-1;
  }
  show_mouse(1);
}

int wind_get( int wi_ghandle, int wi_gfield, ... );

static Rect frect;
static char asc_on=0;

void form_init( OBJECT *tree )
{
  tree[0].ob_next = -1;
  if( next_obj<0 ) next_obj = 0;
  if( !(EDITABLE & OB_FLAGS(next_obj)) /*||
      ((oidx=OB_TYPE(next_obj)) != G_FTEXT &&
      oidx != G_FBOXTEXT)*/ )
  {
    map_tree(tree, ROOT, fn_obj = NIL, find_edit);
    next_obj = fn_obj == NIL ? 0 : fn_obj;
  }
  edit_obj = -1;
}

void form_reinit( int next, int edit, int idx, int curs )
{
  int i;

  next_obj = next;
  edit_idx = idx;
  edit_obj = edit;
  mouse_curs = curs;
  num_keys=0;
  form_app = is_alert ? 0L : curapp;
}

void edit_curs( OBJECT *tree, int type, int cont )
{
  if( !type )  /* possible new edit */
  {
    if( edit_obj != next_obj && next_obj != 0 )
    {
      objc_edit(tree, edit_obj=next_obj, 0, &edit_idx, ED_INIT);
      next_obj = 0;
    }
  }    /* show/erase current */
  else if( edit_obj && (!cont || next_obj != edit_obj && next_obj != 0) )
      objc_edit(tree, edit_obj, 0, &edit_idx, ED_END);
}

int udlr_equiv( OBJECT *o )
{
  int i, ret=0;
  static char wk[] = { XS_UPLINE, XS_DNLINE, XS_LFLINE, XS_RTLINE },  /* 004 */
      wch[] = { '', '', '', '' };

  for( i=0; i<4; i++ )
    if( is_key( &settings.wind_keys[wk[i]], ks, kr ) &&
        (ret = scan_alts(o,wch[i],0)) > 0 ) return ret;
  return ret;
}

unsigned char key_2ascii(void)
{
  unsigned int i;
  unsigned char ch;

  i = (unsigned)kr>>8;
  if( i>=0x78 && i<=0x81 ) i-=0x78-0x2;
  ch = Keytbl((void *)-1L, (void *)-1L, (void *)-1L)->unshift[i];
  if( ch>='a' && ch<='z' ) ch &= 0xdf;
  return ch;
}

void edit_key( long tree, int cont )
{
  int max;

  if( kr )
  {
    if( (max = _xobjc_edit((OBJECT *)tree, form_app, edit_obj, kr|((long)ks<<16), &edit_idx, ED_CHAR)) == 2 )
    {
      edit_obj--;
      edit_curs( (OBJECT *)tree, 1, 0 );  /* turn on */
    }
    else if( max==3 )
    {
      edit_obj++;
      edit_curs( (OBJECT *)tree, 1, 0 );  /* turn on */
    }
  }
  else if( next_obj && cont &&
      u_tedinfo((OBJECT *)tree,next_obj<edit_obj?next_obj:edit_obj)->te_tmplen==X_LONGEDIT )
  {
    edit_curs( (OBJECT *)tree, 1, 0 );  /* turn off */
    edit_obj = next_obj;
    next_obj = 0;
    if( edit_idx > (max=strlen(u_ptext((OBJECT *)tree,edit_obj))) ) edit_idx = max;
    edit_curs( (OBJECT *)tree, 1, 0 );  /* turn on */
  }
}

int scrp_txt( char *p, int del )
{
  if( x_scrp_get( p, del ) )
  {
    strcat( p, "SCRAP.TXT" );
    return 1;
  }
  return 0;
}

void edit_to_clip( long tree )
{
  char temp[120], *p;
  int h;

  if( scrp_txt( temp, 1 ) && (h = Fcreate(temp,0)) > 0 )
  {
    p = u_ptext((OBJECT *)tree,edit_obj);
    Fwrite( h, strlen(p), p );
    Fwrite( h, 2, (void *) "\r\n" );
    Fclose(h);
  }
}

void edit_from_clip( long tree )
{
  char temp[120];
  int h;

  if( scrp_txt( temp, 0 ) && (h = Fopen(temp,0)) > 0 )
  {
    kr = 0;
    ks = 0;
    for(;;)
    {
      if( Fread( h, 1L, (char *)&kr + 1 ) <= 0 ) break;
      if( (char)kr=='\r' || (char)kr=='\n' ) break;
      edit_key( tree, 1 );
    }
    Fclose(h);
  }
}

int form_event( long tree, int event( long tree ), int modal )
{
  int which;
  int cont=TRUE;
  int max, oidx;
  TEDINFO *ted;

  which = event(tree);
                                         /* handle button event  */
  if (which == MU_BUTTON)
  {
    if( asc_on )
    {
      asc_on=0;
      if( (oidx=objc_find( ascii_tbl, 0, 8, mx, my )) > 0 )
      {
        kr = set_asc_cur( mx, oidx );
        ascii_tbl[6].ob_state |= SELECTED;
        _objc_draw( (OBJECT2 *)ascii_tbl, 0L, 6, 0, 0, 0, 0, 0 );
        while(g_mb&1);
        ascii_tbl[6].ob_state &= ~SELECTED;
        ascii_tbl[6].ob_flags |= HIDETREE;
        draw_ascii(1);
        goto do_ascii;
      }
      draw_ascii(1);
      while(g_mb&1);
    }
    else
    {                               /* Which object hit?    */
      next_obj = m_obj;
do_button:
      if (next_obj == NIL)
      {
        next_obj = 0;
        if( !modal ) return TRUE;
        ring_bell();
      }
      else                            /* Process a click      */
      {
        cont = _x_form_mouse( (OBJECT *)tree, form_app, mx, my, br, &edit_obj,
            &next_obj, &edit_idx );
        if( !cont && (oidx=next_obj&0x7fff)!=0x7fff &&
            get_typex(form_app,((OBJECT2 *)tree)+oidx) == X_MOVER &&
            ((OBJECT2 *)u_object((OBJECT *)tree,oidx))->ob_type == G_BUTTON )
        {
          objc_edit( (OBJECT *)tree, edit_obj, 0, &edit_idx, ED_END );
          blit_drag( (OBJECT *)tree, br );
          objc_edit( (OBJECT *)tree, edit_obj, 0, &edit_idx, ED_END );
          next_obj=0;
          cont=1;
        }
      }
    }
  }
  else if (which == MU_KEYBD)
    if( modal && (kr==0x4700 || kr==0x4737) )  /* was Insert, now Clr-Home with or without Shift */
    {
      if( edit_obj && !asc_on )
      {
        draw_ascii(0);
        asc_on = 1;
      }
    }
    else
    {                               /* Control char filter  */
      if( asc_on )
      {
        asc_on=0;
        draw_ascii(1);
      }
/* 004:      if( ks==8 ) */
        if( ks==8 && kr>>8 == 0xf ) /* Alt-Tab */
        {
          map_tree((OBJECT *)tree, ROOT, fn_obj=NIL, find_def);
          if( fn_obj != NIL )
          {
            max = fn_obj;
            map_tree( (OBJECT *)tree, max, fn_obj=NIL, find_ndef );
            if( fn_obj==NIL ) map_tree( (OBJECT *)tree, ROOT, max, find_ndef );
            if( fn_obj != NIL )
            {
              objc_xywh(tree, max, &frect);
              adjust_rect( u_object((OBJECT *)tree,max), &frect, 1 );
              ((OBJECT2 *)u_object((OBJECT *)tree,max))->ob_flags &= ~DEFAULT;
              ((OBJECT2 *)u_object((OBJECT *)tree,fn_obj))->ob_flags |= DEFAULT;
              _objc_draw( (OBJECT2 *)tree, form_app, ROOT, MAX_DEPTH, frect.x,
                  frect.y, frect.w, frect.h );
              hide_mouse();
              alt_redraw( tree, max, 0 );
              _objc_draw( (OBJECT2 *)tree, form_app, fn_obj, MAX_DEPTH, 0, 0,
                  0, 0 );
              alt_redraw( tree, fn_obj, 0 );
              show_mouse(1);
            }
          }
          goto end;
        }
        else if( ks==8 || edit_obj<=0 && !(ks&0xC) &&
            (modal && settings.flags.s.no_alt_modal_equiv ||
            !modal && settings.flags.s.no_alt_modeless_eq) )	/* 004 */
        {
          max = key_2ascii();	/* 004: function now */
          for( oidx=0; oidx<num_keys; oidx++ )
            if( alt[oidx]==max )
            {
              next_obj = alt_obj[oidx];
              which=0;
              br=1;
              goto do_button;
            }
          if( (next_obj = scan_alts((OBJECT *)tree,max,1)) > 0 ||
              (next_obj = udlr_equiv( (OBJECT *)tree )) > 0 )	/* 004 */
          {
            which = 0;
            br = 1;
            goto do_button;
          }
        }
        else if( (next_obj = udlr_equiv( (OBJECT *)tree )) > 0 )	/* 004 */
        {
          which = 0;
          br = 1;
          goto do_button;
        }
        else if( edit_obj>0 && (max=ks&0xb)&3 && max&8 )	/* 004: clipboard ops */
          if( (max = key_2ascii()) == 'X' )
          {
            edit_to_clip(tree);
            ks = 0;
            kr = '\033';	/* clear line, below */
          }
          else if( max == 'C' )
          {
            edit_to_clip(tree);
            goto end;
          }
          else if( max == 'V' )
          {
            edit_from_clip(tree);
            goto end;
          }
      cont = __form_keybd(tree, edit_obj, kr, &next_obj, &kr);
      if( cont && edit_obj>0 ) cont = 2;	/* 004: prevent passing key to app */
do_ascii:
      if( edit_obj>0 )	/* 004: was !=0 */
          edit_key( tree, cont );
      else if( is_alert && !ks )
        if( (max=kr&0xff) >= '1' && max <= '3' )
        {
          next_obj = 8+max-'1';
alertkey: if( !is_hid((OBJECT *)tree,next_obj) )
          {
            which=0;
            br=1;
            goto do_button;
          }
          else next_obj=0;
        }
        else if( (max=kr>>8) >= 0x3b && max <= 0x3d )
        {
          next_obj = 8+max-0x3b;
          goto alertkey;
        }
    }
end:
  mouse_curs = 1;
  return( cont );
}

int _form_do( OBJECT *tr, int next )
{
  long tree = (long)tr;
  int cont;
                                          /* Init. editing        */
  if( test_update( (void *)_form_do ) ) return 0;
  _wind_update( BEG_MCTRL );
  next_obj = next;
  form_init( tr );
  form_reinit( next_obj, 0, 0, 1 );

  /* find alt key equivs */
  hide_mouse();
  if( (((OBJECT2 *)tree)->ob_statex&((X_MAGMASK|X_KBD_EQUIV)>>8)) !=
      ((X_MAGIC|X_KBD_EQUIV)>>8) && do_alts )
  {
    fn_last = 1;	/* draw */
    fn_dir = EXIT;
    map_tree((OBJECT *)tree, ROOT, NIL, find_alt);
    fn_dir = SELECTABLE|RBUTTON|TOUCHEXIT;
    map_tree((OBJECT *)tree, ROOT, NIL, find_alt);
  }
  show_mouse(1);
                                          /* Main event loop      */
  do
  {
    edit_curs( (OBJECT *)tree, 0, 0 );
    cont = form_event( tree, _multi, 1 );
    edit_curs( (OBJECT *)tree, 1, cont );
  }
  while( cont );

  hide_mouse();
  if( !is_alert && undo_alts ) form_redraw_all( tree, 1 );
  show_mouse(1);
  reset_butq();
  _wind_update( END_MCTRL );
  num_keys = 0;
  return(next_obj);
}

int spf_alert( char *s, int num )
{
  char buf[200];

  spf( buf, s, num );
  return( _form_alert( 1, buf ) );
}

int sspf_alert( char *tmpl, char *s )
{
  char buf[200];

  spf( buf, tmpl, s );
  return( _form_alert( 1, buf ) );
}

int x_form_error( char *fmt, int num )
{
  ERRSTRUC *err;
  char *msg, temp[50];
  int i;

  if( test_update( (void *)x_form_error ) ) return 0;
  if( num<0 )
  {
    for( err=dflt_errors; err->num; err++ )
      if( err->num==num ) return sspf_alert( fmt, err->string );
    if( num >= -12 ) return sspf_alert( fmt, dflt_errors[0].string );
  }
  if( (unsigned)num>=sizeof(fe_xref) || (i=fe_xref[num])<0 )
  {
    spf( temp, fe_dflt, num );
    return sspf_alert( fmt, temp );
  }
  return sspf_alert( fmt, fe_str[i] );
}

int form_error( int num )
{
  return x_form_error( fe_tmpl, num );
}

int crit_error( int err, int drive )
{
  int num = -1-err;
  
  if( num<0 || num>=sizeof(ce_xref) )
  {
    _form_alert( 1, ce_bad );
    return 2;
  }
  if( (num=ce_xref[num])!=4 && (drive<0 || drive>'Z'-'A') ) /* 005: check letter */
  {
    _form_alert( 1, ce_invalid );
    return 2;
  }
  return( spf_alert( crit_str[num], drive+'A' ) );
}

int _form_center( OBJECT *tree, Rect *r, int frame )
{
  objc_xywh( (long)tree, ROOT, r );
  tree[0].ob_x = r->x = desktop->working.x + (desktop->working.w-r->w)/2;
  tree[0].ob_y = r->y = desktop->working.y + (desktop->working.h-r->h)/2;
  adjust_rect( tree, r, frame );
  return(1);
}

int form_center( OBJECT *tree, int *x, int *y, int *w, int *h )
{
  Rect r;

  _form_center( tree, &r, 1 );
  *x = r.x;
  *y = r.y;
  *w = r.w;
  *h = r.h;
  return(1);
}

int _form_dial( int flag, Rect *small, Rect *big )
{
  if( (flag==FMD_START || flag==FMD_FINISH) &&
      test_update( (void *)_form_dial ) ) return 0;
  switch( flag )
  {
    case FMD_GROW:
      growbox( small, big );
      break;
    case FMD_START:	/* 004 */
      if( !has_fmd_update && !update.i )
      {
        has_fmd_update = curapp;
        _wind_update( BEG_UPDATE );
      }
      break;
    case FMD_SHRINK:
      shrinkbox( big, small );
      break;
    case FMD_FINISH:
      redraw_all( big );
      if( has_fmd_update==curapp )	/* 004 */
      {
        has_fmd_update = 0L;
        _wind_update( END_UPDATE );
      }
      break;
    case X_FMD_START:
      asc = *big;
      asc_x2 = big->x+big->w-1;
      asc_y2 = big->y+big->h-1;
      return mblit( 0x100, big );
    case X_FMD_FINISH:
      mblit( 0x101, big );
      break;
  }
  return(1);
}

int _form_mouse( OBJECT *tree, int mx, int my, int clicks, int *out )
{
  return _x_form_mouse( tree, curapp, mx, my, clicks, out, out+1, out+2 );
}

int _x_form_mouse( OBJECT *tree, APP *ap, int mx, int my, int clicks, int *edit_obj,
    int *next_obj, int *ed_idx )
{
  int cont, max, oidx, w;
  char *ptr, *ptr2, *ptr3;
  Rect frect;
  TEDINFO *ted;

  my++;
  cont = _form_button( tree, ap, *next_obj, clicks, next_obj );
  if( cont && *next_obj && mouse_curs && u_object(tree,*next_obj)->ob_flags&EDITABLE )
  {
    ptr = (char *)(ted=(TEDINFO *)get_spec(u_object(tree,*next_obj)))->te_ptmplt;
    if( (ptr2 = strchr( ptr, '_' )) != 0 )
    {
      oidx = *ed_idx;
      if( ted->te_font==GDOS_PROP || ted->te_font==GDOS_MONO || ted->te_font==GDOS_BITM )
          *ed_idx = strlen(ted->te_ptext);
      else
      {
        w = ted->te_font==SMALL ? 6 : char_w;
        objc_xywh( (long)tree, *next_obj, &frect );
        switch( ted->te_just )
        {
          case TE_RIGHT:
            max = frect.w - strlen(ptr)*w;
            break;
          case TE_CNTR:
            max = (frect.w - strlen(ptr)*w) >> 1;
            break;
          case TE_LEFT:
          default:
            max=0;
        }
        mx -= frect.x + max + w*(ptr2-ptr);
        *ed_idx = 0;
        ptr3 = (char *)ted->te_ptext;
        while( *ptr3 && mx>=w-1 )
        {
          if( *ptr2++=='_' )
          {
            ++*ed_idx;
            ptr3++;
          }
          mx -= w;
        }
      }
      if( *edit_obj != *next_obj || oidx != *ed_idx )
      {
        objc_edit(tree, *edit_obj, 0, &oidx, ED_END);
        objc_edit(tree, *edit_obj=*next_obj, 0, ed_idx, ED_END);
      }
    }
  }
  return(cont);
}

