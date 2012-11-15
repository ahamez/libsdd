#include "gtest/gtest.h"

#include "sdd/mem/ptr.hh"
#include "sdd/mem/ref_counted.hh"

namespace sdd { namespace mem {

struct unique
{
  mutable std::size_t ref_counter_;
  const int data_;

  unique(int data)
    : ref_counter_(0)
    , data_(data)
  {
  }

  void
  increment_reference_counter()
  const
  {
    ++ref_counter_;
  }
  
  void
  decrement_reference_counter()
  const
  {
    --ref_counter_;
  }

  std::size_t
  reference_counter()
  const
  {
    return ref_counter_;
  }

  int
  data()
  const
  {
    return data_;
  }
};

template <>
struct unique_table<unique>
{
  std::size_t nb_deletions_;

  unique_table()
    : nb_deletions_(0)
  {
  }

  const unique&
  operator()(const unique& x)
  {
    return x;
  }

  void
  erase(unique&)
  {
    ++nb_deletions_;
  }

  void
  reset()
  {
    nb_deletions_ = 0;
  }
};

namespace {

template <>
mem::unique_table<unique>&
global_unique_table()
noexcept
{
  static mem::unique_table<unique> unique_table;
  return unique_table;
}

}

}} // namespace sdd::mem

using namespace sdd::mem;

/*------------------------------------------------------------------------------------------------*/

struct ptr_test
  : public testing::Test
{
  typedef ptr<unique> ptr_type;

  sdd::mem::unique_table<unique>& table_;

  ptr_test()
    : table_(global_unique_table<unique>())
  {
  }

  void
  SetUp()
  {
    table_.reset();
  }
};

/*------------------------------------------------------------------------------------------------*/

TEST_F(ptr_test, object_life)
{
  unique u(42);
  ASSERT_EQ(0, u.ref_counter_);

  {
    ptr_type a(table_(u));
    ASSERT_EQ(1, u.ref_counter_);

    ptr_type b(table_(u));
    ASSERT_EQ(2, u.ref_counter_);

    ASSERT_EQ(a, b);
  }
  ASSERT_EQ(1, table_.nb_deletions_);
  ASSERT_EQ(0, u.ref_counter_);

  table_.reset();
  // copy
  {
    ptr_type a(table_(u));
    ASSERT_EQ(1, u.ref_counter_);
    {
      ptr_type b(a);
      ASSERT_EQ(2, u.ref_counter_);
    }
    ASSERT_EQ(1, u.ref_counter_);
  }
  ASSERT_EQ(1, table_.nb_deletions_);
  ASSERT_EQ(0, u.ref_counter_);

  table_.reset();
  // copy operator
  {
    ptr_type a(table_(u));
    ASSERT_EQ(1, u.ref_counter_);

    unique v(43);
    ASSERT_EQ(0, v.ref_counter_);
    ptr_type b(table_(v));
    ASSERT_EQ(1, v.ref_counter_);

    a = b;
    ASSERT_EQ(0, u.ref_counter_);
    ASSERT_EQ(2, v.ref_counter_);
  }
  ASSERT_EQ(2, table_.nb_deletions_);
  ASSERT_EQ(0, u.ref_counter_);

  table_.reset();
  // move
  {
    ptr_type a(table_(u));
    ASSERT_EQ(1, u.ref_counter_);
    {
      ptr_type b(std::move(a));
      ASSERT_EQ(1, u.ref_counter_);
      ASSERT_EQ(nullptr, &*a);
    }
    ASSERT_EQ(0, u.ref_counter_);
  }
  ASSERT_EQ(1, table_.nb_deletions_);
  ASSERT_EQ(0, u.ref_counter_);

  table_.reset();
  // move operator
  {
    ptr_type a(table_(u));
    ASSERT_EQ(1, u.ref_counter_);

    unique v(43);
    ASSERT_EQ(0, v.ref_counter_);
    ptr_type b(table_(v));
    ASSERT_EQ(1, v.ref_counter_);

    a = std::move(b);
    ASSERT_EQ(0, u.ref_counter_);
    ASSERT_EQ(1, v.ref_counter_);
  }
  ASSERT_EQ(2, table_.nb_deletions_);
  ASSERT_EQ(0, u.ref_counter_);

  table_.reset();
  // swap
  {
    unique v(43);
    ASSERT_EQ(0, v.ref_counter_);

    ptr_type pu1(table_(u));
    ASSERT_EQ(1, u.ref_counter_);

    ptr_type pu2(table_(u));
    ASSERT_EQ(2, u.ref_counter_);
    
    ptr_type pv(table_(v));
    ASSERT_EQ(1, v.ref_counter_);

    using std::swap;
    swap(pu1, pv);
    ASSERT_EQ(2, u.ref_counter_);
    ASSERT_EQ(1, v.ref_counter_);    
    ASSERT_EQ(pv, pu2);
    ASSERT_EQ(43, pu1->data());
    ASSERT_EQ(42, pv->data());
  }
  ASSERT_EQ(2, table_.nb_deletions_);
  ASSERT_EQ(0, u.ref_counter_);
}

/*------------------------------------------------------------------------------------------------*/
