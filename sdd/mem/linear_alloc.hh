#ifndef _SDD_MEM_LINEAR_ALLOC_HH_
#define _SDD_MEM_LINEAR_ALLOC_HH_

#include <cassert>
#include <cstddef>
#include <memory>

namespace sdd { namespace mem {

/*------------------------------------------------------------------------------------------------*/

namespace {

/// @internal
struct arena
{
  using position_type = char*;

  const std::size_t size_;
  std::unique_ptr<char> buffer_;
  position_type position_;

  arena(std::size_t size)
  noexcept
    : size_(size)
    , buffer_(new char[size_])
    , position_(buffer_.get())
  {}

  ~arena()
  {
    assert(used() == 0 && "Memory arena not rewound.");
  }

  // Can't copy an arena.
  arena(const arena&) = delete;
  arena& operator=(const arena&) = delete;

  char*
  allocate(std::size_t n)
  {
    assert(pointer_in_buffer(position_) && "linear_alloc has outlived arena");
    if (buffer_.get() + size_ - position_ >= n)
    {
      char* r = position_;
      position_ += n;
      return r;
    }
    // Not enough room in the buffer, fallback to default allocator.
    return static_cast<char*>(::operator new(n));
  }

  void
  deallocate(char* p, std::size_t n)
  noexcept
  {
    assert(pointer_in_buffer(position_) && "linear_alloc has outlived arena");
    if (pointer_in_buffer(p))
    {
      if (p + n == position_) // The memory pointed by p was the last allocated one.
      {
        position_ = p;
      }
    }
    else
    {
      // The memory pointer by p was no allocated in this arena.
      ::operator delete(p);
    }
  }

  std::size_t
  used()
  const noexcept
  {
    return static_cast<std::size_t>(position_ - buffer_.get());
  }

  void
  reset()
  noexcept
  {
    position_ = buffer_.get();
  }

  void
  rewind(position_type pos)
  noexcept
  {
    assert(pointer_in_buffer(pos));
    position_ = pos;
  }

  position_type
  position()
  const noexcept
  {
    return position_;
  }

  bool
  pointer_in_buffer(char* p)
  const noexcept
  {
    return buffer_.get() <= p and p <= buffer_.get() + size_;
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Rewind an arena to its initial position.
///
/// Use RAII to ensure that the arena is always rewound to its initial position .
struct rewinder
{
  /// @brief The associated memory arena.
  arena& a_;

  /// @brief The position where to rewind the associated arena.
  const arena::position_type pos_;

  /// @brief Constructor.
  ///
  /// Initialize the position to the current position of the arena.
  rewinder(arena& a)
  noexcept
    : a_(a), pos_(a.position())
  {}

  /// @brief Destructor.
  ///
  /// Rewind the arena to its position when this object was constructed;
  ~rewinder()
  {
    a_.rewind(pos_);
  }

  // Can't copy a rewinder.
  rewinder(const rewinder&) = delete;
  rewinder& operator=(const rewinder&) = delete;

  // Can't move a rewinder.
  rewinder(rewinder&&) = delete;
  rewinder& operator=(rewinder&&) = delete;
};

/*------------------------------------------------------------------------------------------------*/

} // namespace anonymous

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Allocate memory contiguously in a preallocated buffer.
///
/// Memory is allocated by moving a pointer in the buffer. Only the memory which was the last
/// one thas was allocated can be deallocated. Thus, this allocator would mostly benefit to
/// recursive algorithms which need a stack. 
/// It's an adaptation of http://home.roadrunner.com/~hinnant/stack_alloc.html .
template <class T>
struct linear_alloc
{
  using value_type = T;

  arena& a_;

  template <class _Up>
  struct rebind
  {
    using other = linear_alloc<_Up>;
  };

  linear_alloc(arena& a)
  noexcept
    : a_(a)
  {}

  template <class U>
  linear_alloc(const linear_alloc<U>& other)
  noexcept
    : a_(other.a_)
  {}

  linear_alloc(const linear_alloc&) = default;
  linear_alloc& operator=(const linear_alloc&) = delete;

  T*
  allocate(std::size_t n)
  const
  {
    return reinterpret_cast<T*>(a_.allocate(n*sizeof(T)));
  }

  void
  deallocate(T* p, std::size_t n)
  const noexcept
  {
    a_.deallocate(reinterpret_cast<char*>(p), n*sizeof(T));
  }

  template <class U, class V>
  friend
  bool
  operator==(const linear_alloc<U>& lhs, const linear_alloc<V>& rhs)
  noexcept
  {
    return &lhs.a_ == &rhs.a_;
  }

  template <class U> friend struct linear_alloc;
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::mem

#endif // _SDD_MEM_LINEAR_ALLOC_HH_
