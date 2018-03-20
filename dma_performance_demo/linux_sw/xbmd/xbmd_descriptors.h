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
//-- Filename: xbmd_descriptors.h
//--
//-- Description: Global XBMD descriptors register struct
//--              
//--------------------------------------------------------------------------------

struct xbmd_descriptors_t{

  int               num_iter;               // Number of Iterations
  unsigned int      wr_enable;              // Write Enable
  unsigned int      rd_enable;              // Read Enable
  int               wr_tlp_size;            // Write TLP Size
  int               rd_tlp_size;            // Read TLP Size 
  int               num_wr_tlps;            // Number of Write TLP's
  int               num_rd_tlps;            // Number of Read TLP's
  const char*       wr_pattern_new;         // Write Pattern
  const char*       rd_pattern_new;         // Read Pattern

  char*             iter_count_t;           // Number of Iterations text
  char*             wr_bytes_to_trans;      // Number of Write Bytes to Transfer
  char*             rd_bytes_to_trans;      // Number of Read Bytes to Transfer
  int               bytes_to_trans;         // Temporary Bytes to Transfer (int)
  bool              wr_pattern_valid;       // Write Pattern Valid
  bool              rd_pattern_valid;       // Read Pattern Valid
  int               wr_pattern_length;      // Write Pattern Length
  int               rd_pattern_length;      // Read Pattern Length 

  const char*       wr_status;              // Write Status text
  const char*       rd_status;              // Read Status text 
  char*             wr_mbps;                // Write Performance text 
  char*             rd_mbps;                // Read Performance text

  bool              phantom_enable;         // Phantom Functions enable
  bool              aut_change_enable;      // Autonomous Change enable 
  bool              trans_streaming;        // Transmit Streaming enable 
  int               random_enable;          // Randomization enable 

  unsigned int      pm_offset;              // Power Management CAP Offset
  unsigned int      msi_offset;             // MSI CAP offset
  unsigned int      pcie_cap_offset;        // PCIE CAP offset
  unsigned int      device_cap_offset;      // Device CAP offset
  unsigned int      device_stat_cont_offset;// Device Status/Control offset
  unsigned int      link_cap_offset;        // Link CAP offset
  unsigned int      link_stat_cont_offset;  // Link Status/Control offset

  char*             neg_link_width_char;    // Negotiated Link Width (char*)
  char*             neg_link_speed_char;    // Negotiated Link Speed (char*)
  int               neg_link_width;         // Negotiated Link Width (int)
  int               neg_link_speed;         // Negotiated Link Spee (int)

  int               g_devFile;              // Device file number

  //Constructor - handles initialization of all variables
  xbmd_descriptors_t() {
   
    // Number of iterations
    num_iter = 1;
    iter_count_t= new char[10];

    // Write/Read TLP Size
    wr_tlp_size = 32;
    rd_tlp_size = 32;

    // Write/Read # TLP's
    num_wr_tlps = 32;               
    num_rd_tlps = 32;

    // Write/Read # bytes to trans
    wr_bytes_to_trans = new char[1];
    rd_bytes_to_trans = new char[1];
    bytes_to_trans = 000000;

    // Write/Read Pattern variables
    wr_pattern_valid = true;
    rd_pattern_valid = true;
    wr_pattern_length = 0;
    rd_pattern_length = 0;
    wr_pattern_new = "FEEDBEEF";
    rd_pattern_new = "FEEDBEEF";

    // Write/Read DMA Enable
    rd_enable = 0x00000000;
    wr_enable = 0x00000000;

    // Write/Read Performance
    rd_mbps = new char[1];
    wr_mbps = new char[1];

    // DMA additional option variables
    phantom_enable = false;
    aut_change_enable = false;
    trans_streaming = false;
    random_enable = 0;

    // CFG Space Capabilities offsets
    pm_offset                  = 0;               
    msi_offset                 = 0;
    pcie_cap_offset            = 0; 
    device_cap_offset          = 0; 
    device_stat_cont_offset    = 0; 
    link_cap_offset            = 0; 
    link_stat_cont_offset      = 0;     

    g_devFile = -1;
    //initialize all variables here with values below and then erase below values
  }
};
