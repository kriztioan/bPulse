PROG:=bpulse
PLATFORM:=$(shell uname -s)
PLATFORM_DIR:=proc/$(PLATFORM)
SRC_DIR:=src
OBJ_DIR:=obj
CPP_FILES:=$(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(PLATFORM_DIR)/*.cpp)
OBJ_FILES:=$(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(CPP_FILES)) 
OBJ_FILES:=$(patsubst $(PLATFORM_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(OBJ_FILES)) 
CPPFLAGS:=-O3 -I ./include -I $(PLATFORM_DIR) -std=c++11
LIBS:=-lpng -lX11 -lXext -lXrender -lfreetype
FRAMEWORKS:=
ifeq ($(PLATFORM),Darwin)
	CPPFLAGS+=-I /usr/local/include/freetype2
	FRAMEWORKS+=-framework IOKit -framework Foundation
else
	CPPFLAGS+=-I /usr/include/freetype2
endif

$(PROG): $(OBJ_FILES)
	$(CXX) -o $@ $^ $(FRAMEWORKS) $(LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) -c $< -o $@ $(CPPFLAGS)

$(OBJ_DIR)/%.o: $(PLATFORM_DIR)/%.cpp
	$(CXX) -c $< -o $@ $(CPPFLAGS)

clean:
	$(RM) $(OBJ_FILES) $(PROG)
