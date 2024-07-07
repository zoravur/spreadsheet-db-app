//
// Created by zoravur on 6/21/24.
//

#ifndef INPUTCONTROLLER_HPP
#define INPUTCONTROLLER_HPP

#include <GLFW/glfw3.h>
// #include "View.hpp"
#include <fmt/core.h>

#include "Subject.hpp"

class View;

class InputController {
public:
    explicit InputController(View& view, GLFWwindow* window): view(view), window(window) {
        glfwSetWindowUserPointer(window, this);
        glfwSetKeyCallback(window, key_callback_dispatch);

        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback_dispatch);
        glfwSetCharCallback(window, char_callback_dispatch);
        glfwSetScrollCallback(window, scroll_callback_dispatch);
    }

    void pollEvents() {
        glfwPollEvents();
    };

    void waitEvents() {
        glfwWaitEvents();
    }

    void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        key_callback_subs.notify(window, key, scancode, action, mods);
    }

    void framebuffer_size_callback(GLFWwindow* window, int w, int h) {
        framebuffer_size_callback_subs.notify(window, w, h);
    }

    void char_callback(GLFWwindow* window, unsigned int codepoint) {
        char_callback_subs.notify(window, codepoint);
    }

    void scroll_callback(GLFWwindow* window, double xoffset, double yoffset, int mods) {
        scroll_callback_subs.notify(window, xoffset, yoffset, mods);
    }

    void pollUntilExit() {
        // view.render();

        while (!glfwWindowShouldClose(window)) {
            waitEvents();

            // if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            //     double xpos, ypos;
            //     glfwGetCursorPos(window, &xpos, &ypos);
            //     event_log.push_back("Mouse Left Button Pressed at (" + std::to_string(xpos) + ", " + std::to_string(ypos) + ")");
            // }
        }
    }
private:
    View& view;
    GLFWwindow* window;

    // Static dispatch function

    static void key_callback_dispatch(GLFWwindow* window, int key, int scancode, int action, int mods) {
        auto* self = static_cast<InputController*>(glfwGetWindowUserPointer(window));
        if (self) {
            self->key_callback(window, key, scancode, action, mods);
        }
    }

    static void framebuffer_size_callback_dispatch(GLFWwindow* window, int w, int h) {
        auto* self = static_cast<InputController*>(glfwGetWindowUserPointer(window));
        if (self) {
            self->framebuffer_size_callback(window, w, h);
        }
    }

    static void char_callback_dispatch(GLFWwindow* window, unsigned int codepoint) {
        auto* self = static_cast<InputController*>(glfwGetWindowUserPointer(window));
        if (self) {
            self->char_callback(window, codepoint);
        }
    }

    static void scroll_callback_dispatch(GLFWwindow* window, double xoffset, double yoffset, int mods) {
        auto* self = static_cast<InputController*>(glfwGetWindowUserPointer(window));
        if (self) {
            self->scroll_callback(window, xoffset, yoffset, mods);
        }
    }

public:
    Subject<GLFWwindow*, int, int, int, int> key_callback_subs;
    Subject<GLFWwindow*, int, int> framebuffer_size_callback_subs;
    Subject<GLFWwindow*, unsigned int> char_callback_subs;
    Subject<GLFWwindow*, double, double, int> scroll_callback_subs;
};



#endif //INPUTCONTROLLER_HPP
