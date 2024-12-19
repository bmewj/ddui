//
//  get_asset_filename.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 07/12/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "get_asset_filename.hpp"
#include "whereami/whereami.h"

std::string get_asset_filename(const std::string& name) {

    std::string executable_dir;
    {
        int length = wai_getExecutablePath(NULL, 0, NULL);
        auto path = new char[length + 1];
        int dirname_length;
        wai_getExecutablePath(path, length, &dirname_length);
        path[length] = '\0';
        executable_dir = std::string(path).substr(0, dirname_length);
    }

    std::string asset_dir;
    #ifdef __APPLE__
        asset_dir = executable_dir.substr(0, executable_dir.length() - 5) + "Resources/assets/";
    #else
        asset_dir = executable_dir + "/assets/";
    #endif

    std::string asset_filename = asset_dir + name;
    return asset_filename;

}
