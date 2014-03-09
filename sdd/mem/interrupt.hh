#ifndef _SDD_MEM_INTERRUPT_HH_
#define _SDD_MEM_INTERRUPT_HH_

#include <exception>

namespace sdd { namespace mem {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @exception interrupt
/// @brief Interrupt an evaluation in the cache.
template <typename T>
class interrupt
  : public std::exception
{
private:

  T result_;

public:

  /// @brief Virtual desctuctor.
  virtual ~interrupt(){};

  /// @internal
  T&
  result()
  noexcept
  {
    return result_;
  }

  /// @brief Get the last computed result before the interruption.
  const T&
  result()
  const noexcept
  {
    return result_;
  }

};

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::mem

#endif // _SDD_MEM_INTERRUPT_HH_
