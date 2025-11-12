#include <gtest/gtest.h>

#include "util/include/perf_test_util.hpp"
#include "yakimov_i_max_values_in_matrix_rows/common/include/common.hpp"
#include "yakimov_i_max_values_in_matrix_rows/mpi/include/ops_mpi.hpp"
#include "yakimov_i_max_values_in_matrix_rows/seq/include/ops_seq.hpp"

namespace yakimov_i_max_values_in_matrix_rows {

class yakimov_i_max_values_in_matrix_rows_Perf_Tests : public ppc::util::BaseRunPerfTests<InType, OutType> {
protected:

  bool CheckTestOutputData(OutType &output_data) final {
    return output_data > 0;
  }

  InType GetTestInputData() final {
    return 30;
  }

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
