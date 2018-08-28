//
//  glfw.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 26/08/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#ifndef ddui_glfw_hpp
#define ddui_glfw_hpp

#include <GL3/gl3w.h>

#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

#include <functional>

namespace ddui {

bool init_glfw();
bool init_window(GLFWwindow* window, std::function<void()> update_proc);
void update_window(GLFWwindow* window);

}

#endif
