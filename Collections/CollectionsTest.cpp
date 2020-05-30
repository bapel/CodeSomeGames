#ifdef Test__

#define CATCH_CONFIG_RUNNER
#define CatchAvailable__

#include <catch.hpp>

#include "Allocator.hpp"
#include "ArrayList.hpp"

int main(int argc, char** argv) 
{
    // global setup...
    int result = Catch::Session().run( argc, argv );
    // global clean-up...
    return result;
}

#endif