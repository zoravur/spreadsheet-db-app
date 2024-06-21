//
// Created by zoravur on 6/20/24.
//

#ifndef FRAMERATECALCULATOR_HPP
#define FRAMERATECALCULATOR_HPP

#include <iostream>
#include <chrono>

class FrameRateCalculator {
public:
    FrameRateCalculator()
        : frame_count(0), fps(0), last_time(std::chrono::high_resolution_clock::now()) {}

    void frame() {
        frame_count++;
        auto now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = now - last_time;
        if (elapsed.count() >= 1.0) {
            fps = frame_count / elapsed.count();
            frame_count = 0;
            last_time = now;
            // std::cout << "FPS: " << fps << std::endl;
        }
    }

    double getFPS() const {
        return fps;
    }

private:
    int frame_count;
    double fps;
    std::chrono::high_resolution_clock::time_point last_time;
};

#endif //FRAMERATECALCULATOR_HPP
