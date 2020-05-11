#pragma once

#include <SimpleMath.h>

using Color = DirectX::SimpleMath::Color;
using Vector2 = DirectX::SimpleMath::Vector2;
using Matrix = DirectX::SimpleMath::Matrix;

inline Color ToColor(const uint32_t color)
{
    using XMCOLOR = DirectX::PackedVector::XMCOLOR;
    auto xmc = XMCOLOR(color);
    return Color(xmc);
}