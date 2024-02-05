/* GEM Resource C Source */

#include <portab.h>
#include <aes.h>
#include "FSEL.H"

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
{ "Item Selector/////////////////",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 31, 1,
  "[PATH:",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 7, 1,
  "aaaaaaaaaaaaaaaaaaaaaaaaaa",
  "__________________________",
  "xxxxxxxxxxxxxxxxxxxxxxxxxx",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 27, 27,
  "\0",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 1, 1,
  "[UPDATE",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 8, 1,
  "[SORT",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 6, 1,
  "[TOOLS",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 7, 1,
  "E[XT",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 5, 1,
  "A[LL",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 5, 1,
  "FOLDER1 .EXT",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 13, 1,
  "FOLDER2 .EXT",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 13, 1,
  "FOLDER3 .EXT",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 13, 1,
  "FOLDER4 .EXT",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 13, 1,
  "FOLDER5 .EXT",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 13, 1,
  "FOLDER6 .EXT",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 13, 1,
  "FOLDER7 .EXT",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 13, 1,
  "FOLDER8 .EXT",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 13, 1,
  "FOLDER9 .EXT",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 13, 1,
  "FOLDERa .EXT",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 13, 1,
  "FSFILENAMEX",
  "________.___",
  "FFFFFFFFFFF",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 12, 13,
  "FILE1   .EXT",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x11F0, 0, 0, 13, 1,
  "FILE2   .EXT",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x11F0, 0, 0, 13, 1,
  "FILE3   .EXT",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x11F0, 0, 0, 13, 1,
  "FILE4   .EXT",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x11F0, 0, 0, 13, 1,
  "FILE5   .EXT",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x11F0, 0, 0, 13, 1,
  "FILE6   .EXT",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x11F0, 0, 0, 13, 1,
  "FILE7   .EXT",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x11F0, 0, 0, 13, 1,
  "FILE8   .EXT",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x11F0, 0, 0, 13, 1,
  "FILE9   .EXT",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x11F0, 0, 0, 13, 1,
  "FILEa   .EXT",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x11F0, 0, 0, 13, 1,
  "FSFILENAMEX",
  "________.___",
  "FFFFFFFFFFF",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 12, 13,
  "A[LL",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 5, 1,
  "file11111111111111111111111111",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 31, 1,
  "file22222222222222222222222222",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 31, 1,
  "file33333333333333333333333333",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 31, 1,
  "file44444444444444444444444444",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 31, 1,
  "file55555555555555555555555555",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 31, 1,
  "file66666666666666666666666666",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 31, 1,
  "file77777777777777777777777777",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 31, 1,
  "file88888888888888888888888888",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 31, 1,
  "file99999999999999999999999999",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 31, 1,
  "file10101010101010101010101010",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 31, 1,
  "HELP",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 5, 1,
  "11111 ITEMS IN",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 0, 15, 1,
  "11111111 BYTES",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 0, 15, 1,
  "bbbbbbbbbbb",
  "Folder: ________.___",
  "FFFFFFFFFFF",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 12, 21,
  "ccccccccccc",
  "Name: ________.___",
  "FFFFFFFFFFF",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 12, 19,
  "Size:",
  "\0",
  "\0",
  IBM  , 0, TE_RIGHT, 0x1100, 0, 0, 6, 1,
  "1234567a bytes",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 15, 1,
  "Date:",
  "\0",
  "\0",
  IBM  , 0, TE_RIGHT, 0x1100, 0, 0, 6, 1,
  "Folders:",
  "\0",
  "\0",
  IBM  , 0, TE_RIGHT, 0x1100, 0, 0, 9, 1,
  "1234",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 5, 1,
  "Files:",
  "\0",
  "\0",
  IBM  , 0, TE_RIGHT, 0x1100, 0, 0, 7, 1,
  "12345",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 6, 1,
  "Are you sure you want to",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 25, 1,
  "delete this item?",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 18, 1,
  "Return",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 7, 1,
  "Ex.: *.{PRG,TOS,TTP}",
  "\0",
  "\0",
  SMALL, 0, TE_LEFT , 0x1100, 0, -1, 21, 1,
  "Ex.: *.PR[GX] *.PI[1-3]",
  "\0",
  "\0",
  SMALL, 0, TE_LEFT , 0x1100, 0, -1, 24, 1,
  "ggggggggggggggggggggggggg",
  "Find: _________________________",
  "xxxxxxxxxxxxxxxxxxxxxxxxx",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 26, 32,
  "Folder:",
  "\0",
  "\0",
  IBM  , 0, TE_RIGHT, 0x1100, 0, 0, 8, 1,
  "12345678.abc",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, 0, 13, 1,
  "File:",
  "\0",
  "\0",
  IBM  , 0, TE_RIGHT, 0x1100, 0, 0, 6, 1,
  "12345678.def",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, 0, 13, 1,
  "Folder:",
  "\0",
  "\0",
  IBM  , 0, TE_RIGHT, 0x1100, 0, 0, 8, 1,
  "12345678.abc",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, 0, 13, 1,
  "File:",
  "\0",
  "\0",
  IBM  , 0, TE_RIGHT, 0x1100, 0, 0, 6, 1,
  "12345678.def",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, 0, 13, 1,
  "ccccccccccc",
  "New name: ________.___",
  "FFFFFFFFFFF",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 12, 23,
  "Folders to go:",
  "\0",
  "\0",
  IBM  , 0, TE_RIGHT, 0x1100, 0, 0, 15, 1,
  "1234",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 5, 1,
  "Files to go:",
  "\0",
  "\0",
  IBM  , 0, TE_RIGHT, 0x1100, 0, 0, 13, 1,
  "12345",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 6, 1
};

OBJECT rs_object[] =
{ 
  /******** Tree 0 FSEL ****************************************************/
        -1, FSTITLE         , FSBYTES         , G_BOX             |0x5D00,   /* Object 0  */
  NONE, WHITEBAK|STATE8|STATE12|STATE15, (LONG)0x00FD1100L,
  0x0000, 0x0000, 0x0028, 0x0413,
  FSPTHB          ,       -1,       -1, G_BOXTEXT         ,   /* Object 1 FSTITLE */
  FLAGS10, STATE12|STATE15, (LONG)&rs_tedinfo[0],
  0x0002, 0x0400, 0x0424, 0x0001,
  FSPLEFT         ,       -1,       -1, G_BOXTEXT         ,   /* Object 2 FSPTHB */
  TOUCHEXIT|FLAGS10|FLAGS13, DRAW3D|STATE9|STATE12|STATE15, (LONG)&rs_tedinfo[1],
  0x0002, 0x0B01, 0x0404, 0x0201,
  FSPATH          ,       -1,       -1, G_BOXCHAR         ,   /* Object 3 FSPLEFT */
  TOUCHEXIT|FLAGS10|FLAGS13, DRAW3D|STATE12|STATE15, (LONG) ((LONG)'\004' << 24)|0x00FF1100L,
  0x0506, 0x0B01, 0x0202, 0x0201,
  FSPRIGHT        ,       -1,       -1, G_FBOXTEXT        ,   /* Object 4 FSPATH */
  EDITABLE|TOUCHEXIT|FLAGS10, NORMAL, (LONG)&rs_tedinfo[2],
  0x0009, 0x0B01, 0x061A, 0x0201,
  FS1COL          ,       -1,       -1, G_BOXCHAR         ,   /* Object 5 FSPRIGHT */
  TOUCHEXIT|FLAGS10|FLAGS13, DRAW3D|STATE12|STATE15, (LONG) ((LONG)'\003' << 24)|0x00FF1100L,
  0x0723, 0x0B01, 0x0202, 0x0201,
  FSUPDATE        ,       -1,       -1, G_BOXTEXT         ,   /* Object 6 FS1COL */
  SELECTABLE|EXIT|FLAGS10, DRAW3D|STATE9|STATE12|STATE15, (LONG)&rs_tedinfo[3],
  0x0002, 0x0803, 0x0506, 0x0201,
  FSSORT          ,       -1,       -1, G_BOXTEXT         ,   /* Object 7 FSUPDATE */
  SELECTABLE|EXIT|FLAGS10, DRAW3D|STATE9|STATE12|STATE15, (LONG)&rs_tedinfo[4],
  0x0608, 0x0803, 0x0008, 0x0201,
  FSTOOLS         ,       -1,       -1, G_BOXTEXT         ,   /* Object 8 FSSORT */
  TOUCHEXIT|FLAGS10|FLAGS13, DRAW3D|STATE9|STATE12|STATE15, (LONG)&rs_tedinfo[5],
  0x0710, 0x0803, 0x0008, 0x0201,
  FSFEXT          ,       -1,       -1, G_BOXTEXT         ,   /* Object 9 FSTOOLS */
  TOUCHEXIT|FLAGS10|FLAGS13, DRAW3D|STATE9|STATE12|STATE15, (LONG)&rs_tedinfo[6],
  0x0019, 0x0803, 0x0008, 0x0201,
        11,       -1,       -1, G_BOXTEXT         ,   /* Object 10 FSFEXT */
  TOUCHEXIT|FLAGS10|FLAGS13, DRAW3D|STATE9|STATE12|STATE15, (LONG)&rs_tedinfo[7],
  0x0121, 0x0803, 0x0005, 0x0201,
  FSOOUTER        ,       12, FSDUP           , G_IBOX            ,   /* Object 11  */
  NONE, NORMAL, (LONG)0x00001100L,
  0x0001, 0x0005, 0x0403, 0x000C,
  FSDDOWN         , FSDA            ,       21, G_BOX             ,   /* Object 12  */
  FLAGS10, NORMAL, (LONG)0x00FF1100L,
  0x0001, 0x0701, 0x0202, 0x0009,
        14,       -1,       -1, G_BOXCHAR         |0x7100,   /* Object 13 FSDA */
  SELECTABLE|EXIT|RBUTTON|FLAGS10, NORMAL, (LONG) ((LONG)'A' << 24)|0x00001100L,
  0x0000, 0x0000, 0x0202, 0x0001,
        15,       -1,       -1, G_BOXCHAR         |0x7100,   /* Object 14  */
  SELECTABLE|EXIT|RBUTTON|FLAGS10, NORMAL, (LONG) ((LONG)'B' << 24)|0x00001100L,
  0x0000, 0x0001, 0x0202, 0x0001,
        16,       -1,       -1, G_BOXCHAR         |0x7100,   /* Object 15  */
  SELECTABLE|EXIT|RBUTTON|FLAGS10, NORMAL, (LONG) ((LONG)'C' << 24)|0x00001100L,
  0x0000, 0x0002, 0x0202, 0x0001,
        17,       -1,       -1, G_BOXCHAR         |0x7100,   /* Object 16  */
  SELECTABLE|EXIT|RBUTTON|FLAGS10, NORMAL, (LONG) ((LONG)'D' << 24)|0x00001100L,
  0x0000, 0x0003, 0x0202, 0x0001,
        18,       -1,       -1, G_BOXCHAR         |0x7100,   /* Object 17  */
  SELECTABLE|EXIT|RBUTTON|FLAGS10, NORMAL, (LONG) ((LONG)'E' << 24)|0x00001100L,
  0x0000, 0x0004, 0x0202, 0x0001,
        19,       -1,       -1, G_BOXCHAR         |0x7100,   /* Object 18  */
  SELECTABLE|EXIT|RBUTTON|FLAGS10, NORMAL, (LONG) ((LONG)'F' << 24)|0x00001100L,
  0x0000, 0x0005, 0x0202, 0x0001,
        20,       -1,       -1, G_BOXCHAR         |0x7100,   /* Object 19  */
  SELECTABLE|EXIT|RBUTTON|FLAGS10, NORMAL, (LONG) ((LONG)'G' << 24)|0x00001100L,
  0x0000, 0x0006, 0x0202, 0x0001,
        21,       -1,       -1, G_BOXCHAR         |0x7100,   /* Object 20  */
  SELECTABLE|EXIT|RBUTTON|FLAGS10, NORMAL, (LONG) ((LONG)'H' << 24)|0x00001100L,
  0x0000, 0x0007, 0x0202, 0x0001,
        12,       -1,       -1, G_BOXCHAR         |0x7100,   /* Object 21  */
  SELECTABLE|EXIT|RBUTTON|FLAGS10, NORMAL, (LONG) ((LONG)'I' << 24)|0x00001100L,
  0x0000, 0x0008, 0x0202, 0x0001,
  FSDUP           ,       -1,       -1, G_BOXCHAR         ,   /* Object 22 FSDDOWN */
  TOUCHEXIT|FLAGS10|FLAGS13, DRAW3D|STATE12|STATE15, (LONG) ((LONG)'\002' << 24)|0x00FF1100L,
  0x0001, 0x060A, 0x0202, 0x0201,
        11,       -1,       -1, G_BOXCHAR         ,   /* Object 23 FSDUP */
  TOUCHEXIT|FLAGS10|FLAGS13, DRAW3D|STATE12|STATE15, (LONG) ((LONG)'\001' << 24)|0x00FF1100L,
  0x0001, 0x0400, 0x0202, 0x0201,
  FSFOUTER        , FSCLOSE         , FSODOWN         , G_BOX             ,   /* Object 24 FSOOUTER */
  NONE, NORMAL, (LONG)0x00FF1100L,
  0x0705, 0x0405, 0x030F, 0x040B,
  FSROOT          ,       -1,       -1, G_BOXCHAR         ,   /* Object 25 FSCLOSE */
  SELECTABLE|EXIT, DRAW3D|STATE12|STATE15, (LONG) ((LONG)'\005' << 24)|0x00FF1100L,
  0x0000, 0x0000, 0x0202, 0x0201,
  FSALL1          ,       -1,       -1, G_BOXCHAR         ,   /* Object 26 FSROOT */
  SELECTABLE|EXIT, DRAW3D|STATE12|STATE15, (LONG) ((LONG)'\134' << 24)|0x00FF1100L,
  0x0302, 0x0000, 0x0202, 0x0201,
  FSONSDT         ,       -1,       -1, G_BOXTEXT         ,   /* Object 27 FSALL1 */
  SELECTABLE|EXIT|FLAGS10, DRAW3D|STATE9|STATE12|STATE15, (LONG)&rs_tedinfo[8],
  0x0604, 0x0000, 0x0208, 0x0201,
        29,       -1,       -1, G_BOXCHAR         ,   /* Object 28 FSONSDT */
  SELECTABLE|EXIT, DRAW3D|STATE12|STATE15, (LONG) ((LONG)'n' << 24)|0x00FF1100L,
  0x010D, 0x0000, 0x0202, 0x0201,
  FSOUP           , FSFFOLD1        ,       39, G_BOX             ,   /* Object 29  */
  NONE, NORMAL, (LONG)0x00FF1100L,
  0x0000, 0x0301, 0x000D, 0x010A,
        31,       -1,       -1, G_TEXT            ,   /* Object 30 FSFFOLD1 */
  SELECTABLE|TOUCHEXIT, NORMAL, (LONG)&rs_tedinfo[9],
  0x0000, 0x0000, 0x000D, 0x0001,
        32,       -1,       -1, G_TEXT            ,   /* Object 31  */
  SELECTABLE|TOUCHEXIT, NORMAL, (LONG)&rs_tedinfo[10],
  0x0000, 0x0001, 0x000D, 0x0001,
        33,       -1,       -1, G_TEXT            ,   /* Object 32  */
  SELECTABLE|TOUCHEXIT, NORMAL, (LONG)&rs_tedinfo[11],
  0x0000, 0x0002, 0x000D, 0x0001,
        34,       -1,       -1, G_TEXT            ,   /* Object 33  */
  SELECTABLE|TOUCHEXIT, NORMAL, (LONG)&rs_tedinfo[12],
  0x0000, 0x0003, 0x000D, 0x0001,
        35,       -1,       -1, G_TEXT            ,   /* Object 34  */
  SELECTABLE|TOUCHEXIT, NORMAL, (LONG)&rs_tedinfo[13],
  0x0000, 0x0004, 0x000D, 0x0001,
        36,       -1,       -1, G_TEXT            ,   /* Object 35  */
  SELECTABLE|TOUCHEXIT, NORMAL, (LONG)&rs_tedinfo[14],
  0x0000, 0x0005, 0x000D, 0x0001,
        37,       -1,       -1, G_TEXT            ,   /* Object 36  */
  SELECTABLE|TOUCHEXIT, NORMAL, (LONG)&rs_tedinfo[15],
  0x0000, 0x0006, 0x000D, 0x0001,
        38,       -1,       -1, G_TEXT            ,   /* Object 37  */
  SELECTABLE|TOUCHEXIT, NORMAL, (LONG)&rs_tedinfo[16],
  0x0000, 0x0007, 0x000D, 0x0001,
        39,       -1,       -1, G_TEXT            ,   /* Object 38  */
  SELECTABLE|TOUCHEXIT, NORMAL, (LONG)&rs_tedinfo[17],
  0x0000, 0x0008, 0x000D, 0x0001,
        29,       -1,       -1, G_TEXT            ,   /* Object 39  */
  SELECTABLE|TOUCHEXIT, NORMAL, (LONG)&rs_tedinfo[18],
  0x0000, 0x0009, 0x000D, 0x0001,
  FSOBIG          ,       -1,       -1, G_BOXCHAR         ,   /* Object 40 FSOUP */
  TOUCHEXIT|FLAGS13, DRAW3D|STATE12|STATE15, (LONG) ((LONG)'\001' << 24)|0x00FF1100L,
  0x010D, 0x0301, 0x0202, 0x0201,
  FSODOWN         , FSOSMALL        , FSOSMALL        , G_BOX             ,   /* Object 41 FSOBIG */
  TOUCHEXIT, NORMAL, (LONG)0x00FF1151L,
  0x010D, 0x0602, 0x0202, 0x0407,
  FSOBIG          ,       -1,       -1, G_BOX             ,   /* Object 42 FSOSMALL */
  TOUCHEXIT, DRAW3D|STATE12|STATE15, (LONG)0x00FF1100L,
  0x0000, 0x0001, 0x0202, 0x0002,
  FSOOUTER        ,       -1,       -1, G_BOXCHAR         ,   /* Object 43 FSODOWN */
  TOUCHEXIT|FLAGS13, DRAW3D|STATE12|STATE15, (LONG) ((LONG)'\002' << 24)|0x00FF1100L,
  0x010D, 0x020A, 0x0202, 0x0201,
  FSFOUTER2       , FSFILE          , FSFDOWN         , G_BOX             ,   /* Object 44 FSFOUTER */
  NONE, NORMAL, (LONG)0x00FF1100L,
  0x0616, 0x0405, 0x030F, 0x040B,
  FSFNSDT         ,       -1,       -1, G_FBOXTEXT        ,   /* Object 45 FSFILE */
  EDITABLE|FLAGS10, NORMAL, (LONG)&rs_tedinfo[19],
  0x0000, 0x0000, 0x000D, 0x0201,
        47,       -1,       -1, G_BOXCHAR         ,   /* Object 46 FSFNSDT */
  SELECTABLE|EXIT, DRAW3D|STATE12|STATE15, (LONG) ((LONG)'n' << 24)|0x00FF1100L,
  0x010D, 0x0000, 0x0202, 0x0201,
  FSFUP           , FSFFILE1        ,       57, G_BOX             ,   /* Object 47  */
  NONE, NORMAL, (LONG)0x00FF1100L,
  0x0000, 0x0301, 0x000D, 0x010A,
        49,       -1,       -1, G_TEXT            ,   /* Object 48 FSFFILE1 */
  SELECTABLE|TOUCHEXIT, NORMAL, (LONG)&rs_tedinfo[20],
  0x0000, 0x0000, 0x000D, 0x0001,
        50,       -1,       -1, G_TEXT            ,   /* Object 49  */
  SELECTABLE|TOUCHEXIT, NORMAL, (LONG)&rs_tedinfo[21],
  0x0000, 0x0001, 0x000D, 0x0001,
        51,       -1,       -1, G_TEXT            ,   /* Object 50  */
  SELECTABLE|TOUCHEXIT, NORMAL, (LONG)&rs_tedinfo[22],
  0x0000, 0x0002, 0x000D, 0x0001,
        52,       -1,       -1, G_TEXT            ,   /* Object 51  */
  SELECTABLE|TOUCHEXIT, NORMAL, (LONG)&rs_tedinfo[23],
  0x0000, 0x0003, 0x000D, 0x0001,
        53,       -1,       -1, G_TEXT            ,   /* Object 52  */
  SELECTABLE|TOUCHEXIT, NORMAL, (LONG)&rs_tedinfo[24],
  0x0000, 0x0004, 0x000D, 0x0001,
        54,       -1,       -1, G_TEXT            ,   /* Object 53  */
  SELECTABLE|TOUCHEXIT, NORMAL, (LONG)&rs_tedinfo[25],
  0x0000, 0x0005, 0x000D, 0x0001,
        55,       -1,       -1, G_TEXT            ,   /* Object 54  */
  SELECTABLE|TOUCHEXIT, NORMAL, (LONG)&rs_tedinfo[26],
  0x0000, 0x0006, 0x000D, 0x0001,
        56,       -1,       -1, G_TEXT            ,   /* Object 55  */
  SELECTABLE|TOUCHEXIT, NORMAL, (LONG)&rs_tedinfo[27],
  0x0000, 0x0007, 0x000D, 0x0001,
        57,       -1,       -1, G_TEXT            ,   /* Object 56  */
  SELECTABLE|TOUCHEXIT, NORMAL, (LONG)&rs_tedinfo[28],
  0x0000, 0x0008, 0x000D, 0x0001,
        47,       -1,       -1, G_TEXT            ,   /* Object 57  */
  SELECTABLE|TOUCHEXIT, NORMAL, (LONG)&rs_tedinfo[29],
  0x0000, 0x0009, 0x000D, 0x0001,
  FSFBIG          ,       -1,       -1, G_BOXCHAR         ,   /* Object 58 FSFUP */
  TOUCHEXIT|FLAGS13, DRAW3D|STATE12|STATE15, (LONG) ((LONG)'\001' << 24)|0x00FF1100L,
  0x010D, 0x0301, 0x0202, 0x0201,
  FSFDOWN         , FSFSMALL        , FSFSMALL        , G_BOX             ,   /* Object 59 FSFBIG */
  TOUCHEXIT, NORMAL, (LONG)0x00FF1151L,
  0x010D, 0x0602, 0x0202, 0x0407,
  FSFBIG          ,       -1,       -1, G_BOX             ,   /* Object 60 FSFSMALL */
  TOUCHEXIT, DRAW3D|STATE12|STATE15, (LONG)0x00FF1100L,
  0x0000, 0x0001, 0x0202, 0x0002,
  FSFOUTER        ,       -1,       -1, G_BOXCHAR         ,   /* Object 61 FSFDOWN */
  TOUCHEXIT|FLAGS13, DRAW3D|STATE12|STATE15, (LONG) ((LONG)'\002' << 24)|0x00FF1100L,
  0x010D, 0x020A, 0x0202, 0x0201,
  FSHELP          , FSCLOSE2        , FSFDOWN2        , G_BOX             ,   /* Object 62 FSFOUTER2 */
  HIDETREE, NORMAL, (LONG)0x00FF1100L,
  0x0105, 0x0805, 0x0021, 0x040B,
  FSROOT2         ,       -1,       -1, G_BOXCHAR         ,   /* Object 63 FSCLOSE2 */
  SELECTABLE|EXIT, DRAW3D|STATE12|STATE15, (LONG) ((LONG)'\005' << 24)|0x00FF1100L,
  0x0000, 0x0000, 0x0202, 0x0201,
  FSNAME2         ,       -1,       -1, G_BOXCHAR         ,   /* Object 64 FSROOT2 */
  SELECTABLE|EXIT, DRAW3D|STATE12|STATE15, (LONG) ((LONG)'\134' << 24)|0x00FF1100L,
  0x0302, 0x0000, 0x0202, 0x0201,
  FSALL2          ,       -1,       -1, G_FBOXTEXT        ,   /* Object 65 FSNAME2 */
  EDITABLE|FLAGS10, NORMAL, (LONG)&rs_tedinfo[30],
  0x0604, 0x0000, 0x0119, 0x0201,
        67,       -1,       -1, G_BOXTEXT         ,   /* Object 66 FSALL2 */
  SELECTABLE|EXIT, DRAW3D|STATE9|STATE12|STATE15, (LONG)&rs_tedinfo[31],
  0x001E, 0x0000, 0x0003, 0x0201,
  FSFUP2          , FSFFILE2        ,       77, G_BOX             ,   /* Object 67  */
  NONE, NORMAL, (LONG)0x00FF1100L,
  0x0000, 0x0301, 0x051E, 0x010A,
        69,       -1,       -1, G_TEXT            ,   /* Object 68 FSFFILE2 */
  SELECTABLE|TOUCHEXIT, NORMAL, (LONG)&rs_tedinfo[32],
  0x0000, 0x0000, 0x051E, 0x0001,
        70,       -1,       -1, G_TEXT            ,   /* Object 69  */
  SELECTABLE|TOUCHEXIT, NORMAL, (LONG)&rs_tedinfo[33],
  0x0000, 0x0001, 0x051E, 0x0001,
        71,       -1,       -1, G_TEXT            ,   /* Object 70  */
  SELECTABLE|TOUCHEXIT, NORMAL, (LONG)&rs_tedinfo[34],
  0x0000, 0x0002, 0x051E, 0x0001,
        72,       -1,       -1, G_TEXT            ,   /* Object 71  */
  SELECTABLE|TOUCHEXIT, NORMAL, (LONG)&rs_tedinfo[35],
  0x0000, 0x0003, 0x051E, 0x0001,
        73,       -1,       -1, G_TEXT            ,   /* Object 72  */
  SELECTABLE|TOUCHEXIT, NORMAL, (LONG)&rs_tedinfo[36],
  0x0000, 0x0004, 0x051E, 0x0001,
        74,       -1,       -1, G_TEXT            ,   /* Object 73  */
  SELECTABLE|TOUCHEXIT, NORMAL, (LONG)&rs_tedinfo[37],
  0x0000, 0x0005, 0x051E, 0x0001,
        75,       -1,       -1, G_TEXT            ,   /* Object 74  */
  SELECTABLE|TOUCHEXIT, NORMAL, (LONG)&rs_tedinfo[38],
  0x0000, 0x0006, 0x051E, 0x0001,
        76,       -1,       -1, G_TEXT            ,   /* Object 75  */
  SELECTABLE|TOUCHEXIT, NORMAL, (LONG)&rs_tedinfo[39],
  0x0000, 0x0007, 0x051E, 0x0001,
        77,       -1,       -1, G_TEXT            ,   /* Object 76  */
  SELECTABLE|TOUCHEXIT, NORMAL, (LONG)&rs_tedinfo[40],
  0x0000, 0x0008, 0x051E, 0x0001,
        67,       -1,       -1, G_TEXT            ,   /* Object 77  */
  SELECTABLE|TOUCHEXIT, NORMAL, (LONG)&rs_tedinfo[41],
  0x0000, 0x0009, 0x051E, 0x0001,
  FSFBIG2         ,       -1,       -1, G_BOXCHAR         ,   /* Object 78 FSFUP2 */
  TOUCHEXIT|FLAGS13, DRAW3D|STATE12|STATE15, (LONG) ((LONG)'\001' << 24)|0x00FF1100L,
  0x061E, 0x0301, 0x0202, 0x0201,
  FSFDOWN2        , FSFSMALL2       , FSFSMALL2       , G_BOX             ,   /* Object 79 FSFBIG2 */
  TOUCHEXIT, NORMAL, (LONG)0x00FF1151L,
  0x061E, 0x0602, 0x0202, 0x0407,
  FSFBIG2         ,       -1,       -1, G_BOX             ,   /* Object 80 FSFSMALL2 */
  TOUCHEXIT, DRAW3D|STATE12|STATE15, (LONG)0x00FF1100L,
  0x0000, 0x0001, 0x0202, 0x0002,
  FSFOUTER2       ,       -1,       -1, G_BOXCHAR         ,   /* Object 81 FSFDOWN2 */
  TOUCHEXIT|FLAGS13, DRAW3D|STATE12|STATE15, (LONG) ((LONG)'\002' << 24)|0x00FF1100L,
  0x061E, 0x020A, 0x0202, 0x0201,
  FSOK            ,       -1,       -1, G_BOXTEXT         |0x1500,   /* Object 82 FSHELP */
  SELECTABLE|EXIT, DRAW3D|STATE10|STATE12|STATE15, (LONG)&rs_tedinfo[42],
  0x0002, 0x0411, 0x0004, 0x0201,
  FSCANCEL        ,       -1,       -1, G_BUTTON          ,   /* Object 83 FSOK */
  SELECTABLE|DEFAULT|EXIT, WHITEBAK|STATE8|STATE9|STATE12|STATE15, (LONG)"[Ok",
  0x0412, 0x0411, 0x0009, 0x0201,
  FSITEMS         ,       -1,       -1, G_BUTTON          |0x1F00,   /* Object 84 FSCANCEL */
  SELECTABLE|EXIT, WHITEBAK|STATE8|STATE9|STATE12|STATE15, (LONG)"Ca[ncel",
  0x001D, 0x0411, 0x0009, 0x0201,
  FSBYTES         ,       -1,       -1, G_BOXTEXT         ,   /* Object 85 FSITEMS */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[43],
  0x0606, 0x0411, 0x040A, 0x0600,
         0,       -1,       -1, G_BOXTEXT         ,   /* Object 86 FSBYTES */
  LASTOB|FLAGS10, NORMAL, (LONG)&rs_tedinfo[44],
  0x0606, 0x0212, 0x040A, 0x0600,
  
  /******** Tree 1 FFOLD ****************************************************/
        -1,        1, FFCANCEL        , G_BOX             |0x5D00,   /* Object 0  */
  NONE, WHITEBAK|STATE8|STATE12|STATE15, (LONG)0x00FD1100L,
  0x0000, 0x0000, 0x0017, 0x0006,
  FFNAME          ,       -1,       -1, G_STRING          |0x1300,   /* Object 1  */
  FLAGS9, NORMAL, (LONG)"New folder",
  0x0400, 0x0400, 0x0016, 0x0001,
  FFOK            ,       -1,       -1, G_FTEXT           ,   /* Object 2 FFNAME */
  EDITABLE|FLAGS10, NORMAL, (LONG)&rs_tedinfo[45],
  0x0401, 0x0002, 0x0014, 0x0001,
  FFCANCEL        ,       -1,       -1, G_BUTTON          ,   /* Object 3 FFOK */
  SELECTABLE|DEFAULT|EXIT, WHITEBAK|STATE8|STATE9|STATE12|STATE15, (LONG)"[Ok",
  0x0402, 0x0004, 0x0008, 0x0201,
         0,       -1,       -1, G_BUTTON          |0x1200,   /* Object 4 FFCANCEL */
  SELECTABLE|EXIT|LASTOB, WHITEBAK|STATE8|STATE9|STATE12|STATE15, (LONG)"[Cancel",
  0x040C, 0x0004, 0x0008, 0x0201,
  
  /******** Tree 2 FINFO ****************************************************/
        -1,        1, FSRDELMSG       , G_BOX             |0x5D00,   /* Object 0  */
  NONE, WHITEBAK|STATE12|STATE15, (LONG)0x00FD1100L,
  0x0000, 0x0000, 0x0020, 0x000D,
  FSROLD          ,       -1,       -1, G_STRING          |0x1300,   /* Object 1  */
  FLAGS9, NORMAL, (LONG)"x",
  0x0400, 0x0200, 0x001F, 0x0001,
  FSRSIZEO        ,       -1,       -1, G_FTEXT           ,   /* Object 2 FSROLD */
  EDITABLE|FLAGS10, NORMAL, (LONG)&rs_tedinfo[46],
  0x0007, 0x0401, 0x0012, 0x0001,
  FSRSIZE         ,       -1,       -1, G_TEXT            ,   /* Object 3 FSRSIZEO */
  NONE, NORMAL, (LONG)&rs_tedinfo[47],
  0x0006, 0x0003, 0x0005, 0x0001,
  FSRDATEO        ,       -1,       -1, G_BOXTEXT         ,   /* Object 4 FSRSIZE */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[48],
  0x000C, 0x0003, 0x000E, 0x0001,
  FSRDATE         ,       -1,       -1, G_TEXT            ,   /* Object 5 FSRDATEO */
  NONE, NORMAL, (LONG)&rs_tedinfo[49],
  0x0006, 0x0004, 0x0005, 0x0001,
  FSRFOLDMSG      ,       -1,       -1, G_STRING          ,   /* Object 6 FSRDATE */
  NONE, NORMAL, (LONG)"xx:xx:xx yy/yy/yy",
  0x000C, 0x0004, 0x0011, 0x0001,
  FSRRO           , FSRFOLDO        , FSRFILES        , G_IBOX            ,   /* Object 7 FSRFOLDMSG */
  NONE, NORMAL, (LONG)0x00001100L,
  0x0000, 0x0005, 0x0020, 0x0002,
  FSRFOLD         ,       -1,       -1, G_TEXT            ,   /* Object 8 FSRFOLDO */
  NONE, NORMAL, (LONG)&rs_tedinfo[50],
  0x0003, 0x0000, 0x0008, 0x0001,
  FSRFILEO        ,       -1,       -1, G_BOXTEXT         ,   /* Object 9 FSRFOLD */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[51],
  0x000C, 0x0000, 0x0004, 0x0001,
  FSRFILES        ,       -1,       -1, G_TEXT            ,   /* Object 10 FSRFILEO */
  NONE, NORMAL, (LONG)&rs_tedinfo[52],
  0x0005, 0x0001, 0x0006, 0x0001,
  FSRFOLDMSG      ,       -1,       -1, G_BOXTEXT         ,   /* Object 11 FSRFILES */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[53],
  0x000C, 0x0001, 0x0005, 0x0001,
  FSROK           ,       -1,       -1, G_BUTTON          |0x1200,   /* Object 12 FSRRO */
  SELECTABLE|FLAGS9, NORMAL, (LONG)"[Read only",
  0x000A, 0x0006, 0x000C, 0x0001,
  FSRSKIP         ,       -1,       -1, G_BUTTON          ,   /* Object 13 FSROK */
  SELECTABLE|DEFAULT|EXIT, WHITEBAK|STATE8|STATE9|STATE12|STATE15, (LONG)"\0",
  0x0002, 0x0008, 0x0008, 0x0201,
  FSRCANCEL       ,       -1,       -1, G_BUTTON          ,   /* Object 14 FSRSKIP */
  SELECTABLE|EXIT, WHITEBAK|STATE8|STATE9|STATE12|STATE15, (LONG)"[Skip",
  0x000C, 0x0008, 0x0008, 0x0201,
  FSRDELMSG       ,       -1,       -1, G_BUTTON          |0x1200,   /* Object 15 FSRCANCEL */
  SELECTABLE|EXIT, WHITEBAK|STATE8|STATE9|STATE12|STATE15, (LONG)"[Abort",
  0x0016, 0x0008, 0x0008, 0x0201,
         0, FSRMSG1         , FSRMSG2         , G_IBOX            ,   /* Object 16 FSRDELMSG */
  NONE, NORMAL, (LONG)0x00001100L,
  0x0003, 0x000A, 0x001B, 0x0002,
  FSRMSG2         ,       -1,       -1, G_TEXT            ,   /* Object 17 FSRMSG1 */
  NONE, NORMAL, (LONG)&rs_tedinfo[54],
  0x0001, 0x0000, 0x0018, 0x0001,
  FSRDELMSG       ,       -1,       -1, G_TEXT            ,   /* Object 18 FSRMSG2 */
  LASTOB, NORMAL, (LONG)&rs_tedinfo[55],
  0x0005, 0x0001, 0x0011, 0x0001,
  
  /******** Tree 3 PATHLIST ****************************************************/
        -1,        1,       21, G_BOX             ,   /* Object 0  */
  FLAGS10, SHADOWED, (LONG)0x00FF1100L,
  0x0000, 0x0000, 0x0024, 0x000A,
  PATH1           ,        2,       11, G_IBOX            ,   /* Object 1  */
  NONE, DISABLED, (LONG)0x00001100L,
  0x0021, 0x0000, 0x0003, 0x000A,
         3,       -1,       -1, G_STRING          ,   /* Object 2  */
  NONE, STATE10|STATE12|STATE15, (LONG)"F1",
  0x0001, 0x0000, 0x0002, 0x0001,
         4,       -1,       -1, G_STRING          ,   /* Object 3  */
  NONE, STATE10|STATE12|STATE15, (LONG)"F2",
  0x0001, 0x0001, 0x0002, 0x0001,
         5,       -1,       -1, G_STRING          ,   /* Object 4  */
  NONE, STATE10|STATE12|STATE15, (LONG)"F3",
  0x0001, 0x0002, 0x0002, 0x0001,
         6,       -1,       -1, G_STRING          ,   /* Object 5  */
  NONE, STATE10|STATE12|STATE15, (LONG)"F4",
  0x0001, 0x0003, 0x0002, 0x0001,
         7,       -1,       -1, G_STRING          ,   /* Object 6  */
  NONE, STATE10|STATE12|STATE15, (LONG)"F5",
  0x0001, 0x0004, 0x0002, 0x0001,
         8,       -1,       -1, G_STRING          ,   /* Object 7  */
  NONE, STATE10|STATE12|STATE15, (LONG)"F6",
  0x0001, 0x0005, 0x0002, 0x0001,
         9,       -1,       -1, G_STRING          ,   /* Object 8  */
  NONE, STATE10|STATE12|STATE15, (LONG)"F7",
  0x0001, 0x0006, 0x0002, 0x0001,
        10,       -1,       -1, G_STRING          ,   /* Object 9  */
  NONE, STATE10|STATE12|STATE15, (LONG)"F8",
  0x0001, 0x0007, 0x0002, 0x0001,
        11,       -1,       -1, G_STRING          ,   /* Object 10  */
  NONE, STATE10|STATE12|STATE15, (LONG)"F9",
  0x0001, 0x0008, 0x0002, 0x0001,
         1,       -1,       -1, G_STRING          ,   /* Object 11  */
  NONE, STATE10|STATE12|STATE15, (LONG)"F10",
  0x0600, 0x0009, 0x0202, 0x0001,
        13,       -1,       -1, G_STRING          ,   /* Object 12 PATH1 */
  SELECTABLE, NORMAL, (LONG)"\0",
  0x0000, 0x0000, 0x0024, 0x0001,
        14,       -1,       -1, G_STRING          ,   /* Object 13  */
  SELECTABLE, NORMAL, (LONG)"\0",
  0x0000, 0x0001, 0x0024, 0x0001,
        15,       -1,       -1, G_STRING          ,   /* Object 14  */
  SELECTABLE, NORMAL, (LONG)"\0",
  0x0000, 0x0002, 0x0024, 0x0001,
        16,       -1,       -1, G_STRING          ,   /* Object 15  */
  SELECTABLE, NORMAL, (LONG)"\0",
  0x0000, 0x0003, 0x0024, 0x0001,
        17,       -1,       -1, G_STRING          ,   /* Object 16  */
  SELECTABLE, NORMAL, (LONG)"\0",
  0x0000, 0x0004, 0x0024, 0x0001,
        18,       -1,       -1, G_STRING          ,   /* Object 17  */
  SELECTABLE, NORMAL, (LONG)"\0",
  0x0000, 0x0005, 0x0024, 0x0001,
        19,       -1,       -1, G_STRING          ,   /* Object 18  */
  SELECTABLE, NORMAL, (LONG)"\0",
  0x0000, 0x0006, 0x0024, 0x0001,
        20,       -1,       -1, G_STRING          ,   /* Object 19  */
  SELECTABLE, NORMAL, (LONG)"\0",
  0x0000, 0x0007, 0x0024, 0x0001,
        21,       -1,       -1, G_STRING          ,   /* Object 20  */
  SELECTABLE, NORMAL, (LONG)"\0",
  0x0000, 0x0008, 0x0024, 0x0001,
         0,       -1,       -1, G_STRING          ,   /* Object 21  */
  SELECTABLE|LASTOB, NORMAL, (LONG)"\0",
  0x0000, 0x0009, 0x0024, 0x0001,
  
  /******** Tree 4 EXTLIST ****************************************************/
        -1,        1,       11, G_BOX             |0x5D00,   /* Object 0  */
  FLAGS10, SHADOWED, (LONG)0x00FF1100L,
  0x0000, 0x0000, 0x0005, 0x000B,
         2,       -1,       -1, G_STRING          ,   /* Object 1  */
  SELECTABLE, NORMAL, (LONG)"Start",
  0x0000, 0x0000, 0x0005, 0x0001,
         3,       -1,       -1, G_STRING          ,   /* Object 2  */
  SELECTABLE, NORMAL, (LONG)"\0",
  0x0000, 0x0001, 0x0005, 0x0001,
         4,       -1,       -1, G_STRING          ,   /* Object 3  */
  SELECTABLE, NORMAL, (LONG)"\0",
  0x0000, 0x0002, 0x0005, 0x0001,
         5,       -1,       -1, G_STRING          ,   /* Object 4  */
  SELECTABLE, NORMAL, (LONG)"\0",
  0x0000, 0x0003, 0x0005, 0x0001,
         6,       -1,       -1, G_STRING          ,   /* Object 5  */
  SELECTABLE, NORMAL, (LONG)"\0",
  0x0000, 0x0004, 0x0005, 0x0001,
         7,       -1,       -1, G_STRING          ,   /* Object 6  */
  SELECTABLE, NORMAL, (LONG)"\0",
  0x0000, 0x0005, 0x0005, 0x0001,
         8,       -1,       -1, G_STRING          ,   /* Object 7  */
  SELECTABLE, NORMAL, (LONG)"\0",
  0x0000, 0x0006, 0x0005, 0x0001,
         9,       -1,       -1, G_STRING          ,   /* Object 8  */
  SELECTABLE, NORMAL, (LONG)"\0",
  0x0000, 0x0007, 0x0005, 0x0001,
        10,       -1,       -1, G_STRING          ,   /* Object 9  */
  SELECTABLE, NORMAL, (LONG)"\0",
  0x0000, 0x0008, 0x0005, 0x0001,
        11,       -1,       -1, G_STRING          ,   /* Object 10  */
  SELECTABLE, NORMAL, (LONG)"\0",
  0x0000, 0x0009, 0x0005, 0x0001,
         0,       -1,       -1, G_STRING          ,   /* Object 11  */
  SELECTABLE|LASTOB, NORMAL, (LONG)"\0",
  0x0000, 0x000A, 0x0005, 0x0001,
  
  /******** Tree 5 FSHELPTXT ****************************************************/
        -1,        1,       33, G_BOX             |0x5D00,   /* Object 0  */
  TOUCHEXIT, WHITEBAK|STATE9|STATE12|STATE15, (LONG)0x00FD1100L,
  0x0000, 0x0000, 0x0028, 0x0413,
         2,       -1,       -1, G_STRING          |0x1300,   /* Object 1  */
  TOUCHEXIT|FLAGS9|FLAGS15, STATE12|STATE15, (LONG)"Keyboard equivalents",
  0x0401, 0x0400, 0x0025, 0x0001,
         3,       -1,       -1, G_BUTTON          ,   /* Object 2  */
  TOUCHEXIT, DRAW3D|STATE10|STATE12|STATE15, (LONG)"ALT",
  0x0402, 0x0002, 0x0403, 0x0001,
         4,       -1,       -1, G_BUTTON          ,   /* Object 3  */
  TOUCHEXIT, DRAW3D|STATE10|STATE12|STATE15, (LONG)"Esc",
  0x0406, 0x0002, 0x0403, 0x0001,
         5,       -1,       -1, G_STRING          ,   /* Object 4  */
  TOUCHEXIT, NORMAL, (LONG)"Up one folder level",
  0x040B, 0x0002, 0x0013, 0x0001,
         6,       -1,       -1, G_BUTTON          ,   /* Object 5  */
  TOUCHEXIT, DRAW3D|STATE10|STATE12|STATE15, (LONG)"ALT",
  0x0402, 0x0003, 0x0403, 0x0001,
         7,       -1,       -1, G_BUTTON          ,   /* Object 6  */
  TOUCHEXIT, DRAW3D|STATE10|STATE12|STATE15, (LONG)"A-Z",
  0x0406, 0x0003, 0x0403, 0x0001,
         8,       -1,       -1, G_STRING          ,   /* Object 7  */
  TOUCHEXIT, NORMAL, (LONG)"Select disk drive",
  0x040B, 0x0003, 0x0013, 0x0001,
         9,       -1,       -1, G_BUTTON          ,   /* Object 8  */
  TOUCHEXIT, DRAW3D|STATE10|STATE12|STATE15, (LONG)"CONTROL",
  0x0402, 0x0004, 0x0407, 0x0001,
        10,       -1,       -1, G_STRING          |0x1300,   /* Object 9  */
  TOUCHEXIT|FLAGS9|FLAGS15, STATE12|STATE15, (LONG)"Hold to enter Search mode:",
  0x040B, 0x0004, 0x001A, 0x0001,
        11,       -1,       -1, G_BUTTON          ,   /* Object 10  */
  TOUCHEXIT, DRAW3D|STATE10|STATE12|STATE15, (LONG)"A-Z",
  0x0409, 0x0006, 0x0006, 0x0001,
        12,       -1,       -1, G_STRING          ,   /* Object 11  */
  TOUCHEXIT, NORMAL, (LONG)"Search for filename",
  0x0011, 0x0006, 0x0013, 0x0001,
        13,       -1,       -1, G_BUTTON          ,   /* Object 12  */
  TOUCHEXIT, DRAW3D|STATE10|STATE12|STATE15, (LONG)"Esc",
  0x0409, 0x0007, 0x0006, 0x0001,
        14,       -1,       -1, G_STRING          ,   /* Object 13  */
  TOUCHEXIT, NORMAL, (LONG)"Clear search filename",
  0x0011, 0x0007, 0x0015, 0x0001,
        15,       -1,       -1, G_BUTTON          ,   /* Object 14  */
  TOUCHEXIT, DRAW3D|STATE10|STATE12|STATE15, (LONG)"Bksp",
  0x0409, 0x0008, 0x0006, 0x0001,
        16,       -1,       -1, G_STRING          ,   /* Object 15  */
  TOUCHEXIT, NORMAL, (LONG)"Delete last letter",
  0x0011, 0x0008, 0x0012, 0x0001,
        17,       -1,       -1, G_BUTTON          ,   /* Object 16  */
  TOUCHEXIT, DRAW3D|STATE10|STATE12|STATE15, (LONG)"Home",
  0x0409, 0x0009, 0x0006, 0x0001,
        18,       -1,       -1, G_STRING          ,   /* Object 17  */
  TOUCHEXIT, NORMAL, (LONG)"Move to first file",
  0x0011, 0x0009, 0x0012, 0x0001,
        19,       -1,       -1, G_BUTTON          ,   /* Object 18  */
  TOUCHEXIT, DRAW3D|STATE10|STATE12|STATE15, (LONG)"Shift",
  0x0004, 0x000A, 0x0005, 0x0001,
        20,       -1,       -1, G_BUTTON          ,   /* Object 19  */
  TOUCHEXIT, DRAW3D|STATE10|STATE12|STATE15, (LONG)"Home",
  0x0409, 0x000A, 0x0006, 0x0001,
        21,       -1,       -1, G_STRING          ,   /* Object 20  */
  TOUCHEXIT, NORMAL, (LONG)"Move to last file",
  0x0011, 0x000A, 0x0011, 0x0001,
        22,       -1,       -1, G_BUTTON          ,   /* Object 21  */
  TOUCHEXIT, DRAW3D|STATE10|STATE12|STATE15, (LONG)"\001 or \002",
  0x0409, 0x000B, 0x0006, 0x0001,
        23,       -1,       -1, G_STRING          ,   /* Object 22  */
  TOUCHEXIT, NORMAL, (LONG)"Selection up/down",
  0x0011, 0x000B, 0x0011, 0x0001,
        24,       -1,       -1, G_BUTTON          ,   /* Object 23  */
  TOUCHEXIT, DRAW3D|STATE10|STATE12|STATE15, (LONG)"Shift",
  0x0004, 0x000C, 0x0005, 0x0001,
        25,       -1,       -1, G_BUTTON          ,   /* Object 24  */
  TOUCHEXIT, DRAW3D|STATE10|STATE12|STATE15, (LONG)"\001 or \002",
  0x0409, 0x000C, 0x0006, 0x0001,
        26,       -1,       -1, G_STRING          ,   /* Object 25  */
  TOUCHEXIT, NORMAL, (LONG)"Page up/down",
  0x0011, 0x000C, 0x000C, 0x0001,
        27,       -1,       -1, G_BOXTEXT         ,   /* Object 26  */
  SELECTABLE|DEFAULT|EXIT, DRAW3D|STATE10|STATE12|STATE15, (LONG)&rs_tedinfo[56],
  0x0409, 0x000D, 0x0006, 0x0001,
        28,       -1,       -1, G_STRING          ,   /* Object 27  */
  TOUCHEXIT, NORMAL, (LONG)"Open file/folder",
  0x0011, 0x000D, 0x0010, 0x0001,
        29,       -1,       -1, G_STRING          |0x1300,   /* Object 28  */
  TOUCHEXIT|FLAGS9|FLAGS15, STATE12|STATE15, (LONG)"Wildcards in path",
  0x0401, 0x040E, 0x0025, 0x0001,
        30,       -1,       -1, G_STRING          ,   /* Object 29  */
  TOUCHEXIT, NORMAL, (LONG)"*  Match any chars  ? match any one",
  0x0002, 0x0010, 0x0023, 0x0001,
        31,       -1,       -1, G_STRING          ,   /* Object 30  */
  TOUCHEXIT, NORMAL, (LONG)"{} List of names",
  0x0002, 0x0011, 0x0010, 0x0001,
        32,       -1,       -1, G_TEXT            ,   /* Object 31  */
  TOUCHEXIT, NORMAL, (LONG)&rs_tedinfo[57],
  0x0016, 0x0011, 0x000F, 0x0001,
        33,       -1,       -1, G_STRING          ,   /* Object 32  */
  TOUCHEXIT, NORMAL, (LONG)"[] Range of letters",
  0x0002, 0x0012, 0x0013, 0x0001,
         0,       -1,       -1, G_TEXT            ,   /* Object 33  */
  LASTOB|TOUCHEXIT, NORMAL, (LONG)&rs_tedinfo[58],
  0x0016, 0x0012, 0x0211, 0x0001,
  
  /******** Tree 6 OPLIST ****************************************************/
        -1, OPINFO          , OPFREE          , G_BOX             |0x5D00,   /* Object 0  */
  FLAGS10, SHADOWED, (LONG)0x00FF1100L,
  0x0000, 0x0000, 0x000B, 0x0009,
  OPFIND          ,       -1,       -1, G_STRING          ,   /* Object 1 OPINFO */
  SELECTABLE, NORMAL, (LONG)" Info...",
  0x0000, 0x0000, 0x000B, 0x0001,
         3,       -1,       -1, G_STRING          ,   /* Object 2 OPFIND */
  SELECTABLE, NORMAL, (LONG)" Find...",
  0x0000, 0x0001, 0x000B, 0x0001,
  OPCOPY          ,       -1,       -1, G_STRING          ,   /* Object 3  */
  NONE, DISABLED, (LONG)"-----------",
  0x0000, 0x0002, 0x000B, 0x0001,
  OPMOVE          ,       -1,       -1, G_STRING          ,   /* Object 4 OPCOPY */
  SELECTABLE, NORMAL, (LONG)" Copy",
  0x0000, 0x0003, 0x000B, 0x0001,
  OPDEL           ,       -1,       -1, G_STRING          ,   /* Object 5 OPMOVE */
  SELECTABLE, NORMAL, (LONG)" Move",
  0x0000, 0x0004, 0x000B, 0x0001,
         7,       -1,       -1, G_STRING          ,   /* Object 6 OPDEL */
  SELECTABLE, NORMAL, (LONG)" Delete",
  0x0000, 0x0005, 0x000B, 0x0001,
  OPFOLD          ,       -1,       -1, G_STRING          ,   /* Object 7  */
  NONE, DISABLED, (LONG)"-----------",
  0x0000, 0x0006, 0x000B, 0x0001,
  OPFREE          ,       -1,       -1, G_STRING          ,   /* Object 8 OPFOLD */
  SELECTABLE, NORMAL, (LONG)" Folder...",
  0x0000, 0x0007, 0x000B, 0x0001,
         0,       -1,       -1, G_STRING          ,   /* Object 9 OPFREE */
  SELECTABLE|LASTOB, NORMAL, (LONG)" Free",
  0x0000, 0x0008, 0x000B, 0x0001,
  
  /******** Tree 7 SORTLIST ****************************************************/
        -1, SORTNAME        ,        5, G_BOX             |0x5D00,   /* Object 0  */
  FLAGS10, SHADOWED, (LONG)0x00FF1100L,
  0x0000, 0x0000, 0x0008, 0x0005,
         2,       -1,       -1, G_STRING          ,   /* Object 1 SORTNAME */
  SELECTABLE, NORMAL, (LONG)"  Name",
  0x0000, 0x0000, 0x0008, 0x0001,
         3,       -1,       -1, G_STRING          ,   /* Object 2  */
  SELECTABLE, NORMAL, (LONG)"  Size",
  0x0000, 0x0001, 0x0008, 0x0001,
         4,       -1,       -1, G_STRING          ,   /* Object 3  */
  SELECTABLE, NORMAL, (LONG)"  Date",
  0x0000, 0x0002, 0x0008, 0x0001,
         5,       -1,       -1, G_STRING          ,   /* Object 4  */
  SELECTABLE, NORMAL, (LONG)"  Type",
  0x0000, 0x0003, 0x0008, 0x0001,
         0,       -1,       -1, G_STRING          ,   /* Object 5  */
  SELECTABLE|LASTOB, NORMAL, (LONG)"  None",
  0x0000, 0x0004, 0x0008, 0x0001,
  
  /******** Tree 8 FFIND ****************************************************/
        -1,        1,       11, G_BOX             |0x5D00,   /* Object 0  */
  NONE, WHITEBAK|STATE8|STATE12|STATE15, (LONG)0x00FD1100L,
  0x0000, 0x0000, 0x0022, 0x000A,
  FFINAME         ,       -1,       -1, G_STRING          |0x1300,   /* Object 1  */
  FLAGS9, NORMAL, (LONG)"Find file",
  0x0400, 0x0400, 0x0021, 0x0001,
         3,       -1,       -1, G_FTEXT           ,   /* Object 2 FFINAME */
  EDITABLE|FLAGS10, NORMAL, (LONG)&rs_tedinfo[59],
  0x0401, 0x0002, 0x001F, 0x0001,
  FFIHID          ,       -1,       -1, G_STRING          ,   /* Object 3  */
  NONE, STATE10|STATE12|STATE15, (LONG)"Can include wildcards * ? [] {}",
  0x0405, 0x0003, 0x0217, 0x0001,
  FFISHOW         , FFIFOO          , FFIFILE         , G_IBOX            ,   /* Object 4 FFIHID */
  NONE, NORMAL, (LONG)0x00001100L,
  0x0000, 0x0005, 0x0022, 0x0002,
  FFIFOLD         ,       -1,       -1, G_TEXT            ,   /* Object 5 FFIFOO */
  NONE, NORMAL, (LONG)&rs_tedinfo[60],
  0x0406, 0x0000, 0x0007, 0x0001,
  FFIFOF          ,       -1,       -1, G_BOXTEXT         ,   /* Object 6 FFIFOLD */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[61],
  0x040E, 0x0000, 0x000C, 0x0001,
  FFIFILE         ,       -1,       -1, G_TEXT            ,   /* Object 7 FFIFOF */
  NONE, NORMAL, (LONG)&rs_tedinfo[62],
  0x0408, 0x0001, 0x0005, 0x0001,
  FFIHID          ,       -1,       -1, G_BOXTEXT         ,   /* Object 8 FFIFILE */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[63],
  0x040E, 0x0001, 0x000C, 0x0001,
  FFINEXT         ,       -1,       -1, G_BUTTON          |0x1200,   /* Object 9 FFISHOW */
  SELECTABLE|EXIT, WHITEBAK|STATE8|STATE9|STATE12|STATE15, (LONG)"[Show",
  0x0002, 0x0008, 0x0008, 0x0201,
        11,       -1,       -1, G_BUTTON          ,   /* Object 10 FFINEXT */
  SELECTABLE|DEFAULT|EXIT, WHITEBAK|STATE8|STATE9|STATE12|STATE15, (LONG)"[Find Next",
  0x000C, 0x0008, 0x000A, 0x0201,
         0,       -1,       -1, G_BUTTON          |0x1200,   /* Object 11  */
  SELECTABLE|EXIT|LASTOB, WHITEBAK|STATE8|STATE9|STATE12|STATE15, (LONG)"[Cancel",
  0x0018, 0x0008, 0x0008, 0x0201,
  
  /******** Tree 9 FCOPY ****************************************************/
        -1, FCOPTITL        , FCOPHID1        , G_BOX             |0x5D00,   /* Object 0  */
  NONE, WHITEBAK|STATE12|STATE15, (LONG)0x00FD1100L,
  0x0000, 0x0000, 0x0422, 0x000B,
  FCOPFOLDO       ,       -1,       -1, G_STRING          |0x1300,   /* Object 1 FCOPTITL */
  FLAGS9, NORMAL, (LONG)"x",
  0x0400, 0x0400, 0x0021, 0x0001,
  FCOPFOLD        ,       -1,       -1, G_TEXT            ,   /* Object 2 FCOPFOLDO */
  NONE, NORMAL, (LONG)&rs_tedinfo[64],
  0x0406, 0x0002, 0x0007, 0x0001,
  FCOPFILEO       ,       -1,       -1, G_BOXTEXT         ,   /* Object 3 FCOPFOLD */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[65],
  0x040E, 0x0002, 0x000C, 0x0001,
  FCOPFILE        ,       -1,       -1, G_TEXT            ,   /* Object 4 FCOPFILEO */
  NONE, NORMAL, (LONG)&rs_tedinfo[66],
  0x0408, 0x0003, 0x0005, 0x0001,
  FCOPHID2        ,       -1,       -1, G_BOXTEXT         ,   /* Object 5 FCOPFILE */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[67],
  0x040E, 0x0003, 0x000C, 0x0001,
  FCOPHID1        , FCOPNAME        , FCOPCOP         , G_BUTTON          |0x1400,   /* Object 6 FCOPHID2 */
  FLAGS9, NORMAL, (LONG)"This name is already in use!",
  0x0001, 0x0005, 0x0020, 0x0005,
         8,       -1,       -1, G_FTEXT           ,   /* Object 7 FCOPNAME */
  EDITABLE|FLAGS10, NORMAL, (LONG)&rs_tedinfo[68],
  0x0001, 0x0001, 0x001E, 0x0002,
  FCOPSKIP        ,       -1,       -1, G_BUTTON          |0x1200,   /* Object 8  */
  SELECTABLE|EXIT, WHITEBAK|STATE8|STATE9|STATE12|STATE15, (LONG)"[Abort",
  0x0016, 0x0003, 0x0008, 0x0201,
  FCOPCOP         ,       -1,       -1, G_BUTTON          ,   /* Object 9 FCOPSKIP */
  SELECTABLE|EXIT, WHITEBAK|STATE8|STATE9|STATE12|STATE15, (LONG)"[Skip",
  0x000C, 0x0003, 0x0008, 0x0201,
  FCOPHID2        ,       -1,       -1, G_BUTTON          ,   /* Object 10 FCOPCOP */
  SELECTABLE|DEFAULT|EXIT, WHITEBAK|STATE8|STATE9|STATE12|STATE15, (LONG)"[Ok",
  0x0002, 0x0003, 0x0008, 0x0201,
         0, FCOPFOO         , FCOPFIGO        , G_IBOX            ,   /* Object 11 FCOPHID1 */
  NONE, NORMAL, (LONG)0x00001100L,
  0x0400, 0x0005, 0x0021, 0x0002,
  FCOPFOGO        ,       -1,       -1, G_TEXT            ,   /* Object 12 FCOPFOO */
  NONE, NORMAL, (LONG)&rs_tedinfo[69],
  0x0006, 0x0000, 0x000E, 0x0001,
  FCOPFIO         ,       -1,       -1, G_BOXTEXT         ,   /* Object 13 FCOPFOGO */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[70],
  0x0015, 0x0000, 0x0004, 0x0001,
  FCOPFIGO        ,       -1,       -1, G_TEXT            ,   /* Object 14 FCOPFIO */
  NONE, NORMAL, (LONG)&rs_tedinfo[71],
  0x0008, 0x0001, 0x000C, 0x0001,
  FCOPHID1        ,       -1,       -1, G_BOXTEXT         ,   /* Object 15 FCOPFIGO */
  LASTOB|FLAGS10, NORMAL, (LONG)&rs_tedinfo[72],
  0x0015, 0x0001, 0x0005, 0x0001
};

OBJECT *rs_trindex[] =
{ &rs_object[0],   /* Tree  0 FSEL             */
  &rs_object[87],   /* Tree  1 FFOLD            */
  &rs_object[92],   /* Tree  2 FINFO            */
  &rs_object[111],   /* Tree  3 PATHLIST         */
  &rs_object[133],   /* Tree  4 EXTLIST          */
  &rs_object[145],   /* Tree  5 FSHELPTXT        */
  &rs_object[179],   /* Tree  6 OPLIST           */
  &rs_object[189],   /* Tree  7 SORTLIST         */
  &rs_object[195],   /* Tree  8 FFIND            */
  &rs_object[207]    /* Tree  9 FCOPY            */
};
