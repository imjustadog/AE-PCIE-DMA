// (c) Copyright 2009  2009 Xilinx, Inc. All rights reserved.
//
// This file contains confidential and proprietary information
// of Xilinx, Inc. and is protected under U.S. and
// international copyright and other intellectual property
// laws.
//
// DISCLAIMER
// This disclaimer is not a license and does not grant any
// rights to the materials distributed herewith. Except as
// otherwise provided in a valid license issued to you by
// Xilinx, and to the maximum extent permitted by applicable
// law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND
// WITH ALL FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES
// AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
// BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-
// INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and
// (2) Xilinx shall not be liable (whether in contract or tort,
// including negligence, or under any other theory of
// liability) for any loss or damage of any kind or nature
// related to, arising under or in connection with these
// materials, including for any direct, or any indirect,
// special, incidental, or consequential loss or damage
// (including loss of data, profits, goodwill, or any type of
// loss or damage suffered as a result of any action brought
// by a third party) even if such damage or loss was
// reasonably foreseeable or Xilinx had been advised of the
// possibility of the same.
//
// CRITICAL APPLICATIONS
// Xilinx products are not designed or intended to be fail-
// safe, or for use in any application requiring fail-safe
// performance, such as life-support or safety devices or
// systems, Class III medical devices, nuclear facilities,
// applications related to the deployment of airbags, or any
// other applications that could lead to death, personal
// injury, or severe property or environmental damage
// (individually and collectively, "Critical
// Applications"). Customer assumes the sole risk and
// liability of any use of Xilinx products in Critical
// Applications, subject only to applicable laws and
// regulations governing limitations on product liability.
//
// THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS
// PART OF THIS FILE AT ALL TIMES.


//--------------------------------------------------------------------------------
//-- Filename: xbmd.c
//--
//-- Description: XBMD device driver. 
//--              
//-- XBMD is an example Red Hat device driver which exercises XBMD design
//-- Device driver has been tested on Red Hat Fedora FC9 2.6.15.
//--------------------------------------------------------------------------------

#include <linux/init.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
//#include <linux/pci-aspm.h>
//#include <linux/pci_regs.h>

#include <asm/uaccess.h>   /* copy_to_user */

#include "xbmd.h"

// semaphores
enum  {
        SEM_READ,
        SEM_WRITE,
        SEM_WRITEREG,
        SEM_READREG,
        SEM_WAITFOR,
        SEM_DMA,
        NUM_SEMS
      };

//semaphores
struct semaphore gSem[NUM_SEMS];

MODULE_LICENSE("Dual BSD/GPL");

// Defines the Vendor ID.  Must be changed if core generated did not set the Vendor ID to the same value
#define PCI_VENDOR_ID_XILINX      0x10ee

// Defines the Device ID.  Must be changed if core generated did not set the Device ID to the same value
#define PCI_DEVICE_ID_XILINX_PCIE 0x0007

// Defining
#define XBMD_REGISTER_SIZE        (4*8)    // There are eight registers, and each is 4 bytes wide.
#define HAVE_REGION               0x01     // I/O Memory region
#define HAVE_IRQ                  0x02     // Interupt

//Status Flags: 
//       1 = Resouce successfully acquired
//       0 = Resource not acquired.      

#define HAVE_REGION 0x01                    // I/O Memory region
#define HAVE_IRQ    0x02                    // Interupt
#define HAVE_KREG   0x04                    // Kernel registration

int             gDrvrMajor = 241;           // Major number not dynamic.
unsigned int    gStatFlags = 0x00;          // Status flags used for cleanup.
unsigned long   gBaseHdwr;                  // Base register address (Hardware address)
unsigned long   gBaseLen;                   // Base register address Length
void           *gBaseVirt = NULL;           // Base register address (Virtual address, for I/O).
char            gDrvrName[]= "xbmd";        // Name of driver in proc.
struct pci_dev *gDev = NULL;                // PCI device structure.
int             gIrq;                       // IRQ assigned by PCI system.
char           *gBufferUnaligned = NULL;    // Pointer to Unaligned DMA buffer.
char           *gReadBuffer      = NULL;    // Pointer to dword aligned DMA buffer.
char           *gWriteBuffer     = NULL;    // Pointer to dword aligned DMA buffer.
dma_addr_t      gReadHWAddr;
dma_addr_t      gWriteHWAddr;
unsigned long SA_SHIRQ = 0;
unsigned long SA_SAMPLE_RANDOM = 0;

int pos;

// Struct Used for Writing CFG Register.  Holds value and register to be written
typedef struct cfgwrite {
	int reg;
	int value;
} cfgwr;

// Struct Used for Writing BMD Register.  Holds value and register to be written
typedef struct bmdwrite {
	int reg;
	int value;
} bmdwr;
//-----------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------

void  XPCIe_IRQHandler (int irq, void *dev_id, struct pt_regs *regs);
u32   XPCIe_ReadReg (u32 dw_offset);
void  XPCIe_WriteReg (u32 dw_offset, u32 val);
void  XPCIe_InitCard (void);
void  XPCIe_InitiatorReset (void);
u32 XPCIe_ReadCfgReg (u32 byte);
u32 XPCIe_WriteCfgReg (u32 byte, u32 value);

//---------------------------------------------------------------------------
// Name:        XPCIe_Open
//
// Description: Book keeping routine invoked each time the device is opened.             
//
// Arguments: inode :
//            filp  :
//
// Returns: 0 on success, error code on failure.
//
// Modification log:
// Date      Who  Description
//
//---------------------------------------------------------------------------
int XPCIe_Open(struct inode *inode, struct file *filp)
{
  printk(KERN_INFO"%s: Open: module opened\n",gDrvrName);
  return SUCCESS;
}

//---------------------------------------------------------------------------
// Name:        XPCIe_Release
//
// Description: Book keeping routine invoked each time the device is closed.             
//
// Arguments: inode :
//            filp  :
//
// Returns: 0 on success, error code on failure.
//
// Modification log:
// Date      Who  Description
//
//---------------------------------------------------------------------------
int XPCIe_Release(struct inode *inode, struct file *filp)
{
  printk(KERN_INFO"%s: Release: module released\n",gDrvrName);
  return(SUCCESS);
}

//---------------------------------------------------------------------------
// Name:        XPCIe_Write
//
// Description: This routine is invoked from user space to write data to
//              the PCIe device. 
//
// Arguments: filp  : file pointer to opened device.
//            buf   : pointer to location in users space, where data is to
//                    be acquired.
//            count : Amount of data in bytes user wishes to send.
//
// Returns: SUCCESS  = Success
//          CRIT_ERR = Critical failure
//
// Modification log:
// Date      Who  Description
//
//---------------------------------------------------------------------------
ssize_t XPCIe_Write(struct file *filp, const char *buf, size_t count,
                       loff_t *f_pos)
{
  int ret = SUCCESS;
  memcpy((char *)gWriteBuffer, buf, count);
  printk(KERN_INFO"%s: XPCIe_Write: %d bytes have been written...\n", gDrvrName, count);
  memcpy((char *)gReadBuffer, buf, count);
  printk(KERN_INFO"%s: XPCIe_Write: %d bytes have been written...\n", gDrvrName, count);
  return (ret);
}

//---------------------------------------------------------------------------
// Name:        XPCIe_Read
//
// Description: This routine is invoked from user space to read data from
//              the PCIe device. ***NOTE: This routine returns the entire
//              buffer, (BUF_SIZE), count is ignored!. The user App must
//              do any needed processing on the buffer.
//
// Arguments: filp  : file pointer to opened device.
//            buf   : pointer to location in users space, where data is to
//                    be placed.
//            count : Amount of data in bytes user wishes to read.
//
// Returns: SUCCESS  = Success
//          CRIT_ERR = Critical failure
//
//  Modification log:
//  Date      Who  Description
//----------------------------------------------------------------------------

ssize_t XPCIe_Read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
  memcpy(buf, (char *)gWriteBuffer, count);
  printk(KERN_INFO"%s: XPCIe_Read: %d bytes have been read...\n", gDrvrName, count);
  return (0);
}

//---------------------------------------------------------------------------
// Name:        XPCIe_Ioctl
//
// Description: This routine is invoked from user space to configure the 
//              running driver. 
//
// Arguments: inode : 
//            filp  : File pointer to opened device.
//            cmd   : Ioctl command to execute.
//            arg   : Argument to Ioctl command.
//
// Returns: 0 on success, error code on failure.
//
// Modification log:
// Date      Who  Description
//
//---------------------------------------------------------------------------
int XPCIe_Ioctl(struct inode *inode, 
                struct file *filp, 
                unsigned int cmd,
                unsigned long arg)
{
  
  u32 regx;
  int ret = SUCCESS;

  switch (cmd) {

    case INITCARD:                    // Initailizes XBMD application
      XPCIe_InitCard();
      break;
    case INITRST:                     // Resets XBMD applications
      XPCIe_InitiatorReset();
      break;
    case DISPREGS:
      break;
    case RDDCSR:                     // Read: Device Control Status Register
      regx = XPCIe_ReadReg(0);  
      *((u32 *)arg) = regx; 
      break;
    case RDDDMACR:                   // Read: DMA Control Status Register
      regx = XPCIe_ReadReg(1);  
      *((u32 *)arg) = regx; 
      break;
    case RDWDMATLPA:                 // Read: Write DMA TLP Address Register
      regx = XPCIe_ReadReg(2);  
      *((u32 *)arg) = regx; 
      break;
    case RDWDMATLPS:                 // Read: Write DMA TLP Size Register
      regx = XPCIe_ReadReg(3);  
      *((u32 *)arg) = regx; 
      break;
    case RDWDMATLPC:                 // Read: Write DMA TLP Count Register
      regx = XPCIe_ReadReg(4);  
      *((u32 *)arg) = regx; 
      break;
    case RDWDMATLPP:                 // Read: Write DMA TLP Pattern Register
      regx = XPCIe_ReadReg(5);  
      *((u32 *)arg) = regx; 
      break;
    case RDRDMATLPP:                 // Read: Read DMA TLP Pattern Register
      regx = XPCIe_ReadReg(6);  
      *((u32 *)arg) = regx; 
      break;
    case RDRDMATLPA:                 // Read: Read DMA TLP Address Register
      regx = XPCIe_ReadReg(7);  
      *((u32 *)arg) = regx; 
      break;
    case RDRDMATLPS:                 // Read: Read DMA TLP Size Register
      regx = XPCIe_ReadReg(8);  
      *((u32 *)arg) = regx; 
      break;
    case RDRDMATLPC:                 // Read: Read DMA TLP Count Register
      regx = XPCIe_ReadReg(9);  
      *((u32 *)arg) = regx; 
      break;
    case RDWDMAPERF:                 // Read: Write DMA Performance Register
      regx = XPCIe_ReadReg(10);  
      *((u32 *)arg) = regx; 
      break;
    case RDRDMAPERF:                 // Read: Read DMA Performance Register
      regx = XPCIe_ReadReg(11);  
      *((u32 *)arg) = regx; 
      break;
    case RDRDMASTAT:                 // Read: Read DMA Status Register
      regx = XPCIe_ReadReg(12);  
      *((u32 *)arg) = regx; 
      break;
    case RDNRDCOMP:                  // Read: Number of Read Completion w/ Data Register
      regx = XPCIe_ReadReg(13);  
      *((u32 *)arg) = regx; 
      break;
    case RDRCOMPDSIZE:               // Read: Read Completion Size Register
      regx = XPCIe_ReadReg(14);  
      *((u32 *)arg) = regx; 
      break;
    case RDDLWSTAT:                  // Read: Device Link Width Status Register
      regx = XPCIe_ReadReg(15);  
      *((u32 *)arg) = regx; 
      break;
    case RDDLTRSSTAT:                // Read: Device Link Transaction Size Status Register
      regx = XPCIe_ReadReg(16);  
      *((u32 *)arg) = regx; 
      break;
    case RDDMISCCONT:                // Read: Device Miscellaneous Control Register
      regx = XPCIe_ReadReg(17);  
      *((u32 *)arg) = regx; 
      break;
    case RDDMISCONT:                // Read: Device MSI Control
      regx = XPCIe_ReadReg(18);  
      *((u32 *)arg) = regx; 
      break;
    case RDDLNKC:                   // Read: Device Directed Link Change Register
      regx = XPCIe_ReadReg(19);  
      *((u32 *)arg) = regx; 
      break;
    case DFCCTL:                    // Read: Device FC Control Register
      regx = XPCIe_ReadReg(20);  
      *((u32 *)arg) = regx; 
      break;
    case DFCPINFO:                  // Read: Device FC Posted Information
      regx = XPCIe_ReadReg(21);  
      *((u32 *)arg) = regx; 
      break;
    case DFCNPINFO:                 // Read: Device FC Non Posted Information
      regx = XPCIe_ReadReg(22);  
      *((u32 *)arg) = regx; 
      break;
    case DFCINFO:                  // Read: Device FC Completion Information
      regx = XPCIe_ReadReg(23);  
      *((u32 *)arg) = regx; 
      break;
    case WRDDMACR:                 // Write: DMA Control Status Register
      XPCIe_WriteReg(1, arg);
      break;
    case WRWDMATLPS:               // Write: Write DMA TLP Size Register
      XPCIe_WriteReg(3, arg);
      break;
    case WRWDMATLPC:               // Write: Write DMA TLP Count Register
      XPCIe_WriteReg(4, arg);
      break;
    case WRWDMATLPP:               // Write: Write DMA TLP Pattern Register
      XPCIe_WriteReg(5, arg);
      break;
    case WRRDMATLPS:               // Write: Read DMA TLP Size Register
      XPCIe_WriteReg(8, arg);
      break;
    case WRRDMATLPC:               // Write: Read DMA TLP Count Register
      XPCIe_WriteReg(9, arg);
      break;
    case WRRDMATLPP:               // Write: Read DMA TLP Pattern Register
      XPCIe_WriteReg(6, arg);
      break;
    case WRDMISCCONT:              // Write: Device Miscellaneous Control Register
      XPCIe_WriteReg(18, arg);
      break;
    case WRDDLNKC:                 // Write: Device Directed Link Change Register
      XPCIe_WriteReg(19, arg);
      break;
    case RDBMDREG:                 // Read: Any XBMD Reg.  Added generic functionality so all register can be read
      regx = XPCIe_ReadReg(*(u32 *)arg);  
      *((u32 *)arg) = regx; 
      break; 
    case RDCFGREG:                 // Read: Any CFG Reg.  Added generic functionality so all register can be read
      regx = XPCIe_ReadCfgReg(*(u32 *)arg);  
      *((u32 *)arg) = regx; 
      break;
   case WRBMDREG:                 // Write: Any BMD Reg.  Added generic functionality so all register can be read
      	XPCIe_WriteReg((*(bmdwr *)arg).reg,(*(bmdwr *)arg).value);
	printk(KERN_WARNING"%d: Write Register.\n", (*(bmdwr *)arg).reg);
	printk(KERN_WARNING"%d: Write Value\n", (*(bmdwr *)arg).value);
	break;
   case WRCFGREG:                 // Write: Any CFG Reg.  Added generic functionality so all register can be read
      	regx = XPCIe_WriteCfgReg((*(cfgwr *)arg).reg,(*(cfgwr *)arg).value);
	printk(KERN_WARNING"%d: Write Register.\n", (*(cfgwr *)arg).reg);
	printk(KERN_WARNING"%d: Write Value\n", (*(cfgwr *)arg).value);
	break;
    default:
      break;
  }

  return ret;
}


// Aliasing write, read, ioctl, etc...
struct file_operations XPCIe_Intf = {
    read:       XPCIe_Read,
    write:      XPCIe_Write,
    ioctl:      XPCIe_Ioctl,
    open:       XPCIe_Open,
    release:    XPCIe_Release,
};


static int XPCIe_init(void)
{
  // Find the Xilinx EP device.  The device is found by matching device and vendor ID's which is defined
  // at the top of this file.  Be default, the driver will look for 10EE & 0007.  If the core is generated 
  // with other settings, the defines at the top must be changed or the driver will not load
  gDev = pci_find_device (PCI_VENDOR_ID_XILINX, PCI_DEVICE_ID_XILINX_PCIE, gDev);
  if (NULL == gDev) {

    // If a matching device or vendor ID is not found, return failure and update kernel log. 
    // NOTE: In fedora systems, the kernel log is located at: /var/log/messages
    printk(KERN_WARNING"%s: Init: Hardware not found.\n", gDrvrName);
    return (CRIT_ERR);
  }

  // Get Base Address of registers from pci structure. Should come from pci_dev
  // structure, but that element seems to be missing on the development system.
  gBaseHdwr = pci_resource_start (gDev, 0);

  if (0 > gBaseHdwr) {
    printk(KERN_WARNING"%s: Init: Base Address not set.\n", gDrvrName);
    return (CRIT_ERR);
  } 

  // Print Base Address to kernel log
  printk(KERN_INFO"%s: Init: Base hw val %X\n", gDrvrName, (unsigned int)gBaseHdwr);

  // Get the Base Address Length
  gBaseLen = pci_resource_len (gDev, 0);

  // Print the Base Address Length to Kernel Log
  printk(KERN_INFO"%s: Init: Base hw len %d\n", gDrvrName, (unsigned int)gBaseLen);

  // Remap the I/O register block so that it can be safely accessed.
  // I/O register block starts at gBaseHdwr and is 32 bytes long.
  // It is cast to char because that is the way Linus does it.
  // Reference "/usr/src/Linux-2.4/Documentation/IO-mapping.txt".
  gBaseVirt = ioremap(gBaseHdwr, gBaseLen);
  if (!gBaseVirt) {
    printk(KERN_WARNING"%s: Init: Could not remap memory.\n", gDrvrName);
    return (CRIT_ERR);
  } 

  // Print out the aquired virtual base addresss
  printk(KERN_INFO"%s: Init: Virt HW address %X\n", gDrvrName, (unsigned int)gBaseVirt);

  // Get IRQ from pci_dev structure. It may have been remapped by the kernel,
  // and this value will be the correct one.
  gIrq = gDev->irq;
  printk(KERN_INFO"%s: Init: Device IRQ: %X\n",gDrvrName, gIrq);


  //---START: Initialize Hardware

  // Check the memory region to see if it is in use
  if (0 > check_mem_region(gBaseHdwr, XBMD_REGISTER_SIZE)) {
    printk(KERN_WARNING"%s: Init: Memory in use.\n", gDrvrName);
    return (CRIT_ERR);
  }

  // Try to gain exclusive control of memory for demo hardware.
  request_mem_region(gBaseHdwr, XBMD_REGISTER_SIZE, "3GIO_Demo_Drv");
  // Update flags
  gStatFlags = gStatFlags | HAVE_REGION;

  printk(KERN_INFO"%s: Init: Initialize Hardware Done..\n",gDrvrName);

  // Request IRQ from OS.
  // In past architectures, the SHARED and SAMPLE_RANDOM flags were called: SA_SHIRQ and SA_SAMPLE_RANDOM
  // respectively.  In older Fedora core installations, the request arguments may need to be reverted back.
  // SA_SHIRQ | SA_SAMPLE_RANDOM
  printk(KERN_INFO"%s: ISR Setup..\n", gDrvrName);
  if (0 > request_irq(gIrq, &XPCIe_IRQHandler, IRQF_SHARED | IRQF_SAMPLE_RANDOM, gDrvrName, gDev)) {
    printk(KERN_WARNING"%s: Init: Unable to allocate IRQ",gDrvrName);
    return (CRIT_ERR);
  }
  // Update flags stating IRQ was successfully obtained
  gStatFlags = gStatFlags | HAVE_IRQ;

  // Bus Master Enable
  if (0 > pci_enable_device(gDev)) {
    printk(KERN_WARNING"%s: Init: Device not enabled.\n", gDrvrName);
    return (CRIT_ERR);
  }

  //--- END: Initialize Hardware

  //--- START: Allocate Buffers

  // Allocate the read buffer with size BUF_SIZE and return the starting address
  gReadBuffer = pci_alloc_consistent(gDev, BUF_SIZE, &gReadHWAddr);
  if (NULL == gReadBuffer) {
    printk(KERN_CRIT"%s: Init: Unable to allocate gBuffer.\n",gDrvrName);
    return (CRIT_ERR);
  }
  // Print Read buffer size and address to kernel log
  printk(KERN_INFO"%s: Read Buffer Allocation: %X->%X\n", gDrvrName, (unsigned int)gReadBuffer, (unsigned int)gReadHWAddr);

  // Allocate the write buffer with size BUF_SIZE and return the starting address
  gWriteBuffer = pci_alloc_consistent(gDev, BUF_SIZE, &gWriteHWAddr);
  if (NULL == gWriteBuffer) {
    printk(KERN_CRIT"%s: Init: Unable to allocate gBuffer.\n",gDrvrName);
    return (CRIT_ERR);
  }
  // Print Write buffer size and address to kernel log  
  printk(KERN_INFO"%s: Write Buffer Allocation: %X->%X\n", gDrvrName, (unsigned int)gWriteBuffer, (unsigned int)gWriteHWAddr);

  //--- END: Allocate Buffers

  //--- START: Register Driver

  // Register with the kernel as a character device.
  if (0 > register_chrdev(gDrvrMajor, gDrvrName, &XPCIe_Intf)) {
    printk(KERN_WARNING"%s: Init: will not register\n", gDrvrName);
    return (CRIT_ERR);
  }
  printk(KERN_INFO"%s: Init: module registered\n", gDrvrName);
  gStatFlags = gStatFlags | HAVE_KREG;

  //--- END: Register Driver

  // The driver is now successfully loaded.  All HW is initialized, IRQ's assigned, and buffers allocated
  printk("%s driver is loaded\n", gDrvrName);


  // Initializing card registers
  XPCIe_InitCard();

  return 0;
}

//--- XPCIe_InitiatorReset(): Resets the XBMD reference design
//--- Arguments: None
//--- Return Value: None
//--- Detailed Description: Writes a 1 to the DCSR register which resets the XBMD design
void XPCIe_InitiatorReset()
{
  XPCIe_WriteReg(0, 1);                   // Write: DCSR (offset 0) with value of 1 (Reset Device)
  XPCIe_WriteReg(0, 0);                   // Write: DCSR (offset 0) with value of 0 (Make Active)
}


//--- XPCIe_InitCard(): Initializes XBMD descriptor registers to default values
//--- Arguments: None
//--- Return Value: None
//--- Detailed Description: 1) Resets device
//---                       2) Writes specific values into the XBMD registers inside the EP 
void XPCIe_InitCard()
{

  XPCIe_WriteReg(0, 1);                   // Write: DCSR (offset 0) with value of 1 (Reset Device)
  XPCIe_WriteReg(0, 0);                   // Write: DCSR (offset 0) with value of 0 (Make Active)

  XPCIe_WriteReg(2, gWriteHWAddr);        // Write: Write DMA TLP Address register with starting address
  XPCIe_WriteReg(3, 0x20);                // Write: Write DMA TLP Size register with default value (32dwords)
  XPCIe_WriteReg(4, 0x2000);              // Write: Write DMA TLP Count register with default value (2000)
  XPCIe_WriteReg(5, 0x00000000);          // Write: Write DMA TLP Pattern register with default value (0x0)

  XPCIe_WriteReg(6, 0xfeedbeef);          // Write: Read DMA Expected Data Pattern with default value (feedbeef)
  XPCIe_WriteReg(7, gReadHWAddr);         // Write: Read DMA TLP Address register with starting address.
  XPCIe_WriteReg(8, 0x20);                // Write: Read DMA TLP Size register with default value (32dwords)
  XPCIe_WriteReg(9, 0x2000);              // Write: Read DMA TLP Count register with default value (2000)
}

//--- XPCIe_exit(): Performs any cleanup required before releasing the device
//--- Arguments: None
//--- Return Value: None
//--- Detailed Description: Performs all cleanup functions required before releasing device
static void XPCIe_exit(void)
{

  // Check if we have a memory region and free it
  if (gStatFlags & HAVE_REGION) {
     (void) release_mem_region(gBaseHdwr, XBMD_REGISTER_SIZE);}

    // Check if we have an IRQ and free it
    if (gStatFlags & HAVE_IRQ) {
        (void) free_irq(gIrq, gDev);
    }

    // Free Write and Read buffers allocated to use
    if (NULL != gReadBuffer)
        (void) kfree(gReadBuffer);
    if (NULL != gWriteBuffer)
        (void) kfree(gWriteBuffer);

    // Free memory allocated to our Endpoint
    pci_free_consistent(gDev, BUF_SIZE, gReadBuffer, gReadHWAddr);
    pci_free_consistent(gDev, BUF_SIZE, gWriteBuffer, gWriteHWAddr);

    gReadBuffer = NULL;
    gWriteBuffer = NULL;

    // Free up memory pointed to by virtual address
    if (gBaseVirt != NULL) {
        iounmap(gBaseVirt);
     }
    
    gBaseVirt = NULL;

    // Unregister Device Driver
    if (gStatFlags & HAVE_KREG) {
      unregister_chrdev(gDrvrMajor, gDrvrName);
    }

    gStatFlags = 0;

    // Update Kernel log stating driver is unloaded
    printk(KERN_ALERT"%s driver is unloaded\n", gDrvrName);
}

// Driver Entry Point
module_init(XPCIe_init);

// Driver Exit Point
module_exit(XPCIe_exit);


void XPCIe_IRQHandler(int irq, void *dev_id, struct pt_regs *regs)
{
  u32 i, regx;

  printk(KERN_WARNING"%s: Interrupt Handler Start ..",gDrvrName);
  
  for (i = 0; i < 32; i++) {
     regx = XPCIe_ReadReg(i);
     printk(KERN_WARNING"%s : REG<%d> : 0x%X\n", gDrvrName, i, regx);
  }

  printk(KERN_WARNING"%s Interrupt Handler End ..\n", gDrvrName);
}

u32 XPCIe_ReadReg (u32 dw_offset)
{
        u32 ret = 0;
        //u32 reg_addr = (u32)(gBaseVirt + (4 * dw_offset));
        //ret = readl(reg_addr);  
        ret = readl(gBaseVirt + (4 * dw_offset));  

        return ret; 
}

void XPCIe_WriteReg (u32 dw_offset, u32 val)
{
        //u32 reg_addr = (u32)(gBaseVirt + (4 * dw_offset));
        writel(val, (gBaseVirt + (4 * dw_offset)));
}

ssize_t* XPCIe_ReadMem(char *buf, size_t count)
{

    int ret = 0;
    dma_addr_t dma_addr;
                                                                                
    //make sure passed in buffer is large enough
    if ( count < BUF_SIZE )  {
      printk("%s: XPCIe_Read: passed in buffer too small.\n", gDrvrName);
      ret = -1;
      goto exit;
    }

    down(&gSem[SEM_DMA]);

    // pci_map_single return the physical address corresponding to 
    // the virtual address passed to it as the 2nd parameter

    dma_addr = pci_map_single(gDev, gReadBuffer, BUF_SIZE, PCI_DMA_FROMDEVICE);
    if ( 0 == dma_addr )  {
        printk("%s: XPCIe_Read: Map error.\n",gDrvrName);
        ret = -1;
        goto exit;
    }

    // Now pass the physical address to the device hardware. This is now
    // the destination physical address for the DMA and hence the to be
    // put on Memory Transactions

    // Do DMA transfer here....

    printk("%s: XPCIe_Read: ReadBuf Virt Addr = %x Phy Addr = %x.\n", 
            gDrvrName, (unsigned int)gReadBuffer, (unsigned int)dma_addr);
                                                                   
    // Unmap the DMA buffer so it is safe for normal access again.
    pci_unmap_single(gDev, dma_addr, BUF_SIZE, PCI_DMA_FROMDEVICE);
                                                                                
    up(&gSem[SEM_DMA]);

    // Now it is safe to copy the data to user space.
    if ( copy_to_user(buf, gReadBuffer, BUF_SIZE) )  {
        ret = -1;
        printk("%s: XPCIe_Read: Failed copy to user.\n",gDrvrName);
        goto exit;
    }
    exit:
      return ret;
}

ssize_t XPCIe_WriteMem(const char *buf, size_t count) {
    int ret = 0;
    dma_addr_t dma_addr;

    if ( (count % 4) != 0 )  {
       printk("%s: XPCIe_Write: Buffer length not dword aligned.\n",gDrvrName);
       ret = -1;
       goto exit;
    }

    // Now it is safe to copy the data from user space.
    if ( copy_from_user(gWriteBuffer, buf, count) )  {
        ret = -1;
        printk("%s: XPCIe_Write: Failed copy to user.\n",gDrvrName);
        goto exit;
    }

    //set DMA semaphore if in loopback
    down(&gSem[SEM_DMA]);

    // pci_map_single return the physical address corresponding to 
    // the virtual address passed to it as the 2nd parameter

    dma_addr = pci_map_single(gDev, gWriteBuffer, BUF_SIZE, PCI_DMA_FROMDEVICE);
    if ( 0 == dma_addr )  {
        printk("%s: XPCIe_Write: Map error.\n",gDrvrName);
        ret = -1;
        goto exit;
    }

    // Now pass the physical address to the device hardware. This is now
    // the source physical address for the DMA and hence the to be
    // put on Memory Transactions

    // Do DMA transfer here....

    printk("%s: XPCIe_Write: WriteBuf Virt Addr = %x Phy Addr = %x.\n",  
           gDrvrName, (unsigned int)gReadBuffer, (unsigned int)dma_addr);
                                                                   
    // Unmap the DMA buffer so it is safe for normal access again.
    pci_unmap_single(gDev, dma_addr, BUF_SIZE, PCI_DMA_FROMDEVICE);
                                                                                
    up(&gSem[SEM_DMA]);

    exit:
      return (ret);
}

u32 XPCIe_ReadCfgReg (u32 byte) {
   u32 pciReg;
   if (pci_read_config_dword(gDev, byte, &pciReg) < 0) {
        printk("%s: XPCIe_ReadCfgReg: Reading PCI interface failed.",gDrvrName);
        return (-1);
   }
   return (pciReg);
}

u32 XPCIe_WriteCfgReg (u32 byte, u32 val) {
   if (pci_write_config_dword(gDev, byte, val) < 0) {
        printk("%s: XPCIe_Read Device Control: Reading PCI interface failed.",gDrvrName);
        return (-1);
   }
   return 1;
}
