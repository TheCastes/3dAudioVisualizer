#include "../include/audio/AudioEngine.h"
#include "../include/renderer/Renderer.h"

#include <juce_core/juce_core.h>
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    std::thread juceThread(AudioEngine::runInBackground);
    for (int i = 0; i < 200 && !AudioEngine::getAudioReady().load(); ++i)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    if (!Renderer::init()) {
        std::cerr << "Renderer init fallito\n";
        return -1;
    }

    while (!glfwWindowShouldClose(Renderer::getWindow())) {
        glfwPollEvents();
        Renderer::render();
    }

    std::cout << "Chiusura in corso...\n";
    Renderer::shutdown();
    juce::MessageManager::getInstance()->stopDispatchLoop();
    juceThread.join();

    std::cout << "Uscita pulita. Tutto funzionante.\n";
    return 0;
}
