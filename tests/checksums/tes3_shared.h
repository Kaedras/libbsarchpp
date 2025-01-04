#pragma once

#include <string>
#include <unordered_map>

namespace checksums {
static const std::unordered_map<std::string, std::string> tes3_shared
    = {{"Bloodmoon.bsa", "dd5d0d39a215e6924c7d26feef48f942df7b9e168c4993723fb56190f4b797a4"},
       {"Morrowind.bsa", "291ddddc472432a79a28d0cec0e63a057a7e77aac79d4c7774808e36fcae9f5b"},
       {"Tribunal.bsa", "c8570343bbb84a8d345e58dc3e4094ba9a7e28c051f3c0360b13ed1d414cabd3"}};
} // namespace checksums
