/*
 *  This file is part of TESTUDO.
 *
 *  Copyright (C) 2018 Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(TESTUDO_STATE_H)
#define      TESTUDO_STATE_H

#include "move.h"
#include "movelist.h"
#include "zobrist.h"

namespace testudo
{

class state
{
public:
  enum class setup {start, empty};
  enum class kind {standard,
                   draw_stalemate, draw_repetition, draw_fifty,
                   mated};
  enum castle_flags {white_kingside = 1, white_queenside = 2,
                     black_kingside = 4, black_queenside = 8};

  // Chess initial state.
  explicit state(setup = setup::start) noexcept;
  // Sets up the board given a FEN description.
  explicit state(const std::string &);

  // Generates the set of legal moves.
  movelist moves() const;
  // Generates the set of legal captures.
  movelist captures() const;

  // Makes a move.
  // If the move is illegal, undoes whatever it did and returns the initial
  // state; otherwise, it returns the updated state.
  state after_move(const move &) const;
  bool make_move(const move &);

  // Returns `true` if square is being attacked by color, `false` otherwise.
  bool attack(square, color) const;

  // Returns `true` if the specified color is in check.
  bool in_check(color) const;
  bool in_check() const { return in_check(side()); }

  // Returns `true` if the argument is a legal move (flags must be correct).
  bool is_legal(const move &) const;

  kind mate_or_draw(const std::vector<hash_t> * = nullptr) const;

  piece operator[](square s) const noexcept { return board_[s]; }

  // Fifty moves draw counter value.
  auto fifty() const noexcept { return fifty_; }
  // En passant square.
  square en_passant() const noexcept { return ep_; }
  // Castle rights.
  auto castle() const noexcept { return castle_; }
  bool kingside_castle(color c) const noexcept
  { return castle_ & (c == BLACK ? black_kingside : white_kingside); }
  bool queenside_castle(color c) const noexcept
  { return castle_ & (c == BLACK ? black_queenside : white_queenside); }

  // Gets side to move.
  color side() const noexcept { return stm_; }
  // Changes side to move.
  void switch_side() noexcept { stm_ = !stm_; }

  state color_flip() const;

  move parse_move(const std::string &) const;

  hash_t hash() const noexcept { return hash_; }

  unsigned piece_count(color, enum piece::type) const;
  square king_square(color) const;

private:
  void add_m(movelist &, square, square, move::flags_t) const;
  template<class F> void process_castles(F) const;
  template<class F> void process_en_passant(F) const;
  template<class F> void process_pawn_captures(F, square) const;
  template<class F> void process_pawn_m(F, square, square, move::flags_t) const;
  template<class F> void process_piece_moves(F, square) const;

  void clear_square(square);
  void fill_square(piece, square);

  friend bool operator==(const state &, const state &);

  std::array<piece, 64> board_; // piece+color or EMPTY
  color stm_;                   // side to move
  std::uint8_t castle_;         // castle permissions
  square ep_;                   // en passant square
  std::uint8_t fifty_;          // handles the fifty-move-draw rule

  hash_t hash_;

  // Piece counter. E.g `piece_cnt[WHITE][piece::knight]` contains the number
  // of white knights on the board.
  // `piece_cnt[WHITE][piece::king]` is special: since there are always two
  // kings, we use the slot to store the kings' position.
  std::uint8_t piece_cnt_[2][6];
};

inline state state::after_move(const move &m) const
{
  state after(*this);
  after.make_move(m);
  return after;
}

inline unsigned state::piece_count(color c, enum piece::type t) const
{
  assert(t != piece::king);
  return piece_cnt_[c][t];
}

inline square state::king_square(color c) const
{
  assert(board_[piece_cnt_[c][piece::king]] == piece(c, piece::king));
  return piece_cnt_[c][piece::king];
}

inline bool state::in_check(color c) const
{
  return attack(king_square(c), !c);
}

inline bool operator==(const state &lhs, const state &rhs)
{
  return lhs.board_ == rhs.board_ && lhs.side() == rhs.side()
         && lhs.castle() == rhs.castle()
         && lhs.en_passant() == rhs.en_passant() && lhs.fifty() == rhs.fifty()
         && lhs.hash() == rhs.hash();
}

inline bool operator!=(const state &lhs, const state &rhs)
{
  return !(lhs == rhs);
}

std::ostream &operator<<(std::ostream &, const state &);

}  // namespace state

#endif  // include guard
