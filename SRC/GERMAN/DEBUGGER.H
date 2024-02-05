/*   Geneva, Copyright ½ 1993, Gribnif Software   **
**   Strings in debugger (GNVA_DB) version        **
**                                                **
**   When translating, change just the things in  **
**   "quotation marks". Leave all \Escape         **
**   sequences and %formats the way they are!     **
**   I will recheck the lengths of alerts when    **
**   the translated file is returned to me.       **/

#ifndef DEBUG_ON
  #define DEBUGGER(x,y,z)
#else
  #define DEBUGGER(x,y,z)       debug_alert(x,y,z)

  #define DEBUG_STR     "[1][Geneva Debugger:|%s: %s|App: %s ID: %d][[Ignorieren |[šberspringen]"
  #define WNGET         "wind_get"
  #define INVHAND       "ungltiges Handle %d"
  #define UNKTYPE       "unbekannter Typ %d"
  #define WNCREATE      "wind_create"
  #define NOHAND        "kein Handles mehr frei|(%d max.)"
  #define WINDOP        "wind_open"
  #define OPDESK        "Kann Desktop|nicht ”ffnen"
  #define ALROPEN       "Fenster %d|schon offen"
  #define NOTOWN        "Fenster %d|geh”rt dieser app nicht"
  #define WNCLOSE       "wind_close"
  #define CLDESK        "Kann Fenster|nicht schlieen"
  #define NOTOPEN       "Fenster %d|nicht offen"
  #define WNDEL         "wind_delete"
  #define DELDESK       "Kann Desktop|nicht l”schen"
  #define DECLOSE       "Warnung:|Fenster %d nicht geschlossen"
  #define WUPDATE       "wind_update"
  #define WNSET         "wind_set"
  #define SETDESK       "Kann Desktop Attribut %d|nicht setzen"
  #define ALRIC         "Fenster %d|already iconifiziert"
  #define NOTIC         "Fenster %d|ist nicht iconifiziert"
  #define NOTDIAL       "Fenster %d|ist kein Dialogfenster"
  #define XWTRE         "x_wind_tree"
  #define GRFMO         "graf_mouse"
  #define MNUICH        "menu_icheck"
  #define NULLTREE      "null tree"
  #define MNUIEN        "menu_ienable"
  #define MNUTNO        "menu_tnormal"
  #define MNUTEX        "menu_text"
  #define OBDRAW        "objc_draw"
  #define OBDEL         "objc_delete"
  #define OBORD         "objc_order"
  #define DELROOT       "Kann Root"|nicht l”schen"
  #define REOROOT       "Kann Root|nicht ”ffnen"
  #define OBJCED        "objc_edit"
  #define RSLOAD        "rsrc_load"
  #define LOADALL       "Kann Datei nicht|vollst„ndig laden"
  #define NOMEM         "Zu wenig|Speicher"
  #define NOTFOUND      "Datei|nicht gefunden"
  #define RSGA          "rsrc_gaddr"
  #define RSSA          "rsrc_saddr"
  #define SHWR          "shel_write"
  #define SHGE          "shel_get"
  #define SHGEFILE      "Keine NEWDESK.INF|oder DESKTOP.INF Datei"
  #define SHPU          "shel_put"
  #define NOTDONE       "Funktion|nicht implementiert"
  #define XSHGE         "x_shel_get"
  #define SHUSED        "Wird schon von|Proze #%d benutzt"
  #define DISKERR       "Disk Fehler #%d"
  #define XSHPU         "x_shel_put"
  #define XWDIALD       "x_wdial_draw"
  #define XWDIALC       "x_wdial_change"
  #define LALLOC        "lalloc"
  #define BADSIZE       "bad size"
  #define LFREE         "lfree"
  #define NOARENA       "Konnte Arena fr|Block nicht finden"
  #define NOUNLINK      "konnte nicht von der|alloc list unlinken"
  #define BADLINK       "recursive link"
  #define LSHRINK	"lshrink"
  #define NEWGTOLD	"neu > alt"
  #define BADCHK	"Prfsummenfehler"
  #define LREALLOC	"lrealloc"
#endif
