#include "yakimov_i_multiplication_of_sparse_matrices_crs_storage_format/seq/include/ops_seq.hpp"

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "yakimov_i_multiplication_of_sparse_matrices_crs_storage_format/common/include/common.hpp"

namespace yakimov_i_multiplication_of_sparse_matrices_crs_storage_format {

namespace {

bool ReadDimensions(std::ifstream &file, MatrixCRS &matrix) {
  bool success = true;
  file >> matrix.rows;
  file >> matrix.cols;

  if (matrix.rows <= 0) {
    success = false;
  }

  if (matrix.cols <= 0) {
    success = false;
  }

  return success;
}

bool ReadRowData(std::ifstream &file, MatrixCRS &matrix, int row_index) {
  int nonzeros = 0;
  file >> nonzeros;

  for (int j = 0; j < nonzeros; ++j) {
    int col_idx = 0;
    file >> col_idx;
    matrix.col_indices.push_back(col_idx);
  }

  for (int j = 0; j < nonzeros; ++j) {
    double value = 0.0;
    file >> value;
    matrix.values.push_back(value);
  }

  matrix.row_pointers[static_cast<size_t>(row_index + 1)] =
      matrix.row_pointers[static_cast<size_t>(row_index)] + nonzeros;

  return true;
}

bool ReadMatrixFromFileImpl(const std::string &filename, MatrixCRS &matrix) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    return false;
  }

  bool success = ReadDimensions(file, matrix);
  if (!success) {
    file.close();
    return false;
  }

  matrix.row_pointers.resize(static_cast<size_t>(matrix.rows + 1));
  matrix.row_pointers[0] = 0;

  for (int i = 0; i < matrix.rows; ++i) {
    success = success && ReadRowData(file, matrix, i);
  }

  file.close();
  return success;
}

void ProcessRowMultiplication(const MatrixCRS &a, const MatrixCRS &b, int row_index, std::vector<double> &row_values) {
  std::ranges::fill(row_values, 0.0);
  int row_start_a = a.row_pointers[static_cast<std::size_t>(row_index)];
  int row_end_a = a.row_pointers[static_cast<std::size_t>(row_index + 1)];
  for (int k = row_start_a; k < row_end_a; ++k) {
    int col_a = a.col_indices[static_cast<std::size_t>(k)];
    double val_a = a.values[static_cast<std::size_t>(k)];
    int row_start_b = b.row_pointers[static_cast<std::size_t>(col_a)];
    int row_end_b = b.row_pointers[static_cast<std::size_t>(col_a + 1)];
    for (int idx = row_start_b; idx < row_end_b; ++idx) {
      int col_b = b.col_indices[static_cast<std::size_t>(idx)];
      double val_b = b.values[static_cast<std::size_t>(idx)];
      row_values[static_cast<std::size_t>(col_b)] += val_a * val_b;
    }
  }
}

void CollectRowResult(const std::vector<double> &row_values, MatrixCRS &result, int row_index) {
  for (size_t j = 0; j < row_values.size(); ++j) {
    if (row_values[j] != 0.0) {
      result.values.push_back(row_values[j]);
      result.col_indices.push_back(static_cast<int>(j));
    }
  }

  result.row_pointers[static_cast<size_t>(row_index + 1)] = static_cast<int>(result.values.size());
}

MatrixCRS MultiplyMatricesImpl(const MatrixCRS &A, const MatrixCRS &B) {
  MatrixCRS result;
  result.rows = A.rows;
  result.cols = B.cols;
  result.row_pointers.resize(static_cast<size_t>(result.rows + 1));
  result.row_pointers[0] = 0;

  std::vector<double> row_values(static_cast<size_t>(result.cols), 0.0);

  for (int i = 0; i < A.rows; ++i) {
    ProcessRowMultiplication(A, B, i, row_values);
    CollectRowResult(row_values, result, i);
  }

  return result;
}

double SumMatrixElementsImpl(const MatrixCRS &matrix) {
  double sum = 0.0;

  for (size_t i = 0; i < matrix.values.size(); ++i) {
    sum += matrix.values[i];
  }

  return sum;
}

}  // namespace

YakimovIMultiplicationOfSparseMatricesSEQ::YakimovIMultiplicationOfSparseMatricesSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0.0;

  std::filesystem::path base_path = std::filesystem::current_path();
  while (base_path.filename() != "ppc-2025-processes-engineers") {
    base_path = base_path.parent_path();
  }

  std::string base_dir =
      base_path.string() + "/tasks/yakimov_i_multiplication_of_sparse_matrices_crs_storage_format/data/";
  matrix_A_filename_ = base_dir + "A_" + std::to_string(GetInput()) + ".txt";
  matrix_B_filename_ = base_dir + "B_" + std::to_string(GetInput()) + ".txt";
}

bool YakimovIMultiplicationOfSparseMatricesSEQ::ValidationImpl() {
  bool input_valid = (GetInput() > 0);
  bool output_valid = (GetOutput() == 0.0);
  return input_valid && output_valid;
}

bool YakimovIMultiplicationOfSparseMatricesSEQ::PreProcessingImpl() {
  bool success = true;

  success = success && ReadMatrixFromFileImpl(matrix_A_filename_, matrix_A_);
  success = success && ReadMatrixFromFileImpl(matrix_B_filename_, matrix_B_);

  if (!success) {
    return false;
  }

  if (matrix_A_.cols != matrix_B_.rows) {
    return false;
  }
  return true;
}

bool YakimovIMultiplicationOfSparseMatricesSEQ::RunImpl() {
  result_matrix_ = MultiplyMatricesImpl(matrix_A_, matrix_B_);
  return true;
}

bool YakimovIMultiplicationOfSparseMatricesSEQ::PostProcessingImpl() {
  double sum = SumMatrixElementsImpl(result_matrix_);
  GetOutput() = sum;
  return true;
}
}  // namespace yakimov_i_multiplication_of_sparse_matrices_crs_storage_format
