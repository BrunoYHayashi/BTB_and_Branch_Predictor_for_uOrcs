CPP = g++
LD = g++
CPPFLAGS = $(FLAGS)
BIN_NAME = uorcs
RM = rm -f

FLAGS =   -O3 -ggdb -Wall -Wextra -Werror -std=c++11
LDFLAGS = -ggdb

################################################################################

LIBRARY = -lz

SRC_PACKAGE = 		opcode_package.cpp 

SRC_TRACE_READER = 	trace_reader.cpp

SRC_PROCESSOR =	 	processor.cpp btb.cpp branch_predictor.cpp

SRC_CORE =  simulator.cpp orcs_engine.cpp\
			$(SRC_TRACE_READER)	\
			$(SRC_PACKAGE) \
			$(SRC_PROCESSOR)

TEST_BTB = test_btb.cpp

TEST_BRANCH_PREDICTOR = test_branch_predictor.cpp

################################################################################
OBJS_CORE = ${SRC_CORE:.cpp=.o}
OBJS = $(OBJS_CORE)
################################################################################
# Implicit rules.
%.o : %.cpp %.hpp
	$(CPP) -c $(CPPFLAGS) $< -o $@

################################################################################

all: uorcs test_btb test_branch_predictor

uorcs: $(OBJS_CORE)
	$(LD) $(LDFLAGS) -o $(BIN_NAME) $(OBJS) $(LIBRARY)

test_btb: test_btb.o btb.o
	$(LD) $(LDFLAGS) -o test_btb test_btb.o btb.o

test_branch_predictor: test_branch_predictor.o branch_predictor.o
	$(LD) $(LDFLAGS) -o test_branch_predictor test_branch_predictor.o branch_predictor.o

clean:
	-$(RM) $(OBJS)
	-$(RM) $(BIN_NAME)
	-$(RM) test_btb test_branch_predictor
	@echo OrCS cleaned!
	@echo
