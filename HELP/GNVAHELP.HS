\screen Getting Help
Accessing the Help Text Viewer
------------------------------

  Geneva provides access to its own Help menu four ways:

  \x3 From Geneva's GEM menu bar, under the "Options" menu. This is
    automatically opened if you press the [Help] key while Geneva is the
    foreground application.

  \x3 From the \#Task Manager\#'s GEM window, by choosing "Help" under its
    "Options" menu or by pressing the [Help] key when the Task Manager's
    window is the foreground window.

  \x3 From dialogs that have a HELP button. You can click on the button or
    press the [Help] key.

  \x3 From the Geneva TOS program's GEM window, by selecting the "Help" under
    its "Options" menu.

  In order for the \#Help File Viewer\# to come up automatically, you must
  have a \#SHOWHELP\# variable installed in \#GEM.CNF\#.

  Related Topics:  \#Window\#  \#Status Line\#
\end

\screen Help Viewer
\hidden Help Text Viewer
\screen Window
The Main Help Viewer Window
---------------------------

  Once you have opened the Help window, you can leave it on the screen for
  reference at any time. It can be resized to a very small area on the
  screen by using the "iconify" (\x7f) gadget. Double-click on the
  iconified window to restore the Viewer to full size.

  The Help File Viewer displays text within its window with certain words
  or phrases underlined.  These are alternate topics, which can be
  referenced by double-clicking on them with the mouse.

  The window's contents can be scrolled with the keyboard by using the
  arrow keys. Hold either [Shift] key to scroll a page at a time.

  Related Topics:  \#Getting Help\#   \#Selecting a Block\#  \#File Menu\#
                   \#SHOWHELP\#       \#Topic Menu\#         \#Status Line\#
                   \#Settings Menu\#
\end

\screen File Menu
The File Menu
-------------

  The "File" menu provides a way to open the Help text if Geneva has not
  located it, and to copy portions of Geneva's Help text to an ASCII
  (standard text) file or to the clipboard. You can also quit the Help menu
  from the File menu.

  Menu entries:
    \#Open Help File\#
    \#Set Help File Path\#
    \#Copy to Clipboard\#
    \#Save Text\#
    \#Save Block\#
    \#Quit\#

  Related Topics:  \#Selecting a Block\#  \#Help Viewer\#
\end

\screen Open Help File
Open Help File
--------------

  Choose this option to select a new help file to view. You will be
  presented with an \#item selector\#, from which you should select the name of
  a help file (which ends with the ".HLP" filename extension). The Help
  Text Viewer is capable of reading help files created by Geneva's own help
  text compiler, and also help files created for the Pure C development
  environment.

  By default, the \#Index\# of the new help file is always displayed first.

  Related Topics:  \#File Menu\#
\end

\screen Set Help File Path
Set Help File Path
------------------

  This option allows you to select a path which contains other help files.
  These files are searched when a particular topic is not found in the
  current help file. Most likely, you will want to have this path point to
  the \\GENEVA\\HELP folder on the disk where Geneva is stored.

  Related Topics:  \#File Menu\#
\end

\screen Selecting a Block
Selecting a Block
-----------------

  You can select a block of help text with the mouse in several ways.
  Double-clicking on a word in the help text will select just that word.
  Also, clicking with the left mouse button will begin a text selection. As
  the mouse pointer is dragged, the selection grows in size. If the mouse
  pointer is dragged above the first line, or below the last line in the
  display, the window will automatically scroll in the appropriate
  direction.  If either [Shift] key is held the window will scroll forward
  or backwards one screen at a time.

  Once a block has been selected, you can also quickly lengthen the block
  by holding either [Shift] key while clicking on an unselected area of the
  text. Furthermore, you can shorten the current block by [Shift]-clicking
  on an area which is already selected.

  Related Topics:  \#Copy to Clipboard\#  \#Save Block\#
\end

\screen Copy to Clipboard
Copy to Clipboard
-----------------

  The "Copy to Clipboard" option will save any block of help text that is
  currently selected into a clipboard file. Other programs which support
  the Atari standard clipboard (like STeno 2.1) can then "Paste" the
  contents of this file into their own documents.

  Related Topics:  \#Selecting a Block\#  \#File Menu\#
\end

\screen Save Text
Save Text
---------

  This option will save the current help screen, in its entirity, to a text
  file. You are presented with an item selector, from which you should
  select the name of a file to receive the text.

  To save just a portion of the text, use \#Save Block\#.

  Related Topics:  \#File Menu\#
\end

\screen Save Block
Save Block
----------

  This option will save the currently selected block of text into a file on
  disk. You are presented with an \#item selector\#, from which you should
  select the name of a file to receive the text.

  Related Topics:  \#Selecting a Block\#  \#File Menu\#
\end

\screen Quit
Quit
----

  When the \#Help Text Viewer\# is running as a desk accessory, this option
  will simply close the Viewer's window. If you require help later on, the
  desk accessory will simply load the help file without running a second
  time.

  When the Viewer is running as a program, this option will cause its
  window to close and the program will be removed from memory.

  Related Topics:  \#File Menu\#
\end

\screen Topic Menu
Topic Menu
----------

  The "Topic" menu provides a search function. "Find ..." opens a dialog in
  which you can type a search string, with an option to have Geneva match
  the case of the string. "Index" displays the help file's index, which is
  usually a list containing the letters "A" to "Z" and a category called
  "Other".

  Menu entries:
    \#Find\#
    \#Index\#
    \#Previous\#
    \#Using Viewer\#

  Related Topics:  \#Help Viewer\#
\end

\screen Find
Find
----

  This option opens a dialog into which you can type a search string
  (including \#wildcard characters\#.)

  If the "Match upper/lowercase differences" option is set, then the case
  of the string you type must exactly match the name of the help topic, as
  it is stored in the help file. If the topic is not found in the current
  help file, and the "Search alternate help file path" option is selected,
  then all files in the path set by the \#Set Help File Path\# option in
  the \#File menu\# will be searched until a match is found.

  Related Topics:  \#Topic Menu\#
\end

\screen Index
Index
-----

  This option displays a list of either the letters "A" to "Z" (and a
  category called "Other"), or all of the topics in the current help file
  in alphabetical order.

  Related Topics:  \#Topic Menu\#
\end

\screen Previous
Previous
--------

  To return to the previous help topic in the hierarchy, you can either
  press the [Undo] key on the keyboard, or select the "Previous" option
  from the menu.

  Related Topics:  \#Topic Menu\#
\end

\screen Using Viewer
Using Viewer
------------

  This option will open the help file you are reading right now.

  Related Topics:  \#Topic Menu\#
\end

\screen Settings Menu
Settings Menu
-------------

  The Settings menu lets you change both the size of the Help text's
  typeface and, if any version of GDOS is loaded, the font itself. Any
  Speedo GDOS font, or monospaced bitmapped font can be used, at any size.

  The Settings menu also provides a way to save and reload the font
  preference. Font preferences take effect immediately. You don't need to
  reboot to see them. Using a small font will allow you to make the Help
  window quite small.

  Related Topics:  \#Help Viewer\#
\end

\screen Status Line
The Status Line
---------------

  The Help window's status line, below the GEM title bar, shows an updated
  display of your location in the Help hierarchy.  Categories and
  sub-categories you have entered are shown from left to right. This
  provides a quick visual reference, and indicates what level you'll go
  back to if you choose \#Previous\# or press the [Undo] key.

  Related Topics:  \#Window\#
\end
