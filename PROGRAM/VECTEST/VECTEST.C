#include "new_aes.h"
#include "xwind.h"
#include "tos.h"

/* Geneva vector test. Copyright 1994, Gribnif Software

   This program installs three routines into vectors that Geneva uses.
   An event is generated when one of these conditions is met:

   1. If a key is pressed which has an ASCII value (e.g.: not the
      arrow, Help, etc. keys).

   2. Whenever the current application is switched.

   3. Whenever the right Shift key is held

*/

#define CJar_cookie	0x434A6172L	/* "CJar" */
#define CJar_xbios	0x434A		/* "CJ" */
#define	CJar_OK		0x6172		/* "ar" */
#define CJar(mode,cookie,value)		(int)xbios(CJar_xbios,mode,cookie,value)

int apid,		/* ID of this process */
    new_apid;		/* ID of X_WM_VECSW process (set by my_appvec) */

long keypress;		/* key pressed by user (set by my_keyvec) */

char *new_app,		/* name of X_WM_VECSW process (set by my_appvec) */
     shift;		/* semaphore to prevent too many events */

/* vector routines in VECTESTS.S */
int my_keyvec( long *key );
int my_appvec( char *process_name, int apid );
int my_genvec(void);

/* old vector values in VECTESTS.S */
extern int (*old_keyvec)( long *key );
extern int (*old_appvec)( char *process_name, int apid );
extern int (*old_genvec)(void);

void install_vex( G_VECTORS *vex )
{
  old_keyvec = vex->keypress;
  vex->keypress = my_keyvec;
  old_appvec = vex->app_switch;
  vex->app_switch = my_appvec;
  old_genvec = vex->gen_event;
  vex->gen_event = my_genvec;
}

void remove_one( void *value, long **start )
{
  /* search along the XBRA chain */
  while( *(*start-3) == 0x58425241L/*"XBRA"*/ )
  {
    if( (long)*start == (long)value )
    {
      *(long *)start = *(*start-1);	/* found it! remove from list */
      return;
    }
    start = (long **)(*start-1);	/* continue along chain */
  }
}

void remove_vex( G_VECTORS *vex )
{
  remove_one( my_keyvec, (long **)&vex->keypress );
  remove_one( my_appvec, (long **)&vex->app_switch );
  remove_one( my_genvec, (long **)&vex->gen_event );
}

int main(void)
{
  G_COOKIE *cook;
  int buf[8];
  char temp[200], *old_app=0L;

  apid = appl_init();
  /* check to make sure Geneva is active */
  if( CJar( 0, GENEVA_COOKIE, &cook ) != CJar_OK || !cook )
      form_alert( 1, "[1][|Geneva is not present][Ok]" );
  shel_write( 9, 1, 0, 0L, 0L );
  form_alert( 1, "[0][Geneva Vector test.|Terminate this program|\
from the Task Manager|to get rid of it.][OK]" );
  install_vex( cook->vectors );
  for(;;)
  {
    evnt_mesag(buf);
    switch(buf[0])
    {
      case AP_TERM:
        remove_vex( cook->vectors );
        return 0;
      case X_WM_VECKEY:
        x_sprintf( temp, "[0][VECTEST:|X_WM_VECKEY event|\
Key %c pressed][OK]", (char)keypress );
        form_alert( 1, temp );
        break;
      case X_WM_VECSW:
        if( new_app != old_app )	/* did it really change? */
        {
          x_sprintf( temp, "[0][VECTEST:|X_WM_VECSW event|\
Switched to %s|(apid %d)][OK]", new_app, new_apid );
          form_alert( 1, temp );
          old_app = new_app;
        }
        break;
      case X_WM_VECEVNT:
        form_alert( 1, "[0][VECTEST:|X_WM_VECEVNT event|\
You are holding the|right Shift key][OK]" );
        shift = 0;	/* clear event semaphore */
        break;
    }
  }
}
