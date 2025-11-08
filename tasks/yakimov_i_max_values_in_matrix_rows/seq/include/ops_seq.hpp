#pragma once

#include "task/include/task.hpp"
#include "yakimov_i_max_values_in_matrix_rows/common/include/common.hpp"
#include <vector>
#include <string>

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
  bool ReadMatrixFromFile(const std::string& filename);

  std::vector<std::vector<InType>> matrix;
  std::vector<InType> maxValues;
  size_t rows;
  size_t cols;
  std::string matrixFilename;
};

}  // namespace yakimov_i_max_values_in_matrix_rows
