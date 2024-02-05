/*   Geneva, Copyright ½ 1993, Gribnif Software   **
**   Strings in MAKEHELP.TTP                      **
**                                                **
**   When translating, change just the things in  **
**   "quotation marks". Leave all \Escape         **
**   sequences and %formats the way they are!     **
**   I will recheck the lengths of alerts when    **
**   the translated file is returned to me.       **/

#define INDEX_TITLE	"Please double-click on an entry in the index, below:"
#define DUP_KEY		"%6ld: A keyword with this name already exists."
#define NO_MEM		"%6ld: Out of memory."
#define EXTERNAL	"%6ld Warning: Keyword \"%s\" not found\n"
#define UNX_EOF		"Unexpected end of file"
#define SYNTAX		"%6ld: Syntax error"
#define SEEK_ERR	"Seek error on output file."
#define OPENERR		"Error opening %s."
#define READERR		"Error reading %s."
#define FILEMEM		"Insufficient free memory for whole file."
#define PASS1		"Pass 1: scanning for screens...\n"
#define LONGKEY		"%6ld: Keyword too long."
#define SPLITKEY	"%6ld: Keyword cannot be broken between lines."
#define MAXLINES	"%6ld: No more than %d lines per screen allowed."
#define UNX_END		"%6ld: Unexpected \"\\end\" statement."
#define MIS_END		"%6ld: Missing \"end\" in text region started on line %ld."
#define MAXLEN		"%6ld: Keyword too long."
#define COPYRT		"Copyright"
#define PASS2		"\nPass 2: scanning for keywords...\n"
#define PASS3		"\nPass 3: creating index...\n"
#define PASS3A		"\nPass 3: finding and marking new references...\n"
#define GOMULT		"Warning: Index too large. Using multiple screens.\n"
#define INDLTR		"Fatal: Too many index entries for the letter %c"
#define INDOTH		"Fatal: Too many index entries for the \"Other\" category."
#define OTHER		"Other"
#define OTHER2		"    \x1dOther..\x1d"
#define INDXMEM		"Out of memory for index."
#define PASS4		"\nPass 4: writing compressed file...\n\n"
#define OUTCREAT	"Could not create %s."
#define XTRAOPEN	"Error opening extra file handles"
#define WRITERR		"\nError writing to output file."
#define WRITFULL	"\nError writing to output file. Disk full."
#define BADFUNC		"%6ld: Bad function name %s."
#define TOOMANY		"%6ld: Function requires too much space."
#define FUNCNUM		"%6ld: Number expected."
#define FUNCNBIG	"%6ld: Number too large (>255)."
#define FUNCNIMG	"%6ld: Too many images."
#define FUNCPARS	"%6ld: Function \"%s\" requires at least %d string parameters."
#define FUNCPARN	"%6ld: Function \"%s\" requires at least %d number parameters."
#define BADEXT		"%6ld Fatal: Keyword {%s} not found\n"
/*------------*/
#define WARNBIG		"\
Warning: Due to size of longest screen,\n\
HLP file will only be readable with\n\
Geneva 005 or newer.\n"
/*------------*/
#define WARNFUNC	"\
Warning: Due to use of functions, HLP\n\
file will only be readable with Geneva\n\
004 or newer.\n"
/*------------*/
#define MAINTITL	"\
   Geneva makehelp utility 1.2\n\
Copyright ½ 1996, Gribnif Software\n\n"
/*------------*/
#define USAGE		"\
Usage: makehelp [-m] [-n<lines>] <source> <hlpfile>\r\n\
   -m: force multiple-screen index\r\n\
   -n: max. number of lines per screen\r\n\
       (default=200)\r\n\
                 or\r\n\
       makehelp -s <source> <new source>\r\n\
   -s: find keywords in text and generate\r\n\
       new source file\r\n"
/*------------*/
#define HELPINFO	"\
       Geneva¿ Makehelp Utility 1.2\r\n\
    Copyright ½ 1996, Gribnif Software\r\n\r\n\
Help file created %s"
/*------------*/
