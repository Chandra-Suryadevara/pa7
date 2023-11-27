CC = g++
SRC_DIR = src
HEADERS_DIR = headers
TEST_DIR = test
OBJ_DIR = objs
SRCS = $(wildcard $(SRC_DIR)/*.cc)
TEST_SRCS = $(wildcard $(TEST_DIR)/*.cc)
OBJS = $(patsubst $(SRC_DIR)/%.cc, $(OBJ_DIR)/%.o, $(SRCS))
TEST_OBJS = $(patsubst $(TEST_DIR)/%.cc, $(OBJ_DIR)/%.o, $(TEST_SRCS))

# Google Test libraries
GTEST_LIB = -lgtest -lgtest_main -lpthread

# Add the header directory to the include path
CCFLAGS = -I$(HEADERS_DIR)

# Default target (run tests)
all: tests

# Create the obj directory if it doesn't exist
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Compile source files to object files in the objs directory
$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cc | $(OBJ_DIR)
	$(CC) $(CCFLAGS) -c $< -o $@

# Compile test source files to object files in the objs directory
$(OBJ_DIR)/%.o : $(TEST_DIR)/%.cc | $(OBJ_DIR)
	$(CC) $(CCFLAGS) -c $< -o $@

%.d : %.cc
	$(CC) $(CCFLAGS) -MM -MP -MT $(OBJ_DIR)/$*.o $< > $@

# Build test objects and run the tests
tests: $(OBJS) $(TEST_OBJS)
	$(CC) -o $@ $(TEST_OBJS) $(OBJS) $(GTEST_LIB)
	./$@

.PHONY : clean
clean:
	rm -rf $(OBJ_DIR) *.d tests

# Include the generated dependency files
-include $(OBJS:.o=.d) $(TEST_OBJS:.o=.d)
