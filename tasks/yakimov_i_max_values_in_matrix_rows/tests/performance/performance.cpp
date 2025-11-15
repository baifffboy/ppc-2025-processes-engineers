#include <gtest/gtest.h>

#include "util/include/perf_test_util.hpp" // NOLINT(misc-include-cleaner)
#include "yakimov_i_max_values_in_matrix_rows/common/include/common.hpp"
#include "yakimov_i_max_values_in_matrix_rows/mpi/include/ops_mpi.hpp"
#include "yakimov_i_max_values_in_matrix_rows/seq/include/ops_seq.hpp"

namespace yakimov_i_max_values_in_matrix_rows {

class YakimovIMaxValuesInMatrixRowsPerfTests : public ppc::util::BaseRunPerfTests<InType, OutType> {
 protected:
  bool CheckTestOutputData(OutType &output_data) final { // NOLINT(readability-convert-member-functions-to-static)
    return output_data > 0;
  }

  InType GetTestInputData() final { // NOLINT(readability-convert-member-functions-to-static)
    return 30;
  }
};

TEST_P(YakimovIMaxValuesInMatrixRowsPerfTests, RunPerfModes) { // NOLINT(bugprone-throwing-static-initialization)
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks =
    ppc::util::MakeAllPerfTasks<InType, YakimovIMaxValuesInMatrixRowsMPI, YakimovIMaxValuesInMatrixRowsSEQ>(
        PPC_SETTINGS_yakimov_i_max_values_in_matrix_rows);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = YakimovIMaxValuesInMatrixRowsPerfTests::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, YakimovIMaxValuesInMatrixRowsPerfTests, kGtestValues, kPerfTestName); // NOLINT(bugprone-throwing-static-initialization)

}  // namespace yakimov_i_max_values_in_matrix_rows
