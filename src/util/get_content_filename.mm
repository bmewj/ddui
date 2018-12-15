//
//  get_content_filename.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 15/12/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "get_content_filename.hpp"
#include <AppKit/AppKit.h>
#include <sys/stat.h>

static std::string app_support_dir;

std::string get_content_filename(std::string name) {

    if (!app_support_dir.empty()) {
        return app_support_dir + name;
    }
    
    NSString* executableName = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleExecutable"];

    // Search for the path
    NSArray* paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
    if ([paths count] == 0)
    {
        // *** creation and return of error object omitted for space
        printf("Can't discover Application Support directory.\n");
        exit(1);
    }

    // Normally only need the first path
    NSString* resolvedPath = [paths objectAtIndex:0];
    resolvedPath = [resolvedPath
        stringByAppendingPathComponent:executableName];
    
    // Create the path if it doesn't exist
    if (mkdir([resolvedPath UTF8String], 0777) != 0 && errno != EEXIST) {
        printf("Can't create Application Support directory.\n");
        exit(1);
    }

    app_support_dir = std::string([resolvedPath UTF8String]) + '/';
    return app_support_dir + name;
}
