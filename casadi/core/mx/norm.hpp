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


#ifndef CASADI_NORM_HPP
#define CASADI_NORM_HPP

#include "mx_node.hpp"

/// \cond INTERNAL
namespace casadi {

  /** \brief Matrix and vector norms

      \author Joel Andersson
      \date 2010-2013
  */
  class CASADI_EXPORT Norm : public MXNode {
  public:

    /** \brief  Constructor */
    explicit Norm(const MX& x);

    /** \brief  Destructor */
    virtual ~Norm() {}
  };

  /** \brief Represents a Frobenius norm
      \author Joel Andersson
      \date 2010-2013
  */
  class CASADI_EXPORT NormF : public Norm {
  public:

    /** \brief  Constructor */
    explicit NormF(const MX& x) : Norm(x) {}

    /** \brief  Destructor */
    virtual ~NormF() {}

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

    /** \brief  Evaluate the function symbolically (MX) */
    virtual void eval(const cpv_MX& input, const pv_MX& output);

    /** \brief Calculate forward mode directional derivatives */
    virtual void evalFwd(const std::vector<cpv_MX>& fwdSeed, const std::vector<pv_MX>& fwdSens);

    /** \brief Calculate reverse mode directional derivatives */
    virtual void evalAdj(const std::vector<pv_MX>& adjSeed, const std::vector<pv_MX>& adjSens);

    /** \brief Generate code for the operation */
    void generate(std::ostream &stream, const std::vector<int>& arg,
                           const std::vector<int>& res, CodeGenerator& gen) const;

    /** \brief  Clone function */
    virtual NormF* clone() const { return new NormF(*this);}

    /** \brief  Print a part of the expression */
    virtual void printPart(std::ostream &stream, int part) const;

    /** \brief Get the operation */
    virtual int getOp() const { return OP_NORMF;}
  };

  /** \brief Represents a 2-norm (spectral norm)
      \author Joel Andersson
      \date 2010-2013
  */
  class CASADI_EXPORT Norm2 : public Norm {
  public:

    /** \brief  Constructor */
    explicit Norm2(const MX& x): Norm(x) {}

    /** \brief  Destructor */
    virtual ~Norm2() {}

    /** \brief  Clone function */
    virtual Norm2* clone() const { return new Norm2(*this);}

    /** \brief  Print a part of the expression */
    virtual void printPart(std::ostream &stream, int part) const;

    /** \brief Get the operation */
    virtual int getOp() const { return OP_NORM2;}
  };

  /** \brief 1-norm
      \author Joel Andersson
      \date 2010-2013
  */
  class CASADI_EXPORT Norm1 : public Norm {
  public:

    /** \brief  Constructor */
    Norm1(const MX& x) : Norm(x) {}

    /** \brief  Destructor */
    virtual ~Norm1() {}

    /** \brief  Clone function */
    virtual Norm1* clone() const { return new Norm1(*this);}

    /** \brief  Print a part of the expression */
    virtual void printPart(std::ostream &stream, int part) const;

    /** \brief Get the operation */
    virtual int getOp() const { return OP_NORM1;}
  };

  /** \brief Represents an infinity-norm operation on a MX
      \author Joel Andersson
      \date 2010
  */
  class CASADI_EXPORT NormInf : public Norm {
  public:

    /** \brief  Constructor */
    NormInf(const MX& x) : Norm(x) {}

    /** \brief  Destructor */
    virtual ~NormInf() {}

    /** \brief  Clone function */
    virtual NormInf* clone() const { return new NormInf(*this);}

    /** \brief  Print a part of the expression */
    virtual void printPart(std::ostream &stream, int part) const;

    /** \brief Get the operation */
    virtual int getOp() const { return OP_NORMINF;}
  };

} // namespace casadi

/// \endcond

#endif // CASADI_NORM_HPP
