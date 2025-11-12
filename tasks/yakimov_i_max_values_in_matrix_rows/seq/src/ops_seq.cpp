#include "yakimov_i_max_values_in_matrix_rows/seq/include/ops_seq.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

#include "util/include/util.hpp"
#include "yakimov_i_max_values_in_matrix_rows/common/include/common.hpp"

namespace yakimov_i_max_values_in_matrix_rows {

YakimovIMaxValuesInMatrixRowsSEQ::YakimovIMaxValuesInMatrixRowsSEQ(const InType& in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0;
  std::filesystem::path exe_path = std::filesystem::read_symlink("/proc/self/exe");
  matrixFilename = exe_path.parent_path().string() + "/../../tasks/yakimov_i_max_values_in_matrix_rows/data/" +
                   std::to_string(GetInput()) + ".txt";
}

bool YakimovIMaxValuesInMatrixRowsSEQ::ValidationImpl() {
  return (GetInput() > 0) && (GetOutput() == 0);
}

bool YakimovIMaxValuesInMatrixRowsSEQ::PreProcessingImpl() {
  if (!ReadMatrixFromFile(matrixFilename)) {
    std::cerr << "Error: Cannot read matrix from file " << matrixFilename << std::endl;
    return false;
  }

  matrix.resize(rows);
  maxValues.resize(rows);
  return true;
}

bool YakimovIMaxValuesInMatrixRowsSEQ::ReadMatrixFromFile(const std::string& filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Error: Cannot open file " << filename << std::endl;
    return false;
  }

  file >> rows >> cols;

  if (rows == 0 || cols == 0) {
    std::cerr << "Error: Invalid matrix rows or columns in file " << filename << std::endl;
    return false;
  }

  matrix.resize(rows);
  for (size_t i = 0; i < rows; i++) {
    matrix[i].resize(cols);
  }

  for (size_t i = 0; i < rows; i++) {
    for (size_t j = 0; j < cols; j++) {
      if (!(file >> matrix[i][j])) {
        std::cerr << "Error: Cannot read matrix element on position " << i << ", " << j << std::endl;
        return false;
      }
    }
  }

  file.close();
  return true;
}

bool YakimovIMaxValuesInMatrixRowsSEQ::RunImpl() {
  if (matrix.empty()) {
    return false;
  }

  for (size_t i = 0; i < rows; i++) {
    if (matrix[i].empty()) {
      return false;
    }

    maxValues[i] = matrix[i][0];

    for (size_t j = 1; j < cols; j++) {
      if (matrix[i][j] > maxValues[i]) {
        maxValues[i] = matrix[i][j];
      }
    }
  }

  return true;
}

bool YakimovIMaxValuesInMatrixRowsSEQ::PostProcessingImpl() {
  if (!maxValues.empty()) {
    OutType result = 0;
    for (const auto& maxVal : maxValues) {
      result += maxVal;
    }
    GetOutput() = result;
    return true;
  } else {
    return false;
  }
}

}  // namespace yakimov_i_max_values_in_matrix_rows
