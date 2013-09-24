#ifndef _SDD_ORDER_ORDER_HH_
#define _SDD_ORDER_ORDER_HH_

#include <algorithm> // find
#include <initializer_list>
#include <iostream>
#include <memory>    // shared_ptr, unique_ptr
#include <sstream>
#include <utility>   // pair
#include <vector>

#include <boost/iterator/transform_iterator.hpp>

#include "sdd/order/order_builder.hh"
#include "sdd/util/boost_multiindex_no_warnings.hh"
#include "sdd/util/hash.hh"

namespace sdd {

namespace bmi = boost::multi_index;

/*------------------------------------------------------------------------------------------------*/

/// @brief Represent an order of identifiers, possibly with some hierarchy.
///
/// It helps associate a variable (generated by the library) in an SDD to an identifiers
/// (provided to the user). An identifier should appear only once by order.
template <typename C>
class order final
{
public:

  /// @brief A user's identifier type.
  typedef typename C::Identifier identifier_type;

  /// @brief A library's variable type.
  typedef typename C::Variable variable_type;

private:

  /// @brief A node in an order.
  ///
  /// An order is actually represented as a linked list of node.
  struct node
  {
    /// @brief The (user's) identifier of this node.
    const identifier_type identifier;

    /// @brief The (library's) variable of this node.
    const variable_type variable;

    /// @brief Absolute position, when seeing the order as flatten.
    ///
    /// Used to establish a total order on identifiers.
    const unsigned int position;

    /// @brief A pointer to following order's head.
    const node* next;

    /// @brief A pointer to the nested order's head.
    const node* nested;

    /// @brief The path to this node.
    const std::shared_ptr<std::vector<identifier_type>> path_ptr;

    /// @brief Constructor.
    node( const identifier_type& id, const variable_type& var, unsigned int pos
        , const node* nxt, const node* nst
        , const std::shared_ptr<std::vector<identifier_type>>& path)
      : identifier(id)
      , variable(var)
      , position(pos)
      , next(nxt)
      , nested(nst)
      , path_ptr(path)
    {}

    /// @brief Compare two node using their abolute position.
    bool
    operator<(const node& other)
    const noexcept
    {
      return position < other.position;
    }
  };

  /// @brief Tag to access nodes using their absolute positions.
  struct by_position {};

  /// @brief Tag to access nodes using their identifiers.
  struct by_identifier {};

  /// @brief The type of the container of nodes.
  ///
  /// A boost::multi_index::multi_index_container is used in order to be able to access nodes
  /// using different criterions.
  typedef bmi::multi_index_container
          < node
          , bmi::indexed_by
            < // sort by node::operator<
              bmi::ordered_unique< bmi::tag<by_position>
                                 , bmi::identity<node>
                                 >

              // retrieve using identifier's hash
            , bmi::hashed_unique< bmi::tag<by_identifier>
                                , bmi::member<node, const identifier_type, &node::identifier>
                                , std::hash<identifier_type>
                                >
            >
          > nodes_type;

  /// @brief
  typedef typename nodes_type::template index<by_identifier>::type::const_iterator
          identifiers_const_iterator;

  /// @brief The concrete order.
  const std::shared_ptr<nodes_type> nodes_ptr_;

  /// @brief The first node in the order.
  const node* head_;

  /// @brief Extract the identifier of a node.
  struct extract_identifier
  {
    typedef const identifier_type& result_type;

    const identifier_type&
    operator()(const node& n)
    const noexcept
    {
      return n.identifier;
    }
  };

public:

  /// @brief
  typedef boost::transform_iterator<extract_identifier, identifiers_const_iterator> const_iterator;

  /// @brief Constructor.
  order(const order_builder<C>& builder)
    : nodes_ptr_(mk_nodes_ptr(builder))
    , head_(mk_head())
  {}

  /// @brief Tell if lhs is before rhs in this order.
  ///
  /// Here, 'before' mean that the order is seen as flatten and thus that an hierarchical
  /// identifier is located before its nested identifiers.
  bool
  compare(const identifier_type& lhs, const identifier_type& rhs)
  const
  {
    const auto& identifiers = nodes_ptr_->template get<by_identifier>();

    const auto lhs_search = identifiers.find(lhs);
    if (lhs_search == identifiers.end())
    {
      std::stringstream ss;
      ss << "order::compare: Identifier " << lhs << " not found";
      throw std::runtime_error(ss.str());
    }

    const auto rhs_search = identifiers.find(rhs);
    if (rhs_search == identifiers.end())
    {
      std::stringstream ss;
      ss << "order::compare: Identifier " << rhs << " not found";
      throw std::runtime_error(ss.str());
    }

    return lhs_search->position < rhs_search->position;
  }

  /// @brief Tell if upper contains nested in its possibily contained hierarchy.
  bool
  contains(const identifier_type& upper, const identifier_type& nested)
  const noexcept
  {
    const auto& identifiers = nodes_ptr_->template get<by_identifier>();

    const auto search = identifiers.find(nested);
    if (search == identifiers.end())
    {
      return false;
    }
    else
    {
      const auto& path = *search->path_ptr;
      return std::find(path.begin(), path.end(), upper) != path.end();
    }
  }

  /// @brief
  const typename nodes_type::template index<by_identifier>::type&
  identifiers()
  const noexcept
  {
    return nodes_ptr_->template get<by_identifier>();
  }

  /// @brief Beginning of identifiers.
  const_iterator
  cbegin()
  const noexcept
  {
    return const_iterator(nodes_ptr_->template get<by_identifier>().begin(), extract_identifier());
  }

  /// @brief End of identifiers.
  const_iterator
  cend()
  const noexcept
  {
    return const_iterator(nodes_ptr_->template get<by_identifier>().end(), extract_identifier());
  }

  /// @brief Tell if two identifiers belong to the same hierarchy.
  bool
  same_hierarchy(const identifier_type& lhs, const identifier_type& rhs)
  const
  {
    const auto& identifiers = nodes_ptr_->template get<by_identifier>();

    const auto lhs_search = identifiers.find(lhs);
    if (lhs_search == identifiers.end())
    {
      std::stringstream ss;
      ss << "Identifier " << lhs << " not found";
      throw std::runtime_error(ss.str());
    }

    const auto rhs_search = identifiers.find(rhs);
    if (rhs_search == identifiers.end())
    {
      std::stringstream ss;
      ss << "Identifier " << rhs << " not found";
      throw std::runtime_error(ss.str());
    }

    // paths are shared, we can compare pointers
    return lhs_search->path_ptr == rhs_search->path_ptr;
  }

  /// @brief Get the variable of an identifier
  variable_type
  identifier_variable(const identifier_type& id)
  const
  {
    const auto& identifiers = nodes_ptr_->template get<by_identifier>();
    const auto search = identifiers.find(id);
    if (search == identifiers.end())
    {
      std::stringstream ss;
      ss << "Identifier " << id << " not found";
      throw std::runtime_error(ss.str());
    }
    return search->variable;
  }

  /// @brief Get the variable of this order's head.
  const variable_type&
  variable()
  const
  {
    if (head_)
    {
      return head_->variable;
    }
    else
    {
      throw std::runtime_error("Calling variable() on an empty order.");
    }
  }

  /// @brief Get the identifier of this order's head.
  const identifier_type&
  identifier()
  const
  {
    if (head_)
    {
      return head_->identifier;
    }
    else
    {
      throw std::runtime_error("Calling identifier() on an empty order.");
    }
  }

  /// @brief Get the next order of this order's head.
  order
  next()
  const
  {
    if (head_)
    {
      return order(nodes_ptr_, head_->next);
    }
    else
    {
      throw std::runtime_error("Calling next() on an empty order.");
    }
  }

  /// @brief Get the nested order of this order's head.
  order
  nested()
  const
  {
    if (head_)
    {
      return order(nodes_ptr_, head_->nested);
    }
    else
    {
      throw std::runtime_error("Calling nested() on an empty order.");
    }
  }

  /// @brief Tell if this order is empty.
  bool
  empty()
  const noexcept
  {
    return head_ == nullptr;
  }

  /// @internal
  std::size_t
  hash()
  const noexcept
  {
    std::size_t seed = 0;
    util::hash_combine(seed, nodes_ptr_.get());
    util::hash_combine(seed, head_);
    return seed;
  }

  /// @internal
  bool
  operator==(const order& other)
  const noexcept
  {
    return nodes_ptr_ == other.nodes_ptr_ and head_ == other.head_;
  }

private:

  /// @brief Construct whith a shallow copy an already existing order.
  order(const std::shared_ptr<nodes_type>& ptr, const node* head)
    : nodes_ptr_(ptr)
    , head_(head)
  {}

  /// @brief Create the concrete order using an order_builder.
  static
  std::shared_ptr<nodes_type>
  mk_nodes_ptr(const order_builder<C>& builder)
  {
    std::shared_ptr<nodes_type> nodes_ptr = std::make_shared<nodes_type>();

    if (builder.empty())
    {
      return nodes_ptr;
    }

    unsigned int pos = 0;

    // To enable recursion in the lambda.
    std::function<
      std::pair<const node*, variable_type>
      (const order_builder<C>&, const std::shared_ptr<std::vector<identifier_type>>&)
    > helper;

    helper = [&helper, &nodes_ptr, &pos]
    (const order_builder<C>& ob, const std::shared_ptr<std::vector<identifier_type>>& path)
    -> std::pair<const node*, unsigned int>
    {
      constexpr variable_type first_variable = 0;

      const unsigned int old_pos = pos++;

      std::pair<const node*, variable_type> nested(nullptr, first_variable);
      std::pair<const node*, variable_type> next(nullptr, first_variable);

      if (not ob.nested().empty())
      {
        const auto new_path = std::make_shared<std::vector<identifier_type>>(*path);
        new_path->push_back(ob.identifier());
        new_path->shrink_to_fit();
        nested = helper(ob.nested(), new_path);
      }

      if (not ob.next().empty())
      {
        next = helper(ob.next(), path);
      }

      const auto& variable = next.second;
      /// TODO Manage artificial identifiers.
      const node n(ob.identifier(), variable, old_pos, next.first, nested.first, path);

      const auto insertion = nodes_ptr->insert(n);
      if (not insertion.second)
      {
        std::stringstream ss;
        ss << "Duplicate identifier " << ob.identifier();
        throw std::runtime_error(ss.str());
      }
      return std::make_pair( &*(insertion.first), variable + 1);
    };

    helper(builder, std::make_shared<std::vector<identifier_type>>());

    return nodes_ptr;
  }

  /// @brief Get the first node in the order.
  const node*
  mk_head()
  const noexcept
  {
    const auto& positions = nodes_ptr_->template get<by_position>();
    const auto begin = positions.begin();
    return begin == positions.end() ? nullptr : &*begin;
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @brief Textual representation of an order.
/// @related order
template <typename C>
std::ostream&
operator<<(std::ostream& os, const order<C>& ord)
{
  const std::function<std::ostream&(const order<C>&, unsigned int)> helper
   = [&helper, &os](const order<C>& o, unsigned int indent)
  -> std::ostream&
  {
    if (not o.empty())
    {
      const std::string spaces(indent, ' ');
      os << spaces << o.identifier() << std::endl;
      if (not o.nested().empty())
      {
        helper(o.nested(), indent + 2);
      }
      if (not o.next().empty())
      {
        helper(o.next(), indent);
      }
    }
    return os;
  };
  return helper(ord, 0);
}

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Hash specialization for sdd::order.
template <typename C>
struct hash<sdd::order<C>>
{
  std::size_t
  operator()(const sdd::order<C>& o)
  const noexcept
  {
    return o.hash();
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_ORDER_ORDER_HH_
