#include "gtest/gtest.h"

#include "sdd/mem/ptr.hh"

namespace sdd { namespace mem {

struct unique
{
  std::size_t ref_counter_;
  const int data_;

  unique(int data)
    : ref_counter_(0)
    , data_(data)
  {
  }

  void
  increment_reference_counter()
  {
    ++ref_counter_;
  }
  
  void
  decrement_reference_counter()
  {
    --ref_counter_;
  }

  bool
  is_not_referenced()
  const
  {
    return ref_counter_ == 0;
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

  unique&
  operator()(unique& x)
  {
    return x;
  }

  void
  erase(const unique*)
  {
    ++nb_deletions_;
  }

  void
  reset()
  {
    nb_deletions_ = 0;
  }
};

}} // namespace sdd::mem

using namespace sdd::mem;

/*------------------------------------------------------------------------------------------------*/

struct ptr_test
  : public testing::Test
{
  using ptr_type = ptr<unique>;

  sdd::mem::unique_table<unique> table_;

  ptr_test()
    : table_()
  {
    table_.reset();
    sdd::mem::set_deletion_handler<unique>([&](const unique* u){table_.erase(u);});
  }

  ~ptr_test()
  {
    sdd::mem::reset_deletion_handler<unique>();
  }
};

/*------------------------------------------------------------------------------------------------*/

TEST_F(ptr_test, object_life)
{
  unique u(42);
  ASSERT_EQ(0u, u.ref_counter_);

  {
    ptr_type a(&table_(u));
    ASSERT_EQ(1u, u.ref_counter_);

    ptr_type b(&table_(u));
    ASSERT_EQ(2u, u.ref_counter_);

    ASSERT_EQ(a, b);
  }
  ASSERT_EQ(1u, table_.nb_deletions_);
  ASSERT_EQ(0u, u.ref_counter_);

  table_.reset();
  // copy
  {
    ptr_type a(&table_(u));
    ASSERT_EQ(1u, u.ref_counter_);
    {
      ptr_type b(a);
      ASSERT_EQ(2u, u.ref_counter_);
    }
    ASSERT_EQ(1u, u.ref_counter_);
  }
  ASSERT_EQ(1u, table_.nb_deletions_);
  ASSERT_EQ(0u, u.ref_counter_);

  table_.reset();
  // copy operator
  {
    ptr_type a(&table_(u));
    ASSERT_EQ(1u, u.ref_counter_);

    unique v(43);
    ASSERT_EQ(0u, v.ref_counter_);
    ptr_type b(&table_(v));
    ASSERT_EQ(1u, v.ref_counter_);

    a = b;
    ASSERT_EQ(0u, u.ref_counter_);
    ASSERT_EQ(2u, v.ref_counter_);
  }
  ASSERT_EQ(2u, table_.nb_deletions_);
  ASSERT_EQ(0u, u.ref_counter_);

  table_.reset();
  // move
  {
    ptr_type a(&table_(u));
    ASSERT_EQ(1u, u.ref_counter_);
    {
      ptr_type b(std::move(a));
      ASSERT_EQ(1u, u.ref_counter_);
    }
    ASSERT_EQ(0u, u.ref_counter_);
  }
  ASSERT_EQ(1u, table_.nb_deletions_);
  ASSERT_EQ(0u, u.ref_counter_);

  table_.reset();
  // move operator
  {
    ptr_type a(&table_(u));
    ASSERT_EQ(1u, u.ref_counter_);

    unique v(43);
    ASSERT_EQ(0u, v.ref_counter_);
    ptr_type b(&table_(v));
    ASSERT_EQ(1u, v.ref_counter_);

    a = std::move(b);
    ASSERT_EQ(0u, u.ref_counter_);
    ASSERT_EQ(1u, v.ref_counter_);
  }
  ASSERT_EQ(2u, table_.nb_deletions_);
  ASSERT_EQ(0u, u.ref_counter_);

  table_.reset();
  // swap
  {
    unique v(43);
    ASSERT_EQ(0u, v.ref_counter_);

    ptr_type pu1(&table_(u));
    ASSERT_EQ(1u, u.ref_counter_);

    ptr_type pu2(&table_(u));
    ASSERT_EQ(2u, u.ref_counter_);
    
    ptr_type pv(&table_(v));
    ASSERT_EQ(1u, v.ref_counter_);

    using std::swap;
    swap(pu1, pv);
    ASSERT_EQ(2u, u.ref_counter_);
    ASSERT_EQ(1u, v.ref_counter_);
    ASSERT_EQ(pv, pu2);
    ASSERT_EQ(43, pu1->data());
    ASSERT_EQ(42, pv->data());
  }
  ASSERT_EQ(2u, table_.nb_deletions_);
  ASSERT_EQ(0u, u.ref_counter_);
}

/*------------------------------------------------------------------------------------------------*/
