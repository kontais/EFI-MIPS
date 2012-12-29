#
# Copyright (c) 1999  Intel Corporation
#
#  Sample EFI Shell script demonstrating output redirection and PAUSE cmd
#

echo -on
echo "Output redirection of a command"
echo "The quick brown fox jumped over the purple cow" 1>a cow.txt

echo "Output redirection of a script with a PAUSE command"
RunPause.nsh 1>a pause.txt

echo "Done."
