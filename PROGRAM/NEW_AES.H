/* New AES definitions for MultiTOS */
/* These are missing from Pure C's AES.H file as of 1.1 */

#ifndef __NEW_AES__
  #define __NEW_AES__
  #include "aes.h"

#ifndef WM_ICONIFY	/* evnt_mesag messages */
  #define WM_ICONIFY	34	/* AES 4.1 and newer */
  #define WM_UNICONIFY	35	/* AES 4.1 */
  #define WM_ALLICONIFY	36	/* AES 4.1 */
  #define WM_TOOLBAR	37	/* AES 4.1 */
#endif
#define RESCH_COMPLETED 61	/* typo corrected from AES.H */
#define SH_WDRAW	72

#ifndef appl_getinfo
int appl_getinfo( int ap_gtype, int *ap_gout1, int *ap_gout2, int *ap_gout3,
    int *ap_gout4 );
#endif

#define G_CICON         33	/* ob_type */

#ifndef FL3DMASK	/* ob_flags */
  #define FL3DMASK	0x0600
  #define FL3DNONE	0x0000
  #define FL3DIND	0x0200
  #define FL3DBAK	0x0400
  #define FL3DACT	0x0600
#endif

#if 0
   /* This is the correct definition of MENU (with the mn_keystate element)
      but you will need to remove the current definition from AES.H if
      you have an older version of the header file where it is incorrect. */
  typedef struct
  {
        OBJECT  *mn_tree;
        int     mn_menu;
        int     mn_item;
        int     mn_scroll;
        int     mn_keystate;
  } MENU;
#endif

#ifndef objc_sysvar
  #define LK3DIND	1
  #define LK3DACT	2
  #define INDBUTCOL	3
  #define ACTBUTCOL	4
  #define BACKGRCOL	5
  #define AD3DVALUE	6
  int objc_sysvar( int ob_smode, int ob_swhich, int ob_sival1,
                   int ob_sival2, int *ob_soval1, int *ob_soval2 );
#endif

#ifndef M_SAVE		/* graf_mouse forms */
  #define M_SAVE	258
  #define M_RESTORE	259
  #define M_PREVIOUS	260
#endif

#ifndef GDOS_PROP	/* TEDINFO te_font values */
  #define GDOS_PROP	0
  #define GDOS_MONO	1
  #define GDOS_BITM	2
#endif

#ifndef SMALLER		/* wind_create, wind_calc */
  #define SMALLER	0x4000	/* AES 4.1 and newer: was 0x1000 in Geneva 003 */
#endif

#ifndef WF_BOTTOM	/* wind_get/set */
  #define WF_BOTTOM	25	/* 1/25/93 */
#endif
#ifndef WF_ICONIFY	/* wind_get/set */
  #define WF_ICONIFY	26	/* AES 4.1 and newer */
  #define WF_UNICONIFY	27	/* AES 4.1 */
  #define WF_UNICONIFYXYWH 28	/* AES 4.1 */
  #define WF_TOOLBAR      30	/* AES 4.1 */
  #define WF_FTOOLBAR     31	/* AES 4.1 */
  #define WF_NTOOLBAR     32	/* AES 4.1 */
#endif

/* The following definitions are ones I've added, to make using
   shel_write easier */

/* shel_write sh_wdoex parameter flags in LSB */
#define SHW_RUNANY	0	/* Run and let AES decide mode */
#define SHW_RUNAPP	1	/* Run an application */
#define SHW_RUNACC	3	/* Run a desk accessory */
#define SHW_SHUTDOWN	4	/* Enter shutdown mode */
#define SHW_NEWREZ	5	/* Change resolution */
#define SHW_BROADCAST	7	/* Broadcast to all processes: was 6 prior to Geneva 006 */
#define SHW_ENVIRON	8	/* Modify environment */
#define SHW_MSGTYPE	9	/* What kind of message app can understand */
#define SHW_SENDTOAES	10	/* Send AES a message */

/* shel_write sh_wdoex parameter flags in MSB */
#define SHD_ENVIRON	(1<<11)	/* Environment string */
#define SHD_DFLTDIR	(1<<10)	/* Default directory string */
#define SHD_PRENICE	(1<<9)	/* MiNT Prenice (priority) level */
#define SHD_PSETLIM	(1<<8)	/* MiNT memory allocation limit */

/* shel_write alternative structure for sh_wpcmd parameter */
typedef struct
{
  char *name;
  long psetlimit;
  long prenice;
  char *dflt_dir;
  char *environ;
  long unused[4];
  unsigned long app_flags; /* Geneva 004 and newer, with XSHW_xxxx modes only */
} SHWRCMD;

#endif __NEW_AES__
