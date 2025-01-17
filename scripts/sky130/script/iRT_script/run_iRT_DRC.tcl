#===========================================================
##   init flow config
#===========================================================
flow_init -config ./iEDA_config/flow_config.json

#===========================================================
##   read db config
#===========================================================
db_init -config ./iEDA_config/db_default_config.json

#===========================================================
##   reset data path
#===========================================================
source ./script/DB_script/db_path_setting.tcl

#===========================================================
##   read lef
#===========================================================
source ./script/DB_script/db_init_lef.tcl

#===========================================================
##   read def
#===========================================================
def_init -path ./result/iRT_result.def

#===========================================================
##   run DRC
#===========================================================
run_drc -config ./iEDA_config/drc_default_config.json -path ./result/report/drc/iRT_drc.rpt
save_drc -path ./result/drc/detail.drc

#read_drc -path ./result/drc/detail.drc

#===========================================================
##   Exit 
#===========================================================
flow_exit
