#pragma once

#include <EASTL\array.h>
#include <EASTL\type_traits.h>
#include <EASTL\algorithm.h>

#include <cstdint>
#include <cassert>

namespace m3
{
    using row_t = int8_t;
    using col_t = int8_t;
    using count_t = uint16_t;

    namespace private_
    {
        template <class T, int R, int C>
        struct Board
        {
            static_assert(eastl::is_pod<T>::value);
            using Array = eastl::array<T, R * C>;

            Array m_Array;

            // @Todo: Ideally should do nothing. Does this memset to 0?
            Board() = default;

            Board(const void* data, size_t dataSize) 
            {
                dataSize = eastl::min(dataSize, sizeof(m_Array));
                memcpy(m_Array.data(), data, dataSize); 
            }

            // Make with an inverted y-axis
            static void MakeInverted(const void* data, size_t dataSize, Board* outBoard)
            {
                assert(dataSize >= sizeof(m_Array));

                // @Todo: Copy whole rows
                auto ptr = (T*)data;
                for (auto i = 0; i < outBoard->Count(); i++)
                {
                    auto r = (outBoard->Rows() - 1) - (i / outBoard->Cols());
                    auto c = i % outBoard->Cols();

                    (*outBoard)(r, c) = ptr[i];
                }
            }

            inline row_t Rows() const { return R; }
            inline col_t Cols() const { return C; }
            inline count_t Count() const { return R * C; }

            inline count_t Index(row_t r, col_t c) const 
            { 
                assert(IsWithinBounds(r, c));
                return r * Cols() + c; 
            }

            inline       T& operator() (row_t r, col_t c)       { return m_Array[Index(r, c)]; }
            inline const T& operator() (row_t r, col_t c) const { return m_Array[Index(r, c)]; }

            inline bool IsWithinBounds(row_t r, col_t c) const
            {
                return r >= 0 && r < Rows() && c >= 0 && c < Cols();
            }
        };
    }

    using gem_id_t = uint8_t;
    using gem_color_t = uint8_t;

    const gem_color_t GemColors[] = { ' ', 'B', 'R', 'O', 'G', 'Y' };

    const row_t BoardRows = 8;
    const col_t BoardCols = 8;

    struct Gem
    {
        gem_id_t Id;
        gem_color_t Color;
    };

    template <class T>
    using Board = private_::Board<T, BoardRows, BoardCols>;

    struct Matches
    {
        struct { row_t Row; col_t Col_0, Col_1; } Row_0;
        struct { row_t Row; col_t Col_0, Col_1; } Row_1;
        struct { col_t Col; row_t Row_0, Row_1; } Col_0;
        struct { col_t Col; row_t Row_0, Row_1; } Col_1;
    };
}