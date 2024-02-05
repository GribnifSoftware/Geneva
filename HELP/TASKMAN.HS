\casesens General Description
\casesens Task Manager
The Geneva Task Manager
-----------------------

  The Task Manager allows you to load and \#terminate\# programs and desk
  accessories at any time. It also gives you access to all of Geneva's
  configurable options.

  The main window contains a list of all currently running applications,
  (including the Task Manager, itself). Desk accessories have a small dot
  before their names. If a name appears in italics, this means that it has
  been put "to \#sleep\#", and is not receiving events. If a name appears in
  small text, this means that it is a \#single-tasking\# process.

  A name in this list can be chosen with the mouse. Once a name is
  selected, you can use the \#Open\#, \#Temporary\#, \#Permanent\#,
  \#Asleep\#, and \#Terminate\# options to affect the task whose name has
  been chosen. Double-clicking on a name causes it to be opened
  immediately. Clicking anywhere in the list where there is not a name, or
  on a name which is already selected, will de-select the name.

  The main window can be "iconized" by clicking on the "delta" gadget.
  This shrinks any open windows to a single "icon" window. Double-clicking
  on the icon window's contents will reopen the Task Manager.

Related topics: \#Menu Options\#
\end

\casesens Menu Options
\casesens File Menu
\casesens Options Menu
Geneva Task Manager menu options:

     File Menu       Options Menu        Settings Menu

     \#Open\#           \#Keyboard Equivs\#    ----- Geneva ----
    -- Flags --     \#Window/Menu Bar\#     \#Save Settings\#
     \#Temporary\#      \#Window Colors\#       \#Reload Settings\#
     \#Permanent\#      \#Dialog Colors\#      -- Task Manager -
     \#Asleep\#         \#Misc. Options\#       \#Save Positions\#
     \#Terminate\#      \#Video\#
     \#Quit\#

Related topics: \#Task Manager\#
\@slinewidth 4
\@line 3 1 15 1 15 11 3 11 3 1
\@line 3 3 15 3
\@line 18 1 35 1 35 11 18 11 18 1
\@line 18 3 35 3
\@line 38 1 56 1 56 11 38 11 38 1
\@line 38 3 56 3
\end

\casesens Opening a Task
\casesens Open
Open menu option
----------------

  If a name is selected in the main window, this option opens (makes
  active) the task. If no name is selected, then an item selector appears
  so that you can load a new task. Up to five tasks can be selected at one
  time by holding the Shift key while choosing the names in the item
  selector; the Task Manager will open them all at once.

  If a task which was started by the Task Manager terminates with an error
  status, the Task Manager will present you with an alert box describing
  the message.

Related topics: \#Menu Options\#
\end

\casesens Program Execution Flags
\casesens Execution flags
\casesens Flags
\hidden Temporary
\hidden Permanent
\casesens Temporary execution flags
\casesens Permanent execution flags
Flags
-----

  Geneva maintains two types of execution flags, temporary and permanent.
  Permanent flags are what Geneva gives to a program when it is first
  started. Once a program has started, its temporary flags can be changed,
  but not saved permanently.

  The "Temporary" option is only available when the name of a task has been
  selected in the main list. Changes made in this mode are only valid
  while the task remains in memory; they cannot be saved permanently.

  The "Permanent" option changes the flags for any new instances of the
  task whose name matches the chosen name. It will NOT change the flags for
  a task that is already running.

  If you start by selecting a task's name from the main list and then
  choose "Permanent", then the permanent flags that Geneva would use for
  that application are displayed by default.

  In the "Permanent" mode, the entire list of flags can be edited. In this
  case, the "Name" field specifies the filename(s) (with optional wildcard
  characters * and ?) of the file(s) which will get the current flags when
  executed. For example, "*.TOS" will give all TOS programs the same flags.

  In Permanent mode, there is a "?" button which gives you an item
  selector which allows you to locate the exact filename of the
  application. Since Geneva does not keep track of the full path of the
  application, only its name, you will have to manually go to the path of
  the application each time this option is used.

  The "Description" field is mostly there for your reference. It should
  contain some useful information about the name or particular version of
  the program that the current flag is intended for. If an application
  does not provide its own name, and there is a "Description" for it, this
  text is used in the Desk menu application list.

  You can also add a "New" set of flags for an application, or delete the
  current set with the "Delete" button. The "Find" button will allow you to
  search for a particular program name.

  When you use the \#Save Settings\# option to rewrite Geneva's settings
  to disk, the entire list of program flags is sorted alphabetically. This
  means that the next time you look at the list in the Task Manager, it
  will be easier to find a particular program.

  Descriptions of program flags:

    \#Multitask\#
    \#Limit window handles\#
    \#Limit memory\#
    \#Clear allocated memory\#
    \#Windows off left edge\#
    \#Maximize window areas\#
    \#Optimize window redraws\#
    \#Redraw upon exiting\#
    \#Special object types\#
    \#Rounded buttons\#
    \#Automatic keyboard equivs\#
    \#Undraw auto keyboard equivs\#
    \#Keep menu bar/desktop\#
    \#AES 4.0 extended messages\#

Related topics: \#Menu Options\#  \#Program Keys\#  \#What Flags to Change\#
\end

\casesens Multitask
Multitask flag
--------------

  If this option is off, then the application will be the only program to
  have access to the menu bar or have windows open. While the program is
  running, you will still be able to use desk accessories, but not other
  programs. When the program quits, multitasking mode will resume. This
  option is provided to improve compatibility with older applications
  written before multitasking was possible.

Related topics: \#Execution flags\#  \#What Flags to Change\#
\end

\casesens Limit window handles
Limit window handles flag
-------------------------

  Some older programs always count on windows being numbered one through
  seven, and never anything else. If this option is on, the application
  will only be able to open seven windows, and they will have the correct
  numbers (as far as the application is concerned).

Related topics: \#Execution flags\#  \#What Flags to Change\#
\end

\casesens Limit memory
Limit memory flag
-----------------

  If this option is selected, the program will only be able to ask for a
  certain number of kilobytes of memory. Some programs take all of the
  system memory when they first run (like Tempus) and this option will
  prevent this from happening.

Related topics: \#Execution flags\#  \#What Flags to Change\#
\end

\casesens Clear allocated memory
Clear allocated memory flag
---------------------------

  If this option is selected, then any memory the program asks for after
  it has initially run will be cleared by Geneva before the program access
  it. This is necessary because some older programs assume that any memory
  they allocate will be completely cleared.

  Turning this option on can produce a noticeable decrease in speed.
  However, if your version of TOS allows programs to be Fast Loaded (i.e.:
  if you have TOS 1.4 or newer) then you can often increase program
  performance by setting both this program flag and the Fast Load attribute
  of the file.

Related topics: \#Execution flags\#  \#What Flags to Change\#
\end

\casesens Maximize window areas
Maximize window areas flag
--------------------------

  Geneva can automatically make the working area of a window (the part
  inside the border) larger by not showing unused window gadgets. If this
  option is turned off, Geneva will act exactly like older versions of
  Atari's operating system.

Related topics: \#Execution flags\#  \#What Flags to Change\#
\end

\casesens Optimize window redraws
Optimize window redraws flag
----------------------------

  This option speeds up window redrawing. Because many programs will
  experience redraw problems when this option is on, it can be set
  individually for any program.

  Note that even when a program responds correctly to this method, it is
  still possible for there to be cosmetic redraw errors because of the way
  GEM works.

  For instance, if you move a window that is off the right edge of the
  screen 3 pixels to the left, the area that is redrawn will be in the
  correct pattern, but it will not match up with the rest of the window.
  The effect of this can be decreased by not using patterns in window
  gadgets.

Related topics: \#Execution flags\#  \#What Flags to Change\#
\end

\casesens Windows off left edge
Windows off left edge flag
--------------------------

  If this option is off, you will not be able to drag windows belonging to
  the application off the left edge of the screen. Certain programs
  experience problems in redrawing when a window is off the left edge. The
  default state for this flag is off.

Related topics: \#Execution flags\#  \#What Flags to Change\#
\end

\casesens Redraw upon exiting
Redraw upon exiting
-------------------

  When this flag is on, all windows belonging to all applications will
  be redrawn when the program quits. This is needed, because some older
  programs leave portions of their data on the screen when they exit.

  The default state for this flag is On.

Related topics: \#Execution flags\#  \#What Flags to Change\#
\end

\casesens Special object types
Special object types flag
-------------------------

  If this option is off, none of the extended dialog object types (like the
  circular radio buttons in the Task Manager) will appear. This is helpful
  when you have a program which uses the extended object types for its own
  purpose. If you find checkboxes and circular radio buttons appearing
  where they shouldn't, try turning this option off.

Related topics: \#Execution flags\#  \#What Flags to Change\#
\end

\casesens Rounded buttons
Rounded buttons flag
--------------------

  Normally, Geneva draws rounded buttons in dialogs. This option allows you
  to disable the feature for programs which do not look correct when it is
  active.

Related topics: \#Execution flags\#  \#What Flags to Change\#
\end

\casesens Automatic keyboard equivs
Automatic keyboard equivs flag
------------------------------

  When a program asks Geneva to handle a dialog, Geneva tries to assign
  keyboard equivalents to the exit buttons. Since some dialogs look odd
  with these small lines underneath the letters of the exit buttons, the
  option can be turned off.

Related topics: \#Execution flags\#  \#What Flags to Change\#
\end

\casesens Undraw auto keyboard equivs
Undraw auto keyboard equivs flag
--------------------------------

  If the \#Automatic keyboard equivs\# option is on, and this one is off,
  the keyboard equivalent lines will not be undrawn every time Geneva
  returns control to the program. This is faster than having the option on,
  but can cause the dialog to look as though it has multiple keyboard
  equivalents in the same button.

Related topics: \#Execution flags\#  \#What Flags to Change\#
\end

\casesens Keep menu bar/desktop
Keep menu bar/desktop flag
--------------------------

  Some programs routinely turn off the menu bar at various times when they
  are running. Since Geneva normally takes this as a cue to find another
  application which has a menu bar and make it active, this can cause the
  original application's windows to become untopped. Therefore, when this
  option is on, Geneva will not search for another application to give the
  menu bar to when the application gives up possession of the menu bar.

Related topics: \#Execution flags\#  \#What Flags to Change\#
\end

\casesens AES 4.0 extended messages
AES 4.0 extended messages flag
------------------------------

  If this option is off, the application will not receive any of the new
  messages defined by Atari for their AES version 4.0. Some old
  applications either use these same message numbers for their own purposes
  or they act strangely when one of them is received.

  Technically speaking, this blocks the following messages:  WM_UNTOPPED,
  WM_ONTOP, AP_TERM, AP_TFAIL, AP_RESCHG, SHUT_COMPLETED, RESCH_COMPLETED,
  AP_DRAGDROP, SH_WDRAW, CH_EXIT.

Related topics: \#Execution flags\#  \#What Flags to Change\#
\end

\casesens Program Keys
\casesens Keyboard settings
\casesens Keys
\casesens Activation key
\casesens Reserve keys
Keyboard settings for programs
------------------------------

  The "Keys" button in the \#Execution flags\# dialog will present you with
  a secondary dialog which allows you to configure some of the keystrokes
  used by the currently selected application:

    Activation key: When this key is pressed, the application will be made
      the active application. The application must already be in memory for
      this to occur.

    Reserve keys: Geneva can be told to ignore up to three keys for an
      application. When you press a key which is reserved, it will not be
      processed by Geneva, instead it will be processed by the application.
      This means that if, for instance, you have the Close Window gadget
      defined to be [Alternate][Esc], and you are running a program which
      uses this keystroke for its own purpose, you can tell Geneva to
      always send the [Alternate][Esc] to the application instead of trying
      to close a window.

  The "Clear" button will erase the current key. The dialog will show the
  string "???" to indicate that the key is undefined.

Related topics: \#Execution flags\#  \#What Flags to Change\#
\end

\casesens What Flags to Change
Determining What Flags to Change
--------------------------------

  Reasons to turn \#Multitask\# Off:

  \x3 The program gives you a dialog which, when clicked on, causes windows
    underneath the dialog to "bleed through".
  \x3 A program puts up a dialog which will not accept mouse clicks when in
    multitasking mode.
  \x3 The program shows you something that, when clicked on, actually
    activates a window or an icon belonging to another application.
  \x3 When a program redraws its windows, it draws over any other windows
    which are open.

  Reasons to turn \#Limit window handles\# On:

  \x3 The program exhibits strange behavior (such as missing mouse clicks
    or improper redraws) when another application has one or more windows
    open.
  \x3 The program attempts to close windows which do not belong to it.
    This can be discovered by using GNVA_DB.PRG.

  Reasons to turn \#Maximize window areas\# Off:

  \x3 The program expects there to be an "empty" horizontal or vertical
    scroll bar. An example of this is WordPerfect, which displays
    information about the current text row/column in the bottom of the
    window.
  \x3 The bottom line of a program's window does not redraw correctly.

  Reasons to turn \#Optimize window redraws\# Off:

  \x3 The program's windows do not always redraw correctly when windows are
    topped, moved from being partially off the edge of the screen toward
    the middle of the screen, or when being resized.

  Reasons to turn \#Limit memory\# On:

  \x3 Once the application has run, you have very little free memory left.

  Reasons to turn \#Clear allocated memory\# On:

  \x3 The application normally needs to have it Fast Load attribute turned
    off. By turning \#Clear allocated memory\# on, fast loading can often
    be performed.
  \x3 The application acts oddly when it is run after running and quitting
    one or more other programs.

  Reasons to turn \#Redraw upon exiting\# On:

  \x3 When the program quits, it leaves portions of its display on the
    screen.

  Reasons to turn \#Special object types\# Off:

  \x3 Some of the program's dialogs or menus display 3D boxes, underlines,
    checkboxes, or rounded radio buttons in unexpected locations.

  Reasons to turn \#Automatic keyboard equivs\# Off:

  \x3 The underlines overlap portions of the text in the dialog's buttons,
    or they appear in undesirable locations.

  Reasons to turn \#Keep menu bar/desktop\# On:

  \x3 The program's windows become "untopped", and Geneva switches to
    another primary application, whenever you use a certain feature.
  \x3 The program's desktop switches rapidly on and off whenever you use a
    certain feature.

  Reasons to turn \#AES 4.0 extended messages\# Off:

  \x3 The program does unexpected things when either its window is no
    longer topped, or one of its windows is becoming topped. Try the
    \#Limit window handles\# option, also.
  \x3 The program does unexpected things when a child program that it
    launched has terminated.

Related topics: \#Execution flags\#
\end

\screen sleep
\casesens Asleep
Asleep
------

  This menu entry shows the state of the application whose name is
  currently selected in the main window. If the name is preceded by a
  checkmark, then the application is "asleep", meaning that all of its
  windows are closed and it cannot receive any GEM events.

  You can click on this option to put an application to sleep or wake it
  up.

Related topics: \#Menu Options\#
\end

\casesens Terminate
Terminate
---------

  This option will terminate the selected application in the main window,
  closing all of its windows and freeing all memory it uses.

  Atari has established a standard method of communicating to a task the
  fact that it is being shut down. If you attempt to terminate an old task
  which does not understand this message, Geneva will warn you and give you
  a chance to cancel the termination.

  Terminating a task which does not understand this message will cause any
  unsaved work to be lost and may also cause future system crashes or other
  odd behavior, depending on the task.

Related topics: \#Menu Options\#
\end

\casesens Quit
Quit
----

  If the \#Task Manager\# is running as a program, this option will close any
  windows it has open and remove it from memory.

  If, however, the \#Task Manager\# is loaded as a desk accessory, the windows
  will close, but the program will remain in memory.

Related topics: \#Menu Options\#
\end

\casesens Keyboard Equivs
\casesens Keyboard equivalents
Keyboard equivalents
--------------------

  This dialog allows you to configure the keyboard equivalents used by
  Geneva. In it, you have a choice of either selecting a window gadget to
  change its keyboard equivalent, or one of the Other Keys.

  The box in the lower part of the dialog shows what key combination is
  assigned to that function. The Control, Shift, and Alt boxes can be
  checked with the mouse. You can also set the key by clicking on the
  "Read Key" button. While the Task Manager is waiting for you to press a
  key, you will not be able to perform any operations in other windows or
  the menu bar.

  When you have finished re-assigning the keyboard equivalents, you can
  close the dialog by pressing the Quit button. The close button in the
  upper left corner of the window will not close the dialog.

  There are also three other keyboard equivalents which are not immediately
  apparent:

  \x3 Iconify, the delta gadget to the left of the Cycle gadget.
  \x3 Iconify All, chosen by holding the Control key while clicking on the
    Iconify gadget.
  \x3 Cycle app's windows, chosen by holding either Shift key while
    clicking on the Cycle window gadget.

Related topics: \#Menu Options\#
\end

\casesens Window/Menu Bar
\casesens Window/Menu bar
Window/Menu bar
---------------

  This dialog allows you to control the \#font\# which Geneva uses in its menu
  bar and any dialogs displyed by programs. It also lets you affect how
  wide and high the window gadgets are.

  When Geneva first runs, it checks to see if the font ID saved in
  GENEVA.CNF is anything other than 1 (for a system font). If so, and GDOS
  is present, it loads all available fonts, and uses the correct font. If
  the particular font is not available, it reverts to the system font.

  Regardless of whether or not the font is available, the closest point
  size to the one you choose is used.

  The window gadgets will have a width which is equal to the width of one
  character in the font, plus the width value you choose in the Windows
  dialog. A similar calculation is used for the gadget height.

  The "Menu separator style" option controls how disabled menu entries in
  dropdown menus appear. The first option gives no change; the second uses
  a solid gray line, and the remaining styles are solid, colored lines.
  These latter styles are only used when the video display can show at
  least 16 colors at once.

  Please note that in order for the changes you make to the font or gadget
  borders to have any effect, you must \#Save Settings\# and then restart
  Geneva. Changes to the separator style are immediate.

Related topics: \#Menu Options\#  \#Fonts\#
\end

\casesens Fonts
\hidden font
Window and Dialog Fonts
-----------------------

  The Window/Menu Font dialog accepts any Speedo GDOS font, and any other
  monospaced GDOS (bitmapped or scalable) font which has all 255
  characters. This font is used when drawing menu bars, windows and
  dialogs. There are a few restrictions, though:

  \x3 If you use Speedo GDOS, it must be version 4.11 or newer.
  \x3 When using a Speedo font, Geneva attempts to remap the characters
    used in window gadgets (like the Full box, the Close box, etc.) While
    this remapping does work with most Speedo fonts, some fonts (like the
    Monospace 821 font) do not have the characters in the right locations
    for this translation, and incorrect characters will appear in the
    gadgets when they are drawn. This is unavoidable.
  \x3 Some versions of Speedo GDOS do not do a very good job of supporting
    the italics and bold special effects which are found in some of
    Geneva's dialogs.

Related topics: \#Menu Options\#  \#Window/Menu bar\#
\end

\casesens Dialog Colors
\casesens Dialog preferences
Dialog preferences
------------------

  This dialog allows you to set the way in which various components of
  other dialogs are displayed.

  Geneva allows you to have separate appearances for several types of
  objects contained within dialogs. If a program uses a special attribute
  type defined by Geneva, then the appearance will change for all of the
  objects in all of the program's dialogs at once, thus allowing you to
  customize a program to your own preference.

  The "Type" box contains a list of the four different types of objects
  that can be assigned these special attributes. The "Sample" shows you one
  object of each of these types, so that you can gauge how they will look
  in actual dialogs.

  Each type of object can have several different attributes:

    Border
    \x3 AES 4 3D: a three-dimensional effect, compatible with Atari's AES
      version 4. In this mode, the text color, fill color, and fill pattern
      are modified by BCOLORS.CPX, not the Task Manager. For the Box
      object, you will probably also want to select the Outline option. In
      this mode, Any text in the object will "move" when it is selected.
    \x3 3D: the original Geneva-style three-dimensional effect. The text of
      the object will "move" when it is selected.
    \x3 Outline: produces a line around the outside of the object.
    \x3 Shadow: a shadow on the lower-right edge of the object.
    \x3 Round: the corners of the outer edge are rounded, instead of square.
      Note that if Outline is also used, the thick part of the object's
      border is moved to the outside.
    \x3 Color: the left and right arrows will cycle through the available
      colors, and clicking on the color sample will produce a popup menu of
      colors.

    Text
    \x3 Shadowed: the text has a shadow behind it. The shadow's color is the
      text color chosen below. The text itself is always drawn in either
      black or white.
    \x3 Bold: bold text. This can even be combined with the Shadowed effect.
    \x3 Opaque: affects how text is drawn. When this flag is set, the area
      around the actual dots that make up a character of text are left
      white. Otherwise, the fill pattern of the object will show through.

    Fill
    \x3 Pattern: this is a pop-up menu which lets you choose from any of
      the eight pre-defined fill patterns. You can also move from one to
      the next by clicking on the left or right arrow buttons.
    \x3 Color: this is a pop-up menu with all of the colors available in the
      resolution you are currently using. (Some colors will be black if the
      current screen resolution does not support at least 16 colors at
      once.) You can use this option to define what color is used when
      drawing the fill pattern of the currently selected object.

  The text attributes of the menu bar cannot be changed. Menu text is
  always black and transparent. Note that any changes you make in this
  dialog do not take effect until you select the "Ok" button.

Related topics: \#Menu Options\#
\end

\casesens Window Colors
\casesens Window gadget colors
Window gadget colors
--------------------

  Geneva provides more window gadgets than GEM normally does, so this
  dialog gives you control over the colors and fill patterns of all of
  them.

  To select a gadget to modify, click on the gadget with the mouse. An
  outline will appear around the gadget you have chosen. Its attributes can
  then be changed.

  The options for text, border, and fill color are here, just like in the
  \#Dialog Colors\# dialog. You also have a few more choices:

  \x3 The 3D button turns on/off the raised appearance of the gadget.
  \x3 The Shadowed button in the Text section turns on or off a shadow
    behind the text. The shadow's color is the text color chosen below.
    The text itself is always drawn in either black or white.
  \x3 The Topped button will toggle between editing the attributes for a
    window which is on top or elsewhere in the stack.

  Note that the Shadowed and 3D buttons affect the window gadget for both
  its topped and untopped appearance.

  To copy one gadget's attributes to another, click and hold the mouse
  button down on a gadget and drag the resulting gray outline to another
  gadget in the window.

  Clicking on the mover bar of this window will only select that gadget or
  copy its attributes to another gadget. However, the window can be moved
  by clicking on any of the white areas within the window.

  This window's appearance does not change when you make another window
  topmost. The text attributes of the menu bar cannot be changed. Menu text
  is always black and transparent.

  When Geneva's settings are saved, using the \#Save Settings\# option,
  the window colors are also saved. If you use another program like
  WCOLORS.CPX, then the settings in that program will override the ones you
  have set in this dialog.

Related topics: \#Menu Options\#
\end

\casesens Misc. Options
\casesens Miscellaneous options
Miscellaneous options
---------------------

  \x3 Pulldown menus: if this option is checked, you must first click on a
    word in the menu bar before a menu will pull down.

  \x3 Tear away menus always active: When this option is checked, the items
    in new tear away menu windows which are opened will be selectable with
    just the left mouse button, even when the tear away window is not
    topmost. In this mode, a tear-away menu window can be "topped" by
    clicking once on its name bar.

  \x3 Top all windows when switching: When this is checked, every time
    a different application's window becomes topmost, all of its windows
    are brought to the front at the same time. Without this option, only
    the one window chosen will be topped.

  \x3 Dialog cursor insert mode: When this option is checked, the cursor in
    editable fields within dialog boxes will operate in Insert mode, which
    is to say that any characters you type are inserted at the current
    cursor position. When the cursor is in overwrite mode, characters are
    overwritten as you type. This option can also be changed by using the
    Insert key when editing a text field in a dialog.

  \x3 Long dialog title underlines: When a dialog takes advantage of the
    special object type that Geneva provides for creating titles with a
    underline, this option controls whether the line goes completely
    across the width of the dialog or if it is just underneath the words
    which make up the title.

  \x3 Alerts follow mouse: When active, this option causes alert boxes t
    appear as close to the current mouse position as possible. If this
    option is off, alerts will appear centered in the middle of the screen.

  \x3 1 column in item selector: This option controls whether or not
    Geneva's Item Selector uses separate lists for files and folders.

  \x3 Grow/shrink boxes: Many programs use a flourish when opening windows
    or dialog boxes. Since some people find them distracting and time-
    consuming, Geneva allows you to disable them.

  \x3 Automatic shell window updates: If the shell program (like NeoDesk
    3.04) understands the special message type meaning "update drives with
    this letter", then Geneva can automatically determine when the contents
    of a disk drive have changed and inform the shell so that it can update
    its windows.

  \x3 Alert \#multitask\# mode changes: If this option is on, then whenever
    one or more applications is about to be put to \#sleep\# because you
    are switching into or out of single-tasking mode, you will get an alert
    from Geneva warning you of this fact.

  \x3 Video from NEWDESK/DESKTOP.INF: When this option is on, Geneva will
    attempt to read the \#Video mode\# (screen resolution) settings from
    either a DESKTOP.INF or NEWDESK.INF file. If this option is off, the
    video mode information contained in \#GENEVA.CNF\# (saved by
    \#Save Settings\#) is used.

  \x3 Preserve system colors: Prevents applications which alter the color
    palette from affecting other applications.

    As you switch out of an application which changed the colors, they will
    be reset to what they were the last time the new application had
    control. Many paint programs will benefit from this option. Also,
    any time an alert or Geneva's item selector is opened, the system
    colors are returned to those of the Geneva Manager menu. This gives a
    more readable item selector in programs like Calamus SL which change
    the system color palette.

    Note that it if you have this option set, you cannot change the colors
    for all applications by using things like a control panel!  As soon as
    you change to another application, the colors will revert to what they
    were previously. Turn this option off before using the control panel.

    Furthermore, in order for the system colors that are used by default to
    be changed, Geneva essentially ignores this flag until all desk
    accessories are loaded. This way, if one of them (like a control panel)
    changes the settings, all applications will get the new colors by
    default. Once all of the desk accessories have loaded, "Preserve system
    colors" takes effect.

  \x3 Preserve mouse on/off: Keeps track of how many times a program makes
    the mouse pointer visible/invisible, and automatically restores the
    mouse to the correct state when you switch between applications,
    similar to the way to the "Preserve system colors" option works.

  \x3 Any dialog without editable fields can respond to just the keyboard
    equivalents, without Alternate being held. The "Not in window" option
    affects this feature for dialogs and alerts that are not contained in a
    window.

    The "In window" option affects this feature for for dialogs that are
    drawn by Geneva and appear in a moveable window, like all of the Task
    Manager's dialogs. Some programs generate their own windowed dialogs
    and will not be affected by this option. By default, this option is
    turned off for dialogs in windows.

  \x3 Window gadget repeat delay: This editable field contains the number of
    milliseconds that will elapse between the time you first click on a
    window gadget which repeats (like the up scroll gadget) and the next
    time the window will scroll. This gives you time to release the mouse
    button before the window automatically scrolls again. The default value
    is 300 milliseconds.

Related topics: \#Menu Options\#
\end

\casesens Video
\casesens Video mode
\casesens Falcon Video mode
Video mode
----------

  In this dialog, you can select any of the video modes supported by your
  computer. Selecting the Ok button will cause all applications to be
  terminated. Geneva will then re-initialize itself in the new resolution
  and reload the default desk accessories. Please read the \#Terminate\#
  help text before using this option.

Related topics: \#Menu Options\#
\end

\casesens Settings
\casesens Save Settings
\casesens Reload Settings
Save settings
-------------

  This option will cause Geneva to save all of its configuration options in
  a file called GENEVA.CNF. When you reload Geneva, these settings are
  automatically re-read.

Reload settings
---------------

  This option will reload all of Geneva's settings from the GENEVA.CNF
  file.

Related topics: \#Menu Options\#
\end

\casesens Save Positions
Save Positions
--------------

  This option will cause the Task Manager to save all of its configuration
  options in a file called GENEVA.CNF. Whenever you load the Task Manager,
  it automatically reads these settings.

  Currently, the Task Manager saves all of the positions of its dialogs
  and also the main window. If you Save Positions in a high resolution
  screen and then run the Task Manager in a lower resolution, you may
  notice that one or more of the dialogs appear partially off the right or
  bottom edge of the screen.

Related topics: \#Menu Options\#
\end
