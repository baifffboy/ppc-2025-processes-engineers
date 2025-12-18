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

  std::vector<int> operations_;  // [src1, dst1, data1, src2, dst2, data2, ...]
  int num_processes_;            // Количество процессов для топологии
  std::string data_filename_;
};

}  // namespace yakimov_i_linear_virtual_topology
