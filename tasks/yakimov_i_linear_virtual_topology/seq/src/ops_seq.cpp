#include "yakimov_i_linear_virtual_topology/seq/include/ops_seq.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

namespace yakimov_i_linear_virtual_topology {

YakimovILinearVirtualTopologySEQ::YakimovILinearVirtualTopologySEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = -1;
  std::filesystem::path base_path = std::filesystem::current_path();
  while (base_path.filename() != "ppc-2025-processes-engineers") {
    base_path = base_path.parent_path();
  }
  data_filename_ =
      base_path.string() + "/tasks/yakimov_i_linear_virtual_topology/data/" + std::to_string(GetInput()) + ".txt";
}

bool YakimovILinearVirtualTopologySEQ::ValidationImpl() {
  return (GetInput() > 0);
}

bool YakimovILinearVirtualTopologySEQ::PreProcessingImpl() {
  if (!ReadOperationsFromFile(data_filename_)) {
    return false;
  }
  return true;
}

bool YakimovILinearVirtualTopologySEQ::ReadOperationsFromFile(const std::string &filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    return false;
  }

  int src, dst, data;
  while (file >> src >> dst >> data) {
    operations_.push_back(src);
    operations_.push_back(dst);
    operations_.push_back(data);
  }

  file.close();

  num_processes_ = 0;
  for (size_t i = 0; i < operations_.size(); i += 3) {
    int src = operations_[i];
    int dst = operations_[i + 1];
    num_processes_ = std::max(num_processes_, std::max(src, dst) + 1);
  }

  return true;
}

bool YakimovILinearVirtualTopologySEQ::RunImpl() {
  int total_received = 0;

  for (size_t i = 0; i < operations_.size(); i += 3) {
    int src = operations_[i];
    int dst = operations_[i + 1];
    int data = operations_[i + 2];

    if (src >= num_processes_ || dst >= num_processes_ || src < 0 || dst < 0) {
      continue;
    }

    if (src == dst) {
      total_received += data;
    } else {
      total_received += data;
    }
  }

  GetOutput() = std::abs(total_received);

  return true;
}

bool YakimovILinearVirtualTopologySEQ::PostProcessingImpl() {
  return true;
}

}  // namespace yakimov_i_linear_virtual_topology
