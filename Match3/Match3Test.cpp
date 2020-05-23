#ifdef Test__

#define CATCH_CONFIG_RUNNER
#define CatchAvailable__

#include <SDL.h>
#include <catch.hpp>

#include "m3Board.hpp"
#include "m3Match.hpp"

int main(int argc, char** argv) 
{
    // global setup...
    int result = Catch::Session().run( argc, argv );
    // global clean-up...
    return result;
}

#endif