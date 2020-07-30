# Usage help
if [ "$1" == "--help" ]; then
  echo "Usage: bash tutorial.sh [--help]"
  exit 0
fi

# Rebuild
bash rebuild.sh || exit 1

# Tutorial
bash run_assist.sh "example_run_folders/tutorial" 0 || exit 1

# Go plot helpers
cd plot_helpers || exit 1

# Utilization plots
cd utilization_plot || exit 1
python utilization_plot.py ../../example_run_folders/tutorial/logs_ns3 ../../example_run_folders/tutorial/logs_ns3/data ../../example_run_folders/tutorial/logs_ns3/pdf 0 1 || exit 1
python utilization_plot.py ../../example_run_folders/tutorial/logs_ns3 ../../example_run_folders/tutorial/logs_ns3/data ../../example_run_folders/tutorial/logs_ns3/pdf 1 0 || exit 1
python utilization_plot.py ../../example_run_folders/tutorial/logs_ns3 ../../example_run_folders/tutorial/logs_ns3/data ../../example_run_folders/tutorial/logs_ns3/pdf 0 2 || exit 1
python utilization_plot.py ../../example_run_folders/tutorial/logs_ns3 ../../example_run_folders/tutorial/logs_ns3/data ../../example_run_folders/tutorial/logs_ns3/pdf 2 0 || exit 1
python utilization_plot.py ../../example_run_folders/tutorial/logs_ns3 ../../example_run_folders/tutorial/logs_ns3/data ../../example_run_folders/tutorial/logs_ns3/pdf 0 3 || exit 1
python utilization_plot.py ../../example_run_folders/tutorial/logs_ns3 ../../example_run_folders/tutorial/logs_ns3/data ../../example_run_folders/tutorial/logs_ns3/pdf 3 0 || exit 1
cd .. || exit 1

# Ping plots
cd ping_plot || exit 1
python ping_plot.py ../../example_run_folders/tutorial/logs_ns3 ../../example_run_folders/tutorial/logs_ns3/data ../../example_run_folders/tutorial/logs_ns3/pdf 1 2 100000000 || exit 1
python ping_plot.py ../../example_run_folders/tutorial/logs_ns3 ../../example_run_folders/tutorial/logs_ns3/data ../../example_run_folders/tutorial/logs_ns3/pdf 1 3 100000000 || exit 1
python ping_plot.py ../../example_run_folders/tutorial/logs_ns3 ../../example_run_folders/tutorial/logs_ns3/data ../../example_run_folders/tutorial/logs_ns3/pdf 2 1 100000000 || exit 1
python ping_plot.py ../../example_run_folders/tutorial/logs_ns3 ../../example_run_folders/tutorial/logs_ns3/data ../../example_run_folders/tutorial/logs_ns3/pdf 2 3 100000000 || exit 1
python ping_plot.py ../../example_run_folders/tutorial/logs_ns3 ../../example_run_folders/tutorial/logs_ns3/data ../../example_run_folders/tutorial/logs_ns3/pdf 3 1 100000000 || exit 1
python ping_plot.py ../../example_run_folders/tutorial/logs_ns3 ../../example_run_folders/tutorial/logs_ns3/data ../../example_run_folders/tutorial/logs_ns3/pdf 3 2 100000000 || exit 1
cd .. || exit 1

# TCP flow plots
cd tcp_flow_plot || exit 1
python tcp_flow_plot.py ../../example_run_folders/tutorial/logs_ns3 ../../example_run_folders/tutorial/logs_ns3/data ../../example_run_folders/tutorial/logs_ns3/pdf 0 100000000 || exit 1
python tcp_flow_plot.py ../../example_run_folders/tutorial/logs_ns3 ../../example_run_folders/tutorial/logs_ns3/data ../../example_run_folders/tutorial/logs_ns3/pdf 1 100000000 || exit 1
python tcp_flow_plot.py ../../example_run_folders/tutorial/logs_ns3 ../../example_run_folders/tutorial/logs_ns3/data ../../example_run_folders/tutorial/logs_ns3/pdf 2 100000000 || exit 1
cd .. || exit 1
