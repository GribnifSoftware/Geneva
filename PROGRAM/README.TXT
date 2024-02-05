Files in this directory
-----------------------

The files here are intended for use primarily by people who are interested
in programming.

GNVA_DB.PRG (in the PROGRAM folder on the Master disk) is a special version
of GENEVA.PRG that includes debugging alerts which will appear when an
application uses an AES function call incorrectly. It traps things like a
bad parameter to wind_set(), a program attempting to close windows that do
not belong to it, etc. These things are simply ignored by the normal
version of Geneva. In some cases, Atari's AES will do unexpected things
when you pass bad values, so GNVA_DB can be quite useful. To disable a
particular alert, click on Ignore All. From that point on (until you
reboot), all alerts of that specific type will be ingored.

SETMOUSE.C is the C source code for the SETMOUSE.PRG which appears in the
GENEVA folder of ths disk.

NEW_GEM.LIB is a Pure C library containing all AES and VDI functions, as
well as Geneva's extended AES functions. It can be directly substituted for
the PCGEMLIB.LIB that comes with Pure C.

XWIND.H is an ANSI C include file which defines Geneva's extended
functions.

XWINDGFA.LST is a GFA BASIC file with the contents of XWIND.H translated
into BASIC.

NEW_AES.H is an ANSI C include file which defines several helpful
extensions to AES functions, and attempts to bring Pure C's AES.H up to
date with MultiTOS 5.0.

The MAKEHELP folder contains the MakeHelp utility for compiling
context-sensitive help files. See MAKEHELP.HLP for more info.

The TASKMAN folder contains the entire C source and resource definition
(HRD) for the Task Manager.

The SKELETON folder contains a C source file for creating new programs that
use Geneva's functions. By changing the #defines, you can modify the
program to work as a desk accessory and/or a program. You may find it
helpful to remove most of the unused #ifs once you have decided how you
want the code to be implemented.

The VECTEST folder contains source code for using Geneva's vectors. It
demonstrates how to access the vectors and use them properly.
