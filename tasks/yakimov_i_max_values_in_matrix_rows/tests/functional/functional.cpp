#include <gtest/gtest.h>

#include <array>
#include <cstddef> // NOLINT(misc-include-cleaner)
#include <string>
#include <tuple> 

#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp" // NOLINT(misc-include-cleaner)
#include "yakimov_i_max_values_in_matrix_rows/common/include/common.hpp"
#include "yakimov_i_max_values_in_matrix_rows/mpi/include/ops_mpi.hpp"
#include "yakimov_i_max_values_in_matrix_rows/seq/include/ops_seq.hpp"

namespace yakimov_i_max_values_in_matrix_rows {

class YakimovIMaxValuesInMatrixRowsFuncTests : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    return std::to_string(std::get<0>(test_param)) + "_" + std::get<1>(test_param);
  }

 protected:
  bool CheckTestOutputData(OutType &output_data) final { // NOLINT(readability-convert-member-functions-to-static)
    return output_data > 0;
  }

  InType GetTestInputData() final { // NOLINT(readability-convert-member-functions-to-static)
    TestType params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());
    return std::get<0>(params);
  }
};

namespace {

TEST_P(YakimovIMaxValuesInMatrixRowsFuncTests, MaxValuesInRows) { // NOLINT(bugprone-throwing-static-initialization)
  ExecuteTest(GetParam());
}

const std::array<TestType, 13> kTestParam = {
    std::make_tuple(1, "tiny"),      std::make_tuple(2, "very_small"), std::make_tuple(3, "small"),
    std::make_tuple(4, "compact"),   std::make_tuple(5, "modest"),     std::make_tuple(6, "medium"),
    std::make_tuple(7, "moderate"),  std::make_tuple(8, "standard"),   std::make_tuple(9, "large"),
    std::make_tuple(10, "generous"), std::make_tuple(11, "spacious"),  std::make_tuple(12, "very_large"),
    std::make_tuple(13, "extended")
    /*std::make_tuple(14, "expanded"),
    std::make_tuple(15, "substantial"),
    std::make_tuple(16, "considerable"),
    std::make_tuple(17, "ample"),
    std::make_tuple(18, "copious"),
    std::make_tuple(19, "abundant"),
    std::make_tuple(20, "plentiful"),
    std::make_tuple(21, "extensive"),
    std::make_tuple(22, "capacious"),
    std::make_tuple(23, "voluminous"),
    std::make_tuple(24, "commodious"),
    std::make_tuple(25, "expansive"),
    std::make_tuple(26, "roomy"),
    std::make_tuple(27, "sizable"),
    std::make_tuple(28, "massive"),
    std::make_tuple(29, "gigantic"),
    std::make_tuple(30, "colossal")*/
};

const auto kTestTasksList = std::tuple_cat(ppc::util::AddFuncTask<YakimovIMaxValuesInMatrixRowsMPI, InType>(
                                               kTestParam, PPC_SETTINGS_yakimov_i_max_values_in_matrix_rows),
                                           ppc::util::AddFuncTask<YakimovIMaxValuesInMatrixRowsSEQ, InType>(
                                               kTestParam, PPC_SETTINGS_yakimov_i_max_values_in_matrix_rows));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kPerfTestName =
    YakimovIMaxValuesInMatrixRowsFuncTests::PrintFuncTestName<YakimovIMaxValuesInMatrixRowsFuncTests>;

INSTANTIATE_TEST_SUITE_P(MatrixTests, YakimovIMaxValuesInMatrixRowsFuncTests, kGtestValues, kPerfTestName); // NOLINT(bugprone-throwing-static-initialization)

}  // namespace

}  // namespace yakimov_i_max_values_in_matrix_rows
