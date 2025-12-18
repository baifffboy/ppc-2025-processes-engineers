#include "yakimov_i_linear_virtual_topology/mpi/include/ops_mpi.hpp"
#include <mpi.h>

namespace yakimov_i_linear_virtual_topology {

YakimovILinearVirtualTopologyMPI::YakimovILinearVirtualTopologyMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0;
}

bool YakimovILinearVirtualTopologyMPI::ValidationImpl() { return true; }
bool YakimovILinearVirtualTopologyMPI::PreProcessingImpl() { return true; }

bool YakimovILinearVirtualTopologyMPI::RunImpl() {
  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  
  int left = (rank > 0) ? rank - 1 : MPI_PROC_NULL;
  int right = (rank < size - 1) ? rank + 1 : MPI_PROC_NULL;
  
  int send_value = rank * 10;
  int recv_from_left, recv_from_right;
  
  MPI_Sendrecv(&send_value, 1, MPI_INT, right, 0,
               &recv_from_left, 1, MPI_INT, left, 0,
               MPI_COMM_WORLD, MPI_STATUS_IGNORE);
   
  MPI_Sendrecv(&send_value, 1, MPI_INT, left, 1,
               &recv_from_right, 1, MPI_INT, right, 1,
               MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  
  return true;
}

bool YakimovILinearVirtualTopologyMPI::PostProcessingImpl() { return true; }

}  // namespace yakimov_i_linear_virtual_topology