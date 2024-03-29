/*   Geneva, Copyright � 1993, Gribnif Software   **
**   Strings in MAKEHELP.TTP                      **
**                                                **
**   When translating, change just the things in  **
**   "quotation marks". Leave all \Escape         **
**   sequences and %formats the way they are!     **
**   I will recheck the lengths of alerts when    **
**   the translated file is returned to me.       **/

/* If necessary you can include: */
/* "German translation by Michael Nolte, Copyright � 1994-1995" */
/* "Deutsche �bersetzung von Michael Nolte, Copyright � 1994-1995" */

#define INDEX_TITLE     "Doppelklicken Sie auf einen der Indexeintr�ge:"
#define DUP_KEY         "%6ld: Ein Schl�sselwort dieses Namens existiert schon."
#define NO_MEM          "%6ld: Zu wenig Speicher."
#define EXTERNAL        "%6ld Warnung: Schl�sselwort \"%s\" nicht gefunden\n"
#define UNX_EOF         "Unerwartetes Dateiende"
#define SYNTAX          "%6ld: Syntax Fehler"
#define SEEK_ERR        "Seek Error in der Ausgabedatei."
#define OPENERR         "Fehler beim �ffnen von %s"
#define READERR         "Fehler beim lesen von %s"
#define FILEMEM         "Nicht genug freier Speicher f�r die ganze Datei."
#define PASS1           "Abschnitt 1: Suche nach Anzeigeseiten...\n"
#define LONGKEY         "%6ld: Schl�sselwort zu lang."
#define SPLITKEY        "%6ld: Schl�sselwort kann nicht getrennt sein."
#define MAXLINES        "%6ld: Maximal %d Zeilen pro Anzeigeseite erlaubt."
#define UNX_END         "%6ld: Unerwartete \"\\end\" Anweisung."
#define MIS_END         "%6ld: Fehlendes \"end\" im Textbereich nach Zeile %ld"
#define MAXLEN          "%6ld: Schl�sselwort zu lang."
#define COPYRT          "Copyright"
#define PASS2           "\nAbschnitt 2: Suche nach Schl�sselw�rtern...\n"
#define PASS3           "\nAbschnitt 3: Erstelle Index...\n"
#define PASS3A          "\nAbschnitt 3: Suche und markiere neue Referenzen...\n"
#define GOMULT          "Warnung: Index zu gro�. Benutzung mehrfacher Anzeigeseiten.\n"
#define INDLTR          "Fatal: Zu viele Index Eintr�ge f�r den Buchstaben %c"
#define INDOTH          "Fatal: Zu viele Index Eintr�ge f�r die Kategorie \"Andere\"."
#define OTHER           "Andere"
#define OTHER2          "    \x1dAndere..\x1d"
#define INDXMEM         "Zu wenig Speicher f�r Index."
#define PASS4           "\nAbschnitt 4: Schreibe komprimierte Datei...\n\n"
#define OUTCREAT        "Konnte %s nicht erzeugen."
#define XTRAOPEN        "Fehler beim �ffnen zus�tzlicher Dateihandles"
#define WRITERR         "\nFehler beim schreiben in Ausgabedatei."
#define WRITFULL        "\nFehler beim schreiben in Ausgabedatei. Laufwerk voll."
#define BADFUNC         "%6ld: Fehlerhafter Funktionsname %s"
#define FUNCNUM         "%6ld: Zahl erwartet."
#define FUNCPARN        "%6ld: Funktion \"%s\" erfordert mindestens %d Zahlparameter."
#define FUNCPARS        "%6ld: Funktion \"%s\" erfordert mindestens %d Stringparameter."
#define FUNCNBIG        "%6ld: Zahl zu gro� (>255)."
#define TOOMANY         "%6ld: Funktion ben�tigt zu viel Speicher."
#define FUNCNIMG        "%6ld: Zu viele Bilder."
#define BADEXT		"%6ld Fatal: Schl�sselwort {%s} nicht gefunden\n"
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
#define MAINTITL        "\
   Geneva Makehelp Utility 1.2\n\
Copyright � 1996, Gribnif Software\n\n"
/*------------*/
#define USAGE           "\
Benutzung: makehelp [-m] [-n<Zeilen>] <Quelle> <Hilfedatei>\r\n\
       -m: Erzwinge mehrfach-Anzeigeseite Index\r\n\
       -n: Maximal Zeilen pro Anzeigeseite\r\n\
           (default=200)\r\n\
                        oder\r\n\
           makehelp -s <Quelle> <neue Quelle>\r\n\
       -s: Suche Schl�sselw�rter im Text und erzeuge\r\n\
           neue Quelldatei\r\n"
/*--------memcpy
get_formxyGetrez
do_iconifycurrxyatoiwg_touchwo_touchset_sepsampstrncmp
rsrc_obfixpathmo_touchgad_tmpfv_touchfo_touchfk_touchdo_helpappwg_newnew_flagDget