#pragma once

#include <string>
#include <unordered_map>

namespace checksums {
static const std::unordered_map<std::string, std::string> tes3
    = {{"Bloodmoon.bsa", "207d66ad1bad37ec9dcb3865ea7c48924db3e698b9264cc5fd18a9ccd5b901f8"},
       {"Morrowind.bsa", "d0b028d51ae499235268aea28ee9e285f8ccc8eb89707f9accbfca7e6946d072"},
       {"Tribunal.bsa", "3901e7a146f7a64a4ae9534c80d5974f7ab15adf5c48c4561d9313c0f7831d69"}};
} // namespace checksums
