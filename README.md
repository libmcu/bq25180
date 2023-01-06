## Integration Guide

### CMake

#### Library

```cmake
add_subdirectory(bq25180)
target_link_libraries(${YOUR_PROJECT} bq25180)
```

#### FetchContent

```cmake
include(FetchContent)
FetchContent_Declare(bq25180
                      GIT_REPOSITORY https://github.com/libmcu/bq25180.git
                      GIT_TAG main
)
FetchContent_MakeAvailable(bq25180)
```

#### Source files

```cmake
include(sources.cmake)
add_executable(${YOUR_PROJECT} ${BQ25180_SRCS})
target_include_directories(${YOUR_PROJECT} ${BQ25180_INCS})
```

### Make

```Make
include sources.mk
SRCS += BQ25180_SRCS
INCS += BQ25180_INCS
```
