#pragma once

#include "../Allocator.hpp"
#include <functional> // std::hash
#include <intrin.h> // __lzcnt

const __m128i set1s[] = 
{
    _mm_set1_epi8(0),
    _mm_set1_epi8(1),
    _mm_set1_epi8(2),
    _mm_set1_epi8(3),
    _mm_set1_epi8(4),
    _mm_set1_epi8(5),
    _mm_set1_epi8(6),
    _mm_set1_epi8(7),
    _mm_set1_epi8(8),
    _mm_set1_epi8(9),
    _mm_set1_epi8(10),
    _mm_set1_epi8(11),
    _mm_set1_epi8(12),
    _mm_set1_epi8(13),
    _mm_set1_epi8(14),
    _mm_set1_epi8(15),
    _mm_set1_epi8(16),
    _mm_set1_epi8(17),
    _mm_set1_epi8(18),
    _mm_set1_epi8(19),
    _mm_set1_epi8(20),
    _mm_set1_epi8(21),
    _mm_set1_epi8(22),
    _mm_set1_epi8(23),
    _mm_set1_epi8(24),
    _mm_set1_epi8(25),
    _mm_set1_epi8(26),
    _mm_set1_epi8(27),
    _mm_set1_epi8(28),
    _mm_set1_epi8(29),
    _mm_set1_epi8(30),
    _mm_set1_epi8(31),
    _mm_set1_epi8(32),
    _mm_set1_epi8(33),
    _mm_set1_epi8(34),
    _mm_set1_epi8(35),
    _mm_set1_epi8(36),
    _mm_set1_epi8(37),
    _mm_set1_epi8(38),
    _mm_set1_epi8(39),
    _mm_set1_epi8(40),
    _mm_set1_epi8(41),
    _mm_set1_epi8(42),
    _mm_set1_epi8(43),
    _mm_set1_epi8(44),
    _mm_set1_epi8(45),
    _mm_set1_epi8(46),
    _mm_set1_epi8(47),
    _mm_set1_epi8(48),
    _mm_set1_epi8(49),
    _mm_set1_epi8(50),
    _mm_set1_epi8(51),
    _mm_set1_epi8(52),
    _mm_set1_epi8(53),
    _mm_set1_epi8(54),
    _mm_set1_epi8(55),
    _mm_set1_epi8(56),
    _mm_set1_epi8(57),
    _mm_set1_epi8(58),
    _mm_set1_epi8(59),
    _mm_set1_epi8(60),
    _mm_set1_epi8(61),
    _mm_set1_epi8(62),
    _mm_set1_epi8(63),
    _mm_set1_epi8(64),
    _mm_set1_epi8(65),
    _mm_set1_epi8(66),
    _mm_set1_epi8(67),
    _mm_set1_epi8(68),
    _mm_set1_epi8(69),
    _mm_set1_epi8(70),
    _mm_set1_epi8(71),
    _mm_set1_epi8(72),
    _mm_set1_epi8(73),
    _mm_set1_epi8(74),
    _mm_set1_epi8(75),
    _mm_set1_epi8(76),
    _mm_set1_epi8(77),
    _mm_set1_epi8(78),
    _mm_set1_epi8(79),
    _mm_set1_epi8(80),
    _mm_set1_epi8(81),
    _mm_set1_epi8(82),
    _mm_set1_epi8(83),
    _mm_set1_epi8(84),
    _mm_set1_epi8(85),
    _mm_set1_epi8(86),
    _mm_set1_epi8(87),
    _mm_set1_epi8(88),
    _mm_set1_epi8(89),
    _mm_set1_epi8(90),
    _mm_set1_epi8(91),
    _mm_set1_epi8(92),
    _mm_set1_epi8(93),
    _mm_set1_epi8(94),
    _mm_set1_epi8(95),
    _mm_set1_epi8(96),
    _mm_set1_epi8(97),
    _mm_set1_epi8(98),
    _mm_set1_epi8(99),
    _mm_set1_epi8(100),
    _mm_set1_epi8(101),
    _mm_set1_epi8(102),
    _mm_set1_epi8(103),
    _mm_set1_epi8(104),
    _mm_set1_epi8(105),
    _mm_set1_epi8(106),
    _mm_set1_epi8(107),
    _mm_set1_epi8(108),
    _mm_set1_epi8(109),
    _mm_set1_epi8(110),
    _mm_set1_epi8(111),
    _mm_set1_epi8(112),
    _mm_set1_epi8(113),
    _mm_set1_epi8(114),
    _mm_set1_epi8(115),
    _mm_set1_epi8(116),
    _mm_set1_epi8(117),
    _mm_set1_epi8(118),
    _mm_set1_epi8(119),
    _mm_set1_epi8(120),
    _mm_set1_epi8(121),
    _mm_set1_epi8(122),
    _mm_set1_epi8(123),
    _mm_set1_epi8(124),
    _mm_set1_epi8(125),
    _mm_set1_epi8(126),
    _mm_set1_epi8(127)
};

namespace NamespaceName__
{
    template <class K, class H = std::hash<K>>
    class SimdHashSet
    {
    public:
        using KeyType = K;
        using HashFunction = H;

        SimdHashSet() = default;
        SimdHashSet(IAllocator* allocator) : m_Allocator(allocator)  {}

        SimdHashSet(CountType capacity, IAllocator* allocator = GetFallbackAllocator()) :
            SimdHashSet(allocator)
        { EnsureCapacity(capacity); }

        ~SimdHashSet()
        { m_Allocator->Free(m_Control); }

        CountType Count() const { return m_Count; }
        CountType Capacity() const { return m_Capacity; }

        __inline bool Add(const KeyType& key)
        { return Add(key, Hash(key)); }

        __inline bool ShouldRehash() const
        { return (m_Count + 1) >= (m_Capacity - (m_Capacity >> 1)); }

        bool Add(const KeyType& key, uint64_t hash)
        {
            if (m_Capacity == 0)
                Rehash(16);

            auto [exists, index] = FindForAdd(key, hash);

            if (exists)
                return false;

            if (ShouldRehash())
            {
                Rehash(m_Capacity << 1);
                std::tie(exists, index) = FindForAdd(key, hash);
            }

            SetControl(index, H2(hash));
            m_Slots[index] = key;
            m_Count++;

            return true;
        }

        __inline bool Contains(ConstRefType<KeyType> key) const
        {
            assert(m_Capacity > 0);
            auto [found, index] = Find(key, Hash(key));
            return found;
        }

        __inline bool Contains(ConstRefType<KeyType> key, uint64_t hash) const
        {
            assert(m_Capacity > 0);
            auto [found, index] = Find(key, hash);
            return found;
        }

        bool Remove(ConstRefType<KeyType> key)
        {
            assert(m_Capacity > 0);

            const auto hash = Hash(key);
            auto [found, index] = Find(key, hash);

            if (found)
            {
                SetControl(index, k_Deleted);
                m_Count--;
                return true;
            }

            return false;
        }

        __inline void Clear()
        { memset(m_Control, k_Empty_x4, m_Capacity); }

        void EnsureCapacity(CountType minCapacity)
        { Rehash(CalcCapacity(minCapacity)); }

        void GetKeys(KeyType* outKeys) const
        {
            for (auto i = 0U; i < m_Capacity; i++)
            {
                if (k_Empty == m_Control[i] || k_Deleted == m_Control[i])
                    continue;

                outKeys.Add(m_Slots[i]);
            }
        }

    private:
        // Capacity is always a power of two and a multiple of 16.
        // Note that all powers of two > 16 are multiple of 16.
        __inline static CountType CalcCapacity(CountType n)
        {
            if (n < 16)
                return 16;

            n--;
            n |= n >> 1;
            n |= n >> 2;
            n |= n >> 4;
            n |= n >> 8;
            n |= n >> 16;
            return n + 1;
        }

        __inline static uint64_t Hash(KeyType x) { return HashFunction()(x); }
        __inline static uint64_t H1(uint64_t hash) { return hash >> 7; }
        __inline static uint8_t  H2(uint64_t hash) { return hash & 0b0111'1111U; }

        __inline IndexType Index(uint64_t hash) const
        { return H1(hash) & (m_Capacity - 1); }

        __inline void SetControl(IndexType index, uint8_t control) const
        {
            m_Control[index] = control;
            if (index < 16)
                m_Control[index + m_Capacity] = control;
        }

        __inline std::pair<bool, IndexType> FindForAdd(const KeyType& key, uint64_t hash) const
        {
        #ifdef HashMetrics__
            auto probeLength = 0U;
        #endif
            const auto index = Index(hash);
            const auto h2 = H2(hash);

            auto pos = index;
            do
            {
                if (k_Empty == m_Control[pos] || k_Deleted == m_Control[pos])
                    return { false, pos };

                if (h2 == m_Control[pos] && key == m_Slots[pos])
                    return { true, pos };

            #ifdef HashMetrics__
                probeLength++;
                MaxProbeLength = Max(probeLength, MaxProbeLength);
            #endif
                pos = (pos + 1) & (m_Capacity - 1);
            }
            // @Todo: Have a max probe length? instead of wrapping around the whole table.
            while (pos != index);

            return { false, -1 };
        }

        __inline std::pair<bool, IndexType> Find(const KeyType& key, uint64_t hash) const
        {
            const auto index = Index(hash);
            const auto h2 = H2(hash);
            const auto mask = m_Capacity - 1;
            const auto value = set1s[h2];//_mm_set1_epi8(h2);

            auto pos = index;
            do
            {
                const auto hashes = *reinterpret_cast<__m128i*>(m_Control + pos);
                auto result = _mm_movemask_epi8(_mm_cmpeq_epi8(value, hashes));

                auto i = 0;
                while (result != 0)
                {
                    if (result & 1)
                    {
                        const auto offset = (pos + i) & mask;
                        if (key == m_Slots[offset])
                            return { true, offset };

                        result >>= 1;
                        i++;
                        continue;
                    }

                    unsigned long shift;
                    _BitScanForward64(&shift, result);
                    result >>= shift;
                    i += shift;
                }

                result = _mm_movemask_epi8(_mm_cmpeq_epi8(k_Empty_x16, hashes));
                if (result != 0)
                    return { false, pos };

                pos = (pos + 16) & mask;
            }
            // @Todo: Have a max probe length? instead of wrapping around the whole table.
            while (pos != index);

            return { false, -1 };
        }

        void Rehash(CountType newCapacity)
        {
            assert(newCapacity <= k_MaxCapacity);

            // src for copying.
            auto control = m_Control;
            auto slots = m_Slots;
            auto capacity = m_Capacity;
            auto count = m_Count;

            const auto sizeOfControl = newCapacity + 16;
            const auto sizeOfSlots = newCapacity * sizeof(KeyType);
            const auto size = sizeOfControl + sizeOfSlots;

            m_Control = (uint8_t*)m_Allocator->Malloc(size, 16);
            m_Slots = (KeyType*)(m_Control + sizeOfControl);
            m_Capacity = newCapacity;
            m_Count = 0;

            memset(m_Control, k_Empty_x4, sizeOfControl);

            for (auto i = 0U; i < capacity; i++)
            {
                if (k_Empty != control[i] && 
                    k_Deleted != control[i])
                    Add(slots[i]);
            }

            m_Allocator->Free(control);
        }

    private:
        constexpr static HashFunction k_HashFunction = HashFunction();
        constexpr static auto k_MaxCapacity = 4'294'967'296; // 2^32

        const static auto k_Empty    = 0b1000'0000U; // 0x80
        const static auto k_Deleted  = 0b1111'1110U;
        const static auto k_Sentinel = 0b1111'1111U;
        const static auto k_Empty_x4 = 0x80808080; // k_Empty x4
        const static inline auto k_Empty_x16 = _mm_set1_epi8(k_Empty);

        uint8_t* m_Control = nullptr;
        KeyType* m_Slots = nullptr;
        IAllocator* m_Allocator = GetFallbackAllocator();
        CountType m_Capacity = 0;
        CountType m_Count = 0;

    public:
    #ifdef HashMetrics__
        static inline uint32_t MaxProbeLength = 0;
    #endif
    };
}