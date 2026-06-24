#pragma once

struct ShaderParameters {
    int   temporalWindow = 20;
    float temporalSigma  = 50.0f;
    int   freqSampleSize = 4;
    float heightScale    = 2.0f;
    float baseRadius     = 0.04f;
    float radiusScale    = 0.12f;
    int   sphereGridSize = 128;
};
