
BIN_PATH=bin
SRC_PATH=src/c

SRC=$(wildcard $(SRC_PATH)/data_structures/*.c)
SRC+=$(wildcard $(SRC_PATH)/locks/*.c)
SRC+=$(wildcard $(SRC_PATH)/read_indicators/*.c)

OBJ=$(patsubst %.c,%.o, $(SRC))

CFLAGS=-O2 -I$(SRC_PATH) -Wall -Wextra -pedantic
LDFLAGS=-L$(BIN_PATH) -lqdlock -pthread

$(BIN_PATH)/libqdlock.a: $(OBJ)
	@mkdir -p $(@D)
	ar rcu $@ $+
	ranlib $@

$(BIN_PATH)/test_lock: $(SRC_PATH)/tests/test_lock.c bin/libqdlock.a
	$(CC) -o $@ $^ -DLOCK_TYPE=OOLock $(CFLAGS) $(LDFLAGS)

test: $(BIN_PATH)/test_lock
	$(BIN_PATH)/test_lock TATAS_LOCK
	$(BIN_PATH)/test_lock QD_LOCK
	$(BIN_PATH)/test_lock MRQD_LOCK
	$(BIN_PATH)/test_lock CCSYNCH_LOCK
	$(BIN_PATH)/test_lock MCS_LOCK
	$(BIN_PATH)/test_lock DRMCS_LOCK

clean:
	rm -r -f $(BIN_PATH)
	find . -name "*.o" -exec rm -f {} \;

.PHONY: test clean

