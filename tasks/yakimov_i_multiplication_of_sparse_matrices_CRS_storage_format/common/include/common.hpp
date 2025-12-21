#pragma once

#include <string>
#include <vector>

#include "task/include/task.hpp"

namespace yakimov_i_multiplication_of_sparse_matrices_CRS_storage_format {

using InType = int;
using OutType = double;
using TestType = std::tuple<int, std::string>;
using BaseTask = ppc::task::Task<InType, OutType>;

struct MatrixCRS {
  std::vector<double> values;
  std::vector<int> col_indices;
  std::vector<int> row_pointers;
  int rows;
  int cols;

  MatrixCRS() : rows(0), cols(0) {}
};

}  // namespace yakimov_i_multiplication_of_sparse_matrices_CRS_storage_format
