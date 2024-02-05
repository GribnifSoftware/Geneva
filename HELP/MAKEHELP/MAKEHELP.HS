\index
\hidden Topics
Geneva Makehelp Utility
-----------------------

  This program allows you to create context-sensitive help files for use in
  your own programs. It works by reading a source text file which includes
  various directives to specify how the text should be split up into
  individual screens, and where references to other screens occur.

  Please choose a topic from the list below:

  \#Source Format\#
  \#Screen Types\#
  \#Special Sequences\#
  \#Named Links\#

  \#Functions\#
  \#Function Descriptions\#

  \#Types of Index\#
  \#Commandline Arguments\#

  \#Source Examples\#
  \#Limits\#
\end

\casesens Source Format
Source Format
-------------

  The basic format for a help file source is a straight ASCII file
  containing one or more blocks:

    \\screen_type1 screen_name1
    [\\screen_type2 screen_name2]
      ...
      [Help text]
    \\end

    \\screen_type3 screen_name3
      ...

  Any text appearing after an \\end directive, but before the next screen
  directive, is ignored. This is a good way to add comments to the help
  file source.

  Each screen definition can have one or more names associated with it.
  Which directive you use affects how the name (or "keyword") is treated by
  the help compiler, and the help viewer. The screen_name can contain up to
  43 characters, including any character (spaces, too.) The actual text of
  a help screen can include up to 100 lines of 100 characters each.

Related Topics:  \#Topics\#  \#Screen Types\#  \#Special Sequences\#
                 \#Formatting Suggestions\#  \#Source Examples\#
\end

\casesens Screen Types
Screen Types
------------

  When Makehelp encounters a keywords at the start of a line in the source
  file, it knows that a new screen of help text is being defined, or an
  additional keyword is being added to the current screen.

  A <screen_name> can be up to 43 characters long, and can use any ASCII
  character (including spaces.)

    \\\#screen\# <screen_name>
    \\\#casesens\# <screen_name>
    \\\#hidden\# <screen_name>
    \\\#index\#

  The following table summarizes the three main directives, whether or not
  the keywords they define would appear in the index, and if a search using
  the string "Abc" would match if the keyword defined by each was "aBc".

    Directive       In Index?       Match Case On       Match Case Off
    \\screen            yes               yes                 yes
    \\casesens          yes                no                 yes
    \\hidden             no                no                  no

Related Topics:  \#Topics\#  \#Types of Index\#  \#Source Format\#  \#Source Examples\#
\end

\casesens screen
\\screen <screen_name>
---------------------

  This directive defines a default screen. The name appearing after the
  directive will appear in the help file index, and can be found during a
  search in GNVAHELP regardless of whether the user provides upper or
  lowercase letters in the search specification.

Related Topics:  \#Screen Types\#  \#Source Format\#  \#Source Examples\#
\end

\casesens casesens
\\casesens <screen_name>
-----------------------

  This defines a case-sensitive name. The name will appear in the help file
  index, and can be found during a search only if the case exactly matches
  what the user types, or if he has turned the "Match upper/lowercase"
  option Off.

Related Topics:  \#Screen Types\#  \#Source Format\#  \#Source Examples\#
\end

\casesens hidden
\\hidden <screen_name>
---------------------

  The purpose of this directive is to provide a way for the programmer to
  reference a help screen whose name will not appear in the index and will
  not be considered during searches. The individual "A.." through "Z.."
  entries of a multiple screen help file index are examples of hidden
  screens.

Related Topics:  \#Screen Types\#  \#Source Format\#  \#Source Examples\#
\end

\casesens index
\\index
------

  This directive is optional. It instructs Makehelp to use the screen
  following it as a replacement for the normal index. Only one \\index can
  be specified per help file. Screens of this type can also be aliased with
  other names by follwing the \\index line with one or more of the other
  types of screen directives.

  An index should contain all of the important screen names contained
  throughout the help file, so that the user can easily see the range of help
  topics. Example:

  \\index Index
  \\screen Alternate Name
    This is the body of the screen. Here is a reference to another
    screen: \\#Another Screen\\#
  \\end

Related Topics:  \#Types of Index\#  \#Screen Types\#  \#Source Format\#
                 \#Source Examples\#
\end

\casesens Named Links
\hidden named link
Named Links
-----------

  Sometimes, it is preferable to link to another screen without having to
  use the destination's screen name as the text of the hypertext link. A
  named link looks like a regular link, except it includes the name of the
  destination screen in curly braces {}.

  For example, to link to the screen named "Screen Name" using the text
  "click here" as the underlined hypertext, this would work:

    To jump somewhere else \\#{Screen Name} click here\\#!!

Related Topics:  \#Topics\#  \#Source Format\#  \#Functions\#
                 \#Source Examples\#
\end

\casesens Special Sequences
Special Sequences
-----------------

  For the most part, any character can be entered. There are several
  special control sequences, though:

  \\\\    Substitute a single backslash \\ character.

  \\x    Substitute the ASCII character with the hexadecimal value that
        follows. One or two characters comprise the hex value. Examples:

          This \\x7f is the delta character.
          \\x3 This is the right arrow.

  \\#    This encloses one or more words which denote the name of a screen
        appearing either somewhere else in the help text source, or in
        another help file. The name will appear underlined when viewed in
        the Help Text Viewer. Double-clicking on the name will jump to the
        appropriate help screen. If the name is not in the current help
        file, then the Help Viewer will search in the path set by the user
        with the \#Set Help File Path\# option in the Help Viewer. A
        preferred filename can be set for missing keywords with the
        \\@\#extern\# function.

        If the keyword is not found in the current help source, you will
        receive a warning message from Makehelp. This serves as a reminder
        to verify that the keyword was not mis-typed.

  \\@    Identifies a \#function\#.

Related Topics:  \#Topics\#  \#Source Format\#  \#Functions\#
                 \#Source Examples\#
  \@extern gnvahelp.hlp
\end

\casesens Functions
\hidden function
Functions
---------

  Any word beginning with the sequence \\@ is evaluated for function calls.
  Each call performs a specific purpose, doing something like drawing a
  particular type of graphic within the help screen. Function calls are
  part of a screen; they must appear between one of the screen directives
  and an \\end directive.

  The placement of function calls is not important. All functions are
  evaluated by the Help Viewer after any text in the help screen is
  displayed. However, order is very important, since many function calls
  affect the output from other calls.

  To make calculating \#coordinates\# of graphics objects easier, it is best to
  place all function calls at the end of a screen, after any normal text.
  Since each line holding a function call still counts against the 100
  lines-per-screen limit, it is often helpful to put multiple function
  calls on the same line, like so:

    \\@line 1 1 5 5
    \\@line 2 2 6 6

    is the same as:

    \\@line 1 1 5 5 \\@line 2 2 6 6

  Each function call accepts one or more parameters, and in some cases a
  certain number of parameters is required. Parameters should be separated
  by one or more spaces in the source file. Numeric parameters can be
  either in hexadecimal (preceded with $ or 0x), or decimal (preceded by
  nothing). These lines are all the same:

    \\@line 20 10 9 11
    \\@line $14 10 9 0xb
    \\@line 0x14 $a $9 $B

  Many of the graphics functions simply set options that affect other
  drawing functions. Because there is no way to know what order the user
  might access help screens in, the options are all reset to their default
  values every time a new screen is encountered. In order to get the
  desired effect, it may be necessary to include the same function calls in
  each of your screens, to initialize the options before any actual drawing
  takes place.

  A .HLP file which uses functions cannot be loaded by older versions of
  GNVAHELP. The version which accompanies release 004 of Geneva (or newer)
  must be used.

Related Topics:  \#Topics\#  \#Source Format\#  \#Special Sequences\#
                 \#Character Coordinates\#  \#Source Examples\#
                 \#Function Descriptions\#
\end

\casesens Character Coordinates
\hidden coordinates
Character Coordinates
---------------------

  Many of the functions use character coordinates to describe the position
  and size of a graphic. The first line after any \\screen, \\casesens,
  \\hidden, and \\index directives is line 0. The upper-left corner of a
  screen is coordinate 0 0. Coordinate 5 0 is the width of 5 text
  characters to the right. For example:

\\screen Example
* This asterisk is at coordinate 0 0.

And * this one is at 4 2.
\\end

  In order to draw most objects using character coordinates in a consistent
  way, a compromise has to be made. When an object is actually drawn, its
  coordinates are adjusted so that they lie in the middle of the
  appropriate character. An object's width and height specify the number of
  character cells the object will be part of; they are not the true size of
  the object. For instance, drawing a box with the function call
  \\@box 1 0 2 2 would actually produce something like this blown-up diagram:

      0   1   2   3

  0

  1

  2

Related Topics:  \#Functions\#  \#Function Descriptions\#
  \@line 4 24 20 24 20 30 4 30 4 24
  \@line 4 26 20 26 \@line 4 28 20 28
  \@line 8 24 8 30 \@line 12 24 12 30 \@line 16 24 16 30
  \@recfl 10 25 5 3
\end

\casesens Function Descriptions
Function Descriptions
---------------------

  In the function descriptions, required parameters appear within < >
  brackets, and optional parameters appear within [ ] brackets. The
  following functions are available:

    \\@\#image\#       A GEM XIMG graphic
    \\@\#button\#      A selectable button that links to another screen

    \\@\#bar\#         A filled, outlined box
    \\@\#rbox\#        A hollow, rounded, outlined box
    \\@\#rfbox\#       A rounded, filled, outlined box
    \\@\#recfl\#       A filled rectangle
    \\@\#line\#        One or more straight lines

    \\@\#swrite\#      Set the writing mode
    \\@\#sfillcol\#    Set the fill color
    \\@\#sfillint\#    Set the fill interior type
    \\@\#sfillstyle\#  Set the fill style
    \\@\#slinecol\#    Set the line color
    \\@\#slineends\#   Set the line start/end types
    \\@\#slinestyle\#  Set the line style
    \\@\#slinewidth\#  Set the line width

    \\@\#extern\#      Define the default file for external keywords

Related Topics:  \#Topics\#  \#Functions\#  \#Character Coordinates\#
\end

\casesens image
\\@image <x> <y> <width> <height> <IMG file1> [IMG file2] [IMG file3]...

  This function includes one or more GEM image (IMG or XIMG) files in the
  source. Unlike all other graphics functions, the coordinates for this
  function really do define the outer size of the entire graphic. The width
  and height are treated as a maximum: the image is expanded until either
  its width or height hits one of these two values.

  Furthermore, the aspect ratio of an image is corrected if the user is in
  a video mode whose aspect differs from that of the image itself. For
  instance, to display an ST High resolution image in ST Medium resolution,
  GNVAHELP compresses the image vertically by 1/2. This prevents the image
  from looking stretched-out.

  By specifying more than one image file name, you are providing alternate
  versions of the same image, to be used depending on the quality of the
  user's video display. XIMG files with any number of bitplanes, up to
  24-bit true color, can be used. When GNVAHELP displays the screen, it
  chooses the image that contains the closest number of bitplanes to the
  current screen mode, without going over. This means that you should
  always have at least a 2-color (1 bitplane) image in the list; otherwise
  anyone looking at your help file in ST High resolution will get a blank
  spot instead of a picture.

  Makehelp does not interpret the contents of an IMG file that is included
  in a HLP file, it simply stores it inside the HLP file. GNVAHELP will
  not display any picture if there is an error in the format of the IMG.
  Example:

    \\@image 3 31 99 3 c:\\images\\2color.img 4color.img 256COLOR.IMG





Related Topics:  \#Functions\#  \#Function Descriptions\#
                 \#Character Coordinates\#
  \@image 3 31 99 3 2color.img 16color.img
\end

\casesens button
\\@button <x> <y> <width> <screen name>

  This function draws a dialog button in the help screen at the chosen
  coordinates. When the user clicks on the button, GNVAHELP will try to
  access a help screen with the name <screen name>. The <screen name> also
  appears as the text inside the button. Example:

    \\@button 4 9 14 Screen Types



  Instead of using normal text, you can also provide a \#named link\# by
  enclosing the location to link to in {}'s. Example:

    \\@button 4 16 14 {Screen Types}Same Thing



Related Topics:  \#Functions\#  \#Function Descriptions\#
                 \#Character Coordinates\#
  \@button 4 9 14 Screen Types
  \@button 4 16 14 {Screen Types}Same Thing
\end

\casesens bar
\\@bar <x> <y> <width> <height>

  Display a filled, outlined box. See also: \\@\#sfill\#, \\@\#swrite\#, \\@\#slinecol\#,
  \\@\#slineends\#, \\@\#slinestyle\#, and \\@\#slinewidth\#. Example:

    \\@bar 4 6 10 2


Related Topics:  \#Functions\#  \#Function Descriptions\#
                 \#Character Coordinates\#
  \@sfillint 2  \@sfillstyle 2  \@bar 4 6 10 2
\end

\casesens line
\\@line <x1> <y1> [x2 y2] [x3 y3]...

  Draw a single point, or a continuous line from coordinate to coordinate.
  See also: \\@\#swrite\#, \\@\#slinecol\#, \\@\#slineends\#, \\@\#slinestyle\#, and
  \\@\#slinewidth\#. Example:

    \\@line 9 7  14 9  9 11  4 9  9 7





Related Topics:  \#Functions\#  \#Function Descriptions\#
                 \#Character Coordinates\#
  \@line 9 7  14 9  9 11  4 9  9 7
\end

\casesens rbox
\\@rbox <x> <y> <width> <height>

  Display a rounded, outlined box with no interior. See also: \\@\#swrite\#,
  \\@\#slinecol\#, \\@\#slineends\#, \\@\#slinestyle\#, and \\@\#slinewidth\#. Example:

    \\@rbox 4 6 10 2


Related Topics:  \#Functions\#  \#Function Descriptions\#
                 \#Character Coordinates\#
  \@sfillint 2  \@sfillstyle 2  \@rbox 4 6 10 2
\end

\casesens rfbox
\\@rfbox <x> <y> <width> <height>

  Display a rounded, filled, outlined box. See also: \\@\#sfill\#, \\@\#swrite\#,
  \\@\#slinecol\#, \\@\#slineends\#, \\@\#slinestyle\#, and \\@\#slinewidth\#. Example:

    \\@rfbox 4 6 10 2


Related Topics:  \#Functions\#  \#Function Descriptions\#
                 \#Character Coordinates\#
  \@sfillint 2  \@sfillstyle 2  \@rfbox 4 6 10 2
\end

\casesens recfl
\\@recfl <x> <y> <width> <height>

  Display a filled rectangle, without an outline. See also: \\@\#sfill\# and
  \\@\#swrite\#. Example:

    \\@recfl 4 6 10 2


Related Topics:  \#Functions\#  \#Function Descriptions\#
                 \#Character Coordinates\#
  \@sfillint 2  \@sfillstyle 2  \@recfl 4 6 10 2
\end

\casesens sfill
sfill Functions
---------------

  The following functions allow you to change the fill attributes:

    \\@\#sfillint\#  \\@\#sfillstyle\#  \\@\#sfillcol\#

Related Topics:  \#Functions\#  \#Function Descriptions\#
\end

\casesens sfillint
\\@sfillint <type>               2 (pattern) by default

  Set the fill interior type, drawn by \\@\#bar\#, \\@\#rfbox\#, and \\@\#recfl\#.
  <type> is one of these values:

    Hollow    0
    Solid     1
    Pattern   2  (default)
    Hatch     3

  A call to \\@\#sfillstyle\# will set the style of the fill when mode 2
  or mode 3 is used.

  \@extern graphics.hlp
  For examples of the types, refer to GRAPHICS.HLP:  \#Fills\#

Related Topics:  \#Functions\#  \#Function Descriptions\#  \#sfill\#
                 \#Character Coordinates\#
\end

\casesens sfillstyle
\\@sfillstyle <style>            7 (solid) by default

  Set the fill interior style, drawn by \\@\#bar\#, \\@\#rfbox\#, and \\@\#recfl\#.
  When \\@\#sfillint\# is set to 2 (Pattern), valid <styles> range from 1
  to 24. When \\@\#sfillint\# is set to 3 (Hatch), values range from 1 to
  12.

  \@extern graphics.hlp
  For examples of the styles, refer to GRAPHICS.HLP:  \#Fills\#

Related Topics:  \#Functions\#  \#Function Descriptions\#  \#sfill\#
                 \#Character Coordinates\#
\end

\casesens sfillcol
\\@sfillcol <color>              1 (black) by default

  Set the color of filled objects, drawn by \\@\#bar\#, \\@\#rfbox\#, and \\@\#recfl\#.
  <color> is the index of the VDI color register to use:

    White            0        Gray             8
    Black (default)  1        Dark Gray        9
    Red              2        Dark Red         10
    Green            3        Dark Green       11
    Blue             4        Dark Blue        12
    Cyan             5        Dark Cyan        13
    Yellow           6        Dark Yellow      14
    Magenta          7        Dark Magenta     15

  \@extern graphics.hlp
  For examples of the colors, refer to GRAPHICS.HLP:  \#Lines\#

Related Topics:  \#Functions\#  \#Function Descriptions\#
                 \#Character Coordinates\#
\end

\casesens slinecol
\\@slinecol <color>              1 (black) by default

  Set the color of lines, drawn by \\@\#bar\#, \\@\#rfbox\#, \\@\#rbox\#, and \\@\#line\#.
  <color> is the index of the VDI color register to use:

    White            0        Gray             8
    Black (default)  1        Dark Gray        9
    Red              2        Dark Red         10
    Green            3        Dark Green       11
    Blue             4        Dark Blue        12
    Cyan             5        Dark Cyan        13
    Yellow           6        Dark Yellow      14
    Magenta          7        Dark Magenta     15

  \@extern graphics.hlp
  For examples of the colors, refer to GRAPHICS.HLP:  \#Lines\#

Related Topics:  \#Functions\#  \#Function Descriptions\#
                 \#Character Coordinates\#
\end

\casesens slineends
\\@slineends <start> <end>       both 0 (square) by default

  Set the type of ends that are used on lines, as drawn by \\@\#line\#, \\@\#bar\#,
  \\@\#rbox\#, and \\@\#rfbox\#. The following values are supported for <start> and
  <end>:

    Square (default)  0
    Arrow             1
    Rounded           2

  Example: Use arrows at start and end of line segments:

    \\@slineends 1 1

  \@extern graphics.hlp
  For examples of the start/end types, refer to GRAPHICS.HLP:  \#Lines\#

Related Topics:  \#Functions\#  \#Function Descriptions\#
                 \#Character Coordinates\#
\end

\casesens slinestyle
\\@slinestyle <style>            0xFF (solid) by default

  Set the appearance of lines, as drawn by \\@\#line\#, \\@\#bar\#, \\@\#rbox\#, and
  \\@\#rfbox\#. This can be used to produce a dashed or dotted effect on lines.

  The style is a single number in the range 0 to 255 ($0 to $FF) whose bits
  give the pattern of the new line. Examples:

    \\@slinestyle 0xFF           XXXXXXXX (solid line, the default)
    \\@slinestyle 0x55           X.X.X.X.
    \\@slinestyle 0xF0           XXXX....
    \\@slinestyle 0xCC           XX..XX..

  \@extern graphics.hlp
  For examples of styles, refer to GRAPHICS.HLP:  \#Lines\#

Related Topics:  \#Functions\#  \#Function Descriptions\#
                 \#Character Coordinates\#
\end

\casesens swrite
\\@swrite <mode>                 1 (Replace) by default

  Set the "writing mode" that will be used by all graphics functions except
  for \\@\#image\# (which always uses Replace mode.) By writing on top of
  existing graphics or text with the various writing modes, different
  effects can be produced.

    Replace     1       (default)
    Transparent 2
    XOR         3
    Erase       4

  \@extern graphics.hlp
  For examples of these modes, refer to GRAPHICS.HLP:  \#Writing Modes\#

Related Topics:  \#Functions\#  \#Function Descriptions\#
                 \#Character Coordinates\#
\end

\casesens slinewidth
\\@slinewidth <width>            1 by default

  Set the thickness of lines drawn with \\@\#line\#, \\@\#bar\#, \\@\#rbox\#, and \\@\#rfbox\#.
  Only odd numbers 1 or greater cause the actual appearance of the line to
  change. Higher numbers produce a thicker line.

  \@extern graphics.hlp
  For examples of the thicknesses, refer to GRAPHICS.HLP:  \#Lines\#

Related Topics:  \#Functions\#  \#Function Descriptions\#
                 \#Character Coordinates\#
\end

\casesens extern
\\@extern <HLP file name>

  When a help file contains references to screen names that are not
  contained in that help file, GNVAHELP searches all .HLP files in the path
  the user has specified in the \#Set Help File Path\# menu entry of
  GNVAHELP.

  This function speeds up the search for an external reference by allowing
  you to tell GNVAHELP the name of the file to look in first. The filename
  should not contain any path or drive information, but should have the
  .HLP extension on the end. Example:

    This is an external reference to Geneva's main help level: \\#Geneva\\#
    \\@extern geneva.hlp

  Only one \\@extern can be specified per help screen. If the help topic is
  not found in the specified file, or the file does not exist, the
  remaining .HLP files are searched, as normal.

Related Topics:  \#Functions\#  \#Function Descriptions\#  \#Special Sequences\#
  \@extern gnvahelp.hlp
\end

\casesens Formatting Suggestions
Source Formatting Suggestions
-----------------------------

\x3 Help screens should include some sort of title.

\x3 Help text is more readable if it is indented, and in short paragraphs.

\x3 Try to use as few lines per screen as possible. Up to 100 lines can be
  used per screen, but this is not recommended.

\x3 Try to limit the visible length of each line of text to less than 80
  characters (75 is probably best). This makes it easier for the user to
  view the entire line when using the system fonts in the Help Text Viewer.

  Note that while the \\# characters are not displayed in the final help
  text, they do consume space internally in the help file. While it is
  possible to have a 75-character line of text with 6 keywords in it (thus
  going past the 100 character-per-line limit), this is not likely to
  happen.

\x3 Do not include Tab characters in the help source.

\x3 While whitespace improves readability when used between paragraphs, it
  only consumes space when used at the ends of lines and after the last
  line of the text.

\x3 Always try to reference as many other sections of the help text as
  possible. Providing a list of related topics at the bottom of each screen
  is very helpful.

Related Topics:  \#Topics\#  \#Source Format\#
\end

\casesens Types of Index
Types of Index
--------------

  By default, Makehelp tries to generate an index for the help file which
  consists of all keywords listed alphabetically on one screen. If this
  would exceed 100 lines, or if you use an optional commandline switch,
  Makehelp will instead generate a main level index with just the entires
  "A.." through "Z.." and will also provide as many individual help screens
  as are necessary for these categories.

Related Topics:  \#Topics\#  \#Commandline Arguments\#  \\\#index\# keyword
\end

\casesens Commandline Arguments
Commandline Arguments
---------------------

  Makehelp can be called from the desktop as a TTP program, or from a
  command line interpreter. It has two distinct modes. The first mode is
  used to automatically highlight all hypertext links that were not already
  pre-defined:

  \x3 Usage: makehelp -s <source> <newsource>

    In this case, <source> is the name of a help source (usually ending in
    ".HS") file. <newsource> is the name of a new .HS file which is
    generated by Makehelp.

    The new file contains all screens in the source file, with all new
    hypertext links starting with the sequence \\## (as opposed to \\#).
    If the same keyword appears more than once in a given screen, only the
    first occurence is marked, so as to avoid too much clutter in the help
    screen.  Since there may be cases where you do not want a particular
    link, the \\## allows you to easily pick out which references were
    generated by Makehelp, and remove the ones that are unwanted.

    Note that all \\## sequences must be either completely removed or
    replaced with \\# before compiling the <newsource>!! You should also be
    aware that the order of the \\\#screen\#s in <newsource> will most
    likely be different from the order they were in in the <source> file,
    and that any comments in <source> will not carry over to <newsource>.
    There may also be other formatting differences which do not affect the
    final .HLP file.

  The other mode is used to generate a finished help (.HLP) file. It has two
  required parameters, and two optional ones:

  \x3 Usage: makehelp [-m] [-n<lines>] <source> <hlpfile>
       -m: force multiple-screen index
       -n: maximum number of lines per screen (default=200)

    The <source> file (which usually ends in the extension ".HS") is
    compiled, and the output is written to the <hlpfile> (which usually
    ends in ".HLP".)

    The -m option forces Makehelp to generate a multiple-screen index, as
    described in the \#Types of Index\# section.

    The -n option allows you to use more than 200 lines of text per help
    screen. Since the help viewer that comes with Geneva 004 and NeoDesk 4
    (005 or older) cannot view more than 100 lines per screen, this option
    can also be used to force a fatal error if more than 100 lines appear
    in any screen. Make sure not to put a space between -n and the number.

Related Topics:  \#Topics\#
\end

\casesens Limits
Limits
------

  Lines per screen:                               (see -n parameter)
  Characters per line (including CR/LF):          100
  Length of a keyword:                            43
  Maximum number of index entries:                5346
  Maximum number of index entries per letter:     198

Related Topics:  \#Topics\#
\end

\casesens Source Examples
Source Examples
---------------

For clarity, the following examples are indented two spaces.

\x3\x3\x3 Examples of some special sequences
  \\screen Sequences
    This is the \\x3 right arrow character.
    Here is the Registered symbol, as part of NeoDesk\\xBE 4.
    This produces a single backslash: \\\\.
  \\end

\x3\x3\x3 Define a normal, case insensitive screen. Also give it an alias
    and a \#named link\#
  *** This is a comment, because it appears before the first directive.
  \\screen Normal Topic
  \\screen First Screen
  First Screen

    This is the text of a help screen which can be found regardless of the
    case of the string the user supplies.
  
    Here is a reference to the screen, below:  \\#Second Screen\\#
              a reference to a hidden screen:  \\#Hidden\\#
         and a named link to the same screen:  \\#{Hidden}Yowza!\\#
  \\end

\x3\x3\x3 A case-sensitive screen
  \\casesens Second Screen
  Second Screen

    This screen can be jumped to from the \\#First Screen\\#, or it can be
    searched for in the index.
  \\end

\x3\x3\x3 A screen with a Hidden name and a normal name
  \\hidden Hidden
  \\screen Third Screen
  Third Screen

    While the name "Third Screen" will appear in the index, the name
    "Hidden" will not. Furthermore, it cannot be searched for at all.
  \\end

\x3\x3\x3 An index screen
  *** Define an index screen which is different from the default
  \\index
    This is an optional index. Without it, the default alphabetical index
    would be used.

          Double-click on a screen name below:

    \\#First Screen\\#      \\#Second Screen\\#      \\#Third Screen\\#
  \\end

\x3\x3\x3 A screen which shows how the \\@extern function works
  \\screen Test "extern"
  Test of "extern" function

    This screen contains a reference to a topic which appears in the file
    TASKMAN.HLP. The "extern" function instructs the help viewer to look
    in a particular HLP file first.

    Jump to TASKMAN.HLP:   \\#Task Manager\\#

    \\@extern TASKMAN.HLP
  \\end

Related Topics:  \#Topics\#  \#Source Format\#  \#Screen Types\#
                 \#Special Sequences\#
\end
