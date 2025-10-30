#include "yakimov_i_max_values_in_matrix_rows/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <numeric>
#include <vector>

#include "yakimov_i_max_values_in_matrix_rows/common/include/common.hpp"
#include "util/include/util.hpp"

namespace yakimov_i_max_values_in_matrix_rows {

YakimovIMaxValuesInMatrixRowsMPI::YakimovIMaxValuesInMatrixRowsMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0;
}

bool YakimovIMaxValuesInMatrixRowsMPI::ValidationImpl() {
  return (GetInput() > 0) && (GetOutput() == 0);
}

bool YakimovIMaxValuesInMatrixRowsMPI::PreProcessingImpl() {
  GetOutput() = 2 * GetInput();
  return GetOutput() > 0;
}

bool YakimovIMaxValuesInMatrixRowsMPI::RunImpl() {
  auto input = GetInput();
  if (input == 0) {
    return false;
  }

  for (InType i = 0; i < GetInput(); i++) {
    for (InType j = 0; j < GetInput(); j++) {
      for (InType k = 0; k < GetInput(); k++) {
        std::vector<InType> tmp(i + j + k, 1);
        GetOutput() += std::accumulate(tmp.begin(), tmp.end(), 0);
        GetOutput() -= i + j + k;
      }
    }
  }

  const int num_threads = ppc::util::GetNumThreads();
  GetOutput() *= num_threads;

  int rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (rank == 0) {
    GetOutput() /= num_threads;
  } else {
    int counter = 0;
    for (int i = 0; i < num_threads; i++) {
      counter++;
    }

    if (counter != 0) {
      GetOutput() /= counter;
    }
  }

  MPI_Barrier(MPI_COMM_WORLD);
  return GetOutput() > 0;
}

bool YakimovIMaxValuesInMatrixRowsMPI::PostProcessingImpl() {
  GetOutput() -= GetInput();
  return GetOutput() > 0;
}

}  // namespace yakimov_i_max_values_in_matrix_rows
