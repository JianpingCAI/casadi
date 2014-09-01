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

#include "dle_internal.hpp"
#include <cassert>
#include "../std_vector_tools.hpp"
#include "../matrix/matrix_tools.hpp"
#include "../mx/mx_tools.hpp"
#include "../sx/sx_tools.hpp"
#include "../function/mx_function.hpp"
#include "../function/sx_function.hpp"

INPUTSCHEME(DLEInput)
OUTPUTSCHEME(DLEOutput)

using namespace std;
namespace casadi {

  DleInternal::DleInternal(const DleStructure& st,
                             int nrhs,
                             bool transp) :
      st_(st), nrhs_(nrhs), transp_(transp) {

    // set default options
    setOption("name", "unnamed_dple_solver"); // name of the function

    addOption("pos_def", OT_BOOLEAN, false, "Assume P positive definite");

    addOption("error_unstable", OT_BOOLEAN, false,
              "Throw an exception when it is detected that Product(A_i, i=N..1) "
              "has eigenvalues greater than 1-eps_unstable");
    addOption("eps_unstable", OT_REAL, 1e-4, "A margin for unstability detection");

    if (nrhs_==1) {
      input_.scheme = SCHEME_DLEInput;
      output_.scheme = SCHEME_DLEOutput;
    }

  }

  DleInternal::~DleInternal() {

  }

  void DleInternal::init() {

    pos_def_ = getOption("pos_def");
    error_unstable_ = getOption("error_unstable");
    eps_unstable_ = getOption("eps_unstable");
    
    A_ = st_[Dle_STRUCT_A];
    V_ = st_[Dle_STRUCT_V];
    C_ = st_[Dle_STRUCT_C];
    
    with_C_ = true;
    if (C_.isNull()) {
      C_ = Sparsity::sparse(0,0);
      st_[Dle_STRUCT_C] = C_;
      with_C_ = false;
    }
    
    int n = A_.size1();

    casadi_assert_message(V_.isSymmetric(), "V must be symmetric but got "
                          << V_.dimString() << ".");
    
    casadi_assert_message(A_.size1()==A_.size2(), "A must be square but got "
                          << A_.dimString() << ".");
    
    int m = with_C_? V_.size1(): n;
    
    if (with_C_) { 
      casadi_assert_message(n==C_.size1(), "Number of rows in C ("
                            << C_.size1() << ") must match dimension of square A ("
                            << n << ")" << ".");
      casadi_assert_message(m==C_.size2(), "Number of columns in C ("
                            << C_.size2() << ") must match dimension of symmetric V ("
                            << m << ")" << ".");
    } else {
      casadi_assert_message(A_.size1()==V_.size1(), "First dimension of A ("
                            << A_.size1() << ") must match dimension of symmetric V ("
                            << V_.size1() << ")" << ".");
    }
    // Allocate inputs
    setNumInputs(1+nrhs_);

    input(0)  = DMatrix::zeros(A_);

    for (int i=0;i<nrhs_;++i) {
      input(1+i)  = DMatrix::zeros(V_);
    }

    // Compute output sparsity by Schmidt iteration with frequency doubling
    Sparsity C = C_;
    Sparsity A = A_;
    Sparsity V = V_;
    
    Sparsity P = mul(mul(C,V),C.T());
    Sparsity Pprev = Sparsity::sparse(n,n);
    
    while (Pprev.size()!=P.size()) {
      Pprev = P;
      C = horzcat(C,mul(A,C));
      V = blkdiag(V,V);
      A = mul(A,A);
      P = mul(mul(C,V),C.T());
    }

    setNumOutputs(nrhs_);
    for (int i=0;i<nrhs_;++i) {
      output(i) = DMatrix::zeros(P);
    }

    FunctionInternal::init();

  }

  void DleInternal::deepCopyMembers(std::map<SharedObjectNode*, SharedObject>& already_copied) {
    FunctionInternal::deepCopyMembers(already_copied);
  }

  std::map<std::string, DleInternal::Plugin> DleInternal::solvers_;

  const std::string DleInternal::infix_ = "dlesolver";


} // namespace casadi


