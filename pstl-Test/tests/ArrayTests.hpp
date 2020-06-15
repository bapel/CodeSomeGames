#pragma once

#include "pstl\Array.hpp"
#include <catch.hpp>

TEST_CASE("Static Array", "[array]")
{
    SECTION("Fill and read values")
    {
        pstl::Array<int, 8> arr;

        REQUIRE(arr.Count() == 8);

        auto i = 0U;
        auto enumerator = arr.GetEnumerator();
        do { enumerator.Current() = i++; }
        while (enumerator.MoveNext());

        i = 0U;
        enumerator.Reset();
        do { REQUIRE(enumerator.Current() == i++); }
        while (enumerator.MoveNext());
    }

    SECTION("Init const and read values")
    {
        const pstl::Array<int, 8> arr = { 0, 1, 2, 3, 4, 5, 6, 7 };

        REQUIRE(arr.Count() == 8);

        auto i = 0U;
        auto enumerator = arr.GetEnumerator();
        do { REQUIRE(enumerator.Current() == i++); }
        while (enumerator.MoveNext());
    }
}