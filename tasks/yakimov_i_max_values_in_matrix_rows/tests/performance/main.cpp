#include <gtest/gtest.h>

#include <algorithm>
#include <fstream>
#include <string>
#include <vector>

#include "util/include/perf_test_util.hpp"
#include "yakimov_i_max_values_in_matrix_rows/common/include/common.hpp"
#include "yakimov_i_max_values_in_matrix_rows/mpi/include/ops_mpi.hpp"
#include "yakimov_i_max_values_in_matrix_rows/seq/include/ops_seq.hpp"

namespace yakimov_i_max_values_in_matrix_rows {

class yakimov_i_max_values_in_matrix_rows_Perf_Tests : public ppc::util::BaseRunPerfTests<InType, OutType> {
  const int kCount_ = 100;
  InType input_data_{};

  void SetUp() override {
    input_data_ = 25;
  }

  bool CheckTestOutputData(OutType &output_data) final {
    // Проверяем корректность вычисления максимумов по строкам
    std::string filename = "../../data/" + std::to_string(input_data_) + ".txt";
    std::ifstream file(filename);
    
    if (!file.is_open()) {
      return false;
    }
    
    size_t rows, cols;
    file >> rows >> cols;
    
    std::vector<std::vector<InType>> matrix(rows, std::vector<InType>(cols));
    std::vector<InType> expected_max_values(rows);
    
    // Читаем матрицу и вычисляем ожидаемые максимумы
    for (size_t i = 0; i < rows; i++) {
      expected_max_values[i] = std::numeric_limits<InType>::min();
      for (size_t j = 0; j < cols; j++) {
        file >> matrix[i][j];
        if (matrix[i][j] > expected_max_values[i]) {
          expected_max_values[i] = matrix[i][j];
        }
      }
    }
    file.close();
    
    // Проверяем, что результат соответствует ожидаемому
    // В данном случае output_data должен содержать сумму всех максимумов
    InType expected_sum = 0;
    for (const auto& max_val : expected_max_values) {
      expected_sum += max_val;
    }
    
    return output_data == expected_sum;
  }

  InType GetTestInputData() final {
    return input_data_;
  }

  private:
    InType input_data_ = 0;
};

TEST_P(yakimov_i_max_values_in_matrix_rows_Perf_Tests, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks =
    ppc::util::MakeAllPerfTasks<InType, YakimovIMaxValuesInMatrixRowsMPI, YakimovIMaxValuesInMatrixRowsSEQ>(
        PPC_SETTINGS_yakimov_i_max_values_in_matrix_rows);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = yakimov_i_max_values_in_matrix_rows_Perf_Tests::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, yakimov_i_max_values_in_matrix_rows_Perf_Tests, kGtestValues, kPerfTestName);

}  // namespace yakimov_i_max_values_in_matrix_rows
