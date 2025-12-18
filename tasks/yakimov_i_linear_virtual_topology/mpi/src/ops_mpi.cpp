#include "yakimov_i_linear_virtual_topology/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <filesystem>
#include <fstream>

namespace yakimov_i_linear_virtual_topology {

YakimovILinearVirtualTopologyMPI::YakimovILinearVirtualTopologyMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0;

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

  file >> num_processes_;

  int src, dst, data;
  while (file >> src >> dst >> data) {
    operations_.push_back(src);
    operations_.push_back(dst);
    operations_.push_back(data);
  }

  file.close();
  return true;
}

void YakimovILinearVirtualTopologyMPI::BroadcastOperations() {
  int rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  MPI_Bcast(&num_processes_, 1, MPI_INT, 0, MPI_COMM_WORLD);

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

// Вспомогательные функции для RunImpl
namespace {

void DetermineLinearTopology(int rank, int num_processes, int &left, int &right) {
  left = (rank > 0) ? rank - 1 : MPI_PROC_NULL;
  right = (rank < num_processes - 1) ? rank + 1 : MPI_PROC_NULL;
}

bool HasEnoughProcesses(int size, int num_processes) {
  return size >= num_processes;
}

void ProcessSender(int rank, int src, int dst, int data, int right, int left, int &total_received) {
  if (rank == src) {
    if (src == dst) {
      total_received += data;
    } else {
      int direction = (dst > src) ? 1 : -1;
      int next = src + direction;

      if ((direction == 1 && right != MPI_PROC_NULL) || (direction == -1 && left != MPI_PROC_NULL)) {
        MPI_Send(&data, 1, MPI_INT, next, 0, MPI_COMM_WORLD);
      }
    }
  }
}

void ProcessReceiver(int rank, int src, int dst, int &total_received) {
  if (rank == dst && rank != src) {
    int received;
    MPI_Recv(&received, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    total_received += received;
  }
}

void ProcessIntermediate(int rank, int src, int dst, int right, int left) {
  if (rank != src && rank != dst) {
    MPI_Status status;
    int flag = 0;
    MPI_Iprobe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &flag, &status);

    if (flag) {
      int received;
      MPI_Recv(&received, 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      int direction = (dst > rank) ? 1 : -1;
      int next = rank + direction;

      if ((direction == 1 && right != MPI_PROC_NULL) || (direction == -1 && left != MPI_PROC_NULL)) {
        MPI_Send(&received, 1, MPI_INT, next, 0, MPI_COMM_WORLD);
      }
    }
  }
}

void ProcessAllOperations(int rank, const std::vector<int> &operations, int right, int left, int &total_received) {
  for (size_t i = 0; i < operations.size(); i += 3) {
    int src = operations[i];
    int dst = operations[i + 1];
    int data = operations[i + 2];

    ProcessSender(rank, src, dst, data, right, left, total_received);
    ProcessReceiver(rank, src, dst, total_received);
    ProcessIntermediate(rank, src, dst, right, left);
  }
}

void CollectAndDistributeResults(int rank, int total_received, OutType &result) {
  int global_total = 0;
  MPI_Reduce(&total_received, &global_total, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

  if (rank == 0) {
    result = global_total;
  }
  MPI_Bcast(&global_total, 1, MPI_INT, 0, MPI_COMM_WORLD);
  result = global_total;
}

}  // namespace

bool YakimovILinearVirtualTopologyMPI::RunImpl() {
  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if (!HasEnoughProcesses(size, num_processes_)) {
    return false;
  }

  int left, right;
  DetermineLinearTopology(rank, num_processes_, left, right);

  int total_received = 0;
  ProcessAllOperations(rank, operations_, right, left, total_received);

  CollectAndDistributeResults(rank, total_received, GetOutput());

  return true;
}

bool YakimovILinearVirtualTopologyMPI::PostProcessingImpl() {
  return true;
}

}  // namespace yakimov_i_linear_virtual_topology
