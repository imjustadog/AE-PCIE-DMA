// (c) Copyright 2009 – 2009 Xilinx, Inc. All rights reserved.
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
//  XBMD_MAIN.cpp description:
//  This module is the main backend source code for the XBMD GUI.  It contains all handlers/callback functions which 
//  are invoked from user interaction in the GUI.  
//---------------------------------------------------------------------------------------------------------------

#include <gtk/gtk.h>           // Include GTK+ libraries
#include <glade/glade.h>       // Include Glade Libraries
#include <sys/ioctl.h>         // Include IOCTL calls to access Kernel Mode Driver

//Include Standard C++ Libraries
#include <stdlib.h> 
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <fcntl.h> 

#include "xbmd.h"              // Include XBMD driver header which defines SUCCESS(0) and FAILURE(-1)
#include "xbmd_main.h"         // Include main program header which defines 
#include "xbmd_ep.h"           // Include XBMD_EP class header. CFG and BMD classes included in XBMD_EP header

using namespace std; // delete

// Location of UI XML file relative to path in which program is running. 
// XML file contains GUI specifics for each widget (XY coordinates, visual height, size, etc...)
#define BUILDER_XML_FILE "xbmd_app.xml"

// Define a re-painting macro
// While "gtk_events_pending" used frequently and ensures GTK main loop 
// is entered after updating text boxes.  Required because 
// re-painting of GUI only occurs within GTK main loop
#define REPAINT_GUI \
  while(gtk_events_pending()) \
  gtk_main_iteration();  

// Performance base variables for color coding performance.  Labeled as global so they can be accessed by multiple
// handlers.  Protection is not required so they remain globals.
int green_base_wr = 0;
int yellow_base_wr = 0;
int green_base_rd = 0;
int yellow_base_rd = 0;
GdkColor color;

// xbmd_ep_t is our Endpoint class type which contains subclasses BMD_T and CFG_T.
// This class structure represents the two major components of a Xilinx PCIe EP design.
// The BMD class contains variables and member functions needed to access the XBMD backend application
// The CFG class contains variables and member functions needed to access the EP configuration space
xbmd_ep_t xbmd_ep;

// xbmd_descriptors_t is a global structure containing all globals related to the backend XBMD descriptor registers.
// xbmd_descriptors_t is marked global to allow access to all variables within different callback functions (handlers)
xbmd_descriptors_t xbmd_descriptors;

// link_speed_mult used to in calculation of performance base variables and is either 2500 or 5000 depending on 
// whether link trained to GEN1 (2500mbps) or GEN2 (5000mbps).
int link_speed_mult = 0;  

// Extern Prototype declarations 
extern int cfg_read_regs(int g_devFile);       // defined in cfg.h and cfg.cpp
extern int cfg_get_capabilities(int g_devFile);       // defined in cfg.h and cfg.cpp
extern int cfg_update_regs(int g_devFile);       // defined in cfg.h and cfg.cpp
extern int read_bmd_regs(int g_devFile);  // defined in bmd.h and bmd.cpp
extern int run_xbmd(xbmd_descriptors_t xbmd_descriptors, int ii);  // defined in bmd.h and bmd.cpp

// Struct stores the GUI widgets which need to be accessed in the handler functions
typedef struct {
  GtkWidget  *window;                       // Main GUI Window Widget
  GtkWidget  *run_count;                    // Provides # iterations to XBMD
  GtkWidget  *neg_link_width;               // Displays Neg Link Width
  GtkWidget  *neg_link_speed;               // Displays Neg Link Speed 
  GtkWidget  *start_button;                 // Button starts DMA transfer
  GtkWidget  *iter_count_text;              // Displays current iteration
  GtkWidget  *write_check;                  // Enables Write DMA
  GtkWidget  *read_check;                   // Enables Read DMA
  GtkWidget  *write_tlp_size;               // Sets Write TLP size
  GtkWidget  *read_tlp_size;                // Sets Read TLP size
  GtkWidget  *write_num_tlps;               // Sets # of Write TLPs to transfer
  GtkWidget  *read_num_tlps;                // Sets # of Read TLPs to transfer
  GtkWidget  *write_tlp_pattern;            // Sets Write TLP payload pattern
  GtkWidget  *read_tlp_pattern;             // Sets Read TLP payload pattern
  GtkWidget  *write_bytes_to_transfer;      // Displays Write DMA bytes to trans
  GtkWidget  *read_bytes_to_transfer;       // Displays Read DMA bytes to trans
  GtkWidget  *write_bytes_transferred;      // Displays Write bytes transferred
  GtkWidget  *read_bytes_transferred;       // Displays Read bytes transferred
  GtkWidget  *write_mbps;                   // Displays Write Performance
  GtkWidget  *read_mbps;                    // Displays Read Performance
  GtkWidget  *write_status;                 // Displays Write status/result
  GtkWidget  *read_status;                  // Displays Read status/result
  GtkWidget  *cfg_space;                    // Displays EP CFG space
  GtkWidget  *bmd_space;                    // Displays EP BMD Descriptor Regs
  GtkWidget  *xbmd_log;                     // Displays XBMD post transfer log
  GtkWidget  *main_status_bar;              // Status bar stating GUI state
  guint      main_status_bar_context_id;    // Status bar ID
} xbmd_app;


/* Function prototypes including GUI handlers */
extern "C" {

// Initializes GUI and prepares DMA variables
gboolean init_app (xbmd_app *app);

// Handles all cleanup necessary when GUI is ended
void on_main_window_destroy(xbmd_app *app);
  
// Called when Start button is clicked
void on_start_button_clicked (GtkButton *button, xbmd_app *app);
    
// Called DMA run count Spin Button is changed
void on_run_count_spinner1_value_changed(GtkSpinButton *spinbutton, xbmd_app *app);

// Called when Write Enable checkbox is toggled
void on_write_checkbox_toggled(GtkToggleButton *wr_check, xbmd_app *app);

// Called when Read Enable checkbox is toggled
void on_read_checkbox_toggled(GtkToggleButton *rd_check, xbmd_app *app);

// Called when Write TLP size box is changed
void on_wr_tlp_box_value_changed(GtkSpinButton *wr_size, xbmd_app *app); 

// Called when Read TLP size box is changed
void on_rd_tlp_box_changed(GtkSpinButton *rd_size, xbmd_app *app);

// Called when # of Write TLP box is changed
void on_wr_num_tlps_spinner_value_changed(GtkSpinButton *wr_tlps, xbmd_app *app);

// Called when # of Read TLP box is changed
void on_rd_num_tlps_spinner_value_changed(GtkSpinButton *rd_tlps, xbmd_app *app);

// Called when Read CFG space button is clicked
void on_read_cfg_button_clicked (GtkButton *button, xbmd_app *app);

// Verifies Write or Read pattern is valid
bool pattern_check(const char* wr_pattern_new, int pattern_length);

// Gets the DMA statistics and displays to status
bool get_results_and_display(xbmd_app *app);

// Erases previous DMA results
void erase_old_results(xbmd_app *app);

// Displays the DMA log 
void display_bmd_log(xbmd_app *app);

// Gets link width and displays. Calulates performance base thresholds
int verify_link_config(xbmd_app *app);

// Toggles the sensitivity of the GUI widgets 
void toggle_gui_sensitivity(xbmd_app *app, bool on_off);
   
}

//--- Main(): Initializes the GUI
//--- Arguments: ARGC and ARGV command line entries - not currently used
//--- Return Value: Returns INT stating SUCCESS or failure
//--- Detailed Description: Sets up and displays GUI and enters GTK loop 
//--- waiting for user inputs
int main (int argc, char *argv[]) {
  xbmd_app      *app;

  //allocate the memory needed by our TutorialTextEditor struct 
  app = g_slice_new (xbmd_app);

  //initialize GTK+ libraries 
  gtk_init (&argc, &argv);

  //calls init app which initializes GUI code.  Passes in xbmd_app struct
  if (init_app (app) < 0) return 1;  

  //show the window   
  gtk_widget_show (app->window);

  if (verify_link_config(app) < 0) return 1;

  //enter GTK+ main loop                    
  gtk_main ();

  //free memory we allocated for TutorialTextEditor struct 
  g_slice_free (xbmd_app, app);
        
  return 0;
}

//--- init_app():   Initialization of GUI and program variables
//--- Arguments:    XBMD APP struct
//--- Return Value: Returns success or failure
//--- Description:  Runs GTK builder to build interface and attaches XBMD APP
//---               struct elements to specific widgets. Opens EP device file.
//---               Sets up performance threshold for color coding performance.
//---               Displays Negotiated Link Width and Speed to GUI.
gboolean init_app (xbmd_app *app) {
  GtkBuilder              *builder;
  GError                  *err=NULL;
  
  // use GtkBuilder to build our interface from the XBMD XML file 
  builder = gtk_builder_new ();
  if (gtk_builder_add_from_file (builder, BUILDER_XML_FILE, &err) == 0) {
    fprintf(stderr, "XBMD_APP xml file not found (xbmd_app.xml). Verify it is present in directory with xbmd_app executable");
    return CRIT_ERR;
  }
      
  // Now that the GUI XML is found, attach each XBMD APP struct element to its widget based on given widget name
  app->window                  = GTK_WIDGET (gtk_builder_get_object (builder, 
                                                           "main_window"));
  app->run_count               = GTK_WIDGET (gtk_builder_get_object (builder, 
                                                           "run_count_spinner"));
  app->iter_count_text         = GTK_WIDGET (gtk_builder_get_object (builder, 
                                                           "iter_count_disp"));
  app->neg_link_width          = GTK_WIDGET (gtk_builder_get_object (builder, 
                                                           "link_width"));
  app->neg_link_speed          = GTK_WIDGET (gtk_builder_get_object (builder, 
                                                           "link_speed"));
  app->start_button            = GTK_WIDGET (gtk_builder_get_object (builder, 
                                                           "start_button"));
  app->write_check             = GTK_WIDGET (gtk_builder_get_object (builder, 
                                                           "write_checkbox"));
  app->read_check              = GTK_WIDGET (gtk_builder_get_object (builder, 
                                                           "read_checkbox"));
  app->write_tlp_size          = GTK_WIDGET (gtk_builder_get_object (builder, 
                                                           "wr_tlp_box"));
  app->read_tlp_size           = GTK_WIDGET (gtk_builder_get_object (builder, 
                                                           "rd_tlp_box"));
  app->write_num_tlps          = GTK_WIDGET (gtk_builder_get_object (builder, 
                                                           "wr_num_tlps_spinner"));
  app->read_num_tlps           = GTK_WIDGET (gtk_builder_get_object (builder, 
                                                           "rd_num_tlps_spinner"));
  app->write_tlp_pattern       = GTK_WIDGET (gtk_builder_get_object (builder, 
                                                           "wr_tlp_pattern_entry"));
  app->read_tlp_pattern        = GTK_WIDGET (gtk_builder_get_object (builder, 
                                                           "rd_tlp_pattern_entry"));
  app->write_bytes_to_transfer = GTK_WIDGET (gtk_builder_get_object (builder, 
                                                           "wr_bytes_to_transfer"));
  app->read_bytes_to_transfer  = GTK_WIDGET (gtk_builder_get_object (builder, 
                                                           "rd_bytes_to_transfer"));
  app->write_bytes_transferred = GTK_WIDGET (gtk_builder_get_object (builder, 
                                                           "wr_bytes_transferred_text"));
  app->read_bytes_transferred  = GTK_WIDGET (gtk_builder_get_object (builder, 
                                                           "rd_bytes_transferred_text"));
  app->write_mbps              = GTK_WIDGET (gtk_builder_get_object (builder, 
                                                           "wr_mbps_text"));
  app->read_mbps               = GTK_WIDGET (gtk_builder_get_object (builder, 
                                                           "rd_mbps_text"));
  app->write_status            = GTK_WIDGET (gtk_builder_get_object (builder, 
                                                           "wr_status_text"));
  app->read_status             = GTK_WIDGET (gtk_builder_get_object (builder, 
                                                           "rd_status_text"));
  app->cfg_space               = GTK_WIDGET (gtk_builder_get_object (builder, 
                                                           "cfg_space"));
  app->bmd_space               = GTK_WIDGET (gtk_builder_get_object (builder, 
                                                           "bmd_space"));
  app->xbmd_log                = GTK_WIDGET (gtk_builder_get_object (builder, 
                                                           "xbmd_log"));
  app->main_status_bar         = GTK_WIDGET (gtk_builder_get_object (builder, 
                                                           "main_statusbar"));
    
  // Connects all signals from GUI to handler functions                                         
  gtk_builder_connect_signals (builder, app);
                  
  // free memory used by GtkBuilder object 
  g_object_unref (G_OBJECT (builder));
        
  // Remove old log files/device configuration files
  remove ("*.txt");

  // Device file name declaration by endpoint.  Used to aquire endpoint and attach application->driver->device
  char devname[] = "/dev/xbmd";
  char* devfilename = devname;

  // Opens device file using standard IOCTL call and returns the device file integer for use later to access device
  // XBMD application and configuration space
  xbmd_descriptors.g_devFile = open(devfilename, O_RDWR);
  if ( xbmd_descriptors.g_devFile < 0 )  {
    fprintf(stderr,"Error opening device file.  Use lspci to verify device is recognized and Device/Vendor ID match\n");
    return CRIT_ERR;
  }

  if (xbmd_ep.cfg.cfg_get_capabilities(xbmd_descriptors.g_devFile) < 0) {
   //FIXME - Do something here
  } else {
    xbmd_descriptors.pm_offset                  = xbmd_ep.cfg.pm_offset;               
    xbmd_descriptors.msi_offset                 = xbmd_ep.cfg.msi_offset;
    xbmd_descriptors.pcie_cap_offset            = xbmd_ep.cfg.pcie_cap_offset; 
    xbmd_descriptors.device_cap_offset          = xbmd_ep.cfg.device_cap_offset; 
    xbmd_descriptors.device_stat_cont_offset    = xbmd_ep.cfg.device_stat_cont_offset; 
    xbmd_descriptors.link_cap_offset            = xbmd_ep.cfg.link_cap_offset; 
    xbmd_descriptors.link_stat_cont_offset      = xbmd_ep.cfg.link_stat_cont_offset;  
  }
  
  if (xbmd_ep.cfg.cfg_update_regs(xbmd_descriptors.g_devFile) < 0) {
  // FIXME - Do something here on error returned
  } 
      
  // Initialize WR/RD "Bytes to Transfer" text box. WR TLP size = DWORDs.
  // we need to multiply by 4 and total # of TLP's to get total bytes to transfer
  sprintf(xbmd_descriptors.wr_bytes_to_trans,"%d",(xbmd_descriptors.wr_tlp_size*xbmd_descriptors.num_wr_tlps*4));
  sprintf(xbmd_descriptors.rd_bytes_to_trans,"%d",(xbmd_descriptors.rd_tlp_size*xbmd_descriptors.num_rd_tlps*4));         
  app->main_status_bar_context_id = gtk_statusbar_get_context_id (GTK_STATUSBAR (app->main_status_bar), "XBMD_READY");

 
  
  //gtk_label_set_text(GTK_LABEL(app->neg_link_speed), xbmd_descriptors.neg_link_speed_char);

  // After initialization, change GUI status bar stating XBMD is ready
  // GTK status bars are built in a stack configuration.  Push and Pop 
  // operations are required to change string displayed.
  gtk_statusbar_push(GTK_STATUSBAR(app->main_status_bar),app->main_status_bar_context_id, "XBMD Performance Demo Ready");
  return SUCCESS;
}


extern "C" {

//------------------------------------------------------------------------------------------------------------------
//--- Section:     XBMD Tab Handlers
//--- Description: Section containing all handler/callback functions for XBMD tab 
//------------------------------------------------------------------------------------------------------------------


//--- Function Name: on_main_window_destroy                              
//--- Arguments:     XBMD APP struct
//--- Return Value:  No return value.  GTK_MAIN_QUIT called which closes GUI.
//--- Description:   Callback invoked when main GUI window is detroyed. 
//---                Responsible for all cleanup and de-allocation of pointers.
void on_main_window_destroy(xbmd_app *app) {
  //delete wr_bytes_to_trans; 
 // delete rd_bytes_to_trans;
  //delete xbmd_descriptors.neg_link_width;
  //delete xbmd_descriptors.neg_link_speed;
  //delete rd_mbps;
  //delete wr_mbps;
  //delete iter_count_t;
  gtk_main_quit ();
}

//--- Function Name: on_start_button_clicked                              
//--- Arguments:     XBMD APP struct
//--- Return Value:  Returns to GTK main loop upon completion. No return value
//--- Description:   Responsible for the following:
//---                  1) Disabling GUI widget sensitivity during transfer
//---                  2) Verifying either Write or Read enable is selected
//---                  3) Running a defined # of DMA iterations if WR||RD enabled
//---                  4) Checking for errors in between each DMA iteration
//---                  5) Display Results and Performance in GUI
//---                  6) Load XBMD log into XBMD tab
void on_start_button_clicked(GtkButton *button, xbmd_app *app) {
   
  // Turn GUI widget sensitivity off   
  toggle_gui_sensitivity(app,FALSE);

  // Remove old XBMD so XBMD log tab is updated with most recent iteration
  remove("prev_xbmd_log.txt");
  remove("xbmd_log.txt");

  // Check to make sure either write or read DMA is enabled before proceeding.  If neither is checked. Display status
  // and return to main GUI.
  if ((xbmd_descriptors.rd_enable != 0x00010000) && (xbmd_descriptors.wr_enable != 0x00000001)) {

    //state statusbar is stack, state functionality
    gtk_statusbar_push(GTK_STATUSBAR(app->main_status_bar),
                                       app->main_status_bar_context_id, 
                                       "Write nor Read selected.  Please enable Write and/or Read checkboxes");
    REPAINT_GUI;  
            
    // Sleep for a couple seconds so that user has enough time to see status 
    // bar stating to enable Reads or Writes
    sleep(2);
    gtk_statusbar_pop(GTK_STATUSBAR(app->main_status_bar),app->
                                      main_status_bar_context_id);
      
    REPAINT_GUI;
        
  } else {
      
    // Read Enable or Write Enable is selected so proceed with DMA transfer.
    // Update Status Bar stating XBMD is running.
    gtk_statusbar_push(GTK_STATUSBAR(app->main_status_bar),app->main_status_bar_context_id, "XBMD RUNNING");
     
    // First erase old results from GUI
    erase_old_results(app);
    
    REPAINT_GUI;

    // Performance values are stored in BMD class.  Must reset old 
    // performance values before running.
    xbmd_ep.bmd.wr_mbps = 0;
    xbmd_ep.bmd.rd_mbps = 0;

    // Perform DMA for certain # of iterations  - use ii instead
    for (int ii = 0; ii < xbmd_descriptors.num_iter; ii++) {
      sprintf(xbmd_descriptors.iter_count_t,"%d",(xbmd_descriptors.num_iter-ii));  // state what this is actually doing 
      gtk_label_set_text(GTK_LABEL(app->iter_count_text), xbmd_descriptors.iter_count_t);
      REPAINT_GUI;
       
      // Call run_xbmd which is a member function of the BMD class and defined in BMD.cpp.  
      if (xbmd_ep.bmd.run_xbmd(xbmd_descriptors, ii)) {

        // If DMA transfer returns critical condition, stop and display failing 
        // condition to GUI status bar.  Critical condition is returned when
        // the following occurs:
        //   1) DMA descriptor Register is not written or read successfully
        //   2) FILEIO is not successfully initiated
        //   3) EP reports a fatal error has been detected
        //   4) MPS read in device control register is invalid
        gtk_statusbar_push(GTK_STATUSBAR(app->main_status_bar),app->main_status_bar_context_id, xbmd_ep.bmd.bmd_fatal_text);
         
        REPAINT_GUI;

        // Sleep for a couple seconds so that user has enough time to see status 
        // bar stating reason for DMA failure.
        // FIXME - Need a pop-up here to select "OK". for now, wait 2 seconds so user can see message.3
        sleep(2);
        gtk_statusbar_pop(GTK_STATUSBAR(app->main_status_bar),app->main_status_bar_context_id);
        REPAINT_GUI;
      }
        
      // After each iteration, check for failure on write or Read DMA.  If read
      // is detected, do not continue with subsequent transfers
      // "get_rd/wr_success" are BMD member functions which returns int stating
      // success or failure and is defined in BMD.cpp
      if ((xbmd_descriptors.rd_enable == 0x00010000) && (!xbmd_ep.bmd.get_rd_success())) {
        break;
      }
      if ((xbmd_descriptors.wr_enable == 0x00000001) && (!xbmd_ep.bmd.get_wr_success())) {
        break;
      }
    }  // end DMA iteration loop

    // After DMA iterations are completed or failure has occurred, calculate 
    // results and display to GUI.  "get_results_and_display" is a supporting 
    // function defined in this file (xbmd_main.cpp).  It returns success or
    // failure stating whether or not results were displayed.      
    if (get_results_and_display(app) < 0) {
      // FIXME do STDERR. gtk_statusbar_push(GTK_STATUSBAR(app->main_status_bar),app->main_status_bar_context_id, "Could not load results");
    }

    // After DMA transfer is completed, show the XBMD log in the XBMD tab. 
    // "display_bmd_log" is a supporting function defined in this file
    // (xbmd_main.cpp)
    display_bmd_log(app);
      
    // Pop "XBMD_RUNNING" string off of status bar stack.
    gtk_statusbar_pop(GTK_STATUSBAR(app->main_status_bar),app->main_status_bar_context_id);
  }

  // Turn Widget Sensitivity back on
  toggle_gui_sensitivity(app,TRUE);

} // end on_start_button_clicked

//--- Function Name: on_run_count_spinner_value_changed                              
//--- Arguments:     XBMD APP struct, Run Count SpinButton Widget
//--- Return Value:  Returns to GTK main loop upon completion. No return value
//--- Description:   Callback invoked when Run Count Spin Button is changed 
//---                Responsible for the following:
//---                  1) Getting current value of run_count spin button and
//---                     setting num_iter int located in global xbmd_descriptors
//---                     struct
void on_run_count_spinner_value_changed(GtkSpinButton *spinbutton, xbmd_app *app) {
  xbmd_descriptors.num_iter = gtk_spin_button_get_value_as_int(spinbutton); 
}

//--- Function Name: on_write_checkbox_toggled                             
//--- Arguments:     XBMD APP struct, Write Togglebutton Widget
//--- Return Value:  Returns to GTK main loop upon completion. No return value
//--- Description:   Callback invoked when Write Checkbox toggled 
//---                Responsible for the following:
//---                  1) Getting current value (true or false) of Write Checkbox
//---                     and setting wr_enable int located in global 
//---                     xbmd_descriptors struct                
void on_write_checkbox_toggled(GtkToggleButton *wr_check, xbmd_app *app) {
  if (gtk_toggle_button_get_active(wr_check)) {
    xbmd_descriptors.wr_enable= 0x00000001;
  } else {
    xbmd_descriptors.wr_enable= 0x00000000;
  }
}

//--- Function Name: on_read_checkbox_toggled                             
//--- Arguments:     XBMD APP struct, Read Togglebutton Widget
//--- Return Value:  Returns to GTK main loop upon completion. No return value
//--- Description:   Callback invoked when Write Checkbox toggled 
//---                Responsible for the following:
//---                  1) Getting current value (true or false) of Read Checkbox
//---                     and setting rd_enable int located in global 
//---                     xbmd_descriptors struct    
void on_read_checkbox_toggled(GtkToggleButton *rd_check, xbmd_app *app) {
  if (gtk_toggle_button_get_active(rd_check)) {
    xbmd_descriptors.rd_enable= 0x00010000;
  } else {
    xbmd_descriptors.rd_enable= 0x00000000;
  }
}

//--- Function Name: on_wr_tlp_box_value_changed                             
//--- Arguments:     XBMD APP struct, Write TlP Size SpinButton Widget
//--- Return Value:  Returns to GTK main loop upon completion. No return value
//--- Description:   Callback invoked when Write TLP size is changed
//---                Responsible for the following:
//---                  1) Getting current value WR TLP size spin button and
//---                     and setting wr_tlp_size int located in global 
//---                     xbmd_descriptors struct  
//---                  2) Updates write_bytes_to_trans widget stating the
//---                     total bytes to transfer in each WR DMA iteration
void on_wr_tlp_box_value_changed(GtkSpinButton *wr_size, xbmd_app *app) {
   xbmd_descriptors.wr_tlp_size = gtk_spin_button_get_value_as_int(wr_size); 
   xbmd_descriptors.bytes_to_trans = xbmd_descriptors.wr_tlp_size*xbmd_descriptors.num_wr_tlps*4;   //lets do an inline function

   //use sprintf to convert bytes_to_trans int to char* and then  update GUI
   sprintf(xbmd_descriptors.wr_bytes_to_trans,"%d",xbmd_descriptors.bytes_to_trans);
   gtk_label_set_text(GTK_LABEL(app->write_bytes_to_transfer), xbmd_descriptors.wr_bytes_to_trans);
}

//--- Function Name: on_rd_tlp_box_value_changed                             
//--- Arguments:     XBMD APP struct, Read TlP Size SpinButton Widget
//--- Return Value:  Returns to GTK main loop upon completion. No return value
//--- Description:   Callback invoked when Read TLP size is changed
//---                Responsible for the following:
//---                  1) Getting current value of RD TLP size spin button and
//---                     and setting rd_tlp_size int located in global 
//---                     xbmd_descriptors struct. 
//---                  2) Updates read_bytes_to_trans widget stating the
//---                     total bytes to transfer in each RD DMA iteration.
void on_rd_tlp_box_value_changed(GtkSpinButton *rd_size, xbmd_app *app) {
   xbmd_descriptors.rd_tlp_size = gtk_spin_button_get_value_as_int(rd_size); 
   xbmd_descriptors.bytes_to_trans = xbmd_descriptors.rd_tlp_size*xbmd_descriptors.num_rd_tlps*4;

   //Use sprintf to convert bytes_to_trans int to char* and then update GUI
   sprintf(xbmd_descriptors.rd_bytes_to_trans,"%d",xbmd_descriptors.bytes_to_trans);
   gtk_label_set_text(GTK_LABEL(app->read_bytes_to_transfer), xbmd_descriptors.rd_bytes_to_trans);;
}

//--- Function Name: on_wr_num_tlps_spinner_value_changed                          
//--- Arguments:     XBMD APP struct, Number of Write TLP's SpinButton Widget
//--- Return Value:  Returns to GTK main loop upon completion. No return value
//--- Description:   Callback invoked when number of Write TLPs to transfer is
//---                changed.
//---                Responsible for the following:
//---                  1) Getting current value of # WR TLPs spin button and
//---                     and setting num_wr_tlps int located in global 
//---                     xbmd_descriptors struct.
//---                  2) Updates write_bytes_to_trans widget stating the
//---                     total bytes to transfer in each WR DMA iteration.
void on_wr_num_tlps_spinner_value_changed(GtkSpinButton *wr_tlps, xbmd_app *app) {
   xbmd_descriptors.num_wr_tlps = gtk_spin_button_get_value_as_int(wr_tlps); 
   xbmd_descriptors.bytes_to_trans = xbmd_descriptors.wr_tlp_size*xbmd_descriptors.num_wr_tlps*4;

   //Convert bytes_to_trans int to char* and update GUI
   sprintf(xbmd_descriptors.wr_bytes_to_trans,"%d",xbmd_descriptors.bytes_to_trans);
   gtk_label_set_text(GTK_LABEL(app->write_bytes_to_transfer), xbmd_descriptors.wr_bytes_to_trans);
}

//--- Function Name: on_rd_num_tlps_spinner_value_changed                          
//--- Arguments:     XBMD APP struct, Number of Read TLP's SpinButton Widget
//--- Return Value:  Returns to GTK main loop upon completion. No return value
//--- Description:   Callback invoked when number of Read TLPs to transfer is
//---                changed.
//---                Responsible for the following:
//---                  1) Getting current value of # RD TLPs spin button and
//---                     and setting num_rd_tlps int located in global 
//---                     xbmd_descriptors struct.
//---                  2) Updates read_bytes_to_trans widget stating the
//---                     total bytes to transfer in each RD DMA iteration.
void on_rd_num_tlps_spinner_value_changed(GtkSpinButton *rd_tlps, xbmd_app *app) {
   xbmd_descriptors.num_rd_tlps = gtk_spin_button_get_value_as_int(rd_tlps); 
   xbmd_descriptors.bytes_to_trans = xbmd_descriptors.rd_tlp_size*xbmd_descriptors.num_rd_tlps*4;

   //Convert bytes_to_trans int to char* and update GUI
   sprintf(xbmd_descriptors.rd_bytes_to_trans,"%d",xbmd_descriptors.bytes_to_trans);
   gtk_label_set_text(GTK_LABEL(app->read_bytes_to_transfer), xbmd_descriptors.rd_bytes_to_trans);
}

//--- Function Name: on_wr_tlp_pattern_entry_changed                         
//--- Arguments:     XBMD APP struct, Write Pattern Entry Widget
//--- Return Value:  Returns to GTK main loop upon completion. No return value
//--- Description:   Callback invoked when the Write TLP pattern is changed
//---                Responsible for the following:
//---                  1) Getting current pattern in Write Entry Widget and
//---                  2) Verify new Write Pattern is valid.  Dis-allow starting
//---                     of DMA trans if invalid (disable start button).
//---                  2) Update wr_pattern_new int in global xbmd_descriptors 
//---                     struct
void on_wr_tlp_pattern_entry_changed(GtkEntry *wr_pattern, xbmd_app *app) {
  xbmd_descriptors.wr_pattern_new = gtk_entry_get_text(wr_pattern);
  xbmd_descriptors.wr_pattern_length=strlen(xbmd_descriptors.wr_pattern_new);

  // verify that write pattern is valid. "pattern_check" is a supporting 
  // function defined in this file (xbmd_main.cpp).  It takes in a pattern 
  // and verifies pattern contains exactly 8 hex values.
  if ((pattern_check(xbmd_descriptors.wr_pattern_new, xbmd_descriptors.wr_pattern_length)) == false) {
      
    //update status bar upon incorrect pattern and turn disable start button
    gtk_label_set_text(GTK_LABEL(app->write_status), "Invalid Write Pattern - Hex values only");
    gtk_widget_set_sensitive (app->start_button,FALSE);
    xbmd_descriptors.wr_pattern_valid = false;
  } else {
    xbmd_descriptors.wr_pattern_valid = true;
  }

  if ((xbmd_descriptors.wr_pattern_length == 8) and (xbmd_descriptors.wr_pattern_valid != false)) {
      
    //update status bar upon incorrect pattern and enable start button
    // fixme - post valid write pattern for a second (GTK_MAIN_ITERATIOn stuff)
    gtk_widget_set_sensitive (app->start_button,TRUE);
    gtk_label_set_text(GTK_LABEL(app->write_status), "Valid Write Pattern");

    // Pattern equals 8 and all values are hex values so set pattern valid = true
    xbmd_descriptors.wr_pattern_valid = true;
  } else if ((xbmd_descriptors.wr_pattern_length !=8) and (xbmd_descriptors.wr_pattern_valid == true)) {
      
    //update status bar upon incorrect pattern and turn disable start button
    gtk_label_set_text(GTK_LABEL(app->write_status), "Invalid Write Pattern-8 Hex numbers ");
    gtk_widget_set_sensitive (app->start_button,FALSE);
    xbmd_descriptors.wr_pattern_valid = false;
  }
}  // END on_wr_tlp_pattern_entry_changed

//--- Function Name: on_rd_tlp_pattern_entry_changed                         
//--- Arguments:     XBMD APP struct, Read Pattern Entry Widget
//--- Return Value:  Returns to GTK main loop upon completion. No return value
//--- Description:   Callback invoked when the Read TLP pattern is changed
//---                Responsible for the following:
//---                  1) Getting current pattern in Read Entry Widget and
//---                  2) Verify new Read Pattern is valid.  Dis-allow starting
//---                     of DMA trans if invalid (disable start button).
//---                  2) Update rd_pattern_new int in global xbmd_descriptors 
//---                     struct
void on_rd_tlp_pattern_entry_changed(GtkEntry *rd_pattern, xbmd_app *app) {
  xbmd_descriptors.rd_pattern_new = gtk_entry_get_text(rd_pattern);
  xbmd_descriptors.rd_pattern_length=strlen(xbmd_descriptors.rd_pattern_new);

  // verify that write pattern is valid. "pattern_check" is a supporting 
  // function defined in this file (xbmd_main.cpp).  It takes in a pattern 
  // and verifies pattern contains exactly 8 hex values.
  if ((pattern_check(xbmd_descriptors.rd_pattern_new,xbmd_descriptors.rd_pattern_length)) == false) {
    gtk_widget_set_sensitive (app->start_button,FALSE);
    gtk_label_set_text(GTK_LABEL(app->read_status), "Invalid Read Pattern -\
                         Hex values only");
    xbmd_descriptors.rd_pattern_valid = false;
  } else {
    xbmd_descriptors.rd_pattern_valid = true;
  }

  if ((xbmd_descriptors.rd_pattern_length == 8) and (xbmd_descriptors.rd_pattern_valid != false)) {

    //update status bar upon incorrect pattern and enable start button
    // fixme - post valid write pattern for a second (GTK_MAIN_ITERATIOn stuff)
    gtk_widget_set_sensitive (app->start_button,TRUE);

    //update status bar upon incorrect pattern and turn disable start button
    gtk_label_set_text(GTK_LABEL(app->read_status), "Valid Read Pattern");

    // Pattern equals 8 and all values are hex values so set pattern valid = true
    xbmd_descriptors.rd_pattern_valid = true;
  } else if ((xbmd_descriptors.rd_pattern_length !=8) and (xbmd_descriptors.rd_pattern_valid == true)) {

    //update status bar upon incorrect pattern and turn disable start button
    gtk_label_set_text(GTK_LABEL(app->read_status), "Invalid Write Pattern-8 Hex numbers");
    gtk_widget_set_sensitive (app->start_button,FALSE);
    xbmd_descriptors.rd_pattern_valid = false;
  }    
}  // END on_rd_tlp_pattern_entry_changed


//-----------------------------------------------------------------------------------------------------------------
//--- Section:     CFG Tab Handlers
//--- Description: Section containing handler/callback functions for CFG log tab 
//-----------------------------------------------------------------------------------------------------------------

//--- Function Name: on_read_cfg_button_clicked                        
//--- Arguments:     XBMD APP struct, Read CFG button
//--- Return Value:  Returns to GTK main loop upon completion. No return value
//--- Description:   Callback invoked when the Read CFG button is clicked within
//--                 the CFG Tab.  
//---                Responsible for the following:
//---                  1) Sequentially reads all PCIe EP configuration space
//---                  2) Stores all values into file "cfg_space.txt"
//---                  3) Load file into CFG text buffer 
void on_read_cfg_button_clicked (GtkButton *button, xbmd_app *app) {
  GError                  *err=NULL;
  gchar                   *text;
  gboolean                result;
  GtkTextBuffer           *buffer;      
 
 // Call read_cfg_regs which is a member function of the CFG class and defined in cfg.cpp.   
 // read_cfg_regs returns either success or critical error.  A critical 
 // error occurs when either of the following occurs:
 //  1) Read of configuration register fails
 //  2) FILEIO fails 
  if (xbmd_ep.cfg.cfg_read_regs(xbmd_descriptors.g_devFile) < 0) {
    
    // Update status bar upon failure
    gtk_statusbar_push(GTK_STATUSBAR(app->main_status_bar),app->main_status_bar_context_id, xbmd_ep.cfg.cfg_fatal_text);
    
    REPAINT_GUI;  
            
    // Sleep for a couple seconds so that user has enough time to see status 
    // bar stating CFG read failed
    sleep(2);
    gtk_statusbar_pop(GTK_STATUSBAR(app->main_status_bar),app->main_status_bar_context_id);
  }
      
    // Load contents of "cfg_space.txt" into text char*
    result = g_file_get_contents ("cfg_space.txt", &text, NULL, &err);
    if (result == FALSE) {
      gtk_statusbar_push(GTK_STATUSBAR(app->main_status_bar),
                         app->main_status_bar_context_id, "CFG_READ FAILED! Could not load data from cfg_regs.txt");
    }
      
    // Disable text buffer sensitivity while loading in new text value
    gtk_widget_set_sensitive (app->cfg_space, FALSE);
      
    // Get the text buffer called attached to xbmd_struct widget "cfg_space"
    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (app->cfg_space));
      
    // Set the text buffer to contents located in text char*
    gtk_text_buffer_set_text (buffer, text, -1);
      
    // Set buffer attribute stating buffer was modified
    gtk_text_buffer_set_modified (buffer, FALSE);
      
    // Turn sensitity back on.
    gtk_widget_set_sensitive (app->cfg_space, TRUE);
      
    // Free the char* text
    g_free (text); 
}


//--------------------------------------------------------------------------------------------------------------
//--- Section:     BMD Tab Handlers
//--- Description: Section containing handler/callback functions for BMD log tab
//---------------------------------------------------------------------------------------------------------------

//--- Function Name: on_read_bmd_button_clicked                        
//--- Arguments:     XBMD APP struct, Read BMD button
//--- Return Value:  Returns to GTK main loop upon completion. No return value
//--- Description:   Callback invoked when the Read BMD button is clicked within
//--                 the BMD Tab.  
//---                Responsible for the following:
//---                  1) Sequentially reads all XBMD descriptor registers
//---                  2) Stores all values into file "bmd_space.txt"
//---                  3) Load file into BMD text buffer window
void on_read_bmd_button_clicked (GtkButton *button, xbmd_app *app) {
  GError                  *err=NULL;
  gchar                   *text;
  gboolean                result;
  GtkTextBuffer           *buffer;  

  // Call read_bmd_regs which is a member function of the BMD class and defined in bmd.cpp.   
  // read_bmd_regs returns either success or critical error.  A critical 
  // error occurs when either of the following occurs:
  //  1) Read of BMD descritptor register fails
  //  2) FILEIO fails     
  if (xbmd_ep.bmd.read_bmd_regs(xbmd_descriptors.g_devFile) < 0) {
      
    // update statusbar upon failure
    gtk_statusbar_push(GTK_STATUSBAR(app->main_status_bar),
                       app->main_status_bar_context_id, 
                       "BMD_READ FAILED! Check that driver loaded correctly and device is recognized");

    REPAINT_GUI;  
            
    // Sleep for a couple seconds so that user has enough time to see status 
    // bar stating CFG read failed
    gtk_statusbar_pop(GTK_STATUSBAR(app->main_status_bar),app->main_status_bar_context_id);
    sleep(2);
  } else {
        
    // Load contents of "cfg_space.txt" into text char*    
    result = g_file_get_contents ("bmd_regs.txt", &text, NULL, &err);
    if (result == FALSE) {
      gtk_statusbar_push(GTK_STATUSBAR(app->main_status_bar),
                         app->main_status_bar_context_id, 
                         "BMD_READ FAILED! Could not load data from bmd_regs.txt");
    }
      
    // Disable text buffer sensitivity while loading in new text value
    gtk_widget_set_sensitive (app->bmd_space, FALSE);
      
    // Get the text buffer called attached to xbmd_struct widget "bmd_space" 
    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (app->bmd_space));
      
    // Set the text buffer to contents located in text char*
    gtk_text_buffer_set_text (buffer, text, -1);
      
    // Set buffer attribute stating buffer was modified
    gtk_text_buffer_set_modified (buffer, FALSE);
      
    // Turn sensitity back on.
    gtk_widget_set_sensitive (app->bmd_space, TRUE);
      
    // Free the char* text
    g_free (text); 
  }
}

//------------------------------------------------------------------------------------------------------------------
//--- Section:      Display BMD Log Tab Handlers
//--- Description:  Section containing handler/callback functions for BMD log tab 
//-------------------------------------------------------------------------------------------------------------------


//--- Function Name: on_display_bmd_log                       
//--- Arguments:     XBMD APP struct, 
//--- Return Value:  Returns to GTK main loop upon completion. No return value
//--- Description:   Called when DMA transfer completes with either failure
//---                or success. 
//---                Responsible for the following:
//---                  1) Loads xbmd_log.txt contents into BMD Log text buffer 
//---                     window
void display_bmd_log(xbmd_app *app) {
  GError                  *err=NULL;
  gchar                   *text;
  gboolean                result;
  GtkTextBuffer           *buffer;  

  // Load contents of "cfg_space.txt" into text char*      
  result = g_file_get_contents ("xbmd_log.txt", &text, NULL, &err);
  if (result == FALSE) {
    gtk_statusbar_push(GTK_STATUSBAR(app->main_status_bar),
                       app->main_status_bar_context_id, "BMD log not found");
  }
    
  // Disable text buffer sensitivity while loading in new text value
  gtk_widget_set_sensitive (app->xbmd_log, FALSE);
    
  // Get the text buffer called attached to xbmd_struct widget "xbmd_log"
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (app->xbmd_log));
    
  // Set the text buffer to contents located in text char*
  gtk_text_buffer_set_text (buffer, text, -1);
    
  // Set buffer attribute stating buffer was modified
  gtk_text_buffer_set_modified (buffer, FALSE);
    
  // Turn sensitity back on.
  gtk_widget_set_sensitive (app->xbmd_log, TRUE);
    
  // Free the char* text
  g_free (text); 
}

//------------------------------------------------------------------------------------------------------------------
//--- Section:      Misc. Supporting Functions 
//--- Description:  Section containing functions called above by handlers 
//-------------------------------------------------------------------------------------------------------------------
   
//--- Function Name: get_results_and_display                       
//--- Arguments:     XBMD APP struct, 
//--- Return Value:  Returns bool (success or failure)
//--- Description:   Called when DMA transfer completes with either failure
//---                or success. 
//---                Responsible for the following:
//---                  1) Identifying if Write or Reads are enabled and only
//---                      display if either or is true.
//---                  2) Getting Write and/or Read performance data
//---                  3) Get the read MBPS
//---                  4) Color coding performance output based on predefind
//---                     thresholds
bool get_results_and_display(xbmd_app *app) {
    
  // Only calculate and display read performance if reads are enabled and 
  // the transfer was successful
  if ((xbmd_descriptors.rd_enable == 0x00010000) and (xbmd_ep.bmd.get_rd_success())) {
      
    // get_rd_mbps is a member function of the BMD class.  It returns a char*
    // containing the performance (in Mbits/second)
    xbmd_descriptors.rd_mbps=xbmd_ep.bmd.get_rd_mbps(xbmd_descriptors.num_iter);
      
    // set the rd_mbps text entry with the performance
    gtk_label_set_text(GTK_LABEL(app->read_mbps), xbmd_descriptors.rd_mbps);
     
    // set the total bytes transfer text entry.  Upon success, it is assumed
    // that the bytes transferred = bytes to be transfered calculated before
    // DMA was initiated and calculated in the following handler functions: 
    //   "on_rd_num_tlps_spinner_value_changed"
    //   "on_rd_tlp_box_value_changed"
    gtk_label_set_text(GTK_LABEL(app->read_bytes_transferred), xbmd_descriptors.rd_bytes_to_trans);

    // Coloring RD Performance based in color thresholds
    if ((xbmd_ep.bmd.rd_mbps/xbmd_descriptors.num_iter) > green_base_rd) {       
      // Place value associated with Forest Green at variable color
      gdk_color_parse("ForestGreen", &color);
    } else if ((xbmd_ep.bmd.rd_mbps/xbmd_descriptors.num_iter) > yellow_base_rd) {      
      // Place value associated with Yellow at variable color
      gdk_color_parse("ForestGreen", &color);    //gdk_color_parse("yellow3", &color);      
    } else {        
      // Place value associated with Red at variable color
      gdk_color_parse("ForestGreen", &color);  //gdk_color_parse("red", &color);    
    }
    // update color in GUI
    gtk_widget_modify_fg(app->read_mbps,GTK_STATE_NORMAL, &color);
  }

  // Only calculate and display read performance if reads are enabled and 
  // the transfer was successful
  if ((xbmd_descriptors.wr_enable == 0x00000001) and (xbmd_ep.bmd.get_wr_success())) {
      
    // get_wr_mbps is a member function of the BMD class.  It returns a char*
    // containing the performance (in Mbits/second)
    xbmd_descriptors.wr_mbps=xbmd_ep.bmd.get_wr_mbps(xbmd_descriptors.num_iter);
      
    // set the rd_mbps text entry with the performance
    gtk_label_set_text(GTK_LABEL(app->write_mbps), xbmd_descriptors.wr_mbps);
     
    // set the total bytes transfer text entry.  Upon success, it is assumed
    // that the bytes transferred = bytes to be transfered calculated before
    // DMA was initiated and calculated in the following handler functions: 
    //   "on_wr_num_tlps_spinner_value_changed"
    //   "on_wr_tlp_box_value_changed"
    gtk_label_set_text(GTK_LABEL(app->write_bytes_transferred), xbmd_descriptors.wr_bytes_to_trans);

   // Coloring WR Performance 
    if ((xbmd_ep.bmd.wr_mbps/xbmd_descriptors.num_iter) > green_base_wr) {        
      // Place value associated with Forest Green at variable color
     gdk_color_parse("ForestGreen", &color);       
    } else if ((xbmd_ep.bmd.wr_mbps/xbmd_descriptors.num_iter) > yellow_base_wr) {        
      // Place value associated with Yellow at variable color
     gdk_color_parse("ForestGreen", &color); //gdk_color_parse("yellow3", &color);
    } else {       
      // Place value associated with Red at variable color
     gdk_color_parse("ForestGreen", &color);  // gdk_color_parse("red", &color);    
    }
    // update color in GUI
    gtk_widget_modify_fg(app->write_mbps,GTK_STATE_NORMAL, &color);
  }

  // Display status regarding of success or failure
  if (xbmd_descriptors.wr_enable == 0x00000001) {
      
    // "get_wr_result_text" is a member function of BMD class and returns a
    // char* defining whether the WR DMA was successful.  
    xbmd_descriptors.wr_status = xbmd_ep.bmd.get_wr_result_text();
      
    // update write status text entry widget
    gtk_label_set_text(GTK_LABEL(app->write_status), xbmd_descriptors.wr_status);

  }
  if (xbmd_descriptors.rd_enable == 0x00010000) {
      
    // "get_wr_result_text" is a member function of BMD class and returns a
    // char* defining whether the RD DMA was successful. 
    xbmd_descriptors.rd_status = xbmd_ep.bmd.get_rd_result_text();
      
    // update read status text entry widget
    gtk_label_set_text(GTK_LABEL(app->read_status), xbmd_descriptors.rd_status);
  }
  return SUCCESS;
} // END get_results_and_display


//--- Function Name: erase_old_results                       
//--- Arguments:     XBMD APP struct, 
//--- Return Value:  No return value
//--- Description:   Erases the old results from GUI.
void erase_old_results(xbmd_app *app){
  const char* erase = " "; 
    
  // Erase value in write_bytes_transferred widget
  gtk_label_set_text(GTK_LABEL(app->write_bytes_transferred), erase);
    
  // Erase value in write_mbps widget
  gtk_label_set_text(GTK_LABEL(app->write_mbps), erase);
    
  // Erase value in write_status widget
  gtk_label_set_text(GTK_LABEL(app->write_status), erase);
    
  // Erase value in read_bytes_transferred widget
  gtk_label_set_text(GTK_LABEL(app->read_bytes_transferred), erase);

  // Erase value in read_mbps widget
  gtk_label_set_text(GTK_LABEL(app->read_mbps), erase);
    
  // Erase value in read_status widget
  gtk_label_set_text(GTK_LABEL(app->read_status), erase);
} // END erase_old_results

//--- Function Name: turn_off_gui_sensitivity                      
//--- Arguments:     XBMD APP struct, 
//--- Return Value:  No return value
//--- Description:   Toggles the GUI widget sensitivity
void toggle_gui_sensitivity(xbmd_app *app, bool on_off){
 //  app->write_check             = GTK_WIDGET (gtk_builder_get_object (builder, 
 //                                                          "write_checkbox"));
 // app->read_check              = GTK_WIDGET (gtk_builder_get_object (builder, 
   //                                                        "read_checkbox"));
 // app->write_tlp_size          = GTK_WIDGET (gtk_builder_get_object (builder, 
     //                                                      "wr_tlp_box"));
 // app->read_tlp_size           = GTK_WIDGET (gtk_builder_get_object (builder, 
       //                                                    "rd_tlp_box"));
//  app->write_num_tlps          = GTK_WIDGET (gtk_builder_get_object (builder, 
         //                                                  "wr_num_tlps_spinner"));
//  app->read_num_tlps           = GTK_WIDGET (gtk_builder_get_object (builder, 
           //                                                "rd_num_tlps_spinner"));
//  app->write_tlp_pattern       = GTK_WIDGET (gtk_builder_get_object (builder, 
             //                                              "wr_tlp_pattern_entry"));
//  app->read_tlp_pattern        = GTK_WIDGET (gtk_builder_get_object (builder, 
               //                                            "rd_tlp_pattern_entry"));

  gtk_widget_set_sensitive (app->start_button ,on_off);
  gtk_widget_set_sensitive (app->run_count, on_off);  
  gtk_widget_set_sensitive (app->write_check ,on_off);
  gtk_widget_set_sensitive (app->read_check, on_off);  
  gtk_widget_set_sensitive (app->write_tlp_size ,on_off);
  gtk_widget_set_sensitive (app->read_tlp_size, on_off);  
  gtk_widget_set_sensitive (app->write_num_tlps ,on_off);
  gtk_widget_set_sensitive (app->read_num_tlps, on_off);  
  gtk_widget_set_sensitive (app->write_tlp_pattern,on_off);
  gtk_widget_set_sensitive (app->read_tlp_pattern, on_off);  
  // Erase value in write_bytes_transferred widget
  //gtk_label_set_text(GTK_LABEL(app->write_bytes_transferred), erase);
    
  // Erase value in write_mbps widget
  //gtk_label_set_text(GTK_LABEL(app->write_mbps), erase);
    
  // Erase value in write_status widget
  //gtk_label_set_text(GTK_LABEL(app->write_status), erase);
    
  // Erase value in read_bytes_transferred widget
  //gtk_label_set_text(GTK_LABEL(app->read_bytes_transferred), erase);

  // Erase value in read_mbps widget
  //gtk_label_set_text(GTK_LABEL(app->read_mbps), erase);
    
  // Erase value in read_status widget
  //gtk_label_set_text(GTK_LABEL(app->read_status), erase);
} // END erase_old_results
   
//--- Function Name: pattern_check                       
//--- Arguments:     const char* pattern_new, int pattern_length 
//--- Return Value:  boolean stating true or false
//--- Description:   Returns true all values within the string pattern_new
//---                are hexidecimal numbers. Otherwise, return false.
bool pattern_check(const char* pattern_new, int pattern_length) {
  for (int k=0; k < pattern_length; k++) { 
    if ((pattern_new[k] != '0') and (pattern_new[k] != '1') and 
        (pattern_new[k] != '2') and (pattern_new[k] != '3') and 
        (pattern_new[k] != '4') and (pattern_new[k] != '5') and 
        (pattern_new[k] != '6') and (pattern_new[k] != '7') and 
        (pattern_new[k] != '8') and (pattern_new[k] != '9') and 
        (pattern_new[k] != 'A') and (pattern_new[k] != 'B') and 
        (pattern_new[k] != 'C') and (pattern_new[k] != 'D') and 
        (pattern_new[k] != 'E') and (pattern_new[k] != 'F') and 
        (pattern_new[k] != 'a') and (pattern_new[k] != 'b') and 
        (pattern_new[k] != 'c') and (pattern_new[k] != 'd') and 
        (pattern_new[k] != 'e') and (pattern_new[k] != 'f')) {
           return false;
    }
  }
  return true;
} // END pattern_check

//--- verify_link_config():Handles all setup related to link width/speed
//--- Arguments:       XBMD APP struct
//--- Return Value:    Returns success or failure
//--- Description:      1)Gets the link width and speed
//---                   2)Compares width and speed against EP capability. Updates status bar upon mismatch
//---                   3)Sets up performance color coding threshold which are dependent upon width and speed.
int verify_link_config(xbmd_app *app) {
  // Identify Negoitated Link Width by accessing EP configuration Link Status Register
  // If a mismatch between Link Status and Link Capability register occurs, main status bar will update stating
  // the link did not train to the width capable by the EP 
  // Bits [9:4] = Negotiated Link Width
  // Set GUI text labels displaying Link Speed/Width
  if (xbmd_ep.cfg.cfg_check_link_width(xbmd_descriptors.g_devFile) < 0) {    
    // Update status bar upon failure
    gtk_statusbar_push(GTK_STATUSBAR(app->main_status_bar),app->main_status_bar_context_id, xbmd_ep.cfg.cfg_fatal_text);
    // Still show the negotiated link width even though it doesn't match capability
    gtk_label_set_text(GTK_LABEL(app->neg_link_width), xbmd_ep.cfg.link_stat_neg_link_width_c); 
    toggle_gui_sensitivity(app,FALSE); 
    REPAINT_GUI;         
    // Sleep for a couple seconds so that user has enough time to see status 
    // bar stating failure
    sleep(3);
    toggle_gui_sensitivity(app,TRUE);
    gtk_statusbar_pop(GTK_STATUSBAR(app->main_status_bar),app->main_status_bar_context_id);
  } else {
    // Set GUI text labels displaying Link Width
    gtk_label_set_text(GTK_LABEL(app->neg_link_width), xbmd_ep.cfg.link_stat_neg_link_width_c);
    REPAINT_GUI; 
  }

  // Identify Negoitated Link Speed by accessing EP configuration Link Status Register
  // If a mismatch between Link Status and Link Capability register occurs, main status bar will update stating
  // the link did not train to the speed capable by the EP 
  // Bits [3:0] Negotiated Link Speed 
  if (xbmd_ep.cfg.cfg_check_link_speed(xbmd_descriptors.g_devFile) <0) {
    // Update status bar upon failure
    gtk_statusbar_push(GTK_STATUSBAR(app->main_status_bar),app->main_status_bar_context_id, xbmd_ep.cfg.cfg_fatal_text);
    // Still show the negotiated link width even though it doesn't match capability
    gtk_label_set_text(GTK_LABEL(app->neg_link_speed), xbmd_ep.cfg.link_stat_neg_link_speed_c); 
    toggle_gui_sensitivity(app,FALSE); 
    REPAINT_GUI;  
            
    // Sleep for a couple seconds so that user has enough time to see status 
    // bar stating failure
    sleep(3);
    toggle_gui_sensitivity(app,FALSE);
    gtk_statusbar_pop(GTK_STATUSBAR(app->main_status_bar),app->main_status_bar_context_id);
  } else {
    // Set GUI text labels displaying Link Speed
    gtk_label_set_text(GTK_LABEL(app->neg_link_speed), xbmd_ep.cfg.link_stat_neg_link_speed_c);
    REPAINT_GUI; 
  }

  // Set Link Width and Speed integers - ADD MORE COMMENTS HERE
  xbmd_descriptors.neg_link_width = xbmd_ep.cfg.link_width;
  xbmd_descriptors.neg_link_speed = xbmd_ep.cfg.link_speed;  

  // Switch statement below uses Negotiated Link Speed/Width to identify 
  // performance thresholds.  Program uses these thresholds to color code
  // the performance in GUI.  Link Speed/Width needed as performance is measured
  // in TRN_CLKs and TRN_CLK frequency varies depending upon link configuration.
  if (xbmd_descriptors.neg_link_speed == 1) {
    link_speed_mult = 2500;
  } else if (xbmd_descriptors.neg_link_speed == 2) {
    link_speed_mult = 5000;
  } else {
    fprintf(stderr, "Negoitated Link speed read not valid: Read LW = %d\n",xbmd_descriptors.neg_link_speed);
    return CRIT_ERR;
  }

  // The four equations below identify the performance thresholds which dicate color of performance text.  
  // The equations are based on the theoretical max which equals: Link Speed * Link Width * 80%.
  // Equation is multiplied by 80% due to 8B/10B encoding used in PCI Express GEN1/GEN2
  // Fraction at the end combines both 8B/10B reduction and margin off of theoretical max.
  // Color coding used to show user aproximately where they stand in relation to theoretical max.

  // ~80% of theoretical max
  green_base_wr = link_speed_mult * xbmd_descriptors.neg_link_width * 65/100;
  // ~70% of theoretical max
  yellow_base_wr = link_speed_mult * xbmd_descriptors.neg_link_width * 55/100;
  
  //~63% of theoretical max - slightly less performance expected for Read Completions when compared to Posted Writes
  green_base_rd = link_speed_mult * xbmd_descriptors.neg_link_width * 50/100;
  //~53% of theoretical max - slightly less performance expected for Read Completions when compared to Posted Writes
  yellow_base_rd = link_speed_mult * xbmd_descriptors.neg_link_width * 40/100;

  return SUCCESS;
}
} // END EXTERN C

#undef REPAINT_GUI



