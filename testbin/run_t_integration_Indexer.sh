#!/bin/bash

LOG_FILE=t_integration_Indexer.log
LOG_LEVEL=test_suite

#CASES=t_TermDocFreqs
#CASES=t_TermDocFreqs/t_TermDocFreqs::index

GLOG_v=3 GLOG_logbuflevel=-1 ./t_integration_Indexer --log_level=$LOG_LEVEL --catch_system_errors=no 2>&1 | tee $LOG_FILE

#GLOG_v=3 GLOG_logbuflevel=-1 ./t_integration_Indexer --log_level=$LOG_LEVEL --catch_system_errors=no --run_test=$CASES --run_config_list 0 2>&1 | tee $LOG_FILE

#GLOG_v=3 GLOG_logbuflevel=-1 ./t_integration_Indexer --log_level=$LOG_LEVEL --catch_system_errors=no --run_test=$CASES --run_config_range 0 8 2>&1 | tee $LOG_FILE
