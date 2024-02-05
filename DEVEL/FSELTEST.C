#ifndef WINDOWS

char fvalid[] = "_!@#$%^&()+-=~`;\'\",<>|[]()";
char fsel_1col, sort_type;
int dwcolors[] = {
    0x1100, 0, 0x1141, 0x11E1, 0x1141, 0x1100, 0, 0, 0x1141, 0, 0x1141, 0x1141,
    0x1151, 0x1141, 0, 0x1141, 0x1141, 0x1151, 0x1141 };
OBJECT *fsel, *tool;
Rect fs_rect;
#define lalloc(x,y) Malloc(x)
#define lfree(x) Mfree(x)
#define char_h 16
#define char_w 8
#define idt_fmt 0
#define spf x_sprintf
#define _wcolors dwcolors
#define ring_bell() Bconout(2,7)
int hide_if( OBJECT *tree, int truth, int idx )
{
  if( !truth ) tree[idx].ob_flags |= HIDETREE;
  else tree[idx].ob_flags &= ~HIDETREE;
  return truth;
}
char *pathend( char *path )
{
  char *p;
  if( (p=strrchr(path,'\\')) != 0 ) return(p+1);
  return(path);
}
void map_tree( OBJECT *tree, int ithis, int last,
               int func( OBJECT *tree, int tmp ) )
{
  int tmp1;

  tmp1 = 0;
  while (ithis != last && ithis != -1)
    if (tree[ithis].ob_tail != tmp1)
    {
      tmp1 = ithis;
      ithis = -1;
      if( (*func)( tree, tmp1 ) ) ithis = tree[tmp1].ob_head;
      if (ithis == -1) ithis = tree[tmp1].ob_next;
    }
    else
    {
      tmp1 = ithis;
      ithis = tree[tmp1].ob_next;
    }
}
int get_but(void)
{
  int dum, b;
  graf_mkstate( &dum, &dum, &b, &dum );
  return(b&1);
}
int get_msey(void)
{
  int dum, y;
  graf_mkstate( &dum, &y, &dum, &dum );
  return(y);
}
int kr, ks, br, mx, my, mb, m_obj;
int _multi( long tree )
{
  int r, buf[16];

  r = evnt_multi( MU_KEYBD|MU_BUTTON, 2, 1, 1,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,
                  buf, 0, 0, &mx, &my, &mb, &ks, &kr, &br );
  m_obj = objc_find((OBJECT *)tree, ROOT, MAX_DEPTH, mx, my);
  return(r);
}
long get_spec( OBJECT *tree )
{
  char *ptr;

  ptr = tree->ob_spec.free_string;
  if( tree->ob_flags & INDIRECT ) ptr = *(char **)ptr;
  return( (long)ptr );
}
void objc_xywh(long tree, int obj, Rect *p)
{
  objc_offset((OBJECT *)tree, obj, &p->x, &p->y);
  *(long *)&(p->w) = *(long *)&(((OBJECT *)tree)[obj].ob_width);
}
int x_form_mouse( OBJECT *tree, int mx, int my, int clicks, int *edit_obj,
                  int *next_obj, int *ed_idx )
{
  int cont, max, oidx;
  char *ptr, *ptr2, *ptr3;
  Rect frect;
  TEDINFO *ted;

  my++;
  cont = form_button( tree, *next_obj, clicks, next_obj );
  if( cont && *next_obj )
  {
    ptr = (char *)(ted=(TEDINFO *)get_spec(&tree[*next_obj]))->te_ptmplt;
    if( (ptr2 = strchr( ptr, '_' )) != 0 )
    {
      objc_xywh( (long)tree, *next_obj, &frect );
      switch( ted->te_just )
      {
        case TE_RIGHT:
          max = frect.w - strlen(ptr)*8;
          break;
        case TE_CNTR:
          max = (frect.w - strlen(ptr)*8) >> 1;
          break;
        case TE_LEFT:
        default:
          max=0;
      }
      mx -= frect.x + max + 8*(ptr2-ptr);
      oidx = *ed_idx;
      *ed_idx = 0;
      ptr3 = ted->te_ptext;
      while( *ptr3 && mx>=7 )
      {
        if( *ptr2++=='_' )
        {
          ++*ed_idx;
          ptr3++;
        }
        mx -= 8;
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

void adjust_rect( OBJECT *obj, Rect *r, int frame )
{
  int i;

  i = frame ? (char)((obj->ob_type==G_BUTTON ? but_spec(obj) :
                      get_spec(obj))>>16) : 0;
  if( i>-3 )
    if( obj->ob_state&OUTLINED ) i = -3;
    else if( obj->ob_state&SHADOWED )
    {
      r->w += 3;
      r->h += 3;
      return;
    }
  if( i<0 )
  {
    r->x += i;
    r->y += i;
    r->w -= (i<<1);
    r->h -= (i<<1);
  }
}

long but_spec( OBJECT *o )
{
  if( (o->ob_flags&(DEFAULT|EXIT) ) == (DEFAULT|EXIT) )
    return( 0x00FD1170L );
  if( o->ob_flags&(DEFAULT|EXIT) ) return( 0x00FE1180L );
  return( 0x00FF1170L );
}

void part_draw( OBJECT *tree, int root, int first, int last )
{
  Rect r;

  objc_offset( tree, first, &r.x, &r.y );
  *(long *)&r.w = *(long *)&tree[first].ob_width;
  r.h += tree[last].ob_y - tree[first].ob_y;
  adjust_rect( tree+first, &r, 1 );
  objc_draw( tree, root, 8, Xrect(r) );
}
#define change_objc(a,b,c,d,e,f) objc_change(a,c,0,Xrect(*d),e,f)

#include "fsel.c"

main()
{
  char path[TOPLEN]="", file[13*5+1]="", *ptr; /*FILENAME.EXT";*/
  int but, i;

  map_tree( fsel=fselind[FSEL], 0, -1, rsrc_obfix );
  map_tree( fselind[FFOLD], 0, -1, rsrc_obfix );
  map_tree( fselind[FFIND], 0, -1, rsrc_obfix );
  map_tree( fselind[FINFO], 0, -1, rsrc_obfix );
  map_tree( fselind[PATHLIST], 0, -1, rsrc_obfix );
  map_tree( fselind[EXTLIST], 0, -1, rsrc_obfix );
  map_tree( fselind[FSHELPTXT], 0, -1, rsrc_obfix );
  map_tree( tool=fselind[OPLIST], 0, -1, rsrc_obfix );
  map_tree( fselind[SORTLIST], 0, -1, rsrc_obfix );
  map_tree( fselind[FCOPY], 0, -1, rsrc_obfix );
  for( i=PATH1; i<PATH1+10; i++ )
    strcpy( u_obspec(fselind[PATHLIST],i)->free_string, "\\" );
  if( char_h==16 )
  {
    fsel[FSFBIG].ob_height += 7;
    fsel[FSOBIG].ob_height += 7;
    fsel[FSFBIG2].ob_height += 7;
  }
  strcpy( (fselind[FFIND]+FFINAME)->ob_spec.tedinfo->te_ptext, "Z*.*" );
  i = wind_create( 0, 0, 0, 640, 400 );
  wind_open( i, 0, 0, 640, 400 );
  form_center( fsel, &fs_rect.x, &fs_rect.y, &fs_rect.w, &fs_rect.h );
  strcpy( path, "H:\\*.*" );
  strcpy( file, "HOUR.DAT" );
  if( __x_fsel_input( path, sizeof(path), file, 5, &but, "Selector" ) && but )
    for( ptr=file; *ptr; ptr+=strlen(ptr)+1 )
    {
      Cconws(ptr);
      Cconws( "\r\n" );
    }
  return(0);
}
#endif
