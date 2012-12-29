#
# Copyright (c) 1999  Intel Corporation
#
#  Test the goto command
#

echo "Sample of GOTO command usage"

:unusedlabel

goto mylabel

echo "GOTO FAILED: should have jumped over this"

:mylabel

echo "After jump - if no failures then PASSED"

set count 0
:backlabel
if %count% == 1 then
    set count 2
endif

echo "Show backwards jump - should see this TWICE"

if %count% == 2 then
    goto endlabel
endif

set count 1

goto backlabel

:endlabel
