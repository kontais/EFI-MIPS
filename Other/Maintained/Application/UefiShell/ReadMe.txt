
Last modified 06/21/2005

EFI Shell Source Code Release Package

This package is the source code of the EFI Shell. The EFI Shell is a special EFI 
application that allows batch scripting, EFI Shell commands, and other EFI Shell 
applications to be launched.

The EFI Shell is a simple, interactive environment that allows users to do the following:
  * Load EFI device drivers.
  * Launch EFI applications.
  * Boot operating systems.
  * Use a set of basic commands to manage files, system environment variables, and other
elements of EFI.

The EFI Shell provides an environment that can be modified to easily adapt to many different
hardware configurations.

The EFI Shell supports the following:
  * A command line interface
  * A set of Shell internal commands and external commands
  * Batch scripting

For more information about the EFI Shell, user can refer the following documents:
  * EFI Shell User's Guide
  * EFI Shell Developer's Guide
  * EFI Shell Release Notes
  * EFI Shell Getting Started Guide
  
================================================================================
Recommended Usage

To use this EFI Shell release package, user should have installed the EFI Developer 
Kit (EDK), can build the NT32 tip, IPF tip and run the Nt32 emulation on the EDK.

To build the EFI Shell with this package, user should do the following things:
1.  Copy this package the the EDK.
      Suppose the EDK is located at C:\EDK, this package is located at C:\Shell.
      Type the copy command:
      C:\> XCOPY C:\Shell C:\EDK\Other\Maintained\Application
      Make sure that the Shell.inf file is 
        C:\EDK\Other\Maintained\Application\Shell\Shell.inf. 
        
2.  To build an IA 32 version of EFI Shell:  
      Open the description file C:\EDK\Sample\Platform\Nt32\Build\Nt32.dsc
        Replace component 
        "Other\Maintained\Application\Shell\Bin\Shell.inf" with 
        "Other\Maintained\Application\Shell\Shell.inf", like this:
        [Components]
        ¡­¡­
        #Other\Maintained\Application\Shell\Bin\Shell.inf
        Other\Maintained\Application\Shell\Shell.inf
        ¡­¡­

        Type the command:
        Cd C:\EDK\Sample\Platform\Nt32\Build
        Set the environment variable:
        C:\EDK\Sample\Platform\Nt32\Build>Set EDK_SOURCE=C:\EDK
        Build:
        C:\EDK\Sample\Platform\Nt32\Build>nmake
        Run the Nt32 emulation:
        C:\EDK\Sample\Platform\Nt32\Build>nmake run

3.  To build an Itanium(R) architecture version of EFI Shell:
      Open the description file C:\EDK\Sample\Platform\IPF\Build\IPF.dsc
        Replace component 
        "Other\Maintained\Application\Shell\Bin\Shell.inf" 
        with "Other\Maintained\Application\Shell\Shell.inf", like this:
        [Components]
        ¡­¡­
        #Other\Maintained\Application\Shell\Bin\Shell.inf
        Other\Maintained\Application\Shell\Shell.inf
        ¡­¡­

        Type the command:
        Cd C:\EDK\Sample\Platform\IPF\Build
        Set the environment variable:
        C:\EDK\Sample\Platform\IPF\Build>Set EDK_SOURCE=C:\EDK
        Build:
        C:\EDK\Sample\Platform\IPF\Build>nmake
================================================================================

