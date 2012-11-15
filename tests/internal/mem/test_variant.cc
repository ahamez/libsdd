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
    typedef unsigned int result_type;
    
    result_type
    operator()(const double)
    const
    {
      return 0;
    }

    result_type
    operator()(const char)
    const
    {
      return 1;
    }

    result_type
    operator()(const int)
    const
    {
      return 2;
    }

    result_type
    operator()(const long)
    const
    {
      return 3;
    }

    result_type
    operator()(const foo&)
    const
    {
      return 4;
    }
  };

  typedef mem::variant<double, char, int, long, foo> variant_type;

  {
    variant_type v((mem::construct<double>()));
    ASSERT_EQ(0, apply_visitor(visitor(), v));
  }
  {
    variant_type v((mem::construct<char>()));
    ASSERT_EQ(1, apply_visitor(visitor(), v));
  }
  {
    variant_type v((mem::construct<int>()));
    ASSERT_EQ(2, apply_visitor(visitor(), v));
  }
  {
    variant_type v((mem::construct<long>()));
    ASSERT_EQ(3, apply_visitor(visitor(), v));
  }
  {
    variant_type v(mem::construct<foo>(), 42);
    ASSERT_EQ(4, apply_visitor(visitor(), v));
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
    typedef unsigned int result_type;
    
    result_type
    operator()(const double, const double)
    const
    {
      return 0;
    }

    result_type
    operator()(const double, const int)
    const
    {
      return 1;
    }

    result_type
    operator()(const double, const foo&)
    const
    {
      return 2;
    }

    result_type
    operator()(const int, const int)
    const
    {
      return 3;
    }

    result_type
    operator()(const int, const double)
    const
    {
      return 4;
    }

    result_type
    operator()(const int, const foo&)
    const
    {
      return 5;
    }

    result_type
    operator()(const foo&, const foo&)
    const
    {
      return 6;
    }

    result_type
    operator()(const foo&, const int)
    const
    {
      return 7;
    }

    result_type
    operator()(const foo&, const double)
    const
    {
      return 8;
    }


  };
  
  typedef mem::variant<double, int, foo> variant_type;

  {
    variant_type v1((mem::construct<double>()));
    variant_type v2((mem::construct<double>()));
    ASSERT_EQ(0, apply_binary_visitor(visitor(), v1, v2));
  }
  {
    variant_type v1((mem::construct<double>()));
    variant_type v2((mem::construct<int>()));
    ASSERT_EQ(1, apply_binary_visitor(visitor(), v1, v2));
  }
  {
    variant_type v1((mem::construct<double>()));
    variant_type v2(mem::construct<foo>(), 42);
    ASSERT_EQ(2, apply_binary_visitor(visitor(), v1, v2));
  }
  {
    variant_type v1((mem::construct<int>()));
    variant_type v2((mem::construct<int>()));
    ASSERT_EQ(3, apply_binary_visitor(visitor(), v1, v2));
  }
  {
    variant_type v1((mem::construct<int>()));
    variant_type v2((mem::construct<double>()));
    ASSERT_EQ(4, apply_binary_visitor(visitor(), v1, v2));
  }
  {
    variant_type v1((mem::construct<int>()));
    variant_type v2(mem::construct<foo>(), 42);
    ASSERT_EQ(5, apply_binary_visitor(visitor(), v1, v2));
  }
  {
    variant_type v1(mem::construct<foo>(), 42);
    variant_type v2(mem::construct<foo>(), 33);
    ASSERT_EQ(6, apply_binary_visitor(visitor(), v1, v2));
  }
  {
    variant_type v1(mem::construct<foo>(), 42);
    variant_type v2((mem::construct<int>()));
    ASSERT_EQ(7, apply_binary_visitor(visitor(), v1, v2));
  }
  {
    variant_type v1(mem::construct<foo>(), 42);
    variant_type v2((mem::construct<double>()));
    ASSERT_EQ(8, apply_binary_visitor(visitor(), v1, v2));
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
    for (size_t i = 0; i < 8192; ++i)
    {
      raw[i] = 0;
    }
  }
  size_t raw[8192];
};


struct visitor1
{
  typedef void result_type;

  result_type
  operator()(const foo& f)
  const
  {
    for (size_t i = 0; i < 4096; ++i)
    {
      ASSERT_EQ(i, f.raw[i]);
    }
  }

  template <typename T>
  result_type
  operator()(const T&)
  const
  {
    FAIL() << "Shouldn't be here.";
  }
};

struct visitor2
{
  typedef void result_type;

  result_type
  operator()(const foo& f)
  const
  {
    for (size_t i = 0; i < 4096; ++i)
    {
      ASSERT_EQ(i, f.raw[i]);
    }
  }

  template <typename T>
  result_type
  operator()(const T&)
  const
  {
    FAIL() << "Shouldn't be here.";
  }
};

struct visitor3
{
  typedef void result_type;

  result_type
  operator()(const foo& f)
  const
  {
    for (size_t i = 0; i < 4096; ++i)
    {
      ASSERT_EQ(i, f.raw[i]);
    }
  }

  template <typename T>
  result_type
  operator()(const T&)
  const
  {
    FAIL() << "Shouldn't be here.";
  }
};


struct visitor4
{
  typedef void result_type;

  result_type
  operator()(const bar& b)
  const
  {
    for (size_t i = 0; i < 8192; ++i)
    {
      ASSERT_EQ(0, b.raw[i]);
    }
  }

  template <typename T>
  result_type
  operator()(const T&)
  const
  {
    FAIL() << "Shouldn't be here.";
  }
};

TEST(variant, construction)
{
  {
    typedef mem::variant<bar, char, foo> variant_type;
    ASSERT_LE(8192 * sizeof(size_t), sizeof(variant_type));
    variant_type v((mem::construct<foo>()));
    apply_visitor(visitor1(), v);
  }
  {
    typedef mem::variant<char, bar, foo> variant_type;
    ASSERT_LE(8192 * sizeof(size_t), sizeof(variant_type));
    variant_type v((mem::construct<foo>()));
    apply_visitor(visitor2(), v);
  }
  {
    typedef mem::variant<foo, char, bar> variant_type;
    ASSERT_LE(8192 * sizeof(size_t), sizeof(variant_type));
    variant_type v((mem::construct<foo>()));
    apply_visitor(visitor3(), v);
  }
  {
    typedef mem::variant<char, bar, foo> variant_type;
    ASSERT_LE(8192 * sizeof(size_t), sizeof(variant_type));
    variant_type v((mem::construct<bar>()));
    apply_visitor(visitor4(), v);
  }
}

/*------------------------------------------------------------------------------------------------*/
