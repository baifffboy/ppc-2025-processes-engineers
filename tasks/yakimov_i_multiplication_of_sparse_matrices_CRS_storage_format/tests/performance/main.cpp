#include <gtest/gtest.h>

#include "yakimov_i_multiplication_of_sparse_matrices_CRS_storage_format/common/include/common.hpp"
#include "yakimov_i_multiplication_of_sparse_matrices_CRS_storage_format/mpi/include/ops_mpi.hpp"
#include "yakimov_i_multiplication_of_sparse_matrices_CRS_storage_format/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace yakimov_i_multiplication_of_sparse_matrices_CRS_storage_format {

class ExampleRunPerfTestProcesses3 : public ppc::util::BaseRunPerfTests<InType, OutType> {
  const int kCount_ = 100;
  InType input_data_{};

  void SetUp() override {
    input_data_ = kCount_;
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return input_data_ == output_data;
  }

  InType GetTestInputData() final {
    return input_data_;
  }
};

TEST_P(ExampleRunPerfTestProcesses3, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks =
    ppc::util::MakeAllPerfTasks<InType, YakimovIMultiplicationOfSparseMatricesCRSStorageFormatMPI, YakimovIMultiplicationOfSparseMatricesCRSStorageFormatSEQ>(PPC_SETTINGS_yakimov_i_multiplication_of_sparse_matrices_CRS_storage_format);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = ExampleRunPerfTestProcesses3::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, ExampleRunPerfTestProcesses3, kGtestValues, kPerfTestName);

}  // namespace yakimov_i_multiplication_of_sparse_matrices_CRS_storage_format
