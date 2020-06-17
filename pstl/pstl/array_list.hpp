#pragma once

#include "pstl\detail\array_list_base.hpp"
#include "pstl\allocator.hpp"

namespace pstl {

    template <class T,
    #ifdef pstl_is_debug__
        bool assert_bounds_ = true
    #else
        bool assert_bounds_ = false
    #endif
    >
    class array_list final : public detail::array_list_base<T, assert_bounds_>
    {
        pstl_assert_trivial__(T);
        using base = detail::array_list_base<T, assert_bounds_>;

    private:
        using base::m_begin;
        using base::m_end;
        T* m_capacity = nullptr;
        allocator* m_allocator = fallback_allocator();

    public:
        // Construction.

        pstl_inline__ array_list() = default;
        pstl_inline__ explicit array_list(allocator* allocator) : array_list(), m_allocator(allocator) {}
        
        pstl_inline__  explicit array_list(size_t n, allocator* allocator = fallback_allocator()) :
            base(allocator->malloc_n<T>(n), n),
            m_capacity(m_begin + n),
            m_allocator(allocator)
        { }

        // @Todo: specialize for int types?
        pstl_inline__  explicit array_list(size_t n, const T& x, allocator* allocator = fallback_allocator()) :
            base(allocator->malloc_n<T>(n), n),
            m_capacity(m_end),
            m_allocator(allocator)
        {
            for (auto i = 0U; i < n; i++)
                m_begin[i] = x;
        }

        // @Intentional: Not implemented. Use memcpy/copy instead.
        array_list(const array_list&) = delete;

        pstl_inline__ array_list(array_list&& other) :
            m_begin(other.m_begin),
            m_end(other.m_end),
            m_capacity(other.m_capacity),
            m_allocator(other.m_allocator)
        {
            other.m_begin = nullptr;
            other.m_end = nullptr;
            other.m_capacity = nullptr;
        }

        // @Todo: Consider?
        // pstl_inline__ array_list(array_list&& other, allocator* allocator);

        pstl_inline__ array_list(std::initializer_list<T> ilist, allocator* allocator = fallback_allocator()) :
            array_list(ilist.size(), allocator)
        {
            const auto size = sizeof(T) * ilist.size();
            memcpy(m_begin, ilist.begin(), size);
        }

        pstl_inline__ ~array_list()
        { m_allocator->free(m_begin); }

        // Assignment.
        // @Todo: implement?

        // array_list& operator= (array_list&& other)
        
        pstl_inline__ array_list& operator= (std::initializer_list<T> ilist)
        {
            const size_t cap = m_capacity - m_begin;
            if (cap < ilist.size())
                set_capacity(ilist.size());

            const auto size = sizeof(T) * ilist.size();
            memcpy(m_begin, ilist.begin(), size);
            m_end = m_begin + cap;

            return *this;
        }

        // Element access.

        pstl_inline__ T& at(size_t index)
        {
            assert(index < (m_end - m_begin));
            return m_begin[index];
        };

        pstl_inline__ const T& at(size_t index) const 
        {
            assert(index < (m_end - m_begin));
            return m_begin[index];
        };

        pstl_inline__       T* data()        { return m_begin; }
        pstl_inline__ const T* data() const  { return m_begin; }

        pstl_inline__       T& front()       { return *m_begin; }
        pstl_inline__ const T& front() const { return *m_begin; }

        pstl_inline__       T& back()        { return *(m_end - 1); }
        pstl_inline__ const T& back() const  { return *(m_end - 1); }

        // Iterators.

        pstl_inline__       T* begin()        { return m_begin; }
        pstl_inline__ const T* begin() const  { return m_begin; }
        pstl_inline__ const T* cbegin() const { return m_begin; }

        pstl_inline__       T* end()          { return m_end; }
        pstl_inline__ const T* end() const    { return m_end; }
        pstl_inline__ const T* cend() const   { return m_end; }

        // Capacity.

        pstl_inline__ bool empty() const      { return m_end == m_begin; }
        pstl_inline__ size_t size() const     { return m_end - m_begin; }
        pstl_inline__ size_t max_size() const { return _UI32_MAX; }
        pstl_inline__ size_t capacity() const { return m_capacity - m_begin; }

        pstl_inline__ void reserve(size_t new_cap)
        {
            const size_t old_cap = m_capacity - m_begin;
            const size_t old_cnt = m_end - m_begin;

            if (old_cap >= new_cap)
            {
                if (old_cnt > new_cap)
                    m_end = m_begin + new_cap;

                return;
            }

            set_capacity(new_cap);
        }

        pstl_inline__ void shrink_to_fit()
        {
            const auto count = m_end - m_begin;
            
            if (count == 0)
            {
                destroy();
                return;
            }
                
            shrink_to_fit(count);
        }

        // Modifiers.

        pstl_inline__ void clear() { m_end = m_begin; }
        
        T* insert(const T* pos, const T& value)
        {
            assert(pos <= m_end);

            auto dst = const_cast<T*>(pos);

            if (m_capacity == m_end)
            {
                const auto offset = pos - m_begin;
                const size_t cap = m_capacity - m_begin;
                set_capacity(cap << 1);
                dst = m_begin + offset;
            }

            const auto size = m_end - dst;
            memmove(dst + 1, dst, sizeof(T) * size);
            *dst = value;
            ++m_end;

            return dst;
        }

        T* erase(const T* pos)
        {
            assert(pos < m_end);

            auto dst = const_cast<T*>(pos);
            const auto size = m_end - dst - 1;
            memmove(dst, dst + 1, sizeof(T) * size);
            --m_end;

            return dst;
        }

        T* erase_unordered(const T* pos)
        {
            assert(pos < m_end);

            auto dst = const_cast<T*>(pos);
            *dst = *(m_end - 1);
            --m_end;

            return dst;
        }

        T* erase(const T* first, const T* last)
        {
            assert(first >= m_begin);
            assert(last < m_end);

            const auto range = last - first;
            const auto size = m_end - last;

            memmove(first, first + range, sizeof(T) * size);
            m_end -= (last - first);

            return last;
        }

        void push_back(const T& value)
        {
            if (m_end == m_capacity)
            {
                const size_t cap = m_capacity - m_begin;
                set_capacity(cap << 1);
            }

            *m_end = value;
            ++m_end;
        }

        void pop_back()
        {
            assert(m_begin != m_end);

            --m_end;
        }

        void resize(size_t count)
        {
            if (m_begin + count > m_end)
                set_capacity(count);

            m_end = m_begin + count;
        }

        // @Todo: specialize for int types?
        void resize(size_t count, const T& value)
        {
            const auto old_end = m_end;
            if (m_begin + count > m_end)
                set_capacity(count);

            m_end = m_begin + count;

            for (const auto i = old_end; i < m_end; i++)
                *i = value;
        }

    private:
        void set_capacity(size_t new_cap)
        {
            new_cap = max(new_cap, 8);

            auto new_begin = m_allocator->malloc_n<T>(new_cap);
            const auto count = m_end - m_begin;

            memcpy(new_begin, m_begin, sizeof(T) * count);
            m_allocator->free(m_begin);

            m_begin = new_begin;
            m_end = m_begin + count;
            m_capacity = m_begin + new_cap;
        }

        void destroy()
        {
            m_allocator->free(m_begin);
            m_begin = nullptr;
            m_end = nullptr;
            m_capacity = nullptr;
        }
    };

    template <class T, bool assert_bounds_>
    bool operator == (
        const array_list<T, assert_bounds_>& a, 
        const array_list<T, assert_bounds_>& b)
    {
        const auto begin = a.begin();
        const auto end = a.end();
        const auto size = end - begin;
        
        if (size != b.end() - b.begin())
            return false; 

        return memcmp(a.begin(), b.begin(), size * sizeof(T)) == 0;
    }
}