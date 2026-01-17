#pragma once

#include "lemlib/asset.hpp"

// Declare all assets here using the ASSET macro
// To add a new asset:
// 1. Add the asset file to the static/ directory
// 2. Add ASSET(asset_name); here
// 3. Include this file in any .cpp file that needs to use the assets
// 
// Note: The ASSET macro creates a static variable, so each .cpp file that includes
// this header gets its own copy of the asset struct (but they all point to the same
// underlying binary data in the firmware image)

ASSET(example_txt);
