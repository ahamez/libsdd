#include "gtest/gtest.h"

#define SDD_VARIANT_UNROLL_UNARY_SIZE  0
#define SDD_VARIANT_UNROLL_BINARY_SIZE 0
#include "sdd/internal/mem/variant.hh"

using namespace sdd::internal;

/*-------------------------------------------------------------------------------------------*/

TEST(variant_no_unroll, unary_visitor)
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

/*-------------------------------------------------------------------------------------------*/

TEST(variant_no_unroll, binary_visitor)
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
    ASSERT_EQ(0, apply_visitor(visitor(), v1, v2));
  }
  {
    variant_type v1((mem::construct<double>()));
    variant_type v2((mem::construct<int>()));
    ASSERT_EQ(1, apply_visitor(visitor(), v1, v2));
  }
  {
    variant_type v1((mem::construct<double>()));
    variant_type v2(mem::construct<foo>(), 42);
    ASSERT_EQ(2, apply_visitor(visitor(), v1, v2));
  }
  {
    variant_type v1((mem::construct<int>()));
    variant_type v2((mem::construct<int>()));
    ASSERT_EQ(3, apply_visitor(visitor(), v1, v2));
  }
  {
    variant_type v1((mem::construct<int>()));
    variant_type v2((mem::construct<double>()));
    ASSERT_EQ(4, apply_visitor(visitor(), v1, v2));
  }
  {
    variant_type v1((mem::construct<int>()));
    variant_type v2(mem::construct<foo>(), 42);
    ASSERT_EQ(5, apply_visitor(visitor(), v1, v2));
  }
  {
    variant_type v1(mem::construct<foo>(), 42);
    variant_type v2(mem::construct<foo>(), 33);
    ASSERT_EQ(6, apply_visitor(visitor(), v1, v2));
  }
  {
    variant_type v1(mem::construct<foo>(), 42);
    variant_type v2((mem::construct<int>()));
    ASSERT_EQ(7, apply_visitor(visitor(), v1, v2));
  }
  {
    variant_type v1(mem::construct<foo>(), 42);
    variant_type v2((mem::construct<double>()));
    ASSERT_EQ(8, apply_visitor(visitor(), v1, v2));
  }
}

/*-------------------------------------------------------------------------------------------*/
