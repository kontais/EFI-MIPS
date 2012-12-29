/*++

Copyright (c) 2009, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Mips32Math.s

Abstract:

  Core stack switching routine, invoked when real system memory is
   discovered and installed.

--*/

#include "Mips.h"


  .text
  .align 3
  .set noreorder
  
/*---------------------------------------------------------------------------
 * UINT64                             
 * LeftShiftU64 (                     
 *   IN UINT64   Operand,             
 *   IN UINT64   CountIn              
 *   )                                
 *                                    
 * Routine Description:               
 *                                    
 *   Left-shift a 64-bit value.       
 *                                    
 * Arguments:                         
 *                                    
 *   Operand - the value to shift     
 *   Count   - shift count            
 *                                    
 * Returns:                           
 *                                    
 *   Operand << Count                 
 *
 *---------------------------------------------------------------------------*/
  .globl LeftShiftU64
  .ent   LeftShiftU64
  .type  LeftShiftU64, @function
LeftShiftU64:
  jr    ra
  nop
  .end  LeftShiftU64

/*---------------------------------------------------------------------------
 * UINT64                                  
 * RightShiftU64 (                         
 *   IN UINT64   Operand,                  
 *   IN UINT64   CountIn                   
 *   )                                     
 *                                         
 * Routine Description:                    
 *                                         
 *   Right-shift an unsigned 64-bit value. 
 *                                         
 * Arguments:                              
 *                                         
 *   Operand - the value to shift          
 *   Count   - shift count                                                
 *                                         
 * Returns:                                
 *                                         
 *   Operand >> Count                      
 *
 *                                                                         
 *------------------------------------------------------------------------*/
  .globl RightShiftU64
  .ent   RightShiftU64
  .type  RightShiftU64, @function
RightShiftU64:
  jr    ra
  nop
  .end  RightShiftU64

/*--------------------------------------------------------------------------
 * INT64
 * ARightShift64 (
 *   IN INT64  Operand,
 *   IN UINT64 CountIn
 *   )
 *
 * Routine Description:
 *  
 *   Arithmatic shift a 64 bit signed value.
 *
 * Arguments:
 *
 *   Operand - the value to shift
 *   Count   - shift count
 *
 * Returns:
 *
 *  Operand >> Count
 *------------------------------------------------------------------------*/
  .globl ARightShift64
  .ent   ARightShift64
  .type  ARightShift64, @function
ARightShift64:
  jr    ra
  nop
  .end  ARightShift64

/*--------------------------------------------------------------------------
 * UINT64                                                                          
 * MulU64x64 (                                                                     
 *   UINT64 Value1,                                                                
 *   UINT64 Value2,                                                                
 *   UINT64 *ResultHigh                                                            
 *   )                                                                             
 *                                                                                 
 * Routine Description:                                                            
 *                                                                                 
 *   Multiply two unsigned 64-bit values.                                          
 *                                                                                 
 * Arguments:                                                                      
 *                                                                                 
 *   Value1      - first value to multiply                                         
 *   Value2      - value to multiply by Value1                                     
 *   ResultHigh  - result to flag overflows                                        
 *                                                                                 
 * Returns:                                                                        
 *                                                                                 
 *   Value1 * Value2                                                               
 *   The 128-bit result is the concatenation of *ResultHigh and the return value   
 *
 *------------------------------------------------------------------------*/
  .globl MulU64x64
  .ent   MulU64x64
  .type  MulU64x64, @function
MulU64x64:
  jr    ra
  nop
  .end  MulU64x64

/*--------------------------------------------------------------------------
 * INT64                                                                         
 * MulS64x64 (                                                                  
 *   INT64 Value1,                                                              
 *   INT64 Value2,                                                              
 *   INT64 *ResultHigh                                                          
 *   )                                                                          
 *                                                                              
 * Routine Description:                                                         
 *                                                                              
 *   Multiply two signed 64-bit values.                                         
 *                                                                              
 * Arguments:                                                                   
 *                                                                              
 *   Value1      - first value to multiply                                      
 *   Value2      - value to multiply by Value1                                  
 *   ResultHigh  - result to flag overflows                                     
 *                                                                              
 * Returns:                                                                     
 *                                                                              
 *   Value1 * Value2                                                            
 *   The 128-bit result is the concatenation of *ResultHigh and the return value
 *
 *------------------------------------------------------------------------*/
  .globl MulS64x64
  .ent   MulS64x64
  .type  MulS64x64, @function
MulS64x64:
  jr    ra
  nop
  .end  MulS64x64


/*--------------------------------------------------------------------------
 *  UINT64                                                                          
 *  DivU64x64 (                                                                     
 *    IN  UINT64   Dividend,                                                        
 *    IN  UINT64   Divisor,                                                         
 *    OUT UINT64   *Remainder OPTIONAL,                                             
 *    OUT UINT32   *Error                                                           
 *    )                                                                             
 *                                                                                  
 *  Routine Description:                                                            
 *                                                                                  
 *    This routine allows a 64 bit value to be divided with a 64 bit value returns  
 *    64bit result and the Remainder                                                
 *                                                                                  
 *  Arguments:                                                                      
 *                                                                                  
 *    Dividend    - dividend                                                        
 *    Divisor     - divisor                                                         
 *    ResultHigh  - result to flag overflows                                        
 *    Error       - flag for error                                                  
 *                                                                                  
 *  Returns:                                                                        
 *                                                                                  
 *    Dividend / Divisor                                                            
 *    Remainder = Dividend mod Divisor                                              
 *
 *------------------------------------------------------------------------*/
  .globl DivU64x64
  .ent   DivU64x64
  .type  DivU64x64, @function
DivU64x64:
  jr    ra
  nop
  .end  DivU64x64


/*--------------------------------------------------------------------------
 * INT64                                                                  
 * DivS64x64 (                                                            
 *   IN  INT64   Dividend,                                                
 *   IN  INT64   Divisor,                                                 
 *   OUT UINT64   *Remainder OPTIONAL,                                    
 *   OUT UINT32   *Error                                                  
 *   )                                                                    
 *                                                                        
 * Routine Description:                                                   
 *                                                                        
 *   This routine allows a 64 bit signed value to be divided with a 64 bit
 *   signed value returns 64bit result and the Remainder.                 
 *                                                                        
 * Arguments:                                                             
 *                                                                        
 *   Dividend    - dividend                                               
 *   Divisor     - divisor                                                
 *   ResultHigh  - result to flag overflows                               
 *   Error       - flag for error                                         
 *                                                                        
 * Returns:                                                               
 *                                                                        
 *   Dividend / Divisor                                                   
 *   Remainder = Dividend mod Divisor                                     
 *
 *------------------------------------------------------------------------*/
  .globl DivS64x64
  .ent   DivS64x64
  .type  DivS64x64, @function
DivS64x64:
  jr    ra
  nop
  .end  DivS64x64
  
  .set reorder