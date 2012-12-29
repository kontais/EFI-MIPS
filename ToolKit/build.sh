#
# Copyright c 1999, 2003
# Intel Corporation.
# All rights reserved.
# 

#
# This File export environment variable for EFI Toolkit build
#

# SDK_BUILD_ENV 
#        bios32 : bios32 environment
#	       nt32   : nt32 environment
#        sal64  : sal64 environment 
#        em64t  : em64t environment

# SDK_INSTALL_DIR
#        EFI Toolkit source directory

# EFI_APPLICATION_COMPATIBILITY
#        EFI_APP_102        : Write application for EFI 1.02
#        EFI_APP_110        : Write application for EFI 1.10
#        EFI_APP_MULTIMODAL : Write application for multiple EFI version
# 


export SDK_BUILD_ENV=Mips32
export SDK_INSTALL_DIR=~/EFI-MIPS/ToolKit
export EFI_APPLICATION_COMPATIBILITY=EFI_APP_110
export EFI_DEBUG=YES


if test -z "$1"
then
  echo Usage: source build.sh \[linux32^\|Mips32\]
  echo SDK_BUILD_ENV is export to Mips32 by default
else
  export SDK_BUILD_ENV=$1
fi


echo SDK_BUILD_ENV=$SDK_BUILD_ENV
echo SDK_INSTALL_DIR=$SDK_INSTALL_DIR
echo EFI_APPLICATION_COMPATIBILITY=$EFI_APPLICATION_COMPATIBILITY
echo EFI_DEBUG=$EFI_DEBUG



