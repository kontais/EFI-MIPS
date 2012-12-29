# Adds exit() and quit() built in functions

def EfiExit(ReturnVal = 0):

    import sys
    sys.exit(ReturnVal)


import __builtin__
__builtin__.quit = __builtin__.exit = EfiExit
