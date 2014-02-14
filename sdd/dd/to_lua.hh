#ifndef _SDD_DD_TO_LUA_HH_
#define _SDD_DD_TO_LUA_HH_

#include <unordered_map>

#include "sdd/dd/definition.hh"

namespace sdd {

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

  /// @brief Constructor.
  to_lua_visitor(std::ostream& os)
    : os_(os), next_id_(0)
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
      os_ << "local n" << id << " = {\n";
      os_ << " variable = 0,\n";
      os_ << " value = 0\n";
      os_ << "}\n\n";
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
      os_ << "local n" << id << " = {\n";
      os_ << " variable = 0,\n";
      os_ << " value = 1\n";
      os_ << "}\n\n";
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

      os_ << "local n" << res.id << " = {\n";
      os_ << " variable = " << res.height << ",\n";
      auto arc_cit = n.begin();
      for (auto succs_cit = succs.cbegin(); succs_cit != succs.end(); ++succs_cit, ++arc_cit)
      {
        os_ << " {\n";
        os_ << "  ";
        for (const auto& v : arc_cit->valuation())
        {
          os_ << v << ",";
        }
        os_ << "\n";
        os_ << "  successor = n" << succs_cit->id << "\n";
        os_ << " }";
        if (succs_cit != succs.end() - 1)
        {
          os_ << ",";
        }
        os_ << "\n";
      }
      os_ << "}\n\n";
      insertion.first->second = res;
    }

    return insertion.first->second;
  }

  /// @brief Hierarchical SDD.
  result_type
  operator()(const hierarchical_node<C>& n)
  const
  {
    const auto insertion = cache_.emplace(reinterpret_cast<const char*>(&n), result_type());
    if (insertion.second)
    {
      using arc_type = typename flat_node<C>::arc_type;

      std::vector<result_type> succs;
      succs.reserve(n.size());

      std::vector<result_type> nested;
      nested.reserve(n.size());

      for (const auto& arc : n)
      {
        succs.push_back(visit(*this, arc.successor()));
      }

      for (const auto& arc : n)
      {
        nested.push_back(visit(*this, arc.valuation()));
      }

      result_type res(succs.front().height + 1, next_id_++);

      os_ << "local n" << res.id << " = {\n";
      os_ << " variable = " << res.height << ",\n";
      auto nested_cit = nested.cbegin();
      for (auto succs_cit = succs.cbegin(); succs_cit != succs.end(); ++succs_cit, ++nested_cit)
      {
        os_ << " {\n";
        os_ << "  n" << nested_cit->id << ",\n";
        os_ << "  successor = n" << succs_cit->id << "\n";
        os_ << " }";
        if (succs_cit != succs.end() - 1)
        {
          os_ << ",";
        }
        os_ << "\n";
      }
      os_ << "}\n\n";

      insertion.first->second = res;
    }
    return insertion.first->second;

  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename C>
inline
void
to_lua(SDD<C> x, std::ostream& os)
{
  const auto res = visit(to_lua_visitor<C>(os), x);
  os << "return n" << res.id << "\n";
}

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

#endif // _SDD_DD_TO_LUA_HH_
