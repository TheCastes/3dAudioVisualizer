#pragma once

struct ShaderParameters {
    int   sphereGridSize = 64;
    int   freqSampleSize = 4;
    int   temporalWindow = 20;
    float temporalSigma  = 30.0f;
    float heightScale    = 2.0f;
    float baseRadius     = 0.04f;
    float radiusScale    = 0.13f;
};
