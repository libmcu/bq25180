# SPDX-License-Identifier: MIT

COMPONENT_NAME = bq25180

SRC_FILES = ../bq25180.c

TEST_SRC_FILES = \
	src/bq25180_test.cpp \
	src/test_all.cpp \

INCLUDE_DIRS = \
	../ \
	$(CPPUTEST_HOME)/include \

MOCKS_SRC_DIRS =
CPPUTEST_CPPFLAGS = -Dassert=fake_assert

include runner.mk
