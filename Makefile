PROG:=bpulse
PLATFORM:=$(shell uname -s)
PLATFORM_DIR:=proc/$(PLATFORM)
ifeq ($(USE_GLX),1)
	GFX_DIR:=GLX
	LIBS:=-lGL
else
ifeq ($(USE_XRENDER),1)
	GFX_DIR:=XRender
	LIBS:=-lXrender
else
$(error Specify USE_GLX=1 or USE_XRENDER=1 to select a graphics backend)
endif
endif
SRC_DIR:=src
OBJ_DIR:=obj
CPP_FILES:=$(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(PLATFORM_DIR)/*.cpp) $(wildcard $(GFX_DIR)/*.cpp)
OBJ_FILES:=$(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(CPP_FILES))
OBJ_FILES:=$(patsubst $(PLATFORM_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(OBJ_FILES))
OBJ_FILES:=$(patsubst $(GFX_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(OBJ_FILES))
CPPFLAGS:=-std=c++17 -O3 -I./include -I$(PLATFORM_DIR) -I$(GFX_DIR)
LIBS+=-lpng -lX11 -lXext -lfreetype
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

$(OBJ_DIR)/%.o: $(GFX_DIR)/%.cpp
	$(CXX) -c $< -o $@ $(CPPFLAGS)

clean:
	$(RM) $(OBJ_FILES) $(PROG)
