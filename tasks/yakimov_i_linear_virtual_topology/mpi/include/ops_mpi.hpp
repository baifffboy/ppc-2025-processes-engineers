#pragma once

#include "yakimov_i_linear_virtual_topology/common/include/common.hpp"
#include "task/include/task.hpp"

namespace yakimov_i_linear_virtual_topology {

class NesterovATestTaskMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit NesterovATestTaskMPI(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace yakimov_i_linear_virtual_topology
