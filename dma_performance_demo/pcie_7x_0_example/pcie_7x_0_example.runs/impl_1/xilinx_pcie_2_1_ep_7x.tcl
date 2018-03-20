proc start_step { step } {
  set stopFile ".stop.rst"
  if {[file isfile .stop.rst]} {
    puts ""
    puts "*** Halting run - EA reset detected ***"
    puts ""
    puts ""
    return -code error
  }
  set beginFile ".$step.begin.rst"
  set platform "$::tcl_platform(platform)"
  set user "$::tcl_platform(user)"
  set pid [pid]
  set host ""
  if { [string equal $platform unix] } {
    if { [info exist ::env(HOSTNAME)] } {
      set host $::env(HOSTNAME)
    }
  } else {
    if { [info exist ::env(COMPUTERNAME)] } {
      set host $::env(COMPUTERNAME)
    }
  }
  set ch [open $beginFile w]
  puts $ch "<?xml version=\"1.0\"?>"
  puts $ch "<ProcessHandle Version=\"1\" Minor=\"0\">"
  puts $ch "    <Process Command=\".planAhead.\" Owner=\"$user\" Host=\"$host\" Pid=\"$pid\">"
  puts $ch "    </Process>"
  puts $ch "</ProcessHandle>"
  close $ch
}

proc end_step { step } {
  set endFile ".$step.end.rst"
  set ch [open $endFile w]
  close $ch
}

proc step_failed { step } {
  set endFile ".$step.error.rst"
  set ch [open $endFile w]
  close $ch
}

set_msg_config -id {HDL 9-1061} -limit 100000
set_msg_config -id {HDL 9-1654} -limit 100000
set_msg_config -id {Synth 8-256} -limit 10000
set_msg_config -id {Synth 8-638} -limit 10000

start_step init_design
set rc [catch {
  create_msg_db init_design.pb
  set_param gui.test TreeTableDev
  debug::add_scope template.lib 1
  create_project -in_memory -part xc7k325tffg900-2
  set_property board_part xilinx.com:kc705:part0:1.1 [current_project]
  set_property design_mode GateLvl [current_fileset]
  set_property webtalk.parent_dir c:/Work_projects/pcie_7x_0_example/pcie_7x_0_example.cache/wt [current_project]
  set_property parent.project_path c:/Work_projects/pcie_7x_0_example/pcie_7x_0_example.xpr [current_project]
  set_property ip_repo_paths c:/Work_projects/pcie_7x_0_example/pcie_7x_0_example.cache/ip [current_project]
  set_property ip_output_repo c:/Work_projects/pcie_7x_0_example/pcie_7x_0_example.cache/ip [current_project]
  add_files -quiet c:/Work_projects/pcie_7x_0_example/pcie_7x_0_example.runs/synth_1/xilinx_pcie_2_1_ep_7x.dcp
  add_files -quiet c:/Work_projects/pcie_7x_0_example/pcie_7x_0_example.srcs/sources_1/ip/pcie_7x_0/pcie_7x_0.dcp
  set_property netlist_only true [get_files c:/Work_projects/pcie_7x_0_example/pcie_7x_0_example.srcs/sources_1/ip/pcie_7x_0/pcie_7x_0.dcp]
  read_xdc -ref pcie_7x_0 -cells inst c:/Work_projects/pcie_7x_0_example/pcie_7x_0_example.srcs/sources_1/ip/pcie_7x_0/source/pcie_7x_0-PCIE_X0Y0.xdc
  set_property processing_order EARLY [get_files c:/Work_projects/pcie_7x_0_example/pcie_7x_0_example.srcs/sources_1/ip/pcie_7x_0/source/pcie_7x_0-PCIE_X0Y0.xdc]
  read_xdc c:/Work_projects/pcie_7x_0_example/pcie_7x_0_example.srcs/constrs_1/imports/example_design/xilinx_pcie_7x_ep_x8g2_KC705_REVC.xdc
  link_design -top xilinx_pcie_2_1_ep_7x -part xc7k325tffg900-2
  close_msg_db -file init_design.pb
} RESULT]
if {$rc} {
  step_failed init_design
  return -code error $RESULT
} else {
  end_step init_design
}

start_step opt_design
set rc [catch {
  create_msg_db opt_design.pb
  catch {write_debug_probes -quiet -force debug_nets}
  opt_design 
  write_checkpoint -force xilinx_pcie_2_1_ep_7x_opt.dcp
  catch {report_drc -file xilinx_pcie_2_1_ep_7x_drc_opted.rpt}
  close_msg_db -file opt_design.pb
} RESULT]
if {$rc} {
  step_failed opt_design
  return -code error $RESULT
} else {
  end_step opt_design
}

start_step place_design
set rc [catch {
  create_msg_db place_design.pb
  place_design 
  write_checkpoint -force xilinx_pcie_2_1_ep_7x_placed.dcp
  catch { report_io -file xilinx_pcie_2_1_ep_7x_io_placed.rpt }
  catch { report_clock_utilization -file xilinx_pcie_2_1_ep_7x_clock_utilization_placed.rpt }
  catch { report_utilization -file xilinx_pcie_2_1_ep_7x_utilization_placed.rpt -pb xilinx_pcie_2_1_ep_7x_utilization_placed.pb }
  catch { report_control_sets -verbose -file xilinx_pcie_2_1_ep_7x_control_sets_placed.rpt }
  close_msg_db -file place_design.pb
} RESULT]
if {$rc} {
  step_failed place_design
  return -code error $RESULT
} else {
  end_step place_design
}

start_step route_design
set rc [catch {
  create_msg_db route_design.pb
  route_design 
  write_checkpoint -force xilinx_pcie_2_1_ep_7x_routed.dcp
  catch { report_drc -file xilinx_pcie_2_1_ep_7x_drc_routed.rpt -pb xilinx_pcie_2_1_ep_7x_drc_routed.pb }
  catch { report_timing_summary -warn_on_violation -max_paths 10 -file xilinx_pcie_2_1_ep_7x_timing_summary_routed.rpt -rpx xilinx_pcie_2_1_ep_7x_timing_summary_routed.rpx }
  catch { report_power -file xilinx_pcie_2_1_ep_7x_power_routed.rpt -pb xilinx_pcie_2_1_ep_7x_power_summary_routed.pb }
  catch { report_route_status -file xilinx_pcie_2_1_ep_7x_route_status.rpt -pb xilinx_pcie_2_1_ep_7x_route_status.pb }
  close_msg_db -file route_design.pb
} RESULT]
if {$rc} {
  step_failed route_design
  return -code error $RESULT
} else {
  end_step route_design
}

start_step write_bitstream
set rc [catch {
  create_msg_db write_bitstream.pb
  write_bitstream -force xilinx_pcie_2_1_ep_7x.bit 
  if { [file exists c:/Work_projects/pcie_7x_0_example/pcie_7x_0_example.runs/synth_1/xilinx_pcie_2_1_ep_7x.hwdef] } {
    catch { write_sysdef -hwdef c:/Work_projects/pcie_7x_0_example/pcie_7x_0_example.runs/synth_1/xilinx_pcie_2_1_ep_7x.hwdef -bitfile xilinx_pcie_2_1_ep_7x.bit -meminfo xilinx_pcie_2_1_ep_7x.mmi -file xilinx_pcie_2_1_ep_7x.sysdef }
  }
  close_msg_db -file write_bitstream.pb
} RESULT]
if {$rc} {
  step_failed write_bitstream
  return -code error $RESULT
} else {
  end_step write_bitstream
}

