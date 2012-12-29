#
# Copyright (c) 1999  Intel Corporation
#
#  Sample EFI Shell script demonstrating the use of the "if" command.
#
#  Note that arguments *must* be delimited by whitespace
#
#  The "if exist" test expects a file named "myfile.txt" to be present
#  in the same directory where the script is run.
#

#
#  First demonstrate if string1 == string2 then / endif
#

mode 100 999
echo -off

echo "Demonstrate the 'if string1 == string2' form of the if command..."
echo " "

echo "Test string equality evaluation"
if "mystring" == "mystring" then
    echo "String equality evaluation PASSED"
endif
if "mystring" == "junk" then
    echo "String inequality evaluation FAILED"
endif

echo "Test not keyword for string equality"
if not "mystring" == "junk" then
    echo "Not keyword with string inequality PASSED"
endif
if not "mystring" == "mystring" then
    echo "Not keyword with string equality FAILED"
endif

echo "Test use of env vars w/ str equality"
set testvar foo
set
if %testvar% == foo then
    echo "String equality with env vars PASSED"
endif
if not %testvar% == foo then
    echo "String inequality with env vars FAILED"
endif
if %testvar%bar == foobar then
    echo "embedded env var with string equality PASSED"
endif

echo "Test %lasterror% in if condition (and quote)"
xyz 123
if not %lasterror% == 00000000 then
    echo "Detecting error using lasterror var PASSED"
endif

#
#  Now demonstrate "if exist <file> then / endif"
#
echo " "
echo " "
echo "Now demonstrate 'if exist file' form of the if command..."
echo " "
echo "     ***********************************************************"
echo "     *  Make sure that a file named 'myfile.txt' is present    *"
echo "     *  in the directory where this script is run.  Otherwise  *"
echo "     *  some false failures will be reported!                  *"
echo "     ***********************************************************"
echo " "

echo "Test file existence evaluation"
if exist myfile.txt then
    echo "File existence evaluation PASSED"
endif
if exist junk.txt then
    echo "File non-existence evaluation FAILED"
endif

echo "Test not keyword for file existence"
if not exist junk.txt then
    echo "Not keyword with file non-existence PASSED"
endif
if not exist myfile.txt then
    echo "Not keyword with file existence FAILED"
endif

echo "Test wildcard expansion with file existence"
if exist *.txt then
    echo "Wildcard expansion with file existence PASSED"
endif
if not exist *.xyz then
    echo "Wildcard expansion with file non-existence PASSED"
endif
if exist *.xyz then
    echo "Wildcard expansion with file non-existence FAILED"
endif
if not exist *.txt then
    echo "Wildcard expansion with file existence FAILED"
endif


#
#  Finally, demonstrate nested if statements
#

if %lasterror% == 0 then
    echo "Nested if stmt"
    if exist junk.dat then
        echo "FAILED: inner if should have evaluated to FALSE"
    endif
    if not exist junk.dat then
        echo "PASSED: inner if correct"
    endif
endif
echo Done.
echo -on
