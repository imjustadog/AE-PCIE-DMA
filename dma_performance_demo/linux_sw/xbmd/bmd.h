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

// --------------------------------------------------------------------------------------------------------------
//  bmd_t class definition:
//  Defines the Public and Private variables associated with bmd_t class type
//  Defines the member functions associated with this class
//---------------------------------------------------------------------------------------------------------------

#include "xbmd_descriptors.h" 
#include "xbmd_main.h"

extern xbmd_descriptors_t xbmd_descriptors;

//*** BMD Class ***//
class bmd_t {
public:

  int wr_mbps;                     // Contains Write Performance.  SUM of total performance per run (not iteration)
  int rd_mbps;                     // Contains Read Performance.  SUM of total performance per run (not iteration)
  const char* wr_result_text;      // Contains text stating success of WR DMA or error condition if one exists
  const char* rd_result_text;      // Contains text stating success of RD DMA or error condition if one exists
  const char* bmd_fatal_text;      // Contains text stating error condition if BMD fails during setup of transfer
  bool wr_success;                 // Bool declaring if Write DMA was successful
  bool rd_success;                 // Bool declaring if Read DMA was successful 
  char* wr_mbps_c;                 // Char string showing performance that is passed to GUI WR MBPS field
  char* rd_mbps_c;                 // Char string showing performance that is passed to GUI RD MBPS field


  // Constructor and destructor prototypes
  bmd_t(void);
  ~bmd_t(void);
   
  // run_xbmd sets up and runs a single DMA iteration.  It takes in a global struct containing descriptor 
  // register values needed to set up transfer
  int run_xbmd(xbmd_descriptors_t xbmd_descriptors, int ii);

  // Gets Read and Write performance values and returns a char string containing the average performance for 
  // and entire DMA run (multiple iterations)
  char* get_rd_mbps(int iter_count);
  char* get_wr_mbps(int iter_count);

  // Returns a boolean for write and/or read stating whether a DMA transfer was successful or not.  Used to identify 
  // if a failure has occurred in a single DMA transfer during a run.
  bool get_wr_success(void);
  bool get_rd_success(void);

  // Returns a const char* stating whether the transfer was successful or not.  Result is provided displayed into 
  // GUI under status fields
  const char* get_wr_result_text(void);
  const char* get_rd_result_text(void);

  // Reads BMD descriptor registers for display to GUI
  int read_bmd_regs(int g_devFile);

};
