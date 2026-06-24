#pragma once

struct ShaderParameters {
    int   temporalWindow = 10;
    float temporalSigma  = 50.0f;
    int   freqSampleSize = 1;
    float heightScale    = 2.0f;
    float baseRadius     = 0.04f;
    float radiusScale    = 0.12f;
};