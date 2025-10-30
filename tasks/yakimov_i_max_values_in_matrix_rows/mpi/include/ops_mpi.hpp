#pragma once

#include "yakimov_i_max_values_in_matrix_rows/common/include/common.hpp"
#include "task/include/task.hpp"

namespace yakimov_i_max_values_in_matrix_rows {

class YakimovIMaxValuesInMatrixRowsMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit YakimovIMaxValuesInMatrixRowsMPI(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace yakimov_i_max_values_in_matrix_rows
