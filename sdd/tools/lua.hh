#ifndef _SDD_TOOLS_LUA_HH_
#define _SDD_TOOLS_LUA_HH_

#include <unordered_map>

#include "sdd/dd/definition.hh"

namespace sdd { namespace tools {

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename C>
struct to_lua_visitor
{
  /// @brief Required by mem::variant visitor mechanism.
  struct result_type
  {
    unsigned int height;
    unsigned int id;

    result_type() = default;

    result_type(unsigned int h, unsigned int i)
      : height(h), id(i)
    {}
  };

  /// @brief A cache is necessary to to know if a node has already been encountered.
  ///
  /// We use the addresses of nodes as key. It's legit because nodes are unified and immutable.
  mutable std::unordered_map<const char*, result_type> cache_;

  /// @brief The stream to export to.
  std::ostream& os_;

  /// @brief The node counter.
  mutable unsigned int next_id_;

  /// @brief
  const unsigned int table_height_;

  /// @brief Constructor.
  to_lua_visitor(std::ostream& os, unsigned int table_height)
    : os_(os), next_id_(0), table_height_(table_height)
  {}

  /// @brief |0|.
  result_type
  operator()(const zero_terminal<C>& n)
  const
  {
    const auto insertion = cache_.emplace(reinterpret_cast<const char*>(&n), result_type());
    if (insertion.second)
    {
      const auto id = next_id_++;
      if ((id  % table_height_) == 0)
      {
        os_ << "table.insert(create, function()\n";
      }
      os_ << "  nodes[" << id << "] = {variable = 0, terminal = 0}\n";
      if ((id  % table_height_) == (table_height_ - 1))
      {
        os_ << "end)\n";
      }
      insertion.first->second.height = 0;
      insertion.first->second.id = id;
    }
    return insertion.first->second;
  }

  /// @brief |1|.
  result_type
  operator()(const one_terminal<C>& n)
  const
  {
    const auto insertion = cache_.emplace(reinterpret_cast<const char*>(&n), result_type());
    if (insertion.second)
    {
      const auto id = next_id_++;
      if ((id  % table_height_) == 0)
      {
        os_ << "table.insert(create, function()\n";
      }
      os_ << "  nodes[" << id << "] = {variable = 0, terminal = 1}\n";
      if ((id  % table_height_) == (table_height_ - 1))
      {
        os_ << "end)\n";
      }
      insertion.first->second.height = 0;
      insertion.first->second.id = id;
    }
    return insertion.first->second;
  }

  /// @brief Flat SDD.
  result_type
  operator()(const flat_node<C>& n)
  const
  {
    const auto insertion = cache_.emplace(reinterpret_cast<const char*>(&n), result_type());
    if (insertion.second)
    {
      using arc_type = typename flat_node<C>::arc_type;

      std::vector<result_type> succs;
      succs.reserve(n.size());

      for (const auto& arc : n)
      {
        succs.push_back(visit(*this, arc.successor()));
      }

      result_type res(succs.front().height + 1, next_id_++);

      if ((res.id  % table_height_) == 0)
      {
        os_ << "table.insert(create, function()\n";
      }

      os_ << "  nodes[" << res.id << "] = { variable = " << res.height << "\n";
      auto arc_cit = n.begin();
      for (auto succs_cit = succs.cbegin(); succs_cit != succs.end(); ++succs_cit, ++arc_cit)
      {
        os_ << "             , {";
        for (const auto& v : arc_cit->valuation())
        {
          os_ << v << ",";
        }
        os_ << " successor = nodes[" << succs_cit->id << "]}\n";
      }
      os_ << "             }\n";

      insertion.first->second = res;

      if ((res.id  % table_height_) == (table_height_ - 1))
      {
        os_ << "end)\n";
      }
    }
    return insertion.first->second;
  }

  /// @brief Hierarchical SDD.
  result_type
  operator()(const hierarchical_node<C>& n)
  const
  {
    os_ << "Hierarchial SDD not supported yet." << std::endl;
    return result_type();
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename C>
struct to_lua
{
  const SDD<C> x_;

  to_lua(const SDD<C>& x)
    : x_(x)
  {}

  friend
  std::ostream&
  operator<<(std::ostream& out, const to_lua& manip)
  {
    const unsigned int table_height = 100;
    out << "local nodes = {}\nlocal create = {}\n";
    const auto last = visit(to_lua_visitor<C>(out, table_height), manip.x_);
    if ((last.id  % table_height) != (table_height - 1))
    {
      out << "end)\n";
    }
    return out << "for _, f in ipairs(create) do f() end\nreturn nodes[#nodes]";
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Dump an SDD as a Lua data structure for inspection purposes.
template <typename C>
to_lua<C>
lua(const SDD<C>& x)
{
  return to_lua<C>(x);
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::tools

#endif // _SDD_TOOLS_LUA_HH_
