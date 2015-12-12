/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <cassert>
#include <iosfwd>
#include <string>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include <cereal/external/rapidjson/document.h>
#include <cereal/external/rapidjson/filestream.h>

#include "sdd/order/order.hh"
#include "sdd/order/order_builder.hh"

namespace sdd { namespace tools {

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename C, bool IsIntegral>
struct mk_order_identifier;

/// @internal
template <typename C>
struct mk_order_identifier<C, true>
{
  typename C::Identifier
  operator()(const rapidjson::Value& v)
  const noexcept
  {
    if (v.IsInt())
    {
      return v.GetInt();
    }
    else
    {
      const std::string str = v.GetString();
      try
      {
        return std::stoi(str);
      }
      catch (const std::invalid_argument&)
      {
        throw std::runtime_error("Expected a integer in order JSON, got " + str);
      }
    }
  }
};

/// @internal
template <typename C>
struct mk_order_identifier<C, false>
{
  std::string
  operator()(const rapidjson::Value& v)
  const noexcept
  {
    assert(v.IsString());
    return v.GetString();
  }
};

/// @internal
template <typename C>
order_builder<C>
load_order_impl(const rapidjson::Value& v)
{
  if(v.IsArray())
  {
    order_builder<C> ob;
    for (rapidjson::SizeType i = 0; i < v.Size() ; ++i)
    {
      ob << load_order_impl<C>(v[i]);
    }
    return ob.height() == 1
         ? ob
         : order_builder<C>(order_identifier<C>(), ob);
  }
  else
  {
    constexpr auto is_integral = std::is_integral<typename C::Identifier>::value;
    return order_builder<C>(mk_order_identifier<C, is_integral>()(v));
  }
}

/*------------------------------------------------------------------------------------------------*/

/// @brief Load an order from JSON.
template <typename C>
boost::optional<order_builder<C>>
load_order(std::istream& in)
{
  std::string buffer(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>{});
  if (buffer.empty())
  {
    return {};
  }

  rapidjson::Document doc;
  doc.Parse<0>(&buffer[0]);
  assert(doc.IsArray());

  const auto ob = load_order_impl<C>(doc);
  return ob.height() == 1 and not ob.nested().empty()
       ? ob.nested()
       : ob;
}

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <bool IsIntegral>
struct print_identifier;

/// @internal
template <>
struct print_identifier<true>
{
  template <typename T>
  void
  operator()(std::ostream& os, T x)
  const
  {
    os << x;
  }
};

/// @internal
template <>
struct print_identifier<false>
{
  template <typename T>
  void
  operator()(std::ostream& os, const T& x)
  const
  {
    os << "\"" << x << "\"";
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @brief Dump an order to JSON
template <typename C>
void
dump_order(order<C> o, std::ostream& out)
{
  constexpr auto id_integral = std::is_integral<typename C::Identifier>::value;
  out << "[";
  if (not o.empty())
  {
    if (not o.nested().empty())
    {
      dump_order(o.nested(), out);
      out << std::endl;
    }
    else
    {
      print_identifier<id_integral>()(out, o.identifier().user());
      out << std::endl;
    }
    o = o.next();
    while (not o.empty())
    {
      if (not o.nested().empty())
      {
        out << ",";
        dump_order(o.nested(), out);
        out << std::endl;
      }
      else
      {
        out << ",";
        print_identifier<id_integral>()(out, o.identifier().user());
        out << std::endl;
      }
      o = o.next();
    }
  }
  out << "]";
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::tools
