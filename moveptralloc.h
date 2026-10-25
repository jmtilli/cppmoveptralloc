// Author / copyright holder: Juha-Matti Tilli, 2026
// License: MIT

#ifndef _MOVEPTRALLOC_H_
#define _MOVEPTRALLOC_H_

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <memory>
#include <stdexcept>

namespace jmtilli { namespace mpa {

struct MovePtrAllocatorImpl {
  private:
    MovePtrAllocatorImpl(const MovePtrAllocatorImpl&) = delete;
    MovePtrAllocatorImpl &operator=(const MovePtrAllocatorImpl&) = delete;

    void new_arena(std::size_t align)
    {
        char *new_arena;
        new_arena = new char[arena_sz];
        std::memcpy(new_arena, &cur_arena, sizeof(cur_arena));
        cur_arena = new_arena;
        cur_arena_off = sizeof(cur_arena);
        if (cur_arena_off & (align-1))
        {
            std::size_t tomove = align - (cur_arena_off&(align-1));
            cur_arena_off += tomove;
        }
    }

    void *special_path(std::size_t sz, std::size_t align)
    {
        size_t alloc_off;
        size_t alloc_sz;
        char *next_arena;
        char *special_arena;
        std::memcpy(&next_arena, cur_arena, sizeof(next_arena));
        alloc_off = sizeof(next_arena);
        if (alloc_off & (align-1))
        {
            alloc_off += align - (alloc_off&(align-1));
        }
        alloc_sz = alloc_off + sz;
        special_arena = new char[alloc_sz];
        // Insert it to the linked list
        std::memcpy(special_arena, &next_arena, sizeof(next_arena));
        std::memcpy(cur_arena, &special_arena, sizeof(special_arena));
        return &special_arena[alloc_off];
    }

    char *cur_arena;
    std::size_t arena_sz;
    std::size_t direct_alloc_threshold;
    std::size_t cur_arena_off;

  public:
    MovePtrAllocatorImpl(std::size_t arena_sz, std::size_t direct_alloc_threshold): arena_sz(arena_sz), direct_alloc_threshold(direct_alloc_threshold)
    {
        char *null_arena = nullptr;
        cur_arena = new char[arena_sz];
        std::memcpy(cur_arena, &null_arena, sizeof(null_arena));
        cur_arena_off = sizeof(null_arena);
    }
    ~MovePtrAllocatorImpl() noexcept
    {
        char *next_arena;
        while (cur_arena)
        {
            std::memcpy(&next_arena, cur_arena, sizeof(next_arena));
            delete[] cur_arena;
            cur_arena = next_arena;
        }
    }

    void *alloc(std::size_t sz, std::size_t align)
    {
        void *result;
        if (align == 0 || (align&(align-1)) != 0 || align > SIZE_MAX-sizeof(char*))
        {
            throw std::invalid_argument("align not power of 2 or too big");
        }
        if (sz > SIZE_MAX - sizeof(char*) - align)
        {
            throw std::bad_alloc();
        }
        if (sz + sizeof(char*) + align > arena_sz || sz > direct_alloc_threshold)
        {
            // Uh-oh. Need a special arena for just this allocation.
            return special_path(sz, align);
        }
        if (cur_arena_off & (align-1))
        {
            std::size_t tomove = align - (cur_arena_off&(align-1));
            cur_arena_off += tomove;
        }
        if (cur_arena_off + sz > arena_sz)
        {
            new_arena(align);
        }
        result = cur_arena + cur_arena_off;
        cur_arena_off += sz;
        return result;
    }
};

template <typename T>
struct MovePtrTypeAllocator {
  private:
    std::shared_ptr<MovePtrAllocatorImpl> impl;
  public:
    using value_type = T;
    using propagate_on_container_copy_assignment = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type;

    MovePtrTypeAllocator(std::shared_ptr<MovePtrAllocatorImpl> impl) noexcept: impl(impl) {}

    template <typename U>
    constexpr MovePtrTypeAllocator(const MovePtrTypeAllocator<U> &other) noexcept: impl(other.impl) {}

    T* allocate(std::size_t n) {
        if (n > SIZE_MAX/sizeof(T))
        {
            throw std::bad_alloc();
        }
        void *ptr = impl->alloc(n*sizeof(T), alignof(T));
        return static_cast<T*>(ptr);
    }
    void deallocate(T* p, std::size_t n) noexcept {
        // nop
    }

    template <typename U>
    friend bool operator==(const MovePtrTypeAllocator<T> &a, const MovePtrTypeAllocator<U> &b) noexcept {
        return a.impl.get() == b.impl.get();
    }
    template <typename U>
    friend bool operator!=(const MovePtrTypeAllocator<T> &a, const MovePtrTypeAllocator<U> &b) noexcept {
        return !(a == b);
    }
};

std::shared_ptr<MovePtrAllocatorImpl> make_impl(size_t arenasz, size_t thresh)
{
    return std::make_shared<MovePtrAllocatorImpl>(arenasz, thresh);
}

template <typename T>
using TypeAlloc = MovePtrTypeAllocator<T>;

template<typename T, typename... Args>
T *alloc_construct_one(TypeAlloc<T> &alloc, Args&&... args)
{
  using Traits = std::allocator_traits<TypeAlloc<T> >;
  T *res;
  res = Traits::allocate(alloc, 1);
  Traits::construct(alloc, res, std::forward<Args>(args)...);
  return res;
}

template<typename T, typename... Args>
void destroy_dealloc_one(TypeAlloc<T> &alloc, T *t)
{
  using Traits = std::allocator_traits<TypeAlloc<T> >;
  Traits::destroy(alloc, t);
  Traits::deallocate(alloc, t, 1);
}

template<typename T, typename... Args>
T *alloc_construct_many(TypeAlloc<T> &alloc, std::size_t n, Args&&... args)
{
  using Traits = std::allocator_traits<TypeAlloc<T> >;
  T *res;
  std::size_t i;
  res = Traits::allocate(alloc, n);
  try
  {
    for (i = 0; i < n; i++)
    {
      Traits::construct(alloc, &res[i], std::forward<Args>(args)...);
    }
    return res;
  }
  catch (...)
  {
    for (; i > 0; i--)
    {
      Traits::destroy(alloc, &res[i-1]);
    }
    Traits::deallocate(alloc, res, n);
    throw;
  }
}

template<typename T, typename... Args>
void destroy_dealloc_many(TypeAlloc<T> &alloc, T *t, std::size_t n)
{
  std::size_t i;
  using Traits = std::allocator_traits<TypeAlloc<T> >;
  for (i = n; i > 0; i--)
  {
    Traits::destroy(alloc, &t[i-1]);
  }
  Traits::deallocate(alloc, t, n);
}

} }

#endif
