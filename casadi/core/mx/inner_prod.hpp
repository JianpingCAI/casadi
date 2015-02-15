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


#ifndef CASADI_INNER_PROD_HPP
#define CASADI_INNER_PROD_HPP

#include "mx_node.hpp"
#include <map>
#include <stack>

/// \cond INTERNAL

namespace casadi {
  /** \brief Matrix inner_prod
      \author Joel Andersson
      \date 2013
  */
  class CASADI_EXPORT InnerProd : public MXNode {
  public:

    /// Constructor
    InnerProd(const MX& x, const MX& y);

    /// Clone function
    virtual InnerProd* clone() const { return new InnerProd(*this);}

    /// Destructor
    virtual ~InnerProd() {}

    /// Evaluate the function (template)
    template<typename T>
    void evalGen(const std::vector<const T*>& input,
                 const std::vector<T*>& output, int* itmp, T* rtmp);

    /// Evaluate the function numerically
    virtual void evalD(const cpv_double& input, const pv_double& output,
                       int* itmp, double* rtmp);

    /// Evaluate the function symbolically (SX)
    virtual void evalSX(const cpv_SXElement& input, const pv_SXElement& output,
                            int* itmp, SXElement* rtmp);

    /** \brief  Propagate sparsity forward */
    virtual void spFwd(const cpv_bvec_t& arg,
                       const pv_bvec_t& res, int* itmp, bvec_t* rtmp);

    /** \brief  Propagate sparsity backwards */
    virtual void spAdj(const pv_bvec_t& arg,
                       const pv_bvec_t& res, int* itmp, bvec_t* rtmp);

    /// Evaluate the function symbolically (MX)
    virtual void eval(const cpv_MX& input, const pv_MX& output);

    /** \brief Calculate forward mode directional derivatives */
    virtual void evalFwd(const std::vector<cpv_MX>& fwdSeed, const std::vector<pv_MX>& fwdSens);

    /** \brief Calculate reverse mode directional derivatives */
    virtual void evalAdj(const std::vector<pv_MX>& adjSeed, const std::vector<pv_MX>& adjSens);

    /** \brief Generate code for the operation */
    void generate(std::ostream &stream, const std::vector<int>& arg,
                           const std::vector<int>& res, CodeGenerator& gen) const;

    /// Print a part of the expression */
    virtual void printPart(std::ostream &stream, int part) const;

    /** \brief Get the operation */
    virtual int getOp() const { return OP_INNER_PROD;}
  };


} // namespace casadi
/// \endcond

#endif // CASADI_INNER_PROD_HPP
