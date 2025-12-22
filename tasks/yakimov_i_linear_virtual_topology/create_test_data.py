import random
import os

def create_test_file(filename, num_triplets, max_process=20, edge_case=None):
    with open(filename, 'w') as f:
        triplets = []
        
        if edge_case == "empty":
            # Пустой файл
            return
            
        elif edge_case == "invalid_process":
            for _ in range(num_triplets):
                sender = random.randint(25, 30)  # Неверные номера процессов
                receiver = random.randint(25, 30)
                data = random.randint(-100, 100)
                f.write(f"{sender} {receiver} {data}\n")
                
        elif edge_case == "large_data":
            for _ in range(num_triplets):
                sender = random.randint(0, max_process-1)
                receiver = random.randint(0, max_process-1)
                data = random.randint(100000, 1000000)
                f.write(f"{sender} {receiver} {data}\n")
                
        elif edge_case == "negative_values":
            for _ in range(num_triplets):
                sender = random.randint(0, max_process-1)
                receiver = random.randint(0, max_process-1)
                data = random.randint(-1000, -1)
                f.write(f"{sender} {receiver} {data}\n")
                
        elif edge_case == "mixed":
            for _ in range(num_triplets):
                sender = random.randint(-5, max_process+5)  # Смесь валидных и невалидных
                receiver = random.randint(-5, max_process+5)
                data = random.randint(-500, 500)
                f.write(f"{sender} {receiver} {data}\n")
                
        elif edge_case == "max_processes":
            for _ in range(num_triplets):
                sender = random.randint(0, max_process-1)
                receiver = random.randint(0, max_process-1)
                data = random.randint(1, 100)
                f.write(f"{sender} {receiver} {data}\n")
                
        else:
            # Обычный случай
            for _ in range(num_triplets):
                sender = random.randint(0, max_process-1)
                receiver = random.randint(0, max_process-1)
                data = random.randint(1, 100)
                f.write(f"{sender} {receiver} {data}\n")

def main():
    # Создаем директорию если ее нет
    os.makedirs("data", exist_ok=True)
    
    # Базовые тесты
    for i in range(1, 6):
        create_test_file(f"data/{i}.txt", random.randint(10, 50))
    
    # Производительность
    for i in range(27, 31):
        create_test_file(f"data/{i}.txt", random.randint(1000, 5000))
    
    # Краевые случаи
    create_test_file("data/31.txt", 0, edge_case="empty")
    create_test_file("data/32.txt", 20, edge_case="invalid_process")
    create_test_file("data/33.txt", 50, edge_case="large_data")
    create_test_file("data/34.txt", 30, edge_case="negative_values")
    create_test_file("data/35.txt", 40, edge_case="mixed")
    create_test_file("data/36.txt", 100, edge_case="max_processes")
    
    print("Test files created successfully!")

if __name__ == "__main__":
    main()