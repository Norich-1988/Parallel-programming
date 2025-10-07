#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <random>
#include <numeric>
#include <mutex>

// --- Константы---
const int NUM_MATRICES = 1000;
const int MATRIX_SIZE = 50;
const int RANDOM_MIN = -100;
const int RANDOM_MAX = 100;

// --- Псевдоним для типа матрицы ---
// Чтобы код был более читаемым, назову `std::vector<std::vector<int>>` просто `Matrix`
using Matrix = std::vector<std::vector<int>>;

std::mutex cout_mutex;

/**
 * @brief Вычисляет след одной матрицы.
 * След матрицы - это сумма элементов на ее главной диагонали.
 * @param matrix Матрица для вычисления.
 * @return След матрицы (целое число).
 */
long long calculate_trace(const Matrix& matrix) {
    long long trace = 0;
    // Идем по главной диагонали (i, i) и суммируем элементы
    for (int i = 0; i < MATRIX_SIZE; ++i) {
        trace += matrix[i][i];
    }
    return trace;
}

/**
 * @brief Функция, которую будет выполнять каждый поток.
 * Она вычисляет сумму следов для своего диапазона матриц.
 * @param matrices Ссылка на вектор всех матриц.
 * @param start_index Начальный индекс диапазона матриц для этого потока.
 * @param end_index Конечный индекс (не включая) диапазона.
 * @param result Ссылка на переменную, куда поток запишет свой результат.
 */
void trace_worker(const std::vector<Matrix>& matrices, int start_index, int end_index, long long& result) {
    long long partial_sum = 0;
    // Проходим по назначенному диапазону матриц
    for (int i = start_index; i < end_index; ++i) {
        partial_sum += calculate_trace(matrices[i]);
    }
    // Сохраняем результат
    result = partial_sum;
}

int main() {
    setlocale(LC_ALL, "Russian");

    std::cout << "Подготовка данных: создание " << NUM_MATRICES << " матриц размером "
              << MATRIX_SIZE << "x" << MATRIX_SIZE << "..." << std::endl;

    // --- 1. Подготовка данных (не входит в измеряемое время) ---
    std::vector<Matrix> matrices(NUM_MATRICES, Matrix(MATRIX_SIZE, std::vector<int>(MATRIX_SIZE)));

    // Настраиваем генератор случайных чисел
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(RANDOM_MIN, RANDOM_MAX);

    // Заполняем матрицы случайными числами
    for (int i = 0; i < NUM_MATRICES; ++i) {
        for (int j = 0; j < MATRIX_SIZE; ++j) {
            for (int k = 0; k < MATRIX_SIZE; ++k) {
                matrices[i][j][k] = distrib(gen);
            }
        }
    }
    std::cout << "Данные подготовлены." << std::endl << std::endl;

    // Список с количеством потоков, которые мы будем тестировать
    std::vector<int> thread_counts = {1, 2, 4, 8};

    // --- 2. Основной цикл для тестирования разного количества потоков ---
    for (int num_threads : thread_counts) {
        std::cout << "--- Запуск вычислений с " << num_threads << " потоком(-ами) ---" << std::endl;

        std::vector<std::thread> threads; // Вектор для хранения потоков
        std::vector<long long> partial_results(num_threads); // Вектор для хранения результатов от каждого потока

        // --- Начало измерения времени ---
        auto start_time = std::chrono::high_resolution_clock::now();

        // --- 3. Распределение работы и запуск потоков ---
        int matrices_per_thread = NUM_MATRICES / num_threads;
        for (int i = 0; i < num_threads; ++i) {
            int start_index = i * matrices_per_thread;
            // Последний поток забирает все оставшиеся матрицы, чтобы не потерять ничего из-за целочисленного деления
            int end_index = (i == num_threads - 1) ? NUM_MATRICES : start_index + matrices_per_thread;

            // Создаем и запускаем поток
            // std::ref используется для передачи аргументов по ссылке
            threads.emplace_back(trace_worker, std::ref(matrices), start_index, end_index, std::ref(partial_results[i]));
        }

        // --- 4. Ожидание завершения всех потоков ---
        // Главный поток будет ждать здесь, пока все дочерние потоки не закончат свою работу
        for (auto& t : threads) {
            t.join();
        }

        // --- Остановка измерения времени ---
        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed_time = end_time - start_time;

        // --- 5. Сбор и вывод результатов ---
        // Суммируем результаты от всех потоков
        long long total_trace = 0;
        for(long long res : partial_results) {
            total_trace += res;
        }

        // Выводим информацию
        std::cout << "Общий след всех матриц: " << total_trace << std::endl;
        std::cout << "Время выполнения: " << elapsed_time.count() << " мс" << std::endl;
        std::cout << "------------------------------------------" << std::endl << std::endl;
    }

    return 0;
}