/*
==========================================================================================
                          "Copyright © 2006 Avnet, Inc. All Rights Reserved.
============================================================================================
THIS SOFTWARE IS provided “AS IS” WITHOUT WARRANTY OF ANY KIND.  AVNET MAKES NO WARRANTIES, 
EITHER EXPRESS OR IMPLIED, WITH RESPECT TO THIS SOFTWARE.  AVNET SPECIFICALLY DISCLAIMS THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE AND ANY WARRANTY 
AGAINST INFRINGEMENT OF ANY INTELLECTUAL PROPERTY RIGHT OF ANY THIRD PARTY WITH REGARD TO 
THIS SOFTWARE. AVNET RESERVES THE RIGHT, WITHOUT NOTICE, TO MAKE CHANGES TO THIS SOFTWARE. 
 
IN NO EVENT SHALL AVNET BE LIABLE FOR ANY INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL 
DAMAGES OF ANY KIND OR NATURE, INCLUDING, WITHOUT LIMITATION, BUSINESS INTERRUPTION COSTS, 
LOSS OF PROFIT OR REVENUE, LOSS OF DATA, PROMOTIONAL OR MANUFACTURING EXPENSES, OVERHEAD, 
INJURY TO REPUTATION OR LOSS OF CUSTOMERS ARISING OUT OF THE USE OR INSTALLATION OF THIS 
SOFTWARE."
============================================================================================ 
*/
#ifndef _IOCTRL_H_
#define _IOCTRL_H_

// DMA Register Locations (byte offset from BAR 0)
#define DCR_OFFSET					0x0				// Device Control Register
#define	DCSR_OFFSET					0x4				// Device Control and Status Register
#define WRITE_ADDR_OFFSET			0x8				// Write DMA TLP Address Register
#define WRITE_SIZE_OFFSET			0xC				// Write DMA TLP Size Register
#define WRITE_COUNT_OFFSET			0x10			// Write DMA TLP Count Register
#define WRITE_PATTERN_OFFSET		0x14			// Write DMA Data Pattern Register
#define READ_PATTERN_OFFSET			0x18			// Read  DMA Data Pattern Register
#define READ_ADDR_OFFSET			0x1C			// Read  DMA TLP Address Register
#define READ_SIZE_OFFSET			0x20			// Read  DMA TLP Size Register
#define READ_COUNT_OFFSET			0x24			// Read  DMA TLP Count Register
#define WRITE_DMA_PERF_OFFSET		0x28			// Write DMA Performance Statistics Register
#define READ_DMA_PERF_OFFSET		0x2C			// Read  DMA Performance Statistics Register
#define READ_DMA_STATUS_OFFSET		0x30			// Read  DMA Status Register
#define READ_DMA_COMPLETE_OFFSET	0x34			// Read  DMA Number of Completions Register
#define READ_DMA_COMPLETE_SIZE		0x38			// Read  DMA Size of Completions Register
#define DEVICE_LINK_WIDTH_STATUS	0x3C			// PCIe Device Link Width Register
#define DEVICE_LINK_TRANS_SIZE		0x40			// PCIe Device Link Transaction Size Register


#define MAX_TRANSFER_SIZE	32768					// RRW - match application allocation

// IOCTL code
#define MDS_IOCTL_SET_BAR CTL_CODE (FILE_DEVICE_UNKNOWN,   \
                                    0x000,                 \
                                    METHOD_BUFFERED,       \
                                    FILE_ANY_ACCESS)

#define MDS_IOCTL_SET_OFFSET CTL_CODE (FILE_DEVICE_UNKNOWN,   \
                                    0x001,                 \
                                    METHOD_BUFFERED,       \
                                    FILE_ANY_ACCESS)

#define MDS_IOCTL_GET_USER_LED_STATE CTL_CODE (FILE_DEVICE_UNKNOWN,   \
                                    0x002,                 \
                                    METHOD_BUFFERED,       \
                                    FILE_ANY_ACCESS)

#define MDS_IOCTL_SET_USER_LED_STATE CTL_CODE (FILE_DEVICE_UNKNOWN,   \
                                    0x003,                 \
                                    METHOD_BUFFERED,       \
                                    FILE_ANY_ACCESS)

#define IOCTL_INITIATOR_RESET CTL_CODE (FILE_DEVICE_UNKNOWN,   \
                                    0x004,                 \
                                    METHOD_BUFFERED,       \
                                    FILE_ANY_ACCESS)

#define IOCTL_CLEAR_INITIATOR_RESET CTL_CODE (FILE_DEVICE_UNKNOWN,   \
                                    0x005,                 \
                                    METHOD_BUFFERED,       \
                                    FILE_ANY_ACCESS)

#define MDS_IOCTL_GET_DIP_STATE CTL_CODE (FILE_DEVICE_UNKNOWN,   \
                                    0x006,                 \
                                    METHOD_BUFFERED,       \
                                    FILE_ANY_ACCESS)

#define MDS_IOCTL_GET_PB_STATE CTL_CODE (FILE_DEVICE_UNKNOWN,   \
                                    0x007,                 \
                                    METHOD_BUFFERED,       \
                                    FILE_ANY_ACCESS)

#define IOCTL_SET_DMA_WRITE_ADDR CTL_CODE (FILE_DEVICE_UNKNOWN,   \
                                    0x008,                 \
                                    METHOD_BUFFERED,       \
                                    FILE_ANY_ACCESS)

#define IOCTL_SET_DMA_WRITE_SIZE CTL_CODE (FILE_DEVICE_UNKNOWN,   \
                                    0x009,                 \
                                    METHOD_BUFFERED,       \
                                    FILE_ANY_ACCESS)

#define IOCTL_SET_DMA_WRITE_COUNT CTL_CODE (FILE_DEVICE_UNKNOWN,   \
                                    0x00A,                 \
                                    METHOD_BUFFERED,       \
                                    FILE_ANY_ACCESS)

#define IOCTL_SET_DMA_WRITE_PATTERN CTL_CODE (FILE_DEVICE_UNKNOWN,   \
                                    0x00B,                 \
                                    METHOD_BUFFERED,       \
                                    FILE_ANY_ACCESS)

#define IOCTL_SET_DMA_READ_ADDR CTL_CODE (FILE_DEVICE_UNKNOWN,   \
                                    0x00C,                 \
                                    METHOD_BUFFERED,       \
                                    FILE_ANY_ACCESS)

#define IOCTL_SET_DMA_READ_SIZE CTL_CODE (FILE_DEVICE_UNKNOWN,   \
                                    0x00D,                 \
                                    METHOD_BUFFERED,       \
                                    FILE_ANY_ACCESS)

#define IOCTL_SET_DMA_READ_COUNT CTL_CODE (FILE_DEVICE_UNKNOWN,   \
                                    0x00E,                 \
                                    METHOD_BUFFERED,       \
                                    FILE_ANY_ACCESS)

#define IOCTL_SET_DMA_READ_PATTERN CTL_CODE (FILE_DEVICE_UNKNOWN,   \
                                    0x00F,                 \
                                    METHOD_BUFFERED,       \
                                    FILE_ANY_ACCESS)


#define IOCTL_DMA_START CTL_CODE (FILE_DEVICE_UNKNOWN,   \
                                    0x010,                 \
                                    METHOD_BUFFERED,       \
                                    FILE_ANY_ACCESS)

#define IOCTL_GET_DMA_STATUS CTL_CODE (FILE_DEVICE_UNKNOWN,   \
                                    0x011,                 \
                                    METHOD_BUFFERED,       \
                                    FILE_ANY_ACCESS)

#define IOCTL_GET_DMA_WRITE_PERF CTL_CODE (FILE_DEVICE_UNKNOWN,   \
                                    0x012,                 \
                                    METHOD_BUFFERED,       \
                                    FILE_ANY_ACCESS)

#define IOCTL_GET_DMA_READ_PERF CTL_CODE (FILE_DEVICE_UNKNOWN,   \
                                    0x013,                 \
                                    METHOD_BUFFERED,       \
                                    FILE_ANY_ACCESS)

#define IOCTL_GET_DMA_WRITE_ADDR CTL_CODE (FILE_DEVICE_UNKNOWN,   \
                                    0x014,                 \
                                    METHOD_BUFFERED,       \
                                    FILE_ANY_ACCESS)

#define IOCTL_GET_DMA_WRITE_SIZE CTL_CODE (FILE_DEVICE_UNKNOWN,   \
                                    0x015,                 \
                                    METHOD_BUFFERED,       \
                                    FILE_ANY_ACCESS)

#define IOCTL_GET_DMA_WRITE_COUNT CTL_CODE (FILE_DEVICE_UNKNOWN,   \
                                    0x016,                 \
                                    METHOD_BUFFERED,       \
                                    FILE_ANY_ACCESS)

#define IOCTL_GET_DMA_WRITE_PATTERN CTL_CODE (FILE_DEVICE_UNKNOWN,   \
                                    0x017,                 \
                                    METHOD_BUFFERED,       \
                                    FILE_ANY_ACCESS)

#define IOCTL_GET_DMA_READ_ADDR CTL_CODE (FILE_DEVICE_UNKNOWN,   \
                                    0x018,                 \
                                    METHOD_BUFFERED,       \
                                    FILE_ANY_ACCESS)

#define IOCTL_GET_DMA_READ_SIZE CTL_CODE (FILE_DEVICE_UNKNOWN,   \
                                    0x019,                 \
                                    METHOD_BUFFERED,       \
                                    FILE_ANY_ACCESS)

#define IOCTL_GET_DMA_READ_COUNT CTL_CODE (FILE_DEVICE_UNKNOWN,   \
                                    0x01A,                 \
                                    METHOD_BUFFERED,       \
                                    FILE_ANY_ACCESS)

#define IOCTL_GET_DMA_READ_PATTERN CTL_CODE (FILE_DEVICE_UNKNOWN,   \
                                    0x01B,                 \
                                    METHOD_BUFFERED,       \
                                    FILE_ANY_ACCESS)

#define IOCTL_GET_DMA_REGISTER CTL_CODE (FILE_DEVICE_UNKNOWN,   \
                                    0x01C,                 \
                                    METHOD_BUFFERED,       \
                                    FILE_ANY_ACCESS)

#define IOCTL_CLEANUP CTL_CODE (FILE_DEVICE_UNKNOWN,   \
                                    0x01D,                 \
                                    METHOD_BUFFERED,       \
                                    FILE_ANY_ACCESS)

#define IOCTL_DISABLE_INTERRUPTS CTL_CODE (FILE_DEVICE_UNKNOWN,   \
                                    0x01E,                 \
                                    METHOD_BUFFERED,       \
                                    FILE_ANY_ACCESS)

#define IOCTL_ENABLE_INTERRUPTS CTL_CODE (FILE_DEVICE_UNKNOWN,   \
                                    0x01F,                 \
                                    METHOD_BUFFERED,       \
                                    FILE_ANY_ACCESS)
#endif // _IOCTRL_H_
