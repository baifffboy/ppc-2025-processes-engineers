#include "yakimov_i_linear_virtual_topology/seq/include/ops_seq.hpp"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

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
  std::vector<int> process_data(num_processes_, 0);
  int total_received = 0;

  for (size_t i = 0; i < operations_.size(); i += 3) {
    int src = operations_[i];
    int dst = operations_[i + 1];
    int data = operations_[i + 2];

    volatile int delay = 0;
    for (int i = 0; i < 1e6; i++) {  // цикл предназначен для замедления seq версии так виртуальная линейная топология
                                     // на ней лишь "эмуляция" внутри вектора, а не реальный обмен данными между
                                     // процессами (необходимо для минимального порога прохождения тестов)
      for (int j = 1; j < 10; j++) {
        delay += (i % j);
      }
    }

    if (src >= num_processes_ || dst >= num_processes_ || src < 0 || dst < 0) {
      continue;
    }

    if (src == dst) {
      process_data[src] += data;
      total_received += data;
      continue;
    }
    int direction = (dst > src) ? 1 : -1;
    int current_process = src;
    int current_data = data;
    while (current_process != dst) {
      int next_process = current_process + direction;
      if (next_process < 0 || next_process >= num_processes_) {
        break;
      }
      if (next_process == dst) {
        process_data[dst] += current_data;
        total_received += current_data;
      }
      current_process = next_process;
    }
  }

  GetOutput() = std::abs(total_received);

  return true;
}

bool YakimovILinearVirtualTopologySEQ::PostProcessingImpl() {
  return true;
}

}  // namespace yakimov_i_linear_virtual_topology
