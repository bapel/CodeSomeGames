#pragma once

#include "Collections.hpp"

Namespace__
{
    class IAllocator
    {
    public:
        virtual ~IAllocator()
        { assert(m_NumAllocations == 0); }

        // @Todo: Debug variants?
        virtual void* Malloc(SizeType size, SizeType alignment, SizeType offset = 0) = 0;
        virtual void Free(void* block) = 0;

        // Alignment is inferred from the type.
        template <class T>
        T* Malloc_n(CountType count, SizeType offset = 0)
        { return static_cast<T*>(Malloc(count * sizeof(T), alignof(T), offset)); }

    protected:
        CountType m_NumAllocations = 0;
    };

    IAllocator* GetFallbackAllocator();
}

#ifdef CatchAvailable__

void SpecificAlignTest(
    NamespaceName__::IAllocator* allocator, 
    NamespaceName__::SizeType offset = 0);

void StructAlignTest(
    NamespaceName__::IAllocator* allocator, 
    NamespaceName__::SizeType offset = 0);

TEST_CASE("Allocator implementations", "[allocators]", )
{
    using namespace NamespaceName__;

    IAllocator* allocators[] = 
    {
        GetFallbackAllocator()
        // @Todo: Add other allocators here.
    };

    const char* names[] = 
    {
        "FallbackAllocator"
    };

    for (auto i = 0; i < sizeof(allocators) / sizeof(IAllocator*); i++)
    {
        SECTION(names[i])
        {
            SECTION("Specific alignment") 
            { SpecificAlignTest(allocators[i]); }

            SECTION("Specific alignment with offset")
            { SpecificAlignTest(allocators[i], 15); }

            SECTION("Struct alignment")
            { StructAlignTest(allocators[i]); }

            SECTION("Struct alignment with offset")
            { StructAlignTest(allocators[i], 10); }
        }
    }
}

void SpecificAlignTest(
    NamespaceName__::IAllocator* allocator, 
    NamespaceName__::SizeType offset)
{
    auto ptr_0 = allocator->Malloc(128,  8, offset);
    auto ptr_1 = allocator->Malloc(128, 16, offset);
    auto ptr_2 = allocator->Malloc(128, 32, offset);
    auto ptr_3 = allocator->Malloc(128, 64, offset);

    REQUIRE(0 == ((uintptr_t)ptr_0 + offset) %  8);
    REQUIRE(0 == ((uintptr_t)ptr_1 + offset) % 16);
    REQUIRE(0 == ((uintptr_t)ptr_2 + offset) % 32);
    REQUIRE(0 == ((uintptr_t)ptr_3 + offset) % 64);

    allocator->Free(ptr_0);
    allocator->Free(ptr_1);
    allocator->Free(ptr_2);
    allocator->Free(ptr_3);
}

void StructAlignTest(
    NamespaceName__::IAllocator* allocator, 
    NamespaceName__::SizeType offset)
{
    __declspec(align(8))  struct Aligned__8 { double v[16]; };
    __declspec(align(16)) struct Aligned_16 { double v[16]; };
    __declspec(align(32)) struct Aligned_32 { double v[16]; };
    __declspec(align(64)) struct Aligned_64 { double v[16]; };

    auto ptr_0 = allocator->Malloc_n<Aligned__8>(16, offset);
    auto ptr_1 = allocator->Malloc_n<Aligned_16>(16, offset);
    auto ptr_2 = allocator->Malloc_n<Aligned_32>(16, offset);
    auto ptr_3 = allocator->Malloc_n<Aligned_64>(16, offset);

    REQUIRE(0 == ((uintptr_t)ptr_0 + offset) %  8);
    REQUIRE(0 == ((uintptr_t)ptr_1 + offset) % 16);
    REQUIRE(0 == ((uintptr_t)ptr_2 + offset) % 32);
    REQUIRE(0 == ((uintptr_t)ptr_3 + offset) % 64);

    allocator->Free(ptr_0);
    allocator->Free(ptr_1);
    allocator->Free(ptr_2);
    allocator->Free(ptr_3);
}

#endif