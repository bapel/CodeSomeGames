#pragma once

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define pfk_is_pod(T__) __is_pod(T__)
#define pfk_max(a__, b__) (a__ > b__) ? a__ : b__
#define pfk_min(a__, b__) (a__ < b__) ? a__ : b__