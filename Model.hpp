//
// Created by zoravur on 6/21/24.
//

#ifndef MODEL_HPP
#define MODEL_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <functional>
#include "Store.hpp"
#include "InputController.hpp"
#include <fmt/format.h>

#include "SpreadsheetModel.hpp"


class Model {
public:
    Model(): selected_cell_({1,1}), view_size_({config["width"].get<float>(), config["height"].get<float>()}), view_origin_({0,0}) {}
        // top_left_.subscribe([](glm::vec2 data, Store<glm::vec2> store) {
        //     fmt::print("top_left_ is {} {}", data.x, data.y);
        // });
        //
        // top_left_.set({0,1});
        // top_left_.set({2,2});


    // these could be replaced by a sparse vector / binary tree. Depends on performance.
    // std::vector col_widths;
    // std::vector row_heights;

    void resizeCallback(GLFWwindow* window, int w, int h) {
        view_size_.set({w, h});
    }

    void navigationCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        if (action == GLFW_RELEASE) {
            return;
        }

        switch(key) {
            case GLFW_KEY_UP:
            case GLFW_KEY_DOWN:
            case GLFW_KEY_LEFT:
            case GLFW_KEY_RIGHT: {
                auto vec = selected_cell_.get();
                auto movement = mapKeyToMovement(key);
                auto new_loc = vec + movement;
                // fmt::print("newLoc: {} {}", new_loc.y, new_loc.x);
                if (spreadsheet_model_.inBounds(new_loc.x, new_loc.y)) {
                    changeSelectedCell(new_loc);
                    // selected_cell_.set(vec);
                }
                break;
            }
            case GLFW_KEY_BACKSPACE: {
                auto coords = selected_cell_.get();
                if (!cur_codepoints.empty()) {
                    cur_codepoints.pop_back();
                    spreadsheet_model_.setCellValue( coords.x, coords.y,
                        convertCodepointsToString(cur_codepoints));
                }
                break;
            }
            default:
                break;
        }
    }

    void scrollCallback(GLFWwindow* window, double xoffset, double yoffset, int mods) {
        auto origin = view_origin_.get();

        // fmt::print("origin: {} {}\n", origin.x, origin.y);


        if (mods & GLFW_MOD_SHIFT) {
            std::swap(xoffset, yoffset);
        }

        // if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) {
        //     std::swap(xoffset, yoffset);
        // }
        origin += -config["scroll_sensitivity"].get<float>()*glm::vec2(xoffset, yoffset);
        if (origin.x < 0 || origin.y < 0) return;

        view_origin_.set(origin);
    }

    void inputTextCallback(GLFWwindow* window, unsigned int codepoint) {
        auto coords = selected_cell_.get();

        // auto str = spreadsheet_model_.getCellValue(coords.y, coords.x);

        cur_codepoints.push_back(codepoint);
        spreadsheet_model_.setCellValue(coords.x, coords.y, convertCodepointsToString(cur_codepoints));
    }

    /**
     * DEBUG METHOD
     */
    void setUpCells() {
        for (size_t i = 1; i <= 25; i++) {
            for (size_t j = i; j <= i+1; j++) {
                spreadsheet_model_.setCellValue(j, i, fmt::format("{}{}", (char)(j+'A'-1), i));
            }
        }
    }

    static std::string convertCodepointsToString(const std::vector<uint32_t>& codepoints) {
        std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
        std::u32string u32str(codepoints.begin(), codepoints.end());
        return converter.to_bytes(u32str);
    }

    static std::vector<uint32_t> convertStringToCodepoints(const std::string& str) {
        std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
        std::u32string u32str = converter.from_bytes(str);
        return {u32str.begin(), u32str.end()};
    }

private:
    // std::vector<std::function<void()>> subscribers;

    glm::vec2 mapKeyToMovement(int key) {
        switch (key) {
            case GLFW_KEY_UP:
                return {0, -1};
            case GLFW_KEY_DOWN:
                return {0, 1};
            case GLFW_KEY_LEFT:
                return {-1, 0};
            case GLFW_KEY_RIGHT:
                return {1, 0};
            default:
                return {0, 0};
        }
    }

    void changeSelectedCell(glm::vec2 new_cell) {
        selected_cell_.set(new_cell);
        cur_codepoints.clear();
        // view_origin_.set(new_cell - glm::vec2(1) * view_size_)
    }

public:
    typedef std::vector<std::vector<std::string>> SpreadsheetData;

    SpreadsheetModel spreadsheet_model_;
    Store<glm::vec2> selected_cell_;
    Store<glm::vec2> view_size_;
    std::vector<uint32_t> cur_codepoints;
    Store<glm::vec2> view_origin_; // The top left corner of the spreadsheet.

    // Maybe create view_model_ class, to handle the relevant calculations for determining how the view looks?
    // This may be necessary since the view cannot update the model.
};



#endif //MODEL_HPP
