//
// Created by zoravur on 6/21/24.
//

#ifndef MODEL_HPP
#define MODEL_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


class Model {
public:
    // these could be replaced by a sparse vector / binary tree. Depends on performance.
    // std::vector col_widths;
    // std::vector row_heights;
    glm::vec2 top_left;
    glm::vec2 view_size;

};



#endif //MODEL_HPP
