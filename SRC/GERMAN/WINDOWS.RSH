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
  "RRR MMM 99 1999",
  "\0",
  "\0",
  IBM  , 0, TE_RIGHT, 0x1100, 0, 0, 16, 1,
  "Namexxxxxxxxxxxxxxxxxxxxx",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 26, 1,
  "Firmaxxxxxxxxxxxxxxxxxxxx",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 26, 1,
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
  NONE, NORMAL, (LONN˙   P  D  <      &–        /
/$Hp\Nπ    &H gRK K` J&_$_NuHÁ >YOï  y    2( B <   Nπ    .àf3¸ˇŸ    ` TK˘    M˘    0.~g   Bn~(U` LNπ    RÄŸ¿f mˇ¸fB( rˇ? mˇ¸RHNπ    AÙ  êï/  NNπ    –ü–º   	2Nπ    $H 
gl êï"UNπ     Lë’CÚà (IC˘    LNπ    "N\L LNπ     NNπ    RÄŸ¿&mˇ¸RK`  fBRK  g¯`€fËB¡Å mˇ¸º *ä/C˘    ë»pNπ    XO(@ 
g JNπ     oJ)W )|    )N Ä)mˇ¸ Ñ y    9h B à y    AË N)H é)|ˇˇˇˇ íBl ñ y    !@ Üp`3Ã    B@XOLﬂ| Nu/
/$Hp.Nπ    &H f JNπ    GÚ `¸   êär≤ÄnÚ&_$_NuHÁ <$H*I(o G˘    px"J SAË ΩNπ     <   Ä"M SAË5Nπ    HÄ S–¿B(6 g L`A˘    "S#H∂(SIÏ ¨p ¿¿*J- : fTM Ma   "H LpNπ    B,  La   Lﬂ< NuHÁ 89    gj$y    G˘    `R0* N∞y    fD(S&äBApNπ    J@g. <  ÜCÍ T SAË TNπ     SB( ª SBh NBy     SB® Ü&å`$R 
f™B9    Lﬂ NuHÁ>YO.à,I*o (9    f  ÙE˘    &R6+ N7|ˇˇ NI˘    p¿y   f9    gt`BBlˇ ƒ| ·JÔJÖl BAB@Nπ    J@g  ûp2"L RAË TNπ     RrpNπ    8, p‡l»| JDgz`z R h 20Nπ    0Nπ     g M`
 R"h  i∂/"N o a   XONπ     R0hˇ r¿| ·HÔHÅh rJDgNπ     RB® ÜBy    7C NXOLﬂ|8NuHÁ86 9    f  ‘E˘     g  »∞π    g  æG˘    9    f& S"P≥¸    fr R“® †"A S ëNπ    B9    9    j R±˘    f¸     9    gr?BB R0( B2Nπ    TO`N R ( Üf4(h r?BB0( B2Nπ    TO`(l  , fˆ L#»    Nπ    ` S"P ) $f(R#l Ü $LﬂNu/&I"y    CÈ ΩNπ    "y    CÈ5 KNπ    p&_NuHÁ8&H$I(o Bë g8 LNπ    & `(0H¿"L KNπ    J@f
AÛ0 $àp` KNπ    RÄ◊¿f‘B@LﬂNu/&H/	 y     h∂"Ka   XOp&_Nu//YO&H(I/CÔ  y    a   XO g @f.Ø &W W ,g
 ;gf4 êã"L KNπ    J@f WëÀ/ LNπ    ∞üf K`&Wfë»`RKRó`∂XO(_&_NuHÁ0$Hg0* : fHÄ¿| _–|ˇøNπ    TJ Ja   &HB JNπ    ÉLﬂNuA˘    a   Nu??OÔˇ86>ÄBo ∞| )f
0A◊Nπ    ?D 
tA◊r0?@ Nπ    OÔ 86NuHÁ $H6 8pˇNπ    j  Jf4* B2p)a   p¿j LfJDg4* B2p2a   LﬂNu/
$y    ` Jp2Nπ     Jp)Nπ    $R 
fÊBy    Bπ    $_Nu/
UO9    jp`8A˘    Nπ    $H 
fE˘    0* r‚h¿| gA◊pNπ    0`0<ˇTO$_Nu9    g649    g.∞< ‡d∞< c∞|qf9    op y    1@`¿    NuB NuHÁ>OÔˇ»6 :8*H/I 4a   J f zGÔ 0M˘    E˘    5C|∂|qfVHy   	"K y    a   XOJ@g F%Mv%o 4r5DÄKÍÇ Sa   %HÇ"S N%HéNπ     Na   B/|     4`∂< ‡e∂< „bñ| ‡`2∂|qg∂|qf6< ` ∂|qf6<`∂|qf#˘        `  ∂< c∂< f "09    f Æ y    ±¸    gNπ     o 4ˇˇg&"HAÍ x#»    Nπ     o 4f^"y    B) `Rrˇ? o 6RHNπ    –º   Å2Nπ    #»    f#¸        ` 6 y    ºˇˇ"o 4RI y    RHNπ    0¿|ˇ g"MAÍÇp(Nπ    "jÇ JNπ    `
"M JNπ    f
B9    ` ‹* : gp\ JNπ     g Ja   p.Nπ     f  åHy   "K y    a   XO0¿| g jé`ë»/H ,/Bß"JNπ    POJ@fPHy   "K y    a   XO/Bß"J o 4Nπ    POJ@f&Hy   #"K y    a   XO/Bß"J o 4Nπ    PO JNπ    0¿| g jég"H NNπ     Na   g@`0ë»Nπ    (HNπ    –< AÄ| : B@AÓ Nπ     LNπ    C˘   + NNπ    0¿| g
"*íg A` y    #»    | pB@∞| 
b ê–@0; N˚   ~Ñj¨∂˛˛<Œ‹p. JNπ    (H gX Ja   π»eNHy   -RL"LA˘   #a   XO f  Hy   5"LA˘   a   XO gBE`Hy   ="LA˘   a   XOzJEf  ‰Jf g  ⁄ y    2( rt‰i¬| g  ƒHy   I"K y    a   XOJ@g  ™"J y    RHNπ     o 4g(C˘   Q y    RHNπ    "o 4RI y    RHNπ     y    RHNπ    , º| ~l`pˇ y    Ä"S JNπ    p5@~5@zB*p#˘         y    0( rr‚h¿| gt`tˇ¬    `>B*p5Ez#˘         y    0( rr‚h¿| gt`tˇ¬    jB@`05@~a   ` ‹JEfa   ` Œ3¸ <    ∫| g∫| g∫|ˇˇf Æ9    g §JEjrˇ`r3¡    BF,y    `$Ω˘    g09    k2. J≤@f
p¿n LfRF,V fÿJFgA˘   SpNπ    U@f
By    ` B,y    `\p¿. ∫g NBApNπ    Ω˘    g>09    k2. J≤@f
rp2 Na   n  f y    .ê` o .êNπ    /W fÓ,V f†#˘        ` –JDf∫y    g æ3≈    `$∏| f ÆpˇNπ    ∫@g †3≈    3¸     3¸ =    zˇ` ˛ƒ,y    `.Ω˘    g$n  Jg. ºgU PgBB Mr0. BNπ    ,V fŒ` J0J@gS@gS@g :` 20*ˇ˛` 0(MAÔ &à` SRìÄ∞< =gfÓ SRìBHo "K y    a   XOJ@g@ SNπ     SC/I ,p 0*ˇ˛ y    —¿ êâ/ AÔ Nπ     Së¿"o 0 Nπ    g ∞2*ˇ˛gl(y    ` LNπ    RÄŸ¿f MNπ    AÙp 0*ˇ˛"y    ”¿±…cX&π    2*ˇ˛“| p 0 KNπ    J@f P Lë˘    "S”»(I#”    `rˇ <   Nπ    (H g $#»    j ˇ˛"M LNπ     LNπ    B4` 0*ˇ˛fB¡Åp˛–D`  ∏jˇ˛c0*ˇ˛`0r 2  "y     MNπ    ∏jˇ˛eB@`0*ˇ˛êD`  ∫ y    1E L`  ™0ê| glê| g
ê| g&`  í 9    g  à @4( BBA09    Nπ    ` ¸ú 9    gb"MAÔ pNπ    ?|ˇˇ BBAÔ r"y    0) B?@ 
Nπ    `4p"MAÔ Nπ    0/ Nπ    /H gBBBA0/ Nπ    J@fB@`pOÔ 8Lﬂ|xNuHÁ8YO.à6 "9    g
9    f  íE˘    AÍ "y    \I Ÿ Ÿ0ëG˘     S0( 5@ ,5@ 5h  pˇ–y    5@ . Wa   (H%Hˇ‰` am
 zn _RLfÍ S!J ŒBy     S?( ?( ?( 4( r0ì… JNπ    \OXOLﬂNu/
/$H9    gJ9    gJG˘     
f&+ R+ JfpˇNπ    6ÄB@Nπ     |   `8S+ f0gNπ    `9    gë»` 
f$y     y     P"y    "ä&_$_Nu/
/E˘    G˘    Rqf#”    0* "jˇ˙ jˇˆNπ    `  vRqf#”    `fRqf
 S (  ∫`V0¿| g:9    g S"h Ü#j 
 í`$ * 
"<   Nπ     S1@ t Shˇø r h @ r0¿| g S"h Ü3j  ñ&_$_Nu/9    f<¸      < ç∫NB& ∂| o$∂|Çl <QBAKNB& g∞ºQBAKg
r“Ä A0º &NuHÁ>OÔˇŒE˘    I˘    9    o  ∆a    JNπ    &H fG˘    * ‡}e&* „}b0*|¿|Ä g"KA◊p2Nπ    /j¶ G◊B9    A˘    a   *pgn*TNπ     JNπ    ,H g<"T6) Bjq|g
0ë»Nπ    "N MNπ    J@f ∆49    2- B0Nπ    `49    2- BpˇNπ    a   B@` úBA0+ t‰h¿| Nπ    J@g bp2"K TAË TNπ    9    fBAë»pNπ    BD09   ÓH¿| gP9    gH4+ p‡jƒ| f:jq|gA˘   ΩpNπ    S@g S@gS@g`( T (  ∫`x3¸ˇ‡    `0+ r‚h¿| Nπ    JDfN9    f82*zgA˘  ANπ    B@ Ja   `*BAë»0< Nπ    A˘  DNπ    `BAë»0<ËNπ     T6( B*HJDfPa   J@gHa    m Ü/( ,"y     Ja   XOë»0Nπ    a   B9    Nπ     m ÜNπ    J@j<3¿    BB29    0Nπ     f(π    `49     T2( BpˇNπ    By    a   pOÔ 2Lﬂ|NuHÁOÔˇà&H8 C˘    A◊Nπ    C˘  IA◊Nπ    B@A◊Nπ    & JCj,C˘  U/	AÔ a   "_Nπ    B@A◊Nπ    & JCjB@`*JDo K2H¡0Nπ    t2p Nπ    ( 0Nπ    0OÔ xLﬂNuHÄÄB@NuHÁ8$H(I&H ,g
 ;gf$&
ñã0H¿"K LNπ    B40 fë»`AÍ `RJ`»LﬂNu//OÔˇà&H"KI◊ LNπ    , : gNπ    –< A¿¸ :`‹‹ \gB@ KNπ     KNπ    ◊¿¸ \pC˘  a LNπ    J@fRL`pC˘  c LNπ    J@fTL"L KNπ    OÔ x(_&_NuHÁ>OÔ˛¸(H*I&o0$o4BCB f"f$
fx`BDë»a   .à gNπ    < B@AÔ Nπ     La   Nπ    /H A˘    Nπ    MÔ ¸p\ MNπ     f- : f,º    &N f0Hy  f"N y    a   XOJ@fC˘  f Na    f,º    &NMÔ Ñ 
g 
 Ma   p.Nπ     g  ˆï `  Jf
"N Sa   &à NNπ    . JGg6 \pˇgC˘   + NNπ    pC˘  c NNπ    J@fCÓ  NNπ    "M NNπ     
gC˘  a NNπ     NNπ    Aˆ  /H | 
g"J`C˘    /I Ä`0"o | o Äa   /H Äp' NNπ    J@f"N MNπ    v / ÄgJCgÃ f$Jf&Jg"z"y    CÈ Ω NNπ     Na   BJCg ˇ o Nπ     gHÄNπ    AÔ Nπ     Wa   0OÔLﬂ|¯NuBßBß"Hë»a   PONu                Ä  ˇˇ  ˇˇ          C          ˇˇˇˇ                           ARGV= SHOWHELP= GEMEXT= TOSEXT= ACCEXT= \ ACC,ACX TOS,TTP PRG,APP,GTP TOSRUN=   [1][One or more applications|will not know they are|being shut down.|Continue anyway?][C[ontinue|[Cancel] [3][This is a singletasking|application. Either it|or all other applications|must be put to sleep.][Sleep [All|Sleep [This|[Cancel] f Ee NEWDESK.INF DESKTOP.INF . .\ PATH=  ˇˇ  

 V á
 ã 0 7
 l Ü è$ è è 0 \  [ [ è< 
 W E  ã ã ã 7 q
 Ä è z ã ; ; & 

 ; q 2  A ã ' a \ _ 
  g
 ã S . /$ a \ <& < :  z    _ D g
 ã O
 Y N > %  å
 â
 å N H" H O à M ã
 [ ã [
 p è ] è ê ã p
 @ m p0 ] è$ = G
 
 j  ` = f w   é f f + A é
 é ! 5 ) å Ü } ~ ç I î  å
 ã
 
  I K Ü  m p  
 [ 
 @ ã ? !
 u P E u [ u è 0 u P u
 u u [ \ [ [
 å Ä 
 Ä  m p  ,  m p ,  m p ,
  [ 
 " F i "  v m l< V 
 
  @   @   @ ã  m p u [  u v u v u è u [ ã â ã å ã â ã å ) +
 ì / ! A ã !$  Q
 !
 A < ã !  c * ã 5 9
 6 s  6 ì A ã& T m p è m è \ m
 è è m m | m m 0 m [ è6 m \ ã& 5 ì ä
  \   \
 B J   /
 4
 3 c  
( X  k " N î  r
 r r & d Y Y ^ Ö ã ? e   &  í2 Ñ 
 < R
 Ü ã å
 Ñ } ~& \ å K =  n o
 1 7 h 7 h  a \ / Z . /  Q" 7 :
 /  Å
    Å  á  u z
 Ç ^  
 {
 7 7  
 A 7 h _  $ ` [  v
   
 [
  U 8
 ( #
 t( ;  b [ F  i è  ]  ] [ ,0 "
 F i = 
 É  Ä   m p
  ê  
 Ä t è  v  ] [
 v
  v è  t T [  ã [ 
  G
 j " L
 ,ˇˇ	    x   4$ x u  P l  ˇˇ	    lˇˇ	    ë Üx P¯ -    Ö C y 	( O
 
  
 ˇˇin_scold_domw9_fixed   _shel_write_graf_mouse_appl_writeshwrcmdpathendnew_vidloop_idsh_execsw_hlptopic	new_flagsdo_frksw_grin_t2
shel_rpath	shel_disp	shell_appmenu_hFgetdtaFsetdta	block_appdo_sigtop_menu
sw_hlpfile	send_termhas_menuFopen__x_appl_termunloadshel_contextshel_putshel_get	ring_bellhas_mintfrk_idFclose
child_apid	close_del
bad_unload
_shel_findsw_accsettings	multitasklallocgo_acc	fix_pfork	dflt_desk	blankdeskunloadernew_rezload_errFseekcur_rez	set_multistrncpyto_sleep	shel_pathmagic_bphelp_app	env_matchapp0find_windowsw_argv	terminatelfreeDgetdrvDsetdrv_x_appl_termswrite_waitscroll_wdialsw_dflt	shel_find	shel_readpreemptlast_parent_sw_tail_form_alert	shel_execnoflagsFsfirstFreadstrrchrPexecdesk_objbp_addrtoggle_multistrcpymemcpystrncmpodd_exlock_curapploadpathinit_aphfix_pathdesktopcurr_bp_x_helpterm_msgsleepingexit_msgDgetpathDsetpath
_objc_drawsw_envenvironacc_runac_open_shel_envrnpad_acc_namePdomainVsetmodeshel_splitlistsw_tailstrcatfind_menu_idblankted
sw_hlpsensset_execmy_pexeclrealloc
find_flags
dflt_flagsstruprstrchrCconws_menu_registershel_dta	fix_warp9sw_exsw_path
shel_setup
set_curappsw_appnewtopcurappsw_flagfind_free_apnodel_all_msgstrlen
shel_envrn
env_buflen_ldivunload_type
mint_preemQDB2     Ë  6  X     ‘  *%>†  	     
     "   ©         á    ä   ≥   ) 8

˙
     q     6   Ë         z     ê       
     2     |      Ò                *
      D     Ë  =    & 
     M     ,  V         p     R  ]    ˘	     ê       q         @     ~  w    
(
     =     @  ä    
            ö    
     f     B  ü    
          D  ™    
     +     4  ±   	 
˝
     )     N  æ   	 
     I     B  Õ    *         ¶  Á   • 	





&


*

,@$


 
,
˛

˝˘

˝
ˇ.
"


          ¥  ˇ    
ˇ"     "     Ñ      


     ^     ¨  4         Ñ     J  N    *
     R    å  ]   > 

$




     $     ò  «    
     #     
  ﬁ    	     t     H  Ò   
      b     î      



     ,    
     0 

$ 

¯
Ê



     L       d                        
   
       
                                     ‡      €          ›       
   ›          €          
          ‡                 
   Â  0     ú  0   
  ú  0     m  @     ü  @     z  @      
          ‡                    ’  0     °  @      ‡                   z  @     •           €            ê   
   
          €       	     ê     •                    ß         ¨         ∞          Â                          
  z  †      m  @     π  @     Ì  @      Ω  @     π  @     ú  0   
  ú  0   
  z  @      ∆  ∞   
     @      œ  –      ÷  –     z  `      á  `      z  †     z  @     ú  0     ã  p     ê       ,          0                ‡     ï   ‡       ‡       4  –         ò  @     ú  @        @     ü  Ä   	  Ω  Ä     •         °  @     •          ´         ±  @     π   ‡     ï   ‡   
  Ω   `     z  @     Ω        ú  0     ∆  `      ∆  ∞     …  @     °  @      m  @     ï   ‡     Ω   `     ÿ  @     ã  ‡      ã  @     z         ã  @      z         Õ         z  @     π       