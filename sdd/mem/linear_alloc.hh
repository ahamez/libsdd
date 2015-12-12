/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <cassert>
#include <cstddef>
#include <memory> // unique_ptr

namespace sdd { namespace mem {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief A memory arena for linear_alloc.
class arena
{
  // Can't copy an arena.
  arena(const arena&) = delete;
  arena& operator=(const arena&) = delete;

public:

  /// @brief The type of position in the memory buffer.
  using position_type = char*;

private:

  /// @brief The size of this memory arena.
  const std::size_t size_;

  /// @brief The underlying memory buffer.
  std::unique_ptr<char[]> buffer_;

  /// @brief The beginning of the free memory.
  position_type position_;

#ifndef NDEBUG
  /// @brief The number of time this arena has been used with a rewinder.
  unsigned int active_;

  /// @brief The number of allocated bytes when this arena wasn't referenced by any rewinder.
  std::size_t unactive_allocated_;
#endif

public:

  /// @brief Construct an arena with a given size.
  arena(std::size_t size)
  noexcept
    : size_(size)
    , buffer_(std::make_unique<char[]>(size_))
    , position_(buffer_.get())
#ifndef NDEBUG
    , active_(0)
    , unactive_allocated_(0)
#endif
  {}

#ifndef NDEBUG
  ~arena()
  {
    // We admit that some bytes can never be deallocated. As a matter of fact, it would be very
    // complex to keep track of the memory allocated when no rewinder is used. For instance,
    // when the user creates a sum, the underlying container uses the linear allocator, but without
    // a rewinder. This is why we keep a trace of the number of rewinders with active_.
    assert(used() == unactive_allocated_ && "Memory arena not rewound.");
  }
#endif

  char*
  allocate(std::size_t n)
  {
    assert(pointer_in_buffer(position_) && "linear_alloc has outlived arena");
    // The following temporary variable avoid a warning about comparison of signed and unsigned
    // values: pointer arithmetic gives a std::ptrdiff_t (signed), but allocate() takes a
    // std::size_t (unsigned). By construction, we are sure that position_ is always between
    // buffer_.get() and buffer_.get() + size_, so the difference is always > 0.
    const std::size_t diff = buffer_.get() + size_ - position_;
    if (diff >= n)
    {
#ifndef NDEBUG
      if (active_ == 0)
      {
        unactive_allocated_ += n;
      }
#endif
      char* r = position_;
      position_ += n;
      return r;
    }
    // Not enough room in the buffer, fallback to the default allocator.
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
#ifndef NDEBUG
        if (active_ == 0)
        {
          unactive_allocated_ -= n;
        }
#endif
        position_ = p;
      }
    }
    else
    {
      // The memory pointed by p was no allocated in this arena.
      ::operator delete(p);
    }
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

#ifndef NDEBUG
  std::size_t
  used()
  const noexcept
  {
    return static_cast<std::size_t>(position_ - buffer_.get());
  }

  void
  activate()
  noexcept
  {
    active_ += 1;
  }

  void
  deactivate()
  noexcept
  {
    active_ -= 1;
  }
#endif

private:

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
class rewinder
{
  // Can't copy a rewinder.
  rewinder(const rewinder&) = delete;
  rewinder& operator=(const rewinder&) = delete;

  // Can't move a rewinder.
  rewinder(rewinder&&) = delete;
  rewinder& operator=(rewinder&&) = delete;

private:

  /// @brief The associated memory arena.
  arena& a_;

  /// @brief The position where to rewind the associated arena.
  const arena::position_type pos_;

public:

  /// @brief Constructor.
  ///
  /// Initialize the position to the current position of the arena.
  rewinder(arena& a)
  noexcept
    : a_(a), pos_(a.position())
  {
#ifndef NDEBUG
    a_.activate();
#endif
  }

  /// @brief Destructor.
  ///
  /// Rewind the arena to its position when this object was constructed;
  ~rewinder()
  {
    a_.rewind(pos_);
#ifndef NDEBUG
    a_.deactivate();
#endif
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Allocate memory contiguously in a preallocated buffer.
///
/// Memory is allocated by moving a pointer in the buffer. Only the memory which was the last
/// one thas was allocated can be deallocated. Thus, this allocator would mostly benefit to
/// recursive algorithms which need a stack. 
/// It's an adaptation of http://howardhinnant.github.io/stack_alloc.html .
template <typename T>
class linear_alloc
{
public:

  using value_type = T;

private:

  arena& a_;

public:

  template <typename _Up>
  struct rebind
  {
    using other = linear_alloc<_Up>;
  };

  linear_alloc(arena& a)
  noexcept
    : a_(a)
  {}

  template <typename U>
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

  template <typename U, typename V>
  friend
  bool
  operator==(const linear_alloc<U>&, const linear_alloc<V>&) noexcept;

  template <typename U> friend class linear_alloc;
};

/*------------------------------------------------------------------------------------------------*/

template <typename U, typename V>
bool
operator==(const linear_alloc<U>& lhs, const linear_alloc<V>& rhs)
noexcept
{
  return &lhs.a_ == &rhs.a_;
}

template <typename U, typename V>
bool
operator!=(const linear_alloc<U>& lhs, const linear_alloc<V>& rhs)
noexcept
{
  return not(lhs == rhs);
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::mem
