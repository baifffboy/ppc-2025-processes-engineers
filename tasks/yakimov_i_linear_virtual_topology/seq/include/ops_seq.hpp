#pragma once

#include <string>
#include <vector>

#include "task/include/task.hpp"
#include "yakimov_i_linear_virtual_topology/common/include/common.hpp"

namespace yakimov_i_linear_virtual_topology {

class YakimovILinearVirtualTopologySEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }
  explicit YakimovILinearVirtualTopologySEQ(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;

  void ProcessAllOperations(std::vector<int> &process_data, int &total_received);
  void ApplyArtificialDelay();
  bool IsValidProcessId(int process_id) const;
  void ProcessSingleOperation(int src, int dst, int data, std::vector<int> &process_data, int &total_received);
  void HandleSameProcessTransfer(int process_id, int data, std::vector<int> &process_data, int &total_received);
  void HandleDifferentProcessTransfer(int src, int dst, int data, std::vector<int> &process_data, int &total_received);
  bool ReadOperationsFromFile(const std::string &filename);

  std::vector<int> operations_;
  int num_processes_;
  std::string data_filename_;
};

}  // namespace yakimov_i_linear_virtual_topology
