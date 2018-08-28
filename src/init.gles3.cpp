//
//  init.gles3.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 28/08/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "init.hpp"

#include <GL3/gl3w.h>

#define NANOVG_GLES3_IMPLEMENTATION
#include <nanovg_gl.h>

NVGcontext* nvgCreate() {
    return nvgCreateGLES3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
}

void nvgDelete(NVGcontext* vg) {
    nvgDeleteGLES3(vg);
}
