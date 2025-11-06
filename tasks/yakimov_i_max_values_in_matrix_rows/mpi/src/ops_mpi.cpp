#include "yakimov_i_max_values_in_matrix_rows/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "util/include/util.hpp"
#include "yakimov_i_max_values_in_matrix_rows/common/include/common.hpp"

namespace yakimov_i_max_values_in_matrix_rows {

YakimovIMaxValuesInMatrixRowsMPI::YakimovIMaxValuesInMatrixRowsMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0;

  matrixFilename = "../../data/" + std::to_string(GetInput()) + ".txt";
}

bool YakimovIMaxValuesInMatrixRowsMPI::ValidationImpl() {
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  if (rank == 0) {
    return (GetInput() > 0) && (GetOutput() == 0);
  }
  return true;
}

bool YakimovIMaxValuesInMatrixRowsMPI::PreProcessingImpl() {
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  
  if (rank == 0) {
    if (!ReadMatrixFromFile(matrixFilename)) {
      std::cerr << "Error: Cannot read matrix from file " << matrixFilename << std::endl;
      return false;
    }

    maxValues.resize(rows, 0);
  }
  
  MPI_Barrier(MPI_COMM_WORLD);
  return true;
}

bool YakimovIMaxValuesInMatrixRowsMPI::ReadMatrixFromFile(const std::string& filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Error: Cannot open file " << filename << std::endl;
    return false;
  }
  
  file >> rows >> cols;
  
  if (rows == 0 || cols == 0) {
    std::cerr << "Error: Invalid matrix rows or columns in file " << filename << std::endl;
    return false;
  }
  
  matrix.resize(rows);
  for (size_t i = 0; i < rows; i++) {
    matrix[i].resize(cols);
  }
  
  for (size_t i = 0; i < rows; i++) {
    for (size_t j = 0; j < cols; j++) {
      if (!(file >> matrix[i][j])) {
        std::cerr << "Error: Cannot read matrix element on position " << i << ", " << j << std::endl;
        return false;
      }
    }
  }
  
  file.close(); 
  return true;
}

bool YakimovIMaxValuesInMatrixRowsMPI::RunImpl() {
  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  
  int matrix_dims[2] = {0, 0};
  if (rank == 0) {
    matrix_dims[0] = static_cast<int>(rows);
    matrix_dims[1] = static_cast<int>(cols);
  }
  
  MPI_Bcast(matrix_dims, 2, MPI_INT, 0, MPI_COMM_WORLD);
  int total_rows = matrix_dims[0];
  int total_cols = matrix_dims[1];
  
  if (total_rows == 0 || total_cols == 0) {
    return false;
  }
  
  int rows_per_process = total_rows / size;
  int remainder = total_rows % size;
  
  int start_row = rank * rows_per_process + std::min(rank, remainder);
  int end_row = start_row + rows_per_process + (rank < remainder ? 1 : 0);
  int local_rows = end_row - start_row;
  
  std::vector<InType> local_max_values(local_rows, 0);
  
  if (rank == 0) {
    for (int i = 0; i < local_rows; i++) {
      local_max_values[i] = matrix[start_row + i][0];
      for (int j = 1; j < total_cols; j++) {
        if (matrix[start_row + i][j] > local_max_values[i]) {
          local_max_values[i] = matrix[start_row + i][j];
        }
      }
    }
    
    for (int proc = 1; proc < size; proc++) {
      int proc_start = proc * rows_per_process + std::min(proc, remainder);
      int proc_rows = rows_per_process + (proc < remainder ? 1 : 0);
      
      std::vector<InType> proc_max_values(proc_rows);
      MPI_Recv(proc_max_values.data(), proc_rows, MPI_INT, proc, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      
      for (int i = 0; i < proc_rows; i++) {
        maxValues[proc_start + i] = proc_max_values[i];
      }
    }
    
    for (int i = 0; i < local_rows; i++) {
      maxValues[start_row + i] = local_max_values[i];
    }
    
  } else {
    std::vector<std::vector<InType>> local_matrix(local_rows, std::vector<InType>(total_cols));
    
    for (int i = 0; i < local_rows; i++) {
      MPI_Recv(local_matrix[i].data(), total_cols, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    
    for (int i = 0; i < local_rows; i++) {
      local_max_values[i] = local_matrix[i][0];
      for (int j = 1; j < total_cols; j++) {
        if (local_matrix[i][j] > local_max_values[i]) {
          local_max_values[i] = local_matrix[i][j];
        }
      }
    }
    
    MPI_Send(local_max_values.data(), local_rows, MPI_INT, 0, 0, MPI_COMM_WORLD);
  }
  
  if (rank == 0) {
    for (int proc = 1; proc < size; proc++) {
      int proc_start = proc * rows_per_process + std::min(proc, remainder);
      int proc_rows = rows_per_process + (proc < remainder ? 1 : 0);
      
      for (int i = 0; i < proc_rows; i++) {
        MPI_Send(matrix[proc_start + i].data(), total_cols, MPI_INT, proc, 0, MPI_COMM_WORLD);
      }
    }
  }
  
  MPI_Barrier(MPI_COMM_WORLD);
  return true;
}

bool YakimovIMaxValuesInMatrixRowsMPI::PostProcessingImpl() {
  return true;
}

}  // namespace yakimov_i_max_values_in_matrix_rows
