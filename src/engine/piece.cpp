/*
 *  This file is part of TESTUDO.
 *
 *  Copyright (C) 2018 Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include "piece.h"

namespace testudo
{

constexpr std::initializer_list<int> piece::offsets_[piece::sup_id];
constexpr score piece::value_[piece::sup_id];

}  // namespace testudo
