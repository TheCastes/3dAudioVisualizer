#pragma once

#include <functional>
#include <string>
#include <vector>

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

    void setLoadCallback(std::function<void(const std::string&)> callback);

    void scanTracks(const std::string& directory);

 
    void beginFrame();
    void draw();
    void render();

private:
    struct Track {
        std::string path;
        std::string name;
    };

    bool initialized = false;

    std::function<void(const std::string&)> loadCallback;
    std::vector<Track> tracks;
    int selectedTrack = -1;
};