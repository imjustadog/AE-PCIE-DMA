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
//  cfg.cpp description:
//  This module is sub-class of the xbmd_ep_t class type.  It is responsible for the following:
//    1) Read Endpoint configuration registers and recording contents in cfg_space.txt for viewing in the GUI
//    2) Reading Link capability structure and returning negotiated link width and speed
//    3) Enabling specific functionality in Endpoint configuration space for DMA transfers (i.e phantom functions, 
//       error reporting, etc..)
//    4) Reading/Writing Endpoint configuration space (currently unused)
//---------------------------------------------------------------------------------------------------------------
#include <sys/ioctl.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include "xbmd.h"
#include "MersenneTwister.h"
#include "cfg.h"

using namespace std;

  // Constructor and destructor prototypes
  cfg_t::cfg_t(void){
    this->link_stat_neg_link_speed_c = new char[1];
    this->link_stat_neg_link_width_c = new char[1];
    this->cfg_fatal_text = " ";
  }
  cfg_t::~cfg_t(void){
    delete link_stat_neg_link_speed_c;
    delete link_stat_neg_link_width_c;
  }

//--- read_cfg_regs(): Reads Endpoint Configuration register and outputs contents to cfg_space.txt.
//--- Arguments: int: endpoint device file
//--- Return Value: Returns int SUCCESS or FAILURE
//--- Detailed Description: This module does the following:
//---                       1) Reads Endpoint Configuration register and outputs contents to cfg_space.txt.
int cfg_t::cfg_read_regs(int g_devFile){
  int ii;
  unsigned int reg_value = 0;
  // FILE IO
  ofstream file;

  // ***** Open/setup log files ***** //
  file.open ("cfg_space.txt");
  if (!file.is_open()) {
    printf("Error initializing CFG report");
    return CRIT_ERR;
  }

  //Read CFG space sequentially and output to text file.  
  // Possible future enhancement would be to associate each offset with specific CFG space name
  file <<endl<<"*** EP Device Type 0 Configuration Space ***\n"<<endl;
  file << "Offset    Value\n"<<endl;
  // Read CFG space up to 120H
  for (ii = 0; ii <0x120;ii=ii+4) {
    reg_value = ii;
    if (ioctl(g_devFile, RDCFGREG, &reg_value) < 0) {

      // If CFG read fails - state failure in file 
      file << "ERROR: Could not read CFG Register 0x"<<ii<<". Verify device is present and driver loaded correctly"<<endl;

      //update cfg_fatal_text for display in main status bar
      this->cfg_fatal_text = "Read of Endpoint Configuration Registers FAILED";
      // return failure condition 
      return CRIT_ERR;
    } else {
      // read is successful so update file
      file <<setw(2)<<left<<"0x"<<hex<<ii<<"  "<<setw(25)<<setfill(' ')<<left<<reg_value;  

      switch (ii) {
			
      case 0: // 128 MPS
        file <<setw(2)<<left<<"Device ID/Vendor ID"<<endl;  
        break;
      case 4:  // 256 MPS
        file <<setw(2)<<left<<"Status/Command"<<endl;  
        break;				
      case 8:  // 512 MPS
        file <<"Class Code/Revision ID  "<<endl;
        break;
      case 12:  // 1024 MPS 
        file <<"BIST/Header Type/Lat. Timer/Cache Line size"<<endl;
        break;	 
      case 16: // 2048 MPS  
        file <<"BAR0"<<endl;
        break;
      case 20: // 4096 MPS  
        file <<"BAR1"<<endl;
        break;
      case 24: // 4096 MPS  
        file <<"BAR2"<<endl;
        break;
      case 28: // 4096 MPS  
        file <<"BAR3"<<endl;
        break;
      case 32: // 4096 MPS  
        file <<"BAR4"<<endl;
        break;
      case 36: // 4096 MPS  
        file <<"BAR5"<<endl;
        break;
      case 40: // 4096 MPS  
        file <<"CIS Pointer"<<endl;
        break;
      case 44: // 4096 MPS  
        file <<"Subsystem ID/ Subsystem Vendor ID"<<endl;
        break;
      case 48: // 4096 MPS  
        file <<"Expansion ROM Base Address"<<endl;
        break;
      case 52: // 4096 MPS  
        file <<"Reserved/Cap. Pointer"<<endl;
        break;
      case 56: // 4096 MPS  
        file <<"Reserved"<<endl;
        break;
      case 60: // 4096 MPS  
        file <<"Max Lat/Min Gnt/INT Pin/ INT Line"<<endl;
        break;
      default: {  
      file <<endl;  // fall through
      }
  }
    }	
  }
  file <<endl<<"*** End Device Configuration Space ***\n"<<endl;
  return SUCCESS;
} // END read_cfg_regs


//--- rdwr_cfg_reg(): Reads or Writes an Endpoint Configuration register (Not currently used)
//--- Arguments: int device file, int register to read or write, int wr_en
//--- Return Value: Returns int FAILURE or SUCCESS for writes or register value for read.  
//--- Detailed Description: This module does the following:
//---                       1) Reads or Writes a Endpoint Configuration register
int cfg_t::cfg_rdwr_reg(int g_devFile,int reg, int reg_value, int wr_en){
  typedef struct cfgwrite {
    int reg;
    int value;
  } cfgwr;
    cfgwr configwr;
  
  //assign the register to read solely in read case
  int read_reg = reg;

  // Write Configuration register
  if (wr_en == 1) {
      
    // Set up CFG.WRITE struct
    configwr.reg = reg;
    configwr.value = reg_value;

    // Due to argument limitations with IOCTL, have to pass in a struct for writes which contains the write value
    // and also the write register offset.  Writing a configuration register may have adverse side affects and could
    // bring down the system so this operation must be used carefully.
    if (ioctl(g_devFile, WRCFGREG, &configwr) < 0) {
      return CRIT_ERR;
    } else {

       // Read EP Type 0 CFG Register and verify value written matches value read
       if (ioctl(g_devFile, RDCFGREG, &read_reg) < 0) {
         // Read of EP CFG register failed. Return CRIT_ERR.       
         return CRIT_ERR;
       } else if (read_reg != reg_value) {
          // Read value did not match what was written.  return CRIT_ERR
          return CRIT_ERR;
       } else {
         // Read value matches so return success
         return SUCCESS;
       }
    }

  // Read Configuration Register
  } else {
     // Read EP Type 0 CFG Register
     if (ioctl(g_devFile, RDCFGREG, &read_reg) < 0) {
       // Read was not successful. Return CRIT_ERR
       return CRIT_ERR;
    } else {
      // Successful read.  Return Register value
      return read_reg;
    }
  }
} // END rdwr_cfg_reg


//--- enable_functionality(): Enables specific functionality within EP configuration space
//--- Arguments: int device file, int dev_ctrl_phantom_func_en, int dev_ctrl_extended_tag_en, int error_reporting_en
//--- Return Value: Returns int FAILURE or SUCCESS for writes or register value for read.  
//--- Detailed Description: This module does the following:
//---                       1) Reads or Writes a Endpoint Configuration register
int cfg_t::cfg_enable_functionality(int g_devFile, int dev_ctrl_phantom_func_en, int dev_ctrl_extended_tag_en, 
                                int error_reporting_en){
  typedef struct cfgwrite {
    int reg;
    int value;
  }cfgwr;
   cfgwr configwr;

  // Read Device Control/Status Register First to get current value
  int reg = this->device_stat_cont_offset;
  int new_device_status_reg = 0;
  if (ioctl(g_devFile, RDCFGREG, &reg) < 0) {
    // failure reading current contents of EP device control register
    return CRIT_ERR;
  } else {
    // All functionality currently allowed is enabled in Device Control Register.  OR all values together with
    // current value
    new_device_status_reg = (reg | error_reporting_en | dev_ctrl_phantom_func_en | dev_ctrl_extended_tag_en);	
  }

  // Now write Device Control Register with bits[19:16] = 0xF
  configwr.reg = this->device_stat_cont_offset;
  configwr.value = new_device_status_reg;
  if (ioctl(g_devFile, WRCFGREG, &configwr) < 0) {
    // Write of EP device control register failed. Return CRIT_ERR.
    return CRIT_ERR;
  } else {
    // Read EP device control register and verify value written matches value read
    reg = this->device_stat_cont_offset;
    if (ioctl(g_devFile, RDCFGREG, &reg) < 0) {
      // Read of EP device control register failed. Return CRIT_ERR.
      return CRIT_ERR;
    } else if (configwr.value != reg) {
      // Read value did not match what was written.  return CRIT_ERR
      return CRIT_ERR;
    } else {
      // Write success. 
      return SUCCESS;
    }
  } 
}

//--- cfg_update_regs(): Updates CFG Registers with latest values
//--- Arguments: int device file,
//--- Return Value: Returns int FAILURE or SUCCESS  
//--- Detailed Description: This module does the following:
//---                       1) Updates CFG Member variables with CFG values
int cfg_t::cfg_update_regs(int g_devFile)
{
  int reg_value = 0;
 
  // Getting device PM Capabilites 
  reg_value = this->pm_offset;
  if (ioctl(g_devFile, RDCFGREG, &reg_value) < 0) {
    this->cfg_fatal_text = "CFG Read Failed - update_regs";
    return CRIT_ERR;
  } else {
     this->pm_capabilities  = (reg_value & 0xFFFF0000) >> 16;
  }

  // Getting device PM Status/Control 
  reg_value = this->pm_offset+0x00000004;
  if (ioctl(g_devFile, RDCFGREG, &reg_value) < 0) {
    this->cfg_fatal_text = "CFG Read Failed - update_regs";
    return CRIT_ERR;
  } else {
     this->pm_stat_control  = (reg_value & 0x0000FFFF);
  }

  // Getting device MSI Control 
  reg_value = this->msi_offset;
  if (ioctl(g_devFile, RDCFGREG, &reg_value) < 0) {
    this->cfg_fatal_text = "CFG Read Failed - update_regs";
    return CRIT_ERR;
  } else {
     this->msi_control  = (reg_value & 0xFFFF0000)>> 16;
  }

  // Getting device Link Width Capability
  reg_value = this->link_cap_offset;
  if (ioctl(g_devFile, RDCFGREG, &reg_value) < 0) {
    this->cfg_fatal_text = "CFG Read Failed - update_regs";
    return CRIT_ERR;
  } else {
     this->link_width_cap  = (reg_value & 0x000003F0)>> 4;
  }

  // Getting device Link Speed Capability
  reg_value = this->link_cap_offset;
  if (ioctl(g_devFile, RDCFGREG, &reg_value) < 0) {
    this->cfg_fatal_text = "CFG Read Failed - update_regs";
    return CRIT_ERR;
  } else {
     this->link_speed_cap  = (reg_value & 0x0000000F);
  }

  // Getting current device Link control 
  reg_value = this->link_stat_cont_offset;
  if (ioctl(g_devFile, RDCFGREG, &reg_value) < 0) {
    this->cfg_fatal_text = "CFG Read Failed - update_regs";
    return CRIT_ERR;
  } else {
     this->link_control = (reg_value & 0x0000FFFF);
  }

  // Getting current device link width
  reg_value = this->link_stat_cont_offset;
  if (ioctl(g_devFile, RDCFGREG, &reg_value) < 0) {
    this->cfg_fatal_text = "CFG Read Failed - update_regs";
    return CRIT_ERR;
  } else {
     this->link_width = (reg_value & 0x03F00000) >> 20;
  }

  // Getting current device Link Speed 
  reg_value = this->link_stat_cont_offset;
  if (ioctl(g_devFile, RDCFGREG, &reg_value) < 0) {
    this->cfg_fatal_text = "CFG Read Failed - update_regs";
    return CRIT_ERR;
  } else {
     this->link_speed = (reg_value & 0x000F0000) >> 16;
  }

return SUCCESS;
}


//--- cfg_get_capabilities(): Gets the capability offsets
//--- Arguments: int device file,
//--- Return Value: Returns int FAILURE or SUCCESS  
//--- Detailed Description: This module does the following:
//---                       1) Polls the device configuration registers and 
//---                          gets the capability offsets
int cfg_t::cfg_get_capabilities(int g_devFile)
{
  // Initialize temporary variables
  int next_cap_offset = 0x00000034;        //Holds next cap offset
  int curr_cap_offset = 0x00000000;        //Holds current cap offset
  int cap_id          = 0x00000000;        //Holds current cap ID
  int end_cap_struct  = 0;                  //Determines end of Linked List

  // Probe the Initial linked list pointer at 34H
  if (ioctl(g_devFile, RDCFGREG, &next_cap_offset) < 0) {
    this->cfg_fatal_text = "CFG Read Failed - get_capabilities";
    return CRIT_ERR;
  } else {
     next_cap_offset = next_cap_offset & 0x000000FF;
  }

  // Traverse link list and identify capabilities until end of list
  while (end_cap_struct == 0) {
    curr_cap_offset = next_cap_offset;
    if (ioctl(g_devFile, RDCFGREG, &next_cap_offset) < 0) {
      this->cfg_fatal_text = "CFG Read Failed - get_capabilities";
      return CRIT_ERR;
    } else {    
     cap_id          = next_cap_offset & 0x000000FF;
     next_cap_offset = (next_cap_offset & 0x0000FF00) >> 8;
    }
    
    switch (cap_id) {		
      case 0x00000001: // Power Management Capability Structure
        this->pm_offset = curr_cap_offset;
        break;
      case 0x00000005: // MSI Capability Structure
        this->msi_offset = curr_cap_offset;
        break;
      case 0x00000010: // PCI Express Capability Structure
        this->pcie_cap_offset          = curr_cap_offset;
        this->device_cap_offset        = curr_cap_offset + 0x00000004;
        this->device_stat_cont_offset  = curr_cap_offset + 0x00000008;
        this->link_cap_offset          = curr_cap_offset + 0x0000000c;
        this->link_stat_cont_offset    = curr_cap_offset + 0x00000010;
        break;
      default: {  
      this->cfg_fatal_text = "Capability Read is not valid";
      return CRIT_ERR;
      }

    }
    if (next_cap_offset == 0) {
      end_cap_struct = 1;
    }
  }
  return SUCCESS;
}

//--- check_link_width(): Gets the link width
//--- Arguments: int device file,
//--- Return Value: Returns int FAILURE or SUCCESS  
//--- Detailed Description: This module does the following:
//---                       1) Gets the negotiated link width inside Link Status Register
//---                       2) Checks the negotiated link width against capabilities of EP
//---                       3) Returns success if it matches, failure if there is a mismatch
//---                       4) Updates cfg member variables with negotiated link width
int cfg_t::cfg_check_link_width(int g_devFile){

  if (this->link_width_cap != this->link_width) {
    sprintf(this->link_stat_neg_link_width_c,"%d",this->link_width);
    this->cfg_fatal_text = "Neg. Link Width != EP Link Width Capability";
    return CRIT_ERR;
  } else {
     sprintf(this->link_stat_neg_link_width_c,"%d",this
->link_width);
     return SUCCESS;

  }
} //END check_link_width

//--- check_link_speed(): Gets the link speed
//--- Arguments: int device file,
//--- Return Value: Returns int FAILURE or SUCCESS  
//--- Detailed Description: This module does the following:
//---                       1) Gets the negotiated link speed inside Link Status Register
//---                       2) Checks the negotiated link speed against capabilities of EP
//---                       3) Returns success if it matches, failure if there is a mismatch
//---                       4) Updates cfg member variables with negotiated link speed
int cfg_t::cfg_check_link_speed(int g_devFile){
	
  if (this->link_speed_cap != link_speed) {
    sprintf(this->link_stat_neg_link_speed_c,"%d",this->link_speed);
    this->cfg_fatal_text = "Neg. Link Speed != EP Link Speed Capability";
    return CRIT_ERR;
  } else {
    sprintf(this->link_stat_neg_link_speed_c,"%d",this->link_speed);
    return SUCCESS;
  }
} //END check_link_speed
