#ifndef WINDOWS

int xwcolors[] = {
  0x1100, 0x1141, 0x11E1, 0x1141, 0x1141, 0x1141,
  0x1100, 0x1100, 0x1100,		    /* info */
  0,					            /* tool */
  0x1100, 0x1100, 0x1100,		    /* menu */
  0x1141, 0x1151, 0x1141, 0x1141,	/* vslider */
  0x1100,				            /* split */
  0x1141, 0x1151, 0x1141, 0x1141,	/* vslider */
  0x1141, 0x1151, 0x1141, 0x1141,	/* hslider */
  0x1100,				            /* split */
  0x1141, 0x1151, 0x1141, 0x1141,	/* hslider */
  0x1141 };
#define _wcolors xwcolors
#define u_object(o,n) (&o[n])
#define COLOR_3D o
int get_msey(void)
{
  int dum, y;
  graf_mkstate( &dum, &y, &dum, &dum );
  return(y);
}
int hide_if( OBJECT *tree, int truth, int idx )
{
  unsigned int *i;

  i = &u_object(tree,idx)->ob_flags;
  if( !truth ) *i |= HIDETREE;
  else *i &= ~HIDETREE;
  return truth;
}
#define change_objc(a,b,c,d,e,f) objc_change(a,c,0,Xrect(*d),e,f)
#define lalloc(x,y) Malloc(x)
#define lfree(x) Mfree(x)
char pman_big;

APP ap[20], *app0=&ap[0];
OBJECT *pman;
Window t, *top_wind=&t;
APP *has_menu, *has_mouse, *has_update;
char pman_big, has_mint;
int char_w=8;

#include "procman.c";

int main(void)
{
  int i, f;

  if( !rsrc_load( "windows.rsc" ) ) return 1;
  rsrc_gaddr( 0, PROCMAN, &pman );
  for( i=0, f=0; i<20; i++ )
    if( appl_search( f, ap[i].dflt_acc+2, &ap[i].ap_type, &ap[i].id ) )
    {
      if( i>0 ) ap[i-1].next = &ap[i];
      ap[i].mint_id = -1;
      ap[i].type = X_MU_DIALOG | MU_MESAG;
      ap[i].asleep = (x_appl_sleep( ap[i].id, -1 )&1)<<1;
      if( ap[i].id==1 ) ap[i].mint_id = 2;
      f = 1;
    }
  graf_mouse( M_ON, 0L );
  wind_update(BEG_UPDATE);
  procman();
  wind_update(END_UPDATE);
  return 0;
}
#endif
