#pragma once

#include "pstl\allocator.hpp"
#include <catch.hpp>

void align_test(pstl::allocator* allocator, size_t offset = 0)
{
    auto ptr_0 = allocator->malloc(128,  8, offset);
    auto ptr_1 = allocator->malloc(128, 16, offset);
    auto ptr_2 = allocator->malloc(128, 32, offset);
    auto ptr_3 = allocator->malloc(128, 64, offset);

    REQUIRE(0 == ((uintptr_t)ptr_0 + offset) %  8);
    REQUIRE(0 == ((uintptr_t)ptr_1 + offset) % 16);
    REQUIRE(0 == ((uintptr_t)ptr_2 + offset) % 32);
    REQUIRE(0 == ((uintptr_t)ptr_3 + offset) % 64);

    allocator->free(ptr_0);
    allocator->free(ptr_1);
    allocator->free(ptr_2);
    allocator->free(ptr_3);
}

void struct_align_test(pstl::allocator* allocator, size_t offset = 0)
{
    __declspec(align(8))  struct Aligned__8 { double v[16]; };
    __declspec(align(16)) struct Aligned_16 { double v[16]; };
    __declspec(align(32)) struct Aligned_32 { double v[16]; };
    __declspec(align(64)) struct Aligned_64 { double v[16]; };

    auto ptr_0 = allocator->malloc_n<Aligned__8>(16, offset);
    auto ptr_1 = allocator->malloc_n<Aligned_16>(16, offset);
    auto ptr_2 = allocator->malloc_n<Aligned_32>(16, offset);
    auto ptr_3 = allocator->malloc_n<Aligned_64>(16, offset);

    REQUIRE(0 == ((uintptr_t)ptr_0 + offset) %  8);
    REQUIRE(0 == ((uintptr_t)ptr_1 + offset) % 16);
    REQUIRE(0 == ((uintptr_t)ptr_2 + offset) % 32);
    REQUIRE(0 == ((uintptr_t)ptr_3 + offset) % 64);

    allocator->free(ptr_0);
    allocator->free(ptr_1);
    allocator->free(ptr_2);
    allocator->free(ptr_3);
}

TEST_CASE("Allocator implementations", "[allocators]", )
{
    using namespace pstl;

    allocator* allocators[] = 
    {
        fallback_allocator()
        // @Todo: Add other allocators here.
    };

    const char* names[] = 
    {
        "fallback_allocator"
    };

    for (auto i = 0; i < sizeof(allocators) / sizeof(allocator*); i++)
    {
        SECTION(names[i])
        {
            SECTION("Specific alignment") 
            { align_test(allocators[i]); }

            SECTION("Specific alignment with offset")
            { align_test(allocators[i], 15); }

            SECTION("Struct alignment")
            { struct_align_test(allocators[i]); }

            SECTION("Struct alignment with offset")
            { struct_align_test(allocators[i], 10); }
        }
    }
}