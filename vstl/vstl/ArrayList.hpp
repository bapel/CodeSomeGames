#pragma once

#include "..\vx\allocator.hpp"

#define vstl_assert_bounds__(Pos__) assert(Pos__ < (m_end - m_buf))

namespace vstl {
namespace detail {
namespace buffer {

    //template <class T> 
    //using trivial = typename std::enable_if_t<std::is_trivial<T>, void>;

    //template <class T> 
    //vx_inline__ void construct(T* ptr)
    //{ 
    //    new(ptr)T; 
    //}

    //template <class T> 
    //vx_inline__ void construct(T* buf, size_t size) 
    //{ 
    //    for (auto p = buf; p < buf + size; p++) 
    //        new(p)T; 
    //}

    //template <class T> 
    //vx_inline__ void construct(T* buf, T* end)      
    //{ 
    //    for (auto p = buf; p < end; p++) 
    //        new(p)T;
    //}

    // Trivial variants of construct do nothing.
    // Use an explicit set to zero memory.

    //template <class T> vx_inline__ trivial<T> construct(T* buf) {}
    //template <class T> vx_inline__ trivial<T> construct(T* buf, size_t size) {}
    //template <class T> vx_inline__ trivial<T> construct(T* buf, T* end) {}

    // 

}}}

namespace vstl {

    template <class T>
    class ArrayList final
    {
        vx_assert_trivial__(T);

    private:
        T* m_buf;
        T* m_end;
        T* m_cap;
        vx::IAllocator* m_allocator;

    public:
        //struct PoD
        //{
        //    T* buf;
        //    T* end;
        //    T* cap;
        //    vx::IAllocator* allocator;
        //};

        //vx_inline__ ArrayList(PoD& pod) :
        //    m_buf(pod.buf),
        //    m_end(pod.end),
        //    m_cap(pod.cap),
        //    m_allocator(pod.allocator)
        //{ }

        //vx_inline__ PoD to_pod()
        //{
        //    PoD pod = { m_buf, m_end, m_cap, m_allocator };
        //    m_buf = m_end = m_cap = nullptr;
        //    return pod;
        //}

        // Construction.

        vx_inline__ ArrayList() : 
            ArrayList(vx::fallback_allocator()) 
        { }

        vx_inline__ explicit ArrayList(vx::IAllocator* allocator) : 
            m_buf(nullptr),
            m_end(nullptr),
            m_cap(nullptr),
            m_allocator(allocator) 
        { }
        
        vx_inline__ explicit ArrayList(size_t count) :
            ArrayList(count, vx::fallback_allocator())
        { }

        vx_inline__ explicit ArrayList(size_t count, vx::IAllocator* allocator)
        {
            m_allocator = allocator;
            m_buf = m_allocator->malloc_n<T>(count);
            m_end = m_buf + count;
            m_cap = m_end;

            // detail::buffer::construct<T>(m_buf, m_end - m_buf);
        }

        // @Todo: specialize for int types?
        vx_inline__ explicit ArrayList(size_t count, const T& value) : 
            ArrayList(count, value, vx::fallback_allocator()) 
        { }

        // @Todo: specialize for int types?
        vx_inline__ explicit ArrayList(size_t count, const T& value, vx::IAllocator* allocator)
        {
            m_allocator = allocator;
            m_buf = m_allocator->malloc_n<T>(count);
            m_end = m_buf + count;
            m_cap = m_end;

            for (auto i = 0U; i < count; i++)
                m_buf[i] = value;
        }

        // @Intentional: Not implemented. Use memcpy/copy instead.
        ArrayList(const ArrayList&) = delete;

        vx_inline__ ArrayList(ArrayList&& other) :
            m_buf(other.m_buf),
            m_end(other.m_end),
            m_cap(other.m_cap),
            m_allocator(other.m_allocator)
        {
            other.m_buf = nullptr;
            other.m_end = nullptr;
            other.m_cap = nullptr;
        }

        // @Todo: Consider?
        // vx_inline__ array_list(array_list&& other, vx::IAllocator* allocator);

        vx_inline__ ArrayList(std::initializer_list<T> ilist) :
            ArrayList(ilist.size(), vx::fallback_allocator())
        {
            const auto size = sizeof(T) * ilist.size();
            memcpy(m_buf, ilist.begin(), size);
        }

        vx_inline__ ArrayList(std::initializer_list<T> ilist, vx::IAllocator* allocator) :
            ArrayList(ilist.size(), allocator)
        {
            const auto size = sizeof(T) * ilist.size();
            memcpy(m_buf, ilist.begin(), size);
        }

        vx_inline__ ~ArrayList() { destroy(); }

        // Assignment.

        // @Intentional: Not implemented. Use memcpy/copy instead.
        vx_inline__ ArrayList& operator= (const ArrayList&) = delete;

        vx_inline__ ArrayList& operator= (ArrayList&& other)
        {
            if (m_allocator == other.m_allocator)
            {
                if (m_buf != nullptr)
                    m_allocator->free(m_buf);

                m_buf = other.m_buf;
                m_end = other.m_end;
                m_cap = other.m_cap;

                other.m_buf = nullptr;
                other.m_end = nullptr;
                other.m_cap = nullptr;

                return *this;
            }

            const size_t cap = m_cap - m_buf;
            if (cap < other.size())
                set_capacity(other.size());

            const auto size = sizeof(T) * other.size();
            memcpy(m_buf, other.begin(), size);
            m_end = m_buf + other.size();

            other.clear();

            return *this;
        }
        
        vx_inline__ ArrayList& operator= (std::initializer_list<T> ilist)
        {
            const size_t cap = m_cap - m_buf;
            if (cap < ilist.size())
                set_capacity(ilist.size());

            const auto size = sizeof(T) * ilist.size();
            memcpy(m_buf, ilist.begin(), size);
            m_end = m_buf + ilist.size();

            return *this;
        }

        // Element access.

        vx_inline__ T& at(size_t pos)
        {
            vstl_assert_bounds__(pos);
            return m_buf[pos];
        };

        vx_inline__ const T& at(size_t pos) const 
        {
            vstl_assert_bounds__(pos);
            return m_buf[pos];
        };

        constexpr       T& operator[](size_t pos)       { return m_buf[pos]; }
        constexpr const T& operator[](size_t pos) const { return m_buf[pos]; }

        vx_inline__       T* data()        { return m_buf; }
        vx_inline__ const T* data() const  { return m_buf; }

        vx_inline__       T& front()       { return *m_buf; }
        vx_inline__ const T& front() const { return *m_buf; }

        vx_inline__       T& back()        { return *(m_end - 1); }
        vx_inline__ const T& back() const  { return *(m_end - 1); }

        // Iterators.

        vx_inline__       T* begin()        { return m_buf; }
        vx_inline__ const T* begin() const  { return m_buf; }
        vx_inline__ const T* cbegin() const { return m_buf; }

        vx_inline__       T* end()          { return m_end; }
        vx_inline__ const T* end() const    { return m_end; }
        vx_inline__ const T* cend() const   { return m_end; }

        // Capacity.

        vx_inline__ bool empty() const      { return m_end == m_buf; }
        vx_inline__ size_t size() const     { return m_end - m_buf; }
        vx_inline__ size_t max_size() const { return _UI32_MAX; }
        vx_inline__ size_t capacity() const { return m_cap - m_buf; }

        vx_inline__ void reserve(size_t new_cap)
        {
            const size_t old_cap = m_cap - m_buf;
            const size_t old_cnt = m_end - m_buf;

            if (old_cap >= new_cap)
            {
                if (old_cnt > new_cap)
                    m_end = m_buf + new_cap;

                return;
            }

            set_capacity(new_cap);
        }

        vx_inline__ void shrink_to_fit()
        {
            const auto count = m_end - m_buf;
            
            if (count == 0)
            {
                destroy();
                return;
            }
                
            shrink_to_fit(count);
        }

        // Modifiers.

        vx_inline__ void clear() { m_end = m_buf; }
        
        T* insert(const T* pos, const T& value)
        {
            assert(pos <= m_end);

            auto dst = const_cast<T*>(pos);

            if (m_cap == m_end)
            {
                const auto offset = pos - m_buf;
                const size_t cap = m_cap - m_buf;
                set_capacity(cap << 1);
                dst = m_buf + offset;
            }

            const auto size = m_end - dst;
            memmove(dst + 1, dst, sizeof(T) * size);
            *dst = value;
            ++m_end;

            return dst;
        }

        T* erase(const T* pos)
        {
            assert(pos <= m_end);

            auto dst = const_cast<T*>(pos);
            const auto size = m_end - dst - 1;
            memmove(dst, dst + 1, sizeof(T) * size);
            --m_end;

            return dst;
        }

        T* erase_unordered(const T* pos)
        {
            assert(pos <= m_end);

            auto dst = const_cast<T*>(pos);
            *dst = *(m_end - 1);
            --m_end;

            return dst;
        }

        T* erase(const T* first, const T* last)
        {
            assert(first >= m_buf);
            assert(last <= m_end);

            if (last == m_end)
            {
                m_end -= (last - first);
                return m_end;
            }

            const auto range = last - first;
            const auto size = m_end - last;
            auto dst = const_cast<T*>(first);

            memmove(dst, first + range, sizeof(T) * size);
            m_end -= (last - first);

            return const_cast<T*>(last);
        }

        void push_back(const T& value)
        {
            if (m_end == m_cap)
            {
                const size_t cap = m_cap - m_buf;
                set_capacity(cap << 1);
            }

            *m_end = value;
            ++m_end;
        }

        void pop_back()
        {
            assert(m_buf != m_end);

            --m_end;
        }

        void resize(size_t count)
        {
            if (m_buf + count > m_end)
                set_capacity(count);

            m_end = m_buf + count;
        }

        // @Todo: specialize for int types?
        void resize(size_t count, const T& value)
        {
            const auto old_end = m_end;
            if (m_buf + count > m_end)
                set_capacity(count);

            m_end = m_buf + count;

            for (const auto i = old_end; i < m_end; i++)
                *i = value;
        }

    private:
        void set_capacity(size_t new_cap)
        {
            new_cap = vx::max(new_cap, 8ULL);

            auto new_begin = m_allocator->malloc_n<T>(new_cap);
            const auto count = m_end - m_buf;

            memcpy(new_begin, m_buf, sizeof(T) * count);
            m_allocator->free(m_buf);

            m_buf = new_begin;
            m_end = m_buf + count;
            m_cap = m_buf + new_cap;
        }

        void destroy()
        {
            m_allocator->free(m_buf);
            m_buf = nullptr;
            m_end = nullptr;
            m_cap = nullptr;
        }
    };

    template <class T>
    bool operator == (const ArrayList<T>& a, const ArrayList<T>& b)
    {
        const auto begin = a.begin();
        const auto end = a.end();
        const auto size = end - begin;
        
        if (size != b.end() - b.begin())
            return false; 

        return memcmp(a.begin(), b.begin(), size * sizeof(T)) == 0;
    }
}

#undef vstl_assert_bounds__