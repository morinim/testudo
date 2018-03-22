/*
 *  This file is part of TESTUDO.
 *
 *  Copyright (C) 2018 Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <cassert>
#include <regex>

#include "san.h"
#include "state.h"
#include "util.h"

namespace testudo
{

namespace SAN
{

move from(std::string text, const state &s)
{
  text = trim(text);

  const auto moves(s.moves());

  // First check castling...
  if (text == "O-O")
  {
    const auto it(
      std::find_if(moves.begin(), moves.end(),
                   [](const auto &m)
                   {
                     return (m.flags & move::castle) && file(m.to) == FILE_G;
                   }));

    return it == moves.end() ? move::sentry() : *it;
  }

  if (text == "O-O-O")
  {
    const auto it(
      std::find_if(moves.begin(), moves.end(),
                   [](const auto &m)
                   {
                     return (m.flags & move::castle) && file(m.to) == FILE_C;
                   }));

    return it == moves.end() ? move::sentry() : *it;
  }

  // ... then other moves.
  const auto char_to_file([](char c) -> unsigned { return 7 - ('h' - c); });
  const auto char_to_rank([](char c) -> unsigned { return c - '1'; });
  const auto to_piece_type([](char c)
                           {
                             switch (c)
                             {
                             case 'Q':  return  piece::queen;
                             case 'R':  return   piece::rook;
                             case 'B':  return piece::bishop;
                             case 'N':  return piece::knight;
                             case 'P':  return   piece::pawn;
                             default:   return  piece::empty;
                             }
                           });

  std::regex re("([PNBRQK])?([a-h])?([1-8])?x?([a-h])([1-8])(=(N|B|R|Q))?");
  // Group         <-1->      <2>     <3>       <4>    <5>     <---7-->
  //                                                         <----6--->

  std::smatch match;
  if (!std::regex_search(text, match, re) || match.size() <= 4)
    return move::sentry();

  const auto to_file(char_to_file(match.str(4)[0]));
  const auto to_rank(char_to_rank(match.str(5)[0]));

  const square to(to_square(to_file, to_rank));
  if (to == NO_SQ)
    return move::sentry();

  const auto from_file(match.str(2).empty()
                       ? 8 : char_to_file(match.str(2)[0]));
  const auto from_rank(match.str(3).empty()
                       ? 8 : char_to_rank(match.str(3)[0]));

  const auto p(match.str(1).empty()
               ? piece::pawn : to_piece_type(match.str(1)[0]));

  const auto promotion_to(match.size() == 8 ?
                          to_piece_type(match.str(7)[0]) : piece::pawn);
  if (promotion_to == piece::pawn)
    return move::sentry();

  for (const auto &m : moves)
    if (m.to == to
        && s[m.from].type() == p
        && (from_file > 7 || file(m.from) == from_file)
        && (from_rank > 7 || rank(m.from) == from_rank)
        && m.promote() == promotion_to)
      return m;

  return move::sentry();
}

}  // namespace SAN

}  // namespace testudo
