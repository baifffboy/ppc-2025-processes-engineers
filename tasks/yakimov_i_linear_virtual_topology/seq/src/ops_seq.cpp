#include "yakimov_i_linear_virtual_topology/seq/include/ops_seq.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "yakimov_i_linear_virtual_topology/common/include/common.hpp"  // Добавлен include

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
  return ReadOperationsFromFile(data_filename_);
}

bool YakimovILinearVirtualTopologySEQ::ReadOperationsFromFile(const std::string &filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    return false;
  }

  int src_val = 0;
  int dst_val = 0;
  int data_val = 0;
  while (file >> src_val >> dst_val >> data_val) {
    operations_.push_back(src_val);
    operations_.push_back(dst_val);
    operations_.push_back(data_val);
  }

  file.close();

  num_processes_ = 0;
  for (size_t i = 0; i < operations_.size(); i += 3) {
    int current_src = operations_[i];      // Переименовано
    int current_dst = operations_[i + 1];  // Переименовано
    num_processes_ = std::max(num_processes_, std::max(current_src, current_dst) + 1);
  }

  return true;
}

bool YakimovILinearVirtualTopologySEQ::RunImpl() {
  std::vector<int> process_data(num_processes_, 0);
  int total_received = 0;

  ProcessAllOperations(process_data, total_received);

  GetOutput() = std::abs(total_received);
  return true;
}

void YakimovILinearVirtualTopologySEQ::ProcessAllOperations(std::vector<int> &process_data, int &total_received) {
  for (size_t i = 0; i < operations_.size(); i += 3) {
    int src_val = operations_[i];
    int dst_val = operations_[i + 1];
    int data_val = operations_[i + 2];

    ApplyArtificialDelay();

    if (!IsValidProcessId(src_val) || !IsValidProcessId(dst_val)) {
      continue;
    }

    ProcessSingleOperation(src_val, dst_val, data_val, process_data, total_received);
  }
}

// Статический метод - не использует члены класса
/*static*/ void YakimovILinearVirtualTopologySEQ::ApplyArtificialDelay() {
  volatile int delay = 0;
  for (int i = 0; i < 1e6; i++) {
    for (int j = 1; j < 10; j++) {
      delay += (i % j);
    }
  }
  (void)delay;
}

[[nodiscard]] bool YakimovILinearVirtualTopologySEQ::IsValidProcessId(int process_id) const {
  return process_id >= 0 && process_id < num_processes_;
}

void YakimovILinearVirtualTopologySEQ::ProcessSingleOperation(int src, int dst, int data,
                                                              std::vector<int> &process_data, int &total_received) {
  if (src == dst) {
    HandleSameProcessTransfer(src, data, process_data, total_received);
    return;
  }

  HandleDifferentProcessTransfer(src, dst, data, process_data, total_received);
}

// Этот метод НЕЛЬЗЯ сделать static - он изменяет параметры
void YakimovILinearVirtualTopologySEQ::HandleSameProcessTransfer(int process_id, int data,
                                                                 std::vector<int> &process_data, int &total_received) {
  process_data[process_id] += data;
  total_received += data;
}

void YakimovILinearVirtualTopologySEQ::HandleDifferentProcessTransfer(int src, int dst, int data,
                                                                      std::vector<int> &process_data,
                                                                      int &total_received) {
  int direction = (dst > src) ? 1 : -1;
  int current_process = src;
  int current_data = data;

  while (current_process != dst) {
    int next_process = current_process + direction;

    if (!IsValidProcessId(next_process)) {
      break;
    }

    if (next_process == dst) {
      process_data[dst] += current_data;
      total_received += current_data;
    }

    current_process = next_process;
  }
}

bool YakimovILinearVirtualTopologySEQ::PostProcessingImpl() {
  return true;
}

}  // namespace yakimov_i_linear_virtual_topology
