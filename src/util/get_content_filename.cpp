//
//  get_content_filename.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 15/12/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "get_content_filename.hpp"
#include "get_asset_filename.hpp"

std::string get_content_filename(const std::string& name) {
    return get_asset_filename(name);
}
