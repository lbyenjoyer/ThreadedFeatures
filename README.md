## ThreadedFeatures

**ThreadedFeatures** is a lightweight header-only C++20 feature manager designed to help you to register and manage application features, supporting both multi-threaded and single-threaded execution. It uses atomic booleans for safe toggling and makes it straightforward to register features that run once or repeatedly, on demand.
## Features

- Header-only: just include the file, no build step required
- Register features as either single-threaded or multithreaded.
- Toggle features on/off using `std::atomic<bool>`.
- Automatically handles thread management for features that require periodic work.
- [MIT License](LICENSE.txt)

## Usage

1. **Include the header**
    ```cpp
    #include "ThreadedFeatures.hpp"
    ```

2. **Register features**
    ```cpp
    std::atomic<bool> myFeatureFlag{false};

    // Register a regular (non-threaded) feature
    REG_FEATURE(myFeatureFlag, [](bool enabled) {
        if (enabled) {
            // Enable the feature
        } else {
            // Disable the feature
        }
    });

    // Register a threaded feature
    REG_FEATURE_IN_THREAD(myThreadedFeatureFlag, [](bool enabled) {
        // Called on toggle
    }, []() {
        // Called periodically when enabled
    }, 10); // Tick every 10 ms
    ```

3. **Update feature states**: 
    Call `FeatureManager::Instance().UpdateAll()` whenever you want to check for toggles and start/stop threads as needed. This is typically done in your application's main loop.

    ```cpp
    while (running) {
        FeatureManager::Instance().UpdateAll();
        // ... other work ...
    }
    ```

4. **Shutdown**: 
    Before exiting your application, stop all managed threads:
    ```cpp
    FeatureManager::Instance().StopAllThreads();
    ```

## API

### `FeatureEntry`
- Holds the data and callbacks for a feature.
- Can be constructed for normal or threaded use.

### `FeatureManager`
- `Register(name, FeatureEntry)`: Registers a feature.
- `UpdateAll()`: Checks toggle flags and manages threads.
- `StopAllThreads()`: Requests all running feature threads to stop.

### Macros
- `REG_FEATURE(name, toggle_cb)`: Register a feature without a thread.
- `REG_FEATURE_IN_THREAD(name, toggle_cb, tick_cb, tick_ms)`: Register a feature that runs `tick_cb` in a thread every `tick_ms` milliseconds when enabled.

## Notes

- C++20 required (`std::jthread`, lambdas, etc.)
- **Cross-platform?**  
  The code relies on standard C++20 features, which are available on modern Linux, macOS, and Windows compilers, and the STL, so it should be cross-platform on any system with a compliant compiler and standard library. 
  **However, platform compatibility ultimately depends on your toolchain’s C++20 support.**

## License

ThreadedFeatures is licensed under the [MIT License](LICENSE.txt).

---

**Note:** Always ensure your application cleans up threads (call `StopAllThreads()`) before shutdown to avoid resource leaks.

*If you have suggestions or improvements, feel free to open an issue or pull request!*
