#include <iostream>
#include <iterator>
#include <sstream>
#include <string>

#include <boost/python.hpp>

#include "sdd/sdd.hh"

/*------------------------------------------------------------------------------------------------*/

namespace sdd { namespace python {

using namespace boost::python;

/*------------------------------------------------------------------------------------------------*/

struct py_identifier
{
  object py;

  py_identifier(object p)
		: py(p)
  {}

  py_identifier(const py_identifier& v)
		: py(v.py)
  {}

  py_identifier&
  operator=(const py_identifier& v)
  {
    py = v.py;
    return *this;
  }

  bool
  operator==(const py_identifier& rhs)
  const
  {
    return call_method<bool>(py.ptr(), "__eq__", rhs.py);
  }  

  bool
  operator<(const py_identifier& rhs)
  const
  {
    return call_method<bool>(py.ptr(), "__lt__", rhs.py);
  }

  std::size_t
  hash()
  const
  {
    return static_cast<std::size_t>(call_method<long>(py.ptr(), "__hash__"));
  }  

  std::string
  name()
  const
  {
    return call_method<std::string>(py.ptr(), "__str__");
  }
};

std::ostream&
operator<<(std::ostream& os, const py_identifier& v)
{
  return os << v.name();
}

/*------------------------------------------------------------------------------------------------*/

struct py_values
{
  object py;

  py_values()
    : py() // None
  {}

  py_values(object p)
		: py(p)
  {}

  py_values(const py_values& v)
		: py(v.py)
  {}

  py_values&
  operator=(const py_values& v)
  {
    py = v.py;
    return *this;
  }

  bool
  operator==(const py_values& rhs)
  const
  {
    return call_method<bool>(py.ptr(), "__eq__", rhs.py);
  }  

  bool
  operator<(const py_values& rhs)
  const
  {
    if (rhs.empty())
    {
      return false;
    }
    else if (empty())
    {
      return true;
    }
    else
    {
      return call_method<bool>(py.ptr(), "__lt__", rhs.py);
    }
  }

  std::size_t
  hash()
  const
  {
    return empty()
         ? std::hash<int>()(0)
         : static_cast<std::size_t>(call_method<long>(py.ptr() , "__hash__" ));
  }

  std::string
  name()
  const
  {
    return call_method<std::string>(py.ptr(), "__str__" );
  }    

  bool
  empty()
  const
  {
    return size() == 0;
  }  

  std::size_t
  size()
  const
  {
    return py.is_none() ? 0 : call_method<std::size_t>(py.ptr(), "__len__" );
  }    
};

std::ostream&
operator<<(std::ostream& os, const py_values& v)
{
  return os << v.name();
}

py_values
sum(const py_values& lhs, const py_values& rhs)
{
  if (not lhs.empty())
  {
    if (not rhs.empty())
    {
      return py_values(call_method<object>(lhs.py.ptr(), "union", rhs.py));
    }
    else
    {
      return lhs;
    }
  }
  else
  {
    return rhs;
  }
}

py_values
difference(const py_values& lhs, const py_values& rhs)
{
  if (not lhs.empty())
  {
    if (not rhs.empty())
    {
      return py_values(call_method<object>(lhs.py.ptr(), "difference", rhs.py));
    }
    else
    {
      return lhs;
    }
  }
  else
  {
    return lhs;
  }
}

py_values
intersection(const py_values& lhs, const py_values& rhs)
{
  if (not lhs.empty())
  {
    if (not rhs.empty())
    {
      return py_values(call_method<object>(lhs.py.ptr(), "intersection", rhs.py));
    }
    else
    {
      return rhs;
    }
  }
  else
  {
    return lhs;
  }
}

}
 namespace values {

  template <>
  struct values_traits<python::py_values>
  {
    static constexpr bool stateful = false;
    static constexpr bool fast_iterable = false;
    static constexpr bool has_value_type = false;
  };
  
}

namespace python {

/*------------------------------------------------------------------------------------------------*/

struct configuration
  : public sdd::conf1
{
  using Variable = int;
  using Identifier = py_identifier;
  using Values = py_values;
};

/*------------------------------------------------------------------------------------------------*/

static manager<configuration> _manager = manager<configuration>::init();

/*------------------------------------------------------------------------------------------------*/

BOOST_PYTHON_MODULE(_sdd)
{
  class_<SDD<configuration>>("SDD", init<int, object, SDD<configuration>>())
    .def(init<int, SDD<configuration>, SDD<configuration>>())
    .def(self_ns::str(self))
    .def(self + self)
    .def(self - self)
    .def(self & self)
    .def(self == self)
    .def(self < self)
    .def("__hash__", &SDD<configuration>::hash)
    ;

  def("zero", &zero<configuration>);
  def("one", &one<configuration>);
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::python

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @brief Hash specialization for sdd::values::bitset.
template <>
struct hash<sdd::python::py_identifier>
{
  std::size_t
  operator()(const sdd::python::py_identifier& id)
  const
  {
    return id.hash();
  }
};

template <>
struct hash<sdd::python::py_values>
{
  std::size_t
  operator()(const sdd::python::py_values& val)
  const
  {
    return val.hash();
  }
};

/*------------------------------------------------------------------------------------------------*/
  
} // namespace std
