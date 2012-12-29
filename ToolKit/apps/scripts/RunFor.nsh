#
# Copyright (c) 1999  Intel Corporation
#
#  Sample EFI Shell script demonstrating the "for" command
#
#  Note how arguments and index variables *must* be delimited by spaces
#
echo -off

echo "For loop to print all scripts in this directory..."
for %x in *.nsh 
   echo For loop iteration for %x
endfor

echo "Nested for loops..."
for %i in 1 2 3
   for %j in  a b c
      echo i == %i , j == %j
   endfor
endfor

echo "Done."
echo -on
