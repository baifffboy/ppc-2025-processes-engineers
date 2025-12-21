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

  bool ReadOperationsFromFile(const std::string &filename);
  void ProcessAllOperations(std::vector<int> &process_data, int &total_received);
  static void ApplyArtificialDelay();
  [[nodiscard]] bool IsValidProcessId(int process_id) const;
  void ProcessSingleOperation(int src, int dst, int data, std::vector<int> &process_data, int &total_received);
  void HandleSameProcessTransfer(int process_id, int data, std::vector<int> &process_data, int &total_received);
  void HandleDifferentProcessTransfer(int src, int dst, int data, std::vector<int> &process_data, int &total_received);

  std::vector<int> operations_;
  int num_processes_{0};
  std::string data_filename_;
};

}  // namespace yakimov_i_linear_virtual_topology
