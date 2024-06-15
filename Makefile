CC = gcc
CXX = g++
CFLAGS = -I./glad/include -I./imgui -I./imgui/backends `pkg-config --cflags glfw3` `pkg-config --cflags freetype2`
CXXFLAGS = $(CFLAGS) -DIMGUI_IMPL_OPENGL_LOADER_GLAD
LDFLAGS = `pkg-config --static --libs glfw3` `pkg-config --static --libs freetype2` -ldl -lGL -lGLU -lX11 -lpthread -lXrandr -lXi
BUILD_DIR = build

# Add all source files here
SRCS = main2.cpp \
       glad/src/glad.c \
       imgui/imgui.cpp \
       imgui/imgui_demo.cpp \
       imgui/imgui_draw.cpp \
       imgui/imgui_tables.cpp \
       imgui/imgui_widgets.cpp \
       imgui/backends/imgui_impl_glfw.cpp \
       imgui/backends/imgui_impl_opengl3.cpp

# Transform source files in SRCS to .o files in BUILD_DIR
OBJS = $(SRCS:%.cpp=$(BUILD_DIR)/%.o)
OBJS := $(OBJS:%.c=$(BUILD_DIR)/%.o)

TARGET = $(BUILD_DIR)/hello_world

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

build/glad/glad.o: glad/glad.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

