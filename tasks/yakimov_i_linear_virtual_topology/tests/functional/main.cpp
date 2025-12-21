#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <string>
#include <tuple>

#include "util/include/func_test_util.hpp"
#include "yakimov_i_linear_virtual_topology/common/include/common.hpp"
#include "yakimov_i_linear_virtual_topology/mpi/include/ops_mpi.hpp"
#include "yakimov_i_linear_virtual_topology/seq/include/ops_seq.hpp"

namespace yakimov_i_linear_virtual_topology {

class YakimovILinearVirtualTopologyFuncTests : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    return std::to_string(std::get<0>(test_param)) + "_" + std::get<1>(test_param);
  }

 protected:
  bool CheckTestOutputData(OutType &output_data) final {
    return output_data >= 0;
  }

  InType GetTestInputData() final {
    TestType params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());
    return std::get<0>(params);
  }
};

namespace {

TEST_P(YakimovILinearVirtualTopologyFuncTests, LinearTopology) {
  ExecuteTest(GetParam());
}

const std::array<TestType, 11> kAllTestParam = {
    std::make_tuple(1, "simple_transfer"),      std::make_tuple(2, "bidirectional"),
    std::make_tuple(3, "complex_routing"),      std::make_tuple(4, "neighbor_transfer"),
    std::make_tuple(5, "self_transfer"),        std::make_tuple(6, "edge_2_processes"),
    std::make_tuple(7, "edge_many_operations"), std::make_tuple(8, "edge_single_process"),
    std::make_tuple(9, "edge_only_neighbors"),  std::make_tuple(10, "edge_negative_values"),
    std::make_tuple(11, "edge_large_values")};

const auto kTestTasksList = std::tuple_cat(ppc::util::AddFuncTask<YakimovILinearVirtualTopologyMPI, InType>(
                                               kAllTestParam, PPC_SETTINGS_yakimov_i_linear_virtual_topology),
                                           ppc::util::AddFuncTask<YakimovILinearVirtualTopologySEQ, InType>(
                                               kAllTestParam, PPC_SETTINGS_yakimov_i_linear_virtual_topology));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kPerfTestName =
    YakimovILinearVirtualTopologyFuncTests::PrintFuncTestName<YakimovILinearVirtualTopologyFuncTests>;

INSTANTIATE_TEST_SUITE_P(TopologyTests, YakimovILinearVirtualTopologyFuncTests, kGtestValues, kPerfTestName);

}  // namespace

}  // namespace yakimov_i_linear_virtual_topology
