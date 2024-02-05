/*   Geneva, Copyright � 1993, Gribnif Software   **
**   Strings in the main program                  **
**                                                **
**   When translating, change just the things in  **
**   "quotation marks". Leave all \Escape         **
**   sequences and %formats the way they are!     **
**   I will recheck the lengths of alerts when    **
**   the translated file is returned to me.       **/

#if defined(_APPLIC)
  #define AES_LANG      0       /* appl_getinfo(3) return */
/*** Keep these next 9 messages to lines of no more than 40 chars each */
  #define NOINST  "\r\nGeneva: Installation aborted due\r\n\
to Alternate key being held.\r\n\n"
  #define TWICEMSG  "\a\033E\
Geneva is already installed\r\n\
from the AUTO folder.\r\n"
  #define OSMSG   "\a\033E\
Something is already hooked into\r\n\
exec_os. Cannot install Geneva.\r\n\
Try moving GENEVA.PRG earlier\r\n\
in the AUTO folder.\r\n"
  #define JARROOM "\r\n\aGeneva: No more room in Cookie Jar.\r\n\
Use a higher number in JARxxx.\r\n"
  #define NOVDI   "\r\n\aGeneva: error opening VDI\r\n\
workstation. Geneva must terminate.\r\n"
  #define ANOMEM  "\033E\aGeneva: Out of memory"
  #define NOJAR   "\033E\aYou must run JARxxx before Geneva.\r\n"
#ifndef DEBUG_ON
  #define COPYRTM "\r\n\
\t     Geneva\r\n\
\t  by Dan Wilga\r\n\
      Version %d.%02d %s\r\n\
Copyright \xBD 2017, Gribnif Software.\r\n\
       All rights reserved.\r\n"
#else
  #define COPYRTM "\r\n\
      Geneva Debug Version\r\n\
\t  by Dan Wilga\r\n\
      Version %d.%02d %s\r\n\
Copyright \xBD 2017, Gribnif Software.\r\n\
       All rights reserved.\r\n"
#endif
  #define DFLTSTR "Default Flags"
  #define ASCII_TITLE "ASCII Table"
  #define SHWOPSTR "Application to Open"
  #define SHWOPEN "[1][Application %s|could not be opened][[Ok]"
  #define SHWEXIT "[3][Process %s|exited with status #%d|(%%s)][Ok]"
  #define SHWUNDF "undefined error"
  #define SHWUNKN "<Unknown>"
  #define SHHELP  "Geneva"
  #define SHNOHELP "[1][The help text viewer|could not be loaded][Ok]"
  #define GENMANAG "  Geneva\xBF Manager"
  #define GENMANAG2 "Geneva\xBF Manager"
  #undef _APPLIC

#elif defined(_XTRAS)
  #define XTERM   "[3][Terminating this application|can cause problems \
if it has|intercepted system vectors.|Continue anyway?][[Terminate|[Cancel]"
  #define XTERM2  "[3][Are you sure you want|to terminate %s?][[Terminate|[Cancel]"
  #define BADSET  "[1][Incompatible|GENEVA.CNF version][[Cancel]"
  #define TOGL2MT "[1][Taking this action will put|\
the current application|to sleep.][C[ontinue|[Cancel]"
  #define TOGL2ST "[1][Taking this action will put|\
all multitasking applications|to sleep.][C[ontinue|[Cancel]"
  #undef _XTRAS

#elif defined(_MENUS)
  #define APPLIC "Applications"
  #undef _MENUS

#elif defined(_FORMS)
  #define ALERR    "1][|ERROR IN ALERT FORMAT!][[1|[2|[3]"
  #define undo_ptr "CANCEL\0\0QUIT\0\0EXIT\0\0ABORT\0\0NO\0\0"
  char *crit_str[] = {
  "[1][The disk in drive %c: is|physically write-protected.][[Retry|[Cancel]",
  "[2][Drive %c: is not responding.|Please check the disk drive|\
or insert a disk.][[Retry|[Cancel]",
  "[1][Data on the disk in|drive %c: may be damaged.][[Retry|[Cancel]",
  "[2][This application cannot read|data on the disk in drive %c:.]\
[[Retry|[Cancel]",
  "[1][Your output device is not|receiving data.][[Retry|[Cancel]",
  "[3][Please insert disk %c:|into drive A:][[Ok]" },
       ce_xref[] = { 4, 1, 1, 2, 1, 1, 2, 2, 4, 2, 2, 2, 0, 3, 4, 2, 5 },
       ce_bad[] =  "[3][GEM error.][[Cancel]",
       ce_invalid[] =
  "[1][TOS is attempting to access an|invalid drive. You should save|\
any open documents and reboot|the computer.][[Ok]", /* 005 */
       *fe_str[] = {
  "This application cannot find|the folder or file you just|\
tried to access.",
  "This application does not have|room to open another document.|\
To make room, close a document|you no longer need.",
  "An item with this name already|exists in the directory, or|this item \
is set to Read Only|status.",
  "There is not enough memory in|your computer for the|\
application you just tried to|run.",
  "This disk drive does not seem|to exist" },
       fe_xref[] = { -1, -1, 0, 0, 1, 2, -1, -1, 3, -1, 3, 3, -1, -1, -1, 4,
                     -1, -1, 0 },
       fe_tmpl[] = "[1][%s][[Cancel]",
       fe_dflt[] =  "TOS error #%d.",
       bad_call[] = "[3][Illegal AES function|call $%x][[Cancel]";
  typedef struct { int num; char *string; } ERRSTRUC;
  ERRSTRUC dflt_errors[] = {
   { IERROR, "Either a program terminated|abnormally, or there was an|\
error reading this disk" },
   { IEDRVNR, "Drive not ready! Please insert|a disk" },
   { IEUNCMD, "Unknown function call" },
   { IEPAPER, "The output device is out of|paper" },
   { IEWRITF, "An error occurred|when writing" },
   { IEREADF, "An error occurred|when reading" },
   { IEWRPRO, "The disk in this drive is|write protected" },
   { IE_CHNG, "The original disk has been|removed from the drive" },
   { IEUNDEV, "This device does not exist" },
   { IEBADSF, "This disk contains one or more|bad sectors" },
   { IEINVFN, "Either Control-C was pressed|or an invalid function was|encountered" },
   { IEFILNF, "There is no file with this|name" },
   { IEPTHNF, "There is no folder with this|name" },
   { IENHNDL, "There are too many files|already open" },
   { IEACCDN, "Access denied. This item is|either a read-only file or \
a|non-empty folder" },
   { IENSMEM, "There is not enough free|memory for this operation" },
   { IEIMBA,  "Invalid memory block address" },
   { IEDRIVE, "This disk drive does not seem|to exist" },
   { IEPLFMT, "This does not seem to be an|executable program" },
     0 };
  #undef _FORMS

#elif defined(_FSELECT)
  #define FSNOMEM1 "[1][Insufficient memory|for item selector][[Ok]"
  #define FSNOMEM2 "[1][There is not enough free|memory for this directory.][[Ok]"
  #define FSNOMEM3 "[1][There is not enough free|memory to copy or move|this many files.][[Ok]"
  #define FS_NSDT  "nsdt"
  #define FSBADREN "[1][Could not rename|to %s!|(%%s)][[Ok]"
  #define FSBADNEW "[1][Could not create|folder %s!|(%%s)][[Ok]"
  #define FSBADRM  "[1][Could not delete %s!|(%%s)][[Ok]"
  #define FSGENERR "[1][Error reading disk!|(%%s)][[Ok]"
  #define FSDTITL  "Item Selector"
  #define FSFR     "[0][%9D bytes total|%9D bytes free][[Ok]"
  #define FSEXTMSG "Hold Shift to set, Control to add"
  #define FSPTHMSG "Hold Shift to set current in list"
  #define FSELONG  "[1][This extension is too|long to set in the list][[Ok]"
  #define FSPLONG  "[1][The file path|is too long][[Sorry]"
  #define FSMATCH  "[3][|No match found][OK]"
  #define FSCOL1   "[1 COL"
  #define FSCOL2   "[2 COL"
  #define FSINFO1  "1 ITEM IN"
  #define FSINFO1P "%d ITEMS IN"
  #define FSINFO2  "1 BYTE"
  #define FSINFO2P "%D BYTES"
  #define FSOKS    "[Ok"
  #define FSDEL    "[Delete"
  #define FSFINFO  "File information"
  #define FSFOINFO "Folder information"
  #define FSFIDEL  "Delete file"
  #define FSFODEL  "Delete folder"
  #define FSCOPY   "Choose dest. path and click on OK"
  #define FSCOPYING "Copying items..."
  #define FSMOVING "Moving items..."
  #define FSMOVE   "[1][Could not move %s!|(%%s)][[Ok]"
  #define FSOPEN   "[1][Could not open %s!|(%%s)][[Ok]"
  #define FSWRITE  "[1][Error writing to %s!|(%%s)][[Ok]"
  #define FSREAD   "[1][Error reading %s!|(%%s)][[Ok]"
  #define FSCREAT  "[1][Error creating %s!|(%%s)][[Ok]"
  #define FSFULL   "[1][|This disk is full!][[Ok]"
  #define FSRBYTES "%D bytes"
  #undef _FSELECT

#elif defined(_SHELL)
  #define SHUTDOWN "[1][One or more applications|will not know they are|\
being shut down.|Continue anyway?][C[ontinue|[Cancel]"
  #define ENTERST  "[3][This is a singletasking|application. Either it|\
or all other applications|must be put to sleep.][Sleep [All|Sleep [This|[Cancel]"
  #undef _SHELL
#endif

/**  Strings in an assembly module that must be translated 

        dc.b    $7, $1b, "H", "Fatal error in Geneva: ", 0
        dc.b    $d, $a, "Press any key to try to quit...", 0
        dc.b    "GEMDOS called recursively", 0
        dc.b    "AES called recursively", 0
        dc.b    "breakpoint reached. Press B to break.", 0
        dc.b    "exec_shell did not terminate", 0
        dc.b    "run_acc did not terminate", 0
**/
