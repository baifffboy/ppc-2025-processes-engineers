#include "yakimov_i_linear_virtual_topology/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <string>

#include "yakimov_i_linear_virtual_topology/common/include/common.hpp"  // Добавлен include

namespace yakimov_i_linear_virtual_topology {

YakimovILinearVirtualTopologyMPI::YakimovILinearVirtualTopologyMPI(const InType &in)
    : num_processes_{0} {  // Убрана инициализация rank_ и size_ (уже в заголовке)
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

bool YakimovILinearVirtualTopologyMPI::ValidationImpl() {
  int rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  return (rank == 0) ? (GetInput() > 0) : true;
}

bool YakimovILinearVirtualTopologyMPI::ReadOperationsFromFile(const std::string &filename) {
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
  return true;
}

void YakimovILinearVirtualTopologyMPI::BroadcastOperations() {
  int rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  int op_count = static_cast<int>(operations_.size());
  MPI_Bcast(&op_count, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (rank != 0) {
    operations_.resize(op_count);
  }
  MPI_Bcast(operations_.data(), op_count, MPI_INT, 0, MPI_COMM_WORLD);
}

bool YakimovILinearVirtualTopologyMPI::PreProcessingImpl() {
  int rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (rank == 0) {
    if (!ReadOperationsFromFile(data_filename_)) {
      return false;
    }
  }

  BroadcastOperations();

  return true;
}

bool YakimovILinearVirtualTopologyMPI::RunImpl() {
  InitializeMPI();

  int local_total = ProcessAllOperations();
  int global_total = CalculateGlobalTotal(local_total);

  SetOutput(global_total);
  return true;
}

void YakimovILinearVirtualTopologyMPI::InitializeMPI() {
  MPI_Comm_rank(MPI_COMM_WORLD, &rank_);
  MPI_Comm_size(MPI_COMM_WORLD, &size_);
  num_processes_ = size_;
}

int YakimovILinearVirtualTopologyMPI::ProcessAllOperations() {
  int local_total = 0;

  for (size_t i = 0; i < operations_.size(); i += 3) {
    int src_val = operations_[i];
    int dst_val = operations_[i + 1];
    int data_val = operations_[i + 2];

    local_total += ProcessSingleOperation(src_val, dst_val, data_val);
    MPI_Barrier(MPI_COMM_WORLD);
  }

  return local_total;
}

int YakimovILinearVirtualTopologyMPI::ProcessSingleOperation(int src, int dst, int data) {
  if (!IsValidOperation(src, dst)) {
    return 0;
  }

  if (src == dst) {
    return ProcessSameProcessOperation(src, data);
  }

  return ProcessDifferentProcessOperation(src, dst, data);
}

bool YakimovILinearVirtualTopologyMPI::IsValidOperation(int src, int dst) const {
  return src < size_ && dst < size_ && src >= 0 && dst >= 0;  // Исправлено по DeMorgan
}

int YakimovILinearVirtualTopologyMPI::ProcessSameProcessOperation(int process_id, int data) const {
  if (rank_ == process_id) {
    return data;
  }
  return 0;
}

int YakimovILinearVirtualTopologyMPI::ProcessDifferentProcessOperation(int src, int dst, int data) {
  int min_proc = std::min(src, dst);
  int max_proc = std::max(src, dst);

  if (rank_ < min_proc || rank_ > max_proc) {
    return 0;
  }

  int direction = CalculateDirection(src, dst);
  return ProcessDataTransfer(src, dst, data, direction);
}

int YakimovILinearVirtualTopologyMPI::CalculateDirection(int src, int dst) {
  return (dst > src) ? 1 : -1;
}

int YakimovILinearVirtualTopologyMPI::ProcessDataTransfer(int src, int dst, int data, int direction) {
  if (rank_ == src) {
    SendDataToNextProcess(data, direction);
    return 0;
  }

  if (rank_ == dst) {
    return ReceiveDataFromPreviousProcess(direction);
  }

  return ForwardDataBetweenProcesses(direction);
}

void YakimovILinearVirtualTopologyMPI::SendDataToNextProcess(int data, int direction) const {
  int next = rank_ + direction;
  MPI_Send(&data, 1, MPI_INT, next, 0, MPI_COMM_WORLD);
}

int YakimovILinearVirtualTopologyMPI::ReceiveDataFromPreviousProcess(int direction) const {
  int received = 0;
  MPI_Status status;
  int prev = rank_ - direction;

  MPI_Recv(&received, 1, MPI_INT, prev, 0, MPI_COMM_WORLD, &status);
  return received;
}

int YakimovILinearVirtualTopologyMPI::ForwardDataBetweenProcesses(int direction) const {
  int received = 0;
  MPI_Status status;
  int prev = rank_ - direction;
  int next = rank_ + direction;

  MPI_Recv(&received, 1, MPI_INT, prev, 0, MPI_COMM_WORLD, &status);
  MPI_Send(&received, 1, MPI_INT, next, 0, MPI_COMM_WORLD);
  return 0;
}

int YakimovILinearVirtualTopologyMPI::CalculateGlobalTotal(int local_total) {
  int global_total = 0;
  MPI_Reduce(&local_total, &global_total, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Bcast(&global_total, 1, MPI_INT, 0, MPI_COMM_WORLD);
  return global_total;
}

void YakimovILinearVirtualTopologyMPI::SetOutput(int global_total) {
  GetOutput() = std::abs(global_total);
}

bool YakimovILinearVirtualTopologyMPI::PostProcessingImpl() {
  return true;
}

}  // namespace yakimov_i_linear_virtual_topology
