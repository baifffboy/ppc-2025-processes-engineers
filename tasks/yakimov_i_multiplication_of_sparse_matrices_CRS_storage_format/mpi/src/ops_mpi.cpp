#include "yakimov_i_multiplication_of_sparse_matrices_CRS_storage_format/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "yakimov_i_multiplication_of_sparse_matrices_CRS_storage_format/common/include/common.hpp"

namespace yakimov_i_multiplication_of_sparse_matrices_CRS_storage_format {

namespace {

bool ReadDimensions(std::ifstream &file, MatrixCRS &matrix) {
  bool success = true;
  file >> matrix.rows;
  file >> matrix.cols;

  if (matrix.rows <= 0) {
    success = false;
  }

  if (matrix.cols <= 0) {
    success = false;
  }

  return success;
}

bool ReadRowData(std::ifstream &file, MatrixCRS &matrix, int row_index) {
  int nonzeros = 0;
  file >> nonzeros;

  for (int j = 0; j < nonzeros; ++j) {
    int col_idx = 0;
    file >> col_idx;
    matrix.col_indices.push_back(col_idx);
  }

  for (int j = 0; j < nonzeros; ++j) {
    double value = 0.0;
    file >> value;
    matrix.values.push_back(value);
  }

  matrix.row_pointers[static_cast<size_t>(row_index + 1)] =
      matrix.row_pointers[static_cast<size_t>(row_index)] + nonzeros;

  return true;
}

bool ReadMatrixFromFileImpl(const std::string &filename, MatrixCRS &matrix) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    return false;
  }

  bool success = ReadDimensions(file, matrix);
  if (!success) {
    file.close();
    return false;
  }

  matrix.row_pointers.resize(static_cast<size_t>(matrix.rows + 1));
  matrix.row_pointers[0] = 0;

  for (int i = 0; i < matrix.rows; ++i) {
    success = success && ReadRowData(file, matrix, i);
  }

  file.close();
  return success;
}

void ProcessRowMultiplication(const MatrixCRS &A, const MatrixCRS &B,
                              int row_index, std::vector<double> &row_values) {
  std::fill(row_values.begin(), row_values.end(), 0.0);

  int row_start_A = A.row_pointers[static_cast<size_t>(row_index)];
  int row_end_A = A.row_pointers[static_cast<size_t>(row_index + 1)];

  for (int k = row_start_A; k < row_end_A; ++k) {
    int col_A = A.col_indices[static_cast<size_t>(k)];
    double val_A = A.values[static_cast<size_t>(k)];

    int row_start_B = B.row_pointers[static_cast<size_t>(col_A)];
    int row_end_B = B.row_pointers[static_cast<size_t>(col_A + 1)];

    for (int l = row_start_B; l < row_end_B; ++l) {
      int col_B = B.col_indices[static_cast<size_t>(l)];
      double val_B = B.values[static_cast<size_t>(l)];
      row_values[static_cast<size_t>(col_B)] += val_A * val_B;
    }
  }
}

void CollectRowResult(const std::vector<double> &row_values,
                      MatrixCRS &result, int row_index) {
  for (size_t j = 0; j < row_values.size(); ++j) {
    if (row_values[j] != 0.0) {
      result.values.push_back(row_values[j]);
      result.col_indices.push_back(static_cast<int>(j));
    }
  }

  result.row_pointers[static_cast<size_t>(row_index + 1)] =
      static_cast<int>(result.values.size());
}

MatrixCRS MultiplyMatricesImpl(const MatrixCRS &A, const MatrixCRS &B) {
  MatrixCRS result;
  result.rows = A.rows;
  result.cols = B.cols;
  result.row_pointers.resize(static_cast<size_t>(result.rows + 1));
  result.row_pointers[0] = 0;

  std::vector<double> row_values(static_cast<size_t>(result.cols), 0.0);

  for (int i = 0; i < A.rows; ++i) {
    ProcessRowMultiplication(A, B, i, row_values);
    CollectRowResult(row_values, result, i);
  }

  return result;
}

double SumMatrixElementsImpl(const MatrixCRS &matrix) {
  double sum = 0.0;

  for (size_t i = 0; i < matrix.values.size(); ++i) {
    sum += matrix.values[i];
  }

  return sum;
}

std::vector<int> GetLocalRowsImpl(int rank, int size, int total_rows) {
  std::vector<int> local_rows;
  int rows_per_process = total_rows / size;
  int remainder = total_rows % size;

  int start_row = (rank * rows_per_process) + std::min(rank, remainder);
  int end_row = start_row + rows_per_process + (rank < remainder ? 1 : 0);

  for (int i = start_row; i < end_row; ++i) {
    local_rows.push_back(i);
  }

  return local_rows;
}

void PrepareRowDataForSending(const std::vector<int> &proc_rows,
                              const MatrixCRS &matrix_A,
                              std::vector<int> &row_nnz,
                              std::vector<int> &col_indices,
                              std::vector<double> &values) {
  for (int row_idx : proc_rows) {
    int row_start = matrix_A.row_pointers[static_cast<size_t>(row_idx)];
    int row_end = matrix_A.row_pointers[static_cast<size_t>(row_idx + 1)];
    int row_nnz_count = row_end - row_start;
    row_nnz.push_back(row_nnz_count);

    for (int i = row_start; i < row_end; ++i) {
      col_indices.push_back(matrix_A.col_indices[static_cast<size_t>(i)]);
    }

    for (int i = row_start; i < row_end; ++i) {
      values.push_back(matrix_A.values[static_cast<size_t>(i)]);
    }
  }
}

void SendRowDataToProcess(int proc, const std::vector<int> &row_nnz,
                          const std::vector<int> &col_indices,
                          const std::vector<double> &values) {
  int row_nnz_size = static_cast<int>(row_nnz.size());
  MPI_Send(row_nnz.data(), row_nnz_size, MPI_INT, proc, 1, MPI_COMM_WORLD);

  int col_indices_size = static_cast<int>(col_indices.size());
  MPI_Send(&col_indices_size, 1, MPI_INT, proc, 2, MPI_COMM_WORLD);
  
  if (col_indices_size > 0) {
    MPI_Send(col_indices.data(), col_indices_size, MPI_INT, proc, 3, MPI_COMM_WORLD);
  }

  int values_size = static_cast<int>(values.size());
  MPI_Send(&values_size, 1, MPI_INT, proc, 4, MPI_COMM_WORLD);
  
  if (values_size > 0) {
    MPI_Send(values.data(), values_size, MPI_DOUBLE, proc, 5, MPI_COMM_WORLD);
  }
}

void SendMatrixDataToProcess(int proc, const std::vector<int> &proc_rows,
                             const MatrixCRS &matrix_A) {
  int num_rows = static_cast<int>(proc_rows.size());
  MPI_Send(&num_rows, 1, MPI_INT, proc, 0, MPI_COMM_WORLD);

  if (num_rows > 0) {
    std::vector<int> row_nnz;
    std::vector<int> col_indices;
    std::vector<double> values;

    PrepareRowDataForSending(proc_rows, matrix_A, row_nnz, col_indices, values);
    SendRowDataToProcess(proc, row_nnz, col_indices, values);
  }
}

void ReceiveRowDataFromMaster(int num_rows,
                              std::vector<int> &row_nnz,
                              std::vector<int> &col_indices,
                              std::vector<double> &values) {
  row_nnz.resize(static_cast<size_t>(num_rows));
  MPI_Recv(row_nnz.data(), num_rows, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

  int col_indices_size = 0;
  MPI_Recv(&col_indices_size, 1, MPI_INT, 0, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  
  if (col_indices_size > 0) {
    col_indices.resize(static_cast<size_t>(col_indices_size));
    MPI_Recv(col_indices.data(), col_indices_size, MPI_INT, 0, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }

  int values_size = 0;
  MPI_Recv(&values_size, 1, MPI_INT, 0, 4, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  
  if (values_size > 0) {
    values.resize(static_cast<size_t>(values_size));
    MPI_Recv(values.data(), values_size, MPI_DOUBLE, 0, 5, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }
}

void BuildLocalMatrixFromReceivedData(const std::vector<int> &row_nnz,
                                      const std::vector<int> &col_indices,
                                      const std::vector<double> &values,
                                      MatrixCRS &local_A_rows) {
  local_A_rows.row_pointers.resize(static_cast<size_t>(local_A_rows.rows + 1));
  local_A_rows.row_pointers[0] = 0;

  size_t col_idx_pos = 0;
  size_t val_pos = 0;
  
  for (int i = 0; i < local_A_rows.rows; ++i) {
    int nnz = row_nnz[static_cast<size_t>(i)];
    
    for (int j = 0; j < nnz; ++j) {
      local_A_rows.col_indices.push_back(col_indices[col_idx_pos]);
      col_idx_pos++;
      local_A_rows.values.push_back(values[val_pos]);
      val_pos++;
    }
    
    local_A_rows.row_pointers[i + 1] = local_A_rows.row_pointers[i] + nnz;
  }
}

void ReceiveMatrixDataFromMaster(int rank, int size,
                                 MatrixCRS &local_A_rows,
                                 std::vector<int> &local_rows,
                                 int rows_A) {
  int num_rows = 0;
  MPI_Recv(&num_rows, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  local_rows = GetLocalRowsImpl(rank, size, rows_A);

  if (num_rows > 0) {
    std::vector<int> row_nnz;
    std::vector<int> col_indices;
    std::vector<double> values;

    local_A_rows.rows = num_rows;
    
    ReceiveRowDataFromMaster(num_rows, row_nnz, col_indices, values);
    BuildLocalMatrixFromReceivedData(row_nnz, col_indices, values, local_A_rows);
  }
}

void ProcessLocalRowForInitialization(int global_row,
                                      const MatrixCRS &matrix_A,
                                      MatrixCRS &local_A_rows,
                                      size_t local_index) {
  int row_start = matrix_A.row_pointers[static_cast<size_t>(global_row)];
  int row_end = matrix_A.row_pointers[static_cast<size_t>(global_row + 1)];
  int row_nnz_count = row_end - row_start;

  for (int j = row_start; j < row_end; ++j) {
    local_A_rows.col_indices.push_back(matrix_A.col_indices[static_cast<size_t>(j)]);
    local_A_rows.values.push_back(matrix_A.values[static_cast<size_t>(j)]);
  }

  local_A_rows.row_pointers[local_index + 1] =
      local_A_rows.row_pointers[local_index] + row_nnz_count;
}

void InitializeLocalRowsOnMaster(int rank, int size,
                                 MatrixCRS &matrix_A,
                                 MatrixCRS &local_A_rows,
                                 std::vector<int> &local_rows) {
  local_rows = GetLocalRowsImpl(rank, size, matrix_A.rows);
  local_A_rows.rows = static_cast<int>(local_rows.size());
  local_A_rows.cols = matrix_A.cols;
  local_A_rows.row_pointers.resize(static_cast<size_t>(local_A_rows.rows + 1));
  local_A_rows.row_pointers[0] = 0;

  for (size_t i = 0; i < local_rows.size(); ++i) {
    int global_row = local_rows[i];
    ProcessLocalRowForInitialization(global_row, matrix_A, local_A_rows, i);
  }
}

void GatherLocalRowCountsMaster(int rank,
                                const MatrixCRS &local_result,
                                const std::vector<int> &local_rows,
                                std::vector<int> &all_nnz_counts) {
  (void)rank;  // Явно помечаем неиспользуемый параметр
  
  std::vector<int> local_nnz_counts;
  
  for (size_t i = 0; i < local_rows.size(); ++i) {
    int row_nnz = local_result.row_pointers[i + 1] - local_result.row_pointers[i];
    local_nnz_counts.push_back(row_nnz);
  }

  for (size_t i = 0; i < local_rows.size(); ++i) {
    int global_row = local_rows[i];
    all_nnz_counts[static_cast<size_t>(global_row)] = local_nnz_counts[i];
  }
}

void GatherLocalRowCountsWorker(int rank,
                                const MatrixCRS &local_result,
                                const std::vector<int> &local_rows) {
  (void)rank;  // Явно помечаем неиспользуемый параметр
  
  std::vector<int> local_nnz_counts;
  
  for (size_t i = 0; i < local_rows.size(); ++i) {
    int row_nnz = local_result.row_pointers[i + 1] - local_result.row_pointers[i];
    local_nnz_counts.push_back(row_nnz);
  }

  if (!local_nnz_counts.empty()) {
    MPI_Send(local_nnz_counts.data(),
             static_cast<int>(local_nnz_counts.size()),
             MPI_INT, 0, 0, MPI_COMM_WORLD);
  } else {
    int dummy = 0;
    MPI_Send(&dummy, 0, MPI_INT, 0, 0, MPI_COMM_WORLD);
  }
}

void ReceiveRowCountsFromProcesses(int size,
                                   int rows_A,
                                   std::vector<int> &all_nnz_counts) {
  for (int proc = 1; proc < size; ++proc) {
    std::vector<int> proc_rows = GetLocalRowsImpl(proc, size, rows_A);
    int proc_num_rows = static_cast<int>(proc_rows.size());

    if (proc_num_rows > 0) {
      std::vector<int> proc_nnz_counts(static_cast<size_t>(proc_num_rows));
      MPI_Recv(proc_nnz_counts.data(), proc_num_rows, MPI_INT, proc, 0,
               MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      for (int i = 0; i < proc_num_rows; ++i) {
        int global_row = proc_rows[static_cast<size_t>(i)];
        all_nnz_counts[static_cast<size_t>(global_row)] =
            proc_nnz_counts[static_cast<size_t>(i)];
      }
    } else {
      int dummy;
      MPI_Recv(&dummy, 0, MPI_INT, proc, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
  }
}

void GatherRowCounts(int rank, int size,
                     const MatrixCRS &local_result,
                     MatrixCRS &result_matrix) {
  std::vector<int> local_rows = GetLocalRowsImpl(rank, size, result_matrix.rows);
  
  if (rank == 0) {
    std::vector<int> all_nnz_counts(static_cast<size_t>(result_matrix.rows), 0);
    
    GatherLocalRowCountsMaster(rank, local_result, local_rows, all_nnz_counts);
    ReceiveRowCountsFromProcesses(size, result_matrix.rows, all_nnz_counts);
    
    result_matrix.row_pointers[0] = 0;
    for (int i = 0; i < result_matrix.rows; ++i) {
      result_matrix.row_pointers[i + 1] =
          result_matrix.row_pointers[i] + all_nnz_counts[static_cast<size_t>(i)];
    }
    
    for (int proc = 1; proc < size; ++proc) {
      MPI_Send(result_matrix.row_pointers.data(),
               result_matrix.rows + 1,
               MPI_INT, proc, 1, MPI_COMM_WORLD);
    }
  } else {
    GatherLocalRowCountsWorker(rank, local_result, local_rows);
    
    MPI_Recv(result_matrix.row_pointers.data(),
             result_matrix.rows + 1,
             MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }
}

void SendLocalRowDataToMaster(const MatrixCRS &local_result,
                              const std::vector<int> &local_rows) {
  for (size_t i = 0; i < local_rows.size(); ++i) {
    int global_row = local_rows[i];
    int local_start = local_result.row_pointers[i];
    int local_end = local_result.row_pointers[i + 1];
    int row_nnz = local_end - local_start;
    
    if (row_nnz > 0) {
      MPI_Send(&local_result.col_indices[static_cast<size_t>(local_start)],
               row_nnz, MPI_INT, 0, global_row * 2, MPI_COMM_WORLD);
      MPI_Send(&local_result.values[static_cast<size_t>(local_start)],
               row_nnz, MPI_DOUBLE, 0, global_row * 2 + 1, MPI_COMM_WORLD);
    }
  }
}

void ReceiveRowDataFromProcess(int proc, int size,
                               MatrixCRS &result_matrix) {
  std::vector<int> proc_rows = GetLocalRowsImpl(proc, size, result_matrix.rows);
  
  for (size_t i = 0; i < proc_rows.size(); ++i) {
    int global_row = proc_rows[i];
    int row_start = result_matrix.row_pointers[static_cast<size_t>(global_row)];
    int row_end = result_matrix.row_pointers[static_cast<size_t>(global_row + 1)];
    int row_nnz = row_end - row_start;
    
    if (row_nnz > 0) {
      MPI_Recv(&result_matrix.col_indices[static_cast<size_t>(row_start)],
               row_nnz, MPI_INT, proc, global_row * 2,
               MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      MPI_Recv(&result_matrix.values[static_cast<size_t>(row_start)],
               row_nnz, MPI_DOUBLE, proc, global_row * 2 + 1,
               MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
  }
}

void GatherLocalRowDataOnMaster(const MatrixCRS &local_result,
                                const std::vector<int> &local_rows,
                                MatrixCRS &result_matrix) {
  for (size_t i = 0; i < local_rows.size(); ++i) {
    int global_row = local_rows[i];
    int local_start = local_result.row_pointers[i];
    int local_end = local_result.row_pointers[i + 1];
    int row_nnz = local_end - local_start;
    
    if (row_nnz > 0) {
      int row_start = result_matrix.row_pointers[static_cast<size_t>(global_row)];
      
      std::copy(local_result.col_indices.begin() + static_cast<size_t>(local_start),
                local_result.col_indices.begin() + static_cast<size_t>(local_end),
                result_matrix.col_indices.begin() + static_cast<size_t>(row_start));
      
      std::copy(local_result.values.begin() + static_cast<size_t>(local_start),
                local_result.values.begin() + static_cast<size_t>(local_end),
                result_matrix.values.begin() + static_cast<size_t>(row_start));
    }
  }
}

void GatherRowData(int rank, int size,
                   const MatrixCRS &local_result,
                   MatrixCRS &result_matrix) {
  std::vector<int> local_rows = GetLocalRowsImpl(rank, size, result_matrix.rows);
  
  if (rank == 0) {
    GatherLocalRowDataOnMaster(local_result, local_rows, result_matrix);
    
    for (int proc = 1; proc < size; ++proc) {
      ReceiveRowDataFromProcess(proc, size, result_matrix);
    }
  } else {
    SendLocalRowDataToMaster(local_result, local_rows);
  }
}

}  // namespace

YakimovIMultiplicationOfSparseMatricesMPI::YakimovIMultiplicationOfSparseMatricesMPI(const InType &in)
    : matrix_A_(),
      matrix_B_(),
      result_matrix_(),
      local_A_rows_(),
      local_result_(),
      matrix_A_filename_(""),
      matrix_B_filename_(""),
      local_rows_(),
      rows_A_(0),
      cols_A_(0),
      rows_B_(0),
      cols_B_(0) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0.0;

  std::filesystem::path base_path = std::filesystem::current_path();
  while (base_path.filename() != "ppc-2025-processes-engineers") {
    base_path = base_path.parent_path();
  }

  std::string base_dir =
      base_path.string() + "/tasks/yakimov_i_multiplication_of_sparse_matrices_CRS_storage_format/data/";
  matrix_A_filename_ = base_dir + "A_" + std::to_string(GetInput()) + ".txt";
  matrix_B_filename_ = base_dir + "B_" + std::to_string(GetInput()) + ".txt";
}

bool YakimovIMultiplicationOfSparseMatricesMPI::ValidationImpl() {
  int rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (rank == 0) {
    bool input_valid = (GetInput() > 0);
    bool output_valid = (GetOutput() == 0.0);
    return input_valid && output_valid;
  }

  return true;
}

bool YakimovIMultiplicationOfSparseMatricesMPI::PreProcessingImpl() {
  int rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  bool success = true;

  if (rank == 0) {
    success = success && ReadMatrixFromFile(matrix_A_filename_, matrix_A_);
    success = success && ReadMatrixFromFile(matrix_B_filename_, matrix_B_);

    if (!success) {
      return false;
    }

    if (matrix_A_.cols != matrix_B_.rows) {
      return false;
    }
  }

  BroadcastMatrixInfo();
  MPI_Barrier(MPI_COMM_WORLD);
  
  return success;
}

bool YakimovIMultiplicationOfSparseMatricesMPI::ReadMatrixFromFile(const std::string &filename, MatrixCRS &matrix) {
  return ReadMatrixFromFileImpl(filename, matrix);
}

void YakimovIMultiplicationOfSparseMatricesMPI::BroadcastMatrixInfo() {
  int rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  int matrix_info[4] = {0, 0, 0, 0};

  if (rank == 0) {
    matrix_info[0] = matrix_A_.rows;
    matrix_info[1] = matrix_A_.cols;
    matrix_info[2] = matrix_B_.rows;
    matrix_info[3] = matrix_B_.cols;
  }

  MPI_Bcast(matrix_info, 4, MPI_INT, 0, MPI_COMM_WORLD);

  rows_A_ = matrix_info[0];
  cols_A_ = matrix_info[1];
  rows_B_ = matrix_info[2];
  cols_B_ = matrix_info[3];
}

std::vector<int> YakimovIMultiplicationOfSparseMatricesMPI::GetLocalRows(int rank, int size, int total_rows) {
  return GetLocalRowsImpl(rank, size, total_rows);
}

void YakimovIMultiplicationOfSparseMatricesMPI::DistributeMatrixA() {
  int rank = 0;
  int size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if (rank == 0) {
    for (int proc = 1; proc < size; ++proc) {
      std::vector<int> proc_rows = GetLocalRows(proc, size, matrix_A_.rows);
      SendMatrixDataToProcess(proc, proc_rows, matrix_A_);
    }

    InitializeLocalRowsOnMaster(rank, size, matrix_A_, local_A_rows_, local_rows_);
  } else {
    ReceiveMatrixDataFromMaster(rank, size, local_A_rows_, local_rows_, rows_A_);
  }
}

void YakimovIMultiplicationOfSparseMatricesMPI::DistributeMatrixB() {
  int rank = 0;
  int size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if (rank == 0) {
    int total_nnz = static_cast<int>(matrix_B_.values.size());
    MPI_Bcast(&total_nnz, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (total_nnz > 0) {
      MPI_Bcast(matrix_B_.row_pointers.data(),
                matrix_B_.rows + 1, MPI_INT, 0, MPI_COMM_WORLD);
      MPI_Bcast(matrix_B_.col_indices.data(),
                total_nnz, MPI_INT, 0, MPI_COMM_WORLD);
      MPI_Bcast(matrix_B_.values.data(),
                total_nnz, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }

    matrix_B_.rows = rows_B_;
    matrix_B_.cols = cols_B_;
  } else {
    int total_nnz = 0;
    MPI_Bcast(&total_nnz, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (total_nnz > 0) {
      matrix_B_.rows = rows_B_;
      matrix_B_.cols = cols_B_;
      matrix_B_.row_pointers.resize(static_cast<size_t>(matrix_B_.rows + 1));
      matrix_B_.col_indices.resize(static_cast<size_t>(total_nnz));
      matrix_B_.values.resize(static_cast<size_t>(total_nnz));

      MPI_Bcast(matrix_B_.row_pointers.data(),
                matrix_B_.rows + 1, MPI_INT, 0, MPI_COMM_WORLD);
      MPI_Bcast(matrix_B_.col_indices.data(),
                total_nnz, MPI_INT, 0, MPI_COMM_WORLD);
      MPI_Bcast(matrix_B_.values.data(),
                total_nnz, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }
  }
}

MatrixCRS YakimovIMultiplicationOfSparseMatricesMPI::MultiplyLocalRows(const MatrixCRS &local_A_rows,
                                                                       const MatrixCRS &B) {
  return MultiplyMatricesImpl(local_A_rows, B);
}

void YakimovIMultiplicationOfSparseMatricesMPI::GatherResults() {
  int rank = 0;
  int size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if (rank == 0) {
    result_matrix_.rows = rows_A_;
    result_matrix_.cols = cols_B_;
  }
  
  int dims[2] = {rows_A_, cols_B_};
  MPI_Bcast(dims, 2, MPI_INT, 0, MPI_COMM_WORLD);
  
  if (rank != 0) {
    result_matrix_.rows = dims[0];
    result_matrix_.cols = dims[1];
  }
  
  result_matrix_.row_pointers.resize(static_cast<size_t>(result_matrix_.rows + 1));
  GatherRowCounts(rank, size, local_result_, result_matrix_);

  if (rank == 0) {
    int total_nnz = result_matrix_.row_pointers[static_cast<size_t>(rows_A_)];
    result_matrix_.col_indices.resize(static_cast<size_t>(total_nnz));
    result_matrix_.values.resize(static_cast<size_t>(total_nnz));
  }

  GatherRowData(rank, size, local_result_, result_matrix_);
}

bool YakimovIMultiplicationOfSparseMatricesMPI::RunImpl() {
  int rank = 0;
  int size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  DistributeMatrixA();
  DistributeMatrixB();
  MPI_Barrier(MPI_COMM_WORLD);

  local_result_ = MultiplyLocalRows(local_A_rows_, matrix_B_);
  MPI_Barrier(MPI_COMM_WORLD);

  GatherResults();

  return true;
}

bool YakimovIMultiplicationOfSparseMatricesMPI::PostProcessingImpl() {
  int rank = 0;
  int size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if (rank == 0) {
    double sum = SumMatrixElementsImpl(result_matrix_);
    GetOutput() = sum;
  }

  double final_result = 0.0;
  if (rank == 0) {
    final_result = GetOutput();
  }

  MPI_Bcast(&final_result, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  GetOutput() = final_result;

  return true;
}

}  // namespace yakimov_i_multiplication_of_sparse_matrices_CRS_storage_format