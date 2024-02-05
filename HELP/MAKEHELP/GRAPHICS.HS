\index
\hidden Index
Help File Graphics Demonstration
--------------------------------

  This file demonstrates the graphics functions which can be included
  in a help file generated with the Geneva MakeHelp program.

  Please choose a topic below:



  \@button 2  8 7 Boxes
  \@button 11 8 7 Fills
  \@button 20 8 7 Images
  \@button 29 8 7 Lines
  \@button 38 8 14 Writing Modes
\end

\screen Images
Images
------

  Below is an image which will be rescaled depending on the font size.
  Its aspect ratio will even be corrected for ST Medium resolution.




  Here is the same image, at three times the size:








  \@image 2 6 50 2 2color.img 16color.img
  \@image 2 11 50 6 2color.img 16color.img
  \@button 62 1 7 Index
\end

\screen Writing Modes
Writing Modes (swrite)
-------------

  1: Replace     2: Transparent

                                      Here is some regular text

                                      Here is some XORed text
  3: XOR         4: Erase
                                      Here is some lightened text



  \@sfillint 2
  \@sfillstyle 2 \@bar 2 5 8 2
  \@sfillstyle 5 \@bar 4 4 4 4
  \@swrite 2
  \@sfillstyle 2 \@bar 17 5 8 2
  \@sfillstyle 5 \@bar 19 4 4 4
  \@swrite 3
  \@sfillstyle 2 \@bar 2 10 8 2
  \@sfillstyle 5 \@bar 4 9 4 4
  \@swrite 4
  \@sfillstyle 2 \@bar 17 10 8 2
  \@sfillstyle 5 \@bar 19 9 4 4
  \@swrite 3
  \@sfillstyle 8 \@recfl 50 6 7 3
  \@swrite 2
  \@sfillcol 0 \@sfillstyle 4 \@recfl 50 8 11 3
  \@button 62 1 7 Index
\end

\screen Boxes
Boxes
-----

  These are the types of boxes that are supported:

    bar     (swrite, sfill, and sline functions affect display)



    recfl   (swrite and sfill functions affect display)



    rbox    (swrite and sline functions affect display)

                  Interior of box is transparent

    rfbox   (swrite, sfill, and sline functions affect display)



  \@sfillint 2 \@sfillstyle 2 \@slinewidth 3
  \@bar 4 6 59 3
  \@recfl 4 10 59 3
  \@rbox 4 14 59 3
  \@rfbox 4 18 59 3
  \@button 62 1 7 Index
\end

\screen Lines
Lines
-----

  Line colors:      Styles:    Start/End Styles:
   1             0xFF             0
   2             0x55
   3             0xF0             1
   4             0xCC
   5              (slinestyle)    2
   6                               (slineends)
   7              Thicknesses:
   8               1
   9               2
  10               3
  11               4
  12               5
  13               6
  14               7
  15              (slinewidth)
  (slinecol)
  \@line 4 4 12 4  \@line 21 4 29 4
  \@slinewidth 5 \@line 35 4 45 4
  \@slineends 0 1 \@line 35 6 45 6
  \@slineends 0 2 \@line 35 8 45 8
  \@slineends 0 0 \@slinewidth 1
  \@slinestyle 0x55 \@line 21 5 29 5
  \@slinestyle 0xF0 \@line 21 6 29 6
  \@slinestyle 0xCC \@line 21 7 29 7
  \@slinestyle 0xFF
  \@line 21 11 29 11
  \@slinewidth 2 \@line 21 12 29 12
  \@slinewidth 3 \@line 21 13 29 13
  \@slinewidth 4 \@line 21 14 29 14
  \@slinewidth 5 \@line 21 15 29 15
  \@slinewidth 6 \@line 21 16 29 16
  \@slinewidth 7 \@line 21 17 29 17
  \@slinewidth 1
  \@slinecol 2 \@line 4 5 12 5
  \@slinecol 3 \@line 4 6 12 6
  \@slinecol 4 \@line 4 7 12 7
  \@slinecol 5 \@line 4 8 12 8
  \@slinecol 6 \@line 4 9 12 9
  \@slinecol 7 \@line 4 10 12 10
  \@slinecol 8 \@line 4 11 12 11
  \@slinecol 9 \@line 4 12 12 12
  \@slinecol 10 \@line 4 13 12 13
  \@slinecol 11 \@line 4 14 12 14
  \@slinecol 12 \@line 4 15 12 15
  \@slinecol 13 \@line 4 16 12 16
  \@slinecol 14 \@line 4 17 12 17
  \@slinecol 15 \@line 4 18 12 18
  \@button 62 1 7 Index
\end

\screen Fills
Fill Interiors & Styles (sfillint,sfillstyle)
-----------------------

  0,n   2,1   2,2   2,3   2,4   2,5   2,6   2,7   2,8   2,9



  2,10  2,11  2,12  2,13  2,14  2,15  2,16  2,17  2,18  2,19



  2,20  2,21  2,22  2,23  2,24  3,1   3,2   3,3   3,4   3,5



  3,6   3,7   3,8   3,9   3,10  3,11  3,12



  \@sfillint 0 \@bar 1 4 6 3
  \@sfillint 2
  \@sfillstyle 1 \@bar 7 4 6 3
  \@sfillstyle 2 \@bar 13 4 6 3
  \@sfillstyle 3 \@bar 19 4 6 3
  \@sfillstyle 4 \@bar 25 4 6 3
  \@sfillstyle 5 \@bar 31 4 6 3
  \@sfillstyle 6 \@bar 37 4 6 3
  \@sfillstyle 7 \@bar 43 4 6 3
  \@sfillstyle 8 \@bar 49 4 6 3
  \@sfillstyle 9 \@bar 55 4 6 3
  \@sfillstyle 10 \@bar 1 8 6 3
  \@sfillstyle 11 \@bar 7 8 6 3
  \@sfillstyle 12 \@bar 13 8 6 3
  \@sfillstyle 13 \@bar 19 8 6 3
  \@sfillstyle 14 \@bar 25 8 6 3
  \@sfillstyle 15 \@bar 31 8 6 3
  \@sfillstyle 16 \@bar 37 8 6 3
  \@sfillstyle 17 \@bar 43 8 6 3
  \@sfillstyle 18 \@bar 49 8 6 3
  \@sfillstyle 19 \@bar 55 8 6 3
  \@sfillstyle 20 \@bar 1 12 6 3
  \@sfillstyle 21 \@bar 7 12 6 3
  \@sfillstyle 22 \@bar 13 12 6 3
  \@sfillstyle 23 \@bar 19 12 6 3
  \@sfillstyle 24 \@bar 25 12 6 3
  \@sfillint 3
  \@sfillstyle 1 \@bar 31 12 6 3
  \@sfillstyle 2 \@bar 37 12 6 3
  \@sfillstyle 3 \@bar 43 12 6 3
  \@sfillstyle 4 \@bar 49 12 6 3
  \@sfillstyle 5 \@bar 55 12 6 3
  \@sfillstyle 6 \@bar 1 16 6 3
  \@sfillstyle 7 \@bar 7 16 6 3
  \@sfillstyle 8 \@bar 13 16 6 3
  \@sfillstyle 9 \@bar 19 16 6 3
  \@sfillstyle 10 \@bar 25 16 6 3
  \@sfillstyle 11 \@bar 31 16 6 3
  \@sfillstyle 12 \@bar 37 16 6 3
  \@button 62 1 7 Index
\end

