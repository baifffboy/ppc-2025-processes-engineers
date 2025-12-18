#include <gtest/gtest.h>

#include "yakimov_i_linear_virtual_topology/common/include/common.hpp"
#include "yakimov_i_linear_virtual_topology/mpi/include/ops_mpi.hpp"
#include "yakimov_i_linear_virtual_topology/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace yakimov_i_linear_virtual_topology {

class ExampleRunPerfTestProcesses2 : public ppc::util::BaseRunPerfTests<InType, OutType> {
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

TEST_P(ExampleRunPerfTestProcesses2, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks =
    ppc::util::MakeAllPerfTasks<InType, YakimovILinearVirtualTopologyMPI, YakimovILinearVirtualTopologySEQ>(PPC_SETTINGS_yakimov_i_linear_virtual_topology);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = ExampleRunPerfTestProcesses2::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, ExampleRunPerfTestProcesses2, kGtestValues, kPerfTestName);

}  // namespace yakimov_i_linear_virtual_topology
