*** This is a comment, because it appears before the first directive.
*** Define an index screen which is different from the default
\index
  This is an optional index. Without it, the default alphabetical index
  would be used.

          Double-click on a screen name below:

  \#First Screen\#      \#Second Screen\#      \#Third Screen\#
  \#Test "extern"\#     \#Test named links\#
\end

*** Define a normal, case insensitive screen. Also give it an alias.
\screen Normal Topic
\screen First Screen
First Screen

  This is the text of a help screen which can be found regardless of the
  case of the string the user supplies.
  
  Here is a reference to the screen, below:  \#Second Screen\#
        and a reference to a hidden screen:  \#Hidden\#
\end

*** Define a case-sensitive screen.
\casesens Second Screen
Second Screen

  This screen can be jumped to from the \#First Screen\#, or it can be
  searched for in the index.
\end

*** Define a screen with a Hidden name and a normal name
\hidden Hidden
\screen Third Screen
Third Screen

  While the name "Third Screen" will appear in the index, the name
  "Hidden" will not. Furthermore, it cannot be searched for at all.
\end

\screen Test "extern"
Test of "extern" function

  This screen contains a reference to a topic which appears in the file
  TASKMAN.HLP. The "extern" function instructs the help viewer to look
  in a particular HLP file first.

  Jump to TASKMAN.HLP:   \#Task Manager\#

  \@extern TASKMAN.HLP
\end

\screen Test named links
Test of named links

  This links to \#{Hidden} another screen\#, however, the screen's name
  does not appear to the user.

  This is the same thing
\@button 25 5 13 {Hidden}In A Button
\end
