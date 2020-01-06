// -*- mode: c++; c-basic-offset:4 -*-
//
// focovjson_utils.h
//
// This file is part of BES CovJSON File Out Module
//
// Copyright (c) 2018 OPeNDAP, Inc.
// Author: Corey Hemphill <hemphilc@oregonstate.edu>
// Author: River Hendriksen <hendriri@oregonstate.edu>
// Author: Riley Rimer <rrimer@oregonstate.edu>
//
// Adapted from the File Out JSON module implemented by Nathan Potter
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//

#ifndef FOCOVJSON_UTILS_H_
#define FOCOVJSON_UTILS_H_ 1

#include <string>
#include <vector>

#include <Array.h>

namespace focovjson {

/**
 * @brief Removes a substring from a given string
 *
 * @param str string to remove a substring from
 * @param subStr sub-string to be removed from a given string
 * 
 * @returns modifies str by reference with the specified substring removed
 */
void removeSubstring(string& str, const string subStr);

/**
 * @brief Compute the constrained shape of the Array and return it in a vector.
 * Also return the total number of elements in the constrained array.
 *
 * @param a The Array to examine
 * @param shape The shape of the Array, taking into account the constraint
 * 
 * @return The total number of elements in the constrained Array.
 */
long computeConstrainedShape(libdap::Array *a, std::vector<unsigned int> *shape);

std::string escape_for_covjson(const std::string &source);

/**
 * Replace every occurrence of 'char_to_escape' with the same preceded
 * by the backslash '\' character.
 */
#if 0
std::string backslash_escape(std::string source, char char_to_escape);
#endif

} // namespace focovjson

#endif /* FOCOVJSON_UTILS_H_ */
