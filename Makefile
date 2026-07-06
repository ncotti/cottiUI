SRC_DIRS ?= src
INC_DIRS ?= inc

CFLAGS ?= -Wall -g -Wpedantic
LDFLAGS ?= -g -lncurses -lpanel -lmenu -lform -lcdk -lm

EXE ?= exe

TEST_SRC_DIRS ?= test
TEST_FRAMEWORK_SRC_DIRS ?= test/framework/unity/src
TEST_INC_DIRS ?= test/framework/unity/src test/framework/fff

include cottimake/cottimake.mk
