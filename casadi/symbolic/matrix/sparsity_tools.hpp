/*
 *    This file is part of CasADi.
 *
 *    CasADi -- A symbolic framework for dynamic optimization.
 *    Copyright (C) 2010 by Joel Andersson, Moritz Diehl, K.U.Leuven. All rights reserved.
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

#ifndef SPARSITY_TOOLS_HPP
#define SPARSITY_TOOLS_HPP

#include "sparsity.hpp"

namespace casadi{

/**
\ingroup expression_tools
@{ 
*/
    
  /** \brief Reshape the sparsity pattern keeping the relative location of the nonzeros
   */
  CASADI_SYMBOLIC_EXPORT Sparsity reshape(const Sparsity& a, int nrow, int ncol);

  /** \brief Transpose the pattern */
  CASADI_SYMBOLIC_EXPORT inline Sparsity transpose(const Sparsity& a){ return a.transpose();}

  /** \brief Vectorize the pattern */
  CASADI_SYMBOLIC_EXPORT Sparsity vec(const Sparsity& a);
  
  /** \brief Get the sparsity resulting from a matrix multiplication
   */
  CASADI_SYMBOLIC_EXPORT Sparsity mul(const Sparsity& a, const Sparsity &b);
  
  /** \brief Concatenate a list of sparsities vertically
  * Alternative terminology: vertical stack, vstack, vertical append, [a;b]
  */
  CASADI_SYMBOLIC_EXPORT Sparsity vertcat(const std::vector<Sparsity > &v);

  /** \brief Concatenate a list of sparsities horizontally
  * Alternative terminology: horizontal stack, hstack, horizontal append, [a b]
  */
  CASADI_SYMBOLIC_EXPORT Sparsity horzcat(const std::vector<Sparsity > &v);

  /** \brief   Construct a Sparsity with given blocks on the diagonal */
  CASADI_SYMBOLIC_EXPORT Sparsity blkdiag(const std::vector< Sparsity > &v);

  #ifndef SWIG
  CASADI_SYMBOLIC_EXPORT Sparsity horzcat(const Sparsity &x, const Sparsity &y);

  CASADI_SYMBOLIC_EXPORT Sparsity vertcat(const Sparsity &x, const Sparsity &y);
  
  CASADI_SYMBOLIC_EXPORT Sparsity blkdiag(const Sparsity &x, const Sparsity &y);
  #endif // SWIG
  
  /** \brief Split up a sparsity pattern horizontally */
  CASADI_SYMBOLIC_EXPORT std::vector<Sparsity> horzsplit(const Sparsity& sp, const std::vector<int>& output_offset);

  /** \brief Split up a sparsity pattern vertically */
  CASADI_SYMBOLIC_EXPORT std::vector<Sparsity> vertsplit(const Sparsity& sp, const std::vector<int>& output_offset);

  /// Obtain the structural rank of a sparsity-pattern
  CASADI_SYMBOLIC_EXPORT int rank(const Sparsity& a);
  
  /// Get upper triangular part
  CASADI_SYMBOLIC_EXPORT inline Sparsity triu(const Sparsity& sp, bool includeDiagonal=true){ return sp.getTriu(includeDiagonal);}

  /// Get lower triangular part
  CASADI_SYMBOLIC_EXPORT inline Sparsity tril(const Sparsity& sp, bool includeDiagonal=true){ return sp.getTril(includeDiagonal);}
  
  /*
  @}
  */
}

#endif // SPARSITY_TOOLS_HPP