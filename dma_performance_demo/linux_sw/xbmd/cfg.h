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

////////////////////////////////////
////**** Class Definitions ****////
//////////////////////////////////

// extern xbmd_descriptors
#include "xbmd_main.h"

//*** CFG Class ***//
class cfg_t {
public:
  // Member variables

  // CFG Offsets
  unsigned int   pm_offset;                      // Power Management Offset
  unsigned int   msi_offset;                     // MSI Offset
  unsigned int   pcie_cap_offset;                // PCIE Cap Offset
  unsigned int   device_cap_offset;              // Device CAP offset
  unsigned int   device_stat_cont_offset;        // Device Stat/Control Offset
  unsigned int   link_cap_offset;                // Link Cap Offset
  unsigned int   link_stat_cont_offset;          // Link Stat/Control Offset

  // CFG Register Values
  unsigned int   link_width_cap;      
  unsigned int   link_speed_cap;
  unsigned int   link_width;
  unsigned int   link_speed;
  unsigned int   link_control;
  unsigned int   pm_stat_control;
  unsigned int   pm_capabilities;
  unsigned int   msi_control;

  char*          link_stat_neg_link_speed_c;     // char* containing negotiated link speed
  unsigned int   link_stat_neg_link_speed;       // int containing negotiated link speed
  char*          link_stat_neg_link_width_c;     // char* containing negotiated link width
  unsigned int   link_stat_neg_link_width;       // int containing negotiated link speed
  const char*    cfg_fatal_text;                 // Contains text stating error condition if BMD fails during setup 
                                                 // of transfer
  // constructor and destructor
  cfg_t(void);
  ~cfg_t(void);

  // Reads CFG registers for display to GUI
  int cfg_read_regs(int g_devFile);

  // Read/Writes CFG space - not currently used
  int cfg_rdwr_reg(int g_devFile,int reg, int reg_value, int wr_en);

  // Enables specific functionality within EP configuration space (Phantom Functions, Extended Tags, Error Reporting
  int cfg_enable_functionality(int g_devFile, int dev_ctrl_phantom_func_en, int dev_ctrl_extended_tag_en, int error_reporting_en);

  // Checks link width and link speed
  int cfg_check_link_width(int g_devFile);
  int cfg_check_link_speed(int g_devFile);

  int cfg_get_capabilities(int g_devFile);
  int cfg_update_regs(int g_devFile);


};

