#pragma once

#include "yakimov_i_multiplication_of_sparse_matrices_CRS_storage_format/common/include/common.hpp"
#include "task/include/task.hpp"

namespace yakimov_i_multiplication_of_sparse_matrices_CRS_storage_format {

class YakimovIMultiplicationOfSparseMatricesCRSStorageFormatSEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }
  explicit YakimovIMultiplicationOfSparseMatricesCRSStorageFormatSEQ(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace yakimov_i_multiplication_of_sparse_matrices_CRS_storage_format
