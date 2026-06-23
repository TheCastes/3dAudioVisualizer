#pragma once

#include <functional>
#include <string>
#include <vector>

#include "../renderer/Colormap.h"

struct GLFWwindow;


class Ui {
public:
    struct PlaybackState {
        std::string trackName;
        bool isPlaying = false;
        double positionSeconds = 0.0;
        double lengthSeconds = 0.0;
    };

    Ui();
    ~Ui();

    Ui(const Ui& copy) = delete;
    Ui& operator=(const Ui&) = delete;
    Ui(Ui&& move) = delete;
    Ui& operator=(Ui&&) = delete;

    bool init(GLFWwindow* window);

    void setBrowseCallback(std::function<void()> callback);
    void setPlayPauseCallback(std::function<void()> callback);
    void setStopCallback(std::function<void()> callback);
    void setShaderCallback(std::function<void(int)> callback);
    void setColormapCallback(std::function<void(int)> callback);

    void beginFrame();
    void draw(const PlaybackState& state, int shaderIndex,
              const std::vector<Colormap>& colormaps, int colormapIndex);
    void render();

private:
    float drawPlayerPanel(const PlaybackState& state);
    void drawShaderPanel(float topY, int shaderIndex, const std::vector<Colormap>& colormaps, int colormapIndex);

    void fileButton(bool hasTrack);
    void trackInfo(const PlaybackState& state);
    void transportControls(bool hasTrack, bool isPlaying);

    void shaderSelector(int shaderIndex);
    void colormapDropdown(const std::vector<Colormap>& colormaps, int colormapIndex);

    bool initialized = false;

    std::function<void()> browseCallback;
    std::function<void()> playPauseCallback;
    std::function<void()> stopCallback;
    std::function<void(int)> shaderCallback;
    std::function<void(int)> colormapCallback;
};