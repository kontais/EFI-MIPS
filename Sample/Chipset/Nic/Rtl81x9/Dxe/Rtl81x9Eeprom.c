/*  $OpenBSD: rtl81x9.c,v 1.75 2011/06/21 16:52:45 tedu Exp $ */

/*
 * Copyright (c) 1997, 1998
 *  Bill Paul <wpaul@ctr.columbia.edu>.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *  This product includes software developed by Bill Paul.
 * 4. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Bill Paul AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL Bill Paul OR THE VOICES IN HIS HEAD
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * RealTek 8129/8139 PCI NIC driver
 *
 * Supports several extremely cheap PCI 10/100 adapters based on
 * the RealTek chipset. Datasheets can be obtained from
 * www.realtek.com.tw.
 *
 * Written by Bill Paul <wpaul@ctr.columbia.edu>
 * Electrical Engineering Department
 * Columbia University, New York City
 */


/*++
Copyright (c) 2012, kontais                                               
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module name:
    Rtl81x9Dev.c

Abstract:

--*/

#include "Rtl81x9.h"
#include "Rtl81x9Dev.h"

UINT16 rt8139_eeprom_yeeloong[] = {
  0x8129 ,0x10ec ,0x8139 ,0x10ec ,0x8139 ,0x4020 ,0xe112 ,0x2300,
  0xf28b ,0x14b7 ,0x4d15 ,0xf7c2 ,0x8801 ,0x43b9 ,0xb0f2 ,0x031a,
  0xdf43 ,0x8a36 ,0xdf43 ,0x8a36 ,0x43b9 ,0xb0f2 ,0x1111 ,0x1111,
  0x0000 ,0x7ed7 ,0x0000 ,0x0000 ,0x0000 ,0x0000 ,0x0000 ,0x2000,
  0x0000 ,0x0000 ,0x0000 ,0x0000 ,0x0000 ,0x0000 ,0x0000 ,0x0000,
  0xffff ,0xffff ,0xffff ,0xffff ,0xffff ,0xffff ,0xffff ,0xffff,
  0xffff ,0xffff ,0xffff ,0xffff ,0xffff ,0xffff ,0xffff ,0xffff,
  0xffff ,0xffff ,0xffff ,0xffff ,0xffff ,0xffff ,0xffff ,0xffff};

#define EE_SET(x)          \
  CSR_WRITE_1(Drv, RL_EECMD,      \
    CSR_READ_1(Drv, RL_EECMD) | x)

#define EE_CLR(x)          \
  CSR_WRITE_1(Drv, RL_EECMD,      \
    CSR_READ_1(Drv, RL_EECMD) & ~x)

/*
 * Send a command and address to the EEPROM, check for ACK.
 */

EFI_STATUS
Rtl81x9EepromSendCmd (
  IN RTL81X9_SNP_DRIVER *Drv,
  IN UINT32             Cmd,
  IN UINT32             Addr
  )
{
  UINT32 Shitf;
  UINT32 Data;
  UINT32 Index;

  Shitf = Drv->NvAddrShift;

  Data  = (Cmd << Shitf) | Addr;

  /*
   * Feed in each bit and strobe the clock.
   */
  for (Index = RL_EECMD_LEN + Shitf; Index; Index--) {
    if (Data & (1 << (Index - 1)))
      EE_SET(RL_EE_DATAIN);
    else
      EE_CLR(RL_EE_DATAIN);

    DELAY(10);
    EE_SET(RL_EE_CLK);
    DELAY(150);
    EE_CLR(RL_EE_CLK);
    DELAY(10);
  }

  return EFI_SUCCESS;
}


/*
 * Read a word of data stored in the EEPROM at address 'addr.'
 */
EFI_STATUS
Rtl81x9EepromGetWord (
  IN RTL81X9_SNP_DRIVER *Drv, 
  IN UINT32             Addr,
  OUT UINT16            *Dest
  )
{
  UINT32  Index;
  UINT16  Word = 0;

  /* Enter EEPROM access mode. */
  CSR_WRITE_1(Drv, RL_EECMD, RL_EEMODE_PROGRAM|RL_EE_SEL);

  /*
   * Send address of word we want to read.
   */
  Rtl81x9EepromSendCmd(Drv, RL_EECMD_READ, Addr);

  CSR_WRITE_1(Drv, RL_EECMD, RL_EEMODE_PROGRAM|RL_EE_SEL);

  /*
   * Start reading bits from EEPROM.
   */
  for (Index = 16; Index > 0; Index--) {
    EE_SET(RL_EE_CLK);
    DELAY(10);
    if (CSR_READ_1(Drv, RL_EECMD) & RL_EE_DATAOUT)
      Word |= 1 << (Index - 1);
    EE_CLR(RL_EE_CLK);
    DELAY(10);
  }

  /* Turn off EEPROM access mode. */
  CSR_WRITE_1(Drv, RL_EECMD, RL_EEMODE_OFF);

  *Dest = Word;

  return EFI_SUCCESS;
}

EFI_STATUS
Rtl81x9EepromSetWord (
  IN RTL81X9_SNP_DRIVER *Drv, 
  IN UINT32             Addr,
  IN UINT16             Word
  )
{
  UINT32 Index;
  UINT32 Cmd;
  UINT32 TimeOut;
  UINT32 Shitf;

  Shitf = Drv->NvAddrShift;

  /* Enter EEPROM access mode. */
  CSR_WRITE_1(Drv, RL_EECMD, RL_EEMODE_PROGRAM|RL_EE_SEL);

  /* Write Enable */
  Rtl81x9EepromSendCmd (Drv, RL_EECMD_EWEN, RL_EECMD_EWEN_ADDR);

  Cmd =   (RL_EECMD_WRITE << Shitf) | Addr;
  Cmd <<= 16;
  Cmd |=  Word;

  CSR_WRITE_1(Drv, RL_EECMD, RL_EEMODE_PROGRAM|RL_EE_SEL);

  /*
   * Feed in each bit and strobe the clock.
   */
  for (Index = RL_EECMD_LEN + Shitf + 16; Index; Index--) {
    if (Cmd & (1 << (Index - 1)))
      EE_SET(RL_EE_DATAIN);
    else
      EE_CLR(RL_EE_DATAIN);

    DELAY(10);
    EE_SET(RL_EE_CLK);
    DELAY(150);
    EE_CLR(RL_EE_CLK);
    DELAY(10);
  }

  CSR_WRITE_1(Drv, RL_EECMD, RL_EEMODE_PROGRAM|RL_EE_SEL);

  TimeOut = 1000;
  while (!(CSR_READ_1(Drv, RL_EECMD) & RL_EE_DATAOUT)) {
    CSR_WRITE_1(Drv, RL_EECMD, RL_EEMODE_PROGRAM|RL_EE_SEL);
    DELAY(10);
    if (!(TimeOut--)) {
      RTL81X9_PRINT("%a", "EepromSetWord Time Out\n");
      return EFI_TIMEOUT;
    }
  }

  /* Write Disable */
  Rtl81x9EepromSendCmd (Drv, RL_EECMD_EWDS, RL_EECMD_EWDS_ADDR);

  /* Turn off EEPROM access mode. */
  CSR_WRITE_1(Drv, RL_EECMD, RL_EEMODE_OFF);

  return EFI_SUCCESS;
}

/*
 * Read a sequence of words from the EEPROM.
 */
EFI_STATUS
Rtl81x9EepromRead (
  IN RTL81X9_SNP_DRIVER *Drv,
  IN UINTN              RegOffset,
  IN UINTN              NumBytes,
  IN OUT VOID           *BufferPtr
  )
{
  UINT32    Index;
  UINT16    Word = 0;
  UINT16    *Ptr;
  UINT32    Count;
  UINT32    WordOff;
  UINT32    Width;

  Width   = Drv->Mode.NvRamAccessSize;
  Count   = NumBytes / Width;
  WordOff = RegOffset / Width;

  for (Index = 0; Index < Count; Index++) {
    Rtl81x9EepromGetWord (Drv, WordOff + Index, &Word);
    Ptr  = (UINT16 *)((UINT8 *) BufferPtr + (Index * Width));
    *Ptr = Word;
    //*Ptr = letoh16(Word);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
Rtl81x9EepromWrite (
  IN RTL81X9_SNP_DRIVER *Drv,
  IN UINTN              RegOffset,
  IN UINTN              NumBytes,
  IN OUT VOID           *BufferPtr
  )
{
  UINT32    Index;
  UINT16    Word = 0;
  UINT32    Count;
  UINT32    WordOff;
  UINT32    Width;

  Width   = Drv->Mode.NvRamAccessSize;
  Count   = NumBytes / Width;
  WordOff = RegOffset / Width;

  for (Index = 0; Index < Count; Index++) {
    
    Word  = *(UINT16 *)((UINT8 *) BufferPtr + (Index * Width));
    //Word = htole16(Word);
    Rtl81x9EepromSetWord (Drv, WordOff + Index, Word);
  }

  return EFI_SUCCESS;
}


EFI_STATUS
Rtl81x9EepromUpdate (RTL81X9_SNP_DRIVER *Drv)
{
  return Rtl81x9EepromWrite (Drv, 0, Drv->Mode.NvRamSize, rt8139_eeprom_yeeloong);
}


EFI_STATUS
Rtl81x9EeepromDump (RTL81X9_SNP_DRIVER *Drv)
{
  UINT32 RegOffset;
  UINT16 Word;
  UINT32 Count;

  for (RegOffset = 0; RegOffset < Drv->Mode.NvRamSize; RegOffset += Drv->Mode.NvRamAccessSize)
  {
    if ((RegOffset % 16) == 0) RTL81X9_PRINT("%a", "\n    ");
    Rtl81x9EepromRead (Drv, RegOffset, 2, &Word);
    RTL81X9_PRINT("0x%04x ", Word);  
  }

  RTL81X9_PRINT("%a", "\n    ");

  return (0);
}

