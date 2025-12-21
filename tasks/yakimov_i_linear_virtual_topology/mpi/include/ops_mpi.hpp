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
  [[nodiscard]] bool IsValidOperation(int src, int dst) const;
  [[nodiscard]] int ProcessSameProcessOperation(int process_id, int data) const;
  int ProcessDifferentProcessOperation(int src, int dst, int data);
  [[nodiscard]] static int CalculateDirection(int src, int dst);
  int ProcessDataTransfer(int src, int dst, int data, int direction);
  void SendDataToNextProcess(int data, int direction) const;
  [[nodiscard]] int ReceiveDataFromPreviousProcess(int direction) const;
  [[nodiscard]] int ForwardDataBetweenProcesses(int direction) const;
  static int CalculateGlobalTotal(int local_total);
  void SetOutput(int global_total);

  std::vector<int> operations_;
  int num_processes_{0};
  std::string data_filename_;
  int rank_{0};
  int size_{0};
};

}  // namespace yakimov_i_linear_virtual_topology
