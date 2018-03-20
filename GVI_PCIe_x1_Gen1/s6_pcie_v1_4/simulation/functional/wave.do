onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate -divider {SYS Interface}
add wave -noupdate -format Logic /board/EP/sys_clk_c
add wave -noupdate -format Logic /board/EP/sys_reset_n_c
add wave -noupdate -divider {TRN Common}
add wave -noupdate -format Logic /board/EP/trn_clk
add wave -noupdate -format Logic /board/EP/trn_reset_n
add wave -noupdate -format Logic /board/EP/trn_lnk_up_n
add wave -noupdate -format Literal /board/EP/trn_fc_sel
add wave -noupdate -format Literal /board/EP/trn_fc_cpld
add wave -noupdate -format Literal /board/EP/trn_fc_cplh
add wave -noupdate -format Literal /board/EP/trn_fc_npd
add wave -noupdate -format Literal /board/EP/trn_fc_nph
add wave -noupdate -format Literal /board/EP/trn_fc_pd
add wave -noupdate -format Literal /board/EP/trn_fc_ph
add wave -noupdate -divider {TRN Rx}
add wave -noupdate -format Literal /board/EP/trn_rbar_hit_n
add wave -noupdate -format Logic /board/EP/trn_rsrc_rdy_n
add wave -noupdate -format Logic /board/EP/trn_rdst_rdy_n
add wave -noupdate -format Logic /board/EP/trn_rsof_n
add wave -noupdate -format Logic /board/EP/trn_reof_n
add wave -noupdate -format Literal /board/EP/trn_rd
add wave -noupdate -format Logic /board/EP/trn_rerrfwd_n
add wave -noupdate -format Logic /board/EP/trn_rsrc_dsc_n
add wave -noupdate -format Logic /board/EP/trn_rnp_ok_n
add wave -noupdate -divider {TRN Tx}
add wave -noupdate -format Logic /board/EP/trn_tsrc_rdy_n
add wave -noupdate -format Logic /board/EP/trn_tdst_rdy_n
add wave -noupdate -format Logic /board/EP/trn_tsof_n
add wave -noupdate -format Logic /board/EP/trn_teof_n
add wave -noupdate -format Literal /board/EP/trn_td
add wave -noupdate -format Logic /board/EP/trn_tstr_n
add wave -noupdate -format Logic /board/EP/trn_terrfwd_n
add wave -noupdate -format Logic /board/EP/trn_tsrc_dsc_n
add wave -noupdate -format Logic /board/EP/trn_terr_drop_n
add wave -noupdate -format Literal /board/EP/trn_tbuf_av
add wave -noupdate -format Logic /board/EP/trn_tcfg_req_n
add wave -noupdate -format Logic /board/EP/trn_tcfg_gnt_n
TreeUpdate [SetDefaultTree]
update
