#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>

#include "../include/ThreadedFeatures.hpp"

std::atomic<bool> FeatureA{ false };
std::atomic<bool> FeatureB{ false };
std::atomic<bool> FeatureC{ false };
std::atomic<bool> FeatureD{ false };
std::atomic<bool> g_ExitThread{ false };

void InitFeatures();

int main() {
    InitFeatures(); // initializing features registration
    // Simulating feature toggle
    FeatureA = true;
    FeatureB = true;
    FeatureC = true;
    FeatureD = true;

    for (size_t i = 0; i < 10; ++i) {
        FeatureManager::Instance().UpdateAll();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        // Feature variables will update every 200ms
    }

    std::cout << "[SIMULATE] Turning FeatureB OFF" << std::endl;
    FeatureB = false;

    for (size_t i = 0; i < 10; ++i) {
        FeatureManager::Instance().UpdateAll();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    std::cout << "Stopping all threads and exiting." << std::endl;
    FeatureManager::Instance().StopAllThreads();

    return 0;
}

void Simplecallback(bool enable);
void Simpletickcallback(void);

void InitFeatures() {
    // register a simple toggleable feature
    REG_FEATURE(FeatureA,
        [](bool enabled) {
            // This callback is called when FeatureA is toggled
            // You can add your logic here, e.g., logging, enabling/disabling functionality, etc.
            std::cout << "[SIMULATE] FeatureA toggled to " << (enabled ? "ON" : "OFF") << std::endl;
    });

    // register a threaded feature
    // prints message every 500ms while enabled
    REG_FEATURE_IN_THREAD(FeatureB,
        [](bool enabled) { // toggle callback
			// This callback will be called once when FeatureB is toggled
            std::cout << "[OUTPUT] FeatureB thread status: " << (enabled ? "STARTED" : "STOPPED") << std::endl;
        },
        []() { // tick callback
			// This callback will be called every 500ms while FeatureB is enabled
            std::cout << "[THREAD] FeatureB ticking in thread..." << std::endl;
        },
        500 // milliseconds tick interval
    );

    // register a threaded feature without toggle callback, ticking every 1000ms
    REG_FEATURE_IN_THREAD(FeatureC,
        nullptr, // no callback
        []() {
			// this callback will be called every 1000ms while FeatureC is enabled
            std::cout << "[THREAD] FeatureC thread is ticking!" << std::endl;
        },
        1000 // milliseconds tick interval
    );

    REG_FEATURE_IN_THREAD(FeatureD,
        Simplecallback,
        Simpletickcallback,
        2000
        );

}

void Simplecallback(bool enable) {
    std::cout << "[OUTPUT] FeatureD thread is " << (enable ? "STARTED" : "STOPPED") << std::endl;
}
void Simpletickcallback(void) {
    std::cout << "[THREAD] FeatureD thread is ticking!" << std::endl;
}