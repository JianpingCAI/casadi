/*
 *    This file is part of CasADi.
 *
 *    CasADi -- A symbolic framework for dynamic optimization.
 *    Copyright (C) 2010-2014 Joel Andersson, Joris Gillis, Moritz Diehl,
 *                            K.U. Leuven. All rights reserved.
 *    Copyright (C) 2011-2014 Greg Horn
 *
 *    CasADi is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License as published by the Free Software Foundation; either
 *    version 3 of the License, or (at your option) any later version.
 *
 *    CasADi is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with CasADi; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */


#ifndef CASADI_INVERSE_HPP
#define CASADI_INVERSE_HPP

#include "mx_node.hpp"
#include <map>
#include <stack>

/// \cond INTERNAL

namespace casadi {
  /** \brief Matrix inverse
      \author Joel Andersson
      \date 2013
  */
  class CASADI_EXPORT Inverse : public MXNode {
  public:

    /// Constructor
    Inverse(const MX& x);

    /// Clone function
    virtual Inverse* clone() const { return new Inverse(*this);}

    /// Destructor
    virtual ~Inverse() {}

    /// Evaluate the function symbolically (MX)
    virtual void eval(const cpv_MX& input, const pv_MX& output);

    /** \brief Calculate forward mode directional derivatives */
    virtual void evalFwd(const std::vector<cpv_MX>& fwdSeed, const std::vector<pv_MX>& fwdSens);

    /** \brief Calculate reverse mode directional derivatives */
    virtual void evalAdj(const std::vector<pv_MX>& adjSeed, const std::vector<pv_MX>& adjSens);

    /// Print a part of the expression */
    virtual void printPart(std::ostream &stream, int part) const;

    /** \brief Get the operation */
    virtual int getOp() const { return OP_INVERSE;}
  };


} // namespace casadi
/// \endcond

#endif // CASADI_INVERSE_HPP
