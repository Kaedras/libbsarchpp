#pragma once

#include <gtest/gtest.h>
#include <tuple>

class BsaTest : public testing::TestWithParam<std::tuple<const char*, const char*>> {};
