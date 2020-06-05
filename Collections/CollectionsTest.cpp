#ifdef Test__

#define CATCH_CONFIG_RUNNER
#define CatchAvailable__

#include "Allocator.hpp"
#include "ArrayList.hpp"
// #include "DynamicBuffer.hpp"
#include "Bitset.hpp"

#pragma warning(push)
#pragma warning(disable : 26439)
#pragma warning(disable : 26444)
#pragma warning(disable : 26451)
#pragma warning(disable : 26495)

#include <catch.hpp>
#include "Tests/Allocator.inl"
#include "Tests/ArrayList.inl"
#include "Tests/Bitset.inl"

#pragma warning(pop)

int main(int argc, char** argv) 
{
    // global setup...
    int result = Catch::Session().run( argc, argv );
    // global clean-up...
    return result;
}

#endif