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


#include "inverse.hpp"
#include "../matrix/matrix_tools.hpp"
#include "mx_tools.hpp"
#include "../sx/sx_tools.hpp"

using namespace std;

namespace casadi {

  Inverse::Inverse(const MX& x) {
    casadi_assert_message(x.size1()==x.size2(),
                          "Inverse: matrix must be square, but you supllied " << x.dimString());
    setDependencies(x);
    setSparsity(Sparsity::dense(x.size1(), x.size2()));
  }

  void Inverse::printPart(std::ostream &stream, int part) const {
    if (part==0) {
      stream << "inv(";
    } else {
      stream << ")";
    }
  }

  void Inverse::eval(const cpv_MX& input, const pv_MX& output) {
    *output[0] = inv(*input[0]);
  }

  void Inverse::evalFwd(const std::vector<cpv_MX>& fwdSeed, const std::vector<pv_MX>& fwdSens) {
    MX inv_X = shared_from_this<MX>();
    for (int d=0; d<fwdSens.size(); ++d) {
      *fwdSens[d][0] = -mul(inv_X, mul(*fwdSeed[d][0], inv_X));
    }
  }

  void Inverse::evalAdj(const std::vector<pv_MX>& adjSeed, const std::vector<pv_MX>& adjSens) {
    MX inv_X = shared_from_this<MX>();
    MX trans_inv_X = inv_X.T();
    for (int d=0; d<adjSeed.size(); ++d) {
      adjSens[d][0]->addToSum(-mul(trans_inv_X, mul(*adjSeed[d][0], trans_inv_X)));
      *adjSeed[d][0] = MX();
    }
  }

} // namespace casadi
