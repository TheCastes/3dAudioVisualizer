#pragma once

#include <functional>
#include <string>

struct GLFWwindow;


class Ui {
public:
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

    void beginFrame();
    void draw(const std::string& trackName, bool isPlaying);
    void render();

private:
    bool initialized = false;

    std::function<void()> browseCallback;
    std::function<void()> playPauseCallback;
    std::function<void()> stopCallback;
};