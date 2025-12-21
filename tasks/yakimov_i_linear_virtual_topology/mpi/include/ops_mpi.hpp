#pragma once

#include <string>
#include <vector>

#include "task/include/task.hpp"
#include "yakimov_i_linear_virtual_topology/common/include/common.hpp"

namespace yakimov_i_linear_virtual_topology {

class YakimovILinearVirtualTopologyMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit YakimovILinearVirtualTopologyMPI(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;

  bool ReadOperationsFromFile(const std::string &filename);
  void BroadcastOperations();
  void InitializeMPI();
  int ProcessAllOperations();
  int ProcessSingleOperation(int src, int dst, int data);
  bool IsValidOperation(int src, int dst) const;
  int ProcessSameProcessOperation(int process_id, int data);
  int ProcessDifferentProcessOperation(int src, int dst, int data);
  int CalculateDirection(int src, int dst) const;
  int ProcessDataTransfer(int src, int dst, int data, int direction);
  void SendDataToNextProcess(int data, int direction);
  int ReceiveDataFromPreviousProcess(int direction);
  int ForwardDataBetweenProcesses(int direction);
  int CalculateGlobalTotal(int local_total);
  void SetOutput(int global_total);

  std::vector<int> operations_;
  int num_processes_;
  std::string data_filename_;
};

}  // namespace yakimov_i_linear_virtual_topology
