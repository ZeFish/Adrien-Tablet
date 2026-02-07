#include "core/system/ha_api.h"
#include <iostream>
#include <chrono>
#include <thread>

// This test expects the ha_call_service to be non-blocking.
// It measures the time taken by the main thread to issue commands.

int main() {
    std::cout << "Starting Async Benchmark..." << std::endl;

    // In the real system, SysInit calls ha_setup.
    // ha_call_service also lazily calls ha_setup if needed.

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

    // We need to wait a bit to let the background threads finish printing in the mock environment
    std::this_thread::sleep_for(std::chrono::seconds(2));

    return 0;
}
