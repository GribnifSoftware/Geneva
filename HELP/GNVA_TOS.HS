\screen Geneva TOS
Geneva TOS
----------

  Geneva TOS is a program which allows you to run TOS (text-based) programs
  in a GEM window. This is a big advantage, since it will often allow you
  to use other GEM programs while having a TOS program active.

Related Topics: \#Setup\#       \#Running a Program\#    \#TOS Window\#
                \#File Menu\#   \#Selecting a Block\#
\end

\screen Setup
Setup
-----

  In order for Geneva TOS to be used effectively, it should be set up to
  run automatically by including a line in \#GEM.CNF\# which defines the
  \#TOSRUN\# environmental variable, like this:

    \#setenv\# \#TOSRUN\#=c:\\geneva\\gnva_tos.prg

  If the \#TOSEXT\# variable is not already defined, create one, like so:

    \#setenv\# \#TOSEXT\#=tos,ttp

  This will make sure that any program ending in ".TOS" or ".TTP" gets run
  by Geneva TOS.

Related Topics: \#Geneva TOS\#
\end

\screen Running a Program
Using Geneva TOS
----------------

  Geneva TOS can run a program in several ways:

    The name of a TOS program can be passed to the GNVA_TOS.PRG as a
    parameter. From NeoDesk, you can do this by simply dragging the TOS
    program's icon to GNVA_TOS.PRG's icon.

    When GNVA_TOS.PRG is in memory, you can select the \#Open\# option from
    its \#File\# menu to run a TOS program.

    If Geneva TOS has been properly set up as the \#TOSRUN\# program then
    any time you run a TOS program, Geneva TOS will be run automatically
    and it will then run the TOS program. Redirection can also be
    accomplished on the \#commandline\#.

Related Topics: \#Geneva TOS\#
\end

\screen Commandline
Commandline Redirection
-----------------------

  When a TTP program is called, most shells will give you the opportunity
  to pass parameters to the program.

  If GNVA_TOS is passed commandline parameters in order to run a TOS or TTP
  program, then the commandline can include the redirection operators >
  (copy output to a file), >> (append output to a file), and < (take input
  from a file). For example, to have ARC.TTP copy any error messages to
  OUT.TXT:

    x archive.arc > out.txt

  IMPORTANT:  If you are running the program from NeoDesk, then you must
  put the redirectors in single quotes when typing them into NeoDesk's
  "Parameters for Execution" dialog. This prevents NeoDesk from trying to
  do the redirection itself. For example:

    x archive.arc '> out.txt'

Related Topics: \#Copy Output to\#  \#Take Input from\#  \#Running a Program\#
\end

\screen TOS Window
The Geneva TOS Window
---------------------

  The main window in Geneva TOS is just like most others. You can scroll
  backward to view previous lines of output from the program, access a menu
  bar, and change the attributes of the window.

  The main window can also be scrolled from the keyboard, provided the
  \#Use hot-keys\# miscellaneous option is on:

    [Control] up arrow              Up one line
    [Control] down arrow            Down one line
    [Control][Shift] up arrow       Up one page
    [Control][Shift] down arrow     Down one page
    [Control][Clr/Home]             Scroll so that the cursor is visible

  The main window can be "iconified" by clicking on the "delta" gadget.
  This shrinks the window to a single "icon" window. Double-clicking on the
  icon window's contents will reopen Geneva TOS. While the window is
  iconified output from the TOS program continues, and is held in the
  scrollback buffer.

Related Topics: \#Geneva TOS\#
\end

\screen File Menu
\screen File
File Menu
---------

  The following options are available in the File menu:

    \#Open\#
    \#Copy Output to\#
    \#Take Input from\#
    \#Copy to Clipboard\#
    \#Save Settings\#
    \#Reload Settings\#

Related Topics: \#Geneva TOS\#
\end

\screen Open
Open
----

  This option produces an \#item selector\#, from which you can select the
  name of a TOS program to run. After selecting the name of a program, you
  are given the opportunity to provide commandline parameters for the
  program.

  If there is not currently a program running, then the new program will be
  run in the same window from which you select this option. If, however,
  there is already a TOS program running, a new copy of Geneva TOS will
  run, and it will run the TOS program.

Related Topics: \#File Menu\#
\end

\screen Copy Output to
Copy Output to
--------------

  Sometimes, it is helpful to be able to have a program's output go
  automatically to a file on disk. This option will provide you with an
  item selector, from which you can select the name of a file to receive a
  copy of any text the TOS program writes to the Geneva TOS window.

  If the file whose name you choose already exists, you will be asked if
  you want to overwrite the file or append the new output to the end of the
  file.

  This option continues to send output to the file until either an error
  occurs when writing the file, or you select the "Copy Output to..."
  option a second time.

  Redirection can also be accomplished on the \#commandline\#.

Related Topics: \#File Menu\#
\end

\screen Take Input from
Take Input from
---------------

  This option will allow you to have the contents of a file take the place
  of keyboard input into a TOS program. This can be useful when you need to
  repeat a long procedure that would otherwise require lots of keystrokes.
  Again, an item selector prompts you for the name of the file to act as
  input into the TOS program. The file must already exist.

  This option continues to use the file for input until either an error
  occurs when reading the file, the end of the file is reached, or you
  select the "Take Input from..." option a second time.

  Redirection can also be accomplished on the \#commandline\#.

Related Topics: \#File Menu\#
\end

\screen Copy to Clipboard
\screen Selecting a Block
Copy to Clipboard
-----------------

  You can select a block of text with the mouse in several ways.
  Double-clicking on a word in the text will select just that word.  Also,
  clicking with the left mouse button will begin a text selection. As the
  mouse pointer is dragged, the selection grows in size.

  Once a block has been selected, you can also quickly lengthen the block
  by holding either [Shift] key while clicking on an unselected area of the
  text. Furthermore, you can shorten the current block by [Shift]-clicking
  on an area which is already selected.

  The "Copy to Clipboard" option will save any block of text that is
  currently selected into a clipboard file. Other programs which support
  the Atari standard clipboard (like STalker 3) can then "Paste" the
  contents of this file into their own documents.

Related Topics: \#File Menu\#
\end

\screen Save as Text
\screen Save Block
Save as Text
------------

  This option will save the current screen, in its entirity, to a text
  file. You are presented with an item selector, from which you should
  select the name of a file to receive the text.

Save Block
----------

  This option will save the currently selected block of text into a file on
  disk. You are presented with an item selector, from which you should
  select the name of a file to receive the text.

Related Topics: \#File Menu\#
\end

\screen Settings
\screen Save Settings
\screen Reload Settings
Save Settings
-------------

  This option will cause Geneva TOS to save all of its configuration
  options in a file called \#GENEVA.CNF\#. Whenever you run Geneva TOS,
  these settings are automatically re-read.

Reload Settings
---------------

  This option will reload all of Geneva TOS's settings from the
  \#GENEVA.CNF\# file.

Related Topics: \#File Menu\#
\end

\screen Options Menu
Options Menu
------------

  The following options are available in the Options Menu:

    \#Font\#
    \#Scrollback\#
    \#Window Elements\#
    \#Misc. Options\#

Related Topics: \#Geneva TOS\#
\end

\screen Font
Font
----

  This option allows you to change the character font and size that are
  used in the Geneva TOS window. This can be a useful way to increase the
  number of lines of text which are visible at one time.

  Geneva TOS uses Speedo GDOS or regular GDOS monospaced fonts. If more
  than one font of this type is available, you can choose which one to use
  with the pop-up menu.

  Since no check is done to make sure that a font has all 255 ASCII
  characters, using a font (like the Monospace 821 font that comes with
  Speedo GDOS) which does not have all of them may produce unexpected
  results, such as blanks or question marks being displayed for some
  characters.

Related Topics: \#Options Menu\#
\end

\screen Scrollback
Scrollback
----------

  This option controls the size of the virtual window that Geneva TOS uses
  to display text. Under most circumstances, you will probably want to
  leave the number of columns at 80.

  The number of lines, however, affects how many lines of text Geneva TOS
  will hold in memory at one time. Once this limit has been reached, as the
  display scrolls, the oldest line of text is destroyed to make way for the
  new line. Of course, the more lines of text are being held in memory, the
  more memory will be used whenever Geneva TOS runs.

Related Topics: \#Options Menu\#
\end

\screen Window Elements
Window Elements
---------------

  The three options under this heading control which portions of the window
  are used. You can increase the visible area of the window by turning off
  some of the GEM window gadgets.

Related Topics: \#Options Menu\#
\end

\screen Misc. Options
\screen Use hot-keys
\screen Set Line A variables
\screen Pass TERMCAP variable
\screen Use extended argument passing
\screen ARGV
\screen Pause when TOS program exits
Misc. Options
-------------

  This dialog allows you to change a few of the more obscure options
  available for programs that are run by Geneva TOS:

  Use hot-keys

    This option controls whether or not Geneva TOS will compare the keys
    you press with the window movement keys or with the keyboard
    equivalents for menu entries. If a program expects to see and act upon
    a sequence like [Control] up arrow and this option is on, Geneva TOS
    will try to act upon the keystroke and the TOS program will never see
    it.

  Set Line A variables

    In order to improve compatibility with some TOS programs, Geneva TOS
    can set these special variables. Some programs check these values to
    determine how large the TOS screen is and what size the TOS text
    characters are.

  Pass TERMCAP variable

    TERMCAP is a carry-over from the Unix operating system. It contains a
    description of TERMinal CAPabilities. This will only be useful for
    programs which are expecting to receive this environmental variable,
    and will have no effect in other programs.

  Use extended argument passing

    Atari has defined a way to pass parameters of unlimited length to
    programs. If this option is on, Geneva TOS will use this method
    (sometimes called "ARGV") when it runs a TOS program.

  Pause when TOS program exits

    If this option is checked, Geneva TOS will display a message prompting
    you to press any key. If this option is not checked, Geneva TOS will
    close as soon as the TOS program terminates. In order to be able to
    examine a program's output after it has terminated, you need to have
    this option checked.

Related Topics: \#Options Menu\#
\end

