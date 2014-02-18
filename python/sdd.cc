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

namespace boost { namespace python {

std::size_t
size(const object& o)
{
  return o.is_none() ? 0 : call_method<std::size_t>(o.ptr(), "__len__" );
}

bool
empty(const object& o)
{
  return size(o) == 0;
}

object
sum(const object& lhs, const object& rhs)
{
  return call_method<object>(lhs.ptr(), "__or__", rhs);
}

object
intersection(const object& lhs, const object& rhs)
{
  return call_method<object>(lhs.ptr(), "__and__", rhs);
}

object
difference(const object& lhs, const object& rhs)
{
  return call_method<object>(lhs.ptr(), "__sub__", rhs);
}

std::ostream&
operator<<(std::ostream& os, const object& o)
{
  return os << call_method<std::string>(o.ptr(), "__str__" );
}

}} // namespace boost::python

/*------------------------------------------------------------------------------------------------*/

namespace sdd { namespace values {

/*------------------------------------------------------------------------------------------------*/

template <>
struct values_traits<boost::python::object>
{
  static constexpr bool stateful = false;
  static constexpr bool fast_iterable = false;
  static constexpr bool has_value_type = false;
};

} // namespace values

/*------------------------------------------------------------------------------------------------*/

namespace python {

namespace bp = boost::python;

/*------------------------------------------------------------------------------------------------*/

struct pyconf
  : public sdd::conf1
{
  using Variable = int;
  using Identifier = std::string;
  using Values = bp::object;
};

/*------------------------------------------------------------------------------------------------*/

double
count_combinations(SDD<pyconf> s)
{
  return sdd::dd::count_combinations(s).template convert_to<double>();
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
bp::object
pass_through(bp::object const& o)
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
  using result_type = bp::object;

  bp::object py;

  py_visitor(bp::object p)
    : py(p)
  {}

  bp::object
  operator()(const hierarchical_node<pyconf>& node)
  const
  {
    return py.attr("hierarchical")(boost::cref(node));
  }

  bp::object
  operator()(const flat_node<pyconf>& node)
  const
  {
    return py.attr("flat")(boost::cref(node));
  }

  bp::object
  operator()(const one_terminal<pyconf>&)
  const
  {
    return py.attr("one")();
  }

  bp::object
  operator()(const zero_terminal<pyconf>&)
  const
  {
    return py.attr("zero")();
  }
};

bp::object
py_visit(bp::object visitor, const SDD<pyconf>& s)
{
  return visit(py_visitor {visitor}, s);
};

/*------------------------------------------------------------------------------------------------*/

BOOST_PYTHON_MODULE(_sdd)
{
  using namespace boost::python;

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

  class_<arc<pyconf, object>, boost::noncopyable>("FlatArc", no_init)
    .def("valuation", &arc<pyconf, object>::valuation, return_internal_reference<>())
    .def("successor", &arc<pyconf, object>::successor)
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
    .def(vector_indexing_suite<path<pyconf>, true /*NoProxy*/>())
    ;

  class_<paths, boost::noncopyable>("Paths", init<SDD<pyconf>>())
    .def("__iter__", &pass_through)
    .def("next", &paths::next)
    ;
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::python

namespace std {

/*------------------------------------------------------------------------------------------------*/

template <>
struct hash<boost::python::object>
{
  std::size_t
  operator()(const boost::python::object& o)
  const
  {
    return static_cast<std::size_t>(boost::python::call_method<long>(o.ptr(), "__hash__"));
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std
