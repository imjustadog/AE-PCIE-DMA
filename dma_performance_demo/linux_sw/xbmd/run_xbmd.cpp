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
//-- Filename: xbmd_app.cpp
//-- Author: Jake Wiltgen
//--
//-- Sample test application for the XBMD device driver: 
//-- Description: This application does the following
//-- 2) Randomly selects TLP Size, TLP Count, TLP Pattern, Transfer Mode, and Phantom Function usage
//-- 3) Reads and Writes XBMD Descriptor registers to set up transfer
//-- 4) After DMA transfer, checks data written and read from the Kernel buffer for errors
//-- 6) All results are recorded into log files (prevlog.txt, log.txt, and errorlog.txt)
//--                           
//--
//--------------------------------------------------------------------------------

#include <sys/ioctl.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include "xbmd.h"
#include "MersenneTwister.h"

using namespace std;

int WriteData(int g_devFile, char* buff, int size)
{
        int ret = write(g_devFile, buff, size);
                                                                                
        return (ret);
}

int ReadData(int g_devFile, char* buff, int size)
{
        int ret = read(g_devFile, buff, size);

        return (ret);
}

struct TransferData  {

        unsigned int data[BUF_SIZE/4];

} *gReadData, *gWriteData;


int run_xbmd(int g_devFile, unsigned int rd_enable, unsigned int wr_enable, int wr_tlp_size, int rd_tlp_size, int num_wr_tlps, int num_rd_tlps, int random, const char* rd_pattern, const char* wr_pattern, bool phantom_enable, bool trans_streaming)
{

  // Variable Declaration
  
  //loop variables
  int j, l;
  unsigned int data;
  unsigned int mode;
  
  // BMD Register Variables
  unsigned int dmacr_reg = 0x00000000; // Temp BMD Stat/Control Register variable

  // Device configuration capabilities/Status variables
  unsigned int dcontrol_reg = 0x00000000;  //Holds DLTRSSTAT BMD register value

  // Error checking variables
  int rd_dma_error = 0x00000000;
	
  // Count, size, and pattern variables
  int tlpsize[11];
  int tlpsizemax = 0;
  unsigned int mps = 32;
  unsigned int reg_value = 0;
  unsigned int wrwdmatlpc=0;
  unsigned int wrwdmatlps=0;
  unsigned int wrrdmatlpc=0;
  unsigned int wrrdmatlps=0;
  unsigned int wrrdmatlpp=0;
  unsigned int wrwdmatlpp=0;
  unsigned int dmisccont = 0;

  unsigned int rd_phantom_en = 0;

  int modulo4096 = 1; // used to form modulo 4095 array          
  int count100000 = 0;  //used to change logs out.  After 10,000 iterations, record into old log and start new one. Prevents large log files 


  // FILE IO
  ofstream file;

// Random Number Variable Declaration
static MTRand *mtrandPtrRdsize = NULL;
static unsigned int seedRdsize = 1; 

static MTRand *mtrandPtrWrsize = NULL;
static unsigned int seedWrsize = 10; 

static MTRand *mtrandPtrRdcount = NULL;
static unsigned int seedRdcount = 1; 

static MTRand *mtrandPtrWrcount = NULL;
static unsigned int seedWrcount = 10; 

static MTRand *mtrandMode = NULL;
static unsigned int seedMode = 1; 

static MTRand *mtrandPattern = NULL;
static unsigned int seedPattern = 1; 

static MTRand *mtrandWrPhantom = NULL;
static unsigned int seedWrPhantom = 1; 

static MTRand *mtrandRdPhantom = NULL;
static unsigned int seedRdPhantom = 1; 

static MTRand *mtrandTransStreaming = NULL;
static unsigned int seedTransStreaming = 1; 

  // Setting Random Number Gen seeds
  seedPattern = 1;
  seedMode=2; 
  seedRdsize=3;
  seedWrsize=4;
  seedRdcount=5;
  seedWrcount=6;
  seedRdPhantom=7;
  seedWrPhantom=8;
  seedTransStreaming = 9;

  // Initialize MTRand pointers
  unsigned long testseed=time(NULL);
  file << "SEED VALUE : "<<testseed<<endl;
  mtrandPtrWrsize = new MTRand();
  mtrandPtrRdsize = new MTRand();
  mtrandPtrWrcount = new MTRand();
  mtrandPtrRdcount = new MTRand();
  mtrandMode  = new MTRand();
  mtrandPattern = new MTRand();
  mtrandWrPhantom = new MTRand();
  mtrandRdPhantom = new MTRand();
  mtrandTransStreaming = new MTRand();

     //////////////////////////////////////
    // ***** Open/setup log files ***** //
   //////////////////////////////////////

// Probably have to use append
 // file.open ("xbmd_app_log.txt");
 // if (!file.is_open()) {
 //    printf("Error opening log file");
 //    return CRIT_ERR;
 // } else {
//        printf("\nXBMD_APP: STARTING DMA TRANSFER\n\n");
//	file << "\n\nXBMD_APP: RUNNING XBMD DMA TRANSFER\n\n";
//	file << "Transfer Details:\n\n";  
//        file << "1) Number of iterations performed = " << iter_count << "\n";
//        file << "2) MPS setting = " << mps << " DW's\n";
//  }


  // First allocate the read and write buffers
  gReadData = (TransferData  *) malloc(sizeof(struct TransferData));	
  gWriteData = (TransferData  *) malloc(sizeof(struct TransferData));	

     //////////////////////////////////////////////////////////////////
    // ***** Get Maximum TLP Size from Device Control Register***** //
   //////////////////////////////////////////////////////////////////

      // Reading device control register to get maximum TLP paylod size (104D = 70H)
      reg_value = DEVICE_CONTROL_STAT_REG_OFFSET;
      cout << "DEVCONTROLREG = " <<hex<< reg_value << endl;
      if (ioctl(g_devFile, RDCFGREG, &reg_value) < 0) {
         printf("Device Control Register 0x40 Read Failed\n");
         file << "Device Control Register 0x40 Read failed" << endl;
         return CRIT_ERR;
      } else {
         dcontrol_reg = (reg_value & 0x000000E0) >> 5; //masking off capability bits
         mps = dcontrol_reg;  
         cout << "NEW MPS = " << mps << endl;    
      }		      
      switch (dcontrol_reg) {
			
         case 0: // 128 MPS
               tlpsizemax = 5;
               break;
         case 1:  // 256 MPS

               tlpsizemax = 6; 
               break;				
         case 2:  // 512 MPS
               tlpsizemax = 7;
               break;

         case 3:  // 1024 MPS 
               tlpsizemax = 8;
               break;	
         
	case 4: // 2048 MPS  
               tlpsizemax = 9;
               break;

         case 5: // 4096 MPS  
               tlpsizemax = 10;
               break;
		
	default: {  //others 
			file << "MPS value failure";
			return CRIT_ERR;
		}
	}

  // Loop builds the tlpsize array which will hold possible TLP SIZE values which are Modulo 4096 - Used in Mode A below
  // = TLPSIZE[1,2,4,8,16,32,64,128,256,512,1024,2048,4096]
  modulo4096 = 1;
  for (l=0; l < 11; l++) {
	tlpsize[l] = modulo4096;
	modulo4096 = modulo4096*2;
  }

     ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // ***** Start of Iterations- this loop will perform the necessary BMD Register setup and error checking for all DMA transfers ***** //
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*  for (i = 0; i < iter_count; i++)
  {
	int mode;

	// check for 10,000 iterations now and then start new log.  Will bump to 100,000 if more info before failure is needed
	if (count100000 == 10000 ) {
		file.close();
		remove("prevlog.txt");
		rename ("log.txt", "prevlog.txt");
	 	// FILE IO: Opening log and error_log file to record results.  Also verify file is open before proceeding.
  		file.open ("log.txt");
  		if (!file.is_open()) {
			printf("Error opening error log file");
			return CRIT_ERR;
  		}
		
		count100000 = 0;
	}
	else {
   		count100000 = count100000 + 1;
	}

	file << "--------------------------------- XBMD App Iter Count = " <<dec<< i <<" ---------------------------------\n";
        printf ("XBMD App Iter_count= %d\n", i);
*/
     ////////////////////////////////////////////////////////////////////////////////////////////
    // ***** Generate random values for TLP count, TLP size, TLP Pattern, Transfer Mode ***** //
   ////////////////////////////////////////////////////////////////////////////////////////////

	// Getting Random Mode (A or B)
	mode = mtrandMode->randInt(1);

	if (random == 1) {

		// Random Scenario - Determine TLP SIZE and COUNT based on mode selected by Mersenne twister random number gen.
  		if (mode == 0) {
			//Mode A is larger transfers (multiple pages) which do not cross page a page boundary (i.e. Modulo 4096 SIZE)
      			wrwdmatlps = tlpsize[mtrandPtrWrsize->randInt(tlpsizemax)];
			wrwdmatlpc = (65536/wrwdmatlps);  		
			wrrdmatlps = tlpsize[mtrandPtrRdsize->randInt(tlpsizemax)];			
			wrrdmatlpc = (65536/wrrdmatlps);
		} else {

			// Mode B initiates smaller transfers (< 1 page) where no page boundary will be crossed.  COUNT and SIZE smaller
      			wrwdmatlps = (1 + mtrandPtrWrsize->randInt(mps-1));  
      			wrwdmatlpc = (1024/wrwdmatlps);  
      			wrrdmatlps = 1 + mtrandPtrRdsize->randInt(mps-1);  
      			wrrdmatlpc = (1024/wrrdmatlps);  
		}

	// Getting Random Pattern
	wrwdmatlpp = mtrandPattern->randInt(0xFFFFFFFF);
        wrrdmatlpp = mtrandPattern->randInt(0xFFFFFFFF);

	} else {
           // Static Scenario -Sets TLP size/count to values set in GUI
           wrwdmatlps = wr_tlp_size;  
           wrwdmatlpc = num_wr_tlps;  
           wrrdmatlps = rd_tlp_size;  
           wrrdmatlpc = num_rd_tlps;  
           wrrdmatlpp = strtoul(rd_pattern, NULL, 16);
           wrwdmatlpp = strtoul(wr_pattern, NULL, 16);

	}

       cout << "wr_tlp_size="<< hex<<   wr_tlp_size<< endl;
       cout << "rd_tlp_size="<< hex<<   rd_tlp_size<< endl;
       cout << "wr_tlp_count="<< hex<<   num_wr_tlps<< endl;
       cout << "rd_tlp_count="<<  hex<<  num_rd_tlps<< endl;

       cout << "wr_tlp_size="<< hex<<   wrwdmatlps<< endl;
       cout << "rd_tlp_size="<< hex<<   wrwdmatlps<< endl;
       cout << "wr_tlp_count="<< hex<<   wrwdmatlpc<< endl;
       cout << "rd_tlp_count="<<  hex<<  wrwdmatlpc<< endl;
       cout << "wr_tlp_pattern="<< hex<<  wrwdmatlpp<< endl;
       cout << "rd_tlp_pattern="<< hex<< wrrdmatlpp<< endl;

//      Transmit Streaming Randomization
        if (trans_streaming == true) {
           dmisccont = (mtrandTransStreaming->randInt(1)) << 9;
           if (dmisccont = 0x00000020) {
              file << "Transmit Streaming Enabled\n"<<endl;
           }
              dmisccont = dmisccont | 0x01010000;
        } else {
           dmisccont = 0x01010000;
        }

        // Phantom Function Randomization (Write Phantom not functional yet; Possible Chipset confliction) 
	if (phantom_enable == true) {
           rd_phantom_en = mtrandRdPhantom->randInt(1);
	   rd_phantom_en = rd_phantom_en << 20;
	   wrrdmatlps = (wrrdmatlps | rd_phantom_en);
        }
    // Initalize the DMA target buffer - Pattern data is data returned on read completions and must match WRRDMATLPP BMD descriptor register
    for (j = 0; j < BUF_SIZE/4; j++)
    {
	gWriteData->data[j]= wrrdmatlpp;  
    }

    WriteData(g_devFile, (char*) gWriteData, BUF_SIZE);

     //////////////////////////////////////////////////////////////////////////////////////////
    // ***** START OF SETTING UP DMA TRANSFER BY WRITING TO XBMD DESCRIPTOR REGISTERS ***** //
   //////////////////////////////////////////////////////////////////////////////////////////

    // Reset Initiator Descriptor Registers
    if (ioctl(g_devFile, INITRST, 0x00000000) < 0) {
      	file << "INITRST Failed\n";
      	return CRIT_ERR;
    }
	
	// Reads BMD control register
    if (ioctl(g_devFile, RDDDMACR, &data) < 0) {
      	file << "RDDDMACR Read Failed\n";
	return CRIT_ERR;
    }
    else {
        dmacr_reg = data;
	#ifdef Verbose
      	file << "BMD Descriptor Registers: \nRDDDMACR Value = " <<hex<< data<<"\n";
	#endif
     }
	// Writes BMD TLP count register indicating # of WRITE TLP's to send from card
    if (ioctl(g_devFile, WRWDMATLPC, wrwdmatlpc) < 0) {
     	file << "WRWDMATLPC Write Failed\n";
	file.close();
	return CRIT_ERR;
    }

	// Writes TLP size register indicating payload of each WRITE TLP sent from card 
    if (ioctl(g_devFile, WRWDMATLPS, wrwdmatlps) < 0) {
      	file << "WRWDMATLPS Write Failed\n";
	file.close();
	return CRIT_ERR;
    }

	// Writes the TLP count indicating number of READ TLP's sent from card
    if (ioctl(g_devFile, WRRDMATLPC, wrrdmatlpc) < 0) {
      	file << "WRWDMATLPC Write Failed\n";
	file.close();
	return CRIT_ERR;
    }

	// Writes the TLP size register indicating requested payload seen in each READ TLP sent from card.
    if (ioctl(g_devFile, WRRDMATLPS, wrrdmatlps) < 0) {
      	file << "WRRDMATLPS Write Failed\n";
	file.close();
	return CRIT_ERR;
    }

  	// this IOCTL call sets the pattern for writes from BMD to system memory and the checker module assumes it will check against this.
    if (ioctl(g_devFile, WRWDMATLPP, wrwdmatlpp) < 0) {  //uncomment to get random data pattern
      	file << "WRRDMATLPP Write Failed\n";
	file.close();
	return CRIT_ERR;
   }

  	// this IOCTL call sets the pattern for writes from BMD to system memory and the checker module assumes it will check against this.
   if (ioctl(g_devFile, WRRDMATLPP, wrrdmatlpp) < 0) {    	
	file << "WRRDMATLPP Write Failed\n";
	file.close();
	return CRIT_ERR;
   }

    // Reads WR_DMA TLP COUNT REGISTER
    if (ioctl(g_devFile, RDWDMATLPC, &data) < 0) {
      	file << "RDWDMATLPC Read Failed\n";
	file.close();
	return CRIT_ERR;
    }
    else { 
	#ifdef Verbose
      	file << "RDWDMATLPC Value = "<<hex<<data<<"\n";
	#endif
     }

    // Reads WR_DMA TLP SIZE REGISTER
    if (ioctl(g_devFile, RDWDMATLPS, &data) < 0) {
      	file << "RDWDMATLPS Read Failed\n";
	file.close();
	return CRIT_ERR;
    }
    else {
	#ifdef Verbose
      file <<"RDWDMATLPS Value = "<<hex<<data<<"\n";
	#endif
    }
    
    // Reads RD_DMA TLP Count Register
    if (ioctl(g_devFile, RDRDMATLPC, &data) < 0) {
      	file << "RDRDMATLPC Read Failed\n";
	file.close();
	return CRIT_ERR;
    }
    else {
	#ifdef Verbose
      	file << "RDRDMATLPC Value = "<<hex<<data<<"\n";
	#endif
    }
    // Reads RD_DMA TLP SIZE REGISTER
    if (ioctl(g_devFile, RDRDMATLPS, &data) < 0) {
      	file << "RDRDMATLPS Read Failed\n";
	file.close();
	return CRIT_ERR;
    }
    else {
	#ifdef Verbose
      	file << "RDRDMATLPS Value = "<<hex<<data<<"\n";
	#endif
    }

    // Reads WR_DMA TLP PATTERN REGISTER
    if (ioctl(g_devFile, RDWDMATLPP, &data) < 0) {
     	file << "RDWDMATLPP Read Failed\n";
	file.close();
	return CRIT_ERR;
    }
    else {
	//cout << "RDWDMATLPP Value = "<< data << "\n";
	#ifdef Verbose
      	file <<"RDWDMATLPP Value = "<<hex<<data<<"\n";
	#endif
    }

    // Reads RD_DMA TLP PATTERN REGISTER
    if (ioctl(g_devFile, RDRDMATLPP, &data) < 0) {
      	printf("RDRDMATLPP Read Failed\n");
	file.close();
      	return CRIT_ERR;
    }
    else {
	//cout << "RDRDMATLPP Value = "<< data << "\n";
	#ifdef Verbose
      	file <<"RDRDMATLPP Value = "<<hex<<data<<"\n\n";
	#endif
    }

    // WRR Control For Soft Core (no cpl streaming on) WRR 1:1
    if (ioctl(g_devFile, WRDMISCCONT, dmisccont) < 0) {
      	file << "WRDMISCCONT Write Failed\n";
	file.close();
	return CRIT_ERR;
    }
 
    // Initiator Start - Writes the Control Status Descriptor Register (DDMACR)
    dmacr_reg = dmacr_reg | rd_enable | wr_enable;
     cout << "DMACR = "<<dmacr_reg<<endl;
    if (ioctl(g_devFile, WRDDMACR, dmacr_reg) < 0 ) {
      	file << "INITSTART Write Failed\n";
	file.close();
  	file.close();
	return CRIT_ERR;
    }
    
     /////////////////////////////////////////////////////////////////////////////////////////////
    // ***** BMD DESCRIPTOR REGISTERS NOW SET UP...WAIT FOR TRANSFER BEFORE CHECING DATA ***** //
   /////////////////////////////////////////////////////////////////////////////////////////////    
    usleep (6000); 	//This wait is required to make sure all data is transfered before checking integrity. 
 
	
      /////////////////////////////////////////////////////////////////////////////////
     // *****                         ERROR CHECKING                         ****** //
    // ***** CHECK ENTIRE KERNEL BUFFER AND XBMD CONTROL REGISTER FOR ERRORS ***** //
   /////////////////////////////////////////////////////////////////////////////////

   // This for loop module zeroes out the data that was previously written from the EP device.  
   for (j = 0; j < BUF_SIZE/4; j++) {
     	gReadData->data[j]= 0x00000000;
   }

   // Read data from Kernel Buffer to User Buffer...	
   ReadData(g_devFile, (char *) gReadData, (wrwdmatlps * wrwdmatlpc * 4));
    
   // Now check to see if it is what we expect..
   for (j = 0; j < (wrwdmatlps * wrwdmatlpc); j++) {
      if (gReadData->data[j] != wrwdmatlpp) { 
         file << "*** Write DMA Error (EP -> Chipset) ***\n Details: Data Error Found when comparing data written to system memory with expected pattern: Write DMA \n TLP Size = " << wrwdmatlps << "\n TLP Count = "<< wrwdmatlpc << "\n";
         file << hex<< gReadData->data[j] << "(Data Buffer)" << " != "<< hex<< wrwdmatlpp << "(Expected)\n\n"; 
         file.close();
         return CRIT_ERR;
      }
   }
 
    // BMD responsible for error checking on read completions.  Records errors in DMACR Register which the module below reads
    if (ioctl(g_devFile, RDDDMACR, &data) < 0) {
      	file << "RDDDMACR Read Failed\n";
    }
    else {
        dmacr_reg = data;  
	if (dmacr_reg != 0x1010101) { 
	file << "*** Read DMA Error ***\n Details: Data Error Found by BMD backend and recorded in DDMACR register \n TLP Size = " << wrwdmatlps << "\n TLP Count = "<< wrwdmatlpc <<"\n";	
	file << "DMACR register contents = 32'h" << hex << dmacr_reg << "\n";
	printf("Error found on DMA Read.  Please see error_log.txt file for details.\n");
	file.close();
        return CRIT_ERR;    
	}
    }

     // Checking Device Status for FATAL error and returning if one is seen.  Change value of 4 to check for other errors (Bit 0 = Correctable, BIt 1 = Non Fatal, Bit 3 = UR)
     reg_value = DEVICE_CONTROL_STAT_REG_OFFSET;
     if (ioctl(g_devFile, RDCFGREG, &reg_value) < 0) {
        printf("Device Status Read Failed\n");
        file <<"Device Status Read failed"<<endl;
        return CRIT_ERR;
     } else {
        // Mask off Fatal Error bit
        if (((reg_value & 0x00040000)>>16) == 0x00000004) {
        printf("CRITICAL ERROR REPORTED BY EP DEVICE"); 	
        return CRIT_ERR;
        }
     }
  

  //cout << "\nXBMD_APP: All iterations completed.  Refer to 'log.txt' file for results\n";
  //file << "\n XBMD_APP: No errors found in test.\n";

  //close out pointers and open files
  file.close();
  delete gReadData;
  delete gWriteData;
  delete mtrandPtrWrsize;
  delete mtrandPtrRdsize;
  delete mtrandPtrWrcount;
  delete mtrandPtrRdcount;
  delete mtrandMode;
  delete mtrandPattern;
  delete mtrandRdPhantom;
  delete mtrandWrPhantom;
}
