//******************************************************************************
// Copyright (c) 2001-2002 Tweak Inc. All rights reserved.
//
// SPDX-License-Identifier: Apache-2.0
//
//******************************************************************************
#ifndef _TwkMathExc_h_
#define _TwkMathExc_h_

#include <stdexcept>
#include <string>

// Declare a named exception class derived from BASE_TYPE.
// Each constructor forwards to BASE_TYPE; the default uses TAG as the message.
#define TWK_EXC_DECLARE(EXC_TYPE, BASE_TYPE, TAG)                                                  \
    class EXC_TYPE : public BASE_TYPE                                                              \
    {                                                                                              \
      public:                                                                                      \
        EXC_TYPE()                                                                                 \
            : BASE_TYPE(TAG)                                                                       \
        {}                                                                                         \
        EXC_TYPE(const char* s)                                                                    \
            : BASE_TYPE(s)                                                                         \
        {}                                                                                         \
        EXC_TYPE(const std::string& s)                                                             \
            : BASE_TYPE(s)                                                                         \
        {}                                                                                         \
    };

// Throw EXC_TYPE with the given message string.
#define TWK_EXC_THROW_WHAT(EXC_TYPE, EXC_WHAT) throw EXC_TYPE(EXC_WHAT)

namespace TwkMath {

TWK_EXC_DECLARE(MathExc, std::runtime_error, "math exception")
TWK_EXC_DECLARE(SingularMatrixExc, MathExc, "singular matrix")
TWK_EXC_DECLARE(BadFrustumExc, MathExc, "bad frustum")
TWK_EXC_DECLARE(MatrixNotAffine, MathExc, "matrix not affine")
TWK_EXC_DECLARE(EigenExc, MathExc, "eigen exception")
TWK_EXC_DECLARE(ZeroScaleExc, MathExc, "zero scale matrix")

} // namespace TwkMath

#endif // _TwkMathExc_h_
