*************************************************************************
   ____  ____ 
  /   /\/   / 
 /___/  \  /   
 \   \   \/    © Copyright 2015 Xilinx, Inc. All rights reserved.
  \   \        This file contains confidential and proprietary 
  /   /        information of Xilinx, Inc. and is protected under U.S. 
 /___/   /\    and international copyright and other intellectual 
 \   \  /  \   property laws. 
  \___\/\___\ 
 
*************************************************************************

Vendor: Xilinx 
Current readme.txt Version: 1.0.0
Date Last Modified:  21APR2015
Date Created: 21APR2015

Associated Filename: xapp1052.zip
Associated Document: XAPP1052.PDF - Bus Master Performance Demonstration
Reference Design for the Xilinx Endpoint
PCI Express Solutions

Supported Device(s): Kintex-7 FPGAs, Virtex-6 FPGAs, Spartan-6 FPGAs 
   
*************************************************************************

Disclaimer: 

      This disclaimer is not a license and does not grant any rights to 
      the materials distributed herewith. Except as otherwise provided in 
      a valid license issued to you by Xilinx, and to the maximum extent 
      permitted by applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE 
      "AS IS" AND WITH ALL FAULTS, AND XILINX HEREBY DISCLAIMS ALL 
      WARRANTIES AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, 
      INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, 
      NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and 
      (2) Xilinx shall not be liable (whether in contract or tort, 
      including negligence, or under any other theory of liability) for 
      any loss or damage of any kind or nature related to, arising under 
      or in connection with these materials, including for any direct, or 
      any indirect, special, incidental, or consequential loss or damage 
      (including loss of data, profits, goodwill, or any type of loss or 
      damage suffered as a result of any action brought by a third party) 
      even if such damage or loss was reasonably foreseeable or Xilinx 
      had been advised of the possibility of the same.

Critical Applications:

      Xilinx products are not designed or intended to be fail-safe, or 
      for use in any application requiring fail-safe performance, such as 
      life-support or safety devices or systems, Class III medical 
      devices, nuclear facilities, applications related to the deployment 
      of airbags, or any other applications that could lead to death, 
      personal injury, or severe property or environmental damage 
      (individually and collectively, "Critical Applications"). Customer 
      assumes the sole risk and liability of any use of Xilinx products 
      in Critical Applications, subject only to applicable laws and 
      regulations governing limitations on product liability.

THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS 
FILE AT ALL TIMES.

*************************************************************************

This readme file contains these sections:

1. REVISION HISTORY
2. OVERVIEW
3. SOFTWARE TOOLS AND SYSTEM REQUIREMENTS
4. DESIGN FILE HIERARCHY
5. INSTALLATION AND OPERATING INSTRUCTIONS
6. OTHER INFORMATION (OPTIONAL)
7. SUPPORT


1. REVISION HISTORY 

Added support for Kintex-7.

            Readme  
Date        Version      Revision Description
=========================================================================
21APR2015   1.0.0        Initial Xilinx release.
=========================================================================



2. OVERVIEW

Please read XAPP1052 to undersand how to use the files in this zip file.


3. SOFTWARE TOOLS AND SYSTEM REQUIREMENTS


4. DESIGN FILE HIERARCHY

Please see XAPP1052 for the file hierarchy of the zip file.


5. INSTALLATION AND OPERATING INSTRUCTIONS 

Please see XAPP1052 for detailed instructions on how to use the files in
this repository.


6. SUPPORT

To obtain technical support for this reference design, go to 
www.xilinx.com/support to locate answers to known issues in the Xilinx
Answers Database or to create a WebCase. 

Known Issue

Item 1: On some Windows based systems 
it has been observed that if the driver is already
loaded when the system boots up, Windows may appear to hang.
The problem appears to be a conflict between the driver
and some USB drivers. The problem is that the USB drivers
stop working and if you have the mouse or keyboard 
plugged into these USB ports they become unresponsive.
This makes it appear the system is hung, but in reality
its just the USB port that is not working. Recommendation
is to try a different USB port. We found this would
solve the issue. This has been seen on an x58 and P55
machine.

Item 2: The x8 implementation for the Block Plus core
On ML555 has difficulty meeting timing. It may be
Necessary to use SmartXplorer to close timing. If using
The v1.12 or v1.11 core see Answer Record 33709 for 
new wrapper file that will help in timing closure.
http://www.xilinx.com/support/answers/33709.htm

Item 3: The Linux driver is only proven to work on
Fedora Core 10. It has not been tested on other 
Linux versions.