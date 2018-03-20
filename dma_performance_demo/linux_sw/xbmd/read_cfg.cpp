//--------------------------------------------------------------------------------
//--
//-- This file is owned and controlled by Xilinx and must be used solely
//-- for design, simulation, implementation and creation of design files
//-- limited to Xilinx devices or technologies. Use with non-Xilinx
//-- devices or technologies is expressly prohibited and immediately
//-- terminates your license.
//--
//-- Xilinx products are not intended for use in life support
//-- appliances, devices, or systems. Use in such applications is
//-- expressly prohibited.
//--
//--            **************************************
//--            ** Copyright (C) 2005, Xilinx, Inc. **
//--            ** All Rights Reserved.             **
//--            **************************************
//--
//--------------------------------------------------------------------------------
//-- Filename: read_cfg.cpp
//-- Author: Jake Wiltgen
//--
//-- Sample test application for the XBMD device driver: 
//-- Description: This application does the following
//-- 1) This module does setup functions.  
//-- 2) Currently is does the following tasks
//--    a) Output Configuration space (Offsets 0x0 - 0x120) to log file and console
//--                           
//--
//--------------------------------------------------------------------------------

#include <sys/ioctl.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include "xbmd.h"

using namespace std;

  // FILE IO
  ofstream file;

int read_cfg(int g_devFile)
{
  int i;
  unsigned int reg_value = 0;

     //////////////////////////////////////
    // ***** Open/setup log files ***** //
   //////////////////////////////////////

  file.open ("cfg_space.txt");
  if (!file.is_open()) {
	printf("Error initializing CFG report");
	return CRIT_ERR;
  }

     /////////////////////////////////////////////////////////////////////////////////
    // ***** Poll Type0 Configuration Space and output to console and log.txt***** //
   /////////////////////////////////////////////////////////////////////////////////
   file <<endl<<"*** EP Device Type 0 Configuration Space ***\n"<<endl;
   file << "Offset    Value\n"<<endl;
   for (i = 0; i <0x120;i=i+4) {
      reg_value = i;
      if (ioctl(g_devFile, RDCFGREG, &reg_value) < 0) {
         printf("CFG Register 0x%x Read Failed\n",&reg_value);
         return CRIT_ERR;
      } else {
         if (i < 0xFF) { 
         file <<"0x"<<hex<<i<<"       "<<reg_value<<endl;  
         }
      }		
   }
   file <<endl<<"*** End Device Configuration Space ***\n"<<endl;
   return SUCCESS;
}
