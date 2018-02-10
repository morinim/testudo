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
#include <sstream>

#include "state.h"
#include "zobrist.h"

namespace testudo
{

namespace
{

// The so called "mailbox" array (because it looks like a mailbox?). It's
// useful to figure out what pieces can go where.
//
// > The 120 elements represent the 64 valid board squares, plus a 2-square
// > "fringe" or "border" around the valid set of board squares. Why do we need
// > a border that is two squares wide? If you think about the Knight, which
// > moves either two ranks or two files in one move, the reason becomes
// > obvious. For sliding pieces, a one-square border would suffice, but for
// > the knight, we need two.
// > One question you might ask is this representation has 12 ranks, which
// > gives two ranks before the real board and two after the real board, as
// > you've explained, but you only have one extra file on each side of the
// > board. Why? When you study this, you will notice that the right-most
// > "illegal" file is adjacent to the left-most illegal file.
// (Robert Hyatt)
//
// > Let's say we have a Rook on A4 (32) and we want to know if it can move one
// > square to the left. We subtract 1, and we get 31 (H5). The Rook obviously
// > cannot move to H5, but we don't know that without doing a lot of annoying
// > work. What we do is figure out A4's mailbox number, which is 61. Then we
// > subtract 1 from 61 (60) and see what `mailbox[60]` is. In this case, it's
// > `-1`, so it's out of bounds and we can forget it.
// (Tom Kerrigan)
const square mailbox[120] =
{
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, A8, B8, C8, D8, E8, F8, G8, H8, -1,
  -1, A7, B7, C7, D7, E7, F7, G7, H7, -1,
  -1, A6, B6, C6, D6, E6, F6, G6, H6, -1,
  -1, A5, B5, C5, D5, E5, F5, G5, H5, -1,
  -1, A4, B4, C4, D4, E4, F4, G4, H4, -1,
  -1, A3, B3, C3, D3, E3, F3, G3, H3, -1,
  -1, A2, B2, C2, D2, E2, F2, G2, H2, -1,
  -1, A1, B1, C1, D1, E1, F1, G1, H1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

const std::size_t mailbox64[/* here goes a `square` */] =
{
  21, 22, 23, 24, 25, 26, 27, 28,
  31, 32, 33, 34, 35, 36, 37, 38,
  41, 42, 43, 44, 45, 46, 47, 48,
  51, 52, 53, 54, 55, 56, 57, 58,
  61, 62, 63, 64, 65, 66, 67, 68,
  71, 72, 73, 74, 75, 76, 77, 78,
  81, 82, 83, 84, 85, 86, 87, 88,
  91, 92, 93, 94, 95, 96, 97, 98
};

const std::array<int, 2> pawn_capture[2] =
{
  {{9, 11}}, {{-11, -9}}
};

// Forward moving offset for a Pawn of a specific color.
const int pawn_fwd[2] = {8, -8};

// Used to determine the castling permissions after a move. What we do is
// logical-AND the castle bits with the castle_mask bits for both of the
// move's squares. Let's say `castle_` is `white_kingside` (`1`). Now we play
// a move where the rook on H1 gets captured. We AND castle with
// `castle_mask[H1]`, so we have `white_kingside & 14`, and `castle_` becomes
// `0` and WHITE can't castle kingside anymore.
const decltype(state().castle()) castle_mask[64] =
{
   7, 15, 15, 15,  3, 15, 15, 11,
  15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15,
  13, 15, 15, 15, 12, 15, 15, 14
};

}  // unnamed namespace

std::ostream &operator<<(std::ostream &o, const state &s)
{
  std::cout << "8";

  for (square i(0); i < 64; ++i)
  {
    if (s[i] == EMPTY)
      std::cout << " .";
    else
      std::cout << ' ' << s[i].letter();

    if ((i + 1) % 8 == 0 && i != 63)
      std::cout << '\n' << rank(i);
  }

  std::cout << "\n\n  a b c d e f g h\n\n";
  return o;
}

state::state(setup t) noexcept
  : stm_(WHITE),
    castle_(white_kingside|white_queenside|black_kingside|black_queenside),
    ep_(-1), fifty_(0), king_(), hash_()
{
  static const std::array<piece, 64> init_piece(
  {{
    BROOK, BKNIGHT, BBISHOP, BQUEEN, BKING, BBISHOP, BKNIGHT, BROOK,
    BPAWN,   BPAWN,   BPAWN,  BPAWN, BPAWN,   BPAWN,   BPAWN, BPAWN,
    EMPTY,   EMPTY,   EMPTY,  EMPTY, EMPTY,   EMPTY,   EMPTY, EMPTY,
    EMPTY,   EMPTY,   EMPTY,  EMPTY, EMPTY,   EMPTY,   EMPTY, EMPTY,
    EMPTY,   EMPTY,   EMPTY,  EMPTY, EMPTY,   EMPTY,   EMPTY, EMPTY,
    EMPTY,   EMPTY,   EMPTY,  EMPTY, EMPTY,   EMPTY,   EMPTY, EMPTY,
    WPAWN,   WPAWN,   WPAWN,  WPAWN, WPAWN,   WPAWN,   WPAWN, WPAWN,
    WROOK, WKNIGHT, WBISHOP, WQUEEN, WKING, WBISHOP, WKNIGHT, WROOK
  }});

  if (t == setup::start)
  {
    board_ = init_piece;
    king_[BLACK] = E8;
    king_[WHITE] = E1;
  }
  else
  {
    std::fill(board_.begin(), board_.end(), EMPTY);
    king_[BLACK] = -1;
    king_[WHITE] = -1;
  }

  hash_ = zobrist::hash(*this);
}

// Sets up the state starting from a FEN (Forsyth-Edwards Notation)
// description.
//
// <FEN> ::=  <Piece Placement> ' ' <Side to move> ' ' <Castling ability>
//             ' ' <En passant target square> ' ' <Halfmove clock>
//             ' ' <Fullmove counter>
//
// Accepts descriptions without the  halfmove clock and fullmove counter
// values.
state::state(const std::string &a_fen) : state(setup::empty)
{
  std::istringstream ss(a_fen);  // used to split the FEN string
  std::string s;

  // ...first read the board...
  if (!(ss >> s))
    throw std::runtime_error("Wrong FEN format");

  square i(0);
  for (auto l : s)
  {
    if (i >= 64)
      throw std::runtime_error("Wrong FEN format");

    switch (l)
    {
    case 'p': board_[i] =   BPAWN; break;
    case 'n': board_[i] = BKNIGHT; break;
    case 'b': board_[i] = BBISHOP; break;
    case 'r': board_[i] =   BROOK; break;
    case 'q': board_[i] =  BQUEEN; break;
    case 'k': board_[i] =   BKING; king_[BLACK] = i; break;
    case 'P': board_[i] =   WPAWN; break;
    case 'N': board_[i] = WKNIGHT; break;
    case 'B': board_[i] = WBISHOP; break;
    case 'R': board_[i] =   WROOK; break;
    case 'Q': board_[i] =  WQUEEN; break;
    case 'K': board_[i] =   WKING; king_[WHITE] = i; break;
    case '/': continue;
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8': i += l - '1'; break;
    default:  throw std::runtime_error("Wrong FEN format");
    }

    ++i;
  }

  // ...then set the turn...
  if (!(ss >> s))
    throw std::runtime_error("Wrong FEN format");
  if (s == "b")
    stm_ = BLACK;

  // ...castle rights...
  if (!(ss >> s))
    throw std::runtime_error("Wrong FEN format");
  if (s.find('K') == std::string::npos)  castle_ &=  ~white_kingside;
  if (s.find('Q') == std::string::npos)  castle_ &= ~white_queenside;
  if (s.find('k') == std::string::npos)  castle_ &=  ~black_kingside;
  if (s.find('q') == std::string::npos)  castle_ &= ~black_queenside;

  // ...en passant square...
  if (!(ss >> s))
    throw std::runtime_error("Wrong FEN format");
  if (s.length() == 2
      && 'a' <= s[0] && s[0] <= 'h'
      && (s[1] == '3' || s[1] == '6'))
  {
    const int f(std::tolower(s[0]) - 'a');
    const int r(s[1] - '1');

    ep_ = (7 - r) * 8 + f;
  }

  hash_ = zobrist::hash(*this);

  // ...fifty moves counter...
  if (!(ss >> s))
    return;
  fifty_ = std::stoul(s);

  // Ignore full move counter.
  ss >> s;
}


void state::add_m(movelist &moves, square from, square to,
                  decltype(move::flags) flags) const
{
  move m(from, to, flags);

  state s1(*this);
  if (s1.make_move(m))
    moves.push_back(m);
}

void state::add_pawn_m(movelist &moves, square from, square to,
                       decltype(move::flags) flags) const
{
  switch (rank(to))
  {
  case 0:  // promotion
  case 7:
    for (unsigned j(move::promotion_n); j <= move::promotion_q; j <<= 1)
      add_m(moves, from, to, flags|j);
    break;
  default:
    add_m(moves, from, to, flags);
  }
}

void state::add_pawn_captures(movelist &moves, square i) const
{
  for (auto delta : pawn_capture[side()])
  {
    const auto to(mailbox[mailbox64[i] + delta]);

    if (valid(to) && board_[to] != EMPTY && board_[to].color() != side())
      add_pawn_m(moves, i, to, move::pawn|move::capture);
  }
}

void state::add_en_passant(movelist &moves) const
{
  if (valid(ep_))
    for (auto delta : pawn_capture[side()])
    {
      const auto from(mailbox[mailbox64[ep_] - delta]);

      if (valid(from) && board_[from].color() == side()
          && board_[from].type() == piece::pawn)
        add_m(moves, from, ep_, move::pawn|move::capture|move::en_passant);
    }
}

movelist state::moves() const
{
  // The maximum number of moves per positions seems to be 218 but you can
  // hardly get more than 70 moves in a standard game.
  movelist ret;
  ret.reserve(80);

  const auto add([&](square from, square to, decltype(move::flags) flags)
                 {
                   state::add_m(ret, from, to, flags);
                 });

  const auto add_pawn_moves([&](square i) -> void
  {
    add_pawn_captures(ret, i);

    auto to(i + pawn_fwd[side()]);
    if (board_[to] == EMPTY)
    {
      add_pawn_m(ret, i, to, move::pawn);

      if (rank(i) == pawn_base_rank(side()))
      {
        to += pawn_fwd[side()];
        if (board_[to] == EMPTY)
          add_pawn_m(ret, i, to, move::pawn|move::two_squares);
      }
    }
  });

  for (square i(0); i < 64; ++i)
    if (board_[i] != EMPTY && board_[i].color() == side())
    {
      if (board_[i].type() == piece::pawn)
        add_pawn_moves(i);
      else
      {
        for (auto delta : board_[i].offsets())
          for (square to(mailbox[mailbox64[i] + delta]);
               valid(to);
               to = mailbox[mailbox64[to] + delta])
          {
            if (board_[to] != EMPTY)
            {
              if (board_[to].color() != side())
                add(i, to, move::capture);
              break;
            }
            add(i, to, 0);
            if (!board_[i].slide())
              break;
          }
      }
    }

  // Castle moves.
  if (side() == WHITE)
  {
    if ((castle() & white_kingside)
        && board_[F1] == EMPTY && board_[G1] == EMPTY)
      add(E1, G1, move::castle);
    if ((castle() & white_queenside)
        && board_[B1] == EMPTY && board_[C1] == EMPTY && board_[D1] == EMPTY)
      add(E1, C1, move::castle);
  }
  else
  {
    if ((castle() & black_kingside)
        && board_[F8] == EMPTY && board_[G8] == EMPTY)
      add(E8, G8, move::castle);
    if ((castle() & black_queenside)
        && board_[B8] == EMPTY && board_[C8] == EMPTY && board_[D8] == EMPTY)
      add(E8, C8, move::castle);
  }

  add_en_passant(ret);

  return ret;
}

// Basically a copy of state::moves(), just modified to generate only captures
// and promotions.
movelist state::captures() const
{
  movelist ret;
  ret.reserve(40);

  // Just a shortcut for the add_m method.
  const auto add([&](square from, square to, decltype(move::flags) flags)
                 {
                   state::add_m(ret, from, to, flags);
                 });

  for (square i(0); i < 64; ++i)
    if (board_[i] != EMPTY && board_[i].color() == side())
    {
      if (board_[i].type() == piece::pawn)
        add_pawn_captures(ret, i);
      else
      {
        for (auto delta : board_[i].offsets())
          for (square to(mailbox[mailbox64[i] + delta]);
               valid(to);
               to = mailbox[mailbox64[to] + delta])
          {
            if (board_[to] != EMPTY)
            {
              if (board_[to].color() != side())
                add(i, to, move::capture);
              break;
            }
            if (!board_[i].slide())
              break;
          }
      }
    }

  add_en_passant(ret);

  return ret;
}

bool state::attack(square target, color attacker) const
{
  for (square i(0); i < 64; ++i)
    if (board_[i].color() == attacker)
    {
      if (board_[i].type() == piece::pawn)
      {
        for (auto delta : pawn_capture[attacker])
          if (mailbox[mailbox64[i] + delta] == target)
            return true;
      }
      else
      {
        for (auto delta : board_[i].offsets())
          for (square to(mailbox[mailbox64[i] + delta]);
               valid(to);
               to = mailbox[mailbox64[to] + delta])
          {
            if (to == target)
              return true;

            if (board_[to] != EMPTY || !board_[i].slide())
              break;
          }
      }
    }

  return false;
}

bool state::in_check(color c) const
{
  assert(board_[king_[c]] == piece(c, piece::king));
  return attack(king_[c], !c);
}

// Erases a piece on a given square and takes care for all the incrementally
// updated stuff: hash keys, piece counters...
void state::clear_square(square i)
{
  const piece p(board_[i]);
  assert(p != EMPTY);

  hash_ ^= zobrist::piece[p.id()][i];
  board_[i] = EMPTY;
}

// Place a piece on a given square and takes care for all the incrementally
// updated stuff: hash keys, piece counters, king location...
void state::fill_square(piece p, square i)
{
  assert(board_[i] == EMPTY);

  hash_ ^= zobrist::piece[p.id()][i];
  board_[i] = p;

  if (p.type() == piece::king)
    king_[p.color()] = i;
}

bool state::make_move(const move &m)
{
  assert (!m.is_sentry());

  const color xside(!side());

  // Test to see if a castle move is legal and move the Rook (the King is
  // moved with the usual move code later).
  if (m.flags & move::castle)
  {
    if (attack(m.from, xside) || attack(m.to, xside))
      return false;

    square from, to;
    switch (m.to)
    {
    case G1:
      if (attack(F1, xside))
        return false;
      from = H1;
      to   = F1;
      break;

    case C1:
      if (attack(D1, xside))
        return false;
      from = A1;
      to   = D1;
      break;

    case G8:
      if (attack(F8, xside))
        return false;
      from = H8;
      to   = F8;
      break;

    default:
      assert(m.to == C8);
      if (attack(D8, xside))
        return false;
      from = A8;
      to   = D8;
      break;
    }

    fill_square(board_[from], to);
    clear_square(from);
  }

  // Update the castle...
  if (castle())
    hash_ ^= zobrist::castle[castle()];
  castle_ &= castle_mask[m.from] & castle_mask[m.to];
  if (castle())
    hash_ ^= zobrist::castle[castle()];

  // ...en passant...
  if (en_passant() != -1)
  {
    hash_ ^= zobrist::ep[file(en_passant())];
    ep_ = -1;
  }
  if (m.flags & move::two_squares)
  {
    ep_ = m.to - pawn_fwd[side()];
    hash_ ^= zobrist::ep[file(en_passant())];
  }

  // ..and fifty-move-draw variables.
  if (m.flags & (move::pawn|move::capture))
    fifty_ = 0;
  else
    ++fifty_;

  // Move the piece.
  if (board_[m.to] != EMPTY)  // capture:
    clear_square(m.to);       // clears the destination square

  const piece p(m.flags & move::promotion
                ? piece(side(), m.promote())
                : board_[m.from]);
  fill_square(p, m.to);

  clear_square(m.from);

  // Erase the Pawn if this is an en passant move.
  if (m.flags & move::en_passant)
  {
    const auto epc(m.to - pawn_fwd[side()]);
    clear_square(epc);
  }

  // Switch sides and test for legality (if we can capture the other King, it's
  // an illegal position and we need to take the move back).
  stm_ = !side();
  hash_ ^= zobrist::side;

  return !in_check(!side());
}

state::kind state::mate_or_draw(const std::vector<hash_t> *history) const
{
  if (moves().empty())
    return in_check() ? kind::mated : kind::draw_stalemate;

  if (fifty() >= 100)
    return kind::draw_fifty;

  if (history)
  {
    assert(!history->empty());
    assert(history->back() == hash());

    unsigned rep(0);
    for (std::size_t i(0); i < history->size(); ++i)
      if ((*history)[i] == history->back())
        ++i;
    if (rep >= 2)
      return kind::draw_repetition;
  }

  return kind::standard;
}

// parses the move `s` (in coordinate notation) and returns the move converted
// in the internal notation.
move state::parse_move(const std::string &s) const
{
  // Make sure the string looks like a move.
  if (s.length() < 4
      || s[0] < 'a' || s[0] > 'h' || s[1] < '0' || s[1] > '9'
      || s[2] < 'a' || s[2] > 'h' || s[3] < '0' || s[3] > '9')
    return move::sentry();

  square from(s[0] - 'a');
  from += 8 * (8 - (s[1] - '0'));

  square to(s[2] - 'a');
  to += 8 * (8 - (s[3] - '0'));

  unsigned promotion(0);
  if (s.length() > 4)
  {
    if (std::tolower(s[4]) == BKNIGHT.letter())
      promotion = move::promotion_n;
    else if (std::tolower(s[4]) == BBISHOP.letter())
      promotion = move::promotion_b;
    else if (std::tolower(s[4]) == BROOK.letter())
      promotion = move::promotion_r;
    else
      promotion = move::promotion_q;
  }

  for (const auto &m : moves())
    if ( m.from == from && m.to == to
         && (!promotion || (promotion && (m.flags & promotion))) )
      return m;

  return move::sentry();
}

}  // namespace testudo
