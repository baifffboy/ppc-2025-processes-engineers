#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>

namespace fs = std::filesystem;

class MatrixGenerator {
 private:
  std::random_device rd;
  std::mt19937 gen;

 public:
  MatrixGenerator() : gen(rd()) {}

  void generateMatrixFile(const std::string& filename, int min_rows, int max_rows, int min_cols, int max_cols) {
    std::ofstream file(filename);
    if (!file.is_open()) {
      std::cerr << "Error: Cannot open file " << filename << std::endl;
      return;
    }

    // ���������� ��������� ������� ������� � �������� ��������
    std::uniform_int_distribution<int> rows_dist(min_rows, max_rows);
    std::uniform_int_distribution<int> cols_dist(min_cols, max_cols);
    std::uniform_int_distribution<int> value_dist(1, 1000);

    int rows = rows_dist(gen);
    int cols = cols_dist(gen);

    // ���������� ������� ������� � ������ ������
    file << rows << " " << cols << std::endl;

    // ���������� � ���������� �������
    for (int i = 0; i < rows; ++i) {
      for (int j = 0; j < cols; ++j) {
        file << value_dist(gen);
        if (j < cols - 1) {
          file << " ";
        }
      }
      file << std::endl;
    }

    file.close();
    std::cout << "Generated: " << filename << " (" << rows << "x" << cols << ")" << std::endl;
  }

  void generateAllMatrices(const std::string& output_dir, int count = 30) {
    // ������� ����������, ���� ��� �� ����������
    fs::create_directories(output_dir);

    // ������� ������� ��� ����� ��������� �������
    const int min_size_small = 100;
    const int max_size_small = 200;

    // ������� ��� ����� ������� �������
    const int min_size_large = 4500;
    const int max_size_large = 5000;

    for (int i = 1; i <= count; ++i) {
      // ��������� ���������� ��������
      double progress = static_cast<double>(i - 1) / (count - 1);

      // ������������� ������� �������� ��� ����� � ��������
      // ����� ������� �� ���� ����������� �����������
      int current_min_rows = min_size_small + static_cast<int>((min_size_large - min_size_small) * progress);
      int current_max_rows = max_size_small + static_cast<int>((max_size_large - max_size_small) * progress);

      // ��� �������� ���������� ������� ������ ���������� ��� ������������
      int current_min_cols = min_size_small + static_cast<int>((min_size_large - min_size_small) * progress * 0.9);
      int current_max_cols = max_size_small + static_cast<int>((max_size_large - max_size_small) * progress * 1.1);

      // �����������, ��� min <= max
      current_min_rows = std::min(current_min_rows, current_max_rows);
      current_min_cols = std::min(current_min_cols, current_max_cols);

      std::string filename = output_dir + "/" + std::to_string(i) + ".txt";
      generateMatrixFile(filename, current_min_rows, current_max_rows, current_min_cols, current_max_cols);
    }
  }
};

int main() {
  MatrixGenerator generator;

  // �������� ���� � ����������, ��� ��������� ����������� ����
  std::string exe_path = fs::current_path().string();
  std::string output_dir = exe_path;

  std::cout << "Generating 30 matrix files with random non-square sizes..." << std::endl;
  std::cout << "Size range: from ~100x100 to ~5000x5000" << std::endl;
  std::cout << "Executable path: " << exe_path << std::endl;
  std::cout << "Output directory: " << output_dir << std::endl;
  std::cout << "==========================================" << std::endl;

  generator.generateAllMatrices(output_dir, 30);

  std::cout << "==========================================" << std::endl;
  std::cout << "Matrix generation completed! 30 files created." << std::endl;
  std::cout << "Files location: " << output_dir << std::endl;

  return 0;
}
