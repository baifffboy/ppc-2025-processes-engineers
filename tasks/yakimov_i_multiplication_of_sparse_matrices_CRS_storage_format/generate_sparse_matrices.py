# generate_sparse_matrices.py
import numpy as np
import random
import os

def save_matrix_crs(filename, matrix):
    """Сохранить матрицу в формате CRS"""
    rows, cols = matrix.shape
    with open(filename, 'w') as f:
        # Размеры матрицы
        f.write(f"{rows} {cols}\n")
        
        for i in range(rows):
            row_values = []
            row_cols = []
            for j in range(cols):
                if matrix[i, j] != 0:
                    row_values.append(matrix[i, j])
                    row_cols.append(j)
            
            # Записываем количество ненулевых элементов в строке
            f.write(f"{len(row_values)} ")
            # Записываем индексы столбцов
            for col in row_cols:
                f.write(f"{col} ")
            # Записываем значения
            for val in row_values:
                f.write(f"{val:.6f} ")
            f.write("\n")

def generate_sparse_matrix(rows, cols, density=0.01):
    """Сгенерировать разреженную матрицу"""
    matrix = np.zeros((rows, cols), dtype=np.float64)
    total_elements = rows * cols
    nonzero_count = int(total_elements * density)
    
    # Генерируем ненулевые элементы
    for _ in range(nonzero_count):
        i = random.randint(0, rows - 1)
        j = random.randint(0, cols - 1)
        value = random.uniform(-100.0, 100.0)
        matrix[i, j] = value
    
    return matrix

def main():
    # Создаем директорию для данных
    data_dir = "data"
    os.makedirs(data_dir, exist_ok=True)
    
    # Функциональные тесты (маленькие матрицы)
    print("Generating functional tests...")
    test_cases = [
        # (rows, cols, density, test_id)
        (3, 3, 0.3, 1),      # Маленькая квадратная
        (2, 4, 0.4, 2),      # Прямоугольная
        (4, 2, 0.4, 3),      # Прямоугольная другая
        (5, 5, 0.2, 4),      # Средняя квадратная
        (3, 5, 0.25, 5),     # Прямоугольная
        (1, 1, 1.0, 31),     # Edge case: 1x1
        (1, 100, 0.05, 32),  # Edge case: 1x100
        (100, 1, 0.05, 33),  # Edge case: 100x1
        (10, 10, 0.1, 34),   # Edge case: маленькая
        (5, 5, 0.0, 35),     # Edge case: все нули
        (20, 20, 0.5, 36),   # Edge case: большая плотность
    ]
    
    for rows, cols, density, test_id in test_cases:
        matrix = generate_sparse_matrix(rows, cols, density)
        filename = os.path.join(data_dir, f"{test_id}.txt")
        save_matrix_crs(filename, matrix)
        print(f"Generated {filename}: {rows}x{cols}, density={density}")
    
    # Производительные тесты (большие матрицы)
    print("\nGenerating performance tests...")
    perf_cases = [
        (3000, 4000, 0.001, 27),
        (4000, 3000, 0.001, 28),
        (3500, 3500, 0.002, 29),
        (5000, 3000, 0.001, 30),
    ]
    
    for rows, cols, density, test_id in perf_cases:
        matrix = generate_sparse_matrix(rows, cols, density)
        filename = os.path.join(data_dir, f"{test_id}.txt")
        save_matrix_crs(filename, matrix)
        print(f"Generated {filename}: {rows}x{cols}, density={density}")
    
    # Генерация матриц для умножения (нужны две матрицы A и B)
    print("\nGenerating multiplication test cases...")
    # Для теста 1 (умножение 3x3 * 3x3)
    A = generate_sparse_matrix(3, 3, 0.3)
    B = generate_sparse_matrix(3, 3, 0.3)
    
    # Сохраняем как пары файлов
    for test_id in [1, 2, 3, 4, 5, 27, 28, 29, 30]:
        # Для каждого теста создаем соответствующую пару матриц
        if test_id <= 5:
            size = test_id + 2
            A = generate_sparse_matrix(size, size, 0.1 + test_id*0.05)
            B = generate_sparse_matrix(size, size, 0.1 + test_id*0.05)
        elif test_id >= 27:
            rows = [3000, 4000, 3500, 5000][test_id-27]
            cols = rows
            A = generate_sparse_matrix(rows, cols, 0.001)
            B = generate_sparse_matrix(cols, rows // 2, 0.001)
        
        # Сохраняем матрицу A
        filename_a = os.path.join(data_dir, f"A_{test_id}.txt")
        save_matrix_crs(filename_a, A)
        
        # Сохраняем матрицу B
        filename_b = os.path.join(data_dir, f"B_{test_id}.txt")
        save_matrix_crs(filename_b, B)
        
        print(f"Generated {filename_a} and {filename_b}")

if __name__ == "__main__":
    main()