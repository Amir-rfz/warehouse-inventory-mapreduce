CXX = g++
CXXFLAGS = -std=c++17 -Iincludes

SRC_DIR = src
OBJ_DIR = build
INCLUDE_DIR = includes

OUT_CAS = warehouseManager.out
OUT_CLUB = store.out
OUT_POS = part.out

VPATH = $(SRC_DIR)

all: $(OUT_CAS) $(OUT_CLUB) $(OUT_POS)

$(OUT_CAS): $(OBJ_DIR)/warehouseManager.o $(OBJ_DIR)/logger.o $(OBJ_DIR)/csv.o $(OBJ_DIR)/strutils.o
	$(CXX) $(CXXFLAGS) $^ -o $@

$(OUT_CLUB): $(OBJ_DIR)/store.o $(OBJ_DIR)/logger.o $(OBJ_DIR)/csv.o $(OBJ_DIR)/strutils.o
	$(CXX) $(CXXFLAGS) $^ -o $@

$(OUT_POS): $(OBJ_DIR)/part.o $(OBJ_DIR)/logger.o $(OBJ_DIR)/csv.o $(OBJ_DIR)/strutils.o
	$(CXX) $(CXXFLAGS) $^ -o $@


$(OBJ_DIR)/warehouseManager.o: $(SRC_DIR)/main.cpp $(INCLUDE_DIR)/logger.hpp $(INCLUDE_DIR)/colorprint.hpp \
							$(INCLUDE_DIR)/csv.hpp $(INCLUDE_DIR)/manual.hpp $(INCLUDE_DIR)/strutils.hpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/store.o: $(SRC_DIR)/store.cpp $(INCLUDE_DIR)/logger.hpp $(INCLUDE_DIR)/colorprint.hpp $(INCLUDE_DIR)/csv.hpp \
					$(INCLUDE_DIR)/manual.hpp $(INCLUDE_DIR)/strutils.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/part.o: $(SRC_DIR)/part.cpp $(INCLUDE_DIR)/logger.hpp $(INCLUDE_DIR)/colorprint.hpp \
						$(INCLUDE_DIR)/csv.hpp $(INCLUDE_DIR)/manual.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/logger.o: $(SRC_DIR)/logger.cpp $(INCLUDE_DIR)/logger.hpp $(INCLUDE_DIR)/colorprint.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/csv.o: $(SRC_DIR)/csv.cpp $(INCLUDE_DIR)/csv.hpp $(INCLUDE_DIR)/strutils.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/strutils.o: $(SRC_DIR)/utils.cpp $(INCLUDE_DIR)/strutils.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

.PHONY: all clean

clean:
	rm -f $(OBJ_DIR)/*.o ./*.out
