#include <algorithm>
#include <vector>

#include "gtest/gtest.h"

#include "sdd/mem/hash_table.hh"

using namespace sdd::mem;

/*------------------------------------------------------------------------------------------------*/

namespace /* anonymous */ {

struct foo
{
  unsigned int data;
  intrusive_member_hook<foo> hook;

  foo(unsigned int d)
    : data(d)
  {}

  bool
  operator==(const foo& other)
  const noexcept
  {
    return data == other.data;
  }

  bool
  operator<(const foo& other)
  const noexcept
  {
    return data < other.data;
  }
};

std::ostream&
operator<<(std::ostream& os, const foo& f)
{
  return os << "foo(" << f.data <<")";
}

struct bar
{
  unsigned int data;
  std::size_t hash;
  intrusive_member_hook<bar> hook;

  bar(unsigned int d, std::size_t h)
    : data(d)
    , hash(h)
  {}

  bool
  operator==(const bar& other)
  const noexcept
  {
    return data == other.data;
  }

  bool
  operator<(const bar& other)
  const noexcept
  {
    return data < other.data;
  }
};

std::ostream&
operator<<(std::ostream& os, const bar& f)
{
  return os << "bar(" << f.data <<")";
}

} // namespace anonymous

namespace std
{

template <>
struct hash<foo>
{
  std::size_t
  operator()(const foo& f)
  const noexcept
  {
    return std::hash<unsigned int>()(f.data);
  }
};

template <>
struct hash<bar>
{
  std::size_t
  operator()(const bar& b)
  const noexcept
  {
    return b.hash;
  }
};

} // namespace std

typedef hash_table<foo> foo_hash_table;
typedef hash_table<bar> bar_hash_table;

/*------------------------------------------------------------------------------------------------*/

TEST(hash_table, creation)
{
  foo_hash_table ht{100};
  ASSERT_EQ(0, ht.size());
}

/*------------------------------------------------------------------------------------------------*/

TEST(hash_table, simple_insertion)
{
  foo_hash_table  ht{10};

  foo f1{42};
  foo f2{42};
  foo f3{43};

  auto insertion = ht.insert(f1);
  ASSERT_EQ(1, ht.size());
  ASSERT_TRUE(insertion.second);
  ASSERT_EQ(f1, *insertion.first);

  insertion = ht.insert(f1);
  ASSERT_EQ(1u, ht.size());
  ASSERT_FALSE(insertion.second);
  ASSERT_EQ(f1, *insertion.first);

  insertion = ht.insert(f2);
  ASSERT_EQ(1u, ht.size());
  ASSERT_FALSE(insertion.second);
  ASSERT_EQ(f1, *insertion.first);

  insertion = ht.insert(f3);
  ASSERT_EQ(2u, ht.size());
  ASSERT_TRUE(insertion.second);
  ASSERT_EQ(f3, *insertion.first);
}

/*------------------------------------------------------------------------------------------------*/

TEST(hash_table, insert_collision)
{
  {
    bar_hash_table  ht{16};
    bar b1{0, 7};
    bar b2{1, 7};

    auto insertion1 = ht.insert(b1);
    ASSERT_EQ(1u, ht.size());
    ASSERT_TRUE(insertion1.second);
    ASSERT_EQ(b1, *insertion1.first);

    auto insertion2 = ht.insert(b2);
    ASSERT_EQ(2u, ht.size());
    ASSERT_TRUE(insertion2.second);
    ASSERT_EQ(b2, *insertion2.first);

    ASSERT_NE(insertion1, insertion2);
  }
  {
    std::vector<bar> vec;
    vec.reserve(100);
    for (unsigned int i = 0; i < 100; ++i)
    {
      vec.push_back(bar(i, i % 16));
    }

    bar_hash_table  ht{16};
    for (auto& b : vec)
    {
      ht.insert(b);
    }

    ASSERT_EQ(100u, ht.size());
  }
}

/*------------------------------------------------------------------------------------------------*/

TEST(hash_table, empty_iteration)
{
  foo_hash_table ht{10};
  const auto begin = ht.begin();
  const auto end = ht.end();
  ASSERT_EQ(0u, std::distance(begin, end));
  ASSERT_EQ(begin, end);
  std::vector<foo> vec {ht.begin(), ht.end()};
  ASSERT_EQ(0u, vec.size());
}

/*------------------------------------------------------------------------------------------------*/

TEST(hash_table, iteration)
{
  {
    foo_hash_table ht{10};
    foo f1{42};
    foo f3{43};
    ht.insert(f1);
    ht.insert(f3);

    ASSERT_EQ(2u, std::distance(ht.begin(), ht.end()));

    std::vector<foo> vec(ht.begin(), ht.end());
    ASSERT_EQ(2u, vec.size());

    std::sort(vec.begin(), vec.end());
    ASSERT_EQ(f1, vec[0]);
    ASSERT_EQ(f3, vec[1]);
  }
  {
    std::vector<bar> vec;
    vec.reserve(100);
    for (unsigned int i = 0; i < 100; ++i)
    {
      vec.push_back(bar(i, i % 16));
    }

    bar_hash_table  ht{16};
    for (auto& b : vec)
    {
      ht.insert(b);
    }

    ASSERT_EQ(100u, std::distance(ht.begin(), ht.end()));
    ASSERT_NE(ht.begin(), ht.end());

    std::vector<bar*> ht_content;
    ht_content.reserve(100);
    for (auto& b : ht)
    {
      ht_content.push_back(&b);
    }

    std::sort( ht_content.begin(), ht_content.end()
             , [](const bar* lhs, const bar* rhs){return *lhs < *rhs;}
             );

    const bool cmp = std::equal( vec.begin(), vec.end(), ht_content.begin()
                               , [](const bar& lhs, const bar* rhs){return lhs == *rhs;}
                               );

    ASSERT_TRUE(cmp);
  }
}

/*------------------------------------------------------------------------------------------------*/

TEST(hash_table, insert_check_miss)
{
  foo_hash_table ht{10};
  foo f1{33};
  ht.insert(f1);

  foo_hash_table::insert_commit_data commit_data;
  const auto insertion = ht.insert_check( 42
                                        , std::hash<unsigned int>()
                                        , [](unsigned int lhs, const foo& rhs)
                                            {
                                              return lhs == rhs.data;
                                            }
                                        , commit_data);
  ASSERT_TRUE(insertion.second);
  ASSERT_EQ(1u, ht.size());

  foo f2{42};
  ht.insert_commit(f2, commit_data);

  ASSERT_EQ(2u, ht.size());

  std::vector<foo> vec(ht.begin(), ht.end());
  ASSERT_EQ(2u, vec.size());

  std::sort(vec.begin(), vec.end());
  ASSERT_EQ(f1, vec[0]);
  ASSERT_EQ(f2, vec[1]);
}

/*------------------------------------------------------------------------------------------------*/

TEST(hash_table, insert_check_hit)
{
  foo_hash_table ht{10};
  foo f1{33};
  ht.insert(f1);

  foo_hash_table::insert_commit_data commit_data;
  const auto insertion = ht.insert_check( 33
                                        , std::hash<unsigned int>()
                                        , [](unsigned int lhs, const foo& rhs)
                                            {
                                              return lhs == rhs.data;
                                            }
                                        , commit_data);
  ASSERT_FALSE(insertion.second);
}


/*------------------------------------------------------------------------------------------------*/

TEST(hash_table, find)
{
  {
    foo_hash_table ht{10};
    foo f1{42};

    ASSERT_EQ(ht.end(), ht.find(f1));

    ht.insert(f1);
    const auto it = ht.find(f1);
    ASSERT_NE(ht.end(), it);
    ASSERT_EQ(f1, *it);
  }
  {
    bar_hash_table ht{10};
    bar b1{42, 10};
    bar b2{43, 10};

    ht.insert(b1);
    ht.insert(b2);

    const auto it1 = ht.find(b1);
    const auto it2 = ht.find(b2);

    ASSERT_NE(ht.end(), it1);
    ASSERT_EQ(b1, *it1);

    ASSERT_NE(ht.end(), it2);
    ASSERT_EQ(b2, *it2);

    ASSERT_NE(it1, it2);
  }
}

/*------------------------------------------------------------------------------------------------*/

TEST(hash_table, erase)
{
  {
    foo_hash_table ht{10};
    foo f1{42};
    ht.insert(f1);
    ASSERT_EQ(1u, ht.size());

    const auto it1 = ht.find(f1);
    ASSERT_NE(ht.end(), it1);
    ht.erase(it1);
    ASSERT_EQ(0u, ht.size());
    
    const auto it2 = ht.find(f1);
    ASSERT_EQ(ht.end(), it2);

    ASSERT_EQ(ht.end(), ht.begin());
  }
  {
    bar_hash_table ht{2};
    bar b1{42, 0};
    bar b2{43, 0};

    ht.insert(b1);
    ht.insert(b2);
    ASSERT_EQ(2u, ht.size());

    const auto it1 = ht.find(b1);
    ht.erase(it1);
    ASSERT_EQ(ht.end(), ht.find(b1));
    ASSERT_EQ(1u, ht.size());

    const auto it2 = ht.find(b2);
    ht.erase(it2);
    ASSERT_EQ(ht.end(), ht.find(b2));
    ASSERT_EQ(0u, ht.size());

    ASSERT_EQ(ht.begin(), ht.end());
  }
  {
    bar_hash_table ht{2};
    bar b1{42, 0};
    bar b2{43, 0};

    ht.insert(b1);
    ht.insert(b2);
    ASSERT_EQ(2u, ht.size());

    const auto it2 = ht.find(b2);
    ht.erase(it2);
    ASSERT_EQ(ht.end(), ht.find(b2));
    ASSERT_EQ(1u, ht.size());

    const auto it1 = ht.find(b1);
    ht.erase(it1);
    ASSERT_EQ(ht.end(), ht.find(b1));
    ASSERT_EQ(0u, ht.size());

    ASSERT_EQ(ht.begin(), ht.end());
  }
}

/*------------------------------------------------------------------------------------------------*/

TEST(hash_table, clear_and_dispose)
{
  std::vector<foo> vec;
  vec.reserve(16);
  for (unsigned int i = 0; i < 16; ++i)
  {
    vec.push_back(foo{i});
  }

  foo_hash_table ht{8};
  for (auto& f : vec)
  {
    ht.insert(f);
  }

  std::size_t cpt = 0;
  ht.clear_and_dispose([&cpt](foo*){++cpt;});

  ASSERT_EQ(0u, ht.size());
  ASSERT_EQ(16u, cpt);

  for (auto& f : vec)
  {
    ASSERT_EQ(ht.end(), ht.find(f));
  }

  ASSERT_EQ(ht.end(), ht.begin());
}

/*------------------------------------------------------------------------------------------------*/
