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


#include "split.hpp"
#include "../std_vector_tools.hpp"
#include "../matrix/matrix_tools.hpp"
#include "mx_tools.hpp"
#include "../sx/sx_tools.hpp"
#include "../function/sx_function.hpp"
#include "../casadi_options.hpp"

using namespace std;

namespace casadi {

  Split::Split(const MX& x, const std::vector<int>& offset) : offset_(offset) {
    setDependencies(x);
    setSparsity(Sparsity::scalar());
  }

  Split::~Split() {
  }

  void Split::evalD(const cpv_double& input, const pv_double& output,
                        int* itmp, double* rtmp) {
    evalGen<double>(input, output, itmp, rtmp);
  }

  void Split::evalSX(const cpv_SXElement& input, const pv_SXElement& output,
                         int* itmp, SXElement* rtmp) {
    evalGen<SXElement>(input, output, itmp, rtmp);
  }

  template<typename T>
  void Split::evalGen(const std::vector<const T*>& input,
                      const std::vector<T*>& output, int* itmp, T* rtmp) {
    // Number of derivatives
    int nx = offset_.size()-1;

    for (int i=0; i<nx; ++i) {
      int nz_first = offset_[i];
      int nz_last = offset_[i+1];
      if (output[i]!=0) {
        copy(input[0]+nz_first, input[0]+nz_last, output[i]);
      }
    }
  }

  void Split::spFwd(const cpv_bvec_t& arg,
                    const pv_bvec_t& res, int* itmp, bvec_t* rtmp) {
    int nx = offset_.size()-1;
    for (int i=0; i<nx; ++i) {
      if (res[i]!=0) {
        const bvec_t *arg_ptr = arg[0] + offset_[i];
        int n_i = sparsity(i).nnz();
        bvec_t *res_i_ptr = res[i];
        for (int k=0; k<n_i; ++k) {
          *res_i_ptr++ = *arg_ptr++;
        }
      }
    }
  }

  void Split::spAdj(const pv_bvec_t& arg,
                    const pv_bvec_t& res, int* itmp, bvec_t* rtmp) {
    int nx = offset_.size()-1;
    for (int i=0; i<nx; ++i) {
      if (res[i]!=0) {
        bvec_t *arg_ptr = arg[0] + offset_[i];
        int n_i = sparsity(i).nnz();
        bvec_t *res_i_ptr = res[i];
        for (int k=0; k<n_i; ++k) {
          *arg_ptr++ |= *res_i_ptr;
          *res_i_ptr++ = 0;
        }
      }
    }
  }

  void Split::generate(std::ostream &stream, const std::vector<int>& arg,
                       const std::vector<int>& res, CodeGenerator& gen) const {
    int nx = nout();
    for (int i=0; i<nx; ++i) {
      int nz_first = offset_[i];
      int nz_last = offset_[i+1];
      int nz = nz_last-nz_first;
      if (res[i]>=0) {
        stream << "  for (i=0, rr=" <<  gen.work(res[i]) << ", "
               << "cr=" << gen.work(arg[0]+nz_first) << "; i<" << nz << "; ++i) "
               << "*rr++ = *cr++;" << endl;
      }
    }
  }

  Horzsplit::Horzsplit(const MX& x, const std::vector<int>& offset) : Split(x, offset) {

    // Split up the sparsity pattern
    output_sparsity_ = horzsplit(x.sparsity(), offset_);

    // Have offset_ refer to the nonzero offsets instead of column offsets
    offset_.resize(1);
    for (std::vector<Sparsity>::const_iterator it=output_sparsity_.begin();
        it!=output_sparsity_.end();
        ++it) {
      offset_.push_back(offset_.back() + it->nnz());
    }
  }

  void Horzsplit::printPart(std::ostream &stream, int part) const {
    if (part==0) {
      stream << "horzsplit(";
    } else {
      stream << ")";
    }
  }

  void Horzsplit::eval(const cpv_MX& arg, const pv_MX& res) {
    int nx = offset_.size()-1;

    // Get column offsets
    vector<int> col_offset;
    col_offset.reserve(offset_.size());
    col_offset.push_back(0);
    for (std::vector<Sparsity>::const_iterator it=output_sparsity_.begin();
        it!=output_sparsity_.end();
        ++it) {
      col_offset.push_back(col_offset.back() + it->size2());
    }

    const MX& x = *arg[0];
    vector<MX> y = horzsplit(x, col_offset);
    for (int i=0; i<nx; ++i) {
      if (res[i]!=0) {
        *res[i] = y[i];
      }
    }
  }

  void Horzsplit::evalFwd(const std::vector<cpv_MX>& fwdSeed, const std::vector<pv_MX>& fwdSens) {
    int nfwd = fwdSens.size();
    int nx = offset_.size()-1;

    // Get column offsets
    vector<int> col_offset;
    col_offset.reserve(offset_.size());
    col_offset.push_back(0);
    for (std::vector<Sparsity>::const_iterator it=output_sparsity_.begin();
        it!=output_sparsity_.end();
        ++it) {
      col_offset.push_back(col_offset.back() + it->size2());
    }

    // Non-differentiated output and forward sensitivities
    for (int d=0; d<nfwd; ++d) {
      const cpv_MX& arg = fwdSeed[d];
      const pv_MX& res = fwdSens[d];
      const MX& x = *arg[0];
      vector<MX> y = horzsplit(x, col_offset);
      for (int i=0; i<nx; ++i) {
        if (res[i]!=0) {
          *res[i] = y[i];
        }
      }
    }
  }

  void Horzsplit::evalAdj(const std::vector<pv_MX>& adjSeed, const std::vector<pv_MX>& adjSens) {
    int nadj = adjSeed.size();
    int nx = offset_.size()-1;

    // Get column offsets
    vector<int> col_offset;
    col_offset.reserve(offset_.size());
    col_offset.push_back(0);
    for (std::vector<Sparsity>::const_iterator it=output_sparsity_.begin();
        it!=output_sparsity_.end();
        ++it) {
      col_offset.push_back(col_offset.back() + it->size2());
    }

    for (int d=0; d<nadj; ++d) {
      if (adjSens[d][0]!=0) {
        vector<MX> v;
        for (int i=0; i<nx; ++i) {
          MX* x_i = adjSeed[d][i];
          if (x_i!=0) {
            v.push_back(*x_i);
            *x_i = MX();
          } else {
            v.push_back(MX(output_sparsity_[i].shape()));
          }
        }
        adjSens[d][0]->addToSum(horzcat(v));
      }
    }
  }

  Diagsplit::Diagsplit(const MX& x,
    const std::vector<int>& offset1,
    const std::vector<int>& offset2) : Split(x, offset1) {

    // Split up the sparsity pattern
    output_sparsity_ = diagsplit(x.sparsity(), offset1, offset2);

    // Have offset_ refer to the nonzero offsets instead of column offsets
    offset_.resize(1);
    for (std::vector<Sparsity>::const_iterator it=output_sparsity_.begin();
        it!=output_sparsity_.end();
        ++it) {
      offset_.push_back(offset_.back() + it->nnz());
    }

    casadi_assert_message(offset_.back()==x.nnz(),
      "DiagSplit:: the presence of nonzeros outside the diagonal blocks in unsupported.");
  }

  void Diagsplit::printPart(std::ostream &stream, int part) const {
    if (part==0) {
      stream << "diagsplit(";
    } else {
      stream << ")";
    }
  }

  void Diagsplit::eval(const cpv_MX& arg, const pv_MX& res) {
    int nx = offset_.size()-1;

    // Get offsets
    vector<int> offset1;
    offset1.reserve(offset_.size());
    offset1.push_back(0);
    vector<int> offset2;
    offset2.reserve(offset_.size());
    offset2.push_back(0);
    for (std::vector<Sparsity>::const_iterator it=output_sparsity_.begin();
        it!=output_sparsity_.end();
        ++it) {
      offset1.push_back(offset1.back() + it->size1());
      offset2.push_back(offset2.back() + it->size2());
    }

    const MX& x = *arg[0];
    vector<MX> y = diagsplit(x, offset1, offset2);
    for (int i=0; i<nx; ++i) {
      if (res[i]!=0) {
        *res[i] = y[i];
      }
    }
  }

  void Diagsplit::evalFwd(const std::vector<cpv_MX>& fwdSeed, const std::vector<pv_MX>& fwdSens) {
    int nfwd = fwdSens.size();
    int nx = offset_.size()-1;

    // Get offsets
    vector<int> offset1;
    offset1.reserve(offset_.size());
    offset1.push_back(0);
    vector<int> offset2;
    offset2.reserve(offset_.size());
    offset2.push_back(0);
    for (std::vector<Sparsity>::const_iterator it=output_sparsity_.begin();
        it!=output_sparsity_.end();
        ++it) {
      offset1.push_back(offset1.back() + it->size1());
      offset2.push_back(offset2.back() + it->size2());
    }

    // Non-differentiated output and forward sensitivities
    for (int d=0; d<nfwd; ++d) {
      const cpv_MX& arg = fwdSeed[d];
      const pv_MX& res = fwdSens[d];
      const MX& x = *arg[0];
      vector<MX> y = diagsplit(x, offset1, offset2);
      for (int i=0; i<nx; ++i) {
        if (res[i]!=0) {
          *res[i] = y[i];
        }
      }
    }
  }

  void Diagsplit::evalAdj(const std::vector<pv_MX>& adjSeed, const std::vector<pv_MX>& adjSens) {
    int nadj = adjSens.size();
    int nx = offset_.size()-1;

    // Get offsets
    vector<int> offset1;
    offset1.reserve(offset_.size());
    offset1.push_back(0);
    vector<int> offset2;
    offset2.reserve(offset_.size());
    offset2.push_back(0);
    for (std::vector<Sparsity>::const_iterator it=output_sparsity_.begin();
        it!=output_sparsity_.end();
        ++it) {
      offset1.push_back(offset1.back() + it->size1());
      offset2.push_back(offset2.back() + it->size2());
    }

    for (int d=0; d<nadj; ++d) {
      if (adjSens[d][0]!=0) {
        vector<MX> v;
        for (int i=0; i<nx; ++i) {
          MX* x_i = adjSeed[d][i];
          if (x_i!=0) {
            v.push_back(*x_i);
            *x_i = MX();
          } else {
            v.push_back(MX(output_sparsity_[i].shape()));
          }
        }
        adjSens[d][0]->addToSum(diagcat(v));
      }
    }
  }

  Vertsplit::Vertsplit(const MX& x, const std::vector<int>& offset) : Split(x, offset) {

    // Split up the sparsity pattern
    output_sparsity_ = vertsplit(x.sparsity(), offset_);

    // Have offset_ refer to the nonzero offsets instead of column offsets
    offset_.resize(1);
    for (std::vector<Sparsity>::const_iterator it=output_sparsity_.begin();
        it!=output_sparsity_.end();
        ++it) {
      offset_.push_back(offset_.back() + it->nnz());
    }
  }

  void Vertsplit::printPart(std::ostream &stream, int part) const {
    if (part==0) {
      stream << "vertsplit(";
    } else {
      stream << ")";
    }
  }

  void Vertsplit::eval(const cpv_MX& arg, const pv_MX& res) {
    int nx = offset_.size()-1;

    // Get row offsets
    vector<int> row_offset;
    row_offset.reserve(offset_.size());
    row_offset.push_back(0);
    for (std::vector<Sparsity>::const_iterator it=output_sparsity_.begin();
        it!=output_sparsity_.end();
        ++it) {
      row_offset.push_back(row_offset.back() + it->size1());
    }

    const MX& x = *arg[0];
    vector<MX> y = vertsplit(x, row_offset);
    for (int i=0; i<nx; ++i) {
      if (res[i]!=0) {
        *res[i] = y[i];
      }
    }
  }

  void Vertsplit::evalFwd(const std::vector<cpv_MX>& fwdSeed, const std::vector<pv_MX>& fwdSens) {
    int nfwd = fwdSens.size();
    int nx = offset_.size()-1;

    // Get row offsets
    vector<int> row_offset;
    row_offset.reserve(offset_.size());
    row_offset.push_back(0);
    for (std::vector<Sparsity>::const_iterator it=output_sparsity_.begin();
        it!=output_sparsity_.end();
        ++it) {
      row_offset.push_back(row_offset.back() + it->size1());
    }

    for (int d=0; d<nfwd; ++d) {
      const cpv_MX& arg = fwdSeed[d];
      const pv_MX& res = fwdSens[d];
      const MX& x = *arg[0];
      vector<MX> y = vertsplit(x, row_offset);
      for (int i=0; i<nx; ++i) {
        if (res[i]!=0) {
          *res[i] = y[i];
        }
      }
    }
  }

  void Vertsplit::evalAdj(const std::vector<pv_MX>& adjSeed, const std::vector<pv_MX>& adjSens) {
    int nadj = adjSeed.size();
    int nx = offset_.size()-1;

    // Get row offsets
    vector<int> row_offset;
    row_offset.reserve(offset_.size());
    row_offset.push_back(0);
    for (std::vector<Sparsity>::const_iterator it=output_sparsity_.begin();
        it!=output_sparsity_.end();
        ++it) {
      row_offset.push_back(row_offset.back() + it->size1());
    }

    for (int d=0; d<nadj; ++d) {
      if (adjSens[d][0]!=0) {
        vector<MX> v;
        for (int i=0; i<nx; ++i) {
          MX* x_i = adjSeed[d][i];
          if (x_i!=0) {
            v.push_back(*x_i);
            *x_i = MX();
          } else {
            v.push_back(MX(output_sparsity_[i].shape()));
          }
        }
        adjSens[d][0]->addToSum(vertcat(v));
      }
    }
  }

  MX Horzsplit::getHorzcat(const std::vector<MX>& x) const {
    // Check x length
    if (x.size()!=nout()) {
      return MXNode::getHorzcat(x);
    }

    // Check x content
    for (int i=0; i<x.size(); ++i) {
      if (!(x[i]->isOutputNode() && x[i]->getFunctionOutput()==i && x[i]->dep().get()==this)) {
        return MXNode::getHorzcat(x);
      }
    }

    // OK if reached this point
    return dep();
  }

  MX Vertsplit::getVertcat(const std::vector<MX>& x) const {
    // Check x length
    if (x.size()!=nout()) {
      return MXNode::getVertcat(x);
    }

    // Check x content
    for (int i=0; i<x.size(); ++i) {
      if (!(x[i]->isOutputNode() && x[i]->getFunctionOutput()==i && x[i]->dep().get()==this)) {
        return MXNode::getVertcat(x);
      }
    }

    // OK if reached this point
    return dep();
  }

  MX Diagsplit::getDiagcat(const std::vector<MX>& x) const {
    // Check x length
    if (x.size()!=nout()) {
      return MXNode::getDiagcat(x);
    }

    // Check x content
    for (int i=0; i<x.size(); ++i) {
      if (!(x[i]->isOutputNode() && x[i]->getFunctionOutput()==i && x[i]->dep().get()==this)) {
        return MXNode::getDiagcat(x);
      }
    }

    // OK if reached this point
    return dep();
  }

} // namespace casadi
