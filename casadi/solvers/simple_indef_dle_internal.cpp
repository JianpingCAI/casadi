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

#include "simple_indef_dle_internal.hpp"
#include <cassert>
#include "../core/std_vector_tools.hpp"
#include "../core/matrix/matrix_tools.hpp"
#include "../core/mx/mx_tools.hpp"
#include "../core/sx/sx_tools.hpp"
#include "../core/function/mx_function.hpp"
#include "../core/function/sx_function.hpp"

#include <numeric>

INPUTSCHEME(DLEInput)
OUTPUTSCHEME(DLEOutput)

using namespace std;
namespace casadi {

  extern "C"
  int CASADI_DLESOLVER_SIMPLE_EXPORT
  casadi_register_dlesolver_simple(DleInternal::Plugin* plugin) {
    plugin->creator = SimpleIndefDleInternal::creator;
    plugin->name = "simple";
    plugin->doc = SimpleIndefDleInternal::meta_doc.c_str();
    plugin->version = 20;
    return 0;
  }

  extern "C"
  void CASADI_DLESOLVER_SIMPLE_EXPORT casadi_load_dlesolver_simple() {
    DleInternal::registerPlugin(casadi_register_dlesolver_simple);
  }

  SimpleIndefDleInternal::SimpleIndefDleInternal(
      const DleStructure& st) : DleInternal(st) {

    // set default options
    setOption("name", "unnamed_simple_indef_dle_solver"); // name of the function

    addOption("compressed_solve",         OT_BOOLEAN, true,
              "When a system with sparse rhs arises, compress to"
              "a smaller system with dense rhs.");
    addOption("linear_solver",            OT_STRING, GenericType(),
              "User-defined linear solver class. Needed for sensitivities.");
    addOption("linear_solver_options",    OT_DICTIONARY,   GenericType(),
              "Options to be passed to the linear solver.");

  }

  SimpleIndefDleInternal::~SimpleIndefDleInternal() {

  }

  void SimpleIndefDleInternal::init() {

    DleInternal::init();

    casadi_assert_message(!pos_def_,
      "pos_def option set to True: Solver only handles the indefinite case.");

    n_ = A_.size1();

    MX As = MX::sym("A", A_);
    MX Vs = MX::sym("V", V_);
    MX Cs = MX::sym("C", C_);

    MX Vss = (Vs+Vs.T())/2;
    if (with_C_) Vss = mul(mul(Cs,Vss),Cs.T());

    MX A_total = DMatrix::eye(n_*n_) - kron(As,As);
    
    MX Pf = solve(A_total, vec(Vss), getOption("linear_solver"));
    
    std::vector<MX> v_in;
    v_in.push_back(As);
    v_in.push_back(Vs);
    v_in.push_back(Cs);
    f_ = MXFunction(v_in, reshape(Pf,n_,n_)(output().sparsity()));
    
    f_.setInputScheme(SCHEME_DLEInput);
    f_.setOutputScheme(SCHEME_DLEOutput);
    f_.init();
    
    
    std::cout << "P" << output().dimString() << std::endl;
    std::cout << "solve" << f_.output().dimString() << std::endl;
    
  }



  void SimpleIndefDleInternal::evaluate() {
    for (int i=0;i<getNumInputs();++i) {
      std::copy(input(i).begin(), input(i).end(), f_.input(i).begin());
    }
    f_.evaluate();
    for (int i=0;i<getNumOutputs();++i) {
      std::copy(f_.output(i).begin(), f_.output(i).end(), output(i).begin());
    }
  }

  Function SimpleIndefDleInternal::getDerivative(int nfwd, int nadj) {
    return f_.derivative(nfwd, nadj);
  }


  void SimpleIndefDleInternal::deepCopyMembers(
      std::map<SharedObjectNode*, SharedObject>& already_copied) {
    DleInternal::deepCopyMembers(already_copied);
  }

  SimpleIndefDleInternal* SimpleIndefDleInternal::clone() const {
    // Return a deep copy
    SimpleIndefDleInternal* node = new SimpleIndefDleInternal(st_);
    node->setOption(dictionary());
    return node;
  }


} // namespace casadi


