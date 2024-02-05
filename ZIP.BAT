cd c:\temp\
if %not %exists temp
  md temp
  md temp\geneva
  md temp\geneva\setmouse
  md temp\geneva\help
  md temp\program
  md temp\program\makehelp
  md temp\program\taskman
  md temp\program\vectest
  md temp\program\skeleton
  md temp\taskman
  md temp\jarxxx
  md temp\doc
  md temp\fonts
endif
cd h:\source\windows\
set .input zip.dat
while %hasch
  getstr s
  if "$s" != "" && %left "$s" 1 != ';'
    set i %scan "$s" " "
    set i $i - 1
    set f %left "$s" $i
    set i $i + 2
    set s %mid "$s" $i 100
    echo ">>$f" "c:\\temp\\" ~ $s
    cp "$f" "c:\\temp\\" ~ "$s" o-
  endif
endwhile

cd c:\temp\temp
;newlzh u /r ..\geneva.lzh *.*
zipjr -ar -ed ..\geneva.zip *.*
if $errnum != 0
  echo Error $errnum
  exit
endif
cd ..
;rm temp FULL

