/*
 * pci_machdep_cs5536.c  
 *   the Virtual Support Module(VSM) for virtulize the PCI configure  
 *   space. so user can access the PCI configure space directly as
 *  a normal multi-function PCI device which following the PCI-2.2 spec.
 *
 * Author : jlliu <liujl@lemote.com>
 * Date : 07-07-05
 *
 */
#include "TianoCommon.h"
#include "Cs5536_pci.h"
#include "cs5536_io.h"
#include "GlxReg.h"
#include "GlxPci.h"
#include "BonitoReg.h"
#include "Pci.h"
#include "PciCfgLib.h"


/* FOR INCLUDING THE RX REGISTERS */
#define  SB_RX

#define  SB_MAR_ERR_EN    0x00000001
#define  SB_TAR_ERR_EN    0x00000002
#define  SB_SYSE_ERR_EN    0x00000010
#define  SB_PARE_ERR_EN    0x00000020
#define  SB_MAR_ERR_FLAG    0x00010000
#define  SB_TAR_ERR_FLAG    0x00020000
#define  SB_SYSE_ERR_FLAG  0x00100000
#define  SB_PARE_ERR_FLAG  0x00200000

#define  SOFT_BAR_SMB_FLAG    0x00000001
#define  SOFT_BAR_GPIO_FLAG    0x00000002
#define  SOFT_BAR_MFGPT_FLAG    0x00000004
#define  SOFT_BAR_IRQ_FLAG    0x00000008
#define  SOFT_BAR_PMS_FLAG    0x00000010
#define  SOFT_BAR_ACPI_FLAG    0x00000020
#define  SOFT_BAR_IDE_FLAG    0x00000400
#define  SOFT_BAR_ACC_FLAG    0x00000800
#define  SOFT_BAR_OHCI_FLAG    0x00001000
#define  SOFT_BAR_EHCI_FLAG    0x00002000
#define  SOFT_BAR_UDC_FLAG    0x00004000
#define  SOFT_BAR_OTG_FLAG    0x00008000


#define  CS5536_IRQ_RANGE    0xffffffe0  // USERD FOR PCI PROBE
#define  CS5536_IRQ_LENGTH    0x20    // THE REGS ACTUAL LENGTH

#define  CS5536_SMB_RANGE    0xfffffff8
#define  CS5536_SMB_LENGTH    0x08

#define  CS5536_GPIO_RANGE    0xffffff00
#define  CS5536_GPIO_LENGTH    0x100

#define  CS5536_MFGPT_RANGE    0xffffffc0
#define  CS5536_MFGPT_LENGTH    0x40

#define  CS5536_ACPI_RANGE    0xffffffe0
#define  CS5536_ACPI_LENGTH    0x20

#define  CS5536_PMS_RANGE    0xffffff80
#define  CS5536_PMS_LENGTH    0x80

#define  CS5536_IDE_RANGE  0xfffffff0
#define  CS5536_IDE_LENGTH  0x10

#define  CS5536_ACC_RANGE  0xffffff80
#define  CS5536_ACC_LENGTH  0x80

#define  CS5536_OHCI_RANGE  0xfffff000
#define  CS5536_OHCI_LENGTH  0x1000

#define  CS5536_EHCI_RANGE  0xfffff000
#define  CS5536_EHCI_LENGTH  0x1000

#define  CS5536_UDC_RANGE  0xffffe000
#define  CS5536_UDC_LENGTH  0x2000

#define  CS5536_OTG_RANGE  0xfffff000
#define  CS5536_OTG_LENGTH  0x1000

// NOTE THE IDE DMA OPERATION, BASE ADDR CONFIG RIGHT OR WRONG???

/************************************************************************/

/*
 * divil_lbar_enable_disable : enable/disable the divil module bar space.
 */
static void divil_lbar_enable_disable(UINT32 enable)
{
  UINT32 hi, lo;
  
  _rdmsr(DIVIL_LBAR_IRQ, &hi, &lo);
  if(enable)
    hi |= 0x01;
  else
    hi &= ~0x01;
  _wrmsr(DIVIL_LBAR_IRQ, hi, lo);

  _rdmsr(DIVIL_LBAR_SMB, &hi, &lo);
  if(enable)
    hi |= 0x01;
  else
    hi &= ~0x01;
  _wrmsr(DIVIL_LBAR_SMB, hi, lo);

  _rdmsr(DIVIL_LBAR_GPIO, &hi, &lo);
  if(enable)
    hi |= 0x01;
  else
    hi &= ~0x01;
  _wrmsr(DIVIL_LBAR_GPIO, hi, lo);

  _rdmsr(DIVIL_LBAR_MFGPT, &hi, &lo);
  if(enable)
    hi |= 0x01;
  else
    hi &= ~0x01;
  _wrmsr(DIVIL_LBAR_MFGPT, hi, lo);

  _rdmsr(DIVIL_LBAR_PMS, &hi, &lo);
  if(enable)
    hi |= 0x01;
  else
    hi &= ~0x01;
  _wrmsr(DIVIL_LBAR_PMS, hi, lo);

  _rdmsr(DIVIL_LBAR_ACPI, &hi, &lo);
  if(enable)
    hi |= 0x01;
  else
    hi &= ~0x01;
  _wrmsr(DIVIL_LBAR_ACPI, hi, lo);

  return;
}

/******************************************************************************/

/*
 * The following functions are not implemented in  pmon.
 */
static void pci_flash_write_reg(UINT32 Reg, UINT32 Value)
{
  return;
}

static UINT32 pci_flash_read_reg(UINT32 Reg)
{
  return 0xffffffff;
}

/*******************************************************************************/

/*
 * isa_write : isa write transfering.
 * WE assume that the ISA is not the BUS MASTER. 
 */
static void pci_isa_write_reg(UINT32 Reg, UINT32 Value)
{
  UINT32 hi, lo;
  UINT32 temp;
  UINT32 softcom;

  _rdmsr(GLCP_DOWSER, &hi, &lo);
  softcom = lo;
  
  switch(Reg){
  case PCI_COMMAND_OFFSET :
    // command
    if( Value & EFI_PCI_COMMAND_IO_SPACE ){
      divil_lbar_enable_disable(1);
    }else{
      divil_lbar_enable_disable(0);
    }
    /* BUS MASTER : is it 0 for SB???  yes...*/
    /* PER response enable or disable. */
    if( Value & EFI_PCI_COMMAND_PARITY_ERROR_RESPOND ){
      _rdmsr(GLPCI_GLD_MSR_ERROR, &hi, &lo);
      lo |= SB_PARE_ERR_EN;
      _wrmsr(GLPCI_GLD_MSR_ERROR, hi, lo);      
    }else{
      _rdmsr(GLPCI_GLD_MSR_ERROR, &hi, &lo);
      lo &= ~SB_PARE_ERR_EN;
      _wrmsr(GLPCI_GLD_MSR_ERROR, hi, lo);      
    }
    // status
    _rdmsr(GLPCI_GLD_MSR_ERROR, &hi, &lo);
    temp = lo & 0x0000ffff;
    if( (Value & EFI_PCI_STATUS_TARGET_TARGET_ABORT) && 
      (lo & SB_SYSE_ERR_EN) ){
      temp |= SB_SYSE_ERR_FLAG;
    }
    if( (Value & EFI_PCI_STATUS_MASTER_TARGET_ABORT) &&
      (lo & SB_TAR_ERR_EN) ){
      temp |= SB_TAR_ERR_FLAG;
    }
    if( (Value & EFI_PCI_STATUS_MASTER_ABORT) &&
      (lo & SB_MAR_ERR_EN) ){
      temp |= SB_MAR_ERR_FLAG;
    }
    if( (Value & EFI_PCI_STATUS_PARITY_DETECT) &&
      (lo & SB_PARE_ERR_EN) ){
      temp |= SB_PARE_ERR_FLAG; 
    }
    lo = temp;
    _wrmsr(GLPCI_GLD_MSR_ERROR, hi, lo);
    break;
  case PCI_CACHELINE_SIZE_OFFSET :
    Value &= 0x0000ff00;
    _rdmsr(GLPCI_CTRL, &hi, &lo);
    hi &= 0xffffff00;
    hi |= (Value >> 8);
    _wrmsr(GLPCI_CTRL, hi, lo);
    break;
  case PCI_BAR0_REG :
    if((Value&PCI_BAR_RANGE_MASK) == PCI_BAR_RANGE_MASK){
      _rdmsr(GLCP_DOWSER, &hi, &lo);
      lo |= SOFT_BAR_SMB_FLAG;
      _wrmsr(GLCP_DOWSER, hi, lo);
    }else if(Value & 0x01){
      hi = 0x0000f001;
      lo = Value & 0x0000fff8;
      _wrmsr(DIVIL_LBAR_SMB, hi, lo);
#ifdef  SB_RX
      hi = ((Value & 0x000ffffc) << 12) | ((CS5536_SMB_LENGTH - 4) << 12) | 0x01;
      lo = ((Value & 0x000ffffc) << 12) | 0x01;
      _wrmsr(GLPCI_R0, hi, lo);
#endif        
    }
    break;
  case PCI_BAR1_REG :
    if((Value&PCI_BAR_RANGE_MASK) == PCI_BAR_RANGE_MASK){
      _rdmsr(GLCP_DOWSER, &hi, &lo);
      lo |= SOFT_BAR_GPIO_FLAG;
      _wrmsr(GLCP_DOWSER, hi, lo);
    }else if(Value & 0x01){
      hi = 0x0000f001;
      lo = Value & 0x0000ff00;
      _wrmsr(DIVIL_LBAR_GPIO, hi, lo);
#ifdef  SB_RX        
      hi = ((Value & 0x000ffffc) << 12) | ((CS5536_GPIO_LENGTH - 4) << 12) | 0x01;
      lo = ((Value & 0x000ffffc) << 12) | 0x01;
      _wrmsr(GLPCI_R1, hi, lo);
#endif        
    }
    break;
  case PCI_BAR2_REG :
    if((Value&PCI_BAR_RANGE_MASK) == PCI_BAR_RANGE_MASK){
      _rdmsr(GLCP_DOWSER, &hi, &lo);
      lo |= SOFT_BAR_MFGPT_FLAG;
      _wrmsr(GLCP_DOWSER, hi, lo);
    }else if(Value & 0x01){
      hi = 0x0000f001;
      lo = Value & 0x0000ffc0;
      _wrmsr(DIVIL_LBAR_MFGPT, hi, lo);
#ifdef SB_RX        
      hi = ((Value & 0x000ffffc) << 12) | ((CS5536_MFGPT_LENGTH - 4) << 12) | 0x01;
      lo = ((Value & 0x000ffffc) << 12) | 0x01;
      _wrmsr(GLPCI_R2, hi, lo);
#endif        
    }
    break;
  case PCI_BAR3_REG :
    if((Value&PCI_BAR_RANGE_MASK) == PCI_BAR_RANGE_MASK){
      _rdmsr(GLCP_DOWSER, &hi, &lo);
      lo |= SOFT_BAR_IRQ_FLAG;
      _wrmsr(GLCP_DOWSER, hi, lo);
    }
    if(Value & 0x01){
      hi = 0x0000f001;
      lo = Value & 0x0000ffe0;
      _wrmsr(DIVIL_LBAR_IRQ, hi, lo);
#ifdef  SB_RX        
      hi = ((Value & 0x000ffffc) << 12) | ((CS5536_IRQ_LENGTH - 4) << 12) | 0x01;
      lo = ((Value & 0x000ffffc) << 12) | 0x01;
      _wrmsr(GLPCI_R3, hi, lo);
#endif        
    }
    break;
  case PCI_BAR4_REG :
    if((Value&PCI_BAR_RANGE_MASK) == PCI_BAR_RANGE_MASK){
      _rdmsr(GLCP_DOWSER, &hi, &lo);
      lo |= SOFT_BAR_PMS_FLAG;
      _wrmsr(GLCP_DOWSER, hi, lo);
    }else if(Value & 0x01){
      hi = 0x0000f001;
      lo = Value & 0x0000ff80;
      _wrmsr(DIVIL_LBAR_PMS, hi, lo);
#ifdef   SB_RX        
      hi = ((Value & 0x000ffffc) << 12) | ((CS5536_PMS_LENGTH - 4) << 12) | 0x01;
      lo = ((Value & 0x000ffffc) << 12) | 0x01;
      _wrmsr(GLPCI_R4, hi, lo);
#endif        
    }
    break;
  case PCI_BAR5_REG :
    if((Value&PCI_BAR_RANGE_MASK) == PCI_BAR_RANGE_MASK){
      _rdmsr(GLCP_DOWSER, &hi, &lo);
      lo |= SOFT_BAR_ACPI_FLAG;
      _wrmsr(GLCP_DOWSER, hi, lo);
    }else if(Value & 0x01){
      hi = 0x0000f001;
      lo = Value & 0x0000ffe0;
      _wrmsr(DIVIL_LBAR_ACPI, hi, lo);
#ifdef   SB_RX        
      hi = ((Value & 0x000ffffc) << 12) | ((CS5536_ACPI_LENGTH - 4) << 12) | 0x01;
      lo = ((Value & 0x000ffffc) << 12) | 0x01;
      _wrmsr(GLPCI_R5, hi, lo);
#endif        
    }
    break;
  default :
    break;      
  }
  
  return;
}

/*
 * isa_read : isa read transfering.
 * we assume that the ISA is not the BUS MASTER. 
 */
static UINT32 pci_isa_read_reg(UINT32 Reg)
{
  UINT32 conf_data;
  UINT32 hi, lo;
  
  switch(Reg){
  case PCI_VENDOR_ID_OFFSET :
    conf_data = 0;
    _rdmsr(GLPCI_GLD_MSR_CAP, &hi, &lo);  /* jlliu : should get the correct Value. */
    if( (lo != 0x0) && (lo != 0xffffffff) ){
      conf_data = (CS5536_ISA_DEVICE_ID << 16 | CS5536_VENDOR_ID);
    }
    break;
  case PCI_COMMAND_OFFSET :
    conf_data = 0;
    // COMMAND      
    _rdmsr(DIVIL_LBAR_SMB, &hi, &lo);
    if(hi & 0x01){
      conf_data |= EFI_PCI_COMMAND_IO_SPACE;
    }
    conf_data |= EFI_PCI_COMMAND_SPECIAL_CYCLE;
    _rdmsr(GLPCI_GLD_MSR_ERROR, &hi, &lo);
    if(lo & SB_PARE_ERR_EN){
      conf_data |= EFI_PCI_COMMAND_PARITY_ERROR_RESPOND;
    }else{
      conf_data &= ~EFI_PCI_COMMAND_PARITY_ERROR_RESPOND;
    }
    // STATUS  
    conf_data |= EFI_PCI_STATUS_66MHZ_SUPPORT;
    conf_data |= EFI_PCI_STATUS_DEVSEL_MEDIUM;
    conf_data |= EFI_PCI_STATUS_BACKTOBACK_SUPPORT;
    _rdmsr(GLPCI_GLD_MSR_ERROR, &hi, &lo);
    if(lo & SB_SYSE_ERR_FLAG)
      conf_data |= EFI_PCI_STATUS_TARGET_TARGET_ABORT;
    if(lo & SB_TAR_ERR_FLAG)
      conf_data |= EFI_PCI_STATUS_MASTER_TARGET_ABORT;
    if(lo & SB_MAR_ERR_FLAG)
      conf_data |= EFI_PCI_STATUS_MASTER_ABORT;
    if(lo & SB_PARE_ERR_FLAG)
      conf_data |= EFI_PCI_STATUS_PARITY_DETECT;
    break;
  case PCI_REVISION_ID_OFFSET :
    _rdmsr(GLCP_CHIP_REV_ID, &hi, &lo);
    conf_data = lo & 0x000000ff;
    conf_data |= (CS5536_ISA_CLASS_CODE << 8);
    break;
  case PCI_CACHELINE_SIZE_OFFSET :
    _rdmsr(GLPCI_CTRL, &hi, &lo);
    hi &= 0x000000f8;
    conf_data = (PCI_NONE_BIST << 24) | (PCI_BRIDGE_HEADER_TYPE << 16) |
      (hi << 8) | PCI_NORMAL_CACHE_LINE_SIZE;
    break;
  /*
   * we only use the LBAR of DIVIL, no RCONF used. 
   * all of them are IO space.
   */
  case PCI_BAR0_REG :
    _rdmsr(GLCP_DOWSER, &hi, &lo);
    if(lo & SOFT_BAR_SMB_FLAG){
      conf_data = CS5536_SMB_RANGE | PCI_MAPREG_TYPE_IO;
      _rdmsr(GLCP_DOWSER, &hi, &lo);
      lo &= ~SOFT_BAR_SMB_FLAG;
      _wrmsr(GLCP_DOWSER, hi, lo);
    }else{
      _rdmsr(DIVIL_LBAR_SMB, &hi, &lo);
      conf_data = lo & 0x0000ffff;
      conf_data |= 0x01;
      conf_data &= ~0x02;
    }
    break;
  case PCI_BAR1_REG :
    _rdmsr(GLCP_DOWSER, &hi, &lo);
    if(lo & SOFT_BAR_GPIO_FLAG){
      conf_data = CS5536_GPIO_RANGE | PCI_MAPREG_TYPE_IO;
      _rdmsr(GLCP_DOWSER, &hi, &lo);
      lo &= ~SOFT_BAR_GPIO_FLAG;
      _wrmsr(GLCP_DOWSER, hi, lo);
    }else{
      _rdmsr(DIVIL_LBAR_GPIO, &hi, &lo);
      conf_data = lo & 0x0000ffff;
      conf_data |= 0x01;
      conf_data &= ~0x02;
    }
    break;
  case PCI_BAR2_REG :
    _rdmsr(GLCP_DOWSER, &hi, &lo);
    if(lo & SOFT_BAR_MFGPT_FLAG){
      conf_data = CS5536_MFGPT_RANGE | PCI_MAPREG_TYPE_IO;
      _rdmsr(GLCP_DOWSER, &hi, &lo);
      lo &= ~SOFT_BAR_MFGPT_FLAG;
      _wrmsr(GLCP_DOWSER, hi, lo);
    }else{
      _rdmsr(DIVIL_LBAR_MFGPT, &hi, &lo);
      conf_data = lo & 0x0000ffff;
      conf_data |= 0x01;
      conf_data &= ~0x02;
    }
    break;
  case PCI_BAR3_REG :
    _rdmsr(GLCP_DOWSER, &hi, &lo);
    if(lo & SOFT_BAR_IRQ_FLAG){
      conf_data = CS5536_IRQ_RANGE | PCI_MAPREG_TYPE_IO;
      _rdmsr(GLCP_DOWSER, &hi, &lo);
      lo &= ~SOFT_BAR_IRQ_FLAG;
      _wrmsr(GLCP_DOWSER, hi, lo);
    }else{
      _rdmsr(DIVIL_LBAR_IRQ, &hi, &lo);
      conf_data = lo & 0x0000ffff;
      conf_data |= 0x01;
      conf_data &= ~0x02;
    }
    break;  
  case PCI_BAR4_REG :
    _rdmsr(GLCP_DOWSER, &hi, &lo);
    if(lo & SOFT_BAR_PMS_FLAG){
      conf_data = CS5536_PMS_RANGE | PCI_MAPREG_TYPE_IO;
      _rdmsr(GLCP_DOWSER, &hi, &lo);
      lo &= ~SOFT_BAR_PMS_FLAG;
      _wrmsr(GLCP_DOWSER, hi, lo);
    }else{
      _rdmsr(DIVIL_LBAR_PMS, &hi, &lo);
      conf_data = lo & 0x0000ffff;
      conf_data |= 0x01;
      conf_data &= ~0x02;
    }
    break;
  case PCI_BAR5_REG :
    _rdmsr(GLCP_DOWSER, &hi, &lo);
    if(lo & SOFT_BAR_ACPI_FLAG){
      conf_data = CS5536_ACPI_RANGE | PCI_MAPREG_TYPE_IO;
      _rdmsr(GLCP_DOWSER, &hi, &lo);
      lo &= ~SOFT_BAR_ACPI_FLAG;
      _wrmsr(GLCP_DOWSER, hi, lo);
    }else{
      _rdmsr(DIVIL_LBAR_ACPI, &hi, &lo);
      conf_data = lo & 0x0000ffff;
      conf_data |= 0x01;
      conf_data &= ~0x02;
    }
    break;
  case PCI_CARDBUS_CIS_OFFSET :
    conf_data = PCI_CARDBUS_CIS_POINTER;
    break;
  case PCI_SVID_OFFSET :
    conf_data = (CS5536_ISA_SUB_ID << 16) | CS5536_SUB_VENDOR_ID;
    break;
  case PCI_EXPANSION_ROM_BASE :
    conf_data = PCI_EXPANSION_ROM_BAR;
    break;
  case PCI_CAPBILITY_POINTER_OFFSET :
    conf_data = PCI_CAPLIST_POINTER;
    break;
  case PCI_INT_LINE_OFFSET :
    conf_data = (PCI_MAX_LATENCY << 24) | (PCI_MIN_GRANT << 16) | 
      (PCI_INTERRUPT_PIN_NONE << 8) | 0x00;
    break;
  default :
    conf_data = 0;
    break;
  }

  return conf_data;
}

/*
 * ide_write : ide write transfering
 */
static void pci_ide_write_reg(UINT32 Reg, UINT32 Value)
{
  UINT32 hi, lo;
  
  switch(Reg){
  case PCI_COMMAND_OFFSET :
    // COMMAND
    if(Value & EFI_PCI_COMMAND_BUS_MASTER){
      _rdmsr(GLIU_PAE, &hi, &lo);
      lo |= (0x03 << 4);
      _wrmsr(GLIU_PAE, hi, lo);      
    }else{
      _rdmsr(GLIU_PAE, &hi, &lo);
      lo &= ~(0x03 << 4);
      _wrmsr(GLIU_PAE, hi, lo);  
    }
    // STATUS
    if(Value & EFI_PCI_STATUS_PARITY_ERROR){
      _rdmsr(GLPCI_GLD_MSR_ERROR, &hi, &lo);
      if(lo & SB_PARE_ERR_FLAG){
        lo = (lo & 0x0000ffff) | SB_PARE_ERR_FLAG;
        _wrmsr(GLPCI_GLD_MSR_ERROR, hi, lo);
      }        
    }
    break;
  case PCI_CACHELINE_SIZE_OFFSET :
    Value &= 0x0000ff00;
    _rdmsr(GLPCI_CTRL, &hi, &lo);
    hi &= 0xffffff00;
    hi |= (Value >> 8);
    _wrmsr(GLPCI_CTRL, hi, lo);
    break;
  case PCI_BAR4_REG :
    if((Value&PCI_BAR_RANGE_MASK) == PCI_BAR_RANGE_MASK){
      _rdmsr(GLCP_DOWSER, &hi, &lo);
      lo |= SOFT_BAR_IDE_FLAG;
      _wrmsr(GLCP_DOWSER, hi, lo);
    }else if(Value & 0x01){
      hi = 0x00000000;
      lo = (Value & 0xfffffff0) | 0x1;
      _wrmsr(IDE_IO_BAR, hi, lo);
#ifdef SB_RX        
      hi = 0x60000000 | ((Value & 0x000ff000) >> 12);
      lo = 0x000ffff0 | ((Value & 0x00000ffc) << 20);
      _wrmsr(GLIU_IOD_BM2, hi, lo);
#endif        
    }
    break;
  case PCI_IDE_CFG_REG :
    if(Value == CS5536_IDE_FLASH_SIGNATURE){
      _rdmsr(DIVIL_BALL_OPTS, &hi, &lo);
      lo |= 0x01;
      _wrmsr(DIVIL_BALL_OPTS, hi, lo);
    }else{
      hi = 0;
      lo = Value;
      _wrmsr(IDE_CFG, hi, lo);      
    }
    break;
  case PCI_IDE_DTC_REG :
    hi = 0;
    lo = Value;
    _wrmsr(IDE_DTC, hi, lo);
    break;
  case PCI_IDE_CAST_REG :
    hi = 0;
    lo = Value;
    _wrmsr(IDE_CAST, hi, lo);
    break;
  case PCI_IDE_ETC_REG :
    hi = 0;
    lo = Value;
    _wrmsr(IDE_ETC, hi, lo);
    break;
  case PCI_IDE_PM_REG :
    hi = 0;
    lo = Value;
    _wrmsr(IDE_PM, hi, lo);
    break;
  default :
    break;      
  }
  
  return;
}

/*
 * ide_read : ide read tranfering.
 */
static UINT32 pci_ide_read_reg(UINT32 Reg)
{
  UINT32 conf_data;
  UINT32 hi, lo;
  
  switch(Reg){
  case PCI_VENDOR_ID_OFFSET :
    conf_data = 0;
    _rdmsr(IDE_GLD_MSR_CAP, &hi, &lo);
    if( (lo != 0x0) && (lo != 0xffffffff) ){
      conf_data = (CS5536_IDE_DEVICE_ID << 16 | CS5536_VENDOR_ID);
    }
    break;
  case PCI_COMMAND_OFFSET :
    conf_data = 0;
    // COMMAND
    _rdmsr(IDE_IO_BAR, &hi, &lo);
    if(lo & 0xfffffff0)
      conf_data |= EFI_PCI_COMMAND_IO_SPACE;
    _rdmsr(GLIU_PAE, &hi, &lo);
    if( (lo & 0x30) == 0x30 )
      conf_data |= EFI_PCI_COMMAND_BUS_MASTER;
    //STATUS
    conf_data |= EFI_PCI_STATUS_66MHZ_SUPPORT;
    conf_data |= EFI_PCI_STATUS_BACKTOBACK_SUPPORT;
    _rdmsr(GLPCI_GLD_MSR_ERROR, &hi, &lo);
    if(lo & SB_PARE_ERR_FLAG)
      conf_data |= EFI_PCI_STATUS_PARITY_ERROR;
    conf_data |= EFI_PCI_STATUS_DEVSEL_MEDIUM;
    break;
  case PCI_REVISION_ID_OFFSET :
    _rdmsr(IDE_GLD_MSR_CAP, &hi, &lo);
    conf_data = lo & 0x000000ff;
    conf_data |= (CS5536_IDE_CLASS_CODE << 8);
    break;
  case PCI_CACHELINE_SIZE_OFFSET :
    _rdmsr(GLPCI_CTRL, &hi, &lo);
    hi &= 0x000000f8;
    conf_data = (PCI_NONE_BIST << 24) | (PCI_NORMAL_HEADER_TYPE << 16) |
      (hi << 8) | PCI_NORMAL_CACHE_LINE_SIZE;
    break;
  case PCI_BAR0_REG :
    conf_data = 0x00000000;
    break;
  case PCI_BAR1_REG :
    conf_data = 0x00000000;
    break;
  case PCI_BAR2_REG :
    conf_data = 0x00000000;
    break;
  case PCI_BAR3_REG :
    conf_data = 0x00000000;
    break;
  case PCI_BAR4_REG :
    _rdmsr(GLCP_DOWSER, &hi, &lo);
    if(lo & SOFT_BAR_IDE_FLAG){
      conf_data = CS5536_IDE_RANGE | PCI_MAPREG_TYPE_IO;
      _rdmsr(GLCP_DOWSER, &hi, &lo);
      lo &= ~SOFT_BAR_IDE_FLAG;
      _wrmsr(GLCP_DOWSER, hi, lo);
    }else{
      _rdmsr(IDE_IO_BAR, &hi, &lo);
      //conf_data = lo >> 4;
      conf_data = lo & 0xfffffff0;
      conf_data |= 0x01;
      conf_data &= ~0x02;
    }
    break;
  case PCI_BAR5_REG :
    conf_data = 0x00000000;
    break;
  case PCI_CARDBUS_CIS_OFFSET :
    conf_data = PCI_CARDBUS_CIS_POINTER;
    break;
  case PCI_SVID_OFFSET :
    conf_data = (CS5536_IDE_SUB_ID << 16) | CS5536_SUB_VENDOR_ID;
    break;
  case PCI_EXPANSION_ROM_BASE :
    conf_data = PCI_EXPANSION_ROM_BAR;
    break;
  case PCI_CAPBILITY_POINTER_OFFSET :
    conf_data = PCI_CAPLIST_POINTER;
    break;
  case PCI_INT_LINE_OFFSET :
    conf_data = (PCI_MAX_LATENCY << 24) | (PCI_MIN_GRANT << 16) | 
      (PCI_INTERRUPT_PIN_NONE << 8) | 0x00;
    break;
  case PCI_IDE_CFG_REG :
    _rdmsr(IDE_CFG, &hi, &lo);
    conf_data = lo;
    break;
  case PCI_IDE_DTC_REG :
    _rdmsr(IDE_DTC, &hi, &lo);
    conf_data = lo;
    break;
  case PCI_IDE_CAST_REG :
    _rdmsr(IDE_CAST, &hi, &lo);
    conf_data = lo;
    break;
  case PCI_IDE_ETC_REG :
    _rdmsr(IDE_ETC, &hi, &lo);
    conf_data = lo;
  case PCI_IDE_PM_REG :
    _rdmsr(IDE_PM, &hi, &lo);
    conf_data = lo;
    break;
    
  default :
    conf_data = 0;
    break;
  }

  return conf_data;
}

static void pci_acc_write_reg(UINT32 Reg, UINT32 Value)
{
  UINT32 hi, lo;

  switch(Reg){
  case PCI_COMMAND_OFFSET :
    // COMMAND
    if(Value & EFI_PCI_COMMAND_BUS_MASTER){
      _rdmsr(GLIU_PAE, &hi, &lo);
      lo |= (0x03 << 8);
      _wrmsr(GLIU_PAE, hi, lo);      
    }else{
      _rdmsr(GLIU_PAE, &hi, &lo);
      lo &= ~(0x03 << 8);
      _wrmsr(GLIU_PAE, hi, lo);  
    }
    // STATUS
    if(Value & EFI_PCI_STATUS_PARITY_ERROR){
      _rdmsr(GLPCI_GLD_MSR_ERROR, &hi, &lo);
      if(lo & SB_PARE_ERR_FLAG){
        lo = (lo & 0x0000ffff) | SB_PARE_ERR_FLAG;
        _wrmsr(GLPCI_GLD_MSR_ERROR, hi, lo);
      }        
    }
    break;
  case PCI_BAR0_REG :
    if((Value&PCI_BAR_RANGE_MASK) == PCI_BAR_RANGE_MASK){
      _rdmsr(GLCP_DOWSER, &hi, &lo);
      lo |= SOFT_BAR_ACC_FLAG;
      _wrmsr(GLCP_DOWSER, hi, lo);
    }else if( Value & 0x01 ){
      Value &= 0xfffffffc;
      hi = 0xA0000000 | ((Value & 0x000ff000) >> 12);
      lo = 0x000fff80 | ((Value & 0x00000fff) << 20);
      _wrmsr(GLIU_IOD_BM1, hi, lo);
    }
    break;
  default :
    break;      
  }

  return;
}

static UINT32 pci_acc_read_reg(UINT32 Reg)
{
  UINT32 hi, lo;
  UINT32 conf_data;

  switch(Reg){
  case PCI_VENDOR_ID_OFFSET :
    conf_data = (CS5536_ACC_DEVICE_ID << 16 | CS5536_VENDOR_ID);
    break;
  case PCI_COMMAND_OFFSET :
    
    conf_data = 0;
    // COMMAND
    _rdmsr(GLIU_IOD_BM1, &hi, &lo);
    if( ( (lo & 0xfff00000) || (hi & 0x000000ff) ) 
        && ((hi & 0xf0000000) == 0xa0000000) )
      conf_data |= EFI_PCI_COMMAND_IO_SPACE;
    _rdmsr(GLIU_PAE, &hi, &lo);
    if( (lo & 0x300) == 0x300 )
      conf_data |= EFI_PCI_COMMAND_BUS_MASTER;
    /* conf_data |= PCI_COMMAND_BACKTOBACK_ENABLE??? HOW TO GET..*/
    //STATUS
    conf_data |= EFI_PCI_STATUS_66MHZ_SUPPORT;
    conf_data |= EFI_PCI_STATUS_BACKTOBACK_SUPPORT;
    _rdmsr(GLPCI_GLD_MSR_ERROR, &hi, &lo);
    if(lo & SB_PARE_ERR_FLAG)
      conf_data |= EFI_PCI_STATUS_PARITY_ERROR;
    conf_data |= EFI_PCI_STATUS_DEVSEL_MEDIUM;
    break;
  case PCI_REVISION_ID_OFFSET :
    _rdmsr(ACC_GLD_MSR_CAP, &hi, &lo);
    conf_data = lo & 0x000000ff;
    conf_data |= (CS5536_ACC_CLASS_CODE << 8);
    break;
  case PCI_CACHELINE_SIZE_OFFSET :
    conf_data = (PCI_NONE_BIST << 24) | (PCI_NORMAL_HEADER_TYPE << 16) |
      (PCI_NORMAL_LATENCY_TIMER << 8) | PCI_NORMAL_CACHE_LINE_SIZE;
    break;
  case PCI_BAR0_REG :
    _rdmsr(GLCP_DOWSER, &hi, &lo);
    if(lo & SOFT_BAR_ACC_FLAG){
      conf_data = CS5536_ACC_RANGE | PCI_MAPREG_TYPE_IO;
      _rdmsr(GLCP_DOWSER, &hi, &lo);
      lo &= ~SOFT_BAR_ACC_FLAG;
      _wrmsr(GLCP_DOWSER, hi, lo);
    }else{
      _rdmsr(GLIU_IOD_BM1, &hi, &lo);
      conf_data = (hi & 0x000000ff) << 12;
      conf_data |= (lo & 0xfff00000) >> 20; 
      conf_data |= 0x01;
      conf_data &= ~0x02;
    }
    break;
  case PCI_BAR1_REG :
    conf_data = 0x000000;
    break;    
  case PCI_BAR2_REG :
    conf_data = 0x000000;
    break;
  case PCI_BAR3_REG :
    conf_data = 0x000000;
    break;
  case PCI_BAR4_REG :
    conf_data = 0x000000;
    break;
  case PCI_BAR5_REG :
    conf_data = 0x000000;
    break;
  case PCI_CARDBUS_CIS_OFFSET :
    conf_data = PCI_CARDBUS_CIS_POINTER;
    break;
  case PCI_SVID_OFFSET :
    conf_data = (CS5536_ACC_SUB_ID << 16) | CS5536_SUB_VENDOR_ID;
    break;
  case PCI_EXPANSION_ROM_BASE :
    conf_data = PCI_EXPANSION_ROM_BAR;
    break;
  case PCI_CAPBILITY_POINTER_OFFSET :
    conf_data = PCI_CAPLIST_USB_POINTER;
    break;
  case PCI_INT_LINE_OFFSET :
    conf_data = (PCI_MAX_LATENCY << 24) | (PCI_MIN_GRANT << 16) | 
      (0x01 << 8) | (0x00);

    break;
  default :
    conf_data = 0;
    break;
  }

  return conf_data;
}

/*
 * ohci_write : ohci write tranfering.
 */
static void pci_ohci_write_reg(UINT32 Reg, UINT32 Value)
{
  UINT32 hi, lo;
  
  switch(Reg){
  case PCI_COMMAND_OFFSET :
    // COMMAND
    if(Value & EFI_PCI_COMMAND_BUS_MASTER){
      _rdmsr(USB_MSR_OHCB, &hi, &lo);
      hi |= (1 << 2);
      _wrmsr(USB_MSR_OHCB, hi, lo);
    }else{
      _rdmsr(USB_MSR_OHCB, &hi, &lo);
      hi &= ~(1 << 2);
      _wrmsr(USB_MSR_OHCB, hi, lo);
    }
    if(Value & EFI_PCI_COMMAND_MEMORY_SPACE){
      _rdmsr(USB_MSR_OHCB, &hi, &lo);
      hi |= (1 << 1);
      _wrmsr(USB_MSR_OHCB, hi, lo);
    }else{
      _rdmsr(USB_MSR_OHCB, &hi, &lo);
      hi &= ~(1 << 1);
      _wrmsr(USB_MSR_OHCB, hi, lo);        
    }
    // STATUS
    if(Value & EFI_PCI_STATUS_PARITY_ERROR){
      _rdmsr(GLPCI_GLD_MSR_ERROR, &hi, &lo);
      if(lo & SB_PARE_ERR_FLAG){
        lo = (lo & 0x0000ffff) | SB_PARE_ERR_FLAG;
        _wrmsr(GLPCI_GLD_MSR_ERROR, hi, lo);
      }        
    }
    break;
  case PCI_BAR0_REG :
    if((Value&PCI_BAR_RANGE_MASK) == PCI_BAR_RANGE_MASK){
      _rdmsr(GLCP_DOWSER, &hi, &lo);
      lo |= SOFT_BAR_OHCI_FLAG;
      _wrmsr(GLCP_DOWSER, hi, lo);
    }else if( (Value & 0x01) == 0x00 ){
      _rdmsr(USB_MSR_OHCB, &hi, &lo);
      //lo = (Value & 0xffffff00) << 8;
      lo = Value;
      _wrmsr(USB_MSR_OHCB, hi, lo);
      
      hi = 0x40000000 | (Value&0xff000000) >> 24;
      lo = 0x000fffff | (Value&0x00fff000) << 8;
      _wrmsr(GLIU_P2D_BM3, hi, lo);
    }
    break;
  case PCI_INT_LINE_OFFSET :
    Value &= 0x000000ff;
    break;
  case PCI_USB_PM_REG :
    break;
  default :
    break;      
  }
  
  return;
}

/*
 * ohci_read : ohci read transfering.
 */
static UINT32 pci_ohci_read_reg(UINT32 Reg)
{
  UINT32 conf_data;
  UINT32 hi, lo;
  
  switch(Reg){
  case PCI_VENDOR_ID_OFFSET :
    conf_data = 0;
    _rdmsr(USB_GLD_MSR_CAP, &hi, &lo);
    if( (lo != 0x0) && (lo != 0xffffffff) ){
      conf_data = (CS5536_OHCI_DEVICE_ID << 16 | CS5536_VENDOR_ID);
    }
    break;
  case PCI_COMMAND_OFFSET :
    conf_data = 0;
    // COMMAND
    _rdmsr(USB_MSR_OHCB, &hi, &lo);
    if(hi & 0x04)
      conf_data |= EFI_PCI_COMMAND_BUS_MASTER;
    if(hi & 0x02)
      conf_data |= EFI_PCI_COMMAND_MEMORY_SPACE;
    // STATUS
    conf_data |= EFI_PCI_STATUS_66MHZ_SUPPORT;
    conf_data |= EFI_PCI_STATUS_BACKTOBACK_SUPPORT;
    _rdmsr(GLPCI_GLD_MSR_ERROR, &hi, &lo);
    if(lo & SB_PARE_ERR_FLAG)
      conf_data |= EFI_PCI_STATUS_PARITY_ERROR;
    conf_data |= EFI_PCI_STATUS_DEVSEL_MEDIUM;
    break;
  case PCI_REVISION_ID_OFFSET :
    _rdmsr(USB_GLD_MSR_CAP, &hi, &lo);
    conf_data = lo & 0x000000ff;
    conf_data |= (CS5536_OHCI_CLASS_CODE << 8);
    break;
  case PCI_CACHELINE_SIZE_OFFSET :
    conf_data = (PCI_NONE_BIST << 24) | (PCI_NORMAL_HEADER_TYPE << 16) |
      (0x00 << 8) | PCI_NORMAL_CACHE_LINE_SIZE;
    break;
  case PCI_BAR0_REG :
    _rdmsr(GLCP_DOWSER, &hi, &lo);
    if(lo & SOFT_BAR_OHCI_FLAG){
      conf_data = CS5536_OHCI_RANGE | PCI_MAPREG_TYPE_MEM;
      _rdmsr(GLCP_DOWSER, &hi, &lo);
      lo &= ~SOFT_BAR_OHCI_FLAG;
      _wrmsr(GLCP_DOWSER, hi, lo);
    }else{
      _rdmsr(USB_MSR_OHCB, &hi, &lo);
      //conf_data = lo >> 8;
      conf_data = lo & 0xffffff00;
      conf_data &= ~0x0000000f; // 32bit mem
    }
    break;
  case PCI_BAR1_REG :
    conf_data = 0x000000;
    break;    
  case PCI_BAR2_REG :
    conf_data = 0x000000;
    break;
  case PCI_BAR3_REG :
    conf_data = 0x000000;
    break;
  case PCI_BAR4_REG :
    conf_data = 0x000000;
    break;
  case PCI_BAR5_REG :
    conf_data = 0x000000;
    break;
  case PCI_CARDBUS_CIS_OFFSET :
    conf_data = PCI_CARDBUS_CIS_POINTER;
    break;
  case PCI_SVID_OFFSET :
    conf_data = (CS5536_OHCI_SUB_ID << 16) | CS5536_SUB_VENDOR_ID;
    break;
  case PCI_EXPANSION_ROM_BASE :
    conf_data = PCI_EXPANSION_ROM_BAR;
    break;
  case PCI_CAPBILITY_POINTER_OFFSET :
    conf_data = PCI_CAPLIST_USB_POINTER;
    break;
  case PCI_INT_LINE_OFFSET :
    conf_data = (PCI_MAX_LATENCY << 24) | (PCI_MIN_GRANT << 16) | 
      (PCI_INTERRUPT_PIN_D << 8) | 0x00;
    break;
  case PCI_USB_PM_REG :
    conf_data = 0;
    break;
  case 0x50 :
    _rdmsr(GLIU_P2D_BM3, &hi, &lo);
    _rdmsr(USB_MSR_OHCB, &hi, &lo);
    conf_data = 0;
    break;    
  default :
    conf_data = 0;
    break;
  }

  return conf_data;
}

static void pci_ehci_write_reg(UINT32 Reg, UINT32 Value)
{
  UINT32 hi, lo;
  
  switch(Reg){
  case PCI_COMMAND_OFFSET :
    // COMMAND
    if(Value & EFI_PCI_COMMAND_BUS_MASTER){
      _rdmsr(USB_MSR_EHCB, &hi, &lo);
      hi |= (1 << 2);
      _wrmsr(USB_MSR_EHCB, hi, lo);
    }else{
      _rdmsr(USB_MSR_EHCB, &hi, &lo);
      hi &= ~(1 << 2);
      _wrmsr(USB_MSR_EHCB, hi, lo);
    }
    if(Value & EFI_PCI_COMMAND_MEMORY_SPACE){
      _rdmsr(USB_MSR_EHCB, &hi, &lo);
      hi |= (1 << 1);
      _wrmsr(USB_MSR_EHCB, hi, lo);
    }else{
      _rdmsr(USB_MSR_EHCB, &hi, &lo);
      hi &= ~(1 << 1);
      _wrmsr(USB_MSR_EHCB, hi, lo);        
    }
    // STATUS
    if(Value & EFI_PCI_STATUS_PARITY_ERROR){
      _rdmsr(GLPCI_GLD_MSR_ERROR, &hi, &lo);
      if(lo & SB_PARE_ERR_FLAG){
        lo = (lo & 0x0000ffff) | SB_PARE_ERR_FLAG;
        _wrmsr(GLPCI_GLD_MSR_ERROR, hi, lo);
      }        
    }
    break;
  case PCI_BAR0_REG :
    if((Value&PCI_BAR_RANGE_MASK) == PCI_BAR_RANGE_MASK){
      _rdmsr(GLCP_DOWSER, &hi, &lo);
      lo |= SOFT_BAR_EHCI_FLAG;
      _wrmsr(GLCP_DOWSER, hi, lo);
    }else if( (Value & 0x01) == 0x00 ){
      _rdmsr(USB_MSR_EHCB, &hi, &lo);
      lo = Value;
      _wrmsr(USB_MSR_EHCB, hi, lo);
      
      Value &= 0xfffffff0;
      hi = 0x40000000 | ((Value & 0xff000000) >> 24);
      lo = 0x000fffff | ((Value & 0x00fff000) << 8);
      _wrmsr(GLIU_P2D_BM4, hi, lo);
    }
    break;
  case PCI_EHCI_LEGSMIEN_REG :
    _rdmsr(USB_MSR_EHCB, &hi, &lo);
    hi &= 0x003f0000;
    hi |= (Value & 0x3f) << 16;
    _wrmsr(USB_MSR_EHCB, hi, lo);
    break;
  case PCI_EHCI_FLADJ_REG :
    _rdmsr(USB_MSR_EHCB, &hi, &lo);
    hi &= ~0x00003f00;
    hi |= Value & 0x00003f00;
    _wrmsr(USB_MSR_EHCB, hi, lo);
    break;
  default :
    break;      
  }
  
  return;
}

static UINT32 pci_ehci_read_reg(UINT32 Reg)
{
  UINT32 conf_data;
  UINT32 hi, lo;
  
  switch(Reg){
  case PCI_VENDOR_ID_OFFSET :
    conf_data = (CS5536_EHCI_DEVICE_ID << 16 | CS5536_VENDOR_ID);
    break;
  case PCI_COMMAND_OFFSET :
    conf_data = 0;
    // COMMAND
    _rdmsr(USB_MSR_EHCB, &hi, &lo);
    if(hi & 0x04)
      conf_data |= EFI_PCI_COMMAND_BUS_MASTER;
    if(hi & 0x02)
      conf_data |= EFI_PCI_COMMAND_MEMORY_SPACE;
    // STATUS
    conf_data |= EFI_PCI_STATUS_66MHZ_SUPPORT;
    conf_data |= EFI_PCI_STATUS_BACKTOBACK_SUPPORT;
    _rdmsr(GLPCI_GLD_MSR_ERROR, &hi, &lo);
    if(lo & SB_PARE_ERR_FLAG)
      conf_data |= EFI_PCI_STATUS_PARITY_ERROR;
    conf_data |= EFI_PCI_STATUS_DEVSEL_MEDIUM;
    break;
  case PCI_REVISION_ID_OFFSET :
    _rdmsr(USB_GLD_MSR_CAP, &hi, &lo);
    conf_data = lo & 0x000000ff;
    conf_data |= (CS5536_EHCI_CLASS_CODE << 8);
    break;
  case PCI_CACHELINE_SIZE_OFFSET :
    conf_data = (PCI_NONE_BIST << 24) | (PCI_NORMAL_HEADER_TYPE << 16) |
      (PCI_NORMAL_LATENCY_TIMER << 8) | PCI_NORMAL_CACHE_LINE_SIZE;
    break;
  case PCI_BAR0_REG :
    _rdmsr(GLCP_DOWSER, &hi, &lo);
    if(lo & SOFT_BAR_EHCI_FLAG){
      conf_data = CS5536_EHCI_RANGE | PCI_MAPREG_TYPE_MEM;
      _rdmsr(GLCP_DOWSER, &hi, &lo);
      lo &= ~SOFT_BAR_EHCI_FLAG;
      _wrmsr(GLCP_DOWSER, hi, lo);
    }else{
      _rdmsr(USB_MSR_EHCB, &hi, &lo);
      conf_data = lo & 0xffffff00;
      conf_data &= ~0x0000000f; // 32bit mem
    }
    break;
  case PCI_BAR1_REG :
    conf_data = 0x000000;
    break;    
  case PCI_BAR2_REG :
    conf_data = 0x000000;
    break;
  case PCI_BAR3_REG :
    conf_data = 0x000000;
    break;
  case PCI_BAR4_REG :
    conf_data = 0x000000;
    break;
  case PCI_BAR5_REG :
    conf_data = 0x000000;
    break;
  case PCI_CARDBUS_CIS_OFFSET :
    conf_data = PCI_CARDBUS_CIS_POINTER;
    break;
  case PCI_SVID_OFFSET :
    conf_data = (CS5536_EHCI_SUB_ID << 16) | CS5536_SUB_VENDOR_ID;
    break;
  case PCI_EXPANSION_ROM_BASE :
    conf_data = PCI_EXPANSION_ROM_BAR;
    break;
  case PCI_CAPBILITY_POINTER_OFFSET :
    conf_data = PCI_CAPLIST_USB_POINTER;
    break;
  case PCI_INT_LINE_OFFSET :
    conf_data = (PCI_MAX_LATENCY << 24) | (PCI_MIN_GRANT << 16) | 
      (0x01 << 8) | 0x00;
    break;
  case PCI_EHCI_LEGSMIEN_REG :
    _rdmsr(USB_MSR_EHCB, &hi, &lo);
    conf_data = (hi & 0x003f0000) >> 16;
    break;
  case PCI_EHCI_LEGSMISTS_REG :
    _rdmsr(USB_MSR_EHCB, &hi, &lo);
    conf_data = (hi & 0x3f000000) >> 24;
    break;
  case PCI_EHCI_FLADJ_REG :
    _rdmsr(USB_MSR_EHCB, &hi, &lo);
    conf_data = hi & 0x00003f00;
    break;
  default :
    conf_data = 0;
    break;
  }

  return conf_data;
}


static void pci_udc_write_reg(UINT32 Reg, UINT32 Value)
{
  UINT32 hi, lo;
  
  switch(Reg){
  case PCI_COMMAND_OFFSET :
    // COMMAND
    if(Value & EFI_PCI_COMMAND_BUS_MASTER){
      _rdmsr(USB_MSR_UDCB, &hi, &lo);
      hi |= (1 << 2);
      _wrmsr(USB_MSR_UDCB, hi, lo);
    }else{
      _rdmsr(USB_MSR_UDCB, &hi, &lo);
      hi &= ~(1 << 2);
      _wrmsr(USB_MSR_UDCB, hi, lo);
    }
    if(Value & EFI_PCI_COMMAND_MEMORY_SPACE){
      _rdmsr(USB_MSR_UDCB, &hi, &lo);
      hi |= (1 << 1);
      _wrmsr(USB_MSR_UDCB, hi, lo);
    }else{
      _rdmsr(USB_MSR_UDCB, &hi, &lo);
      hi &= ~(1 << 1);
      _wrmsr(USB_MSR_UDCB, hi, lo);        
    }
    // STATUS
    if(Value & EFI_PCI_STATUS_PARITY_ERROR){
      _rdmsr(GLPCI_GLD_MSR_ERROR, &hi, &lo);
      if(lo & SB_PARE_ERR_FLAG){
        lo = (lo & 0x0000ffff) | SB_PARE_ERR_FLAG;
        _wrmsr(GLPCI_GLD_MSR_ERROR, hi, lo);
      }        
    }
    break;
  case PCI_BAR0_REG :
    if((Value&PCI_BAR_RANGE_MASK) == PCI_BAR_RANGE_MASK){
      _rdmsr(GLCP_DOWSER, &hi, &lo);
      lo |= SOFT_BAR_UDC_FLAG;
      _wrmsr(GLCP_DOWSER, hi, lo);
    }else if( (Value & 0x01) == 0x00 ){
      _rdmsr(USB_MSR_UDCB, &hi, &lo);
      lo = Value;
      _wrmsr(USB_MSR_UDCB, hi, lo);
      
      Value &= 0xfffffff0;
      hi = 0x40000000 | ((Value & 0xff000000) >> 24);
      lo = 0x000fffff | ((Value & 0x00fff000) << 8);
      _wrmsr(GLIU_P2D_BM0, hi, lo);
    }
    break;
  default :
    break;      
  }
  
  return;
}

static UINT32 pci_udc_read_reg(UINT32 Reg)
{
  UINT32 conf_data;
  UINT32 hi, lo;
  
  switch(Reg){
  case PCI_VENDOR_ID_OFFSET :
    conf_data = (CS5536_UDC_DEVICE_ID << 16 | CS5536_VENDOR_ID);
    break;
  case PCI_COMMAND_OFFSET :
    conf_data = 0;
    // COMMAND
    _rdmsr(USB_MSR_UDCB, &hi, &lo);
    if(hi & 0x04)
      conf_data |= EFI_PCI_COMMAND_BUS_MASTER;
    if(hi & 0x02)
      conf_data |= EFI_PCI_COMMAND_MEMORY_SPACE;
    // STATUS
    conf_data |= EFI_PCI_STATUS_66MHZ_SUPPORT;
    conf_data |= EFI_PCI_STATUS_BACKTOBACK_SUPPORT;
    _rdmsr(GLPCI_GLD_MSR_ERROR, &hi, &lo);
    if(lo & SB_PARE_ERR_FLAG)
      conf_data |= EFI_PCI_STATUS_PARITY_ERROR;
    conf_data |= EFI_PCI_STATUS_DEVSEL_MEDIUM;
    break;
  case PCI_REVISION_ID_OFFSET :
    _rdmsr(USB_GLD_MSR_CAP, &hi, &lo);
    conf_data = lo & 0x000000ff;
    conf_data |= (CS5536_UDC_CLASS_CODE << 8);
    break;
  case PCI_CACHELINE_SIZE_OFFSET :
    conf_data = (PCI_NONE_BIST << 24) | (PCI_NORMAL_HEADER_TYPE << 16) |
      (0x00 << 8) | PCI_NORMAL_CACHE_LINE_SIZE;
    break;
  case PCI_BAR0_REG :
    _rdmsr(GLCP_DOWSER, &hi, &lo);
    if(lo & SOFT_BAR_UDC_FLAG){
      conf_data = CS5536_UDC_RANGE | PCI_MAPREG_TYPE_MEM;
      _rdmsr(GLCP_DOWSER, &hi, &lo);
      lo &= ~SOFT_BAR_UDC_FLAG;
      _wrmsr(GLCP_DOWSER, hi, lo);
    }else{
      _rdmsr(USB_MSR_UDCB, &hi, &lo);
      conf_data = lo & 0xfffff000;
      conf_data &= ~0x0000000f; // 32bit mem
    }
    break;
  case PCI_BAR1_REG :
    conf_data = 0x000000;
    break;    
  case PCI_BAR2_REG :
    conf_data = 0x000000;
    break;
  case PCI_BAR3_REG :
    conf_data = 0x000000;
    break;
  case PCI_BAR4_REG :
    conf_data = 0x000000;
    break;
  case PCI_BAR5_REG :
    conf_data = 0x000000;
    break;
  case PCI_CARDBUS_CIS_OFFSET :
    conf_data = PCI_CARDBUS_CIS_POINTER;
    break;
  case PCI_SVID_OFFSET :
    conf_data = (CS5536_UDC_SUB_ID << 16) | CS5536_SUB_VENDOR_ID;
    break;
  case PCI_EXPANSION_ROM_BASE :
    conf_data = PCI_EXPANSION_ROM_BAR;
    break;
  case PCI_CAPBILITY_POINTER_OFFSET :
    conf_data = PCI_CAPLIST_USB_POINTER;
    break;
  case PCI_INT_LINE_OFFSET :
    conf_data = (PCI_MAX_LATENCY << 24) | (PCI_MIN_GRANT << 16) | 
      (0x01 << 8) | 0x00;
    break;
  default :
    conf_data = 0;
    break;
  }

  return conf_data;
}

static void pci_otg_write_reg(UINT32 Reg, UINT32 Value)
{
  UINT32 hi, lo;
  
  switch(Reg){
  case PCI_COMMAND_OFFSET :
    // COMMAND
    if(Value & EFI_PCI_COMMAND_MEMORY_SPACE){
      _rdmsr(USB_MSR_UOCB, &hi, &lo);
      hi |= (1 << 1);
      _wrmsr(USB_MSR_UOCB, hi, lo);
    }else{
      _rdmsr(USB_MSR_UOCB, &hi, &lo);
      hi &= ~(1 << 1);
      _wrmsr(USB_MSR_UOCB, hi, lo);        
    }
    // STATUS
    if(Value & EFI_PCI_STATUS_PARITY_ERROR){
      _rdmsr(GLPCI_GLD_MSR_ERROR, &hi, &lo);
      if(lo & SB_PARE_ERR_FLAG){
        lo = (lo & 0x0000ffff) | SB_PARE_ERR_FLAG;
        _wrmsr(GLPCI_GLD_MSR_ERROR, hi, lo);
      }        
    }
    break;
  case PCI_BAR0_REG :
    if((Value&PCI_BAR_RANGE_MASK) == PCI_BAR_RANGE_MASK){
      _rdmsr(GLCP_DOWSER, &hi, &lo);
      lo |= SOFT_BAR_OTG_FLAG;
      _wrmsr(GLCP_DOWSER, hi, lo);
    }else if( (Value & 0x01) == 0x00 ){
      _rdmsr(USB_MSR_UOCB, &hi, &lo);
      lo = Value & 0xffffff00;
      _wrmsr(USB_MSR_UOCB, hi, lo);
      
      Value &= 0xfffffff0;
      hi = 0x40000000 | ((Value & 0xff000000) >> 24);
      lo = 0x000fffff | ((Value & 0x00fff000) << 8);
      _wrmsr(GLIU_P2D_BM1, hi, lo);
    }
    break;
  default :
    break;      
  }
  
  return;
}

static UINT32 pci_otg_read_reg(UINT32 Reg)
{
  UINT32 conf_data;
  UINT32 hi, lo;
  
  switch(Reg){
  case PCI_VENDOR_ID_OFFSET :
    conf_data = (CS5536_OTG_DEVICE_ID << 16 | CS5536_VENDOR_ID);
    break;
  case PCI_COMMAND_OFFSET :
    conf_data = 0;
    // COMMAND
    _rdmsr(USB_MSR_UOCB, &hi, &lo);
    if(hi & 0x02)
      conf_data |= EFI_PCI_COMMAND_MEMORY_SPACE;
    // STATUS
    conf_data |= EFI_PCI_STATUS_66MHZ_SUPPORT;
    conf_data |= EFI_PCI_STATUS_BACKTOBACK_SUPPORT;
    _rdmsr(GLPCI_GLD_MSR_ERROR, &hi, &lo);
    if(lo & SB_PARE_ERR_FLAG)
      conf_data |= EFI_PCI_STATUS_PARITY_ERROR;
    conf_data |= EFI_PCI_STATUS_DEVSEL_MEDIUM;
    break;
  case PCI_REVISION_ID_OFFSET :
    _rdmsr(USB_GLD_MSR_CAP, &hi, &lo);
    conf_data = lo & 0x000000ff;
    conf_data |= (CS5536_OTG_CLASS_CODE << 8);
    break;
  case PCI_CACHELINE_SIZE_OFFSET :
    conf_data = (PCI_NONE_BIST << 24) | (PCI_NORMAL_HEADER_TYPE << 16) |
      (0x00 << 8) | PCI_NORMAL_CACHE_LINE_SIZE;
    break;
  case PCI_BAR0_REG :
    _rdmsr(GLCP_DOWSER, &hi, &lo);
    if(lo & SOFT_BAR_OTG_FLAG){
      conf_data = CS5536_OTG_RANGE | PCI_MAPREG_TYPE_MEM;
      _rdmsr(GLCP_DOWSER, &hi, &lo);
      lo &= ~SOFT_BAR_OTG_FLAG;
      _wrmsr(GLCP_DOWSER, hi, lo);
    }else{
      _rdmsr(USB_MSR_UOCB, &hi, &lo);
      conf_data = lo & 0xffffff00;
      conf_data &= ~0x0000000f;
    }
    break;
  case PCI_BAR1_REG :
    conf_data = 0x000000;
    break;    
  case PCI_BAR2_REG :
    conf_data = 0x000000;
    break;
  case PCI_BAR3_REG :
    conf_data = 0x000000;
    break;
  case PCI_BAR4_REG :
    conf_data = 0x000000;
    break;
  case PCI_BAR5_REG :
    conf_data = 0x000000;
    break;
  case PCI_CARDBUS_CIS_OFFSET :
    conf_data = PCI_CARDBUS_CIS_POINTER;
    break;
  case PCI_SVID_OFFSET :
    conf_data = (CS5536_OTG_SUB_ID << 16) | CS5536_SUB_VENDOR_ID;
    break;
  case PCI_EXPANSION_ROM_BASE :
    conf_data = PCI_EXPANSION_ROM_BAR;
    break;
  case PCI_CAPBILITY_POINTER_OFFSET :
    conf_data = PCI_CAPLIST_USB_POINTER;
    break;
  case PCI_INT_LINE_OFFSET :
    conf_data = (PCI_MAX_LATENCY << 24) | (PCI_MIN_GRANT << 16) | 
      (0x01 << 8) | 0x00;
    break;
  default :
    conf_data = 0;
    break;
  }

  return conf_data;
}


EFI_STATUS
Cs5536PciWrite (
  IN UINT64      Address, 
  IN UINT32       Data
)
{
  UINT32 Bus, Device, Function, Reg;

  PciBreakAddress (Address, &Bus, &Device, &Function, &Reg);

  if (Bus != 0 || Device != PCI_IDSEL_CS5536) {
    return EFI_UNSUPPORTED;
  }  

  if (Reg >= PCI_MSR_CTRL) {
    return EFI_UNSUPPORTED;
  }

  Reg = Reg & 0xfc;
  
  switch (Function) {
  case CS5536_ISA_FUNC:
    pci_isa_write_reg(Reg, Data);    
    break;

  case CS5536_FLASH_FUNC:
    pci_flash_write_reg(Reg, Data);
    break;
  
  case CS5536_IDE_FUNC:
    pci_ide_write_reg(Reg, Data);
    break;

  case CS5536_ACC_FUNC:
    pci_acc_write_reg(Reg, Data);
    break;

  case CS5536_OHCI_FUNC:
    pci_ohci_write_reg(Reg, Data);
    break;

  case CS5536_EHCI_FUNC:
    pci_ehci_write_reg(Reg, Data);
    break;

  case CS5536_UDC_FUNC:
    //pci_udc_write_reg(Reg, Data);
    break;

  case CS5536_OTG_FUNC:
    //pci_otg_write_reg(Reg, Data);
    break;
  
  default :
    break;  
  }
  
  return EFI_SUCCESS;
}



EFI_STATUS
Cs5536PciRead (
  IN UINT64      Address, 
  IN OUT UINT32  *Data
)
{
  UINT32 TmpData;
  UINT32 Bus, Device, Function, Reg;

  PciBreakAddress (Address, &Bus, &Device, &Function, &Reg);

  if (Bus != 0 || Device != PCI_IDSEL_CS5536) {
    return EFI_UNSUPPORTED;
  }

  if (Reg >= PCI_MSR_CTRL) {
    return EFI_UNSUPPORTED;
  }

  *Data = (UINT32) -1;

  Reg = Reg & 0xfc;
  
  switch (Function) {
  case CS5536_ISA_FUNC:
    *Data = pci_isa_read_reg(Reg);    
    break;

  case CS5536_FLASH_FUNC:
    *Data = pci_flash_read_reg(Reg);
    break;
  
  case CS5536_IDE_FUNC:
    *Data = pci_ide_read_reg(Reg);
    break;

  case CS5536_ACC_FUNC:
    *Data = pci_acc_read_reg(Reg);
    break;

  case CS5536_OHCI_FUNC:
    *Data = pci_ohci_read_reg(Reg);
    break;

  case CS5536_EHCI_FUNC:
    *Data = pci_ehci_read_reg(Reg);
    break;

  case CS5536_UDC_FUNC:
    //*Data = pci_udc_read_reg(Reg);
    break;

  case CS5536_OTG_FUNC:
    //*Data = pci_otg_read_reg(Reg);
    break;
  
  default :
    break;
  }
   
  return EFI_SUCCESS;
}

