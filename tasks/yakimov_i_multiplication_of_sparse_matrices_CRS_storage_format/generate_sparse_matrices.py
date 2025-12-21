import random
import os

def generate_matrix_file(rows, cols, density, filename):
    """Генерирует корректный файл матрицы в формате CRS"""
    with open(filename, 'w') as f:
        f.write(f"{rows} {cols}\n")
        
        for i in range(rows):
            # Определяем ненулевые элементы в строке
            max_nnz = int(cols * density)
            if max_nnz == 0:
                max_nnz = 1
            
            # Выбираем случайное количество ненулевых элементов
            nnz = random.randint(0, max_nnz)
            
            # Выбираем случайные столбцы
            if nnz > 0:
                cols_indices = random.sample(range(cols), nnz)
                cols_indices.sort()
            else:
                cols_indices = []
            
            f.write(f"{nnz} ")
            
            # Записываем индексы столбцов
            for col in cols_indices:
                f.write(f"{col} ")
            
            # Записываем значения
            for _ in cols_indices:
                value = random.uniform(-100.0, 100.0)
                f.write(f"{value:.6f} ")
            
            f.write("\n")

def generate_test_files():
    """Генерирует все тестовые файлы"""
    os.makedirs("data", exist_ok=True)
    
    # Маленькие тестовые матрицы (1-10)
    test_cases = [
        (3, 3, 0.3),    # 1.txt - маленькая квадратная
        (2, 4, 0.4),    # 2.txt - прямоугольная 2x4
        (4, 2, 0.4),    # 3.txt - прямоугольная 4x2
        (10, 10, 0.2),  # 4.txt - средняя квадратная
        (3, 5, 0.3),    # 5.txt - прямоугольная 3x5
        (1, 1, 1.0),    # 31.txt - крайний случай 1x1
        (1, 100, 0.1),  # 32.txt - крайний случай 1x100
        (100, 1, 0.1),  # 33.txt - крайний случай 100x1
        (5, 5, 0.1),    # 34.txt - маленькая с низкой плотностью
        (5, 5, 0.0),    # 35.txt - все нули
        (5, 5, 0.8),    # 36.txt - высокая плотность
    ]
    
    for i, (rows, cols, density) in enumerate(test_cases):
        idx = i + 1 if i < 5 else 30 + (i - 4)  # 1-5, 31-36
        generate_matrix_file(rows, cols, density, f"data/A_{idx}.txt")
        # Для матрицы B нужны совместимые размеры (cols_B = rows_A следующей матрицы)
        if i < len(test_cases) - 1:
            next_rows = test_cases[i + 1][0]
        else:
            next_rows = test_cases[0][0]
        generate_matrix_file(cols, next_rows, density, f"data/B_{idx}.txt")
    
    # Большие матрицы для производительности (27-30)
    for i in range(27, 31):
        rows = random.randint(3000, 5000)
        cols = random.randint(3000, 5000)
        generate_matrix_file(rows, cols, 0.01, f"data/A_{i}.txt")
        # Матрица B должна быть совместима с A (cols_A = rows_B)
        b_cols = random.randint(3000, 5000)
        generate_matrix_file(cols, b_cols, 0.01, f"data/B_{i}.txt")

if __name__ == "__main__":
    generate_test_files()
    print("Тестовые файлы успешно созданы!")