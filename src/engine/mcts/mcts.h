/**
 *  \file
 *  \remark This file is part of LIGHT MCTS.
 *
 *  \copyright Copyright (C) 2018 Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(LIGHT_MCTS_H)
#define      LIGHT_MCTS_H

#include <algorithm>
#include <cassert>
#include <functional>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <sstream>
#include <vector>

#include "timer.h"

namespace light_mcts
{

template<class C>
const typename C::value_type &random_element(const C &c)
{
  thread_local std::mt19937 random_engine;

  std::uniform_int_distribution<std::size_t> d(0, c.size() - 1);

  return *std::next(c.begin(),
                    static_cast<typename C::difference_type>(d(random_engine)));
}

///
/// Infrastructure to conduct a UCT search.
///
/// Assumes two alternating players, with game results in the range
/// `[0.0, 1.0]`.
///
/// The `STATE` class is something like:
///
///     class state
///     {
///     public:
///       using action = /* the representation of an action/move */;
///                      // also required is a sentinel value action::sentry()
///
///       std::vector<action> actions() const;  // set of actions in the
///                                             // current state
///       void make_action(action);             // performs the required action
///       unsigned agent_id() const;            // active agent
///       std::vector<double> eval() const;     // score from the POV of the
///                                             // `i`-th agent
///       bool is_final() const;                // returns `true` if the state
///                                             // state is final
///     };
///
/// \note
/// Code initially based on the Python implementation at
/// http://mcts.ai/code/index.html (Peter Cowling, Ed Powley, Daniel Whitehouse
/// - University of York).
///
template<class STATE>
class uct
{
public:
  using action = typename STATE::action;
  using scores_t = std::result_of_t<decltype(&STATE::eval)(STATE)>;

  struct params
  {
    std::chrono::milliseconds max_search_time = std::chrono::milliseconds(0);
    std::uintmax_t max_iterations = 0;

    unsigned simulation_depth = std::numeric_limits<unsigned>::max();

    bool verbose = true;
    unsigned log_depth = 1000;
    std::ostream *log = nullptr;
  };

  explicit uct(const STATE &, const params & = params());

  std::pair<action, scores_t> run();

  params p;

private:
  struct node;

  void print_info(node &, std::uintmax_t, std::chrono::milliseconds) const;

  STATE root_state_;
};  // class uct

///
/// A node in the game tree.
///
template<class STATE>
struct uct<STATE>::node
{
  using agent_id_t = std::result_of_t<decltype(&STATE::agent_id)(STATE)>;

  static double uct_k;

  node(const STATE &, const action & = action::sentry(), node * = nullptr);

  node *select_child();
  node *add_child(const STATE &);
  void update(const scores_t &);

  bool fully_expanded() const;

  std::string graph(unsigned) const;

  action parent_action;
  node *parent_node;

  std::vector<action> untried_actions;
  std::vector<node> child_nodes;

  /// Score of the state from multiple POVs
  scores_t score;
  std::intmax_t visits;

  agent_id_t agent_id;   /// id of the active player
};  // struct uct::node

template<class STATE> double uct<STATE>::node::uct_k = 1.0;

///
/// \param[in] state
/// \param[in] parent_action the move that got us to this (`action::sentry()`
///                          for the root node) node/state
/// \param[in] parent_node   `nullptr` for the root node
///
template<class STATE>
uct<STATE>::node::node(const STATE &state, const action &parent_a,
                       node *parent_n)
  : parent_action(parent_a), parent_node(parent_n),
    untried_actions(state.actions()), child_nodes(), score({}), visits(0),
    agent_id(state.agent_id())
{
  child_nodes.reserve(untried_actions.capacity());
}

template<class STATE>
bool uct<STATE>::node::fully_expanded() const
{
  return untried_actions.empty() && child_nodes.size();
}

template<class STATE>
std::string uct<STATE>::node::graph(unsigned log_depth) const
{
  std::string ret("digraph g {");
  unsigned nodes(0);

  const std::function<void (const node &, unsigned, unsigned)> visit(
    [&](const node &n, unsigned depth, unsigned parent_id)
    {
      const auto id(++nodes);
      const auto node_name("N" + std::to_string(id));

      ret += node_name + " [label=\"" + std::to_string(n.agent_id);

      ret += " ( ";
      for (auto s : n.score)
        ret += std::to_string(s) + " ";
      ret += ")";

      ret += "/" + std::to_string(n.visits) + "\"";

      if (n.child_nodes.empty())
        ret += " shape=rectangle";

      ret += "];";

      if (parent_id)
      {
        const auto parent_name("N" + std::to_string(parent_id));
        ret += parent_name + std::string("->") + node_name;

        std::ostringstream ss;
        ss << n.parent_action;
        ret += " [label=\"" + ss.str() + "\"];";
      }

      if (depth < log_depth)
        for (const auto &child : n.child_nodes)
          visit(child, depth + 1, id);
    });

  visit(*this, 0, 0);

  ret += "}";

  return ret;
}

///
/// Selects a child node using the UCB formula.
///
template<class STATE>
typename uct<STATE>::node *uct<STATE>::node::select_child()
{
  assert(fully_expanded());

  const auto ucb(
    [this](const node &child)
    {
      if (!child.visits)
        return std::numeric_limits<typename scores_t::value_type>::max();

      assert(child.visits <= visits);
      assert(!child.score.empty());

      // Agent-just-moved point of view for the score.
      return child.score[agent_id] / child.visits
             + uct_k * std::sqrt(2 * std::log(visits) / child.visits);
    });

  return &*std::max_element(child_nodes.begin(), child_nodes.end(),
                            [ucb](const node &lhs, const node &rhs)
                            {
                              return ucb(lhs) < ucb(rhs);
                            });
}

///
/// Adds a new child to the current node and updates the list of untried
/// actions.
///
/// \param[in] s current state
/// \return      the added child node
///
/// Assumes that the child node has been reached via the last untried action.
///
template<class STATE>
typename uct<STATE>::node *uct<STATE>::node::add_child(const STATE &s)
{
  assert(untried_actions.size());

  // Reallocation would create dangling pointers.
  assert(child_nodes.size() < child_nodes.capacity());

  child_nodes.emplace_back(s, untried_actions.back(), this);
  untried_actions.pop_back();

  return &child_nodes.back();
}

///
/// Updates this node (one additional visit and additional value).
///
/// \param[in] sv score vector
///
template<class STATE>
void uct<STATE>::node::update(const scores_t &sv)
{
  ++visits;

  if (score.empty())
    score = sv;
  else
  {
    assert(score.size() == sv.size());
    for (std::size_t i(0); i < sv.size(); ++i)
      score[i] += sv[i];
  }
}

template<class STATE>
uct<STATE>::uct(const STATE &root_state, const params &ps)
  : p(ps), root_state_(root_state)
{
}

///
/// Conducts a UCT search for starting from the root state.
///
/// \return the best action from the root state
///
template<class STATE>
std::pair<typename uct<STATE>::action, typename uct<STATE>::scores_t>
uct<STATE>::run()
{
  bool stop_request(false);
  decltype(p.max_iterations) iterations(0);
  timer t;

  node root_node(root_state_);

  if (root_state_.is_final())
    return {uct<STATE>::action::sentry(), root_state_.eval()};

  while (!stop_request)
  {
    node *n(&root_node);
    STATE state(root_state_);

    // Selection.
    while (n->fully_expanded())
    {
      n = n->select_child();
      state.make_action(n->parent_action);
    }

    // Expansion.
    if (n->untried_actions.size())  // node can be expanded
    {
      state.make_action(n->untried_actions.back());
      n = n->add_child(state);
    }

    // Simulation (aka Playout / Rollout).
    auto remaining_depth(p.simulation_depth);
    for (auto actions(n->untried_actions);
         remaining_depth && !state.is_final();
         --remaining_depth, actions = state.actions())
    {
      state.make_action(random_element(actions));
    }

    // Backpropagate.
    const auto scores(state.eval());
    for (; n; n = n->parent_node)
      n->update(scores);

    // Poll for stop conditions.
    if ((++iterations & 1023) == 0)
    {
      stop_request = (p.max_iterations && iterations >= p.max_iterations)
                     || t.elapsed(p.max_search_time);
    }
  }

  if (p.verbose)
  {
    for (const auto &n : root_node.child_nodes)
    {
      std::cout << "#-------------------------------------------\n"
                << "# move: " << n.parent_action << "   score: "
                << n.score[root_node.agent_id] << '/' << n.visits
                << std::endl;
    }
  }

  if (p.log)
    (*p.log) << root_node.graph(p.log_depth) << std::flush;

  const node &best(*std::max_element(root_node.child_nodes.begin(),
                                     root_node.child_nodes.end(),
                                     [](const node &lhs, const node &rhs)
                                     {
                                       return lhs.visits < rhs.visits;
                                     }));
  return {best.parent_action, best.score};
}

}  // namespace light_mcts

#endif  // include guard
