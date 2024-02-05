\screen Multitasking
Multitasking
------------

  Multitasking allows many applications to be used at the same time. You
  can launch your word processor, your telecommunications software and your
  desktop-publishing program, for example, and switch among them instantly.
  Even processes that require a lot of disk and computational activity can
  operate simultaneously with other applications under a properly designed
  multitasking system.

  Geneva's multitasking is called "cooperative multitasking." Under
  cooperative multitasking, all applications have equal access to your
  Atari's central processing chip, and the processor's time is apportioned
  to each application that needs attention.

  Another type of multiple program management, "pre-emptive multitasking,"
  imposes severe controls over each application's access to the computer's
  processing power. This provides a benefit of versatility in controlling
  applications, but at a cost of increased memory consumption and an
  overall slowdown in the speed of all applications. Geneva's cooperative
  multitasking keeps such side effects to a minimum.

Related Topics: \#Single-tasking\#
\end

\screen Single-tasking
Single-tasking
--------------

  Geneva provides a way to disable its normal way of running many
  applications at once. This is provided for older applications which may
  not be compatible with having more than one program use GEM's resources
  at one time.

  This mode can be entered by creating a program flag in the \#Task Manager\#'s
  \#Flags\# dialog.

Related Topics: \#Multitasking\#
\end

\screen Gribnif Software
  You can also reach Gribnif Software by electronic mail via GEnie (E-Mail
  address: "GRIBNIF"), CompuServe (E-Mail address:  "75300,1131"), and via
  Internet by sending mail to "gribnif@genie.geis.com".
\end

\screen Installing Geneva
\screen AUTO folder
A minimal configuration of Geneva consists of:

  JAR10.PRG in the AUTO folder
  GENEVA.PRG in the AUTO folder
  \#GEM.CNF\# in the GENEVA folder or root directory
  \#GENEVA.CNF\# in the GENEVA folder or root directory
  GNVA_TOS.PRG in the GENEVA folder
  GNVA_TOS.RSC in the GENEVA folder
  TASKMAN.ACC in the root directory
  TASKMAN.RSC in the root directory
  GNVAHELP.PRG in the GENEVA\\HELP folder
  GNVAHELP.RSC in the GENEVA\\HELP folder
  *.HLP in the GENEVA\\HELP folder

  Like many auto-running programs, Geneva should be run before some
  programs and after others, although the order is not critical. Here are
  some general guidelines.

  Programs you may want to place ahead of Geneva:

    Most AUTO-folder boot managers (SuperBoot, DeskManager, XBoot,
    Start-it, Elf Boot and others). Many of these programs cannot manage
    the auto-run selection of any programs that precede them in the AUTO
    folder.

    All so-called TOSFIX and POOLFIX programs.

    PinHead, the shareware fast-load manager.

    JARXXX (named JAR10, JAR015 and so on) from \#Gribnif Software\#.
    JARXXX should come before most other AUTO-folder programs, too.

  Programs that must run after Geneva:

    G+Plus, the replacement GDOS program from CodeHead Software.  Warp9,
    the screen accellerator from CodeHead Software.
\end

\screen JARxxx
What "JARxxx" Does
------------------

  JARXXX.PRG, which is placed in your \#AUTO folder\# by the installation
  program, will have numbers in the last two or three places of the name,
  such as JAR10 or JAR010. It creates a so-called "Cookie Jar" in the
  Atari's memory where programs can store information that can be accessed
  by all applications.

  It can be thought of as the computer's version of a small Post-It Note
  depository where programs can be informed about various things. In one
  example, an \#AUTO folder\# program could leave a cookie in the Cookie
  Jar to let a companion desk accessory know that it is running. Another
  example might be a cookie left by a special-purpose program to let any
  other application find out what hardware is installed on the computer.

Related Topics: \#AUTO folder\#
\end

\screen DESKTOP.INF
\screen NEWDESK.INF
\screen Bootup
\screen Desk Accessories
What Geneva Does During Bootup
------------------------------

  Geneva manages the bootup process by initializing itself, setting the
  screen resolution to the parameter specified in the computer's
  DESKTOP.INF or NEWDESK.INF files (or, if specified there, in the
  GENEVA.CNF), and loading all its special internal settings from two
  files -- \#GENEVA.CNF\# and \#GEM.CNF\#. (Defaults are used if one or
  both are not found.) These must be located in the root directory of the
  boot disk or in the GENEVA folder in the root directory of the boot disk.
  If GENEVA.CNF is present, the screen resolution information in that file
  is used instead of DESKTOP.INF or NEWDESK.INF.

  Geneva then loads all the desk accessories located in the directory
  specified in the \#ACCPATH\# statement in GEM.CNF. If no \#GEM.CNF\#
  exists, or if \#GEM.CNF\# does not have an \#ACCPATH\# statement, Geneva
  loads the accessories from the root directory of the boot disk. After the
  desk accessories have finished loading, Geneva runs any GEM program
  specified as the auto-running application in the computer's DESKTOP.INF
  or NEWDESK.INF file.

  Then Geneva checks for two more instructions in \#GEM.CNF\# on what
  programs to run, using the \#run\#, \#runsleep\# and \#shell\#
  statements. \#run\# can specify any number of programs to run (and
  \#runsleep\# tells Geneva to run them in suspended mode), whereas
  \#shell\# runs a desktop shell such as NeoDesk.  After the shell, if any,
  is loaded, Geneva switches to background mode. Its \#Executive Menu\# is
  always available as a menu item in the "Desk" menu of all GEM
  applications; clicking on the name \#Geneva Manager\# in that menu brings
  Geneva's \#Executive Menu\# to the foreground.

Related Topics: \#GEM.CNF\#  \#GENEVA.CNF\#  \#run\#  \#runsleep\#
                \#runacc\#   \#shell\#       \#Bypassing Geneva\#
\end

\screen Bypassing Geneva
Bypassing Geneva at Bootup
--------------------------

  Geneva checks for the presence of both the [Alternate] and [Control] keys
  during bootup. If it finds that [Alternate] is being held down, Geneva
  removes itself from the bootup process.  This lets you jump-start your
  computer under the regular operating system at any time, without needing
  to tinker with the \#AUTO folder\#.

  If Geneva finds that [Control] is being pressed, Geneva runs normally,
  but prevents the loading of any \#desk accessories\#. This is a quick way to
  bypass \#desk accessories\# that are misbehaving under Geneva.

Related Topics: \#Bootup\#
\end

\screen GEM.CNF
The GEM.CNF File
----------------

  GEM.CNF is a simple text file that you can edit to customize the bootup
  procedure. GEM.CNF must be located in the root directory of the boot disk
  or in the GENEVA folder; if Geneva is run from the desktop or from a
  shell, GEM.CNF must be located in the same folder (or root directory) as
  GENEVA.PRG.  Changes you make to GEM.CNF do not take effect until you
  reboot.

  Here is a sample GEM.CNF. Lines beginning with # are comments:

    #gem.cnf
    #last edit date: Sept. 15, 1993
    \#setenv\# PATH=.,c:\\geneva,c:\\system,c:\\rsc,d:\\utils
    setenv \#ACCPATH\#=c:\\deskacc
    setenv \#ACCEXT\#=ACC,ACX
    setenv \#GEMEXT\#=PRG,APP,GTP,EXE
    setenv \#TOSEXT\#=TOS,TTP
    setenv \#TOSRUN\#=c:\\geneva\\gnva_tos.prg
    setenv \#SHOWHELP\#=c:\\geneva\\help\\gnvahelp
    setenv STFAX=e:\\fax
    setenv TMP=d:\\temp
    setenv TEMP=d:\\temp
    \#run\# d:\\fileview\\1stview.prg d:\\aladdin\\messages.txt
    #run \\geneva\\setmouse\\setmouse.prg
    \#shell\# c:\\neodesk\\neodesk.exe

Related Topics: \#Bootup\#
\end

\screen setenv
\screen ACCPATH
\screen ACCEXT
\screen GEMEXT
\screen TOSEXT
\screen TOSRUN
\screen SHOWHELP
Environmental Variables in \#GEM.CNF\#
----------------------------------

  Most of the declarations in \#GEM.CNF\# use the "setenv" statement to set
  environmental variables. All environmental variables are global -- that
  is, they are available for use by all applications. The order of the
  "setenv" declarations is not important, although by convention the PATH
  statement is listed first.

  PATH: Describes the file paths (root directories and folders) where
  applications can locate certain files, expecially resource files.
  Applications search the paths in the order that they are listed.

  ACCPATH: Tells Geneva where to look for \#desk accessories\# when it starts
  up. The format for ACCPATH works like the format for PATH.

  ACCEXT: Lists the filename extensions of \#desk accessories\#. The standard
  entries, of course, are "ACC" and "ACX".

  GEMEXT: Lists the filename extensions of GEM applications. The usual ones
  are "PRG", "APP" and "GTP" (program, application and
  "GEM-Takes-Parameters" program, respectively).

  TOSEXT: Lists the filename extensions of TOS programs. They should always
  be "TOS" and "TTP".

  TOSRUN: Lists the full path and filename of the supervisory application
  Geneva should use when running TOS and TTP programs.  This usually points
  to the TOS-in-a-window utility, \#Geneva TOS\#.

  SHOWHELP: Lists the full path and filename of Geneva's built-in hypertext
  help program, GNVAHELP.

  Geneva will attempt to run a program in GEM mode if the filename
  extension is not found in the ACCEXT, GEMEXT or TOSEXT variables.

Related Topics: \#Bootup\#  \#GEM.CNF\#  \#run\#
\end

\screen run
\screen runsleep
\screen shell
\screen runacc
Running Programs in \#GEM.CNF\#
---------------------------

  "Run" lists programs that Geneva should launch as active applications,
  and "runsleep" lists programs that should be run in an inactive state.

  Any number of "run" and "runsleep" commands can be listed in the
  \#GEM.CNF\# file, each on a separate line. Further, a "command tail" can
  be passed to any program listed in a "run" command.

  "Runacc" is a special version of "run", just for desk accessories. It
  causes the desk accessory to be loaded into memory without opening. It
  is available to be opened in the "Desk" menu.

  The "shell" command is a special kind of "run" statement. The application
  named as the shell receives window-update messages from Geneva, whereas
  programs launched with the "run" command do not.

Related Topics: \#GEM.CNF\#  \#Installing Geneva\#  \#Bootup\#
\end

\screen GENEVA.CNF
GENEVA.CNF
----------

  Geneva creates GENEVA.CNF or alters its contents when you use the
  Save Settings option in the \#Task Manager\#.

  Although GENEVA.CNF is created and edited by Geneva, through the Task
  Manager, other applications can also add their own specific settings to
  it. Both the \#Geneva Help\# and the \#Geneva TOS\# utilities will add
  their own settings to GENEVA.CNF.

  GENEVA.CNF already has a series of recommended settings for many popular
  applications. You can add a new application to the list or change the
  settings of an application already listed at any time by using the
  \#Task Manager\#.

Related Topics: \#GEM.CNF\#  \#Installing Geneva\#  \#Bootup\#
\end

\screen Executive Menu
\screen Open
\screen File
\screen Desk Menu
\screen Desk
\screen Geneva Manager
\screen Geneva
Geneva's Executive Menu
-----------------------

  Any program can be run from Geneva's own Executive Menu, using the "Open"
  option under the "File" menu, or from the similar menu in the
  \#Task Manager\#.

  Opening the "Desk" menu will show a two-part list of running programs.
  The top section will always list Geneva along with any other currently
  running GEM applications. The application that is currently topped -- the
  one that controls the main GEM menu bar at that time -- will have a
  checkmark next to its name. Below a dotted rule is a list of installed
  \#desk accessories\#.

  An application that is running in \#single-tasking\# mode is listed in
  small type in the "Desk" menu; applications and accessories that are
  sleeping are listed in italic type. A single click on the name will bring
  an application back to life.

  Clicking on the name of any application in the first group makes it the
  foreground application; and clicking on the name of any desk accessory
  brings it to the screen, without changing the foreground status of the
  current application. You can also switch from one application to another
  by clicking once on the GEM window of the other application.

  This menu can also be "torn away" so that it will appear as a window on
  the desktop at all times.

  The Executive Menu also contains an option to produce an \#ASCII Table\#.

Related Topics: \#Quitting GEM Applications\#      \#Tear Away Menus\#
                \#Putting Applications to Sleep\#  \#Switching Applications\#
\end

\screen ASCII Table
ASCII Table
-----------

  Geneva provides a way to enter characters into an application that
  cannot normally be typed on the keyboard.

  If you either select the ASCII Table option from the \#Executive Menu\#,
  or press [Control][Alternate][Space], a small window with the entire
  character set will appear.

  Clicking on a character in this table will send that character to the
  Application whose window is on top. For this reason, you must make sure
  that a window belonging to the desired application is on top before
  selecting the character. You do not need to hold the right mouse button
  while selecting the character.

  While the left mouse button is held, the ASCII index of the selected
  character is displayed in the name bar of the ASCII Table window.

Related Topics: \#Executive Menu\#
\end

\screen Quitting GEM Applications
\screen Terminate
Quitting GEM Applications
-------------------------

  You should always attempt to exit a GEM application in the standard way,
  using "Quit" in the application's "File" menu, for example. If an
  application cannot be exited in the normal way, you can hold the
  [Control] key while clicking on the application's name in the \#Desk\#
  menu.

  Geneva will always warn you if terminating a GEM application could cause
  a problem. Some newer GEM programs are able to receive shutdown messages
  from a supervising system such as Geneva, but most older applications are
  not.

Related Topics: \#Executive Menu\#                 \#Tear Away Menus\#
                \#Putting Applications to Sleep\#  \#Switching Applications\#
\end

\screen Sleep
\screen Putting Applications to Sleep
Putting Applications to Sleep
-----------------------------

  Any running application or desk accessory can be suspended at any time.
  When this happens, communication between the sleeping program and the
  operating system is cut off; although it still occupies space in memory,
  it is no longer able to do anything, and cannot interact with the screen,
  the keyboard or any other software and hardware in the computer. Anything
  the application was doing at the moment it went to sleep is frozen, and
  will be resumed immediately when it is awakened.

  Geneva offers five ways to put a single program to sleep, and one way to
  put many programs to sleep at once. To suspend the operation of a single
  application or desk accessory, do one of the following:

    [Shift]-click on the name of the application or accessory in the
    \#Desk\# menu.

    Press [Control][Alternate][Z] while the application or accessory is the
    foreground task on the screen.  (This keyboard shortcut can be
    redefined within the \#Task Manager\#.)

    Select the name of the program in the \#Task Manager\#'s window and
    click on "Asleep" in the File menu.

    Select the name in the \#Task Manager\#'s window and press
    [Control][Z].

    Use the \#runsleep\# command in \#GEM.CNF\#.

Related Topics: \#Executive Menu\#          \#Putting Applications to Sleep\#
                \#Switching Applications\#
\end

\screen Switching Applications
Switching Applications
----------------------

  You can switch from one application to another using two different types
  of keyboard shortcut. The first shortcut, which by default is
  [Alternate][Tab], cycles foreground status among all running applications
  and all opened \#desk accessories\#.  The second is a user-defined hotkey
  that brings a specific application to the foreground.

  You can change the task-cycling hotkey combination using the
  \#Task Manager\#'s "Keyboard ..." menu under the "Options" menu.

  Geneva provides five different ways to switch among applications using
  the mouse:

    Clicking on any visible part of a window belonging to a background
    application.

    Clicking on the name of an application in the "Desk" menu.

    Double-clicking on the name of an application in the \#Task Manager\#
    window.

    Clicking on the name of an application in the \#Task Manager\# window
    and then choosing "Open" under the \#Task Manager\#'s "File" menu.

    Clicking on the "\#Cycle\# Windows" gadget (a left-facing double arrow) at
    the upper right corner of the topmost GEM window.

Related Topics: \#Executive Menu\#
\end

\screen Window Cycling
\screen Cycle Windows
\hidden Cycling Windows
\screen Cycle
Window Cycling
--------------

  All GEM windows with a Mover bar also have an extra gadget, in the upper
  right corner next to the "Full" gadget. It is called the "Cycle Windows"
  gadget, and looks like a double arrowhead or sideways chevron. Pressing
  this gadget when more than one window is open will bring the bottom
  window to the top. Windows that have a Name bar but do not have a "Full"
  gadget will still have a "Cycle Windows" gadget in the upper left corner.
  However, windows without a Name bar will not have a "Cycle" gadget.

  You can also cycle only the windows belonging to the current application
  by pressing [Shift][Alternate][Backspace]. This can be done with the
  mouse by holding either [Shift] key while clicking on the "Cycle Windows"
  gadget.

Related Topics: \#Resizing Windows\# \#Background Windows\#
\end

\screen Resizing Windows
\screen Resize
Resizing Windows
----------------

  Geneva improves window resizing in two ways. First, Geneva corrects an
  annoying idiosyncrasy of standard GEM windows to collapse slightly when
  you click on the "Resize" gadget at the bottom right corner of the
  window. An errant click makes the standard window smaller in both
  directions, and this is often not what you intended. Geneva prevents this
  by framing the window a little wider than its borders each time it
  detects a click on that gadget. If no dragging operation is detected, the
  window collapses back to its original size.

  Second, Geneva lets you resize a window to the left and up as well and to
  the right and down. In normal resizing, you are not likely to notice
  this; it is a feature that stays out of the way until you need it. The
  enhanced mode allows you to begin a normal resizing operation (by
  clicking on the "Resize" gadget and holding the left mouse button down)
  while dragging the pointer all the way to the left edge of the window. At
  the left edge, the outline for the right edge of the window will suddenly
  pop back to the right edge, and you will have switched into the enhanced
  window-sizing mode. You can continue to drag past the left edge or above
  the top of the window, or in any other direction, and the window will
  assume the size and shape of the dotted outline when you release the
  mouse button.

Related Topics: \#Cycling Windows\#  \#Background Windows\#
\end

\screen Background Windows
Operating on Background Windows
-------------------------------

  Every function that can be performed on a foreground window can be done
  on a background window by holding down the right mouse button while using
  the left button as usual. Background windows can be dragged, resized,
  closed and scrolled.

  Operating on background windows this way is much faster than topping them
  and performing the operation, because the computer does not have to
  redraw the display the same way it does when another window is topped.
  Note that a few applications may not allow you to manipulate their
  windows in the background. In these cases, there is nothing Geneva can do
  to correct the application's misbehavior.
\end

\screen Window Keyboard Shortcuts
Keyboard Shortcuts for Window Operations
----------------------------------------

  You don't need to use the mouse for most window operations.  Geneva's
  keyboard equivalents can be used for everything that does not involve
  dragging. Here is a list of the default keyboard shortcuts, all of which
  can be changed in the \#Task Manager\#'s \#Keyboard\# dialog in its
  "Options" menu:

    Close             [Alternate][Esc]
    Iconify           [Alternate][`]
    Iconify All       [Control][Alternate][`]
    \#Cycle\#             [Alternate][Backspace]
    Cycle Within App  [Shift][Alternate][Backspace]
    Full              [Alternate][Delete]
    Info Left         [Control][(]  keypad [(] only
    Info Right        [Control][/]  keypad [/] only
    Up                [Control][8]  keypad [8] only
    Page Up           [Shift][Control][8]  keypad [8] only
    Down              [Control][2]  keypad [2] only
    Page Down         [Shift][Control][2]  keypad [2] only
    Left              [Control][4]  keypad [4] only
    Page Left         [Shift][Control][4]  keypad [4] only
    Right             [Control][6]  keypad [6] only
    Page Right        [Shift][Control][6]  keypad [6] only

Related Topics: \#Menu Keyboard Shortcuts\#
\end

\screen Tear Away Menus
Menus can be "Torn Off" the Menu Bar
------------------------------------

  Any GEM dropdown menu can be turned into a stand-alone menu, in its own
  GEM window, by holding down the [Control] key while clicking on the
  menu's name in a GEM menu bar. The new window can then be moved anywhere
  on the screen, and will remain floating there until you quit the
  application it is associated with or until you click on the "Close"
  gadget at the upper left of the torn-off window. Each tear-away window
  also has a \#Cycle Windows\# gadget at the upper right corner as well.

  Geneva creates its own special tear-away menu when you hold down
  [Control] and click on the \#Desk\# menu of any application, including
  Geneva itself. The tear-away menu that appears is titled "Applications",
  and lists all applications currently running, just as the "Desk" menu
  does when it is not torn away.  The "Applications" menu is continuously
  updated by Geneva.

Related Topics: \#Menu Keyboard Shortcuts\#  \#Executive Menu\#
\end

\screen Menu Keyboard Shortcuts
Menu Keyboard Shortcuts
-----------------------

  You don't need to use the mouse to operate GEM menus.  Geneva switches
  immediately into its special keyboard mode when you press
  [Alternate][spacebar]. This drops down the foremost application's
  \#Desk\# menu.

    Start menu          [Alternate][spacebar]
    Up one entry        [up arrow]
    Down one entry      [down arrow]
    First entry         [Shift][up arrow]
    Last entry          [Shift][down arrow]
    Previous dropdown   [left arrow]
    Next dropdown       [right arrow]
    Select entry        [spacebar] or [Return]
    Close menu          [Esc] or [Undo]

  Another method  makes use of direct keyboard equivalents that Geneva
  creates for menu items when you press [Alternate][spacebar]. They are
  shown as underlines at the appropriate letter in each menu title.
  Pressing the letter "F" on the keyboard will open the "File" menu;
  pressing "E" will open the "Edit" menu, and so on.

  When you use the mouse in a menu that has been opened in keyboard mode,
  the menu reverts to normal operation.

Related Topics: \#Window Keyboard Shortcuts\#
\end

\screen Dialogs
\screen Dialog Improvements
Dialog Improvements
-------------------

  Dialogs -- the GEM boxes that appear when you are asked to make a choice
  or enter text in response to a menu option -- are greatly enhanced under
  Geneva. These improvements are both functional and cosmetic, and operate
  in all GEM applications and most \#desk accessories\#.

Related Topics: \#Dialog Keyboard Shortcuts\#  \#Default Button\#
                \#Movable Dialog Boxes\#       \#Alert Boxes\#
                \#Editing Text in Dialogs\#
\end

\screen Dialog Keyboard Shortcuts
\screen Default Button
Dialog Keyboard Shortcuts
-------------------------

  Most dialog-box choices that normally require the mouse can be made from
  the keyboard. This is always optional, and you can continue to use the
  mouse in the normal way if you wish. Here are the keyboard actions:

  Default Button:

    Normally, the default button in a dialog -- the action that occurs when
    you press [Return] -- is displayed with a thicker-than-usual border.
    This is typically the "Exit" button.

    Geneva lets you cycle the default status among all the buttons in the
    dialog by pressing [Alternate][Tab]. When you do this, you should see
    the thick border switching from one button to the next. When the
    default changes to the button you want, press [Return].

  Exit Button:

    The [Undo] key usually will cancel a dialog. Geneva scans the dialog's
    buttons for the presence of buttons labeled "Cancel", "Quit", "Exit",
    "Abort" or "No", and assigns the [Undo] keypress to the first one of
    the five that it finds.

  Shortcut Keys:

    Keyboard equivalents are automatically created for the buttons in
    dialogs, using the same technique described elsewhere for GEM menu
    equivalents. However, because dialogs often require the entry of text
    from the keyboard, the equivalents in dialogs are [Alternate]-key
    combinations, such as [Alternate][A] or [Alternate][B], instead of lone
    keypresses. The equivalent key is underlined in each button.

    The \#Task Manager\# has options which allow you to make the shortcut
    keys not require [Alternate] to be held.

  Arrows:

    Dialog buttons containing the up/down arrow characters can be activated
    by using the window up/down line window keyboard equivalents. By
    default, these are [Control][keypad 8] for up and [Control][keypad 2]
    for down.

  Cut/Paste:

    When a dialog has an edit cursor, these options will cut/paste to the
    Atari clipboard:

      Cut:   [Shift][Alternate][X]
      Copy:  [Shift][Alternate][C]
      Paste: [Shift][Alternate][V]

Related Topics: \#Dialog Improvements\#
\end

\screen Movable Dialog Boxes
Moveable Dialog Boxes
---------------------

  When you are running applications that make use of a dialog technique
  employed by Geneva, you will be able to move dialog boxes to any location
  on the screen. Geneva creates a dialog box that has a small symbol of two
  overlapping rectangles at the upper right. Press and hold this "Mover"
  gadget with the left mouse button and drag the box to another location.

  You can make the box temporarily invisible by holding down the right
  mouse button and then pressing the left button over the Mover gadget.
  When both buttons are released, the dialog will appear again.

  If you double-click on the Mover gadget, the dialog will be centered on
  the screen.

Related Topics: \#Dialog Improvements\#
\end

\screen Alert Boxes
Alert Boxes
-----------

  Alert boxes may have one to three buttons.  In addition to the normal
  \#Dialog Keyboard Shortcuts\#, alerts also respond to two other sets of
  shortcut keys -- the [1], [2] and [3] keys on the keyboard or keypad and
  the [F1], [F2] or [F3] keys in the top row of the keyboard. The numbers
  correspond to the order of the buttons, from left to right.

  You can also configure the \#Task Manager\# to make alert boxes appear
  centered under the current position of your mouse pointer, using the
  \#Misc. Options\# dialog under "Options".

Related Topics: \#Dialog Improvements\#
\end

\screen Editing Text in Dialogs
Editing Text in Dialogs
-----------------------

  As with the standard GEM system, these functions operate on text fields
  in a dialog box:

    \x3 The [Tab] and down-arrow keys move to the next editable field.
    \x3 The up-arrow key moves to the previous editable field.
    \x3 The left and right-arrow keys move the cursor within a field.
    \x3 The [Esc] key clears an editable field.
    \x3 The [Backspace] and [Delete] keys delete characters before and
      after the cursor.

  Geneva adds these enhancements:

    \x3 [Tab] and down-arrow will move the cursor to the first editable
      field in a dialog box if it is already at the last one.
    \x3 Up-arrow and [Shift][Tab] will also wrap, from the first field
      to the last.
    \x3 [Shift] left-arrow will move the cursor to the leftmost character,
      and [Shift] right-arrow will move all the way to the right.
    \x3 [Control] left arrow and [Control] right arrow move left or right one
      word.
    \x3 [Shift][Delete] deletes to the end of the line.
    \x3 [Shift][Backspace] deletes to the beginning of the line.
    \x3 [Return] moves the cursor to the next editable field if there is no
      default button in a dialog box.
    \x3 [Shift][Alternate][X] cuts the text to the Atari clipboard.
    \x3 [Shift][Alternate][C] copies the text to the clipboard.
    \x3 [Shift][Alternate][V] pastes from the clipboard.

  The mouse will position the cursor to the character closest to where the
  mouse button was pressed, not just to the end of the line.

  The [Insert] key toggles insert and overwrite modes. Insert mode is shown
  by the thin vertical cursor; overwrite mode is shown by a large
  rectangular cursor.

Related Topics: \#Dialog Improvements\#  \#Entering ASCII Characters\#
\end

\screen Entering ASCII Characters
Entering ASCII Characters in a Dialog
-------------------------------------

  Geneva permits the entry of characters that are not in the normal
  keyboard set of letters, numbers and punctuation marks.  If the
  application that is showing the dialog box allows such characters, you
  can enter them two ways:

  Press the [Clr/Home] key, which will open a box containing the full
  character set available on your Atari. Click on the character or symbol
  you want to appear in the edit field, or click anywhere outside the box
  to cancel.

  Use the ASCII value of the character, if you know it, by holding down the
  [Alternate] key while typing the ASCII value on the keypad. When you
  release the [Alternate] key, the character or symbol will appear in the
  edit field.

Related Topics: \#Dialog Improvements\#
\end

\screen Item Selector
The Item Selector
-----------------

  Geneva's Item Selector offers many improvements over the one that is
  normally part of Atari's operating system:

  Related Topics:
    \#ALL\#                     \#Closing Folders\#
    \#EXT Button\#              \#One-column mode\#
    \#Quick-Search Mode\#       \#SORT Button\#
    \#Scrolling\#               \#Selecting Files\#
    \#Selecting Folders\#       \#Switching Paths\#
    \#TOOLS Button\#            \#UPDATE Button\#
    \#Wildcards\#
\end

\screen One-column mode
\screen Two-column mode
One-column mode
---------------

  The item selector normally shows a single listing of files and folders.
  If the item selector is being used in its two-column mode, clicking on "1
  COL" at the upper left switches the file selector to its other mode, in
  which folders are listed before files in a single window. To the right of
  the names are entries for size and creation dates.

  To switch to the dual-column display, click on "2 COL" at the upper left.

Two-Column Mode
---------------

  This mode lists folders in the left column and files in the right. A
  single click on any folder in the left column opens it to show its
  contents on the right, and [Shift]-click selects a folder without opening
  it.

  In two-column mode, Geneva shows only the folder names and filenames when
  the lists appear, but the display can be switched instantly to show the
  folder or file size, creation date and creation time. The four types of
  display are denoted by a small letter, which can be "n", "s", "d" or "t",
  above the top scroll arrow in each listing window. Clicking on the letter
  switches the display to the next one in the sequence.

Related Topics: \#Item Selector\#
\end

\screen Selecting Files
\screen Selecting Folders
\screen ALL
Selecting and Opening a File
----------------------------

  To open a folder in the item selector, click once on its name. To select
  a folder without opening it, press [Shift] while clicking.

  Clicking once on a file in the item selector selects it.  Clicking on the
  "OK" button or pressing [Return] opens the file or performs whatever
  operation for which the file was selected.  Double-clicking on a file
  produces the same result. You can deselect a file by clicking on it again
  or by clicking on another area of the window.

  If a file is read-only, a checkmark will appear in front of its name in
  the item selector. This only applies to 1 column mode.

  Some applications, such as Geneva itself when it launches programs, will
  let you open more than one file at a time.  Geneva's item selector lets
  you select any number of files at once. To select more than one file,
  hold down [Shift] while clicking on the additional filenames. Deselect a
  file in a group with the same combination.  Note that you can select all
  files by clicking on the "ALL" button, also.

Related Topics: \#Item Selector\#  \#Closing Folders\#
\end

\screen Scrolling
Scrolling the Item Selector Lists
---------------------------------

  Scroll arrows at the top and bottom of the scroll bars increment the list
  by one entry when clicked once, and continue scrolling when you hold down
  the mouse button. Double-clicking on the arrows scrolls the list to the
  top or bottom. Clicking on the area of the scroll bar above or below the
  rectangular scroll gadget pages the list forward or backward by 10
  entries, if the list can be extended that far.

  Using the mouse to drag the scroll gadget up or down produces real-time
  scrolling, at whatever speed you are dragging the scroll gadget. This is
  a handy way to browse through a long list of folders or files.

Related Topics: \#Item Selector\#
\end

\screen Closing Folders
Navigating Out of Folders in the Item Selector
----------------------------------------------

  Clicking on the "Close" gadget (\x5) at the top left of the file window
  closes the current folder and backs out one level in the hierarchy of
  folders. If you are already at the root level, the "Close" gadget will
  have no effect.

  Clicking on the backslash (\\) to the right of the "Close" gadget will
  immediately bring the list back to the root level. To back out to an
  intermediate level, double-click on the folder name in the path window at
  the top of the file selector. Pressing the key combination
  [Alternate][\\] has the same effect.

Related Topics: \#Item Selector\#  \#Selecting Folders\#
                \#Switching Paths\#
\end

\screen Switching Paths
Switching Paths
---------------

  The "PATH:" button, at the left of the path editing line, opens a pop-up
  menu that will be blank (except for backslashes) when you first open it.

  A [Shift]-click will place the current item-selector path into any
  highlighted slot in the pop-up menu.  The arrow keys can be used to
  scroll to any blank path slot before you [Shift]-click to install the
  path, or you can use the mouse to place the cursor at one of the
  positions. The current path will replace the path previous stored if you
  [Shift]-click on a line that is not blank. You can save up to 10 paths.
  Paths are not saved for the next session unless you save Geneva's
  settings through the Task Manager.

  Any folder which is part of the current path can be jumped to by double-
  clicking on that portion of the PATH: line.

Related Topics: \#Item Selector\#  \#Selecting Folders\#
                \#Closing Folders\#
\end

\screen UPDATE Button
The UPDATE Button
-----------------

  The "UPDATE" button to the right of the one-column or two-column mode
  button forces the operating system to reread all information from the
  current drive. Press it any time you have inserted a new desk in a floppy
  drive, a Floptical drive or a removable hard drive and are not sure if
  the operating system has recognized the change.

Related Topics: \#Item Selector\#
\end

\screen SORT Button
The SORT Button
---------------

  The "SORT" button, to the right of the \#UPDATE Button\#, lets you choose
  how files are sorted in the item selector's display.  The current sorting
  method is shown with a checkmark. Clicking on any of the choices or
  clicking outside the "SORT" dialog closes the pop-up display. The options
  are sorting by "Name", "Type", "Size", "Date" and "None" (no sorting).
  They work this way:

    "Name"  Alphabetically by the main filename.
    "Type"  Alphabetically by the filename extension.
    "Size"  From the largest to smallest.
    "Date"  From the newest to the oldest.
    "None"  The physical order on the disk.

  The sorting method is saved in Geneva's configuration through the
  \#Task Manager\#. Note that folders are also sorted, but sorting by size
  has the same effect as sorting by name, since folders all have the same
  size (zero bytes).

Related Topics: \#Item Selector\#
\end

\screen TOOLS Button
\hidden TOOLS menu
The TOOLS Button
----------------

  The "TOOLS" button, to the right of the \#SORT Button\#, opens a pop-up
  menu that provides seven file utilities. Most of them will be grayed out
  unless you first select a file or folder. The options are \#Info\#,
  \#Find\#, \#Copy\#, \#Move\#, \#Delete\#, \#Folder\# and \#Free\#.

Related Topics: \#Item Selector\#
\end

\screen Info
\screen File information
Info
----

  Opens a "File information" dialog showing the name, size and creation
  time and date of the selected file. If a folder is selected, the dialog
  shows the name, creation time and date, the number of folders within that
  folder, if any, and the total number and size of files within that
  folder.

  Files can be renamed from the dialog, as can folders on Ataris running
  TOS 1.04 or greater. If you rename a file or folder, click on "OK" to
  make the change permanent, or click on "Skip" or "Abort" to cancel the
  change. "Abort" is also used to exit the dialog at any time, and "Skip"
  will also exit if you have only one item selected. If you have selected
  multiple files or folders, "Skip" calls up the listing for the next item.

  A file's read-only status can also be changed or set in this dialog. If
  a file is read-only, a checkmark will appear in front of its name in the
  main item selector dialog. This only applies to 1 column mode.

  Normally, clicking on the name of a folder opens the folder. To select it
  for the \#TOOLS menu\#, hold down [Shift] while clicking on the name.

  A quiker way to open the Info dialog is to hold the [Control] key while
  clicking on the name of a file or folder with the mouse.

Related Topics: \#Item Selector\#  \#TOOLS menu\#
\end

\screen Find
Find
----

  This option will allow you to search for a particular filename.  This is
  very helpful when you know approximately what the name of a file is, but
  are not completely certain.

  You are presented with a dialog, into which you can enter a string of
  characters to describe what filenames to search for.  You can enter any
  of the normal filename characters, as well as any of the
  \#wildcard characters\#.

  The "Find Next" button will begin searching at the current directory, and
  traverse any folders, looking for the first filename that matches the
  string you provided. Once a filename is found, you have the option to
  either "Show" the file's location, "Find" the next match, or "Abort" the
  entire operation.

Related Topics: \#Item Selector\#  \#TOOLS menu\#
\end

\screen Copy
Copy
----

  This option will copy any number of files and folders to a new location.
  Begin by selecting the items you want to copy. Then, select the "Copy"
  option in the \#TOOLS menu\#. At this point, the display will change,
  reminding you that the item selector is now looking for the destination
  path (the location you want to copy the files to.)

  Once you have changed to the correct destination path, using the normal
  method of going from one folder to another, you can click on the "Ok"
  button at the bottom of the item selector to proceed with the copy
  operation. To cancel at any time, select the "Cancel" button.

  As files are copied, you will see a display of the current status. If
  another item already exists with the same name as a file or folder you
  are copying, you will be given an error message describing the conflict.
  At this point, you can either select "Ok" to have the existing copy of
  the file overwritten, change the name and press "Ok" to provide a new
  name for the new copy of the file, "Skip" the current item, or "Abort"
  the entire operation.

  To select several items to be copied, just hold down either [Shift] key
  while clicking with the mouse on the names of the items.

Related Topics: \#Item Selector\#  \#TOOLS menu\#
\end

\screen Move
Move
----

  This works identically to the way \#Copy\# does, except that the original
  version of the file is deleted afterward.

Related Topics: \#Item Selector\#  \#TOOLS menu\#
\end

\screen Delete
Delete
------

  Choosing "Delete" opens a dialog that looks like the \#File information\#
  dialog, with the substitution of a "Delete" button for the "OK" button. A
  warning text is added to make sure you want to erase the file. Chose
  "Skip" or "Abort" to exit the dialog.

Related Topics: \#Item Selector\#  \#TOOLS menu\#
\end

\screen Folder
Folder
------

  This opens a text-entry box where you can type in the name of a new
  folder. The folder will be created within the current directory. Choose
  "Cancel" to exit.

Related Topics: \#Item Selector\#  \#TOOLS menu\#
\end

\screen Free
Free
----

  The "Free" option displays the total space and free space on the current
  drive. The listing is in bytes.

Related Topics: \#Item Selector\#  \#TOOLS menu\#
\end

\screen EXT Button
The EXT Button
--------------

  The "EXT" button, to the right of the \#TOOLS button\#, pops up a menu of
  filename extensions (the three characters that follow the period in a
  filename).

  These are preset by default, but any of them can be changed by a method
  like the one used to enter paths. To enter an extension, choose the one
  you want and make sure it shows up in the path window at the top of the
  item selector. Click on the "EXT" button and use the arrow keys to scroll
  to the extension you want to replace, and then do a [Shift]-click.

  To insert one of the extensions into the item selector, open the "EXT"
  menu and click on the extension you want. Remember to save Geneva's
  settings through the \#Task Manager\# if you want to save the changes you
  have made to the extension menu.

  The first entry in the "EXT" popup, "Start", allows you to quickly return
  to the original template that was used when the item selector first
  opened.

Related Topics: \#Item Selector\#
\end

\screen Quick-Search Mode
Quick-Search Mode
-----------------

  Pressing [Tab] or [Shift][Tab] or one of the [Control]-key combinations
  listed below will switch Geneva's item selector to its quick-search mode.

  Using the Arrow Keys:

    After pressing [Tab] or [Shift][Tab], these operations are possible:

    \x3 Scrolling: The [up-arrow] and [down-arrow] keys move the cursor
      within the file list. The list will scroll, file by file, in either
      direction.
    \x3 Paging: Using the [Shift] key with the arrow keys scrolls the list
      one page (10 files) at a time.
    \x3 Go to end: [Shift][Clr/Home] scrolls to the bottom of the list.
    \x3 Go to start: [Clr/Home] scrolls to the top of the list.

  Entering a Search Character:

    The [Tab] or [Shift][Tab] keys also activate the character-search mode.
    Typing a single letter from A to Z moves the cursor to the folder name
    or filename that begins with the first matched character.  Another way
    of performing a character search is to press [Control] and the
    character, such as [Control][G].

    To search again if Geneva has found a match, press [Esc] and use the
    search procedure again. If you wish, you can use the [Tab] or
    [Shift][Tab] keys instead of the [Esc] key to cycle the cursor through
    the item selector's fields to return to the folder or file lists. If
    the item selector is in one-column mode, press the [Tab] key three
    times to return to character-search mode in the list. If the item
    selector is in two-column mode, press the [Tab] key three times or four
    times, depending on which list (folders or files) you want to search.

    Pressing [Return] when Geneva has located a file or folder through
    character-search mode performs the desired operation if the item is a
    file, or opens it if it is a folder.

Related Topics: \#Item Selector\#
\end

\screen Wildcards
\screen wildcard characters
Using Wildcards
---------------

  The Atari operating system, uses standard wildcard characters to
  represent any single character (?) or any group of multiple characters
  (*) in a file or folder name.  Geneva adds two other wildcard
  expressions, square brackets and curly brackets. It also allows for the
  negation of any expression, with the exclamation point (!).

  * (asterisk): Represents any group of characters or no characters at all.
  Under Geneva, the asterisk can be placed on either side of a character;
  under the standard method, characters to the right of an asterisk are
  ignored.

  ? (question mark): Represents a single character. Unlike the asterisk,
  the question mark cannot stand for a null (no character). You can use
  multiple question marks.

  [  ] (square brackets):  Represent a list of characters that can each
  substitute for a single position in a filename. The list can be
  individual characters, a range of characters separated by a hyphen, or a
  combination of these two methods.

  {  } (curly brackets): Represent a list of character strings or words,
  separated by commas, that can be substituted for the main filename or the
  extension. The entire main filename or extension must be given.  The
  [...] wildcard can be used inside the {...} wildcard, but the [...]
  wildcard cannot contain the {...} wildcard.

  Examples:

    *.TXT           Finds all files with ".TXT" extensions.
    A*.DOC          Finds all files with names beginning with "A" that have
                    any (or no) characters following the "A" and have "DOC"
                    as the extension.
    MYFILE.?X?      Finds all files with the primary name "MYFILE" and any
                    three-letter extension that contains "X" as the middle
                    character.
    *.P[IC][1-3]    Finds all Degas-format pictures, which by convention
                    must have an extension that begins with a "P" and has
                    either an "I" or a "C" as the second character and a
                    number from "1" to "3" as the third character.
    [A-CZ]*.PRG     Finds all files whose primary names begin with "A",
                    "B", "C" or "Z" and have "PRG" as an extension.
    *.{PRG,TOS}     Finds all files that have "PRG" and "TOS" extensions.
    *.{AC[CX],PRG}  Finds all files that have extensions that match "ACC",
                    "ACX" and "PRG".
    NOTE.{DOC,TXT}  Finds all files with the primary name "NOTE" that have
                    "DOC" and "TXT" extensions.

  The exclamation point (!) can be used for inversion ("not") of the next
  item in a string. If the next item is just a regular character, then that
  one letter will be inverted; if the next item is a {} or [] bracketed
  expression, then the meaning of that expression will be inverted.
  Examples:

    *.PRG           Ending in "PRG"
    *.!ABC          *.?BC, excluding *.ABC
    *.AB!C          *.AB?, excluding *.ABC
    *.!A!BC         *.??C, excluding *.ABC
    [A-D]*.*        Starting with "A", "B", "C", or "D"
    ![A-D]*.*       Anything not starting with "A", "B", "C", or "D"
    *.{PRG,PRX}     Ending in "PRG" or "PRX"
    *.!{PRG,PRX}    Anything not ending in "PRG" or "PRX"

  Any number of !'s can be used within a wildcard. The effect is that the
  expressions are negatively added together. For example:

    !{STALKER,NEODESK?}.!{AC[CX],RSC}

  Matches anything but:

    STALKER.ACC    STALKER.ACX    STALKER.RSC
    NEODESK?.ACC   NEODESK?.ACX   NEODESK?.RSC

  However, NEODESK.ACC will match, since it does not have a character
  between the "K" and the period.

Related Topics: \#Item Selector\#  \#Find\#
\end
