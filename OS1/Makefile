SUBMITTERS := <208036319>_<315184259>
COMPILER := g++
COMPILER_FLAGS := --std=c++11
SRCS := Commands.cpp signals.cpp smash.cpp
HDRS := Commands.h signals.h

SMASH_BIN := smash

$(SMASH_BIN):
	$(COMPILER) $(COMPILER_FLAGS) $^ $(SRCS) -o $@

clean:
	rm -rf $(SMASH_BIN)
	