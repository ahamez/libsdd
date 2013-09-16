#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>

#include <boost/python.hpp>
#include <boost/python/suite/indexing/indexing_suite.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include "sdd/sdd.hh"

/*------------------------------------------------------------------------------------------------*/

namespace sdd { namespace python {

using namespace boost::python;

/*------------------------------------------------------------------------------------------------*/

struct py_values
{
  object py;

  py_values()
    : py() // default to None
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

  object
  content()
  const noexcept
  {
    return py;
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
    else if (this->empty())
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
      return py_values(call_method<object>(lhs.py.ptr(), "__or__", rhs.py));
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
      return py_values(call_method<object>(lhs.py.ptr(), "__sub__", rhs.py));
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
      return py_values(call_method<object>(lhs.py.ptr(), "__and__", rhs.py));
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

} // namespace python

namespace values {

  template <>
  struct values_traits<python::py_values>
  {
    static constexpr bool stateful = false;
    static constexpr bool fast_iterable = false;
    static constexpr bool has_value_type = false;
  };

} // namespace values

namespace python {

/*------------------------------------------------------------------------------------------------*/

struct pyconf
  : public sdd::conf1
{
  using Variable = int;
  using Identifier = std::string;
  using Values = py_values;
};

/*------------------------------------------------------------------------------------------------*/

double
count_combinations(SDD<pyconf> s)
{
  return sdd::count_combinations(s).template convert_to<double>();
}

/*------------------------------------------------------------------------------------------------*/

struct _manager
{
  std::shared_ptr<manager<pyconf>> ptr;

  _manager()
    : ptr(new manager<pyconf>(manager<pyconf>::init()))
  {}
};

/*------------------------------------------------------------------------------------------------*/

inline
object
pass_through(object const& o)
{
  return o;
}

struct paths
{
  path_generator<pyconf> gen;

  paths(const SDD<pyconf>& s)
  noexcept
    : gen(s.paths())
  {}

  static
  path<pyconf>
  next(paths& p)
  {
    if (p.gen)
    {
      const path<pyconf> res = p.gen.get();
      p.gen(); // compute next path
      return res;
    }
    else
    {
      PyErr_SetString(PyExc_StopIteration, "No more paths.");
      boost::python::throw_error_already_set();
      const path<pyconf>* dummy = nullptr;
      return *dummy;
    }
  }
};

/*------------------------------------------------------------------------------------------------*/

struct py_visitor
{
  using result_type = object;

  object py;

  py_visitor(object p)
    : py(p)
  {}

  object
  operator()(const hierarchical_node<pyconf>& node)
  const
  {
    return py.attr("hierarchical")(boost::cref(node));
  }

  object
  operator()(const flat_node<pyconf>& node)
  const
  {
    return py.attr("flat")(boost::cref(node));
  }

  object
  operator()(const one_terminal<pyconf>&)
  const
  {
    return py.attr("one")();
  }

  object
  operator()(const zero_terminal<pyconf>&)
  const
  {
    return py.attr("zero")();
  }
};

object
py_visit(object visitor, const SDD<pyconf>& s)
{
  return visit(py_visitor {visitor}, s);
};

/*------------------------------------------------------------------------------------------------*/



BOOST_PYTHON_MODULE(_sdd)
{
  using sdd_type = SDD<pyconf>;
  using hierarchical = hierarchical_node<pyconf>;
  using flat = flat_node<pyconf>;

  class_<_manager, boost::noncopyable>("_manager");

  class_<arc<pyconf, sdd_type>, boost::noncopyable>("HierarchicalArc", no_init)
    .def("valuation", &arc<pyconf, sdd_type>::valuation, return_internal_reference<>())
    .def("successor", &arc<pyconf, sdd_type>::successor)
    ;

  class_<hierarchical, boost::noncopyable>("HierarchicalNode", no_init)
    .def("variable", &hierarchical::variable)
    .def("__iter__", range<return_internal_reference<>>(&hierarchical::begin, &hierarchical::end))
    ;

  class_<arc<pyconf, py_values>, boost::noncopyable>("FlatArc", no_init)
    .def("valuation", &arc<pyconf, py_values>::valuation, return_internal_reference<>())
    .def("successor", &arc<pyconf, py_values>::successor)
    ;

  class_<flat, boost::noncopyable>("FlatNode", no_init)
    .def("variable", &flat::variable)
    .def("__iter__", range<return_internal_reference<>>(&flat::begin, &flat::end))
    ;

  class_<sdd_type>("SDD", init<int, object, sdd_type>())
    .def(init<int, sdd_type, sdd_type>())
    .def(self_ns::str(self))
    .def(self + self)
    .def(self - self)
    .def(self & self)
    .def(self == self)
    .def(self < self)
    .def("__hash__", &sdd_type::hash)
    ;

  def("zero", &zero<pyconf>);
  def("one", &one<pyconf>);

  def("count_combinations", &count_combinations);

  def("visit", &py_visit);

  class_<path<pyconf>>("Path", no_init)
    .def(vector_indexing_suite<path<pyconf>>())
    ;

  class_<paths, boost::noncopyable>("Paths", init<SDD<pyconf>>())
    .def("__iter__", &pass_through)
    .def("next", &paths::next)
    ;

  class_<py_values>("PyValues", no_init)
    .def("content", &py_values::content)
    ;
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::python

namespace std {

/*------------------------------------------------------------------------------------------------*/

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
