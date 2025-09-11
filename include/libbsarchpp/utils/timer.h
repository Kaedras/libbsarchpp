#pragma once

#include <chrono>
#include <iostream>
#include <string>
#include <utility>

class timer
{
public:
    explicit timer(std::string name)
        : start(std::chrono::high_resolution_clock::now())
        , name(std::move(name))
    {}
    ~timer()
    {
        using namespace std::chrono_literals;
        std::cout << name << " took " << (std::chrono::high_resolution_clock::now() - start) / 1ms << " ms" << '\n';
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start;
    std::string name;
};