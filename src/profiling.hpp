//
//  profiling.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 14/12/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#ifndef ddui_profiling_hpp
#define ddui_profiling_hpp

#include <ctime>
#include <chrono>

namespace profiling {

void frame_start();
void frame_end();
extern int num_set_immediates;
extern int num_repaints;

}

#endif
