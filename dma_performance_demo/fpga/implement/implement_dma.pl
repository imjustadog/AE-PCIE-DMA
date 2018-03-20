#!/usr/local/bin/perl

#--------------------------------------------------------------------------------
#--
#-- This file is owned and controlled by Xilinx and must be used solely
#-- for design, simulation, implementation and creation of design files
#-- limited to Xilinx devices or technologies. Use with non-Xilinx
#-- devices or technologies is expressly prohibited and immediately
#-- terminates your license.
#--
#-- Xilinx products are not intended for use in life support
#-- appliances, devices, or systems. Use in such applications is
#-- expressly prohibited.
#--
#--            **************************************
#--            ** Copyright (C) 2005, Xilinx, Inc. **
#--            ** All Rights Reserved.             **
#--            **************************************
#--
#--------------------------------------------------------------------------------
#-- Filename: implement_dma.pl
#-- Author: Jake Wiltgen
#--
#-- This script takes user inputs to identify the development board targeted and will run through synthesis and implementation.  The end result will
#-- will be a .bit file located in the results directory which can be used to test the Xilinx bus master reference design in HW.                       
#--
#--------------------------------------------------------------------------------

#Main variables:
use Cwd;
use File::Find;
use File::Copy;
$dir = getcwd();
$SCRIPT_NAME = "Implement_dma";
#Operating System Specific variables used for system calls
$l = "\\";               #slash direction
$remove = "del";         #remove call
$force = "/S /Q";	       #force switch
$move = "move";          #move call

$custom == 0;             #Determines if default or custom flow is used

$pcie_core = 0;
$dev_board = 0;
$link_width = 0;
$dev_silicon = 0;
$gen_speed = "";
$silicon = "";

$prod = "null";
$board = "null";
#$lw = "null";

$NGDBUILD_OPTIONS = "-aul -verbose"; 
$MAP_OPTIONS      = "-timing -ol high -xe c -pr b"; 
$PAR_OPTIONS      = "-ol high -xe c"; 
$TRCE_OPTIONS     = "-u 20 -v 200";
$BITGEN_OPTIONS   = "-w -g DriveDone:Yes -g EN_TSTBLCLAMP:1 -g TWR_DLY_L:110 -g TWR_DLY_U:110";



#----------------------------- OPENING MESSAGE ----------------------------------#
print "\n\n\n\n";
print "******************************************************************************\n";
print "*******  Xilinx Bus Master Reference Design Implementation Script  ***********\n";
print "******************************************************************************\n";

print "INTRO: This script will prompt the user for inputs and do the following:\n";
print "  1) Cleanup old results\n";
print "  2) Run XST with BMD attached to PCI Express Solution\n";
print "  3) Run Design through Implementation tools (NGDBUILD, MAP, PAR, TRCE, BITGEN)\n";
print "  4) Place output files into 'Results' directory\n\n";

print "INFO: Supported Xilinx PCI Express solutions are:\n";
print "  1) Endpoint Block Plus for PCI Express\n";
print "  2) Endpoint PIPE for PCI Express\n";
print "  3) Spartan-6 Integrated Block for PCI Express\n";
print "  4) Virtex-6 Integrated Block for PCI Express\n\n";

print "INFO: Supported Xilinx Platforms are:\n";
print "  1) ML555 (LX50T)\n";
print "  2) Hi-Tech Global (FX70T)\n";
print "  3) Spartan 3 PCI Express Starter Kit (S31000)\n";
print "  4) SP605 (Spartan-6 Integrated Block for PCI Express)\n";
print "  5) ML605 (Virtex-6 Integrated Block for PCI Express)\n\n";

print "Press return to continue:\n";
$temp=<STDIN>; 
#--------------------------------------------------------------------------------#


######################### 
# Program Main Function #
#########################
print("\n");
print ("\nINFO: Xilinx BMD Implementation Script\n\n");
&find_operating_system;
&cleanup;
&identify_flow;
&get_pcie_solution;

if ($custom == 2) {
  &identify_xst_scr_custom;
  &identify_ucf_custom;
} else {
  &get_xil_dev_board;
  &get_link_width;
  &identify_xst_scr_default;
  &identify_ucf_default;
}
&run_synthesis;
&run_ngdbuild;
&run_map;
&run_par;
&run_trce;
&run_bitgen;
print ("\nINFO: $SCRIPT_NAME complete\n\n");

################################ 
# Program Supporting Functions #
################################
sub find_operating_system
{
  if ($^O =~ /MSWin/){
	 $l = "\\";
	 $remove = "del";
	 $force = "/S /Q";
	 $move = "move";
 	 print "\n\nINFO: Windows Operating System found\n\n";
  }elsif ($^O =~ /linux/) {
	 $l= "/";
	 $remove = "rm";
	 $force = "-rf";
	 $move = "mv";
	 print "\nINFO: Linux Operating System found\n\n";
  }else {print "\n\nCRIT_ERR: Operating System not supported"; exit;}
} #end find_operating_system

sub cleanup {
  print "\nClean up the results directory\n";
  system("$remove $force *.srp");
  system("$remove $force results");
  system("mkdir results");
} #end cleanup


#-----------------------------------------------------------------------#
#This subroutine identifies if the customer would like to use a standard
#flow or if custom implementation is desired
#-----------------------------------------------------------------------#
sub identify_flow {
  $valid_entry =0;
  while ($valid_entry == 0){
    print "\n\n***************************************************************************\n\n";
    print "Are you targetting a default Xilinx Development Platform or custom platform?\n";
    print "1) for Default\n";
    print "2) for Custom\n";
    print "Enter 1 or 2\n:";
    $custom=<STDIN>; 
    if ($custom == 1 || $custom ==2) {
      $valid_entry =1;
    }else {
      print "\n\nInvalid number!\n";
    }
  }
} #end identify_flow


#-----------------------------------------------------------------------#
#This subroutine identifies the Xilinx PCI Express solution generated
#using Xilinx CORE Generator
#-----------------------------------------------------------------------#
sub get_pcie_solution {
  $valid_entry = 0;
    while ($valid_entry == 0) {
      print "\n\nPlease enter the PCI Express solution generated using CORE Generator:\n";
      print "1)Endpoint Block Plus for PCI Express (Virtex-5)\n";
      print "2)Endpoint PIPE for Spartan3\n";
      print "3)Spartan-6 Integrated Block for PCI Express\n";
      print "4)Virtex-6 Integrated Block for PCI Express\n";
      print "Enter 1, 2, 3, or 4\n:";
      $pcie_core=<STDIN>; 
      if (($pcie_core > 0) and ($pcie_core < 5)) {
        $valid_entry = 1;
      }else {print "\n\nInvalid number!\n";}
      if ($pcie_core == 1) {
        $prod = "blk_plus";
      } elsif ($pcie_core == 2) {
        $prod = "pipe";
      } elsif ($pcie_core == 3) {
        $prod = "s6";
      } elsif ($pcie_core == 4) {
        $prod = "v6";
      }

    }
} #end get_pcie_solution	


#-----------------------------------------------------------------------#
#This subroutine identifies the Xilinx development board targeted if 
#the default mode is selected
#   $dev_board = 1 => ML555
#   $dev_board = 2 => Hi-Tech Global
#   $dev_board = 3 => Spartan3 PCI Express Starter Kit
#   $dev_board = 4 => SP605
#   $dev_board = 5 => ML605

#-----------------------------------------------------------------------#
sub get_xil_dev_board {
  $valid_entry = 0;
  while ($valid_entry == 0) {  
    print "\n\nPlease enter the Xilinx development board you are targetting:\n";
    if ($pcie_core==1){ #Block Plus
      print "1) ML555\n";
      print "2) Hi-Tech Global\n";
      print "Enter 1 or 2\n:";
      $dev_board=<STDIN>;
      if (($dev_board == 1) or ($dev_board == 2)) {
        $valid_entry = 1;
      } else {print "\n\nInvalid number!\n";}
      if ($dev_board == 1) {
        $board = "ml555";
      } elsif ($dev_board == 2) {
        $board = "htg";
      }
      
    }elsif ($pcie_core==2){ #Endpoint Pipe  
      print "1) Spartan3 PCI Express Starter Kit\n";
      print "Enter 1\n:";
      $dev_board=<STDIN>+2;
      if ($dev_board == 3){
        $board = "s3kit";
        $valid_entry=1;
      }else {print "\n\nInvalid number!\n";}
         
    }elsif ($pcie_core==3){ #Spartan-6 Integrated Block 
      print "1) SP605\n";
      print "Enter 1\n:";
      $dev_board=<STDIN>+3;
      if ($dev_board == 4){
        $board = "sp605";
        $valid_entry=1;
      }else {print "\n\nInvalid number!\n";}
         
    }elsif ($pcie_core==4){ #Virtex-6 Integrated Block
      print "\n1) ML605\nEnter 1\n:";
      $dev_board=<STDIN>+4;
      if ($dev_board == 5){
        $board = "ml605";
        $valid_entry=1;
      }else {print "\n\nInvalid number!\n";}
       
    }else  {print "Not a valid number";}  #Not a valid Xilinx PCIe solution
  } #end while
} #end get_xil_dev_board


#-----------------------------------------------------------------------#
#This subroutine identifies the Link Width of the Xilinx PCI Express 
#solution generated 
#the default mode is selected
#   $link_width = 1 => X1
#   $link_width = 4 => X4
#   $link_width = 8 => X8
#-----------------------------------------------------------------------#
sub get_link_width {
  $valid_entry = 0;
  while ($valid_entry == 0) {
    if ($dev_board==1){  # ML555 - Supported widths are X1, X4, X8
      print "\n\nPlease enter link width for the PCI Express core generated (1,4, or 8)\n:";
        $link_width=<STDIN>;
        $link_wdith = chomp($link_width);
        if (($link_width == 1) or ($link_width == 4)or ($link_width == 8)) {
          $valid_entry = 1;
        }else {print "\n\nInvalid number!\n";}
        
    }elsif ($dev_board == 2)  { #Hi-Tech Global Board X1 or X4
      print "\n\nPlease enter link width for the PCI Express core generated (1 or 4)\n:";
      $link_width=<STDIN>;
      $link_wdith = chomp($link_width);
      if (($link_width == 1) or ($link_width == 4)) {
        $valid_entry = 1;
      }else {print "\n\nInvalid number!\n";}
         
    }elsif (($dev_board == 3))  { #Spartan Starter Kit
      $link_width = 1;
      $valid_entry = 1;
        
    }elsif (($dev_board == 4))  { #SP605
      $link_width = 1;
      $valid_entry = 1;
         
    }elsif ($dev_board == 5)  { #ML605
      print "\n\nPlease enter link width and speed for the PCI Express core generated.\n";
      print "1. x1 Gen 1\n";
      print "2. x1 Gen 2\n";
      print "3. x4 Gen 1\n";
      print "4. x4 Gen 2\n";
      print "5. x8 Gen 1\n";
      print "6. x8 Gen 2\n";
      print "Enter 1, 2, 3, 4, 5, or 6\n:";
      $dev_width_speed = <STDIN>;
      
      if($dev_width_speed == 1){
         $link_width = 1;
         $gen_speed = "_gen1";
         $valid_entry = 1;
      }elsif($dev_width_speed == 2){
         $link_width = 1;
         $gen_speed = "_gen2";
         $valid_entry = 1;
      }elsif($dev_width_speed == 3){
         $link_width = 4;
         $gen_speed = "_gen1";
         $valid_entry = 1;
      }elsif($dev_width_speed == 4){
         $link_width = 4;
         $gen_speed = "_gen2";
         $valid_entry = 1;
      }elsif($dev_width_speed == 5){
         $link_width = 8;
         $gen_speed = "_gen1";
         $valid_entry = 1;
      }elsif($dev_width_speed == 6){
         $link_width = 8;
         $gen_speed = "_gen2";
         $valid_entry = 1;
      }else {print "\n\nInvalid Number!\n";
        $valid_entry = 0;
      }
    
    
     if($link_width == 8 and $gen_speed eq "_gen2"){
      }else{
       print "\n\nPlease enter your silicon type.\n";
       print "1. ES Silicon (make sure to use v1.3 the V6 PCIe core)\n";
       print "2. C Grade (this supports v1.6 of the V6 PCIe core)\n";
       print "Enter 1 or 2\n:";
       $dev_silicon = <STDIN>;
       if($dev_silicon == 1){
         $silicon = "_es";
         $valid_entry = 1;
       }elsif($dev_silicon == 2){
         $silicon = "_prod";
         $valid_entry = 1;
       }else {print "\n\nInvalid Number!\n";
         $valid_entry = 0;
         }}
     }else {print "\n\nInvalid number!\n";}     
    # }else  {print "Not a valid number";} 
  } #end while
} #get_link_width

#-----------------------------------------------------------------------#
#This subroutine identifies the XST SCR file to be used based on inputs
#provide above.  It locates the XST SCR by the link width and pcie core
#selected.
#-----------------------------------------------------------------------#
sub identify_xst_scr_default {
 if($prod eq "v6" and $link_width == 8 and $gen_speed eq "_gen2"){
    $XST_SCR ="${dir}${l}xst${l}xst_${prod}_${board}_x${link_width}${gen_speed}.scr";  
 }elsif($prod eq "blk_plus" and $link_width == 8){
    $XST_SCR ="${dir}${l}xst${l}xst_${prod}_${board}_x${link_width}.scr";     
 }elsif($prod eq "v6"){
    $XST_SCR ="${dir}${l}xst${l}xst_${prod}_${board}${silicon}.scr"
 }elsif($prod eq "blk_plus"){
    $XST_SCR ="${dir}${l}xst${l}xst_${prod}_${board}.scr";        
 }else {
    $XST_SCR ="${dir}${l}xst${l}xst_${prod}_${link_width}_lane_${board}.scr";  
 }

} # end identify_xst_scr_default

#-----------------------------------------------------------------------#
#This subroutine identifies the UCF file to be used based on inputs
#provide above.  It locates the UCF by the link width and development
#board selected.
#-----------------------------------------------------------------------#
sub identify_ucf_default {
$NGDBUILD_UCF ="${dir}${l}ucf${l}xilinx_pci_exp_${prod}_${link_width}_lane_ep_${board}${gen_speed}.ucf"; 
} #end identify_ucf_default

#-----------------------------------------------------------------------#
# Checks if a file contains SCR
#-----------------------------------------------------------------------#
sub search_xst_scr(){
  if ($_ =~ /scr$/) {
    if ($k =~ 0) {
	   print "\nPlease select the SCR file you would like to use";
      $k = $k+1;
    }
  print "\n",$j,") ", $_;
  #print "\n",$j,") ", $File::Find::name;
  $scrarray[$j] = $File::Find::name; 
  $j=$j+1;   
  }
}

#-----------------------------------------------------------------------#
#This subroutine does a recursive search of the implement directory and
#all directories below it.  It will store each relevant file name and 
#display to the console for the user to select. 
#-----------------------------------------------------------------------#
sub identify_xst_scr_custom {
  print "CUSTOM MODE ENTERED\n\n";
  print "Custom mode assumes you have created valid .SCR, .PRJ and UCF files.\n";
  print "This script does a recursive search of appropriate files in and below the implment directory.\n\n";
  print "It will display all relevant files to the screen for the user to select.";
  print "SCR and PRJ files should be placed into XST directory under implement.\n";
  print "UCF file should be placed into UCF directory under implement.\n"; 
    
  $j=0; # temp for loop	
  $k=0;
  $scr_path = "${dir}${l}xst"; 

  find(\&search_xst_scr, $scr_path);
     print "\n:"; 
     $scrtemp =<STDIN>; 
     $cust_scr = $scrarray[$scrtemp];
     $XST_SCR ="${cust_scr}";
}

#-----------------------------------------------------------------------#
# Checks if a file contains UCF
#-----------------------------------------------------------------------#
 sub search_ucf(){
   if ($_ =~ /ucf$/){
     if ($k =~ 0){
       print "\nPlease select the UCF file you would like to use";
       $k = $k+1;
     }
     print "\n",$j,") ", $_;
     #print "\n",$j,") ", $File::Find::name;
     $cdcarray[$j] = $File::Find::name; 
     $j=$j+1;   
   }
}

#-----------------------------------------------------------------------#
#This subroutine does a recursive search of the implement directory and
#all directories below it.  It will store each relevant file name and 
#display to the console for the user to select. 
#-----------------------------------------------------------------------#
sub identify_ucf_custom {
  $j=0; # temp for loop	
  $k=0;
  $ucf_path = "${dir}${l}ucf"; 
  find(\&search_ucf, $ucf_path);
     print "\n:"; 
     $ucftemp =<STDIN>; 
     $cust_ucf = $cdcarray[$ucftemp];
     $NGDBUILD_UCF ="${cust_ucf}";    
}

sub run_synthesis
{
  print ("\nINFO: Starting synthesis\n");
  print "\nINFO: FOUND  $XST_SCR\n\n";
  $status = 0;
  $status = system("xst -ifn $XST_SCR");
  if($status != 0)
  {
    print ("ERROR: Synthesis completed with errors.\n");
    print ("     : $SCRIPT_NAME Terminating!\n");
    print ("\n");
    exit 1;
  }
  system("$move xilinx_pci_exp_ep.ngc ${dir}${l}results${l}xilinx_pci_exp_ep_top.ngc");
  print ("INFO: Synthesis step complete\n");  
}

sub run_ngdbuild
{
  print ("\nINFO: Starting NGDBUILD\n");
  print ("INFO: UCF File: $NGDBUILD_UCF\n");

  $command = "ngdbuild $NGDBUILD_OPTIONS -uc $NGDBUILD_UCF -sd ${dir}${l}..${l}..${l}..${l}..${l} ${dir}${l}results${l}xilinx_pci_exp_ep_top.ngc";
  print ("INFO: $command\n");
  $status = 0;
  $status = system ("$command");
  if($status != 0)
  {
    print ("ERROR: NGDBUILD completed with errors.\n");
    print ("     : $SCRIPT_NAME Terminating!\n");
    print ("\n");
    exit 1;
  }
  print ("INFO: NGDBUILD complete\n");
}

sub run_map
{
  system("$move xilinx_pci_exp_ep_top.ngd ${dir}${l}results${l}xilinx_pci_exp_ep_top.ngd");
  system("$move xilinx_pci_exp_ep_top.bld ${dir}${l}results${l}xilinx_pci_exp_ep_top.bld");

  print ("\nINFO: Starting MAP\n");
  $MAPTARGET = $TARGET_NAME . "_map";

  $command = "map $MAP_OPTIONS -o ${dir}${l}results${l}mapped.ncd ${dir}${l}results${l}xilinx_pci_exp_ep_top.ngd ${dir}${l}results${l}mapped.pcf";
  print ("INFO: $command\n");
  $status = 0;
  $status = system ("$command");
  if($status != 0)
  {
    print ("ERROR: MAP completed with errors.\n");
    print ("     : $SCRIPT_NAME Terminating!\n");
    print ("\n");
    exit 1;
  }
  print ("INFO: MAP complete\n");
}

sub run_par
{
  print ("\nINFO: Starting PAR\n");

  $command = "par $PAR_OPTIONS -w ${dir}${l}results${l}mapped.ncd ${dir}${l}results${l}routed.ncd ${dir}${l}results${l}mapped.pcf";
  print ("INFO: $command\n");
  $status = 0;
  $status = system ("$command");
  if($status != 0)
  {
    print ("ERROR: PAR completed with errors.\n");
    print ("     : $SCRIPT_NAME Terminating!\n");
    print ("\n");
    exit 1;
  }
  print ("INFO: PAR complete\n");
}

sub run_trce
{
  print ("\nINFO: Starting TRCE\n");

  $command = "trce $TRCE_OPTIONS ${dir}${l}results${l}routed.ncd ${dir}${l}results${l}mapped.pcf";
  print ("INFO: $command\n");
  $status = 0;
  $status = system ("$command");
  if($status != 0)
  {
    print ("ERROR: TRCE completed with errors.\n");
    print ("     : $SCRIPT_NAME Terminating!\n");
    print ("\n");
    exit 1;
  }
  print ("INFO: TRCE complete\n");
}

sub run_bitgen
{
  print ("\nINFO: Starting BITGEN\n");
  $command = "bitgen -w ${dir}${l}results${l}routed.ncd";
  print ("INFO: $command\n");
  $status = 0;
  $status = system ("$command");
  if($status != 0)
  {
    print ("ERROR: BITGEN completed with errors.\n");
    print ("     : $SCRIPT_NAME Terminating!\n");
    print ("\n");
    exit 1;
  }
  print ("INFO: BITGEN complete\n");
}

#START PROMGEN
#print "Running design through promgen\n";
#if ($dev_board == 1 ) { #ML555
#   system("promgen -p mcs -c FF -x xcf32p -o ${dir}${l}results{l}routed_ml555.mcs -u 0 ${dir}${l}results{l}routed.bit");
#} elsif ($dev_board == 2) { #HTG
#	system("promgen -p mcs -x xcf128x -data_width 16 -o ${dir}${l}results{l}routed_htg.mcs -u 0 ${dir}${l}results{l}routed.bit");
#} elsif ($dev_board == 3) { #Spartan 3 PCIe Starter Kit
#   system("promgen -p mcs -c FF -x xcf08p -o ${dir}${l}results{l}routed_s31000.mcs -u 0 ${dir}${l}results{l}routed.bit");
#}
#END PROMGEN

#CLEANUP
system("$move *.twr ${dir}${l}results${l}");
system("$move *.twx ${dir}${l}results${l}");
system("$remove *.xrpt");
system("$remove *.ise");
system("$remove *.xml");
system("$remove $force xlnx_auto_0_xdb")
