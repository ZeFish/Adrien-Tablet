#include "core/system/ha_api.h"
#include <iostream>
#include <chrono>

int main() {
    std::cout << "Starting Blocking Benchmark..." << std::endl;
    auto start = std::chrono::high_resolution_clock::now();

    // Simulate 5 button presses
    for(int i=0; i<5; i++) {
        std::cout << "Call " << i << std::endl;
        ha_call_service("light", "turn_on", "light.test");
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    std::cout << "Total time for 5 calls: " << elapsed.count() << "s" << std::endl;
    std::cout << "Average time per call: " << elapsed.count() / 5.0 << "s" << std::endl;

    return 0;
}
