#include "string.h"
#include "stdlib.h"
#include "tos.h"
#include "stdio.h"
#include "ctype.h"
#include "time.h"
#ifdef GERMAN
  #include "..\german\help_str.h"
#else
  #include "help_str.h"
#endif

#define LINES0   	200
#define LINELEN		100
#define HELP_ID		0x476E0100	/* Gn 1.0 */
#define HELP_ID1	0x476E0101	/* Gn 1.1 */
#define HELP_ID2	0x476E0102	/* Gn 1.2 */
#define MAXKEYLEN	((LINELEN-2-(4*2)-(2*2))/2)  /* -CR/LF -jumps -spaces */
#define COLUMN_WID	(MAXKEYLEN+2+4)	/* +2 spaces +4 for jump addr */
#define NIMG	   	255

#define TYPE_INDEX	3
#define TYPE_HIDDEN	2
#define TYPE_CASE   	1
#define TYPE_SCREEN   	0

#define NEXT   { filelen--; ptr++; }
#define NEXTCR { filelen--; if( *++ptr=='\n' ) line++; }

#define F_IMAGE  0
#define F_BUTTON 1

typedef struct
{
  char *name;
  char ints, strings;
} FUNCTION;

FUNCTION funcs[] = { "image", 4, -99, "button", 3, 1, "bar", 4, 0,
    "line", -2, 0, "rbox", 4, 0, "rfbox", 4, 0, "recfl", 4, 0, "sfillcol", 1, 0,
    "sfillint", 1, 0, "sfillstyle", 1, 0, "slinecol", 1, 0, "slineends", 2, 0,
    "slinestyle", 1, 0, "swrite", 1, 0, "slinewidth", 1, 0, "extern", 0, 1, 0L };

typedef struct btree
{	/* layout must not change because of user_index memcpy() */
  struct btree *left, *right, *brother, *younger;
  char name[MAXKEYLEN+1];
  char index_name[MAXKEYLEN+1];
  char type, *text;
  long len, txt_pos, line;
  unsigned int tbl_pos;
} BTREE;

typedef struct header
{
  long id;
  int hdsize;		/* sizeof(HEADER) */
  long tbl_len,		/* longword, alphabetical pointers */
       caps_start,	/* 6-byte table, relative to entry */
       caps_len,
       caps_entries,
       sens_start,
       sens_len,
       sens_entries,
       img_start;
  int  img_len;
} HEADER;

HEADER head;
BTREE *root, *user_index;
struct
{
  long offset, unpacked;
} img_tbl[NIMG];
long line, caps_ent, sens_ent, caps_topic, sens_topic;
char *get_ptr;
long get_len;
char out_buf[1024], *out=out_buf, img_buf[1024], fatal, to_hs;
int outhand, tbl_hand, caps_hand, sens_hand, caps_thand, sens_thand,
    out_err, entries[27], ent_left[27], multi_ind, nimg, cimg, img_hand;
unsigned int nodenum, unique;
char find_buf[MAXKEYLEN+1], *find_txt;
int find_len, lines=LINES0, biggest;
char copy_info[200],	/* must be large enough for entire copyright msg */
     (*index)[][LINELEN], multi;

#define N		 4096	/* size of ring buffer */
#define F		   18	/* upper limit for match_length */
#define THRESHOLD	2   /* encode string into position and length
						   if match_length is greater than this */
#define NIL			N	/* index for root of binary search trees */

unsigned long int
  textsize = 0,	/* text size counter */
  codesize = 0;	/* code size counter */
unsigned char
  text_buf[N + F - 1];	/* ring buffer of size N,
  with extra F-1 bytes to facilitate string comparison */
int match_position, match_length,  /* of longest match.  These are
		set by the InsertNode() procedure. */
  lson[N + 1], rson[N + 257], dad[N + 1];  /* left & right children &
		parents -- These constitute binary search trees. */

BTREE *add_tree( char *name, char type )
{
  BTREE *b, *par;
  int i;

  for( b=root, par=0L; b; )
  {
    if( type!=TYPE_CASE && b->type!=TYPE_CASE ) i = strcmpi(name,b->name);
    else i=strcmp(name,b->name);
    if( !i )
    {
      printf( DUP_KEY, line );
      exit(2);
    }
    else
    {
      par = b;
      if( i<0 ) b = b->left;
      else b = b->right;
    }
  }
  if( (b=(BTREE *)malloc(sizeof(BTREE))) == 0 )
  {
    printf( NO_MEM, line );
    exit(3);
  }
  if( !par ) root = b;
  else if( i<0 ) par->left = b;
  else par->right = b;
/*%  b->parent = par; */
  b->left = b->right = b->brother = b->younger = 0L;
  strcpy( b->name, name );
  if( type!=TYPE_CASE ) strupr( b->name );
  strcpy( b->index_name, name );
  b->type = type;
  b->text = 0L;
  b->len = 0L;
  b->tbl_pos = ~0;
  return b;
}

void traverse( BTREE *b, void func(BTREE *b) )
{
  if(b)
  {
    traverse( b->left, func );
    (*func)(b);
    traverse( b->right, func );
  }
}

int find_it( BTREE *b )
{  /* return:  0: found  -1: not found yet  1: not found */
  int i;

  if( b )
  {
    if( (i=find_it(b->left)) >= 0 ) return i;
    /* if strcmp returns a positive result, it's definitely not here */
    if( b->type==TYPE_CASE ) i=strcmp(b->name,find_buf);
    else i=strcmpi(b->name,find_buf);
    if( i == 0 )
    {
      while( b->brother ) b=b->brother;
      nodenum = b->tbl_pos;
      return i;
    }
      /* if this is the index pass, it might come later */
/*    else if( i>0 && !multi_ind ) return i;*/

    if( (i=find_it(b->right)) >= 0 ) return i;
  }
  return -1;
}

/*% int find_index( BTREE *b )
{
  int i;

  if( b )
  {
    if( (i=find_index(b->left)) >= 0 ) return i;
    /* if strcmp returns a positive result, it's definitely not here */
    if( b->type==TYPE_CASE ) i=strcmp(b->name,find_txt);
    else i=strcmpi(b->name,find_txt);
    if( i == 0 )
    {
      while( b->brother ) b=b->brother;
      user_index = b;
      return i;
    }
    else if( i>0 ) return i;
    if( (i=find_index(b->right)) >= 0 ) return i;
  }
  return -1;
} */

void scankey( BTREE *b )
{
  long len, line;
  unsigned int i;
  char *ptr, *ptr2, *ptr3, c;
  unsigned char *but;

  if(b)
  {
    len = b->len;
    ptr = b->text;
    line = b->line;
    while( len > 0 )
    {
      but = 0L;		/* 005 */
      while( len && *ptr!='\x1d' && *ptr!='\x1e' )
      {
        len--;
        if( *ptr++ == '\n' ) line++;	/* 005 */
      }
      if( len>2 && *ptr=='\x1e' )
      {
        if( *(ptr+2) == F_BUTTON )
        {
          but = (unsigned char *)(ptr+1);	/* 005 */
          ptr += 6;	/* move ahead to '\x1d' */
          len -= 6;
        }
        else
        {
          i = *(unsigned char *)(ptr+1) + 1;
          ptr += i;
          len -= i;
          continue;	/* skip keyword check */
        }
      }
      if( len > 0 )
      {
        ptr3 = ++ptr;
        ptr += 2;
        len -= 3;	/* 005: was 2 */
        if( (i = *ptr=='{' && (ptr2 = strchr(ptr,'}')) != 0 && ptr2 < strchr(ptr,'\x1d')) != 0 )	/* 005 */
        {
          len--;
          ptr++;
        }
        ptr2 = find_buf;
        do
          len--;
        while( (c = *ptr2++ = *ptr++) != '\x1d' && (!i || c!='}')/*005*/ );
        *(ptr2-1) = 0;
        if( find_it( root ) )
        {
          if(i)
          {
            fatal = 1;
            printf( BADEXT, line, find_buf );
          }
          else printf( EXTERNAL, line, find_buf );
          nodenum = ~0;
        }
        if(i)				/* 005 */
        {
          while( isspace(*ptr) && len>0 )
          {
            ptr++;
            len--;
          }
          if( !to_hs )
          {
            b->len -= (i=ptr-ptr3-2);
            memcpy( ptr3+2, ptr, len );
            ptr -= i;
            if( but ) *but -= i;
          }
          do
            len--;
          while( *ptr++ != '\x1d' );
        }
        *ptr3++ = nodenum>>8;
        *ptr3 = nodenum;
      }
    }
  }
}

void set_tbl( BTREE *b )
{
  if( b && !b->brother ) b->tbl_pos = unique++;
}

void calc_size( BTREE *b )
{
  char c;

  if(b)
  {
    if( b->type==TYPE_CASE )
    {
      caps_ent++;
      caps_topic += strlen(b->name)+1;
    }
    else if( b->type==TYPE_SCREEN )
    {
      sens_ent++;
      sens_topic += strlen(b->name)+1;
    }
    if( b->type!=TYPE_HIDDEN )
      if( isalpha(c=b->name[0]) ) entries[(c&0x5f)-'A']++;
      else entries[26]++;
  }
}

void Encode(void);

void _put( int hand, void *buf, long len )
{
  if( !out_err )
    if( len>0 && Fwrite( hand, len, buf ) != len )
        out_err = len<0 ? len : 14;
}

int _putl( int hand, char *buf )
{
  int i;

  _put( hand, buf, i=strlen(buf) );
  return i;
}

int find_it2( BTREE *b )
{  /* return:  0: found  -1: not found yet  1: not found */
  int i, l;

  if( b )
  {
    if( (i=find_it2(b->left)) >= 0 ) return i;
    l = strlen(b->name);
    /* if strcmp returns a positive result, it's definitely not here */
    if( b->type==TYPE_CASE ) i=strncmp(b->name,find_txt,l);
    else i=strncmpi(b->name,find_txt,l);
    if( i==0 )
    {
      find_len = strlen(b->name);
      while( b->brother ) b=b->brother;
      nodenum = b->tbl_pos;
      return i;
    }
    else if( i>0 ) return i;
    if( (i=find_it2(b->right)) >= 0 ) return i;
  }
  return -1;
}

void out_b( BTREE *b )
{
  long len, l;
  char *cur, *p, temp[20];
  int refs[50], r=0, i;
  FUNCTION *f;

  len = b->len;
  cur = p = b->text;
  while( len>0 )
  {
    while( isspace(*p) )
    {
      p++;
      if( --len<=0 ) goto end;
    }
    if( *p=='\\' )
    {
      _put( outhand, cur, p-cur );
      cur = ++p;
      len--;
      _putl( outhand, "\\\\" );
    }
    else if( *p=='\x1e' )
    {
      _put( outhand, cur, p-cur );
      cur = ++p;
      if( --len<3 ) goto end;
      l = *(unsigned char *)p;
      _putl( outhand, "\\@" );
      f = &funcs[*(p+1)];
      cur = p+2;
      l -= 2;
      _putl( outhand, f->name );
      if( (i = f->ints) < 0 ) i = l;
      while( --i>=0 )
      {
        sprintf( temp, " %d", *(unsigned char *)cur++ );
        _putl( outhand, temp );
        l--;
      }
      while(l>0)
      {
        _putl( outhand, " " );
        if( *cur=='\x1d' )
        {
          cur += 2;
          for( i=4; *++cur!='\x1d'; i++ )
            _put( outhand, cur, 1 );
          cur++;
        }
        else cur += i = _putl( outhand, cur ) + 1;
        l -= i;
      }
      len -= cur-p;
      p = cur;
      while( !isspace(*p) && *p!='\\' && *p!='\x1d' )
      {
        p++;
        if( --len<=0 ) goto end;
      }
    }
    else if( *p=='\x1d' )
    {
      _put( outhand, cur, p-cur );
      _putl( outhand, "\\#" );
      cur = p += 3;
      len -= 4;
      while( *p++ != '\x1d' ) len--;
      _put( outhand, cur, p-cur-1 );
      _putl( outhand, "\\#" );
      cur = p;
      while( !isspace(*p) && *p!='\\' && *p!='\x1d' )
      {
        p++;
        if( --len<=0 ) goto end;
      }
    }
    else
    {
      find_txt = p;
      if( !find_it2(root) && find_len<=len && nodenum!=b->tbl_pos )
      {
        for( i=0; i<r; i++ )
          if( refs[i]==nodenum ) break;
        if( i==r )
        {
          if( r<50 ) refs[r++] = nodenum;
          _put( outhand, cur, p-cur );
          _putl( outhand, "\\##" );
          _put( outhand, p, find_len );
          _putl( outhand, "\\#" );
          p += find_len;
          len -= find_len;
          cur = p;
          continue;
        }
      }
      while( !isspace(*p) && *p!='\\' && *p!='\x1d' )
      {
        p++;
        if( --len<=0 ) goto end;
      }
    }
  }
end:
  p += len;	/* in case len<0 */
  _put( outhand, cur, p-cur );
}

void out_hs( BTREE *b )
{
  static char names[4][11] = { "\\screen ", "\\casesens ", "\\hidden ", "\\index" };
  BTREE *b2;

  if( b && !b->brother )
  {
    if( (unsigned char)(b->index_name[0])<' ' && b->type!=TYPE_INDEX ) return;
    /* preserve the order by going backward, then go forward again */
    for( b2=b;;)
    {
      if( !b2->younger ) break;
      b2 = b2->younger;
    }
    for(;;)
    {
      _put( outhand, names[b2->type], strlen(names[b2->type]) );
      if( b2->type!=3/*005*/ ) _put( outhand, b2->index_name, strlen(b2->index_name) );
      _put( outhand, "\r\n", 2 );
      if( !b2->brother ) break;
      b2 = b2->brother;
    }
    out_b(b);
    _put( outhand, "\\end\r\n\r\n", 8 );
  }
}

void write_enc( BTREE *b )
{
  long l, l2, len;
  unsigned int i;
  int h;
  char *p, *p0, *p2;

  if( b && !out_err && !b->brother )
  {
    b->txt_pos = Fseek( 0L, outhand, 1 ) + out - out_buf;
    if( nimg )
      for( p=b->text, l=b->len; --l>=0; )
        if( *p++ == '\x1e' && l>2 )
        {
          i = *(unsigned char *)p;
          if( *(p+1) == F_IMAGE )
          {
            l -= i;
            i -= 2+funcs[F_IMAGE].ints;
            p0 = p;
            p2 = p += 2+funcs[F_IMAGE].ints;
            while(i)
            {
              if( (h = Fopen(p,0)) < 0 )
              {
                printf( OPENERR, p );
                exit(h);
              }
              for(len=0L;;len+=l2)
                if( (l2 = Fread( h, sizeof(img_buf), img_buf )) == 0 ) break;
                else if( l2<0 )
                {
                  printf( READERR, p );
                  exit((int)l2);
                }
                else
                {
                  _put( img_hand, img_buf, l2 );
                  if( !len ) *(char *)&img_tbl[cimg].offset = img_buf[4]|img_buf[5];
                }
              Fclose(h);
              h = strlen(p)+1;
              i -= h;
              p += h;
              img_tbl[cimg].unpacked = len;
              *p2++ = cimg++;
            }
            *p0 = (char)(p2-p0);
            memcpy( p2, p, l );
            b->len -= p-p2;
            p = p2;
          }
          else
          {
            p += i;
            l -= i;
          }
        }
    get_len = b->len;
    get_ptr = b->text;
    textsize = codesize = 0;
    putchar('.');
    Encode();
  }
}

void write_alias( BTREE *b )
{
  long l;
  BTREE *b2;

  if( b && !out_err )
  {
    if( !b->brother ) _put( tbl_hand, &b->txt_pos, 4L );
    b2 = b;
    while( b2->brother ) b2 = b2->brother;
    if( b->type==TYPE_CASE )
    {
      l = Fseek( 0L, caps_thand, 1 ) - Fseek( 0L, caps_hand, 1 );
      _put( caps_hand, &l, 4L );
      _put( caps_hand, &b2->tbl_pos, 2L );
      _put( caps_thand, b->name, strlen(b->name)+1 );
    }
    else if( b->type==TYPE_SCREEN )
    {
      l = Fseek( 0L, sens_thand, 1 ) - Fseek( 0L, sens_hand, 1 );
      _put( sens_hand, &l, 4L );
      _put( sens_hand, &b2->tbl_pos, 2L );
      _put( sens_thand, b->name, strlen(b->name)+1 );
    }
  }
}

int get_line( int i )
{
  int j, ret=2;

  for( j=0; j<i; j++ )
    if( entries[j] ) ret += (entries[j]+3)>>1;
  return ret;
}

void index_it( BTREE *b )
{
  BTREE *b2;
  char c, *ptr, temp[COLUMN_WID];
  int i, half, index_line;

  if( b && b->type!=TYPE_HIDDEN )
  {
    if( isalpha(c=b->name[0]) ) i = (c&0x5f)-'A';
    else i = 26;
    if( multi && i != multi_ind ) return;
    b2 = b;
    while( b2->brother ) b2 = b2->brother;
    half = entries[i]>>1;
    index_line = multi ? 1 : get_line(i);
    if( ent_left[i] <= half )  /* rt column */
        ptr = (*index)[index_line+(half-ent_left[i])] + COLUMN_WID;
    else ptr = (*index)[index_line+(entries[i]-ent_left[i])];
    memcpy( ptr, temp, sprintf( temp,
        "  \x1d%c%c%s\x1d", b2->tbl_pos>>8, b2->tbl_pos, b->index_name ) );
    ent_left[i]--;
  }
}

void InitTree(void)  /* initialize trees */
{
	int  i;

	/* For i = 0 to N - 1, rson[i] and lson[i] will be the right and
	   left children of node i.  These nodes need not be initialized.
	   Also, dad[i] is the parent of node i.  These are initialized to
	   NIL (= N), which stands for 'not used.'
	   For i = 0 to 255, rson[N + i + 1] is the root of the tree
	   for strings that begin with character i.  These are initialized
	   to NIL.  Note there are 256 trees. */

	for (i = N + 1; i <= N + 256; i++) rson[i] = NIL;
	for (i = 0; i < N; i++) dad[i] = NIL;
}

void InsertNode(int r)
	/* Inserts string of length F, text_buf[r..r+F-1], into one of the
	   trees (text_buf[r]'th tree) and returns the longest-match position
	   and length via the global variables match_position and match_length.
	   If match_length = F, then removes the old node in favor of the new
	   one, because the old one will be deleted sooner.
	   Note r plays double role, as tree node and position in buffer. */
{
	int  i, p, cmp;
	unsigned char  *key;

	cmp = 1;  key = &text_buf[r];  p = N + 1 + key[0];
	rson[r] = lson[r] = NIL;  match_length = 0;
	for ( ; ; ) {
		if (cmp >= 0) {
			if (rson[p] != NIL) p = rson[p];
			else {  rson[p] = r;  dad[r] = p;  return;  }
		} else {
			if (lson[p] != NIL) p = lson[p];
			else {  lson[p] = r;  dad[r] = p;  return;  }
		}
		for (i = 1; i < F; i++)
			if ((cmp = key[i] - text_buf[p + i]) != 0)  break;
		if (i > match_length) {
			match_position = p;
			if ((match_length = i) >= F)  break;
		}
	}
	dad[r] = dad[p];  lson[r] = lson[p];  rson[r] = rson[p];
	dad[lson[p]] = r;  dad[rson[p]] = r;
	if (rson[dad[p]] == p) rson[dad[p]] = r;
	else                   lson[dad[p]] = r;
	dad[p] = NIL;  /* remove p */
}

void DeleteNode(int p)  /* deletes node p from tree */
{
	int  q;

	if (dad[p] == NIL) return;  /* not in tree */
	if (rson[p] == NIL) q = lson[p];
	else if (lson[p] == NIL) q = rson[p];
	else {
		q = lson[p];
		if (rson[q] != NIL) {
			do {  q = rson[q];  } while (rson[q] != NIL);
			rson[dad[q]] = lson[q];  dad[lson[q]] = dad[q];
			lson[q] = lson[p];  dad[lson[p]] = q;
		}
		rson[q] = rson[p];  dad[rson[p]] = q;
	}
	dad[q] = dad[p];
	if (rson[dad[p]] == p) rson[dad[p]] = q;  else lson[dad[p]] = q;
	dad[p] = NIL;
}


int _getc(void)
{
  if( !get_len ) return EOF;
  get_len--;
  return *((unsigned char *)get_ptr)++;
}

void flush_out(void)
{
  long l;

  if( !out_err && out_buf!=out )
  {
    if( Fwrite( outhand, l=out-out_buf, out_buf ) != l )
        out_err = l<0 ? l : 14;
    out = out_buf;
  }
}

void _putc( char c )
{
  if( out-out_buf == sizeof(out_buf) ) flush_out();
  *out++ = c;
}

void _puts( void *s, long len )
{
  while(len--)
    _putc( *((char *)s)++ );
}

void Encode(void)
{
	int  i, c, len, r, s, last_match_length, code_buf_ptr;
	unsigned char  code_buf[17], mask;

	InitTree();  /* initialize trees */
	code_buf[0] = 0;  /* code_buf[1..16] saves eight units of code, and
		code_buf[0] works as eight flags, "1" representing that the unit
		is an unencoded letter (1 byte), "0" a position-and-length pair
		(2 bytes).  Thus, eight units require at most 16 bytes of code. */
	code_buf_ptr = mask = 1;
	s = 0;  r = N - F;
	memset( &text_buf[s], ' ', N-F );
/*	for (i = s; i < r; i++) text_buf[i] = ' ';*/  /* Clear the buffer with
		any character that will appear often. */
	for (len = 0; len < F && (c = _getc()) != EOF; len++)
		text_buf[r + len] = c;  /* Read F bytes into the last F bytes of
			the buffer */
	if ((textsize = len) == 0) return;  /* text of size zero */
	for (i = 1; i <= F; i++) InsertNode(r - i);  /* Insert the F strings,
		each of which begins with one or more 'space' characters.  Note
		the order in which these strings are inserted.  This way,
		degenerate trees will be less likely to occur. */
	InsertNode(r);  /* Finally, insert the whole string just read.  The
		global variables match_length and match_position are set. */
	do {
		if (match_length > len) match_length = len;  /* match_length
			may be spuriously long near the end of text. */
		if (match_length <= THRESHOLD) {
			match_length = 1;  /* Not long enough match.  Send one byte. */
			code_buf[0] |= mask;  /* 'send one byte' flag */
			code_buf[code_buf_ptr++] = text_buf[r];  /* Send uncoded. */
		} else {
			code_buf[code_buf_ptr++] = (unsigned char) match_position;
			code_buf[code_buf_ptr++] = (unsigned char)
				(((match_position >> 4) & 0xf0)
			  | (match_length - (THRESHOLD + 1)));  /* Send position and
					length pair. Note match_length > THRESHOLD. */
		}
		if ((mask <<= 1) == 0) {  /* Shift mask left one bit. */
			for (i = 0; i < code_buf_ptr; i++)  /* Send at most 8 units of */
				_putc(code_buf[i]);     /* code together */
			codesize += code_buf_ptr;
			code_buf[0] = 0;  code_buf_ptr = mask = 1;
		}
		last_match_length = match_length;
		for (i = 0; i < last_match_length &&
				(c = _getc()) != EOF; i++) {
			DeleteNode(s);		/* Delete old strings and */
			text_buf[s] = c;	/* read new bytes */
			if (s < F - 1) text_buf[s + N] = c;  /* If the position is
				near the end of buffer, extend the buffer to make
				string comparison easier. */
			s = (s + 1) & (N - 1);  r = (r + 1) & (N - 1);
				/* Since this is a ring buffer, increment the position
				   modulo N. */
			InsertNode(r);	/* Register the string in text_buf[r..r+F-1] */
		}
		textsize += i;
		while (i++ < last_match_length) {	/* After the end of text, */
			DeleteNode(s);					/* no need to read, but */
			s = (s + 1) & (N - 1);  r = (r + 1) & (N - 1);
			if (--len) InsertNode(r);		/* buffer may not be empty. */
		}
	} while (len > 0);	/* until length of string to be processed is zero */
	if (code_buf_ptr > 1) {		/* Send remaining code. */
		for (i = 0; i < code_buf_ptr; i++) _putc(code_buf[i]);
		codesize += code_buf_ptr;
	}
}

int unx_eof(void)
{
  printf( UNX_EOF );
  return 6;
}

int syntax(void)
{
  printf( SYNTAX, line );
  return 7;
}

void _fseek( long pos, int hand )
{
  if( Fseek( pos, hand, 0 ) != pos )
  {
    printf( SEEK_ERR );
    exit(16);
  }
}

void blank_index(void)
{
  int i;

  memset( index, ' ', lines*(long)LINELEN );
  for( i=lines; --i>=0; )
  {
    (*index)[i][LINELEN-2] = '\r';
    (*index)[i][LINELEN-1] = '\n';
  }
}

long pack_index(void)
{
  int i, j, k, last;
  long out;

  for( out=lines*(long)LINELEN, last=0, i=lines; --i>=0; )
  {
    for( j=0; j<LINELEN-2; j++ )
      if( (*index)[i][j] != ' ' ) break;
    if( j>=LINELEN-2 && !last ) out -= LINELEN;
    else
    {
      if( j<LINELEN-2 )
      {
        if( !last ) last = i;
        for( k=LINELEN-2; --k>j; )
          if( (*index)[i][k] != ' ' ) break;
      }
      else k = -1;
      memcpy( &(*index)[i][k+1], &(*index)[i][LINELEN-2], out-((i+1)*LINELEN)+2 );
      out -= LINELEN-k-3;
    }
  }
  return out;
}

int shift( int i, int argc, char *argv[] )
{
  argc--;
  for( ; i<argc; i++ )
    argv[i] = argv[i+1];
  return argc;
}

int bad_func( char *s )
{
  printf( BADFUNC, line, s ? s : "" );
  return 30;
}

int many_parms(void)
{
  printf( TOOMANY, line );
  return -34;
}

int skip_space( long *len, char **p, int mode )
{	/* mode: -1=skip spaces  0=skip until space  1=skip until eol */
  long filelen = *len;
  char *ptr = *p, *nonsp=0L, sp;

  while( *ptr!='\r' && *ptr!='\n' && filelen )
  {
    sp = isspace(*ptr);
    if( !strncmp(ptr,"\\@",2) || mode<0 && !sp || mode==0 && sp ) break;
    if( !sp ) nonsp = ptr;
    NEXT
  }
  if( mode>0 && nonsp )
  {
    filelen += ptr-nonsp-1;
    ptr = nonsp+1;
  }
  if( !filelen ) return 0;
  *len = filelen;
  *p = ptr;
  return 1;
}

int get_num( char *p, int len, int base )
{
  int n;
  char *p2;
  static char nums[] = "0123456789ABCDEF";

  for( n=0; --len>=0; p++ )
    if( (p2=strchr( nums, toupper(*p) )) == 0 )
    {
      printf( FUNCNUM, line );
      return -31;
    }
    else if( (n = n*base+(p2-nums)) > 255 )
    {
      printf( FUNCNBIG, line );
      return -33;
    }
  return n;
}

int from_asc( char *p, int len )
{
  int base=10;

  if( len>2 && !strncmpi(p,"0x",2) )
  {
    p += 2;
    len -= 2;
    base = 16;
  }
  else if( len>1 && *p=='$' )
  {
    p++;
    len--;
    base = 16;
  }
  if( !len )
  {
    printf( FUNCNUM, line );
    return -31;
  }
  return get_num( p, len, base );
}

int read_parms( char **out, FUNCTION *f, long filelen, char *ptr, int strs )
{
  int i, min, min0, max;
  long l0 = filelen;
  char *p;

  if( (i = strs ? f->strings : f->ints) >= 0 ) min = max = i;
  else
  {
    min = i==-99 ? 0 : -i;
    max = -1;
  }
  min0 = min;
  if( !skip_space( &filelen, &ptr, -1 ) ) return -unx_eof();
  while( max-- && *ptr!='\r' && *ptr!='\n' && strncmp(ptr,"\\@",2) )
  {
    p = ptr;
    if(strs)
    {
      if( !skip_space( &filelen, &ptr, max>=0 ) ) return -unx_eof();
      if( f==&funcs[F_BUTTON] )
      {
        *(*out)++ = '\x1d';
        *(*out)++ = 0;
        *(*out)++ = 0;
      }
      strncpy( *out, p, i=ptr-p );
      *out += i;
      *(*out)++ = f==&funcs[F_BUTTON] ? '\x1d' : 0;
      if( f==&funcs[F_IMAGE] )
        if( ++nimg > NIMG )
        {
          printf( FUNCNIMG, line );
          return -35;
        }
    }
    else if( !skip_space( &filelen, &ptr, 0 ) ) return -unx_eof();
    else if( (i = from_asc(p,ptr-p)) < 0 ) return i;
    else *(*out)++ = (char)i;
    min--;
    if( !skip_space( &filelen, &ptr, -1 ) ) return -unx_eof();
  }
  if( min>0 )
  {
    printf( strs ? FUNCPARS : FUNCPARN, line, f->name, min0 );
    return -32;
  }
  return l0-filelen;
}

int hex2asc( long filelen, char *ptr )
{
  int n, len;
  char *p;
  
  if( filelen>=3 ) len = 2;
  else len = filelen-1;
  for( n=0, p=ptr+1; n<len && isxdigit(*p++); n++ );
  if( (n = get_num( ptr+1, len=n, 16 )) < 0 ) return -1;
  *(ptr-1) = n;
  len++;
  memcpy( ptr, ptr+len, filelen-len );
  return len;
}

int scan_func( long filelen, char *ptr, char **nptr )
{
  char *p, *p0;
  long l0 = filelen;
  FUNCTION *f;
  int i, n, min, max;

  p0 = ptr++;	/* skip @ */
  filelen--;
  while( isspace(*ptr) && filelen ) NEXT
  if( !filelen ) return -bad_func(0L);
  p = ptr;
  while( !isspace(*ptr) && filelen ) NEXT
  if( !filelen ) return -unx_eof();
  for( f=funcs, n=0; f->name; f++, n++ )
    if( !strncmpi( f->name, p, ptr-p ) )
    {
      p = p0+2;		/* enough space for len + type */
      if( (i=read_parms( &p, f, filelen, ptr, 0 )) < 0 ) return i;
      if( (i=read_parms( &p, f, filelen-=i, ptr+=i, 1 )) < 0 ) return i;
      filelen -= i;
      ptr += i;
      if( (i=p-p0) > 255 ) return -many_parms();
      *p0 = (char)i;
      *(p0+1) = n;
      i = l0-filelen;
      memcpy( *nptr=p, p0+i, filelen );
      return i;
    }
  *ptr = 0;
  return -bad_func(p);
}

int main( int argc, char *argv[] )
{
  int in, i;
  char *ptr, *ptr2, *file, type, *text=0L, *line_start, c, temp[]="x..", *np,
      has_func=0;
  BTREE *b=0L, *new;
  long filelen, l, textline=0L, end;
  time_t ntime;

  printf( MAINTITL );
  for( i=1; i<argc; )
    if( !strcmpi(argv[i],"-m") )
    {
      multi++;
      argc = shift( i, argc, argv );
    }
    else if( !strcmpi(argv[i],"-s") )
    {
      to_hs++;
      argc = shift( i, argc, argv );
    }
    else if( !strncmpi(argv[i],"-n",2) )
    {
      if( (lines = atoi(argv[i]+2)) <= 0 )
      {
        printf( USAGE );
        return 1;
      }
      argc = shift( i, argc, argv );
    }
    else i++;
  if( argc < 3 )
  {
    printf( USAGE );
    return 1;
  }
  if( (index = malloc( lines*(long)LINELEN )) == 0 )
  {
    printf( INDXMEM );
    return 3;
  }
  if( (in=Fopen(argv[1],0)) < 0 )
  {
    printf( OPENERR, argv[1] );
    return in;
  }
  if( (filelen=Fseek( 0L, in, 2 )) < 0 )
  {
    printf( READERR, argv[1] );
    return (int)filelen;
  }
  if( (file = malloc(filelen+1)) == 0 )
  {
    printf( FILEMEM );
    return 5;
  }
  if( (l=Fseek( 0L, in, 0 )) < 0 ||
      (l = Fread( in, filelen, ptr=file )) != filelen )
  {
    printf( READERR, argv[1] );
    return (int)l;
  }
  Fclose(in);
  printf( PASS1 );
  line = 1;
  line_start = ptr;
  while( filelen )
  {
    while( isspace(*ptr) && filelen )
    {
      filelen--;
      if( *ptr++ == '\n' )
      {
        line++;
        line_start = ptr;
      }
    }
    if( filelen )
    {
      if( *ptr=='\\' && --filelen != 0 )
        switch( *++ptr )
        {
          case '\\':	/*  "\\" change it to just a \ */
            memcpy( ptr, ptr+1, --filelen );
            break;
          case 'x':
            if( (i = hex2asc( filelen, ptr )) < 0 ) return -i;
            filelen -= i;
            break;
          case '@':
            if( (i = scan_func( filelen, ptr, &np )) < 0 ) return -i;
            *(ptr-1) = '\x1e';
            filelen -= i;
            ptr = np;
            has_func = 1;
            break;
          case '#':
            *(ptr-1) = '\x1d';
            ptr2 = ++ptr;
            filelen--;
            while( filelen && !strchr( "\r\n\\", *ptr ) ) NEXT
            if( !filelen ) return unx_eof();
            if( *ptr=='\\' )
            {
              if( ptr==ptr2 ) return syntax();
              NEXT
              if( !filelen ) return unx_eof();
              if( *ptr != '#' ) return syntax();
              if( (l=ptr-ptr2-1) > MAXKEYLEN )
              {
                printf( LONGKEY, line );
                return 9;
              }
              memcpy( ptr2+1, ptr2, l );
              *ptr = '\x1d';
            }
            else
            {
              printf( SPLITKEY, line );
              return 8;
            }
            break;
          default:
            ptr2 = ptr;
            while( !isspace(*ptr) && filelen ) NEXT
/* eof w/o eol            if( !filelen ) return unx_eof(); */
            c = *ptr;
            *ptr++ = 0;
            if( filelen ) filelen--;
            if( !strcmpi( ptr2, "end" ) )
              if( b )
                if( (i=line-textline) > lines )
                {
                  printf( MAXLINES, line, lines );
                  return 18;
                }
                else
                {
                  if( i > biggest ) biggest = i;	/* 005 */
                  b->text = text;
                  b->len = ptr2-text-1;
                  b->line = textline;			/* 005 */
                  b = 0L;
                  textline = 0L;
                  break;
                }
              else
              {
                printf( UNX_END, line );
                return 11;
              }
            if( textline && textline!=line )
            {
              printf( MIS_END, line, textline );
              return 12;
            }
            type = -1;
            if( !strcmpi( ptr2, "index" ) ) type = TYPE_INDEX;
            else if( c=='\r' || c=='\n' ) return syntax();
            if( !strcmpi( ptr2, "casesens" ) ) type = TYPE_CASE;
            else if( !strcmpi( ptr2, "screen" ) ) type = TYPE_SCREEN;
            else if( !strcmpi( ptr2, "hidden" ) ) type = TYPE_HIDDEN;
            else if( type<0 ) break;
            while( (*ptr==' ' || *ptr=='\t') && filelen ) NEXT
            if( !filelen ) return unx_eof();
            ptr2 = ptr;
            while( *ptr!='\r' && *ptr!='\n' && filelen ) NEXT
            while( *(ptr-1)==' ' || *(ptr-1)=='\t' )	/* 005 */
            {
              filelen++;
              ptr--;
            }
            if( !filelen ) return unx_eof();
            if( (l=ptr-ptr2) > MAXKEYLEN )
            {
              printf( LONGKEY, line );
              return 9;
            }
            c = *ptr;
            *ptr++ = 0;
            filelen--;
            while( c!='\r' && c!='\n' && filelen )	/* 005 */
            {
              c = *ptr++;
              filelen--;
            }
            if( type==TYPE_INDEX )
            {
              ptr2 = "\2";
              if( c=='\n' ) line++;	/* 005 */
            }
            new = add_tree( ptr2, type );
            if(b)
              if( b->type==TYPE_INDEX )	/* index must not have brothers */
              {
                if( (new->younger=b->younger) != 0 )
                    b->younger->brother = new;
                new->brother = b;
                b->younger = new;
              }
              else
              {
                b->brother = new;
                new->younger = b;
                b = new;
              }
            else b = new;
            if( type==TYPE_INDEX ) user_index = new;
            text = ptr;
            textline = line;
            if( filelen && c=='\r' && *ptr=='\n' )
            {
              textline++;
              text++;
            }
        }
      else while( filelen && *ptr!='\r' && *ptr!='\n' && *ptr!='\\' ) NEXT
    }
    if( text && ptr-line_start > LINELEN )
    {
      printf( MAXLEN, line, LINELEN );
      return 20;
    }
  }
  new = add_tree( "\1", TYPE_HIDDEN );		/* make sure it's first */
  add_tree( COPYRT, 0 ) -> brother = new;	/* create an alias */
  time(&ntime);
  new->len = sprintf( copy_info, HELPINFO, ctime(&ntime) );
  new->text = copy_info;
  if( !user_index )
  {
    new = add_tree( "\2", TYPE_HIDDEN );	/* make sure it's second */
    /* new is index btree */
  }
  else if( !to_hs )
  {
/*%    memcpy( (char *)new + 4*sizeof(BTREE *),
            (char *)user_index + 4*sizeof(BTREE *),
            sizeof(BTREE) - 4*sizeof(BTREE *) );
    user_index->brother = new;  */
    user_index->type = TYPE_HIDDEN;
  }
  printf( PASS2 );
  traverse( root, set_tbl );
  traverse( root, scankey );
  if( fatal ) exit(17);		/* 005 */
  if( to_hs )
  {
    printf( PASS3A );
    goto do_hs;
  }
  traverse( root, calc_size );
  if( !user_index )
  {
    printf( PASS3 );
    if( get_line(27)-1 > lines && !multi )
    {
      printf( GOMULT );
      multi++;
    }
    blank_index();
    memcpy( ent_left, entries, sizeof(entries) );
    if( !multi ) traverse( root, index_it );
    else
    {
    for( i=0; i<26; i++ )
      if( entries[i] )
      {
        if( entries[i] > (lines<<1) )
          if( i<25 )
          {
            printf( INDLTR, i+'A' );
            return 25;
          }
          else
          {
            printf( INDOTH );
            return 25;
          }
        temp[0] = i-26;		/* make sure at end */
        b = add_tree( temp, TYPE_HIDDEN );
        if( i<25 )
        {
          temp[0] = i+'A';
          strcpy( b->name, temp );
        }
        else strcpy( b->name, OTHER );
        strcpy( b->index_name, b->name );
        multi_ind = i;
        traverse( root, index_it );
        b->tbl_pos = unique++;
        b->len = pack_index();
        if( (b->text = malloc(b->len)) == 0 )
        {
          printf( INDXMEM );
          return 3;
        }
        memcpy( b->text, index, b->len );
        blank_index();
      }
    for( i=0, in=2; i<26; i++ )
      if( entries[i] )
      {
        if( i<25 ) sprintf( (*index)[in], "    \x1dzz%c..\x1d", i+'A' );
        else strcpy( (*index)[in], OTHER2 );
        (*index)[in][strlen((*index)[in])] = ' ';
        in++;
      }
    new->text = (*index)[0];
    new->len = in*LINELEN;
    scankey(new);
    }
    strncpy( (*index)[0], INDEX_TITLE, sizeof(INDEX_TITLE)-1 );
    new->text = (*index)[0];
    new->len = pack_index();
  }
  printf( PASS4 );
do_hs:
  if( (outhand = Fcreate(argv[2],0)) < 0 )
  {
    printf( OUTCREAT, argv[2] );
    return outhand;
  }
  if( to_hs ) traverse( root, out_hs );
  else
  {
    caps_topic = (caps_topic+1)&-2L;	/* word-align */
    sens_topic = (sens_topic+1)&-2L;
    if( biggest>100 )	/* 005 */
    {
      head.id = HELP_ID2;
      printf( WARNBIG );
    }
    else if( has_func )
    {
      head.id = HELP_ID1;
      printf( WARNFUNC );
    }
    else head.id = HELP_ID;
    head.caps_start = (head.hdsize = has_func ? sizeof(HEADER) : sizeof(HEADER)-6 ) +
                      (head.tbl_len = (unique+1)<<2);	/* entry at end */
    head.caps_len = caps_ent*6L + caps_topic;
    head.caps_entries = caps_ent;
    head.sens_start = head.caps_start+head.caps_len;
    head.sens_len = sens_ent*6L + sens_topic;
    head.sens_entries = sens_ent;
    if( nimg )
    {
      head.img_start = head.sens_start+head.sens_len;
      head.img_len = (nimg+1)*sizeof(img_tbl[0]);
      if( (img_hand = Fcreate("!HLPIMG!.$$$",0)) < 0 )
      {
        printf( OUTCREAT, "!HLPIMG!.$$$" );
        return img_hand;
      }
    }
    _puts( &head, head.hdsize );
    _puts( file, head.tbl_len+head.caps_len+head.sens_len+head.img_len );/* dummy tbls */
    traverse( root, write_enc );
  }
  flush_out();
  end = Fseek( 0L, outhand, 1 );
  Fclose(outhand);
  if( !to_hs )
  {
    if( (tbl_hand = Fopen(argv[2],1)) < 0 ||
        (caps_hand = Fopen(argv[2],1)) < 0 ||
        (caps_thand = Fopen(argv[2],1)) < 0 ||
        (sens_hand = Fopen(argv[2],1)) < 0 ||
        (sens_thand = Fopen(argv[2],1)) < 0 )
    {
      printf( XTRAOPEN );
      return 15;
    }
    _fseek( head.hdsize, tbl_hand );
    _fseek( head.caps_start, caps_hand );
    _fseek( head.caps_start+caps_ent*6L, caps_thand );
    _fseek( head.sens_start, sens_hand );
    _fseek( head.sens_start+sens_ent*6L, sens_thand );
    traverse( root, write_alias );
    _put( tbl_hand, &end, 4L );	/* write dummy entry at end of table */
    Fclose(tbl_hand);
    Fclose(caps_hand);
    Fclose(caps_thand);
    Fclose(sens_hand);
    if( nimg )
    {
      for( i=0, l=0L; i<nimg; i++ )	/* find largest img */
        if( (filelen = img_tbl[i].unpacked) > l ) l = filelen;
      free(file);
      if( (file = malloc(l)) == 0 )	/* allocate enough memory for it */
      {
        printf( FILEMEM );
        return 5;
      }
      _fseek( end, sens_thand );
      _fseek( 0L, img_hand );
      for( i=0, l=end; i<=nimg; i++ )
      {
        if( (filelen = Fread( img_hand, img_tbl[i].unpacked, file )) !=
            img_tbl[i].unpacked )
        {
          printf( READERR, "!HLPIMG!.$$$" );
          return (int)filelen;
        }
        img_tbl[i].offset = (img_tbl[i].offset&0xFF000000L) | l;
        outhand = sens_thand;
        get_len = filelen;
        get_ptr = file;
        textsize = codesize = 0;
        putchar('.');
        Encode();
        flush_out();
        l = Fseek( 0L, sens_thand, 1 );
      }
      Fclose( img_hand );
      Fdelete("!HLPIMG!.$$$");
      _fseek( head.img_start, sens_thand );
      _put( sens_thand, img_tbl, head.img_len );
    }
    Fclose(sens_thand);
  }
  if( out_err < 0 )
  {
    printf( WRITERR );
    return out_err;
  }
  if( out_err > 0 )
  {
    printf( WRITFULL );
    return out_err;
  }
  return 0;
}
