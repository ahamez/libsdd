#ifndef _SDD_TOOLS_LOAD_ORDER_HH_
#define _SDD_TOOLS_LOAD_ORDER_HH_

#include <cassert>
#include <iosfwd>
#include <string>
#include <stdexcept>
#include <type_traits>

#include <cereal/external/rapidjson/document.h>

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
      ob = ob << load_order_impl<C>(v[i]);
    }
    return order_builder<C>(order_identifier<C>(), ob);
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
order_builder<C>
load_order(std::istream& in)
{
  std::string buffer;
  buffer.reserve(16384);
  {
    std::string line;
    while(std::getline(in,line))
    {
      buffer += line;
    }
  }

  rapidjson::Document doc;
  doc.Parse<0>(&buffer[0]);
  assert(doc.IsArray());

  return load_order_impl<C>(doc);
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::tools

#endif // _SDD_TOOLS_LOAD_ORDER_HH_
