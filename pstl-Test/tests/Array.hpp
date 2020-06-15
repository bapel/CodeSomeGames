#pragma once

#include "pstl\Array.hpp"
#include <catch.hpp>

TEST_CASE("Static Array", "[array]")
{
    pstl::Array<int, 8> a;

    REQUIRE(a.Count() == 8);
}