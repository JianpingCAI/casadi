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


#ifndef CASADI_MULTIPLICATION_CPP
#define CASADI_MULTIPLICATION_CPP

#include "multiplication.hpp"
#include "../matrix/matrix_tools.hpp"
#include "mx_tools.hpp"
#include "../std_vector_tools.hpp"
#include "../function/function_internal.hpp"

using namespace std;

namespace casadi {

  Multiplication::Multiplication(const MX& z, const MX& x, const MX& y) {
    casadi_assert_message(
      x.size2() == y.size1() && x.size1() == z.size1() && y.size2() == z.size2(),
      "Multiplication::Multiplication: dimension mismatch. Attempting to multiply "
      << x.dimString() << " with " << y.dimString()
      << " and add the result to " << z.dimString());

    setDependencies(z, x, y);
    setSparsity(z.sparsity());
  }

  void Multiplication::printPart(std::ostream &stream, int part) const {
    if (part==0) {
      stream << "(";
    } else if (part==1) {
      stream << "+mul(";
    } else if (part==2) {
      stream << ", ";
    } else {
      stream << "))";
    }
  }

  void Multiplication::evaluateD(const double* const* input, double** output,
                                 int* itmp, double* rtmp) {
    evaluateGen<double>(input, output, itmp, rtmp);
  }

  void Multiplication::evaluateSX(const SXElement* const* input, SXElement** output,
                                  int* itmp, SXElement* rtmp) {
    evaluateGen<SXElement>(input, output, itmp, rtmp);
  }

  template<typename T>
  void Multiplication::evaluateGen(const T* const* input, T** output, int* itmp, T* rtmp) {
    if (input[0]!=output[0]) {
      copy(input[0], input[0]+dep(0).nnz(), output[0]);
    }
    casadi_mm_sparse(input[1], dep(1).sparsity(),
                     input[2], dep(2).sparsity(),
                     output[0], sparsity(), rtmp);
  }

  void Multiplication::evalFwd(const MXPtrVV& fwdSeed, MXPtrVV& fwdSens) {
    for (int d=0; d<fwdSens.size(); ++d) {
      *fwdSens[d][0] = *fwdSeed[d][0]
        + mul(dep(1), *fwdSeed[d][2], MX::zeros(dep(0).sparsity()))
        + mul(*fwdSeed[d][1], dep(2), MX::zeros(dep(0).sparsity()));
    }
  }

  void Multiplication::evalAdj(MXPtrVV& adjSeed, MXPtrVV& adjSens) {
    for (int d=0; d<adjSeed.size(); ++d) {
      adjSens[d][1]->addToSum(mul(*adjSeed[d][0], dep(2).T(), MX::zeros(dep(1).sparsity())));
      adjSens[d][2]->addToSum(mul(dep(1).T(), *adjSeed[d][0], MX::zeros(dep(2).sparsity())));
      if (adjSeed[d][0]!=adjSens[d][0]) {
        adjSens[d][0]->addToSum(*adjSeed[d][0]);
        *adjSeed[d][0] = MX();
      }
    }
  }

  void Multiplication::eval(const MXPtrV& input, MXPtrV& output) {
    *output[0] = mul(*input[1], *input[2], *input[0]);
  }

  void Multiplication::propagateSparsity(double** input, double** output,
                                         int* itmp, bvec_t* rtmp, bool fwd) {
    bvec_t *zd = reinterpret_cast<bvec_t*>(input[0]);
    bvec_t *rd = reinterpret_cast<bvec_t*>(output[0]);
    const size_t n = this->nnz();
    if (fwd) {
      if (zd!=rd) copy(zd, zd+n, rd);
      Sparsity::mul_sparsityF(reinterpret_cast<bvec_t*>(input[1]),
                              dep(1).sparsity(),
                              reinterpret_cast<bvec_t*>(input[2]),
                              dep(2).sparsity(),
                              reinterpret_cast<bvec_t*>(input[0]),
                              dep(0).sparsity(), rtmp);
    } else {
      Sparsity::mul_sparsityR(reinterpret_cast<bvec_t*>(input[1]),
                              dep(1).sparsity(),
                              reinterpret_cast<bvec_t*>(input[2]),
                              dep(2).sparsity(),
                              reinterpret_cast<bvec_t*>(output[0]),
                              sparsity(), rtmp);
      if (zd!=rd) {
        for (int i=0; i<n; ++i) {
          zd[i] |= rd[i];
          rd[i] = bvec_t(0);
        }
      }
    }
  }

  void Multiplication::generateOperation(std::ostream &stream,
                                         const std::vector<std::string>& arg,
                                         const std::vector<std::string>& res,
                                         CodeGenerator& gen) const {
    // Check if inplace
    bool inplace = arg.at(0).compare(res.front())==0;

    // Copy first argument if not inplace
    if (!inplace) {
      stream << "  for (i=0; i<" << this->nnz() << "; ++i) " << res.front()
             << "[i]=" << arg.at(0) << "[i];" << endl;
    }

    // Perform sparse matrix multiplication
    gen.addAuxiliary(CodeGenerator::AUX_MM_SPARSE);
    stream << "  casadi_mm_sparse(";
    stream << arg.at(1) << ", s" << gen.addSparsity(dep(1).sparsity()) << ", ";
    stream << arg.at(2) << ", s" << gen.addSparsity(dep(2).sparsity()) << ", ";
    stream << res.front() << ", s" << gen.addSparsity(sparsity()) << ", rrr);" << endl;
  }

  void DenseMultiplication::generateOperation(std::ostream &stream,
                                              const std::vector<std::string>& arg,
                                              const std::vector<std::string>& res,
                                              CodeGenerator& gen) const {
    // Check if inplace
    bool inplace = arg.at(0).compare(res.front())==0;

    // Copy first argument if not inplace
    if (!inplace) {
      stream << "  for (i=0; i<" << this->nnz() << "; ++i) " << res.front()
             << "[i]=" << arg.at(0) << "[i];" << endl;
    }

    int nrow_x = this->dep(1).size1();
    int nrow_y = this->dep(2).size1();
    int ncol_y = this->dep(2).size2();
    stream << "  for (i=0, rr=" << res.front() <<"; i<" << ncol_y << "; ++i)";
    stream << " for (j=0; j<" << nrow_x << "; ++j, ++rr)";
    stream << " for (k=0, ss=" << arg.at(1) << "+j, tt="
           << arg.at(2) << "+i*" << nrow_y << "; k<" << nrow_y << "; ++k)";
    stream << " *rr += ss[k*" << nrow_x << "]**tt++;" << endl;
  }

} // namespace casadi

#endif // CASADI_MULTIPLICATION_CPP
