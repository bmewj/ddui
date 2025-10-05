//
//  init.d3d11.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 28/08/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "init.hpp"

#define NANOVG_D3D11_IMPLEMENTATION
#include <nanovg_d3d11.h>

NVGcontext* nvgCreate(void* device) {
    //return nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
	return nvgCreateD3D11((ID3D11Device*)device, 0);
}

void nvgDelete(NVGcontext* vg) {
    nvgDeleteD3D11(vg);
}
