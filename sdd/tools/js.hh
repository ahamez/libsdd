/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <sstream>
#include <string>

#include <cereal/external/rapidjson/document.h>
#include <cereal/external/rapidjson/prettywriter.h>

#include "sdd/hom/definition.hh"

namespace sdd { namespace tools {

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename C>
struct hom_to_json_visitor
{
  rapidjson::Value& value;
  rapidjson::Document::AllocatorType& allocator;

  void
  operator()(const hom::_composition<C>& h)
  const
  {
    value.AddMember("name", "o", allocator);
    rapidjson::Value children(rapidjson::kArrayType);
    rapidjson::Value left_child(rapidjson::kObjectType);
    visit(hom_to_json_visitor{left_child, allocator}, h.left);
    children.PushBack(left_child, allocator);
    rapidjson::Value right_child(rapidjson::kObjectType);
    visit(hom_to_json_visitor{right_child, allocator}, h.right);
    children.PushBack(right_child, allocator);
    value.AddMember("children", children, allocator);
  }

  void
  operator()(const hom::_fixpoint<C>& h)
  const
  {
    value.AddMember("name", "*", allocator);
    rapidjson::Value children(rapidjson::kArrayType);
    rapidjson::Value child(rapidjson::kObjectType);
    visit(hom_to_json_visitor{child, allocator}, h.h);
    children.PushBack(child, allocator);
    value.AddMember("children", children, allocator);
  }

  void
  operator()(const hom::_function<C>& h)
  const
  {
    std::stringstream ss;
    ss << h;
    rapidjson::Value s;
    s.SetString(ss.str().c_str(), static_cast<rapidjson::SizeType>(ss.str().size()), allocator);
    value.AddMember("name", s, allocator);
  }

  void
  operator()(const hom::_identity<C>&)
  const
  {
    value.AddMember("name", "id", allocator);
  }

  void
  operator()(const hom::_local<C>& h)
  const
  {
    const auto target = "@ " + std::to_string(h.target);
    rapidjson::Value s;
    s.SetString(target.c_str(), static_cast<rapidjson::SizeType>(target.size()), allocator);
    value.AddMember("name", s, allocator);
    rapidjson::Value children(rapidjson::kArrayType);
    rapidjson::Value child(rapidjson::kObjectType);
    visit(hom_to_json_visitor{child, allocator}, h.h);
    children.PushBack(child, allocator);
    value.AddMember("children", children, allocator);
  }

  void
  operator()(const hom::_sum<C>& h)
  const
  {
    value.AddMember("name", "+", allocator);
    rapidjson::Value children(rapidjson::kArrayType);
    for (const auto& operand : h)
    {
      rapidjson::Value child(rapidjson::kObjectType);
      visit(hom_to_json_visitor{child, allocator}, operand);
      children.PushBack(child, allocator);
    }
    value.AddMember("children", children, allocator);
  }

  template <typename T>
  void
  operator()(const T&)
  const
  {
    value.AddMember("name", "TODO", allocator);
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename C>
struct hom_to_js
{
  const homomorphism<C> h_;

  friend
  std::ostream&
  operator<<(std::ostream& os, const hom_to_js& manip)
  {
    char buf[1024*128];
    rapidjson::MemoryPoolAllocator<> allocator(buf, sizeof(buf));

    rapidjson::Document document(&allocator, 1024);
    document.SetObject();
    visit(hom_to_json_visitor<C>{document, allocator}, manip.h_);

    // write directly to os
    struct _stream
    {
      std::ostream& of;
      void Put(char ch) {of.put(ch);}
      void Flush() {}
    };
    auto stream = _stream{os};
    rapidjson::PrettyWriter<_stream> writer(stream);
    document.Accept(writer);

    return os << std::endl;
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename C>
hom_to_js<C>
js(const homomorphism<C>& h)
{
  return {h};
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::tools
