/* GEM Resource C Source */

#include <portab.h>
#include <aes.h>
#include "WINDOWS.H"

#if !defined(WHITEBAK)
#define WHITEBAK    0x0040
#endif
#if !defined(DRAW3D)
#define DRAW3D      0x0080
#endif

#define FLAGS9  0x0200
#define FLAGS10 0x0400
#define FLAGS11 0x0800
#define FLAGS12 0x1000
#define FLAGS13 0x2000
#define FLAGS14 0x4000
#define FLAGS15 0x8000
#define STATE8  0x0100
#define STATE9  0x0200
#define STATE10 0x0400
#define STATE11 0x0800
#define STATE12 0x1000
#define STATE13 0x2000
#define STATE14 0x4000
#define STATE15 0x8000

TEDINFO rs_tedinfo[] =
{ "\0",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x11A1, 0, 1, 1, 1,
  "\0",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 1, 1, 1,
  "V.VV MMM 99 1999",
  "\0",
  "\0",
  IBM  , 0, TE_RIGHT, 0x1100, 0, 0, 17, 1,
  "x",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 2, 1,
  "x",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 2, 1,
  "x",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 2, 1,
  "x",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 2, 1,
  "x",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 2, 1,
  "x",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 2, 1,
  "x",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 2, 1,
  "x",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 2, 1,
  "x",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 2, 1,
  "x",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 2, 1,
  "wind_update/blocked \001       dialog \001",
  "\0",
  "\0",
  SMALL, 0, TE_RIGHT, 0x1100, 0, 0, 37, 1,
  "owns mouse \001        timer \001",
  "\0",
  "\0",
  SMALL, 0, TE_RIGHT, 0x1100, 0, 0, 28, 1,
  "has/owns menu bar \001      message \001",
  "\0",
  "\0",
  SMALL, 0, TE_RIGHT, 0x1100, 0, 0, 35, 1,
  "has window/top \001      mouse 2 \001",
  "\0",
  "\0",
  SMALL, 0, TE_RIGHT, 0x1100, 0, 0, 32, 1,
  "mouse 1 \001",
  "\0",
  "\0",
  SMALL, 0, TE_RIGHT, 0x1100, 0, 0, 10, 1,
  "[RESET wind_update",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 19, 1,
  "button \001",
  "\0",
  "\0",
  SMALL, 0, TE_RIGHT, 0x1100, 0, 0, 9, 1,
  "keyboard \001",
  "\0",
  "\0",
  SMALL, 0, TE_RIGHT, 0x1100, 0, 0, 11, 1
};

WORD RSBB0DATA[] =
{ 0x0000, 0x03FF, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x3FFF, 0xF000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0001, 0xFFFF, 0xFC00, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0007, 0xFFFF, 0xFC00, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x000F, 0xF007, 0xF800, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x003F, 0x8000, 0xF800, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x007E, 0x0000, 0x3800, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x00FC, 0x0000, 0x3800, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x01F8, 0x0000, 0x3000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x03F8, 0x0000, 0x3000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x07F0, 0x0000, 0x3000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0FE0, 0x0000, 0x0000, 0x00FC, 
  0x001E, 0x0038, 0x0000, 0xFC00, 
  0xFFF8, 0x3800, 0x00FE, 0x1800, 
  0x0FE0, 0x0000, 0x0000, 0x03FF, 
  0x007E, 0x01FC, 0x0003, 0xFF03, 
  0xFFF8, 0x7C00, 0x03FF, 0xFBB4, 
  0x1FE0, 0x0000, 0x0000, 0x0FFF, 
  0x01FE, 0x03FC, 0x000F, 0xFF07, 
  0xFFF0, 0xFE00, 0x07FF, 0xF13C, 
  0x1FC0, 0x0000, 0x0000, 0x3E3F, 
  0x83FE, 0x07FC, 0x003E, 0x3F87, 
  0xFFE0, 0xFE00, 0x1E0F, 0xF124, 
  0x3FC0, 0x0000, 0x0000, 0x781F, 
  0x87FE, 0x0FFC, 0x0078, 0x1F86, 
  0x01E1, 0xFE00, 0x3C07, 0xF124, 
  0x3FC0, 0x0000, 0x0000, 0xF81F, 
  0x9CFC, 0x19FC, 0x00F8, 0x1F8C, 
  0x01C1, 0x3E00, 0x7807, 0xF000, 
  0x3FC0, 0x0000, 0x0001, 0xF01F, 
  0x98FC, 0x21F8, 0x01F0, 0x1F8C, 
  0x0380, 0x1E01, 0xF807, 0xE000, 
  0x7F80, 0x0000, 0x0003, 0xF01F, 
  0x00FC, 0x41F8, 0x03F0, 0x1F08, 
  0x0780, 0x1E03, 0xF007, 0xE000, 
  0x7F80, 0x0000, 0x0007, 0xE03F, 
  0x00FC, 0x81F8, 0x07E0, 0x3F00, 
  0x0F00, 0x1E03, 0xF007, 0xE000, 
  0x7F80, 0x0000, 0x0007, 0xE03E, 
  0x00F9, 0x83F8, 0x07E0, 0x3E00, 
  0x1F00, 0x1C07, 0xE00F, 0xE000, 
  0x7F80, 0x0000, 0x000F, 0xE07E, 
  0x01FB, 0x03F0, 0x0FE0, 0x7E00, 
  0x3F00, 0x1C07, 0xE00F, 0xC000, 
  0x7F80, 0x03FF, 0xFE0F, 0xC0FC, 
  0x01FA, 0x03F0, 0x0FC0, 0xFC00, 
  0x3E00, 0x1C0F, 0xE01F, 0xC000, 
  0x7F80, 0x03FF, 0xFE0F, 0xC1F0, 
  0x01FE, 0x03F0, 0x0FC1, 0xF000, 
  0x7E00, 0x3C0F, 0xE01F, 0xC000, 
  0x7F80, 0x000F, 0xF01F, 0xC7C0, 
  0x01FC, 0x07E0, 0x1FC7, 0xC000, 
  0xFE00, 0x380F, 0xC03F, 0xC000, 
  0x7F80, 0x000F, 0xE01F, 0xFF00, 
  0x01FC, 0x07E0, 0x1FFF, 0x0000, 
  0xFC00, 0x381F, 0xC03F, 0x8000, 
  0x7F80, 0x000F, 0xE01F, 0xF000, 
  0x03F8, 0x07E0, 0x1FF0, 0x0000, 
  0xFC00, 0x701F, 0xC07F, 0x8000, 
  0x7FC0, 0x000F, 0xE03F, 0x8000, 
  0x03F8, 0x0FE0, 0x3F80, 0x0001, 
  0xFC00, 0x701F, 0xC0DF, 0x8000, 
  0x3FC0, 0x000F, 0xE03F, 0x8000, 
  0x03F0, 0x0FC0, 0x3F80, 0x0001, 
  0xFC00, 0x601F, 0x809F, 0x8000, 
  0x3FC0, 0x001F, 0xC03F, 0x8000, 
  0x03F0, 0x0FC0, 0x3F80, 0x0001, 
  0xFC00, 0xE03F, 0x81BF, 0x0000, 
  0x3FE0, 0x001F, 0xC03F, 0x8000, 
  0x07F0, 0x0FC0, 0x3F80, 0x0001, 
  0xFC00, 0xC03F, 0x833F, 0x0000, 
  0x1FE0, 0x001F, 0xC03F, 0x8002, 
  0x07E0, 0x1FC2, 0x3F80, 0x0201, 
  0xFC01, 0x803F, 0x863F, 0x0000, 
  0x0FF0, 0x001F, 0xC03F, 0x8006, 
  0x07E0, 0x1FC6, 0x3F80, 0x0601, 
  0xFC03, 0x003F, 0x8C3F, 0x1800, 
  0x07FC, 0x003F, 0x803F, 0xC01C, 
  0x0FE0, 0x1FCE, 0x3FC0, 0x1C01, 
  0xFE07, 0x003F, 0x987F, 0x3800, 
  0x03FF, 0x00FF, 0x803F, 0xE0F8, 
  0x0FE0, 0x1FF8, 0x3FE0, 0xF801, 
  0xFF0E, 0x003F, 0xF87F, 0xF000, 
  0x01FF, 0xFFFF, 0x001F, 0xFFE0, 
  0x0FC0, 0x1FF0, 0x1FFF, 0xE000, 
  0xFFFC, 0x003F, 0xF07F, 0xC000, 
  0x007F, 0xFFF0, 0x001F, 0xFF80, 
  0x0FC0, 0x1FC0, 0x1FFF, 0x8000, 
  0x7FF0, 0x003F, 0xC07F, 0x8000, 
  0x000F, 0xFE00, 0x000F, 0xFE00, 
  0x1FC0, 0x1F80, 0x0FFE, 0x0000, 
  0x7FE0, 0x001F, 0x807E, 0x0000, 
  0x0000, 0x0000, 0x0003, 0xF000, 
  0x1C00, 0x1E00, 0x03F0, 0x0000, 
  0x1F00, 0x000E, 0x0038, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0008, 0x0000, 0x0000, 
  0x8000, 0x0000, 0x0000, 0x0000, 
  0x0800, 0x0000, 0x0000, 0x0000, 
  0x780F, 0x0018, 0x0000, 0x0001, 
  0x8000, 0x0000, 0x0080, 0x0000, 
  0x1800, 0x0000, 0x0000, 0x0000, 
  0x180C, 0x0008, 0x4108, 0x0000, 
  0x8040, 0x0000, 0x0080, 0x0000, 
  0x0840, 0x0002, 0x0800, 0x0000, 
  0x1414, 0x0008, 0x4008, 0x0000, 
  0x8000, 0x0000, 0x0140, 0x0000, 
  0x0800, 0x0002, 0x0000, 0x0000, 
  0x1414, 0x0008, 0x4008, 0x0000, 
  0x8000, 0x0000, 0x0140, 0x0000, 
  0x0800, 0x0002, 0x0000, 0x0000, 
  0x1414, 0x2108, 0xF11E, 0x7870, 
  0x8C42, 0xE1E0, 0x0220, 0xBC2F, 
  0x0843, 0xC3C7, 0x883C, 0x2E00, 
  0x1224, 0x6308, 0x4308, 0x8488, 
  0x90C7, 0x121C, 0x0221, 0xC671, 
  0x88C4, 0x2422, 0x1846, 0x7100, 
  0x1224, 0x2108, 0x4108, 0x3C80, 
  0xA042, 0x1210, 0x07F0, 0x8220, 
  0x8844, 0x21E2, 0x0882, 0x2100, 
  0x1144, 0x2108, 0x4108, 0x4470, 
  0xE042, 0x1210, 0x0410, 0x8220, 
  0x8844, 0x0222, 0x0882, 0x2100, 
  0x1144, 0x2108, 0x4108, 0x8408, 
  0x9042, 0x11E0, 0x0808, 0x8220, 
  0x8844, 0x0422, 0x0882, 0x2100, 
  0x1084, 0x2308, 0x4108, 0x8C88, 
  0x8842, 0x1100, 0x0808, 0xC431, 
  0x0844, 0x2462, 0x08C4, 0x2100, 
  0x7C9F, 0x1D9C, 0x3386, 0x7671, 
  0xC4E7, 0x19F8, 0x1C1C, 0xB82E, 
  0x1CE3, 0xC3B1, 0x9C78, 0x7180, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0208, 0x0000, 0x8020, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0208, 0x0000, 0x8020, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0210, 0x0000, 0x8020, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x01E0, 0x0001, 0xC070, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x000F, 0xF800, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0002, 0x0800, 
  0x0020, 0x0000, 0x0000, 0x0000, 
  0x0200, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0002, 0x0800, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0200, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0002, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0200, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0002, 0x20B8, 
  0xEE21, 0x63C2, 0xE173, 0x8785, 
  0xC780, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0003, 0xE1C4, 
  0x4463, 0xA467, 0x138C, 0x484E, 
  0x2200, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0002, 0x2084, 
  0x4421, 0x0822, 0x1108, 0x4844, 
  0x2200, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0002, 0x2084, 
  0x2821, 0x0822, 0x1108, 0x4FC4, 
  0x2200, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0002, 0x0884, 
  0x2821, 0x0822, 0x1108, 0x4804, 
  0x2200, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0002, 0x0884, 
  0x1021, 0x0C42, 0x1108, 0x4844, 
  0x2200, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x000F, 0xF9C6, 
  0x1073, 0x8787, 0x1B9C, 0x678E, 
  0x3180, 0x0000, 0x0000, 0x0000
};

BITBLK rs_bitblk[] =
{ RSBB0DATA,  24,  74,   0,   0, 0x0001
};

OBJECT rs_object[] =
{ 
  /******** Tree 0 DESK ****************************************************/
        -1,       -1,       -1, G_BOX             ,   /* Object 0  */
  LASTOB, NORMAL, (LONG)0x00001143L,
  0x0000, 0x0000, 0x0034, 0x0009,
  
  /******** Tree 1 WIND ****************************************************/
        -1, WCLOSE          , WSIZE           , G_IBOX            ,   /* Object 0  */
  NONE, SHADOWED, (LONG)0x00011100L,
  0x0000, 0x0000, 0x0234, 0x0C12,
  WMOVE           ,       -1,       -1, G_BOXCHAR         ,   /* Object 1 WCLOSE */
  SELECTABLE, DRAW3D|STATE11|STATE12|STATE15, (LONG) ((LONG)'\005' << 24)|0x00011100L,
  0x0000, 0x0000, 0x0302, 0x0301,
  WICONIZ         ,       -1,       -1, G_BOXTEXT         ,   /* Object 2 WMOVE */
  NONE, NORMAL, (LONG)&rs_tedinfo[0],
  0x0202, 0x0000, 0x042B, 0x0301,
  WBACK           ,       -1,       -1, G_BOXCHAR         ,   /* Object 3 WICONIZ */
  SELECTABLE, DRAW3D|STATE11|STATE12|STATE15, (LONG) ((LONG)'\177' << 24)|0x00011100L,
  0x042D, 0x0000, 0x0302, 0x0301,
  WFULL           ,       -1,       -1, G_BOXCHAR         ,   /* Object 4 WBACK */
  SELECTABLE, DRAW3D|STATE11|STATE12|STATE15, (LONG) ((LONG)'\256' << 24)|0x00011100L,
  0x052F, 0x0000, 0x0302, 0x0301,
  WILEFT          ,       -1,       -1, G_BOXCHAR         ,   /* Object 5 WFULL */
  SELECTABLE, DRAW3D|STATE11|STATE12|STATE15, (LONG) ((LONG)'\007' << 24)|0x00011100L,
  0x0731, 0x0000, 0x0302, 0x0301,
  WINFO           ,       -1,       -1, G_BOXCHAR         ,   /* Object 6 WILEFT */
  SELECTABLE|TOUCHEXIT, DRAW3D|STATE11|STATE12|STATE15, (LONG) ((LONG)'\004' << 24)|0x00011100L,
  0x0000, 0x0201, 0x0302, 0x0301,
  WIRT            ,       -1,       -1, G_BOXTEXT         ,   /* Object 7 WINFO */
  NONE, NORMAL, (LONG)&rs_tedinfo[1],
  0x0202, 0x0201, 0x062F, 0x0301,
  WTOOLBOX        ,       -1,       -1, G_BOXCHAR         ,   /* Object 8 WIRT */
  SELECTABLE|TOUCHEXIT, DRAW3D|STATE11|STATE12|STATE15, (LONG) ((LONG)'\003' << 24)|0x00011100L,
  0x0731, 0x0201, 0x0302, 0x0301,
  WMNLEFT         ,       -1,       -1, G_IBOX            ,   /* Object 9 WTOOLBOX */
  NONE, NORMAL, (LONG)0x00011100L,
  0x0000, 0x0402, 0x0234, 0x0301,
  WMENU           ,       -1,       -1, G_BOXCHAR         ,   /* Object 10 WMNLEFT */
  SELECTABLE|TOUCHEXIT, DRAW3D|STATE11|STATE12|STATE15, (LONG) ((LONG)'\004' << 24)|0x00011100L,
  0x0000, 0x0603, 0x0302, 0x0301,
  WMNRT           ,       -1,       -1, G_BOX             ,   /* Object 11 WMENU */
  NONE, NORMAL, (LONG)0x00011170L,
  0x0202, 0x0603, 0x062F, 0x0301,
  WUP             ,       -1,       -1, G_BOXCHAR         ,   /* Object 12 WMNRT */
  SELECTABLE|TOUCHEXIT, DRAW3D|STATE11|STATE12|STATE15, (LONG) ((LONG)'\003' << 24)|0x00011100L,
  0x0631, 0x0603, 0x0302, 0x0301,
  WVBIGSL         ,       -1,       -1, G_BOXCHAR         ,   /* Object 13 WUP */
  SELECTABLE|TOUCHEXIT, DRAW3D|STATE11|STATE12|STATE15, (LONG) ((LONG)'\001' << 24)|0x00011100L,
  0x0631, 0x0804, 0x0302, 0x0301,
  WDOWN           , WVSMLSL         , WVSMLSL         , G_BOX             ,   /* Object 14 WVBIGSL */
  NONE, NORMAL, (LONG)0x00011111L,
  0x0631, 0x0A05, 0x0302, 0x0204,
  WVBIGSL         ,       -1,       -1, G_BOX             ,   /* Object 15 WVSMLSL */
  NONE, DRAW3D|STATE12|STATE15, (LONG)0x00011101L,
  0x0000, 0x0401, 0x0302, 0x0602,
  WVSPLIT         ,       -1,       -1, G_BOXCHAR         ,   /* Object 16 WDOWN */
  SELECTABLE|TOUCHEXIT, DRAW3D|STATE11|STATE12|STATE15, (LONG) ((LONG)'\002' << 24)|0x00011100L,
  0x0631, 0x0B09, 0x0302, 0x0301,
  WUP2            ,       -1,       -1, G_BOX             ,   /* Object 17 WVSPLIT */
  NONE, NORMAL, (LONG)0x00011100L,
  0x0000, 0x0F0A, 0x0234, 0x0600,
  WVBIGSL2        ,       -1,       -1, G_BOXCHAR         ,   /* Object 18 WUP2 */
  SELECTABLE|TOUCHEXIT, DRAW3D|STATE11|STATE12|STATE15, (LONG) ((LONG)'\001' << 24)|0x00011100L,
  0x0631, 0x040B, 0x0302, 0x0301,
  WDOWN2          , WVSMLSL2        , WVSMLSL2        , G_BOX             ,   /* Object 19 WVBIGSL2 */
  NONE, NORMAL, (LONG)0x00011111L,
  0x0631, 0x060C, 0x0302, 0x0204,
  WVBIGSL2        ,       -1,       -1, G_BOX             ,   /* Object 20 WVSMLSL2 */
  NONE, DRAW3D|STATE12|STATE15, (LONG)0x00011101L,
  0x0000, 0x0401, 0x0302, 0x0602,
  WLEFT           ,       -1,       -1, G_BOXCHAR         ,   /* Object 21 WDOWN2 */
  SELECTABLE|TOUCHEXIT, DRAW3D|STATE11|STATE12|STATE15, (LONG) ((LONG)'\002' << 24)|0x00011100L,
  0x0631, 0x0710, 0x0302, 0x0301,
  WHBIGSL         ,       -1,       -1, G_BOXCHAR         ,   /* Object 22 WLEFT */
  SELECTABLE|TOUCHEXIT, DRAW3D|STATE11|STATE12|STATE15, (LONG) ((LONG)'\004' << 24)|0x00011100L,
  0x0000, 0x0911, 0x0302, 0x0301,
  WRT             , WHSMLSL         , WHSMLSL         , G_BOX             ,   /* Object 23 WHBIGSL */
  NONE, NORMAL, (LONG)0x00011111L,
  0x0202, 0x0911, 0x0715, 0x0301,
  WHBIGSL         ,       -1,       -1, G_BOX             ,   /* Object 24 WHSMLSL */
  NONE, DRAW3D|STATE12|STATE15, (LONG)0x00011101L,
  0x0603, 0x0000, 0x0306, 0x0301,
  WHSPLIT         ,       -1,       -1, G_BOXCHAR         ,   /* Object 25 WRT */
  SELECTABLE|TOUCHEXIT, DRAW3D|STATE11|STATE12|STATE15, (LONG) ((LONG)'\003' << 24)|0x00011100L,
  0x0717, 0x0911, 0x0302, 0x0301,
  WLEFT2          ,       -1,       -1, G_BOX             ,   /* Object 26 WHSPLIT */
  NONE, NORMAL, (LONG)0x00011100L,
  0x011A, 0x0804, 0x0600, 0x040E,
  WHBIGSL2        ,       -1,       -1, G_BOXCHAR         ,   /* Object 27 WLEFT2 */
  SELECTABLE|TOUCHEXIT, DRAW3D|STATE11|STATE12|STATE15, (LONG) ((LONG)'\004' << 24)|0x00011100L,
  0x051A, 0x0911, 0x0302, 0x0301,
  WRT2            , WHSMLSL2        , WHSMLSL2        , G_BOX             ,   /* Object 28 WHBIGSL2 */
  NONE, NORMAL, (LONG)0x00011111L,
  0x071C, 0x0911, 0x0712, 0x0301,
  WHBIGSL2        ,       -1,       -1, G_BOX             ,   /* Object 29 WHSMLSL2 */
  NONE, DRAW3D|STATE12|STATE15, (LONG)0x00011101L,
  0x0603, 0x0000, 0x0306, 0x0301,
  WSIZE           ,       -1,       -1, G_BOXCHAR         ,   /* Object 30 WRT2 */
  SELECTABLE|TOUCHEXIT, DRAW3D|STATE11|STATE12|STATE15, (LONG) ((LONG)'\003' << 24)|0x00011100L,
  0x052F, 0x0911, 0x0302, 0x0301,
         0,       -1,       -1, G_BOXCHAR         ,   /* Object 31 WSIZE */
  SELECTABLE|LASTOB|TOUCHEXIT, DRAW3D|STATE11|STATE12|STATE15, (LONG) ((LONG)'\006' << 24)|0x00011100L,
  0x0631, 0x0911, 0x0302, 0x0301,
  
  /******** Tree 2 COPYRT ****************************************************/
        -1, COPMOVE         , COPDATE         , G_BOX             ,   /* Object 0  */
  TOUCHEXIT, WHITEBAK|STATE9|STATE12|STATE15, (LONG)0x00FD1100L,
  0x0000, 0x0000, 0x001B, 0x000B,
  COPLOGO         ,       -1,       -1, G_BUTTON          |0x1100,   /* Object 1 COPMOVE */
  TOUCHEXIT, NORMAL, (LONG)"\0",
  0x0019, 0x0000, 0x0002, 0x0001,
         3,       -1,       -1, G_IMAGE           ,   /* Object 2 COPLOGO */
  TOUCHEXIT, NORMAL, (LONG)&rs_bitblk[0],
  0x0002, 0x0001, 0x0018, 0x0A04,
         4,       -1,       -1, G_STRING          ,   /* Object 3  */
  TOUCHEXIT, STATE10|STATE12|STATE15, (LONG)"by Dan Wilga",
  0x0009, 0x0006, 0x0009, 0x0001,
         5,       -1,       -1, G_STRING          ,   /* Object 4  */
  TOUCHEXIT, STATE10|STATE12|STATE15, (LONG)"Copyright \275 1994 Gribnif Software",
  0x0001, 0x0007, 0x0618, 0x0001,
         6,       -1,       -1, G_STRING          ,   /* Object 5  */
  TOUCHEXIT, STATE10|STATE12|STATE15, (LONG)"All Rights Reserved.",
  0x0006, 0x0008, 0x000F, 0x0001,
  COPDATE         ,       -1,       -1, G_STRING          ,   /* Object 6  */
  TOUCHEXIT, NORMAL, (LONG)"Release",
  0x0401, 0x0009, 0x0018, 0x0001,
         0,       -1,       -1, G_TEXT            ,   /* Object 7 COPDATE */
  LASTOB|TOUCHEXIT, NORMAL, (LONG)&rs_tedinfo[2],
  0x0401, 0x0009, 0x0018, 0x0001,
  
  /******** Tree 3 MAGMENU ****************************************************/
        -1,        1,        6, G_IBOX            ,   /* Object 0  */
  NONE, STATE12|STATE15, (LONG)0x00000000L,
  0x0000, 0x0000, 0x0050, 0x0019,
         6,        2,        2, G_BOX             ,   /* Object 1  */
  NONE, NORMAL, (LONG)0x00001100L,
  0x0000, 0x0000, 0x0050, 0x0201,
         1,        3,        5, G_IBOX            ,   /* Object 2  */
  NONE, NORMAL, (LONG)0x00000000L,
  0x0002, 0x0000, 0x0017, 0x0301,
         4,       -1,       -1, G_TITLE           ,   /* Object 3  */
  NONE, NORMAL, (LONG)" Geneva",
  0x0000, 0x0000, 0x0008, 0x0301,
         5,       -1,       -1, G_TITLE           ,   /* Object 4  */
  NONE, NORMAL, (LONG)" File",
  0x0008, 0x0000, 0x0006, 0x0301,
         2,       -1,       -1, G_TITLE           ,   /* Object 5  */
  NONE, NORMAL, (LONG)" Options",
  0x000E, 0x0000, 0x0009, 0x0301,
         0,        7,       14, G_IBOX            ,   /* Object 6  */
  NONE, NORMAL, (LONG)0x00000000L,
  0x0000, 0x0301, 0x0050, 0x0017,
        10, MAGABOUT        ,        9, G_BOX             ,   /* Object 7  */
  NONE, NORMAL, (LONG)0x00FF1000L,
  0x0002, 0x0000, 0x0015, 0x0002,
         9,       -1,       -1, G_STRING          ,   /* Object 8 MAGABOUT */
  NONE, NORMAL, (LONG)"  About Geneva... ^A ",
  0x0000, 0x0000, 0x0015, 0x0001,
         7,       -1,       -1, G_STRING          ,   /* Object 9  */
  NONE, DISABLED, (LONG)"---------------------",
  0x0000, 0x0001, 0x0015, 0x0001,
        14, MAGOPEN         , MAGQUIT         , G_BOX             ,   /* Object 10  */
  NONE, NORMAL, (LONG)0x00FF1100L,
  0x000A, 0x0000, 0x0013, 0x0003,
        12,       -1,       -1, G_STRING          ,   /* Object 11 MAGOPEN */
  NONE, NORMAL, (LONG)"  Open          ^O ",
  0x0000, 0x0000, 0x0013, 0x0001,
  MAGQUIT         ,       -1,       -1, G_STRING          ,   /* Object 12  */
  NONE, DISABLED, (LONG)"-------------------",
  0x0000, 0x0001, 0x0013, 0x0001,
        10,       -1,       -1, G_STRING          ,   /* Object 13 MAGQUIT */
  NONE, NORMAL, (LONG)"  Quit Geneva  \001^Q ",
  0x0000, 0x0002, 0x0013, 0x0001,
         6, MAGASCII        , MAGHELP         , G_BOX             ,   /* Object 14  */
  NONE, NORMAL, (LONG)0x00FF1000L,
  0x0010, 0x0000, 0x0014, 0x0002,
  MAGHELP         ,       -1,       -1, G_STRING          ,   /* Object 15 MAGASCII */
  NONE, NORMAL, (LONG)"  ASCII Table... ^T ",
  0x0000, 0x0000, 0x0014, 0x0001,
        14,       -1,       -1, G_STRING          ,   /* Object 16 MAGHELP */
  LASTOB, NORMAL, (LONG)"  Help...      Help ",
  0x0000, 0x0001, 0x0014, 0x0001,
  
  /******** Tree 4 PROCMAN ****************************************************/
        -1, PMIBOX          , PMSHIFT         , G_BOX             |0x5D00,   /* Object 0  */
  NONE, WHITEBAK|STATE9|STATE12|STATE15, (LONG)0x00FD1100L,
  0x0000, 0x0000, 0x0036, 0x0014,
  PMTITL0         ,        2,        3, G_IBOX            ,   /* Object 1 PMIBOX */
  NONE, NORMAL, (LONG)0x00001100L,
  0x0426, 0x0000, 0x000F, 0x0802,
         3,       -1,       -1, G_STRING          ,   /* Object 2  */
  NONE, STATE10|STATE12|STATE15, (LONG)"Preemptive",
  0x0402, 0x0400, 0x000A, 0x0001,
  PMIBOX          ,       -1,       -1, G_STRING          |0x5D00,   /* Object 3  */
  NONE, NORMAL, (LONG)"ID  state",
  0x0401, 0x0201, 0x0009, 0x0001,
  PMTITL          ,       -1,       -1, G_STRING          ,   /* Object 4 PMTITL0 */
  NONE, STATE10|STATE12|STATE15, (LONG)"----------- AES State -----------",
  0x000C, 0x0400, 0x0021, 0x0001,
         6,       -1,       -1, G_STRING          |0x5D00,   /* Object 5 PMTITL */
  NONE, NORMAL, (LONG)" Name     sleep  umew  apid  events",
  0x0002, 0x0201, 0x0023, 0x0001,
  PMI2            , PMLINE1         ,       16, G_BOX             ,   /* Object 6  */
  NONE, NORMAL, (LONG)0x00FF1000L,
  0x0401, 0x0802, 0x0031, 0x000A,
         8,       -1,       -1, G_TEXT            ,   /* Object 7 PMLINE1 */
  SELECTABLE|TOUCHEXIT|FLAGS10, NORMAL, (LONG)&rs_tedinfo[3],
  0x0000, 0x0000, 0x0031, 0x0001,
         9,       -1,       -1, G_TEXT            ,   /* Object 8  */
  SELECTABLE|TOUCHEXIT, NORMAL, (LONG)&rs_tedinfo[4],
  0x0000, 0x0001, 0x0031, 0x0001,
        10,       -1,       -1, G_TEXT            ,   /* Object 9  */
  SELECTABLE|TOUCHEXIT, NORMAL, (LONG)&rs_tedinfo[5],
  0x0000, 0x0002, 0x0031, 0x0001,
        11,       -1,       -1, G_TEXT            ,   /* Object 10  */
  SELECTABLE|TOUCHEXIT, NORMAL, (LONG)&rs_tedinfo[6],
  0x0000, 0x0003, 0x0031, 0x0001,
        12,       -1,       -1, G_TEXT            ,   /* Object 11  */
  SELECTABLE|TOUCHEXIT, NORMAL, (LONG)&rs_tedinfo[7],
  0x0000, 0x0004, 0x0031, 0x0001,
        13,       -1,       -1, G_TEXT            ,   /* Object 12  */
  SELECTABLE|TOUCHEXIT, NORMAL, (LONG)&rs_tedinfo[8],
  0x0000, 0x0005, 0x0031, 0x0001,
        14,       -1,       -1, G_TEXT            ,   /* Object 13  */
  SELECTABLE|TOUCHEXIT, NORMAL, (LONG)&rs_tedinfo[9],
  0x0000, 0x0006, 0x0031, 0x0001,
        15,       -1,       -1, G_TEXT            ,   /* Object 14  */
  SELECTABLE|TOUCHEXIT, NORMAL, (LONG)&rs_tedinfo[10],
  0x0000, 0x0007, 0x0031, 0x0001,
        16,       -1,       -1, G_TEXT            ,   /* Object 15  */
  SELECTABLE|TOUCHEXIT, NORMAL, (LONG)&rs_tedinfo[11],
  0x0000, 0x0008, 0x0031, 0x0001,
         6,       -1,       -1, G_TEXT            ,   /* Object 16  */
  SELECTABLE|TOUCHEXIT, NORMAL, (LONG)&rs_tedinfo[12],
  0x0000, 0x0009, 0x0031, 0x0001,
  PMI0            , PMUP            , PMDOWN          , G_IBOX            ,   /* Object 17 PMI2 */
  NONE, NORMAL, (LONG)0x00001100L,
  0x0432, 0x0802, 0x0302, 0x000A,
  PMBIG           ,       -1,       -1, G_BOXCHAR         ,   /* Object 18 PMUP */
  TOUCHEXIT|FLAGS13, DRAW3D|STATE12|STATE15, (LONG) ((LONG)'\001' << 24)|0x00FF1100L,
  0x0100, 0x0000, 0x0202, 0x0201,
  PMDOWN          , PMSMALL         , PMSMALL         , G_BOX             ,   /* Object 19 PMBIG */
  TOUCHEXIT, NORMAL, (LONG)0x00FF1151L,
  0x0100, 0x0301, 0x0202, 0x0007,
  PMBIG           ,       -1,       -1, G_BOX             ,   /* Object 20 PMSMALL */
  TOUCHEXIT, DRAW3D|STATE12|STATE15, (LONG)0x00FF1100L,
  0x0000, 0x0001, 0x0202, 0x0002,
  PMI2            ,       -1,       -1, G_BOXCHAR         ,   /* Object 21 PMDOWN */
  TOUCHEXIT|FLAGS13, DRAW3D|STATE12|STATE15, (LONG) ((LONG)'\002' << 24)|0x00FF1100L,
  0x0100, 0x0E08, 0x0202, 0x0201,
  PMI1            ,       23,       30, G_IBOX            ,   /* Object 22 PMI0 */
  NONE, NORMAL, (LONG)0x00001100L,
  0x0401, 0x080C, 0x0027, 0x0804,
        24,       -1,       -1, G_TEXT            ,   /* Object 23  */
  TOUCHEXIT, STATE10|STATE12|STATE15, (LONG)&rs_tedinfo[13],
  0x0402, 0x0800, 0x001B, 0x0800,
        25,       -1,       -1, G_TEXT            ,   /* Object 24  */
  TOUCHEXIT, STATE10|STATE12|STATE15, (LONG)&rs_tedinfo[14],
  0x0409, 0x0001, 0x0015, 0x0800,
        26,       -1,       -1, G_TEXT            ,   /* Object 25  */
  TOUCHEXIT, STATE10|STATE12|STATE15, (LONG)&rs_tedinfo[15],
  0x0405, 0x0801, 0x001A, 0x0800,
        27,       -1,       -1, G_TEXT            ,   /* Object 26  */
  TOUCHEXIT, STATE10|STATE12|STATE15, (LONG)&rs_tedinfo[16],
  0x0408, 0x0002, 0x0018, 0x0800,
  PMUPDT          ,       -1,       -1, G_TEXT            ,   /* Object 27  */
  TOUCHEXIT, STATE10|STATE12|STATE15, (LONG)&rs_tedinfo[17],
  0x041A, 0x0802, 0x0007, 0x0800,
        29,       -1,       -1, G_BOXTEXT         ,   /* Object 28 PMUPDT */
  SELECTABLE|EXIT|FLAGS10|FLAGS13, DRAW3D|STATE9|STATE10|STATE12|STATE15, (LONG)&rs_tedinfo[18],
  0x0408, 0x0003, 0x040D, 0x0001,
        30,       -1,       -1, G_TEXT            ,   /* Object 29  */
  TOUCHEXIT, STATE10|STATE12|STATE15, (LONG)&rs_tedinfo[19],
  0x041B, 0x0003, 0x0007, 0x0800,
  PMI0            ,       -1,       -1, G_TEXT            ,   /* Object 30  */
  TOUCHEXIT, STATE10|STATE12|STATE15, (LONG)&rs_tedinfo[20],
  0x041B, 0x0803, 0x0008, 0x0800,
  PMDETAIL        , PMTERM          , PMCANCEL        , G_IBOX            ,   /* Object 31 PMI1 */
  NONE, NORMAL, (LONG)0x00001100L,
  0x0428, 0x080C, 0x040D, 0x0806,
  PMSLEEP         ,       -1,       -1, G_BUTTON          ,   /* Object 32 PMTERM */
  SELECTABLE|EXIT, WHITEBAK|STATE8|STATE9|STATE12|STATE15, (LONG)"[Terminate",
  0x0400, 0x0001, 0x000B, 0x0201,
  PMCANCEL        ,       -1,       -1, G_BUTTON          ,   /* Object 33 PMSLEEP */
  SELECTABLE|EXIT, WHITEBAK|STATE8|STATE9|STATE12|STATE15, (LONG)"[Sleep/Wake",
  0x0400, 0x0802, 0x000B, 0x0201,
  PMI1            ,       -1,       -1, G_BUTTON          |0x1F00,   /* Object 34 PMCANCEL */
  SELECTABLE|DEFAULT|EXIT, WHITEBAK|STATE8|STATE9|STATE12|STATE15, (LONG)"[Cancel",
  0x0400, 0x0005, 0x000B, 0x0201,
  PMSHIFT         ,       -1,       -1, G_BUTTON          |0x1200,   /* Object 35 PMDETAIL */
  SELECTABLE|TOUCHEXIT|FLAGS9, STATE12|STATE15, (LONG)"[Details",
  0x0410, 0x0811, 0x000A, 0x0001,
         0,       -1,       -1, G_STRING          ,   /* Object 36 PMSHIFT */
  LASTOB|TOUCHEXIT, STATE10|STATE12|STATE15, (LONG)"Hold SHIFT to make multiple selections",
  0x000D, 0x0013, 0x041C, 0x0800
};

OBJECT *rs_trindex[] =
{ &rs_object[0],   /* Tree  0 DESK             */
  &rs_object[1],   /* Tree  1 WIND             */
  &rs_object[33],   /* Tree  2 COPYRT           */
  &rs_object[41],   /* Tree  3 MAGMENU          */
  &rs_object[58]    /* Tree  4 PROCMAN          */
};
