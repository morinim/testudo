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

  std::cout << "\n\n  a b c d e f g h\n";
  return o;
}

state::state(setup t) noexcept
  : stm_(WHITE), castle_(0), ep_(-1), fifty_(0), hash_(0), piece_cnt_{}
{
  std::fill(board_.begin(), board_.end(), EMPTY);

  if (t == setup::start)
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

    castle_ = white_kingside|white_queenside|black_kingside|black_queenside;

    for (square i(0); i < 64; ++i)
      if (init_piece[i] != EMPTY)
        fill_square(init_piece[i], i);
  }

  // Fill square has already placed the pieces, but we need to embed into the
  // hash key other state information hence the call to `zobrist::hash`.
  hash_ = zobrist::hash(*this);
}

// Performs a vertical flipping (i.e. mirroring) of all pieces along the
// horizontal axis between the 4th and 5th rank, also swapping the color of the
// flipped pieces, the side to move, the castling rights and the rank of a
// possible en-passant target square (e.g. a white pawn on C2 becomes a black
// pawn on C7).
state state::color_flip() const
{
  state ret(setup::empty);

  for (square i(0); i < 64; ++i)
    if (board_[i] != EMPTY)
      ret.fill_square(piece(!board_[i].color(), board_[i].type()), flip(i));
  ret.hash_ = zobrist::hash(*this);

  ret.stm_ = !side();

  ret.castle_ = 0;
  if (castle() & white_kingside)
    ret.castle_ |= black_kingside;
  if (castle() & white_queenside)
    ret.castle_ |= black_queenside;
  if (castle() & black_kingside)
    ret.castle_ |= white_kingside;
  if (castle() & black_queenside)
    ret.castle_ |= white_queenside;

  if (en_passant())
    ret.ep_ = en_passant() ^ 56;

  ret.fifty_ = fifty_;

  return ret;
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
    case 'p': fill_square(  BPAWN, i); break;
    case 'n': fill_square(BKNIGHT, i); break;
    case 'b': fill_square(BBISHOP, i); break;
    case 'r': fill_square(  BROOK, i); break;
    case 'q': fill_square( BQUEEN, i); break;
    case 'k': fill_square(  BKING, i); break;
    case 'P': fill_square(  WPAWN, i); break;
    case 'N': fill_square(WKNIGHT, i); break;
    case 'B': fill_square(WBISHOP, i); break;
    case 'R': fill_square(  WROOK, i); break;
    case 'Q': fill_square( WQUEEN, i); break;
    case 'K': fill_square(  WKING, i); break;
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
  if (s.find('K') != std::string::npos)  castle_ |=  white_kingside;
  if (s.find('Q') != std::string::npos)  castle_ |= white_queenside;
  if (s.find('k') != std::string::npos)  castle_ |=  black_kingside;
  if (s.find('q') != std::string::npos)  castle_ |= black_queenside;

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
                  move::flags_t flags) const
{
  move m(from, to, flags);

  state s1(*this);
  if (s1.make_move(m))
    moves.push_back(m);
}

template<class F>
void state::process_pawn_m(F f, square from, square to,
                           move::flags_t flags) const
{
  switch (rank(to))
  {
  case 0:  // promotion
  case 7:
    for (unsigned j(move::promotion_n); j <= move::promotion_q; j <<= 1)
      f(from, to, flags|j);
    break;
  default:
    f(from, to, flags);
  }
}

template<class F>
void state::process_pawn_captures(F f, square i) const
{
  assert(board_[i] == piece(side(), piece::pawn));

  for (auto delta : board_[i].offsets())
  {
    const auto to(mailbox[mailbox64[i] + delta]);

    if (valid(to) && board_[to].color() == !side())
      process_pawn_m(f, i, to, move::pawn|move::capture);
  }
}

template<class F>
void state::process_en_passant(F f) const
{
  if (valid(en_passant()))
    for (auto delta : piece(side(), piece::pawn).offsets())
    {
      const auto from(mailbox[mailbox64[ep_] - delta]);

      if (valid(from) && board_[from] == piece(side(), piece::pawn))
        f(from, en_passant(), move::pawn|move::capture|move::en_passant);
    }
}

template<class F>
void state::process_piece_moves(F f, square i) const
{
  assert(board_[i].color() == side());

  const piece p(board_[i]);

  if (p.type() == piece::pawn)
  {
    process_pawn_captures(f, i);

    auto to(i + pawn_fwd[side()]);
    if (board_[to] == EMPTY)
    {
      process_pawn_m(f, i, to, move::pawn);

      if (rank(i) == second_rank(side()))
      {
        to += pawn_fwd[side()];
        if (board_[to] == EMPTY)
          process_pawn_m(f, i, to, move::pawn|move::two_squares);
      }
    }
  }
  else  // not a pawn
  {
    for (auto delta : p.offsets())
      for (square to(mailbox[mailbox64[i] + delta]);
           valid(to);
           to = mailbox[mailbox64[to] + delta])
      {
        if (board_[to] != EMPTY)
        {
          if (board_[to].color() == !side())
            f(i, to, move::capture);
          break;
        }
        f(i, to, 0);
        if (!p.slide())
          break;
      }
  }
}

template<class F>
void state::process_castles(F f) const
{
  if (side() == WHITE)
  {
    if ((castle() & white_kingside)
        && board_[F1] == EMPTY && board_[G1] == EMPTY)
      f(E1, G1, move::castle);
    if ((castle() & white_queenside)
        && board_[B1] == EMPTY && board_[C1] == EMPTY && board_[D1] == EMPTY)
      f(E1, C1, move::castle);
  }
  else
  {
    if ((castle() & black_kingside)
        && board_[F8] == EMPTY && board_[G8] == EMPTY)
      f(E8, G8, move::castle);
    if ((castle() & black_queenside)
        && board_[B8] == EMPTY && board_[C8] == EMPTY && board_[D8] == EMPTY)
      f(E8, C8, move::castle);
  }
}

movelist state::moves() const
{
  // The maximum number of moves per positions seems to be 218 but you can
  // hardly get more than 70 moves in a standard game.
  movelist ret;
  ret.reserve(80);

  const auto add([&](square from, square to, move::flags_t flags)
                 {
                   state::add_m(ret, from, to, flags);
                 });

  for (square i(0); i < 64; ++i)
    if (board_[i].color() == side())
      process_piece_moves(add, i);

  process_castles(add);

  process_en_passant(add);

  return ret;
}

// Basically a copy of `state::moves()`, just modified to generate only
// captures and promotions.
movelist state::captures() const
{
  movelist ret;
  ret.reserve(40);

  // Just a shortcut for the add_m method.
  const auto add([&](square from, square to, move::flags_t flags)
                 {
                   state::add_m(ret, from, to, flags);
                 });

  for (square i(0); i < 64; ++i)
  {
    const piece p(board_[i]);

    if (p.color() == side())
    {
      if (p.type() == piece::pawn)
        process_pawn_captures(add, i);
      else
      {
        for (auto delta : p.offsets())
          for (square to(mailbox[mailbox64[i] + delta]);
               valid(to);
               to = mailbox[mailbox64[to] + delta])
          {
            if (board_[to] != EMPTY)
            {
              if (board_[to].color() == !side())
                add(i, to, move::capture);
              break;
            }
            if (!p.slide())
              break;
          }
      }
    }
  }

  process_en_passant(add);

  return ret;
}

// Could be more efficient but reusing the `process_*` code we try to avoid as
// many bugs as possible.
bool state::is_legal(const move &m) const
{
  assert(valid(m.from) && valid(m.to));

  if (board_[m.from].color() != side())
    return false;

  bool found(false);
  const auto find([&](square from, square to, move::flags_t flags)
                  {
                    if (from == m.from && to == m.to && flags == m.flags)
                      found = true;
                  });

  if ( !(m.flags & (move::en_passant|move::castle)) )
    process_piece_moves(find, m.from);
  else if (m.flags & move::castle)
    process_castles(find);
  else
    process_en_passant(find);

  if (!found)
    return false;

  state s1(*this);
  return s1.make_move(m);
}

bool state::attack(square target, color attacker) const
{
  for (unsigned p(piece::pawn); p <= piece::knight; ++p)
    for (auto delta : piece(!attacker, p).offsets())
    {
      const auto from(mailbox[mailbox64[target] + delta]);

      if (valid(from) && board_[from] == piece(attacker, p))
        return true;
    }

  for (unsigned p(piece::bishop); p <= piece::rook; ++p)
    for (auto delta : piece(attacker, p).offsets())
      for (square from(mailbox[mailbox64[target] + delta]);
           valid(from);
           from = mailbox[mailbox64[from] + delta])

      {
        if (board_[from] == piece(attacker, p)
            || board_[from] == piece(attacker, piece::queen))
          return true;

        if (board_[from] != EMPTY)
          break;
      }

  return false;
}

bool state::in_check(color c) const
{
  assert(board_[piece_cnt_[c][piece::king]] == piece(c, piece::king));
  return attack(piece_cnt_[c][piece::king], !c);
}

// Erases a piece on a given square and takes care for all the incrementally
// updated stuff: hash keys, piece counters...
void state::clear_square(square i)
{
  assert(valid(i));
  const piece p(board_[i]);
  assert(p != EMPTY);

  hash_ ^= zobrist::piece[p.id()][i];
  board_[i] = EMPTY;

  assert(p.type() == piece::king || piece_cnt_[p.color()][p.type()]);
  if (p.type() != piece::king)
    --piece_cnt_[p.color()][p.type()];
}

// Place a piece on a given square and takes care for all the incrementally
// updated stuff: hash keys, piece counters, king location...
void state::fill_square(piece p, square i)
{
  assert(p != EMPTY);
  assert(valid(i));
  assert(board_[i] == EMPTY);

  hash_ ^= zobrist::piece[p.id()][i];
  board_[i] = p;

  if (p.type() == piece::king)
    piece_cnt_[p.color()][piece::king] = i;
  else
    ++piece_cnt_[p.color()][p.type()];
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
  if (valid(en_passant()))
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

  const piece p(is_promotion(m)
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

// Parses the move `s` (in coordinate notation) and returns the move converted
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
