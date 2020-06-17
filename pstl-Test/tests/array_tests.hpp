#pragma once

#include "pstl\array.hpp"
#include <catch.hpp>

TEST_CASE("Static Array", "[array]")
{
    SECTION("Fill and read values (random-access)")
    {
        pstl::array<int, 8> arr;
        REQUIRE(arr.size() == 8);

        for (auto i = 0U; i < arr.size(); i++)
            arr[i] = i;

        for (auto i = 0U; i < arr.size(); i++)
            REQUIRE(arr[i] == i);
    }

    SECTION("Fill and read values (iterators)")
    {
        pstl::array<int, 8> arr;
        REQUIRE(arr.size() == 8);

        for (auto iter = arr.begin(); iter < arr.end(); iter++)
            *iter = 3;

        for (auto iter = arr.cbegin(); iter < arr.cend(); iter++)
            REQUIRE(*iter == 3);

        for (auto& i : arr)
            i = 5;

        for (const auto& i : arr)
            REQUIRE(i == 5);
    }

    SECTION("Init const and read values (random-access)")
    {
        const pstl::array<int, 8> arr = { 0, 1, 2, 3, 4, 5, 6, 7 };
        REQUIRE(arr.size() == 8);

        for (auto i = 0U; i < arr.size(); i++)
            REQUIRE(arr[i] == i);
    }

    SECTION("Init const and read values (iterators)")
    {
        const pstl::array<int, 8> arr = { 3, 3, 3, 3, 3, 3, 3, 3 };
        REQUIRE(arr.size() == 8);

        for (auto iter = arr.cbegin(); iter < arr.cend(); iter++)
            REQUIRE(*iter == 3);

        for (const auto& i : arr)
            REQUIRE(i == 3);
    }
}