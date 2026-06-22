#pragma once

#include <functional>
#include <string>

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

    void beginFrame();
    void draw(const PlaybackState& state, int shaderIndex);
    void render();

private:
    bool initialized = false;

    std::function<void()> browseCallback;
    std::function<void()> playPauseCallback;
    std::function<void()> stopCallback;
    std::function<void(int)> shaderCallback;
};