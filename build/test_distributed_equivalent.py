# # Check cores
# num_cores=$(nproc --all)
#
# ## 1 core tests
# #bash run_assist.sh "test_runs/test_distributed_1_core_default" 1 || exit 1
# #bash run_assist.sh "test_runs/test_distributed_1_core_nullmsg" 1 || exit 1
# #
# ## 2 core tests (set to >= 4 because Travis does not support ns-3 MPI properly with more than 1 logical process)
# #if [ "${num_cores}" -ge "4" ]; then
# #  bash run_assist.sh "test_runs/test_distributed_2_core_default" 2 || exit 1
# #  bash run_assist.sh "test_runs/test_distributed_2_core_nullmsg" 2 || exit 1
# #fi
#
# return 0

exit(0)
