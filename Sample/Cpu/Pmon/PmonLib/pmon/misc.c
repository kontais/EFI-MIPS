typedef unsigned int U32;
typedef unsigned short U16;
typedef unsigned char  U8;
extern int printf (const char *fmt, ...);
extern void IoWrite8(U16 port, U8 data);
extern U8 IoRead8(U16 port);
extern U32 PciRead32(U32 Address);
extern U32 PciWrite32(U32 addr, U32 data);
extern U8 SMBusRead8(U8 SlaveAddr, U8 Reg);
extern U32 GetL1ICacheSize(void);
extern U32 GetL1DCacheSize(void);
extern U32 GetL1ICacheLineSize(void);
extern U32 GetL1DCacheLineSize(void);

#define  PCI_CFG_ADDR(idsel, func, reg)   ( (1 << (11 + (idsel))) + ((func) << 8) + (reg) )


void testReboot(void)
{
  IoWrite8(0x0381, 0xf4);
  IoWrite8(0x0382, 0xec);
  IoWrite8(0x0383, 0x01);
}

U32 readmsr(U32 addr, U32 *Hi, U32 *Lo)
{
  PciWrite32(PCI_CFG_ADDR(14,0,0xf4), addr);
  *Lo = PciRead32(PCI_CFG_ADDR(14,0,0xf8));
  *Hi = PciRead32(PCI_CFG_ADDR(14,0,0xfc));
  return 0;
}


unsigned int misc(void)
{
  U32 Hi,Lo;
  int i;

  printf("misc test\n");
  //testReboot();
  IoWrite8(0x70, 0);
  printf("RTC REG 0 = %d\n",IoRead8(0x71));
  printf("CS5536 SB reg0 = 0x%x\n", PciRead32(PCI_CFG_ADDR(14,0,0)));
  readmsr(0x0, &Hi,&Lo);
  printf("MSR SB reg0 = 0x%x: 0x%x\n", Hi, Lo);
  readmsr(0x40000000, &Hi,&Lo);
  printf("MSR USB reg0 = 0x%x: 0x%x\n", Hi, Lo);

  readmsr(0x8000000b, &Hi,&Lo);
  printf("SMBUS BASE = 0x%x: 0x%x\n", Hi, Lo);

  readmsr(0x8000000c, &Hi,&Lo);
  printf("GPIO BASE = 0x%x: 0x%x\n", Hi, Lo);

  readmsr(0x8000000d, &Hi,&Lo);
  printf("MFGPT BASE = 0x%x: 0x%x\n", Hi, Lo);

  readmsr(0x8000000e, &Hi,&Lo);
  printf("ACPI BASE = 0x%x: 0x%x\n", Hi, Lo);

  readmsr(0x8000000f, &Hi,&Lo);
  printf("PMS BASE = 0x%x: 0x%x\n", Hi, Lo);

  printf("SPD 0xa1\n");
  for (i=0; i < 10; i++)
  {
    printf("SMBus 0xa1 Reg %d 0x%2x\n", i, SMBusRead8(0xa1, i));
  }
  printf("L1 I Cache Size = %d\n", GetL1ICacheSize());
  printf("L1 D Cache Size = %d\n", GetL1DCacheSize());
  printf("L1 I Cache Line Size = %d\n", GetL1ICacheLineSize());
  printf("L1 D Cache Line Size = %d\n", GetL1DCacheLineSize());


  return 0;
}


