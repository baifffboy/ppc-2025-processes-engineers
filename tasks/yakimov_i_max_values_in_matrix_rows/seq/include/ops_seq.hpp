#pragma once

#include "yakimov_i_max_values_in_matrix_rows/common/include/common.hpp"
#include "task/include/task.hpp"

namespace yakimov_i_max_values_in_matrix_rows {

class YakimovIMaxValuesInMatrixRowsSEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }
  explicit YakimovIMaxValuesInMatrixRowsSEQ(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace yakimov_i_max_values_in_matrix_rows
