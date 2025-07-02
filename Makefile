PROGS:=bpulse
PLATFORM:=$(shell uname -s)
PLATFORM_DIR:=proc/$(PLATFORM)
ifeq ($(USE_GLFW),1)
	GFX_DIR:=gfx/GLFW
	LIBS:=-lglfw -lGL -lGLEW
else
ifeq ($(USE_GLX),1)
	GFX_DIR:=gfx/GLX
	LIBS:=-lGL
else
ifeq ($(USE_XRENDER),1)
	GFX_DIR:=gfx/XRender
	LIBS:=-lXrender
else
$(error Specify USE_GLFW=1, USE_GLX=1, or USE_XRENDER=1 to select a graphics backend)
endif
endif
endif
SRC_DIR:=src
OBJ_DIR:=obj
CPP_FILES:=$(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(PLATFORM_DIR)/*.cpp) $(wildcard $(GFX_DIR)/*.cpp)
OBJ_FILES:=$(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(CPP_FILES))
OBJ_FILES:=$(patsubst $(PLATFORM_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(OBJ_FILES))
OBJ_FILES:=$(patsubst $(GFX_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(OBJ_FILES))
DEP_FILES:=deps.d
CPPFLAGS:=-std=c++17 -O3 -MMD -MF $(DEP_FILES) -I./include -I$(PLATFORM_DIR) -I$(GFX_DIR)
LIBS+=-lpng -lX11 -lXext -lfreetype
FRAMEWORKS:=
ifeq ($(PLATFORM),Darwin)
	CPPFLAGS+=-I/usr/local/include/freetype2
	FRAMEWORKS+=-framework IOKit -framework Foundation -framework OpenGL
else
	CPPFLAGS+=-I/usr/include/freetype2
endif

all: $(PROGS)

-include $(DEP_FILES)

$(PROGS): $(OBJ_FILES)
	$(CXX) -o $@ $^ $(FRAMEWORKS) $(LIBS) $(CPPFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) -c $< -o $@ $(CPPFLAGS)

$(OBJ_DIR)/%.o: $(PLATFORM_DIR)/%.cpp
	$(CXX) -c $< -o $@ $(CPPFLAGS)

$(OBJ_DIR)/%.o: $(GFX_DIR)/%.cpp
	$(CXX) -c $< -o $@ $(CPPFLAGS)

clean:
	$(RM) $(DEP_FILES) $(OBJ_FILES) $(PROGS)
