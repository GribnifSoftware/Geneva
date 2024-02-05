#include "stdio.h"
#include "tos.h"
#include "aes.h"
#include "xwind.h"

void load_mouse( char *name, int index )
{
  int hand, is_mkm;
  ANI_MOUSE mouse;
  
  if( (hand = Fopen( name, 0 )) > 0 )
  {
    is_mkm = Fseek(0L,hand,2) != 74L;
    Fseek(0L,hand,0);
    if( is_mkm )
    {
      Fread( hand, 32*sizeof(MFORM), &mouse.form );
      Fread( hand, 2*sizeof(int), &mouse.frames );
    }
    else
    {
      Fread( hand, sizeof(MFORM), &mouse.form[0] );
      mouse.frames = 1;
      mouse.delay = 0;
    }
    Fclose(hand);
    graf_mouse( X_SET_SHAPE+index, (MFORM *)&mouse );
  }
}

int main(void)
{
  FILE *f;
  char fname[80], buf[80];
  int num;
  
  appl_init();
  if( (f = fopen("setmouse.dat","r")) != NULL )
  {
    graf_mouse( M_OFF, 0L );
    while( fgets( buf, 80, f ) != NULL )
    {
      if( sscanf( buf, "%d %s\n", &num, fname ) == 2 )
          load_mouse( fname, num );
    }
    graf_mouse( ARROW, 0L );
    graf_mouse( M_ON, 0L );
  }
  else form_alert( 1, "[1][SETMOUSE.DAT|not found][OK]" );
  return 0;
}
