/*   Geneva, Copyright � 1993, Gribnif Software   **
**   Strings in the main program                  **
**                                                **
**   When translating, change just the things in  **
**   "quotation marks". Leave all \Escape         **
**   sequences and %formats the way they are!     **
**   I will recheck the lengths of alerts when    **
**   the translated file is returned to me.       **/

#if defined(_APPLIC)
  #define AES_LANG      1       /* appl_getinfo(3) return */
/*** Keep these next 9 messages to lines of no more than 40 chars each */
  #define NOINST  "\r\nGeneva: Installationsabbruch weil\r\n\
Alternate Taste gedr�ckt wird.\r\n\n"
  #define TWICEMSG  "\a\033E\
Geneva wurde bereits im\r\n\
im AUTO-Ordner gestartet.\r\n"
  #define OSMSG   "\a\033E\
Etwas h�ngt bereits in exec_os.\r\n\
Geneva kann nicht starten.\r\n\
Setzen Sie GENEVA.PRG im AUTO-\r\n\
Ordner weiter nach vorne.\r\n"
  #define JARROOM "\r\n\aGeneva: Zu wenig Platz im Cookie Jar.\r\n\
Erh�hen Sie die Anzahl mit JARxxx\r\n"
  #define NOVDI   "\r\n\aGeneva: Fehler beim �ffnen der VDI\r\n\
workstation. Geneva mu� beendet werden\r\n"
  #define ANOMEM  "\033E\aGeneva: Zu wenig Speicher"
  #define NOJAR   "\033E\aJARxxx mu� vor Geneva laufen.\r\n"
#ifndef DEBUG_ON
  #define COPYRTM "\r\n\
\t     Geneva\r\n\
\t von Dan Wilga\r\n\
      Release %03x %s\r\n\
Copyright \xBD 2017, Gribnif Software.\r\n\
     Alle Rechte vorbehalten.\r\n"
#else
  #define COPYRTM "\r\n\
      Geneva Debug Version\r\n\
\t von Dan Wilga\r\n\
      Release %03x %s\r\n\
Copyright \xBD 2017, Gribnif Software.\r\n\
     Alle Rechte vorbehalten.\r\n"
#endif
  #define DFLTSTR "Default Flags"
  #define ASCII_TITLE "ASCII Tabelle"
  #define SHWOPSTR "W�hlen Sie das zu startende Programm"
  #define SHWOPEN "[1][Konnte Anwendung %s|nicht �ffnen][[Ok]"
  #define SHWEXIT "[3][Proze� %s mit|Status #%d verlassen|(%%s)][Ok]"
  #define SHWUNDF "undefinierter Fehler"
  #define SHWUNKN "<Unbekannt>"
  #define SHHELP  "Geneva"
  #define SHNOHELP "[1][Konnte Hilfesystem|nicht laden][Ok]"
  #define GENMANAG "  Geneva\xBF Manager"
  #define GENMANAG2 "Geneva\xBF Manager"
  #undef _APPLIC

#elif defined(_XTRAS)
  #define XTERM   "[3][Terminierung dieser Anwendung|kann Probleme \
verursachen|falls Systemvektoren|ge�ndert wurden!][[Trotzdem|[Abbruch]"
  #define XTERM2  "[3][Soll %s wirklich|terminiert werden?][[Ja|[Abbruch]"
  #define BADSET  "[1][Inkompatible|GENEVA.CNF Version][[Abbruch]"
  #define TOGL2MT "[1][Die Anwendung wird dadurch|\
schlafen gelegt.][[Weiter|[Abbruch]"
  #define TOGL2ST "[1][Alle Multitasking Anwendungen|\
werden dadurch schlafen|gelegt.][[Weiter|[Abbruch]"
  #undef _XTRAS

#elif defined(_MENUS)
  #define APPLIC "Anwendungen"
  #undef _MENUS

#elif defined(_FORMS)
  #define ALERR    "1][|ERROR IN ALERT FORMAT!][[1|[2|[3]"
  #define undo_ptr "ABBRUCH\0\0ENDE\0\0AUSGANG\0\0NEIN\0\0CANCEL\0\0"
  char *crit_str[] = {
  "[1][Die Diskette in Laufwerk %c:|ist schreibgesch�tzt.][[Nochmal|[Abbruch]",
  "[2][Laufwerk %c: antwortet nicht.|Bitte Laufwerk pr�fen|\
oder Diskette einlegen.][[Nochmal|[Abbruch]",
  "[1][Daten der Diskette im|Laufwerk %c: k�nnten defekt|sein.][[Nochmal|[Abbruch]",
  "[2][Die Anwendung kann Daten auf|der Diskette in Laufwerk %c:|nicht lesen.]\
[[Nochmal|[Abbruch]",
  "[1][Das Ausgabeger�t empf�ngt|keine Daten.][[Nochmal|[Abbruch]",
  "[3][Bitte Diskette %c:|in Laufwerk A: einlegen][[Ok]" },
       ce_xref[] = { 4, 1, 1, 2, 1, 1, 2, 2, 4, 2, 2, 2, 0, 3, 4, 2, 5 },
       ce_bad[] =  "[3][GEM Fehler.][[Abbruch]",
       ce_invalid[] =
  "[1][TOS is attempting to access an|invalid drive. You should save|\
any open documents and reboot|the computer.][[Ok]", /* 005 */
       *fe_str[] = {
  "Die Anwendung findet den|Ordner nicht auf den|zugegriffen werden sollte.",
  "Die Anwendung kann keine|weitere Datei �ffnen.|\
Schlie�en Sie eine nicht|mehr ben�tigte Datei.",
  "Ein Objekt dieses Namens|existiert bereits in diesem|Ordner, oder das Objekt|\
ist schreibgesch�tzt.",
  "F�r diese Anwendung steht zu|wenig Speicher zur Verf�gung.",
  "Dieses Laufwerk scheint|nicht zu existieren" },
       fe_xref[] = { -1, -1, 0, 0, 1, 2, -1, -1, 3, -1, 3, 3, -1, -1, -1, 4,
                     -1, -1, 0 },
       fe_tmpl[] = "[1][%s][[Abbruch]",
       fe_dflt[] =  "TOS Fehler #%d.",
       bad_call[] = "[3][Illegaler AES|Funktionsaufruf $%x][[Abbruch]";
  typedef struct { int num; char *string; } ERRSTRUC;
  ERRSTRUC dflt_errors[] = {
   { IERROR, "Ein Programm wurde abnormal|beendet, oder es gab einen|\
Laufwerkslesefehler" },
   { IEDRVNR, "Laufwerk nicht bereit!|Bitte Diskette einlegen" },
   { IEUNCMD, "Unbekannter Funktionsaufruf" },
   { IEPAPER, "Das Ausgabeger�t hat|kein Papier mehr" },
   { IEWRITF, "Ein Schreibfehler|ist aufgetreten" },
   { IEREADF, "Ein Lesefehler|ist aufgetreten" },
   { IEWRPRO, "Dieses Laufwerk|ist schreibgesch�tzt" },
   { IE_CHNG, "Die urspr�ngliche Diskette|wurde entfernt" },
   { IEUNDEV, "Dieses Ger�t|existiert nicht" },
   { IEBADSF, "Die Diskette hat einen oder|mehrere fehlerhafte Sektoren" },
   { IEINVFN, "Control-C wurde gedr�ckt|oder eine ung�ltige Funktion|ist aufgetreten" },
   { IEFILNF, "Eine Datei solchen|Namens existiert nicht." },
   { IEPTHNF, "Ein Ordner solchen|Namens existiert nicht." },
   { IENHNDL, "Zuviele Datei Dateien|sind bereits offen" },
   { IEACCDN, "Zugriff verweigert. Dieses|Objekt ist eine schreibge-|sch�tzte Datei oder|\
ein nicht-leerer Ordner" },
   { IENSMEM, "Es gibt zu wenig Speicher|f�r diesen Vorgang" },
   { IEIMBA,  "Ung�ltige Speicher-|bereichs Addresse" },
   { IEDRIVE, "Dieses Laufwerk scheint|nicht zu existieren" },
   { IEPLFMT, "Das scheint kein ausf�hrbares|Program zu sein" },
     0 };
  #undef _FORMS

#elif defined(_FSELECT)
  #define FSNOMEM1 "[1][Zu wenig Speicher f�r|den Fileselektor][[Ok]"
  #define FSNOMEM2 "[1][Zu wenig Speicher f�r|diesen Ordner.][[Ok]"
  #define FSNOMEM3 "[1][Zu wenig Speicher zum|kopieren so vieler Dateien.][[Ok]"
  #define FS_NSDT  "ngdh"       /* Name, Size, Date, Time */
  #define FSBADREN "[1][Konnte nicht in|%s umbennenen!|(%%s)][[Ok]"
  #define FSBADNEW "[1][Konnte Ordner %s|nicht erzeugen!|(%%s)][[Ok]"
  #define FSBADRM  "[1][Konnte %s|nicht l�schen!|(%%s)][[Ok]"
  #define FSGENERR "[1][Disklesefehler!|(%%s)][[Ok]"
  #define FSDTITL  "Fileselektor"
  #define FSFR     "[0][%9D Byte insg.|%9D Byte frei][[Ok]"
  #define FSEXTMSG "Shift->Setzen, Control->hinzuf�gen"
  #define FSPTHMSG "Shift f�gt aktuellen Pfad ein"
  #define FSELONG  "[1][Diese Extension ist|l�nger als erlaubt][[Ok]"
  #define FSPLONG  "[1][Die Pfadangabe|ist zu lang.][[Abbruch]"
  #define FSMATCH  "[3][|Keine passende gefunden][OK]"
  #define FSCOL1   "[1 COL"
  #define FSCOL2   "[2 COL"
  #define FSINFO1  "1 OBJ. IN"
  #define FSINFO1P "%d OBJ. IN"
  #define FSINFO2  "1 BYTE"
  #define FSINFO2P "%D BYTE"
  #define FSOKS    "[Ok"
  #define FSDEL    "[L�schen"
  #define FSFINFO  "Datei Informationen"
  #define FSFOINFO "Ordner Informationen"
  #define FSFIDEL  "Datei l�schen"
  #define FSFODEL  "Ordner l�schen"
  #define FSCOPY   "Zielpfad w�hlen und OK anklicken"
  #define FSCOPYING "Kopiere Objekte..."
  #define FSMOVING "Verschiebe Objekte..."
  #define FSMOVE   "[1][Konnte %s|nicht verschieben!|(%%s)][[Ok]"
  #define FSOPEN   "[1][Konnte %s|nicht �ffnen!|(%%s)][[Ok]"
  #define FSWRITE  "[1][Fehler beim schreiben|in %s!|(%%s)][[Ok]"
  #define FSREAD   "[1][Fehler beim lesen|von %s!|(%%s)][[Ok]"
  #define FSCREAT  "[1][Fehler beim erzeugen|von %s!|(%%s)][[Ok]"
  #define FSFULL   "[1][Dieses Laufwerk|ist voll!][[Ok]"
  #define FSRBYTES "%D Bytes"
  #undef _FSELECT

#elif defined(_SHELL)
  ##ifdef DEBUG
  int vdi_hand;
  int work_in[] = { 1, 7, 1, 1, 1, 1, 1, 1, 1, 1, 2 }, work_out[57],
      contrl[12], intin[50], intout[10], ptsin[50], ptsout[50];
  int char_w=8, char_h=16, ptsiz=10;
#endif

void v_ftext16( int handle, int x, int y, int *wstr, int strlen );
void vqt_f_extent16( int handle, int *wstr, int strlen, int *extent );

static int RSBB0DATA[] =
{ 0x03C0, 0x0C30, 0x1008, 0x2004,
  0x2004, 0x4002, 0x4002, 0x4002,
  0x4002, 0x2004, 0x2004, 0x1008,
  0x0C30, 0x03C0, 0x0000
};

static int RSBB1DATA[] =
{ 0x03C0, 0x0C30, 0x1008, 0x23C4,
  0x27E4, 0x4FF2, 0x4FF2, 0x4FF2,
  0x4FF2, 0x27E4, 0x23C4, 0x1008,
  0x0C30, 0x03C0, 0x0000
};

static int RSBB2DATA[] =
{ 0xFFFF, 0x8001, 0x8001, 0x8001,
  0x8001, 0x8001, 0x8001, 0x8001,
  0x8001, 0x8001, 0x8001, 0x8001,