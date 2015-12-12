#include "gtest/gtest.h"

#include "sdd/mem/variant.hh"

using namespace sdd;

/*------------------------------------------------------------------------------------------------*/

TEST(variant, unary_visitor)
{
  struct foo
  {
    foo(int) {}
  };

  struct visitor
  {
    unsigned int
    operator()(const double)
    const
    {
      return 0;
    }

    unsigned int
    operator()(const char)
    const
    {
      return 1;
    }

    unsigned int
    operator()(const int)
    const
    {
      return 2;
    }

    unsigned int
    operator()(const long)
    const
    {
      return 3;
    }

    unsigned int
    operator()(const foo&)
    const
    {
      return 4;
    }
  };

  using variant_type = mem::variant<double, char, int, long, foo>;

  {
    variant_type v((mem::construct<double>()));
    ASSERT_EQ(0u, apply_visitor(visitor(), v));
  }
  {
    variant_type v((mem::construct<char>()));
    ASSERT_EQ(1u, apply_visitor(visitor(), v));
  }
  {
    variant_type v((mem::construct<int>()));
    ASSERT_EQ(2u, apply_visitor(visitor(), v));
  }
  {
    variant_type v((mem::construct<long>()));
    ASSERT_EQ(3u, apply_visitor(visitor(), v));
  }
  {
    variant_type v(mem::construct<foo>(), 42);
    ASSERT_EQ(4u, apply_visitor(visitor(), v));
  }
}

/*------------------------------------------------------------------------------------------------*/

TEST(variant, binary_visitor)
{
  struct foo
  {
    foo(int) {}
  };
  
  struct visitor
  {
    unsigned int
    operator()(const double, const double)
    const
    {
      return 0;
    }

    unsigned int
    operator()(const double, const int)
    const
    {
      return 1;
    }

    unsigned int
    operator()(const double, const foo&)
    const
    {
      return 2;
    }

    unsigned int
    operator()(const int, const int)
    const
    {
      return 3;
    }

    unsigned int
    operator()(const int, const double)
    const
    {
      return 4;
    }

    unsigned int
    operator()(const int, const foo&)
    const
    {
      return 5;
    }

    unsigned int
    operator()(const foo&, const foo&)
    const
    {
      return 6;
    }

    unsigned int
    operator()(const foo&, const int)
    const
    {
      return 7;
    }

    unsigned int
    operator()(const foo&, const double)
    const
    {
      return 8;
    }
  };
  
  using variant_type = mem::variant<double, int, foo>;

  {
    variant_type v1((mem::construct<double>()));
    variant_type v2((mem::construct<double>()));
    ASSERT_EQ(0u, apply_binary_visitor(visitor(), v1, v2));
  }
  {
    variant_type v1((mem::construct<double>()));
    variant_type v2((mem::construct<int>()));
    ASSERT_EQ(1u, apply_binary_visitor(visitor(), v1, v2));
  }
  {
    variant_type v1((mem::construct<double>()));
    variant_type v2(mem::construct<foo>(), 42);
    ASSERT_EQ(2u, apply_binary_visitor(visitor(), v1, v2));
  }
  {
    variant_type v1((mem::construct<int>()));
    variant_type v2((mem::construct<int>()));
    ASSERT_EQ(3u, apply_binary_visitor(visitor(), v1, v2));
  }
  {
    variant_type v1((mem::construct<int>()));
    variant_type v2((mem::construct<double>()));
    ASSERT_EQ(4u, apply_binary_visitor(visitor(), v1, v2));
  }
  {
    variant_type v1((mem::construct<int>()));
    variant_type v2(mem::construct<foo>(), 42);
    ASSERT_EQ(5u, apply_binary_visitor(visitor(), v1, v2));
  }
  {
    variant_type v1(mem::construct<foo>(), 42);
    variant_type v2(mem::construct<foo>(), 33);
    ASSERT_EQ(6u, apply_binary_visitor(visitor(), v1, v2));
  }
  {
    variant_type v1(mem::construct<foo>(), 42);
    variant_type v2((mem::construct<int>()));
    ASSERT_EQ(7u, apply_binary_visitor(visitor(), v1, v2));
  }
  {
    variant_type v1(mem::construct<foo>(), 42);
    variant_type v2((mem::construct<double>()));
    ASSERT_EQ(8u, apply_binary_visitor(visitor(), v1, v2));
  }
}

/*------------------------------------------------------------------------------------------------*/

struct foo
{
  foo()
  {
    for (size_t i = 0; i < 4096; ++i)
    {
      raw[i] = i;
    }
  }
  size_t raw[4096];
};

struct bar
{
  bar()
  {
    for (auto& elem : raw)
    {
      elem = 0;
    }
  }
  size_t raw[8192];
};


struct visitor1
{
  void
  operator()(const foo& f)
  const
  {
    for (size_t i = 0; i < 4096; ++i)
    {
      ASSERT_EQ(i, f.raw[i]);
    }
  }

  template <typename T>
  void
  operator()(const T&)
  const
  {
    FAIL() << "Shouldn't be here.";
  }
};

struct visitor2
{
  void
  operator()(const foo& f)
  const
  {
    for (size_t i = 0; i < 4096; ++i)
    {
      ASSERT_EQ(i, f.raw[i]);
    }
  }

  template <typename T>
  void
  operator()(const T&)
  const
  {
    FAIL() << "Shouldn't be here.";
  }
};

struct visitor3
{
  void
  operator()(const foo& f)
  const
  {
    for (size_t i = 0; i < 4096; ++i)
    {
      ASSERT_EQ(i, f.raw[i]);
    }
  }

  template <typename T>
  void
  operator()(const T&)
  const
  {
    FAIL() << "Shouldn't be here.";
  }
};


struct visitor4
{
  void
  operator()(const bar& b)
  const
  {
    for (auto & elem : b.raw)
    {
      ASSERT_EQ(0u, elem);
    }
  }

  template <typename T>
  void
  operator()(const T&)
  const
  {
    FAIL() << "Shouldn't be here.";
  }
};

TEST(variant, construction)
{
  {
    using variant_type = mem::variant<bar, char, foo>;
    ASSERT_LE(8192 * sizeof(size_t), sizeof(variant_type));
    variant_type v((mem::construct<foo>()));
    apply_visitor(visitor1(), v);
  }
  {
    using variant_type = mem::variant<char, bar, foo>;
    ASSERT_LE(8192 * sizeof(size_t), sizeof(variant_type));
    variant_type v((mem::construct<foo>()));
    apply_visitor(visitor2(), v);
  }
  {
    using variant_type = mem::variant<foo, char, bar>;
    ASSERT_LE(8192 * sizeof(size_t), sizeof(variant_type));
    variant_type v((mem::construct<foo>()));
    apply_visitor(visitor3(), v);
  }
  {
    using variant_type = mem::variant<char, bar, foo>;
    ASSERT_LE(8192 * sizeof(size_t), sizeof(variant_type));
    variant_type v((mem::construct<bar>()));
    apply_visitor(visitor4(), v);
  }
}

/*------------------------------------------------------------------------------------------------*/
