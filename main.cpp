#include "json.hpp"
using json = nlohmann::json;
json config;

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <iostream>
#include <ft2build.h>
#include <map>
#include <iostream>
#include <fstream>
// #include "shaders.hpp"
// #include "FrameRateCalculator.hpp"
// #include "Model.hpp"
#include "View.hpp"
#include "InputController.hpp"

#include <fmt/format.h>



void read_config_json(json& config) {
    std::ifstream input_file("config.json");
    input_file >> config;
}

int main() {
    std::vector<std::string> event_log;
    FrameRateCalculator framerate_calculator;

    fmt::print("{}\n", __LINE__);

    read_config_json(config);

    Model model;
    View view(model);
    view.setupWatchers();

    fmt::print("{}\n", __LINE__);


    if (view.initGl()) {
        fmt::print("{}\n", __LINE__);

        return -1;
    }

    fmt::print("{}\n", __LINE__);


    InputController input_controller(view, view.window);
    input_controller.framebuffer_size_callback_subs.subscribe([&model](GLFWwindow* window, int w, int h) {
        model.resizeCallback(window, w, h);
    });
    input_controller.key_callback_subs.subscribe([&model](GLFWwindow* window, int key, int scancode, int action, int mods) {
        model.navigationCallback(window, key, scancode, action, mods);
    });
    input_controller.char_callback_subs.subscribe([&model](GLFWwindow* window, unsigned int codepoint) {
        model.inputTextCallback(window, codepoint);
    });
    input_controller.scroll_callback_subs.subscribe([&model](GLFWwindow* window, double xoffset, double yoffset, int mods) {
        model.scrollCallback(window, xoffset, yoffset, mods);
    });

    view.render();
    // model.setUpCells();
    input_controller.pollUntilExit();
    return 0;
}

