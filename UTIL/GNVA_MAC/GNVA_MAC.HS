\casesens Geneva Macro
\casesens Topics
Geneva Macro Editor
-------------------

  The Geneva Macro Editor allows you to record things that your applica-
  tions do while running with Geneva. You can then play back these macros
  by pressing a key on the keyboard.

  Please choose a topic from this list:

    \#Installation\#

    \#What Are Macros?\#   \#What Can and Can't Be Recorded\#
    \#Recording a Macro\#  \#Why Did My Macro Stop Working?\#

    \#Main Window\#  \#Selecting Macros\#   \#Editing Macro Names\#
    \#Keystrokes\#   \#Editing Keystrokes\# \#Global Macros\#
    \#Autostarting Macros\#  \#Suggestions\#

    \#Editing Macros\#  \#Edit Window Menus\#

    \#Advanced Topics\#
\end

\casesens Installation
Installation
------------

  The Geneva Macro Editor can be installed as either a program or a desk
  accessory. To run it as a program, simply double-click on GNVA_MAC.PRG.
  When used this way, the macro editor can be passed \#commandline\#
  parameters.

  To run it as a desk accessory, move GNVA_MAC.PRG and GNVA_MAC.RSC to the
  same folder you normally load your desk accessories from. Rename the
  program to GNVA_MAC.ACC and reboot the computer.

  Once it has installed as a desk accessory, you may also want to change
  the \#Macro file (GMA) directory\# in the \#Preferences\# dialog. Refer to
  the section of this manual describing the Preferences dialog for more
  information. Also note that in order for the \#Automatic GMA loading\#
  option to work, you must run the Editor as a desk accessory.

  In either mode, you can also copy the GNVA_MAC.HLP file to the
  GENEVA\\HELP directory, where Geneva's own help files should also be
  stored. This will allow you to get online help.

Related Topics:  \#Topics\#
\end

\casesens What Are Macros?
What Are Macros?
----------------

  In computer terms, a macro is a way to perform a complex series of
  operations by one much simpler action. With Geneva, a macro consists of
  one or more of the four distinct types of "event":

  \x3 Timer:      Wait a number of milliseconds (1/1000 of a second) before
                continuing with the next event

  \x3 Button:     Click the left and/or right mouse button

  \x3 Mouse:      Position the mouse pointer to a specific X,Y coordinate on
                the screen

  \x3 Keyboard:   Act as though a particular key on the keyboard had been
                pressed

  A Macro is played back by assigning a combination of keys on the keyboard
  to it. Pressing this combination of keys will play the macro. A macro can
  be stopped at any time by holding both Shift keys on the keyboard.

  A macro can also generate certain \#extended events\#, such as typing the
  date or time into an application, making a sound, or displaying a message.

Related Topics:  \#Topics\#
                 \#What Can and Can't Be Recorded\#
                 \#Why Did My Macro Stop Working?\#
                 \#Recording a Macro\#
                 \#Extended Events\#
\end

\casesens What Can and Can't Be Recorded
What Can and Can't Be Recorded
------------------------------

  All Timer, Button, Mouse, and Keyboard events above can be recorded.
  However, some programs do not use the AES portion of GEM at all, and
  their events cannot be recorded. An example of this is if you were to
  run LHARC.TTP without Geneva TOS. In this case, LHARC would not have any
  windows, menu bar, or dialogs, and any keypresses you might send to LHARC
  cannot be recorded.

Related Topics:  \#Topics\#
                 \#What Are Macros?\#
                 \#Why Did My Macro Stop Working?\#
                 \#Recording a Macro\#
\end

\casesens Why Did My Macro Stop Working?
Why Did My Macro Stop Working?
------------------------------

  One of the most important things to remember about the type of macros
  this editor uses is that they are "position dependent". This means that
  if you record the action of clicking with the mouse on a button in a
  dialog box, and then move the dialog slightly, playing back the macro
  will cause the mouse button to "click" at a location that might not be
  where the button is anymore! You need to be very careful when recording
  mouse events for this reason.

  Another important thing to consider is pausing, or timer events. Let's
  say you record a macro that opens a disk drive window and then double-
  clicks on an icon in the window. If you play this macro it should work
  just fine. But what happens when you add a lot of files to the directory?
  Now the window takes a little longer to open, and the part of the macro
  that double-clicks on the icon might act too soon because the delay
  before double-clicking is too short for the new number of files in the
  directory. In this case, you would have to manually find and edit the
  timer event to make it longer.

Related Topics:  \#Topics\#
                 \#What Are Macros?\#
                 \#What Can and Can't Be Recorded\#
                 \#Recording a Macro\#
\end

\casesens Main Window
The Main Window
---------------

The main macro editor window contains these elements:

\x3 Close box: Clicking here will quit the program if it was run as a PRG.
  If run as a desk accessory, just the main window and any dialogs will
  close; the program stays in memory.

\x3 Mover bar: This allows you to move the main window. It also contains the
  name of the current macro (\#GMA\#) file, if any.

\x3 Iconify gadget: The iconify (\x7f) gadget will shrink the main window and
  any open dialogs down to a small window toward the lower left of the
  screen. Double-clicking on the icon in the iconified window will cause
  the window(s) to re-open. Holding the Control key on the keyboard while
  selecting the iconify gadget will cause all windows, including any
  editing windows, to be iconified.

\x3 Information: When no items are selected, the info bar gives a total size
  of all items. When one or more items are selected, a total of those items
  is given.

\x3 Menu bar: Clicking on a title in the menu bar will allow you to choose
  items in the menu. See the \#Main Window Menus\# section for descriptions of
  each menu entry.

\x3 Working area: The working area of the main window contains a list of
  macros that are currently in memory. Each macro has an editable name of
  up to 20 characters and, to the right, a \#keystroke\# that can be used to
  play that macro.

Related Topics:  \#Topics\#  \#Selecting Macros\#  \#Editing Macro Names\#
                 \#Keystrokes\#  \#Editing Keystrokes\#  \#Global Macros\#
\end

\casesens Selecting Macros
Selecting Macros in the List
----------------------------

  Various operations can be performed on macros, once they have been
  selected.

  Clicking once with the mouse in the blank area of a macro not occupied by
  the name or the \#keystroke\# button will select that macro and de-select
  all other macros.

  To select more than one macro at once, hold either Shift key while
  clicking with the mouse.

  To select a range of macros, start by selecting one macro. Position the
  window so that the end of the range is visible. Hold the Control key
  while clicking on the last macro in the range. All macros in the range
  are now selected.

  Once you have selected items in this list, you can use the \#Edit\# menu to 
  make changes to the items. Cut and Delete operations cannot be carried out
  on \#Global Macros\#, which will appear with greyed-out text. None of a
  global macro's options can be changed, either.

Related Topics:  \#Topics\#  \#Main Window\#  \#Editing Macro Names\#
                 \#Keystrokes\#  \#Editing Keystrokes\#  \#Autostarting Macros\#
\end

\casesens Editing Macro Names
Editing Macro Names
-------------------

  To change the name of one or more macros, start by clicking on the name
  of a macro you wish to edit. A standard GEM edit cursor will appear
  within the name, allowing you to use the normal editing keys to alter the
  name. Other names in the list can be edited by using the up/down arrow
  keys, or the Tab key on the keyboard.

  Any time you select a macro, as described in the \#Selecting Macros\#
  section, the edit cursor is turned off.

Related Topics:  \#Topics\#  \#Main Window\#  \#Selecting Macros\#
                 \#Keystrokes\#  \#Editing Keystrokes\#
\end

\casesens Keystrokes
\hidden keystroke
Keystrokes
----------

  Whenever the Macro Editor needs to represent a key on the keyboard, it
  uses the same notation Geneva normally does:

  \x3 A "carat" (^) character means that Control is held.
  \x3 An "up arrow" () character means that either Shift key is held.
  \x3 A "close box" () character means that the Alternate key is held.

  Any other character means that that key on the keyboard is pressed. For
  instance, the notation ^K means that Control and Shift are held as the K
  key is pressed.

  If there is a "???" instead of the name of a key on the keyboard, then
  this means that no key has been assigned.

Related Topics:  \#Topics\#  \#Main Window\#  \#Editing Macro Names\#
                 \#Selecting Macros\#  \#Editing Keystrokes\#
\end

\casesens Editing Keystrokes
Editing Keystrokes
------------------

  In order to change the \#keystroke\# that will activate a macro, click on the
  button to the right of the macro's name. In this mode, you cannot use
  menus or dialogs; the editor is waiting for a new keypress.

  To set the key, hold the appropriate combination of Control, Shift, and
  Alternate keys, and then press one of the other keys on the keyboard.
  The text of the button will update to reflect your choice. Note that
  there is no difference between holding the left Shift key versus the
  right Shift key.

  If you attempt to change the keystroke to one that is already assigned to
  another macro, you will hear a bell signal alerting you to the fact.

  You can instead clear-out the keyboard equivalent, so that no keystroke
  will cause the macro to play, by pressing the right mouse button. The
  text in the button will change to "???".

  You should be very careful when deciding what keystroke to use for a
  macro. Anytime the key you choose is pressed, the macro will run. This
  means that you really need to evaluate what keypresses are "safe". You
  should always avoid using keystrokes that do not include one or more of
  the Shift, Alternate, or Control keys being held, since these are typed
  very frequently in text editors and dialogs.

Related Topics:  \#Topics\#  \#Main Window\#  \#Editing Macro Names\#
                 \#Selecting Macros\#  \#Keystrokes\#  \#Advanced Topics\#
\end

\casesens Autostarting Macros
Autostarting Macros
-------------------

  Any macro can be set to start automatically at a certain time, and can
  repeat indefinitely at preset intervals. Some uses for autostart macros
  include automatically saving the document you are editing in an
  application, making periodic backups, or reminding you of important
  events.

  To autostart a macro, begin by deciding how often you want the macro to
  repeat. Choose "once" if you only want the macro to play one time,
  otherwise choose a different interval from the popup menu.

  Then set the time and date for the macro to begin playing. Enter the time
  by clicking on the time field to the right of the interval popup. An edit
  cursor appears, allowing you to enter the time in 24-hour (military)
  format. Remember that in 24-hour time, 1 pm is 13:00. The date must also
  be entered. By default, the date and time of a macro match the date and
  time it was created.

  Now choose the "Autostart" checkbox. If the date/time you entered has
  already gone by, the time of the macro is automatically incremented to
  the next time the macro will play.

  When the correct date and time are reached, the macro will play. If the
  macro was set to only play once, the Autostart button is deselected.
  otherwise, the date and time are incremented to reflect the next time
  the macro will play.

  If a newly-loaded macro file contains autostart macros that should have
  already been played, they are played as soon as the macro file finishes
  loading.

Related Topics:  \#Topics\#  \#Main Window\#  \#Selecting Macros\#
\end

\casesens Main Window Menus
Main Window Menus
-----------------

  The main window contains these menus:

      \#File\#            \#Edit\#           \#Record\#          \#Options\#

     Open        Edit Macro         New Macro     \#Preferences\#
     New         New Macro          Timer         \#Defaults\#
     Save        Delete Globals     Button        Save Settings
     Save as     Cut                Mouse         About
     Quit        Copy               Keyboard      Help
                 Paste         
                 Delete

Related Topics:  \#Topics\#  \#Main Window\#
  \@slinewidth 3
  \@line 3 4 13 4 13 12 3 12 3 4 \@line 3 6 13 6
  \@line 15 4 32 4 32 14 15 14 15 4 \@line 15 6 32 6
  \@line 34 4 46 4 46 12 34 12 34 4 \@line 34 6 46 6
  \@line 48 4 64 4 64 12 48 12 48 4 \@line 48 6 64 6
\end

\casesens File
\hidden GMA
The Main Window File Menu
-------------------------

\x3 Open: Opens a macro (GMA) file. All changes to the file currently in
  memory are lost. To edit the list of \#Global Macros\#, open GLOBAL.GMA.

\x3 New: Erases any macros from memory.

\x3 Save: Saves the file currently in memory, using its current name. If
  no name is defined, then this acts the same as "Save as".

\x3 Save as: Saves the file, allowing you to choose a new name. To start a
  list of \#Global Macros\#, save the current file to the name GLOBAL.GMA,
  in the \#GMA\# directory.

\x3 Quit: Quits the program, if it was run as a PRG. If run as a desk
  accessory, just the main window and any dialogs will close; the program
  stays in memory.

  When the macro editor first runs, it tries to load a file called
  DEFAULT.GMA that is in the \#Macro file (GMA) directory\#, as specified in
  the \#Preferences\# dialog.

Related Topics:  \#Main Window\#  \#Main Window Menus\#
\end

\casesens Global Macros
Global Macros
-------------

  To avoid having to duplicate the same macro in many files, the macro
  editor can treat a set of macros as "global", meaning they are always in
  memory. This feature is especially useful when combined with the
  "Automatic GMA loading" feature.

  Global macros cannot be deleted or edited directly. They appear in gray
  text, and their options are disabled. The only way to change them is to
  edit the file GLOBAL.GMA. In order to function properly, this file must
  be in the \#Macro file (GMA) directory\#, as specified in the
  \#Preferences\# dialog.

  Global macros can be opened, so that their individual steps can be copied
  to the clipboard, but changes are not allowed.

  A global macro can be overridden by simply creating a regular macro with
  the same keyboard equivalent.

Related Topics:  \#Main Window\#  \#Topics\#  \#Selecting Macros\#
\end

\casesens Edit
The Main Window Edit Menu
-------------------------

\x3 Edit Macro: An edit window is opened for any selected macros, allowing
  you to edit the individual events that make up the macro.

\x3 New Macro: A new, empty macro is created and an edit window is opened,
  allowing you to add new events. See also \#Recording a Macro\#.

\x3 Delete Globals: Any \#Global Macros\# are deleted from memory. The
  contents of the GLOBAL.GMA file are not affected.

\x3 Cut: The selected macros are removed from memory and saved to the
  clipboard.

\x3 Copy: The selected macros are just copied to the clipboard.

\x3 Paste: If no macros are selected, the contents of the clipboard are
  pasted at the end of the macro list. If one or more macros are selected,
  the contents of the clipboard are inserted, starting just before the
  first selected macro.

\x3 Delete: The selected macros are removed from memory. This operation
  cannot be undone.

Related Topics:  \#Main Window\#  \#Main Window Menus\#  \#Editing Macros\#
                 \#Advanced Topics\#
\end

\casesens Record
\casesens Recording a Macro
\casesens New Macro
The Main Window Record Menu
---------------------------

\x3 When Record->New Macro is chosen, a new macro entry is created in the
  list. The macro is currently undefined, so its name and keypress are
  blank. A small dialog also appears, which tells you what keypress to use
  to start and later stop recording. This keypress is defined in the
  \#Preferences\# dialog, and defaults to Control-Shift-Alt-Esc.

  Once this dialog appears, you should enter the application you want to
  record the macro for, and make sure things are set up properly on the
  screen so that you can begin recording.

  Press the start/stop key, and you will hear a short tone to indicate that
  recording has begun. If the application you are recording is still
  allowing other applications to function, you will periodically notice the
  status dialog updating to reflect the total number of events that have
  been recorded so far. Up to 9999 events can be recorded for each macro,
  depending on the setting in the \#Preferences\# dialog, though only the
  first 1000 or so can be edited afterward.

  When you are done recording, press the start/end key a second time. You
  will hear a second, descending tone to indicate the end of recording. At
  this point, it is probably a good idea to return to the main macro editor
  screen and assign a name and a keystroke to the new macro.

\x3 The rest of the entries in the Record menu control what types of events
  will be recorded both by the New Macro function and also by the "Record"
  function in the \#Event\# menu of an edit window.

  If a type of event does not have a checkmark in front of it, that type of
  event will not be recorded. This is a good way to keep macros as short as
  possible. For instance, if you are recording a word processing macro to
  type the characters "Fido", you can save some space in the macro by not
  recording Timer, Button, or Mouse events.

\x3 Rather than recording a macro, a new, empty macro can be created with
  the New Macro command in the \#Edit\# menu.

Related Topics:  \#Topics\#  \#Main Window\#  \#Main Window Menus\#
                 \#Editing Macros\#  \#Extended Events\#
\end

\casesens Preferences
Preferences
-----------

  This dialog controls several settings that affect how the Macro Editor
  works:

  \x3 Start/stop recording:  When recording a macro, this keypress causes the
    actual recording to start and, later on, to end. The keypress can be
    edited, as described in the \#Editing Keystrokes\# section, by
    clicking on the button.

  \x3 Max. events per macro:  This is the largest number of events that any
    one macro can contain. Recording may end prematurely if the maximum
    number of events is exceeded. Increasing this number will allow you to
    record longer macros, but it also requires more memory (each step
    requires 6 bytes.)

  \x3 Macro file (GMA) directory:  This is the first place the macro editor
    looks for macro files. Also, when the macro editor first runs, it tries
    to load a file called DEFAULT.GMA that is in this directory. This
    directory is also used in conjunction with the "Automatic GMA loading"
    option.

  \x3 Automatic GMA loading:  When this option is chosen, the macro editor
    will try to load a new GMA file every time you switch focus from one
    application to another in Geneva. This allows you to have a separate
    set of macros for each major application you run.

    The name of the GMA file that gets loaded is taken from the file name
    of the application you switch to. For instance, if you switch to Atari
    Works, then the macro editor will try to load the file WORKS.GMA. For
    NeoDesk, the file would be NEODESK.GMA, since the main NeoDesk program
    is NEODESK.EXE. If there is no GMA file for the application, the
    DEFAULT.GMA file is loaded.

    The automatic loading will only occur if there are NO Geneva Macro
    Editor windows open at all. This is so that any edits you might be in
    the process of making will not be overwritten by loading a new file.

    If the main window is closed and the macros have changed without having
    been saved, then any time a new macro file would have been loaded
    automatically, the system bell will sound instead to remind you that no
    macro file was loaded. Save the macros that have changed and this bell
    will no longer sound.

Related Topics:  \#Options\#  \#Main Window\#  \#Main Window Menus\#
\end

\casesens Defaults
\casesens New Event Defaults & Formats
Defaults
--------

  This dialog lets you control what type of sound/date/time event is chosen
  by default when you add a new event of one of these types. It also lets
  you change the available formats of the time and date generated.

  \x3 Sound: Choose the default sound used when the "Sound" item in the
    \#Event\# menu is chosen.

  \x3 Date Format: The default format for a date event can be set here by
    selecting the "DFLT" button to the left of a format.

    Usually, the default formats are sufficient for most people, but they
    can be changed if you desire. The formats are entered using a C-style
    string.  The following special sequences cause certain parts of a date
    to be inserted:

      %/  The M/D/Y separator, as taken from the _IDT system cookie.
      %%  A percent sign (%).
      %b  The month's name, as a 3-letter abbreviation (Jan, Feb, etc.)
      %B  The month's full name (January, February, etc.)
      %d  The day of the month (1-31).
      %m  The month number (1-12)
      %y  The 2-digit year (00-99)
      %Y  The 4-digit year

    Clicking on the "Test" button will display a popup menu that you can
    use to check the formats.

  \x3 Time Format: Just as with the date format, the default format for a
    time event can also be changed by selecting the "DFLT" button to the
    left of a format. The formats themselves can also be edited, using
    these sequences:

      %H  The hour (00-23)
      %I  The hour (1-12, usually followed with %p)
      %M  The number of minutes (1-59)
      %p  am or pm
      %s  The number of seconds (accurate to within 2 seconds)

  Note that changing date/time formats will immediately affect any macros
  which use the formats in the same position in the list.

Related Topics:  \#Options\#  \#Main Window\#  \#Main Window Menus\#
\end

\casesens Options
\casesens Macro file (GMA) directory
\casesens Automatic GMA loading
The Main Window Options Menu
----------------------------

\x3 \#Preferences\#: This dialog controls several settings that affect how the
  Macro Editor works.

\x3 \#Defaults\#: This dialog lets you control what type of sound/date/time 
  event is chosen by default when you add a new event of one of these
  types. It also lets you change the available formats of the time and date
  generated.

\x3 Save Settings: Selecting this option will cause all of the dialog
  positions, the size and location of the main window, and the
  \#Preferences\# to be saved to GENEVA.CNF. When the Macro Editor first
  runs, it reads these settings.

\x3 About: Version number and copyright information about the Macro Editor.

\x3 Help: Obtains online help for the main window.

Related Topics:  \#Main Window\#  \#Main Window Menus\#
\end

\casesens Editing Macros
Editing Macros
--------------

  Rather than deleting a macro that is not correct, it is often easier to
  edit an existing one so that it is more like what you intended.

  If you double-click in the selection area of a macro, or select one or
  more macros and then use the \#Edit\# Macro menu entry, then a separate
  edit window will open for each macro you have chosen.

  An edit window is a lot like the main window. It has the following
  differences:

  \x3 The name in the mover bar is that of the macro being edited.

  \x3 The working area of the window contains the events which make up the
    macro:

    \x3 Timer: The length of a pause, in milliseconds (1/1000 of a second).

    \x3 Button: The state of the left and right mouse buttons. An X next to
      "Left" or "Right" means that the appropriate button is pressed at
      that time. Simply click on either button to change the state.

    \x3 Mouse: The X (horizontal) and Y (vertical) coordinates of the mouse
      pointer on the screen. Clicking on the Read button allows you to use
      a real mouse position. Move the mouse pointer to a location on the
      screen and click on the left mouse button to set this new location in
      the event. Click with the right mouse button to cancel the operation.

    \x3 Keyboard: A keypress appears in a button, along with the ASCII
      character produced by that keypress, if any. Clicking on this button
      allows you to change the key, as described in the \#Editing Keystrokes\#
      section. When editing the key, you can press the left mouse button to
      produce a complete table of all ASCII characters. This allows you to
      produce keystrokes that cannot normally be typed on your keyboard.
      Select a character with the mouse, or click outside the table to
      cancel the operation. If the character you click on can be produced
      from the keyboard, the correct sequence will appear in the button,
      along with the ASCII character.

      The first event in a macro is usually a keyboard event without any
      key assigned to it, like "^???". This is so that the initial state of
      the Control, Shift, and Alternate keys will be set correctly when the
      macro plays back. If the macro does not rely on the state of these
      keys while doing mouse movement or button presses, then this first
      event can probably be removed.

    \x3 The values for Timer and Mouse events can be edited by clicking on the
      editable field with the mouse. At this point, a regular GEM edit
      cursor appears, allowing you to change the numerical value. While you
      are in this mode, you cannot use the menu bar or other applications.
      Press Return to end editing.

  \x3 Events in the list can be selected just like macros, as described in
    the \#Selecting Macros\# section.

  \x3 New events (including sounds, the date or the current time, and
    message dialogs) can also be added by using the \#Event\# menu.

Related Topics:  \#Topics\#  \#Edit\#  \#Record\#  \#Edit Window Menus\#
                 \#Advanced Topics\#  \#Extended Events\#
\end

\casesens Edit Window Menus
Edit Window Menus
-----------------

  Edit windows contain these menus:

     \#Editý\#       \#Event\#          \#Optionsý\#

     Cut        Timer        Edit All Timers
     Copy       Button       Remove All
     Paste      Mouse        Optimize
     Delete     Keyboard
     Close      Sound
                Date
                Time
                Message
                Record
                Test
                Help

Related Topics:  \#Topics\#  \#Editing Macros\#
  \@slinewidth 3
  \@line 3 4 12 4 12 13 3 13 3 4 \@line 3 6 12 6
  \@line 14 4 25 4 25 18 14 18 14 4 \@line 14 6 25 6
  \@line 27 4 45 4 45 10 27 10 27 4 \@line 27 6 45 6
\end

\casesens Editý
The Edit Window "Edit" Menu
---------------------------

\x3 Cut: The selected events are removed from memory and saved to the
  clipboard.

\x3 Copy: The selected events are just copied to the clipboard.

\x3 Paste: If no events are selected, the contents of the clipboard are
  pasted at the end of the list. If one or more events are selected, the
  contents of the clipboard are inserted, starting just before the first
  selected event.

\x3 Delete: The selected events are removed from memory. This operation
  cannot be undone.

\x3 Close: Closes the editor window, just like clicking on the window close
  gadget.

  The Cut and Delete options cannot be used on \#Global Macros\#.

Related Topics:  \#Edit Window Menus\#  \#Advanced Topics\#
\end

\casesens Event
The Edit Window "Event" Menu
----------------------------

  The first four entries in this menu will insert a new event of the chosen
  type. If no events are already selected, the new event is inserted at the
  end of the list. If one or more events are selected, the event is
  inserted just before the first selected event:

  \x3 Timer: Inserts a timer event, and immediately enters edit mode,
    allowing you to set the length of the pause, in milliseconds (1/1000 of
    a second).

  \x3 Button: Inserts a new button event. The states of the mouse buttons can
    be set by clicking on the "Left" and "Right" buttons.

  \x3 Mouse: Inserts a new mouse movement event. The X (horizontal) and Y
    (vertical) coordinates of the event are preset to be the current
    position of the mouse pointer on the screen.

  \x3 Keyboard: Inserts a new keyboard event. Edit mode is entered
    automatically, allowing you to change the key, as described in the
    \#Editing Keystrokes\# section.

  \x3 Sound: Inserts a sound event. When the correct point in a macro is
    reached, a sound is played.

  \x3 Date: The current system date is sent through the keyboard, as though
    you had typed it. Click on the button to choose a format from the popup
    menu. It may be helpful to add a carriage return or space keypress
    after the date.

  \x3 Time: The current system time is sent through the keyboard. Click on
    the button to choose a format from the popup menu.

  \x3 Message: When this event occurs, a dialog appears on the screen with
    up to five lines of text. A message can be used to provide information
    about what the macro is doing, or as a reminder to the person using the
    computer. Note that only one message window can be open at a time, and
    the Macro Editor must not be asleep. See \#Suggestions\# for more info.

  The remainder of the options in this menu are:

  \x3 Record: Records a sequence of events, in the same way that \#New Macro\#
    works. If no events in the list are already selected, the new events
    are inserted at the end of the list. If one or more events are already
    selected, the new events are inserted just before the first selected
    event.

  \x3 Test: Plays the current macro, from start to finish. Macro playback
    can always be aborted by holding both Shift keys on the keyboard.

  \x3 Help: Obtains online help for the edit window.

Related Topics:  \#Edit Window Menus\#  \#Extended Events\#
\end

\casesens Optionsý
The Edit Window Options Menu
----------------------------

  The options in this menu will perform an operation on all events in the
  macro, if no events are selected. If some events are selected, then only
  those events will be affected. Note that these changes cannot be undone.

  \x3 Edit All Timers: Opens a secondary dialog into which you can enter a
    new duration for any timer events. Only timer events are modified.

  \x3 Remove All: The options under this heading delete all events of the
    chosen type.

  \x3 Optimize: You can choose to automatically remove any events of the
    chosen type which are not necessary to the execution of the average
    macro. This makes the macro play more quickly, and the macro file takes
    up less disk space and RAM. The "All" option optimizes all types of
    events, instead of just one.

    Because eliminating events in this way can lead to a macro which does
    not function properly, it is important that you understand what this
    option will do for each type of event:

    \x3 Timer: Any consecutive timer events are added together into one
      event. A timer event appearing at the end of a macro, one with a
      delay of zero, or one which is between two mouse movement events, is
      removed.

    \x3 Button: Any events where the states of the mouse buttons do not
      actually change are removed.

    \x3 Mouse: Any mouse movement that is followed by another mouse movement
      is removed. This is the most likely type of optimization to cause
      problems, since macros like one to select a menu entry rely heavily
      on the position of the mouse. However, for a macro to select a button
      in a dialog, you do not need to include all the events leading up to
      the mouse clicking at its final location, so this type of macro could
      have its mouse events optimized.

    \x3 Keyboard: Any keypresses that only change the state of the Control,
      Shift, or Alternate keys without actually typing a character, which
      are followed by another keystroke, get removed.

Related Topics:  \#Edit Window Menus\#
\end

\casesens Extended Events
\hidden extended events
Extended Events
---------------

  In addition to keyboard, mouse, and timer events, four other types
  of event can be generated: typing the date or time, playing a system
  sound, and displaying a message.

  In order to take advantage of any of these events, you first need to
  have a macro. If you do not already have a macro to add the event to, do
  the following:

  1. From the main window's menu, select Record -> New Macro.
  2. Press the macro start/stop key twice. This creates a (nearly) empty
     macro.
  3. Assign a name and a keystroke to the macro (optional).
  4. Open the macro for editing by double-clicking in the white area
     around the macro's name. A new window will open.
  5. Any events already in this window can be deleted. Select the events
     and use the Edit -> Delete menu entry.
  6. Add the new event by choosing it from the \#Event\# menu.

  For more information on what each of the extended events does, see the
  description of the \#{Event}Edit Window "Event" Menu\#.

Related Topics:  \#What Are Macros?\#  \#Record\#  \#Editing Macros\#  \#Event\#
\end

\casesens Suggestions
Suggestions
-----------

\x3 It may be necessary to insert a timer event after a message event, in
  order to keep have the message window appear on screen at the correct
  time.

\x3 A quick way to close a message window is to insert a key event that
  contains the Return keystroke. One application of this would be a
  birthday reminder:

    1. Create a new macro and set it to autostart on the correct day, at a
       time you are likely to be using your computer.
    2. Add the 

\x3 Macros can call other macros. Simply end the first macro with a Key event
 using the keystroke of another macro. A macro can also go in an infinite
 loop by doing this same thing with its own starting keystroke. Hold both
 Shift keys to stop any macro from playing back.

Related Topics:  \#Topics\#
\end

\casesens Advanced Topics
\casesens commandline
Advanced Topics: Commandline Parameters
---------------------------------------

  If the Macro Editor is passed the name of a file when run as a GEM
  program (usually with the PRG or GTP filename extension), it loads this
  file automatically at startup.

  Furthermore, if any text after the filename matches the name of a macro
  contained in that file, then the macro is executed. The Macro Editor will
  not open its window and will quit as soon as the macro has finished
  playing, unless the [Control] key on the keyboard is held.

  Example 1: open the file NEODESK.GMA in the default GMA directory

    Paramters passed:  neodesk.gma

  Example 2: open the file C:\\GENEVA\\NEODESK.GMA

    Paramters passed:  c:\\geneva\\neodesk.gma

  Example 3: open the file NEODESK.GMA and execute the macro "Open C:"

    Paramters passed:  neodesk.gma Open C:

  Example 4: execute the macro "Open C:" from DEFAULT.GMA

    Paramters passed:  Open C:

Advanced Topics: Cut/Paste
--------------------------

  When the Macro Editor cuts events from an editor window, these events are
  stored in a format that allows them to be pasted back into the main
  window as a new macro. This allows you to isolate part of a macro and
  create a new macro from it. The new macro will, of course, have no name
  or keyboard equivalent initially.

  Similarly, an entire macro that has been saved to the clipboard from the
  main window can be pasted into an edit window.

Advanced Topics: Making Large-scale Changes
-------------------------------------------

  Because the Macro Editor uses a standard SCRAP.TXT file, you can make
  more complex changes to a macro by using a text editor:

  1. Copy the macro to the clipboard using the Edit menu.

  2. Locate the SCRAP.TXT file in your clipboard folder and load it into a
     text editor or word processor. Depending on the program you are using,
     you may be able to simply Paste it into the program.

  3. Make the appropriate changes, and save the file. Be sure to save "as
     ASCII" if you are using a word processor.

  4. Paste the macro back into the Macro Editor.

Advanced Topics: Foreign Keyboards
----------------------------------

  A recorded keystroke consists of two things: a "scan code", which is a
  number saying which physical key was pressed on the keyboard, and the
  ASCII value, which is the character produced by that keypress. If these
  values do not match properly, a program you ask to process the macro may
  not react as you expect. It may act as though a different key alltogether
  has been pressed.

  The Macro Editor tries very hard to make sure that the ASCII value and
  the scan code of a keypress will always match. However, you should be
  aware that there is one case where the two will be different, and that is
  for different versions of TOS intended for different countries. If, for
  instance, you record the keypress to produce a double-quote, ", on a US
  keyboard and play this macro back on a machine with UK TOS, you may not
  get the expected result because the key at this position on the keyboard
  is supposed to produce a different ASCII value. Since some programs rely
  on the ASCII portion of the keycode and others rely on the scan code, it
  is difficult to predict what will happen.

  The solution is to edit the macro and correct the keypress. One other
  situation that may cause a similar problem is if you use a utility which
  allows you to alter the keyboard mapping. A macro recorded with one
  keyboard configuration may not produce the same results with another.

Related Topics:  \#Topics\#  
\end
