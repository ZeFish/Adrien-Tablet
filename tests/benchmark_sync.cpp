#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include <future>

// Simulate the current implementation: Fixed Delay
long long scenario_fixed_delay(int loading_time_ms) {
    auto start = std::chrono::high_resolution_clock::now();

    // Simulate creating the task (which runs concurrently)
    std::thread t([loading_time_ms](){
        std::this_thread::sleep_for(std::chrono::milliseconds(loading_time_ms));
    });
    t.detach();

    // Main thread just sleeps for a fixed duration regardless of actual loading time
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));

    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

// Simulate the optimized implementation: Semaphore Synchronization
long long scenario_semaphore(int loading_time_ms) {
    auto start = std::chrono::high_resolution_clock::now();

    std::promise<void> signal;
    std::future<void> done = signal.get_future();

    std::thread t([&signal, loading_time_ms](){
        std::this_thread::sleep_for(std::chrono::milliseconds(loading_time_ms));
        signal.set_value(); // xSemaphoreGive
    });
    t.detach();

    // Main thread waits for the signal
    done.wait(); // xSemaphoreTake

    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

int main() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(200, 800); // Simulate loading takes 200-800ms

    const int iterations = 5;
    long long total_fixed = 0;
    long long total_sem = 0;

    std::cout << "Running Benchmark: Fixed Delay (1500ms) vs Synchronization\n";
    std::cout << "---------------------------------------------------------\n";
    std::cout << "Loading Time (Simulated) | Fixed Delay Time | Sync Time\n";

    for(int i=0; i<iterations; ++i) {
        int load_time = distrib(gen);

        long long t_fixed = scenario_fixed_delay(load_time);
        long long t_sem = scenario_semaphore(load_time);

        total_fixed += t_fixed;
        total_sem += t_sem;

        std::cout << "        " << load_time << "ms          |     " << t_fixed << "ms      |   " << t_sem << "ms\n";
    }

    std::cout << "---------------------------------------------------------\n";
    std::cout << "Average Time (Fixed): " << (total_fixed / iterations) << "ms\n";
    std::cout << "Average Time (Sync):  " << (total_sem / iterations) << "ms\n";
    std::cout << "Improvement: ~" << ((total_fixed - total_sem) / iterations) << "ms saved per boot.\n";

    return 0;
}
