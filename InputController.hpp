//
// Created by zoravur on 6/21/24.
//

#ifndef INPUTCONTROLLER_HPP
#define INPUTCONTROLLER_HPP

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
#include "json.hpp"
#include "shaders.hpp"
#include "FrameRateCalculator.hpp"
#include <fmt/format.h>

class InputController {
public:
    explicit InputController(GLFWwindow* window): window(window) {
        glfwSetWindowUserPointer(window, this);
        glfwSetKeyCallback(window, key_callback_dispatch);
    }

    void pollEvents() {
        glfwPollEvents();
    };

    void waitEvents() {
        glfwWaitEvents();

    }

    void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        for (const auto& callback : subscribers) {
            callback(window, key, scancode, action, mods);
        }

        // switch (action) {
        //     case GLFW_PRESS:
        //         fmt::print("Key {} PRESS\n", (char)key); break;
        //     case GLFW_REPEAT:
        //         fmt::print("Key {} REPEAT\n", (char)key); break;
        //     case (GLFW_RELEASE):
        //         fmt::print("Key {} up\n", (char)key);
        //         break;
        //     default:
        //         break;
        // }
    }

    size_t subscribe(const std::function<void(GLFWwindow*, int, int, int, int)>& callback) {
        subscribers.push_back(callback);
        return subscribers.size()-1;
    }

    void unsubscribe(size_t index) {
        if (index < subscribers.size()) {
            subscribers.erase(subscribers.begin() + index);
        }
    }
private:
    GLFWwindow* window;
    std::vector<std::function<void(GLFWwindow*, int, int, int, int)>> subscribers;

    // Static dispatch function
    static void key_callback_dispatch(GLFWwindow* window, int key, int scancode, int action, int mods) {
        auto* self = static_cast<InputController*>(glfwGetWindowUserPointer(window));
        if (self) {
            self->key_callback(window, key, scancode, action, mods);
        }
    }
};



#endif //INPUTCONTROLLER_HPP
