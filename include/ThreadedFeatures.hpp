#pragma once

#include <atomic>
#include <functional>
#include <unordered_map>
#include <string>
#include <thread>
#include <chrono>
#include <mutex>

struct FeatureEntry {
    std::atomic<bool>* flag = nullptr;
    bool lastState = false;

    std::function<void(bool)> onToggle;
    std::function<void()> onTick;

    bool threaded = false;
    std::chrono::milliseconds tickRate{ 3 };
    std::jthread thread;

    FeatureEntry() = default;

    FeatureEntry(std::atomic<bool>* flagRef,
        std::function<void(bool)> toggleCallback,
        bool runInThread = false,
        std::function<void()> tickFunction = nullptr,
        std::chrono::milliseconds tickInterval = std::chrono::milliseconds(3))
        : flag(flagRef),
        onToggle(std::move(toggleCallback)),
        onTick(std::move(tickFunction)),
        threaded(runInThread),
        tickRate(tickInterval) {
    }
};

class FeatureManager {
public:
    static FeatureManager& Instance() {
        static FeatureManager instance;
        return instance;
    }

    FeatureManager(const FeatureManager&) = delete;
    FeatureManager& operator=(const FeatureManager&) = delete;

    void Register(const std::string& name, FeatureEntry&& entry) {
        std::scoped_lock lock(featuresMutex);
        features[name] = std::move(entry);
    }

    void UpdateAll() {
        std::scoped_lock lock(featuresMutex);
        for (auto& [name, entry] : features) {
            if (!entry.flag) continue;
            bool current = entry.flag->load();

            if (current != entry.lastState) {
                entry.lastState = current;

                if (entry.onToggle)
                    entry.onToggle(current);

                if (entry.threaded) {
                    if (entry.thread.joinable())
                        entry.thread.request_stop();

                    if (current && entry.onTick) {
                        entry.thread = std::jthread([tick = entry.onTick, rate = entry.tickRate](std::stop_token st) {
                            while (!st.stop_requested()) {
                                tick();
                                std::this_thread::sleep_for(rate);
                            }
                            });
                    }
                }
            }
        }
    }

    void StopAllThreads() {
        std::scoped_lock lock(featuresMutex);
        for (auto& [_, entry] : features) {
            if (entry.thread.joinable())
                entry.thread.request_stop();
        }
    }

private:
    FeatureManager() = default;

    std::unordered_map<std::string, FeatureEntry> features;
    std::mutex featuresMutex;
};

#define REG_FEATURE(name, toggle_cb) \
    FeatureManager::Instance().Register(#name, FeatureEntry(&name, toggle_cb, false))

#define REG_FEATURE_IN_THREAD(name, toggle_cb, tick_cb, tick_ms) \
    FeatureManager::Instance().Register(#name, FeatureEntry(&name, toggle_cb, true, tick_cb, std::chrono::milliseconds(tick_ms)))
