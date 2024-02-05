\index
\hidden Index
\casesens Geneva View
Geneva File Viewer
------------------

  The Geneva File Viewer gives you a GEM window that acts almost exactly
  like the file viewer in NeoDesk, right down to special effects like
  inverse text and cursor positioning, but also lets you easily scroll
  backward in the text, and even provides the ability to copy or print the
  file.

  Please select a topic from the list below:
    \#Installation\#
    \#Running\#
    \#NeoDesk Desk Accessory\#
    \#Display Modes\#
    \#<More>\#
    \#Quitting\#
    \#Main Window\#
    \#Selecting a Block\#
    \#Dropdown Menus\#
\end

\casesens Installation
Installation
------------

  The Geneva File Viewer can be installed as a desk accessory or as a
  program, though it is probably preferable to use it as a program. Copy
  GNVAVIEW.PRG, GNVAVIEW.RSC, GNVAVIEW.HLP to your GENEVA folder.

  If you do want to use it as a desk accessory, you can rename GNVAVIEW.PRG
  to GNVAVIEW.ACC and place both it and GNVAVIEW.RSC in the same directory
  as the rest of your desk accessories. GNVAVIEW.HLP can either be in that
  directory, or in the GENEVA\\HELP folder.

Related Topics:  \#Index\#  \#Running\#  \#NeoDesk Desk Accessory\#
\end

\casesens Running
Running It
----------

  If you are using a shell program, like NeoDesk, you might be able to
  instruct it to use GNVAVIEW as your text file viewer. In NeoDesk 4, this
  is accomplished in the Settings menu's Paths dialog. Enter the full path
  of GNVAVIEW.PRG (like C:\\GENEVA\\GNVAVIEW.PRG) into the "Alternate file
  viewer" line, and save this new configuration. Now, whenever you double-
  click on a file that is not executable, a new copy of Geneva View will
  open and display the file.

  You can also pass the names of a file to view as a parameter. From
  NeoDesk, you could do this by dragging the file's icon to GNVAVIEW.PRG's
  icon. From a commandline interpreter, you can type a line like:

    gnvaview path\\filename.ext

  If you run GNVAVIEW.PRG without any parameters, the window will open
  without displaying a file, allowing you to \#Open\# one.

Related Topics:  \#Index\#  \#Installation\#  \#NeoDesk Desk Accessory\#
\end

\casesens NeoDesk Desk Accessory
Using Geneva View as a NeoDesk Desk Accessory
---------------------------------------------

  If GNVAVIEW is installed as a NeoDesk Desk Accessory, then files can be
  dragged to an icon on the desktop which represents the accessory version
  of GNVAVIEW.

  In order to do this, you must first tell NeoDesk that Geneva View knows
  how to communicate with it. In NeoDesk 4, this is done in the Settings->
  Accessories dialog. In NeoDesk 3, this is done in the "Set Preferences"
  dialog's "Accessories" sub-dialog. Enter the name GNVAVIEW into one of
  the empty slots in the appropriate dialog.

  You can then drag an icon of GNVAVIEW.ACC from a directory window onto
  the desktop. From that point on, double-clicking on this desktop icon
  will open Geneva View, and dragging a file to this icon will cause Geneva
  View to open and display the file.

Related Topics:  \#Index\#  \#Installation\#  \#Running\#
\end

\screen Display Modes
\casesens <More>
\casesens Quitting
Display Modes
-------------

  The Viewer can work in two different modes, as controlled in the
  \#Scrollback\# dialog. In "Pause after every page" mode, the text file is
  read from disk and displayed one screenful at a time, with a <More>
  prompt at the end of each screen. When you see this prompt, you can
  perform one of the following actions by pressing a key on the keyboard:

  ^C, Q, [Esc], or [Undo]       Quit the Viewer
  [Space]                       Display the next page
  [Enter] or [Return]           Display the next line of the file
  [Up Arrow]                    Scroll up one line
  [Shift][Up Arrow]             Scroll up one page
  [Down Arrow]                  Scroll down one line
  [Shift][Down Arrow]           Scroll down one page
  [Left Arrow]                  Scroll left one line
  [Shift][Left Arrow]           Scroll left one page
  [Right Arrow]                 Scroll right one line
  [Shift][Right Arrow]          Scroll right one page
  [Clr/Home]                    Top of scrollback buffer
  [Shift][Clr/Home]             End of scrollback buffer

  The amount of text you can scroll backward to view is controlled by the
  \#Scrollback\# dialog. When in this mode, the mover bar of the window tells
  you that it is "paging" the file.

  In "Read as much as possible" mode, the Viewer reads as much as it can of
  the file all at once. Because Geneva View is a full VT-52 terminal
  emulator, it needs to interpret each character in the file as it is read,
  and this can take a few seconds for a large file. While the file is being
  read, the window's name changes to show you the relative completeness of
  the operation. You can hold both [Shift] keys at any time, and the Viewer
  will show you what has been read so far and then go to Paging mode.

  As the file is being read, the Viewer automatically increases the size of
  the scrollback buffer to accomodate the new text. However, if there is
  not enough free memory to read more of the file, or if the file gets
  larger than a certain amount, the Viewer will revert to Paging mode for
  the rest of the file.

  Because of the way GEM works, no more than 32,767 rows of pixels can be
  contained in the scrollback buffer at one time. Therefore, the maximum
  number of lines of text that can be viewed depends on the height of the
  font you are using. In simple terms, this means that you can view no more
  than 2047 lines of text when using the 10 point system font, and no more
  than 4095 lines when using the 9 point font. After this number of lines
  has been reached, the Viewer reverts to Paging mode.

Related Topics:  \#Index\#  \#Main Window\#  \#Scrollback\#
\end

\casesens Main Window
The Main Window
---------------

The main window contains these elements:

\x3 Close box:      Clicking here will close the main window. If run as a
                  program, the Viewer will quit.

\x3 Mover bar:      This allows you to move the main window. It also shows
                  you the name of the file being viewed, and gives
                  information about what the Viewer is doing. See the
                  section on \#Display Modes\# for more information.

\x3 Iconify gadget: The iconify (\x7f) gadget will shrink the main window
                  down to a small window toward the lower left of the
                  screen. Double-clicking on the icon in the iconified
                  window will cause the window to reopen.

\x3 Menu bar:       Clicking on a title in the menu bar will allow you to
                  choose items in the menu. See the section below for
                  descriptions of each menu entry.

\x3 Working area:   The working area of the window is where the file is
                  displayed. You can also use the mouse to select portions
                  of the main window for copy and print operations.

\x3 Scroll bars:    The scroll bars allow you to move within the scrollback
                  buffer. The size of this buffer is controlled in the
                  \#Scrollback\# dialog.

Related Topics:  \#Index\#  \#Display modes\#  \#Selecting a Block\#
\end

\casesens Selecting a Block
\hidden block
Selecting a Block
-----------------

  You can select a block of text in the window in several ways. Double-
  clicking with the mouse on a word will select just that word. Also,
  clicking with the left mouse button will begin a text selection. As the
  mouse pointer is dragged, the selection grows in size. If the mouse
  pointer is dragged above the first line, or below the last line in the
  display, the window will automatically scroll in the appropriate
  direction. If either [Shift] key is held the window will scroll forward
  or backwards one screen at a time.

  Once a block has been selected, you can also quickly lengthen the block
  by holding either [Shift] key while clicking on an unselected area of the
  text. Furthermore, you can shorten the current block by [Shift]-clicking
  on an area which is already selected.

Related Topics:  \#Index\#  \#Main Window\#  \#File Menu\#
\end

\casesens Dropdown Menus
Dropdown Menus
--------------

  The File Viewer contains these dropdown menus:

    \#File\#   Options: \#Show as Hex\#
                    \#Font\#
                    \#Scrollback\#
                    \#Window Elements\#
                    \#About Geneva View\#
                    \#Help\#

Related Topics:  \#Index\#  \#Main Window\#
\end

\casesens File Menu
\hidden File
\hidden Open
The File Menu
-------------

\x3 Open:              Allows you to choose a new file to view, using an item
                     selector.

\x3 Copy to Clipboard: The selected \#block\# or, if no block is selected, the
                     entire file, is copied to the Atari standard clipboard.

\x3 Save Block:        The selected \#block\# is saved to disk using the name you
                     supply in the item selector.

\x3 Print:             The selected \#block\# or, if no block is selected, the
                     entire file, is printed. No formatting of the text is
                     performed; it is sent as pure ASCII.

\x3 Form Feed:         If checked, a form feed character will be sent to
                     your printer after any text sent to the printer. Most
                     laser printers need to receive a form feed in order to
                     eject the current page.

\x3 Save Settings:     Saves the position and size of the window, as well
                     as the \#Scrollback\#, \#Font\#, and Form Feed settings,
                     to the GENEVA.CNF file.

\x3 Reload Settings:   Reloads all settings from the values last saved in
                     GENEVA.CNF.

Related Topics:  \#Dropdown Menus\#  \#Selecting a Block\#
\end

\casesens Show as Hex
Show as Hex
-----------

  Normally, a file is interpreted the same way showing a file from Atari's
  desktop works: as printable text and VT-52 codes. However, if a file does
  not contain displayable sequences, it is often useful to be able to view
  the file as a hexadecimal and ASCII dump. When this option is used, the
  file is displayed in 16-byte lines, showing both the hexadecimal value
  and ASCII character. Any ASCII 0 characters appear as a  in the right-
  hand column.

Related Topics:  \#Index\#  \#Dropdown Menus\#
\end

\casesens Font
Font
----

  This dialog lets you change both the size of the text's typeface and, if
  any version of GDOS is loaded, the font itself. Any monospaced or Speedo
  GDOS font can be used, at any size. Using a small font will allow you to
  see more of the text file at once.

  Since no check is done to make sure that a font has all 255 ASCII
  characters, using a font (like the Monospace 821 font that comes with
  Speedo GDOS) which does not have all of them may produce unexpected
  results, such as blanks or question marks being displayed for some
  characters.

Related Topics:  \#Index\#  \#Dropdown Menus\#
\end

\casesens Scrollback
Scrollback
----------

  This option controls the size of the virtual window that Geneva View uses
  to display text. Under most circumstances, you will probably want to
  leave the number of columns at 80.

  The number of lines, however, affects how many lines of text Geneva View
  will hold in memory at one time. Once this limit has been reached, as the
  display scrolls, the oldest line of text is destroyed to make way for the
  new line. Of course, the more lines of text are being held in memory, the
  more memory will be used whenever Geneva View runs.

  For a description of the other options in this dialog, refer to the
  \#Display Modes\# section.

Related Topics:  \#Index\#  \#Dropdown Menus\#
\end

\casesens Window Elements
Window Elements
---------------

  The three options under this heading control which portions of the window
  are used. You can increase the visible area of the window by turning off
  some of the GEM window gadgets.

Related Topics:  \#Index\#  \#Dropdown Menus\#
\end

\casesens About Geneva View
\casesens Help
\x3 About Geneva View: Gives information about the version of Geneva View you
                     are using.

\x3 Help:              Provides you with the online help you're looking at
                     now.

Related Topics:  \#Index\#  \#Dropdown Menus\#
\end
