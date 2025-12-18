#include "yakimov_i_linear_virtual_topology/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <filesystem>
#include <fstream>

namespace yakimov_i_linear_virtual_topology {

YakimovILinearVirtualTopologyMPI::YakimovILinearVirtualTopologyMPI(const InType &in) {
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

  MPI_Comm_size(MPI_COMM_WORLD, &num_processes_);

  return true;
}

bool YakimovILinearVirtualTopologyMPI::RunImpl() {
  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  num_processes_ = size;
  int total_received = 0;

  for (size_t i = 0; i < operations_.size(); i += 3) {
    int src = operations_[i];
    int dst = operations_[i + 1];
    int data = operations_[i + 2];

    if (src >= size || dst >= size || src < 0 || dst < 0) {
      MPI_Barrier(MPI_COMM_WORLD);
      continue;
    }

    if (src == dst) {
      if (rank == src) {
        total_received += data;
      }
      MPI_Barrier(MPI_COMM_WORLD);
      continue;
    }

    int min_proc = std::min(src, dst);
    int max_proc = std::max(src, dst);

    if (rank >= min_proc && rank <= max_proc) {
      int direction = (dst > src) ? 1 : -1;

      if (rank == src) {
        int next = rank + direction;
        MPI_Send(&data, 1, MPI_INT, next, 0, MPI_COMM_WORLD);
      } else if (rank == dst) {
        int received;
        MPI_Status status;
        int prev = rank - direction;
        MPI_Recv(&received, 1, MPI_INT, prev, 0, MPI_COMM_WORLD, &status);
        total_received += received;
      } else {
        int received;
        MPI_Status status;
        int prev = rank - direction;
        int next = rank + direction;

        MPI_Recv(&received, 1, MPI_INT, prev, 0, MPI_COMM_WORLD, &status);
        MPI_Send(&received, 1, MPI_INT, next, 0, MPI_COMM_WORLD);
      }
    }

    MPI_Barrier(MPI_COMM_WORLD);
  }

  int global_total = 0;
  MPI_Reduce(&total_received, &global_total, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Bcast(&global_total, 1, MPI_INT, 0, MPI_COMM_WORLD);

  GetOutput() = std::abs(global_total);

  return true;
}

bool YakimovILinearVirtualTopologyMPI::PostProcessingImpl() {
  return true;
}

}  // namespace yakimov_i_linear_virtual_topology
