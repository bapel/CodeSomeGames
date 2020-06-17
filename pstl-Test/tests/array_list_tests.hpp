#pragma once

#include <pstl\array_list.hpp>
#include <catch.hpp>

TEST_CASE("Array list construction", "[array-list]")
{
    SECTION("Array list must be initially empty and unallocated")
    {
        pstl::array_list<int> ints;

        REQUIRE(0 == ints.size());
        REQUIRE(0 == ints.capacity());
        REQUIRE(nullptr == ints.data());
    }

    SECTION("Array list init with count should adequately allocate")
    {
        const auto n = 200;
        pstl::array_list<int> ints(n);

        REQUIRE(ints.size() == n);
        REQUIRE(ints.capacity() >= n);
        REQUIRE(ints.data() != nullptr);
    }

    SECTION("Array list construction from init list")
    {
        pstl::array_list<int> a = { 0, 1, 2, 3, 4, 5, 6 };

        for (auto i = 0U; i < a.size(); i++)
            REQUIRE(a[i] == i);
    }

    SECTION("Array list assign from init list")
    {
        pstl::array_list<int> a;
        a = { 0, 1, 2, 3, 4, 5, 6 };

        for (auto i = 0U; i < a.size(); i++)
            REQUIRE(a[i] == i);
    }
}

TEST_CASE("Array list reserve", "[array-list]")
{
    SECTION("Reserve after default construct")
    {
        pstl::array_list<int> ints;

        ints.reserve(10);

        REQUIRE(ints.data() != nullptr);
        REQUIRE(ints.size() == 0);
        REQUIRE(ints.capacity() >= 10);
    }

    SECTION("Reserve smaller")
    {
        pstl::array_list<int> ints(10);

        ints.reserve(0);

        REQUIRE(ints.data() != nullptr);
        REQUIRE(ints.size() == 0);
        REQUIRE(ints.capacity() >= 0);
    }

    SECTION("Reserve larger")
    {
        pstl::array_list<int> ints(10);

        ints.reserve(20);

        REQUIRE(ints.data() != nullptr);
        REQUIRE(ints.size() == 10);
        REQUIRE(ints.capacity() >= 20);
    }
}

TEST_CASE("Array list add", "[array-list]")
{
    SECTION("Add, no growth")
    {
        pstl::array_list<int> ints;
        ints.reserve(40);

        for (auto i = 0; i < 20; i++)
            ints.push_back(0);

        REQUIRE(ints.size() == 20);
        REQUIRE(ints.capacity() >= 40);
    }

    SECTION("Add with growth")
    {
        pstl::array_list<int> ints;
        ints.reserve(10);

        for (auto i = 0; i < 20; i++)
            ints.push_back(0);

        REQUIRE(ints.size() == 20);
        REQUIRE(ints.capacity() >= 20);
    }

    SECTION("Add, no init capacity")
    {
        pstl::array_list<int> ints;

        for (auto i = 0; i < 10; i++)
            ints.push_back(0);

        REQUIRE(ints.size() == 10);
        REQUIRE(ints.capacity() >= 10);
    }

    SECTION("Retrieve added values")
    {
        pstl::array_list<int> ints;
        ints.reserve(10);

        for (auto i = 0U; i < ints.capacity(); i++)
            ints.push_back(i);

        for (auto i = 0U; i < ints.size(); i++)
            REQUIRE(ints[i] == i);
    }
}

TEST_CASE("Array list insert", "[array-list]")
{
    SECTION("Insert in the middle")
    {
        pstl::array_list<int> a = { 0, 1,     2, 3, 4, 5, 6 };
        pstl::array_list<int> b = { 0, 1, -2, 2, 3, 4, 5, 6 };

        a.insert(a.begin() + 2, -2);

        REQUIRE(b == a);
    }

    SECTION("Insert at start")
    {
        pstl::array_list<int> a = {     0, 1, 2, 3, 4, 5, 6 };
        pstl::array_list<int> b = { -1, 0, 1, 2, 3, 4, 5, 6 };

        a.insert(a.begin() + 0, -1);

        REQUIRE(b == a);
    }

    SECTION("Insert at end")
    {
        pstl::array_list<int> a = { 0, 1, 2, 3, 4, 5, 6 };
        pstl::array_list<int> b = { 0, 1, 2, 3, 4, 5, 6, -1 };

        a.insert(a.end(), -1);

        REQUIRE(b == a);
    }

    SECTION("Insert into empty")
    {
        pstl::array_list<int> a;

        a.insert(0, 13);

        REQUIRE(a[0] == 13);
        REQUIRE(a.size() == 1);
        REQUIRE(a.capacity() >= 1);
    }
}

TEST_CASE("Array list removal", "[array-list]")
{
    SECTION("Removal from the middle")
    {
        pstl::array_list<int> a = { 0, 1, 2, 3, 4, 5, 6 };
        pstl::array_list<int> b = { 0, 1,    3, 4, 5, 6 };

        auto at = a.begin() + 2;
        REQUIRE(a.erase(at) == at);
        REQUIRE(b == a);
    }

    SECTION("Removal from the start")
    {
        pstl::array_list<int> a = { 0, 1, 2, 3, 4, 5, 6 };
        pstl::array_list<int> b = {    1, 2, 3, 4, 5, 6 };

        auto at = a.begin();
        REQUIRE(a.erase(at) == at);
        REQUIRE(b == a);
    }

    SECTION("Removal from the end")
    {
        pstl::array_list<int> a = { 0, 1, 2, 3, 4, 5, 6 };
        pstl::array_list<int> b = { 0, 1, 2, 3, 4, 5 };

        REQUIRE(a.erase(a.end() - 1) == a.end());
        REQUIRE(b == a);
    }

    SECTION("Unordered removal from the middle")
    {
        pstl::array_list<int> a = { 0, 1, 2, 3, 4, 5, 6 };
        pstl::array_list<int> b = { 0, 1, 6, 3, 4, 5 };

        auto at = a.begin() + 2;
        REQUIRE(a.erase_unordered(at) == at);
        REQUIRE(b == a);
    }

    SECTION("Unordered removal from the start")
    {
        pstl::array_list<int> a = { 0, 1, 2, 3, 4, 5, 6 };
        pstl::array_list<int> b = { 6, 1, 2, 3, 4, 5 };

        auto at = a.begin();
        REQUIRE(a.erase_unordered(at) == at);
        REQUIRE(b == a);
    }

    SECTION("Unordered removal from the end")
    {
        pstl::array_list<int> a = { 0, 1, 2, 3, 4, 5, 6 };
        pstl::array_list<int> b = { 0, 1, 2, 3, 4, 5 };

        REQUIRE(a.erase_unordered(a.end() - 1) == a.end());
        REQUIRE(b == a);
    }
}

TEST_CASE("Array list resize", "[array-list]")
{
    pstl::array_list<int> ints;

    ints.resize(20);

    REQUIRE(ints.size() == 20);
    REQUIRE(ints.capacity() >= 20);

    ints.resize(10);

    REQUIRE(ints.size() == 10);
    REQUIRE(ints.capacity() >= 10);

    ints.resize(30);

    REQUIRE(ints.size() == 30);
    REQUIRE(ints.capacity() >= 30);
}