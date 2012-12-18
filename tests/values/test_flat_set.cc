#include "gtest/gtest.h"

#include "sdd/values/flat_set.hh"

typedef sdd::values::flat_set<unsigned int> flat_set;

/*------------------------------------------------------------------------------------------------*/

TEST(flat_set_test, empty)
{
  flat_set fs;
  ASSERT_TRUE(fs.empty());
}

/*------------------------------------------------------------------------------------------------*/

TEST(flat_set_test, insertion)
{
  flat_set fs;
  ASSERT_TRUE(fs.empty());
  fs.insert(10);
  fs.insert(1);
  fs.insert(42);
  fs.insert(1);
  fs.insert(42);
  ASSERT_EQ(flat_set({1,10,42}), fs);
  ASSERT_EQ(3u, fs.size());
}

/*------------------------------------------------------------------------------------------------*/

TEST(flat_set_test, unicity)
{
  {
    flat_set fs1;
    ASSERT_TRUE(fs1.empty());
    flat_set fs2;
    ASSERT_TRUE(fs2.empty());
    ASSERT_EQ(fs1.data(), fs2.data());
  }
  {
    flat_set fs1 {1,2,3};
    flat_set fs2 {1,2,3};
    ASSERT_EQ(fs1.data(), fs2.data());
  }
}

/*------------------------------------------------------------------------------------------------*/

TEST(flat_set_test, difference)
{
  {
    flat_set fs1;
    ASSERT_TRUE(fs1.empty());
    flat_set fs2;
    ASSERT_TRUE(fs2.empty());
    ASSERT_TRUE((difference(fs1, fs2).empty()));
    ASSERT_TRUE((difference(fs2, fs1).empty()));
  }
  {
    flat_set empty;
    flat_set fs {1,2,3};
    ASSERT_EQ((flat_set {1,2,3}), difference(fs, empty));
    ASSERT_EQ(empty, difference(empty, fs));
    ASSERT_EQ(empty, difference(fs, fs));
  }
  {
    flat_set fs1 {1,2,3};
    flat_set fs2 {2,3,5};
    ASSERT_EQ((flat_set {1}), difference(fs1, fs2));
    ASSERT_EQ((flat_set {5}), difference(fs2, fs1));
  }
}

/*------------------------------------------------------------------------------------------------*/

TEST(flat_set_test, intersection)
{
  {
    flat_set fs1;
    ASSERT_TRUE(fs1.empty());
    flat_set fs2;
    ASSERT_TRUE(fs2.empty());
    ASSERT_TRUE((intersection(fs1, fs2).empty()));
    ASSERT_TRUE((intersection(fs2, fs1).empty()));
  }
  {
    flat_set empty;
    flat_set fs {1,2,3};
    ASSERT_EQ(empty, intersection(empty, fs));
    ASSERT_EQ(empty, intersection(fs, empty));
  }
  {
    flat_set fs1 {1,2,3};
    flat_set fs2 {2,3,5};
    ASSERT_EQ((flat_set {2,3}), intersection(fs1, fs2));
    ASSERT_EQ((flat_set {2,3}), intersection(fs2, fs1));
  }
}

/*------------------------------------------------------------------------------------------------*/

TEST(flat_set_test, sum)
{
  {
    flat_set fs1;
    ASSERT_TRUE(fs1.empty());
    flat_set fs2;
    ASSERT_TRUE(fs2.empty());
    ASSERT_TRUE((sum(fs1, fs2).empty()));
    ASSERT_TRUE((sum(fs2, fs1).empty()));
  }
  {
    flat_set empty;
    flat_set fs {1,2,3};
    ASSERT_EQ(fs, sum(empty, fs));
    ASSERT_EQ(fs, sum(fs, empty));
  }
  {
    flat_set fs1 {1,2,3};
    flat_set fs2 {2,3,5};
    ASSERT_EQ((flat_set {1,2,3,5}), sum(fs1, fs2));
    ASSERT_EQ((flat_set {1,2,3,5}), sum(fs2, fs1));
  }
}

/*------------------------------------------------------------------------------------------------*/
