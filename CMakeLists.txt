# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.16)

project(bq25180 LANGUAGES C CXX)

include(${CMAKE_CURRENT_SOURCE_DIR}/sources.cmake)

add_library(${PROJECT_NAME} STATIC ${BQ25180_SRCS})
target_compile_features(${PROJECT_NAME} PRIVATE c_std_99)
target_include_directories(${PROJECT_NAME} PUBLIC ${BQ25180_INCS})
