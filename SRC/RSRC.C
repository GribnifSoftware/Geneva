#include "tos.h"
#include "aes.h"
#include "win_var.h"
#include "win_inc.h"
#include "string.h"
#include "xwind.h"
#include "debugger.h"

long *cicon_ptr, num_cic;

void fixx( int *i )
{
  *i = ((*i&0xFF)*char_w) + (*i>>8);
}

void fixy( int *i )
{
  *i = ((*i&0xFF)*char_h) + (*i>>8);
}

int obfix( OBJECT *tree, int ind )
{
  int *i = &u_object(tree,ind)->ob_x;

  fixx( i++ );
  fixy( i++ );
  fixx( i++ );
  fixy( i );
  return(1);
}

void map_tree( OBJECT *tree, int current, int last, int func( OBJECT *tree, int tmp ) )
{
  int tmp1;

  tmp1 = 0;
  while (current != last && current != -1)
    if (tree[current].ob_tail != tmp1)
    {
      tmp1 = current;
      current = -1;
      if( (*func)( tree, tmp1 ) ) current = tree[tmp1].ob_head;
      if (current == -1) current = tree[tmp1].ob_next;
    }
    else
    {
      tmp1 = current;
      current = tree[tmp1].ob_next;
    }
}

static long add;

void rsc_incr( long *loc )
{
  *loc += add;
}

int long_rsc( RSHDR *rsc )
{
  return ((RSHDR2 *)rsc)->rsh_extvrsn == X_LONGRSC;
}

long *get_cicons( RSHDR *rsc )
{
  char is_new;

  is_new = long_rsc(rsc);
  return (long *)(((long)rsc + (is_new ? ((RSHDR2 *)rsc)->rsh_rssize :
      rsc->rsh_rssize) + sizeof(long) + 1L)&-2L);
}

void fix_rsc( RSHDR *rshdr, char *rsc )
{
  unsigned i, j, siz;
  char *ptr, is_new;
  RSHDR2 *rshdr2;

  if( (is_new = long_rsc(rshdr)) != 0 ) siz = sizeof(RSHDR2);
  else siz = sizeof(RSHDR);
  rshdr2 = ((RSHDR2 *)rshdr);
  for( ptr=(is_new?rshdr2->rsh_object:rshdr->rsh_object)+rsc-siz,
      i=(is_new?rshdr2->rsh_nobs:rshdr->rsh_nobs)+1; --i>0; ptr+=sizeof(OBJECT) )
  {
    if( (j=((OBJECT *)ptr)->ob_type&0xff) == G_CICON ) ((OBJECT *)ptr)->ob_spec.index =
        num_cic && ((OBJECT *)ptr)->ob_spec.index<num_cic ?
        *(cicon_ptr+((OBJECT *)ptr)->ob_spec.index) : 0L;
    else if( j != G_BOXCHAR && j != G_IBOX && j != G_BOX ) rsc_incr(
        (long *)&((OBJECT *)ptr)->ob_spec.free_string );
    obfix( (OBJECT *)ptr, 0 );
  }
  for( ptr=(is_new?rshdr2->rsh_tedinfo:rshdr->rsh_tedinfo)+rsc-siz,
      i=(is_new?rshdr2->rsh_nted:rshdr->rsh_nted)+1; --i>0; ptr+=sizeof(TEDINFO) )
  {
    rsc_incr( (long *)&((TEDINFO *)ptr)->te_ptext );
    rsc_incr( (long *)&((TEDINFO *)ptr)->te_ptmplt );
    rsc_incr( (long *)&((TEDINFO *)ptr)->te_pvalid );
  }
  for( ptr=(is_new?rshdr2->rsh_iconblk:rshdr->rsh_iconblk)+rsc-siz,
      i=(is_new?rshdr2->rsh_nib:rshdr->rsh_nib)+1; --i>0; ptr+=sizeof(ICONBLK) )
  {
    rsc_incr( (long *)&((ICONBLK *)ptr)->ib_pmask );
    rsc_incr( (long *)&((ICONBLK *)ptr)->ib_pdata );
    rsc_incr( (long *)&((ICONBLK *)ptr)->ib_ptext );
  }
  for( ptr=(is_new?rshdr2->rsh_bitblk:rshdr->rsh_bitblk)+rsc-siz,
      i=(is_new?rshdr2->rsh_nbb:rshdr->rsh_nbb)+1; --i>0; ptr+=sizeof(BITBLK) )
    rsc_incr( (long *)&((BITBLK *)ptr)->bi_pdata );
  for( ptr=(is_new?rshdr2->rsh_frstr:rshdr->rsh_frstr)+rsc-siz,
      i=(is_new?rshdr2->rsh_nstring:rshdr->rsh_nstring)+1; --i>0; ptr+=sizeof(long) )
    rsc_incr( (long *)ptr );
  for( ptr=(is_new?rshdr2->rsh_frimg:rshdr->rsh_frimg)+rsc-siz,
      i=(is_new?rshdr2->rsh_nimages:rshdr->rsh_nimages)+1; --i>0; ptr+=sizeof(long) )
    rsc_incr( (long *)ptr );
  for( ptr=(is_new?rshdr2->rsh_trindex:rshdr->rsh_trindex)+rsc-siz,
      i=(is_new?rshdr2->rsh_ntree:rshdr->rsh_ntree)+1; --i>0; ptr+=sizeof(long) )
    rsc_incr( (long *)ptr );
}

int _rsrc_free( long *out )
{
  if( !*(out+1) ) return(0);
  lfree((void *)*(out+1));
  *out++ = *out++ = 0;
  *(int *)out = 0;		/* 006 */
  return(1);
}

void std_to_byte (unsigned int *col_data, long len, int old_planes, unsigned long *farbtbl2, MFDB *s)
{	long  x, i, mul[32], pos;
	unsigned int np, *new_data, pixel, color, back[32];
	int  memflag = 0;
	unsigned char *p1, *p2;
	unsigned long  colback;

	if (s->fd_addr == col_data)
	{
		if ((col_data = (unsigned int *)lalloc (len * 2 * s->fd_nplanes,-1)) == 0L)
			return;
		memcpy (col_data, s->fd_addr, len * 2 * s->fd_nplanes);
		memflag = 1;
	}
	new_data = (unsigned int *)s->fd_addr;
	p1 = (unsigned char *)new_data;

	if (old_planes < 8)
	{
		colback = farbtbl2[(1 << old_planes) - 1];
		farbtbl2[(1 << old_planes) - 1] = farbtbl2[255];
	}

	for (i = 0; i < old_planes; i++)
		mul[i] = i * len;

	pos = 0;

	for (x = 0; x < len; x++)
	{
		for (np = 0; np < old_planes; np++)
			back[np] = col_data[mul[np] + x];

		for (pixel = 0; pixel < 16; pixel++)
		{
			color = 0;
			for (np = 0; np < old_planes; np++)
			{
				color |= ((back[np] & 0x8000) >> (15 - np));
				back[np] <<= 1;
			}

			switch (v_bpp)
			{
				case 2:
					new_data[pos++] = *(unsigned int *)&farbtbl2[color];
					break;

				case 3:
					p2 = (unsigned char *)&farbtbl2[color];
					*(p1++) = *(p2++);
					*(p1++) = *(p2++);
					*(p1++) = *(p2++);
					break;

				case 4:
					((unsigned long *)new_data)[pos++] = farbtbl2[color];
					break;
			}
		}
	}

	if (old_planes < 8)
		farbtbl2[(1 << old_planes) - 1] = colback;

	if (memflag)
		lfree (col_data);
}

void xfix_cicon (unsigned int *col_data, long len, int old_planes, MFDB *s, int devspef )

{	long  x, i, old_len, rest_len, mul[32], pos;
	unsigned int np, *new_data, mask, pixel, bit, color, back[32], old_col[32], maxcol;
	char  got_mem = 0;
	MFDB  d;

	len >>= 1;

	if (old_planes == vplanes)
/* 004	if (old_planes == new_planes) */
	{	if (s)
		{	/*004 if (new_planes == vplanes)
			{ */
				d = *s;
				d.fd_stand = 0;
				s->fd_addr = col_data;
				if (d.fd_addr == s->fd_addr)
				  if ((d.fd_addr = lalloc (len * 2 * old_planes,-1)) == 0L)
 				      d.fd_addr = s->fd_addr;
				  else got_mem = 1;

				vr_trnfm (vdi_hand, s, &d);
				if (got_mem)
				{
					memcpy (s->fd_addr, d.fd_addr, len * 2 * old_planes);
					lfree (d.fd_addr);
				}
/*004			}
			else
				memcpy (s->fd_addr, col_data, len * 2 * new_planes); */
		}
		return;
	}

/*004	if (new_planes <= 8) */
	if( vplanes <= 8 )
	{
		old_len  = old_planes * len;
		rest_len = vplanes /*new_planes*/ * len - old_len;

		if (s)
		{
			new_data = &((unsigned int *)s->fd_addr)[old_len];
			memset (new_data, 0, rest_len * 2);
			memcpy (s->fd_addr, col_data, old_len * 2);
			col_data = (unsigned int *)s->fd_addr;
		}
		else
			new_data = (unsigned int *)&col_data[old_len];

		for (x = 0; x < len; x++)
		{
			mask = 0xffff;

			for (i = 0; i < old_len; i += len)
				mask &= (unsigned int)col_data[x+i];

			if (mask)
				for (i = 0; i < rest_len; i += len)
					new_data[x+i] |= mask;
		}

		if (s && devspef)	/* ins ger�teabh�ngige Format konvertieren */
		{
			d = *s;
			d.fd_stand = 0;
			if ((d.fd_addr = lalloc (len * 2 * vplanes /*new_planes*/,-1)) == 0L)
				d.fd_addr = s->fd_addr;

			vr_trnfm (vdi_hand, s, &d);
			if (d.fd_addr != s->fd_addr)
			{
				memcpy (s->fd_addr, d.fd_addr, len * 2 * vplanes /*new_planes*/);
				lfree (d.fd_addr);
			}
		}
	}
	else	/* TrueColor, bzw RGB-orientierte Pixelwerte */
	{
		if (!v_bpp || !s)
		{
			for (i = 0; i < vplanes /*new_planes*/; i++)
				mul[i] = i * len;

			if (old_planes < 8)
			{
				maxcol = (1 << old_planes) - 1;
				memcpy (old_col, farbtbl[maxcol], vplanes /*new_planes*/ * sizeof (int));
				memset (farbtbl[maxcol], 0, vplanes /*new_planes*/ * sizeof (int));
			}

			if (s)
			{
				new_data = &((unsigned int *)s->fd_addr)[old_len];
				memset (new_data, 0, rest_len * 2);
				memcpy (s->fd_addr, col_data, old_len * 2);
				col_data = (unsigned int *)s->fd_addr;
			}

			for (x = 0; x < len; x++)
			{
				bit = 1;
				for (np = 0; np < old_planes; np++)
					back[np] = col_data[mul[np] + x];

				for (pixel = 0; pixel < 16; pixel++)
				{
					color = 0;
					for (np = 0; np < old_planes; np++)
					{
						color += ((back[np] & 1) << np);
						back[np] >>= 1;
					}

					for (np = 0; np < vplanes /*new_planes*/; np++)
					{	pos = mul[np] + x;
						col_data[pos] = (col_data[pos] & ~bit) | (farbtbl[color][np] & bit);
					}

					bit <<= 1;
				}
			}
			if (old_planes < 8)
				memcpy (farbtbl[maxcol], old_col, vplanes /*new_planes*/ * sizeof (int));

			if (s && devspef)	/* ins ger�teabh�ngige Format konvertieren */
			{
				d = *s;
				d.fd_stand = 0;
				if ((d.fd_addr = lalloc (len * 2 * vplanes /*new_planes*/,-1)) == 0L)
					d.fd_addr = s->fd_addr;

				vr_trnfm (vdi_hand, s, &d);
				if (d.fd_addr != s->fd_addr)
				{
					memcpy (s->fd_addr, d.fd_addr, len * 2 * vplanes /*new_planes*/);
					lfree (d.fd_addr);
				}
			}
		}
		else
		{
			std_to_byte (col_data, len, old_planes, farbtbl2, s);
			s->fd_stand = 0;	/* 004 */
		}
	}
}

void trans_cicon( CICONBLK *c, CICON *best, int alloc )
{
  MFDB d;
  long len, vlen;
  char eq;

  d.fd_stand = 1;
  d.fd_nplanes = vplanes;
  d.fd_w       = c->monoblk.ib_wicon;
  d.fd_h       = c->monoblk.ib_hicon;
  d.fd_wdwidth = d.fd_w >> 4;

  vlen = (len = (long)(d.fd_wdwidth<<1)*d.fd_h) * vplanes;
  if( (eq = best->num_planes == vplanes && !alloc) != 0 ) d.fd_addr = best->col_data;
  else if( (d.fd_addr = lalloc( vlen, curapp->id )) == 0L )
      return;
  if( alloc ) memcpy( (void *)best->col_data=d.fd_addr, best->col_data, vlen );
/*  else best->next_res = (CICON *)1L;	006 */
  xfix_cicon ((unsigned int *)best->col_data, len, best->num_planes, &d, 1);
  if( !eq ) best->col_data = (int *)d.fd_addr;
  if (best->sel_data)
  {
    if( eq ) d.fd_addr = best->sel_data;
    else if( (d.fd_addr = lalloc( vlen, curapp->id )) == 0L )
    {
      best->sel_data = 0L;
      return;
    }
    if( alloc ) memcpy( best->sel_data=(int *)d.fd_addr, best->sel_data, vlen );
    xfix_cicon ((unsigned int *)best->sel_data, len, best->num_planes, &d, 1);
    if( !eq ) best->sel_data = (int *)d.fd_addr;
  }
  best->num_planes = vplanes;
}

int _rsrc_rcfix( RSHDR *rsc, long *out )
{
  char is_new, *txt, **ptxt;
  OBJECT **ptr;
  int i;
  long *l, *cicons, ii, ic;
  CICONBLK *c;
  CICON *ci, *next, *best;

  is_new = long_rsc(rsc);
  *(out+1) = add = (long)rsc;
  num_cic = 0L;
  if( (rsc->rsh_vrsn & (1<<2)) || is_new )
  {
    l = get_cicons(rsc);
    *l += (long)rsc;
    cicons = *(long **)l;
    for( l=cicons, ii=0L; !*l; l++, ii++ );
    if( *l++ == -1L )
    {
      cicon_ptr = cicons;
      num_cic = ii;
      for( c=(CICONBLK *)l; *cicons!=-1L; )
      {
        *cicons++ = (long)c;
        ii = (long)((c->monoblk.ib_wicon+15)>>4) * c->monoblk.ib_hicon;	/* words in mono data */
        ic = *(long *)((long)c + sizeof(ICONBLK));	/* number of color icons */
        txt = (char *)((c->monoblk.ib_pmask =
            (c->monoblk.ib_pdata = (int *)((long)c + sizeof(ICONBLK) +
            sizeof(long))) + ii) + ii);
        ptxt = &c->monoblk.ib_ptext;
        *ptxt = *ptxt && (unsigned long)*ptxt < (unsigned long)txt-
            (unsigned long)rsc ? (char *)((long)rsc + (long)*ptxt) : txt;
        ci = (CICON *)((long)txt + 12);
        best = 0L;
        while( --ic>=0 )
        {
          ci->col_data = (int *)(ci+1);
          next = (CICON *)(ci->col_mask = ci->col_data + ii*ci->num_planes);
          if( ci->sel_data )
          {
            ci->sel_data = ci->col_mask + ii;
            ci->sel_mask = ci->sel_data + ii*ci->num_planes;
            next = (CICON *)ci->sel_mask;
          }
          if( ci->num_planes <= vplanes && (!best || ci->num_planes > best->num_planes) )
              best = ci;
          ci->next_res = 0L;
          ci = (CICON *)((int *)next + ii);
        }
        *(CICON **)((long)c + sizeof(ICONBLK)) = best;	/* 006 */
        if( best ) trans_cicon( c, best, 0 );
        c = (CICONBLK *)ci;
      }
    }
    else rsc->rsh_vrsn &= ~(1<<2);
  }
  fix_rsc( rsc, (char *)rsc+(is_new ? sizeof(RSHDR2) : sizeof(RSHDR)) );
  *out=(is_new?((RSHDR2 *)rsc)->rsh_trindex:rsc->rsh_trindex)+(long)rsc;
  *(int *)(out+2) = is_new ? -1 : rsc->rsh_rssize;	/* 006 */
/**  for( ptr=(OBJECT **)(*out=(is_new?((RSHDR2 *)rsc)->rsh_trindex:
      rsc->rsh_trindex)+(long)rsc), i=is_new?((RSHDR2 *)rsc)->rsh_ntree:
      rsc->rsh_ntree; --i>=0; )
    map_tree( *ptr++, 0, -1, obfix ); **/
  return 1;
}

int _rsrc_load( char *name, long *out )
{
  int hand, noerr=0;
  RSHDR *rsc;
  char temp[120];
  BASPAG *old_bp;
  DTA *old, rsld;

  old_bp = shel_context(0L);
  old = Fgetdta();
  Fsetdta(&rsld);
  strcpy( temp, name );
  if( shel_find(temp) && !Fsfirst(temp,0x27)/*004: was 0x37*/ &&
      (hand = Fopen(temp,0)) != 0 )
  {
    shel_context(old_bp);
    rsc = (RSHDR *)lalloc(rsld.d_length,curapp->id);
    old_bp = shel_context(0L);
    if( rsc != 0 )
      if( Fread( hand, rsld.d_length, rsc ) == rsld.d_length )
      {
        _rsrc_rcfix( rsc, out );
        noerr = 1;
      }
      else
      {
        shel_context(old_bp);
        lfree(rsc);
        old_bp = shel_context(0L);
        DEBUGGER(RSLOAD,LOADALL,0);
        *out++ = *out++ = 0;
        *(int *)out = 0;		/* 006 */
      }
    else
    {
      DEBUGGER(RSLOAD,NOMEM,0);
      *out++ = *out++ = 0;
      *(int *)out = 0;		/* 006 */
    }
    Fclose(hand);
  }
  else DEBUGGER(RSLOAD,NOTFOUND,0);
  Fsetdta(old);
  shel_context(old_bp);
  return(noerr);
}

int _rsrc_gaddr( int type, unsigned int index, long *out, char **gaddr );

long *rsrc_addr( int type, unsigned int index, long *out )
{
  RSHDR *rsh;
  int len;
  char *p, is_new;

  rsh=(RSHDR *)*(out+1);
  if( type!=R_TREE && !rsh || type==R_TREE && !*out ) return(0);
  is_new = long_rsc(rsh);
  switch( type )
  {
    case R_TREE:
      return (long *)&((OBJECT **)(*out))[index];
    case R_OBJECT:
      len = sizeof(OBJECT);
      goto get;
    case R_TEDINFO:
      len = sizeof(TEDINFO);
      goto get;
    case R_ICONBLK:
      len = sizeof(ICONBLK);
      goto get;
    case R_BITBLK:
      len = sizeof(BITBLK);
get:  return (long *)(index*len + (long)rsh + (is_new ?
          *((long *)rsh+type) : *((unsigned int *)rsh+type)));
    case R_STRING:
      return (long *)&((long *)((is_new ? ((RSHDR2 *)rsh)->rsh_frstr :
          rsh->rsh_frstr) + (long)rsh))[index];
    case R_IMAGEDATA:
      type = 7;
      len = sizeof(char *);
      goto get;
    case R_OBSPEC:
      _rsrc_gaddr( R_OBJECT, index, out, &p );
      return (long *)&((OBJECT *)p)->ob_spec.index;
    case R_TEPTEXT:
      _rsrc_gaddr( R_TEDINFO, index, out, &p );
      return (long *)&((TEDINFO *)p)->te_ptext;
    case R_TEPTMPLT:
      _rsrc_gaddr( R_TEDINFO, index, out, &p );
      return (long *)&((TEDINFO *)p)->te_ptmplt;
    case R_TEPVALID:
      _rsrc_gaddr( R_TEDINFO, index, out, &p );
      return (long *)&((TEDINFO *)p)->te_pvalid;
    case R_IBPMASK:
      _rsrc_gaddr( R_ICONBLK, index, out, &p );
      return (long *)&((ICONBLK *)p)->ib_pmask;
    case R_IBPDATA:
      _rsrc_gaddr( R_ICONBLK, index, out, &p );
      return (long *)&((ICONBLK *)p)->ib_pdata;
    case R_IPBTEXT:
      _rsrc_gaddr( R_ICONBLK, index, out, &p );
      return (long *)&((ICONBLK *)p)->ib_ptext;
    case R_BIPDATA:
      _rsrc_gaddr( R_BITBLK, index, out, &p );
      return (long *)&((BITBLK *)p)->bi_pdata;
    case R_FRSTR:
      type = 5;
      len = sizeof(char *);
      goto get;
    case R_FRIMG:
      type = 8;
      len = sizeof(char *);
      goto get;
    default:
      DEBUGGER(RSGA,UNKTYPE,type);
      return(0);
  }
}

int _rsrc_gaddr( int type, unsigned int index, long *out, char **gaddr )
{
  long *addr;

  if( (addr = rsrc_addr( type, index, out )) == 0 ) return 0;
  if( type==R_TREE || type==R_STRING ) *gaddr = *(char **)addr;
  else *gaddr = (char *)addr;
  return 1;
}

int _rsrc_saddr( int type, unsigned int index, long *out, long *saddr )
{
  long *addr;

  if( (addr = rsrc_addr( type, index, out )) == 0 ) return 0;
  *addr = *saddr;
  return 1;
}
/*  RSHDR *rsh;

  if( (rsh=(RSHDR *)*(out+1)) == 0 ) return(0);
  switch( type )
  {
    case R_FRSTR:
      type = 5;
      goto set;
    case R_FRIMG:
      type = 8;
set:  *(long *)(index*sizeof(char *) + (long)rsh +
          (long_rsc(rsh) ? *((unsigned long *)rsh+type) :
          *((unsigned int *)rsh+type))) = *saddr;
      return(1);
    default:
      DEBUGGER(RSSA,UNKTYPE,type);
      return(0);
  }
}*/


