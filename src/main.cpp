#define GLFW_INCLUDE_NONE 
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_core/juce_core.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <atomic>
#include <thread>
#include <cmath>

std::atomic<float> g_audioLevel{0.0f};
std::atomic<bool> g_audioReady{false};

struct AudioEngine : public juce::AudioIODeviceCallback {
    double sr = 44100.0;
    double phase = 0.0;

    void audioDeviceAboutToStart(juce::AudioIODevice* d) override { sr = d->getCurrentSampleRate(); }
    void audioDeviceStopped() override {}

    void audioDeviceIOCallbackWithContext(const float* const* inputChannelData, int numInputChannels, float* const* outputChannelData, int numOutputChannels, int numSamples, const juce::AudioIODeviceCallbackContext&) override {
        float sum = 0.0f;
        for (int i = 0; i < numSamples; ++i) {
            double s = std::sin(phase);
            phase += 2.0 * juce::MathConstants<double>::pi * 440.0 / sr;
            float v = static_cast<float>(s) * 0.1f;
            sum += v * v;

            if (numOutputChannels > 0 && outputChannelData[0]) outputChannelData[0][i] = v;
            if (numOutputChannels > 1 && outputChannelData[1]) outputChannelData[1][i] = v;
        }
        g_audioLevel.store(std::sqrt(sum / numSamples), std::memory_order_relaxed);
    }
};

void runJUCEBackground() {
    juce::ScopedJuceInitialiser_GUI juceInit;
    AudioEngine eng;
    juce::AudioDeviceManager dm;

    auto err = dm.initialise(0, 2, nullptr, true);
    if (err.isEmpty()) {
        dm.addAudioCallback(&eng);
        dm.restartLastAudioDevice();
        g_audioReady.store(true, std::memory_order_release);
        std::cout << "JUCE Audio inizializzato\n";
        juce::MessageManager::getInstance()->runDispatchLoop();
    } else {
        std::cerr << "JUCE Audio init skipped: " << err.toStdString() << "\n";
    }
}

int main() {
    std::thread juceThread(runJUCEBackground);
    for (int i = 0; i < 200 && !g_audioReady.load(); ++i) 
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    if (!glfwInit()) { std::cerr << "GLFW init fallito\n"; return -1; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    GLFWwindow* win = glfwCreateWindow(800, 600, "OpenGL 4.6 + JUCE 8 Audio", nullptr, nullptr);
    if (!win) { std::cerr << "Creazione finestra fallita\n"; return -1; }
    glfwMakeContextCurrent(win);

    if (!gladLoadGL()) { std::cerr << "GLAD inizializzazione fallita\n"; return -1; }
    std::cout << "GLAD OK | OpenGL: " << glGetString(GL_VERSION) << "\n";

    glm::vec3 v(3.0f, 4.0f, 0.0f);
    std::cout << "GLM OK | Length: " << glm::length(v) << "\n";

    const char* vs = "#version 460 core\nlayout(location=0) in vec3 p;\nvoid main() { gl_Position = vec4(p, 1.0); }\n";
    const char* fs = "#version 460 core\nout vec4 c; uniform float u_level;\nvoid main() { c = vec4(u_level, 0.3, 1.0 - u_level, 1.0); }\n";

    GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vShader, 1, &vs, nullptr); glCompileShader(vShader);
    GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fShader, 1, &fs, nullptr); glCompileShader(fShader);

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vShader); glAttachShader(prog, fShader); glLinkProgram(prog);
    glUseProgram(prog);
    glDeleteShader(vShader); glDeleteShader(fShader);

    float verts[] = { -0.5f, -0.5f, 0.0f,  0.5f, -0.5f, 0.0f,  0.0f, 0.5f, 0.0f };
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao); glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
    std::cout << "✅ Rendering loop avviato...\n";

    while (!glfwWindowShouldClose(win)) {
        glfwPollEvents();

        float level = g_audioLevel.load(std::memory_order_relaxed);
        glUniform1f(glGetUniformLocation(prog, "u_level"), level);

        glClear(GL_COLOR_BUFFER_BIT);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);

        glfwSwapBuffers(win);
    }

    glDeleteProgram(prog); glDeleteBuffers(1, &vbo); glDeleteVertexArrays(1, &vao);
    glfwDestroyWindow(win); glfwTerminate();

    std::cout << "Chiusura in corso...\n";
    juce::MessageManager::getInstance()->stopDispatchLoop();
    juceThread.join();

    std::cout << "Uscita pulita. Tutto funzionante.\n";
    return 0;
}