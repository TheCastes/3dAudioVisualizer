#pragma once

#include <string>
#include <vector>

#include "Colormap.h"
#include "RenderMode.h"
#include "ShaderParameters.h"

struct ShaderControls {
    const std::vector<std::string>& shaderNames;
    const std::vector<RenderMode>& shaderModes;
    int shaderIndex;
    const std::vector<Colormap>& colormaps;
    int colormapIndex;
    ShaderParameters& parameters;
};