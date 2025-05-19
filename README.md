# Pipeline Project

A multithreaded data processing pipeline library in C++, designed to pass shared data through a sequence of filters in separate threads using thread-safe queues.

## Project Structure

```
PIPELINE PROJECT
├── CMakeLists.txt           # Top-level CMake build configuration
├── src                      # Source code for the pipeline library
│   ├── CMakeLists.txt       # CMake configuration for building the library
│   └── pipeline
│       ├── CMakeLists.txt   # CMake configuration for the pipeline module
│       ├── include
│       │   └── pipeline      # Public headers for the pipeline
│       │       ├── pipeline.h
│       │       ├── threadsafequeue.h
│       │       └── threadsafequeue.tpp Implementation of thread-safe queue
│       └── pipeline.cpp     # Implementation of the pipeline
└── tests                    # Unit tests for the pipeline library
    ├── CMakeLists.txt       # CMake configuration for the tests
    ├── filter_mock.h        # Mocks for pipeline filters
    └── pipeline_test.cpp    # Test cases using Google Test/GMock
```

## Prerequisites

* C++17 compiler (e.g., GCC 7+, Clang 5+, MSVC 2017+)
* CMake 3.10 or higher
* Google Test & Google Mock (fetched via `FetchContent` in tests)

## Building the Library

1. Create and navigate to a build directory:

   ```bash
   mkdir build && cd build
   ```
2. Configure with CMake:

   ```bash
   cmake ..
   ```
3. Build the static library:

   ```bash
   cmake --build .
   ```
4. The output `libpipeline.a` (or `libpipeline.lib` on Windows) will be in the build directory.

## Using the Library in Another Project

1. In your project's `CMakeLists.txt`, add:

   ```cmake
   add_subdirectory(path/to/PIPELINE_PROJECT/src)

   target_include_directories(YourApp PRIVATE
       ${CMAKE_SOURCE_DIR}/path/to/PIPELINE_PROJECT/src/pipeline/include
   )
   target_link_libraries(YourApp PRIVATE pipeline)
   ```
2. Include headers in your code:

   ```cpp
   #include <pipeline/pipeline.h>
   ```
3. Link against `pipeline` when building your application.

## Running Tests

From the top-level build folder:

```bash
cmake --build . --target tests
ctest --output-on-failure
```

## Pipeline Usage Example

```cpp
#include <pipeline/pipeline.h>
#include "filter_mock.h"

int main() {
    using namespace infodif;
    Pipeline pipeline;
    // Add filters (either function callbacks or Filter-derived classes)
    pipeline.add_filter(std::make_unique<MockFilter>());
    pipeline.add_tmp_filter(
        std::make_unique<std::function<void(std::shared_ptr<SharedData>)>>(
            [](auto data) { /* process data */ }
        )
    );

    pipeline.start();
    // Send data into the pipeline, handle output...
    // pipeline.push_data(data);
    // pipeline.get_output_queue();
    pipeline.stop();
    return 0;
}
```

## Contributing

Contributions welcome! Please submit issues and pull requests to the repository.

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.
