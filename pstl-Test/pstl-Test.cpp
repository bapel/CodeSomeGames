#define CATCH_CONFIG_RUNNER

#pragma warning(push)
#pragma warning(disable : 26439)
#pragma warning(disable : 26444)
#pragma warning(disable : 26451)
#pragma warning(disable : 26495)

#include <catch.hpp>

#include "tests\AllocatorTests.hpp"
#include "tests\ArrayTests.hpp"

#pragma warning(pop)

int main(int argc, char** argv) 
{
    // global setup...
    int result = Catch::Session().run( argc, argv );
    // global clean-up...
    return result;
}