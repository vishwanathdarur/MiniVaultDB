CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Iinclude

# Find all cpp inside src recursively
SRC := $(shell find src -name '*.cpp')
OBJ := $(patsubst %.cpp, %.o, $(SRC))

all: $(OBJ)

# Compile .cpp → .o, creating directories if needed
%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Build and run a specific test
test:
	$(CXX) $(CXXFLAGS) $(OBJ) $(TEST) -o test.out
	./test.out

clean:
	rm -f $(OBJ) test.out