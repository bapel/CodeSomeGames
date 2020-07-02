#pragma once

#include <vstl\UnorderedSet.hpp>

TEST_CASE("Unordered set construction", "[set]")
{
    vstl::UnorderedSet<uint64_t> set(10);

    REQUIRE( set.insert(2).second);
    REQUIRE(!set.insert(2).second);
}