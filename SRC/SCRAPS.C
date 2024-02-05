#include "aes.h"
#include "string.h"
#include "win_var.h"
#include "win_inc.h"

int scrp_read( char *path )
{
  strcpy( path, scrap_dir );
  if( *path ) return 1;
  return 0;
}

int scrp_write( char *path )
{
  strncpy( scrap_dir, path, sizeof(scrap_dir)-1 );
  return(1);
}

int x_scrp_get( char *out, int do_delete )
{
  long map;
  int ret;
  char ok=0, *e;
  DTA dta, *old;
  BASPAG *proc;

  proc = shel_context(0L);
  old = Fgetdta();
  Fsetdta(&dta);
  scrp_read(out);
  map = Drvmap();
  if( *out && *(out+1)==':' && (map & (*out&=0x5f)-'A')!=0 )
    if( (e=pathend(out))<=out+3 && !*e )	/* 004: fixed a bit */
    {
      ok=1;
      if( e<out+3 ) strcat(out,"\\");
    }
    else
    {
      if( !*e ) *(e-1) = 0;
      if( !Fsfirst(out,FA_SUBDIR) ) ok=1;
      strcat(out,"\\");
    }
  if( !ok )
  {
    out[0] = (map&(1<<2)) ? 'C' : 'A';
    strcpy( out+1, ":\\CLIPBRD" );
    if( Fsfirst(out,0x37) && Dcreate( out ) )
    {
      Fsetdta(old);
      shel_context(proc);
      return 0;
    }
    strcat( out, "\\" );
  }
  scrp_write(out);
  if( do_delete )
  {
    strcat( out, "SCRAP.*" );
    ret = Fsfirst( out, 0x23 );
    while( !ret )
    {
      strcpy( pathend(out), dta.d_fname );
      Fdelete( out );
      ret = Fsnext();
    }
  }
  *pathend(out) = 0;
  Fsetdta(old);
  shel_context(proc);
  return 1;
}
