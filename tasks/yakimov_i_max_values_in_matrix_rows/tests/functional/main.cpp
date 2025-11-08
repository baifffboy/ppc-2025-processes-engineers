#include <gtest/gtest.h>    

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"
#include "yakimov_i_max_values_in_matrix_rows/common/include/common.hpp"
#include "yakimov_i_max_values_in_matrix_rows/mpi/include/ops_mpi.hpp"
#include "yakimov_i_max_values_in_matrix_rows/seq/include/ops_seq.hpp"

namespace yakimov_i_max_values_in_matrix_rows {

class yakimov_i_max_values_in_matrix_rows_Func_Tests : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    return std::to_string(std::get<0>(test_param)) + "_" + std::get<1>(test_param);
  }

 protected:
  void SetUp() override {
    TestType params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());
    input_data_ = std::get<0>(params);
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return output_data > 0;
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_ = 0;
};

namespace {

TEST_P(yakimov_i_max_values_in_matrix_rows_Func_Tests, MatmulFromPic) {
  ExecuteTest(GetParam());
}

const std::array<TestType, 5> kTestParam = {
  std::make_tuple(1, "very_small"),
  std::make_tuple(5, "small"), 
  std::make_tuple(10, "medium"),
  std::make_tuple(15, "large"),
  std::make_tuple(20, "very_large")
};

const auto kTestTasksList = std::tuple_cat(ppc::util::AddFuncTask<YakimovIMaxValuesInMatrixRowsMPI, InType>(
                                               kTestParam, PPC_SETTINGS_yakimov_i_max_values_in_matrix_rows),
                                           ppc::util::AddFuncTask<YakimovIMaxValuesInMatrixRowsSEQ, InType>(
                                               kTestParam, PPC_SETTINGS_yakimov_i_max_values_in_matrix_rows));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kPerfTestName =
    yakimov_i_max_values_in_matrix_rows_Func_Tests::PrintFuncTestName<yakimov_i_max_values_in_matrix_rows_Func_Tests>;

INSTANTIATE_TEST_SUITE_P(MatrixTests, yakimov_i_max_values_in_matrix_rows_Func_Tests, kGtestValues, kPerfTestName);

}  // namespace

}  // namespace yakimov_i_max_values_in_matrix_rows
