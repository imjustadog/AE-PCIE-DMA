#
# Simulator
#

database require simulator -hints {
    simulator "ncsim -gui work.board"
}

#
# groups
#
catch {group new -name {SYS Interface} -overlay 0}
catch {group new -name {TRN Common} -overlay 0}
catch {group new -name {TRN Rx} -overlay 0}
catch {group new -name {TRN Tx} -overlay 0}

group using {SYS Interface}
group set -overlay 0
group set -comment {}
group clear 0 end

group insert \
    board.EP.sys_clk_c \
    board.EP.sys_reset_n_c

group using {TRN Common}
group set -overlay 0
group set -comment {}
group clear 0 end

group insert \
    board.EP.trn_clk \
    board.EP.trn_reset_n \
    board.EP.trn_lnk_up_n \
    {board.EP.trn_fc_sel[2:0]} \
    {board.EP.trn_fc_cpld[11:0]} \
    {board.EP.trn_fc_cplh[7:0]} \
    {board.EP.trn_fc_npd[11:0]} \
    {board.EP.trn_fc_nph[7:0]} \
    {board.EP.trn_fc_pd[11:0]} \
    {board.EP.trn_fc_ph[7:0]}

group using {TRN Rx}
group set -overlay 0
group set -comment {}
group clear 0 end

group insert \
    {board.EP.trn_rbar_hit_n[6:0]} \
    board.EP.trn_rsrc_rdy_n \
    board.EP.trn_rdst_rdy_n \
    board.EP.trn_rsof_n \
    board.EP.trn_reof_n \
    {board.EP.trn_rd[31:0]} \
    board.EP.trn_rerrfwd_n \
    board.EP.trn_rsrc_dsc_n \
    board.EP.trn_rnp_ok_n

group using {TRN Tx}
group set -overlay 0
group set -comment {}
group clear 0 end

group insert \
    board.EP.trn_tsrc_rdy_n \
    board.EP.trn_tdst_rdy_n \
    board.EP.trn_tsof_n \
    board.EP.trn_teof_n \
    {board.EP.trn_td[31:0]} \
    board.EP.trn_tstr_n \
    board.EP.trn_terrfwd_n \
    board.EP.trn_tsrc_dsc_n \
    board.EP.trn_terr_drop_n \
    {board.EP.trn_tbuf_av[5:0]} \
    board.EP.trn_tcfg_req_n \
    board.EP.trn_tcfg_gnt_n

#
# Design Browser windows
#
if {[catch {window new WatchList -name "Design Browser 1" -geometry 700x500+0+462}] != ""} {
    window geometry "Design Browser 1" 700x500+0+462
}
window target "Design Browser 1" on
browser using {Design Browser 1}
browser set \
    -scope simulator::board.EP
browser yview see simulator::board.EP
browser timecontrol set -lock 0

#
# Waveform windows
#
if {[catch {window new WaveWindow -name "Waveform 1" -geometry 800x600+0+0}] != ""} {
    window geometry "Waveform 1" 800x600+0+0
}
window target "Waveform 1" on
waveform using {Waveform 1}
waveform sidebar visibility partial
waveform set \
    -primarycursor TimeA \
    -signalnames name \
    -signalwidth 175 \
    -units ns \
    -valuewidth 75
cursor set -using TimeA -time 0
cursor set -using TimeA -marching 1
waveform baseline set -time 0

set groupId [waveform add -groups {{SYS Interface}}]

set groupId [waveform add -groups {{TRN Common}}]

set groupId [waveform add -groups {{TRN Rx}}]

set groupId [waveform add -groups {{TRN Tx}}]


waveform xview limits 0 2000ns

#
# Console window
#
console set -windowname Console
