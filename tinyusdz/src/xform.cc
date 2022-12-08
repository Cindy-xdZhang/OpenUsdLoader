// SPDX-License-Identifier: MIT
// Copyright 2022 - Present, Syoyo Fujita.

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#endif

#include "external/linalg.h"

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#include "math-util.inc"
#include "pprinter.hh"
#include "prim-types.hh"
#include "tiny-format.hh"
#include "value-types.hh"
#include "xform.hh"

namespace tinyusdz {

using matrix44d = linalg::aliases::double4x4;
using matrix33d = linalg::aliases::double3x3;
using matrix22d = linalg::aliases::double2x2;
using double3x3 = linalg::aliases::double3x3;

// linalg quat: (x, y, z, w)

value::matrix3d to_matrix3x3(const value::quath &q) {
  double3x3 m33 = linalg::qmat<double>(
      {double(half_to_float(q.imag[0])), double(half_to_float(q.imag[1])),
       double(half_to_float(q.imag[2])), double(half_to_float(q.real))});

  value::matrix3d m;
  Identity(&m);

  memcpy(m.m, &m33[0][0], sizeof(double) * 3 * 3);

  return m;
}

value::matrix3d to_matrix3x3(const value::quatf &q) {
  double3x3 m33 = linalg::qmat<double>({double(q.imag[0]), double(q.imag[1]),
                                        double(q.imag[2]), double(q.real)});

  value::matrix3d m;
  Identity(&m);

  memcpy(m.m, &m33[0][0], sizeof(double) * 3 * 3);

  return m;
}

value::matrix3d to_matrix3x3(const value::quatd &q) {
  double3x3 m33 =
      linalg::qmat<double>({q.imag[0], q.imag[1], q.imag[2], q.real});

  value::matrix3d m;
  Identity(&m);

  memcpy(m.m, &m33[0][0], sizeof(double) * 3 * 3);

  return m;
}

value::matrix4d to_matrix(const value::matrix3d &m33,
                          const value::double3 &tx) {
  value::matrix4d m;
  Identity(&m);

  m.m[0][0] = m33.m[0][0];
  m.m[0][1] = m33.m[0][1];
  m.m[0][2] = m33.m[0][2];
  m.m[1][0] = m33.m[1][0];
  m.m[1][1] = m33.m[1][1];
  m.m[1][2] = m33.m[1][2];
  m.m[2][0] = m33.m[2][0];
  m.m[2][1] = m33.m[2][1];
  m.m[2][2] = m33.m[2][2];

  m.m[3][0] = tx[0];
  m.m[3][1] = tx[1];
  m.m[3][2] = tx[2];

  return m;
}

value::matrix3d to_matrix3x3(const value::matrix4d &m44, value::double3 *tx) {
  value::matrix3d m;
  Identity(&m);

  m.m[0][0] = m44.m[0][0];
  m.m[0][1] = m44.m[0][1];
  m.m[0][2] = m44.m[0][2];
  m.m[1][0] = m44.m[1][0];
  m.m[1][1] = m44.m[1][1];
  m.m[1][2] = m44.m[1][2];
  m.m[2][0] = m44.m[2][0];
  m.m[2][1] = m44.m[2][1];
  m.m[2][2] = m44.m[2][2];

  if (tx) {
    (*tx)[0] = m44.m[3][0];
    (*tx)[1] = m44.m[3][1];
    (*tx)[2] = m44.m[3][2];
  }

  return m;
}

value::matrix4d to_matrix(const value::quath &q) {
  // using double4 = linalg::aliases::double4;

  double3x3 m33 = linalg::qmat<double>(
      {double(half_to_float(q.imag[0])), double(half_to_float(q.imag[1])),
       double(half_to_float(q.imag[2])), double(half_to_float(q.real))});

  value::matrix4d m;
  Identity(&m);

  m.m[0][0] = m33[0][0];
  m.m[0][1] = m33[0][1];
  m.m[0][2] = m33[0][2];
  m.m[1][0] = m33[1][0];
  m.m[1][1] = m33[1][1];
  m.m[1][2] = m33[1][2];
  m.m[2][0] = m33[2][0];
  m.m[2][1] = m33[2][1];
  m.m[2][2] = m33[2][2];

  return m;
}

value::matrix4d to_matrix(const value::quatf &q) {
  double3x3 m33 = linalg::qmat<double>({double(q.imag[0]), double(q.imag[1]),
                                        double(q.imag[2]), double(q.real)});

  value::matrix4d m;
  Identity(&m);

  m.m[0][0] = m33[0][0];
  m.m[0][1] = m33[0][1];
  m.m[0][2] = m33[0][2];
  m.m[1][0] = m33[1][0];
  m.m[1][1] = m33[1][1];
  m.m[1][2] = m33[1][2];
  m.m[2][0] = m33[2][0];
  m.m[2][1] = m33[2][1];
  m.m[2][2] = m33[2][2];

  return m;
}

value::matrix4d to_matrix(const value::quatd &q) {
  double3x3 m33 =
      linalg::qmat<double>({q.imag[0], q.imag[1], q.imag[2], q.real});

  value::matrix4d m;
  Identity(&m);

  m.m[0][0] = m33[0][0];
  m.m[0][1] = m33[0][1];
  m.m[0][2] = m33[0][2];
  m.m[1][0] = m33[1][0];
  m.m[1][1] = m33[1][1];
  m.m[1][2] = m33[1][2];
  m.m[2][0] = m33[2][0];
  m.m[2][1] = m33[2][1];
  m.m[2][2] = m33[2][2];

  return m;
}

value::matrix4d inverse(const value::matrix4d &_m) {
  matrix44d m;
  // memory layout is same
  memcpy(&m[0][0], _m.m, sizeof(double) * 4 * 4);

  matrix44d inv_m = linalg::inverse(m);

  value::matrix4d outm;

  memcpy(outm.m, &inv_m[0][0], sizeof(double) * 4 * 4);

  return outm;
}

value::matrix3d inverse(const value::matrix3d &_m) {
  matrix33d m;
  // memory layout is same
  memcpy(&m[0][0], _m.m, sizeof(double) * 3 * 3);

  matrix33d inv_m = linalg::inverse(m);

  value::matrix3d outm;

  memcpy(outm.m, &inv_m[0][0], sizeof(double) * 3 * 3);

  return outm;
}

double determinant(const value::matrix4d &_m) {
  matrix44d m;
  // memory layout is same
  memcpy(&m[0][0], _m.m, sizeof(double) * 4 * 4);

  double det = linalg::determinant(m);

  return det;
}

double determinant(const value::matrix3d &_m) {
  matrix33d m;
  // memory layout is same
  memcpy(&m[0][0], _m.m, sizeof(double) * 3 * 3);

  double det = linalg::determinant(m);

  return det;
}

bool inverse(const value::matrix4d &_m, value::matrix4d &inv_m) {
  double det = determinant(_m);

  // 1e-9 comes from pxrUSD
  // determinant should be positive(absolute), but take a fabs() just in case.
  if (std::fabs(det) < 1e-9) {
    return false;
  }

  inv_m = inverse(_m);
  return true;
}

bool inverse(const value::matrix3d &_m, value::matrix3d &inv_m) {
  double det = determinant(_m);

  // 1e-9 comes from pxrUSD
  // determinant should be positive(absolute), but take a fabs() just in case.
  if (std::fabs(det) < 1e-9) {
    return false;
  }

  inv_m = inverse(_m);
  return true;
}

value::matrix2d transpose(const value::matrix2d &_m) {
  matrix22d m;
  matrix22d tm;
  // memory layout is same
  memcpy(&m[0][0], _m.m, sizeof(double) * 2 * 2);
  tm = linalg::transpose(m);

  value::matrix2d dst;

  // memory layout is same
  memcpy(&dst.m[0][0], &tm[0][0], sizeof(double) * 2 * 2);

  return dst;
}

value::matrix3d transpose(const value::matrix3d &_m) {
  matrix33d m;
  matrix33d tm;
  // memory layout is same
  memcpy(&m[0][0], _m.m, sizeof(double) * 3 * 3);
  tm = linalg::transpose(m);

  value::matrix3d dst;

  // memory layout is same
  memcpy(&dst.m[0][0], &tm[0][0], sizeof(double) * 3 * 3);

  return dst;
}

value::matrix4d transpose(const value::matrix4d &_m) {
  matrix44d m;
  matrix44d tm;
  // memory layout is same
  memcpy(&m[0][0], _m.m, sizeof(double) * 4 * 4);
  tm = linalg::transpose(m);

  value::matrix4d dst;

  // memory layout is same
  memcpy(&dst.m[0][0], &tm[0][0], sizeof(double) * 4 * 4);

  return dst;
}

value::float4 matmul(const value::matrix4d &m, const value::float4 &p) {
  return value::MultV<value::matrix4d, value::float4, float, 4>(m, p);
}

value::double4 matmul(const value::matrix4d &m, const value::double4 &p) {
  return value::MultV<value::matrix4d, value::double4, double, 4>(m, p);
}

namespace {

///
/// Xform evaluation with method chain style.
///
class XformEvaluator {
 public:
  XformEvaluator() { Identity(&m); }

  XformEvaluator &RotateX(const double angle) {  // in degrees

    double rad = math::radian(angle);

    value::matrix4d rm;

    rm.m[1][1] = std::cos(rad);
    rm.m[1][2] = std::sin(rad);
    rm.m[2][1] = -std::sin(rad);
    rm.m[2][2] = std::cos(rad);

    m = value::Mult<value::matrix4d, double, 4>(rm, m);

    return (*this);
  }

  XformEvaluator &RotateY(const double angle) {  // in degrees

    double rad = math::radian(angle);

    value::matrix4d rm;

    rm.m[0][0] = std::cos(rad);
    rm.m[0][2] = -std::sin(rad);
    rm.m[2][0] = std::sin(rad);
    rm.m[2][2] = std::cos(rad);

    m = value::Mult<value::matrix4d, double, 4>(rm, m);

    return (*this);
  }

  XformEvaluator &RotateZ(const double angle) {  // in degrees

    double rad = math::radian(angle);

    value::matrix4d rm;

    rm.m[0][0] = std::cos(rad);
    rm.m[0][1] = std::sin(rad);
    rm.m[1][0] = -std::sin(rad);
    rm.m[1][1] = std::cos(rad);

    m = value::Mult<value::matrix4d, double, 4>(rm, m);

    return (*this);
  }

  std::string error() const { return err; }

  nonstd::expected<value::matrix4d, std::string> result() const {
    if (err.empty()) {
      return m;
    }

    return nonstd::make_unexpected(err);
  }

  std::string err;
  value::matrix4d m;
};

}  // namespace

bool Xformable::EvaluateXformOps(double t,
                                 value::TimeSampleInterpolationType tinterp,
                                 value::matrix4d *out_matrix,
                                 bool *resetXformStack,
                                 std::string *err) const {
  const auto RotateABC =
      [](const XformOp &x) -> nonstd::expected<value::matrix4d, std::string> {
    value::double3 v;
    if (auto h = x.get_value<value::half3>()) {
      v[0] = double(half_to_float(h.value()[0]));
      v[1] = double(half_to_float(h.value()[1]));
      v[2] = double(half_to_float(h.value()[2]));
    } else if (auto f = x.get_value<value::float3>()) {
      v[0] = double(f.value()[0]);
      v[1] = double(f.value()[1]);
      v[2] = double(f.value()[2]);
    } else if (auto d = x.get_value<value::double3>()) {
      v = d.value();
    } else {
      if (x.suffix.empty()) {
        return nonstd::make_unexpected(
            fmt::format("`{}` is not half3, float3 or double3 type.\n",
                        to_string(x.op_type)));
      } else {
        return nonstd::make_unexpected(
            fmt::format("`{}:{}` is not half3, float3 or double3 type.\n",
                        to_string(x.op_type), x.suffix));
      }
    }

    // invert input, and compute concatenated matrix
    // inv(ABC) = inv(A) x inv(B) x inv(C)
    // as done in pxrUSD.

    if (x.inverted) {
      v[0] = -v[0];
      v[1] = -v[1];
      v[2] = -v[2];
    }

    double xAngle = v[0];
    double yAngle = v[1];
    double zAngle = v[2];

    XformEvaluator eval;

    if (x.inverted) {
      if (x.op_type == XformOp::OpType::RotateXYZ) {
        eval.RotateZ(zAngle);
        eval.RotateY(yAngle);
        eval.RotateX(xAngle);
      } else if (x.op_type == XformOp::OpType::RotateXZY) {
        eval.RotateY(yAngle);
        eval.RotateZ(zAngle);
        eval.RotateX(xAngle);
      } else if (x.op_type == XformOp::OpType::RotateYXZ) {
        eval.RotateZ(zAngle);
        eval.RotateX(xAngle);
        eval.RotateY(yAngle);
      } else if (x.op_type == XformOp::OpType::RotateYZX) {
        eval.RotateX(xAngle);
        eval.RotateZ(zAngle);
        eval.RotateY(yAngle);
      } else if (x.op_type == XformOp::OpType::RotateZYX) {
        eval.RotateX(xAngle);
        eval.RotateY(yAngle);
        eval.RotateZ(zAngle);
      } else if (x.op_type == XformOp::OpType::RotateZXY) {
        eval.RotateY(yAngle);
        eval.RotateX(xAngle);
        eval.RotateZ(zAngle);
      } else {
        /// ???
        return nonstd::make_unexpected("[InternalError] RotateABC");
      }
    } else {
      if (x.op_type == XformOp::OpType::RotateXYZ) {
        eval.RotateX(xAngle);
        eval.RotateY(yAngle);
        eval.RotateZ(zAngle);
      } else if (x.op_type == XformOp::OpType::RotateXZY) {
        eval.RotateX(xAngle);
        eval.RotateZ(zAngle);
        eval.RotateY(yAngle);
      } else if (x.op_type == XformOp::OpType::RotateYXZ) {
        eval.RotateY(yAngle);
        eval.RotateX(xAngle);
        eval.RotateZ(zAngle);
      } else if (x.op_type == XformOp::OpType::RotateYZX) {
        eval.RotateY(yAngle);
        eval.RotateZ(zAngle);
        eval.RotateX(xAngle);
      } else if (x.op_type == XformOp::OpType::RotateZYX) {
        eval.RotateZ(zAngle);
        eval.RotateX(xAngle);
        eval.RotateY(yAngle);
      } else if (x.op_type == XformOp::OpType::RotateZXY) {
        eval.RotateZ(zAngle);
        eval.RotateX(xAngle);
        eval.RotateY(yAngle);
      } else {
        /// ???
        return nonstd::make_unexpected("[InternalError] RotateABC");
      }
    }

    return eval.result();
  };

  // Concat matrices
  //
  // Matrix concatenation ordering is its appearance order(right to left)
  // This is same with a notation in math equation: i.e,
  //
  // xformOpOrder = [A, B, C]
  //
  // M = A x B x C
  //
  // p' = A x B x C x p
  // (C is first applied to p)
  //
  value::matrix4d cm;
  Identity(&cm);

  for (size_t i = 0; i < xformOps.size(); i++) {
    const auto x = xformOps[i];

    value::matrix4d m;  // local matrix
    Identity(&m);

    if (x.is_timesamples()) {
      (void)t;
      (void)tinterp;
      if (err) {
        (*err) += "TODO: xformOp property with timeSamples.\n";
      }
      return false;
    }

    switch (x.op_type) {
      case XformOp::OpType::ResetXformStack: {
        if (i != 0) {
          if (err) {
            (*err) +=
                "!resetXformStack! should only appear at the first element of "
                "xformOps\n";
          }
          return false;
        }

        // Notify resetting previous(parent node's) matrices
        if (resetXformStack) {
          (*resetXformStack) = true;
        }
        break;
      }
      case XformOp::OpType::Transform: {
        if (auto sxf = x.get_value<value::matrix4f>()) {
          value::matrix4f mf = sxf.value();
          for (size_t j = 0; j < 4; j++) {
            for (size_t k = 0; k < 4; k++) {
              m.m[j][k] = double(mf.m[j][k]);
            }
          }
        } else if (auto sxd = x.get_value<value::matrix4d>()) {
          m = sxd.value();
        } else {
          if (err) {
            (*err) += "`xformOp:transform` is not matrix4f or matrix4d type.\n";
          }
          return false;
        }

        if (x.inverted) {
          // Singular check.
          // pxrUSD uses 1e-9
          double det = determinant(m);

          if (std::fabs(det) < 1e-9) {
            if (err) {
              if (x.suffix.empty()) {
                (*err) +=
                    "`xformOp:transform` is singular matrix and cannot be "
                    "inverted.\n";
              } else {
                (*err) += fmt::format(
                    "`xformOp:transform:{}` is singular matrix and cannot be "
                    "inverted.\n",
                    x.suffix);
              }
            }

            return false;
          }

          m = inverse(m);
        }

        break;
      }
      case XformOp::OpType::Scale: {
        double sx, sy, sz;

        if (auto sxh = x.get_value<value::half3>()) {
          sx = double(half_to_float(sxh.value()[0]));
          sy = double(half_to_float(sxh.value()[1]));
          sz = double(half_to_float(sxh.value()[2]));
        } else if (auto sxf = x.get_value<value::float3>()) {
          sx = double(sxf.value()[0]);
          sy = double(sxf.value()[1]);
          sz = double(sxf.value()[2]);
        } else if (auto sxd = x.get_value<value::double3>()) {
          sx = sxd.value()[0];
          sy = sxd.value()[1];
          sz = sxd.value()[2];
        } else {
          if (err) {
            (*err) += "`xformOp:scale` is not half3, float3 or double3 type.\n";
          }
          return false;
        }

        if (x.inverted) {
          // FIXME: Safe division
          sx = 1.0 / sx;
          sy = 1.0 / sy;
          sz = 1.0 / sz;
        }

        m.m[0][0] = sx;
        m.m[1][1] = sy;
        m.m[2][2] = sz;

        break;
      }
      case XformOp::OpType::Translate: {
        double tx, ty, tz;
        if (auto txh = x.get_value<value::half3>()) {
          tx = double(half_to_float(txh.value()[0]));
          ty = double(half_to_float(txh.value()[1]));
          tz = double(half_to_float(txh.value()[2]));
        } else if (auto txf = x.get_value<value::float3>()) {
          tx = double(txf.value()[0]);
          ty = double(txf.value()[1]);
          tz = double(txf.value()[2]);
        } else if (auto txd = x.get_value<value::double3>()) {
          tx = txd.value()[0];
          ty = txd.value()[1];
          tz = txd.value()[2];
        } else {
          if (err) {
            (*err) +=
                "`xformOp:translate` is not half3, float3 or double3 type.\n";
          }
          return false;
        }

        if (x.inverted) {
          tx = -tx;
          ty = -ty;
          tz = -tz;
        }

        m.m[3][0] = tx;
        m.m[3][1] = ty;
        m.m[3][2] = tz;

        break;
      }
      // FIXME: Validate ROTATE_X, _Y, _Z implementation
      case XformOp::OpType::RotateX: {
        double angle;  // in degrees
        if (auto h = x.get_value<value::half>()) {
          angle = double(half_to_float(h.value()));
        } else if (auto f = x.get_value<float>()) {
          angle = double(f.value());
        } else if (auto d = x.get_value<double>()) {
          angle = d.value();
        } else {
          if (err) {
            if (x.suffix.empty()) {
              (*err) +=
                  "`xformOp:rotateX` is not half, float or double type.\n";
            } else {
              (*err) += fmt::format(
                  "`xformOp:rotateX:{}` is not half, float or double type.\n",
                  x.suffix);
            }
          }
          return false;
        }

        XformEvaluator xe;
        xe.RotateX(angle);
        auto ret = xe.result();

        if (ret) {
          m = ret.value();
        } else {
          if (err) {
            (*err) += ret.error();
          }
          return false;
        }
        break;
      }
      case XformOp::OpType::RotateY: {
        double angle;  // in degrees
        if (auto h = x.get_value<value::half>()) {
          angle = double(half_to_float(h.value()));
        } else if (auto f = x.get_value<float>()) {
          angle = double(f.value());
        } else if (auto d = x.get_value<double>()) {
          angle = d.value();
        } else {
          if (err) {
            if (x.suffix.empty()) {
              (*err) +=
                  "`xformOp:rotateY` is not half, float or double type.\n";
            } else {
              (*err) += fmt::format(
                  "`xformOp:rotateY:{}` is not half, float or double type.\n",
                  x.suffix);
            }
          }
          return false;
        }

        XformEvaluator xe;
        xe.RotateY(angle);
        auto ret = xe.result();

        if (ret) {
          m = ret.value();
        } else {
          if (err) {
            (*err) += ret.error();
          }
          return false;
        }
        break;
      }
      case XformOp::OpType::RotateZ: {
        double angle;  // in degrees
        if (auto h = x.get_value<value::half>()) {
          angle = double(half_to_float(h.value()));
        } else if (auto f = x.get_value<float>()) {
          angle = double(f.value());
        } else if (auto d = x.get_value<double>()) {
          angle = d.value();
        } else {
          if (err) {
            if (x.suffix.empty()) {
              (*err) +=
                  "`xformOp:rotateZ` is not half, float or double type.\n";
            } else {
              (*err) += fmt::format(
                  "`xformOp:rotateZ:{}` is not half, float or double type.\n",
                  x.suffix);
            }
          }
          return false;
        }

        XformEvaluator xe;
        xe.RotateZ(angle);
        auto ret = xe.result();

        if (ret) {
          m = ret.value();
        } else {
          if (err) {
            (*err) += ret.error();
          }
          return false;
        }
        break;
      }
      case XformOp::OpType::Orient: {
        // value::quat stores elements in (x, y, z, w)
        // linalg::quat also stores elements in (x, y, z, w)

        value::matrix3d rm;
        if (auto h = x.get_value<value::quath>()) {
          rm = to_matrix3x3(h.value());
        } else if (auto f = x.get_value<value::quatf>()) {
          rm = to_matrix3x3(f.value());
        } else if (auto d = x.get_value<value::quatd>()) {
          rm = to_matrix3x3(d.value());
        } else {
          if (err) {
            if (x.suffix.empty()) {
              (*err) += "`xformOp:orient` is not quath, quatf or quatd type.\n";
            } else {
              (*err) += fmt::format(
                  "`xformOp:orient:{}` is not quath, quatf or quatd type.\n",
                  x.suffix);
            }
          }
          return false;
        }

        // FIXME: invert before getting matrix.
        if (x.inverted) {
          value::matrix3d inv_rm;
          if (inverse(rm, inv_rm)) {
          } else {
            if (err) {
              if (x.suffix.empty()) {
                (*err) +=
                    "`xformOp:orient` is singular and cannot be inverted.\n";
              } else {
                (*err) += fmt::format(
                    "`xformOp:orient:{}` is singular and cannot be inverted.\n",
                    x.suffix);
              }
            }
          }

          rm = inv_rm;
        }

        m = to_matrix(rm, {0.0, 0.0, 0.0});

        break;
      }

      case XformOp::OpType::RotateXYZ:
      case XformOp::OpType::RotateXZY:
      case XformOp::OpType::RotateYXZ:
      case XformOp::OpType::RotateYZX:
      case XformOp::OpType::RotateZXY:
      case XformOp::OpType::RotateZYX: {
        auto ret = RotateABC(x);

        if (!ret) {
          (*err) += ret.error();
          return false;
        }

        m = ret.value();
      }
    }

    cm = m * cm;  // row-major, so `m` fistly.
    // operator* is equivalent to
    // cm = value::Mult<value::matrix4d, double, 4>(m, cm);
  }

  (*out_matrix) = cm;

  return true;
}

std::vector<value::token> Xformable::xformOpOrder() const {
  std::vector<value::token> toks;

  for (size_t i = 0; i < xformOps.size(); i++) {
    std::string ss;

    auto xformOp = xformOps[i];

    if (xformOp.inverted) {
      ss += "!invert!";
    }
    ss += to_string(xformOp.op_type);
    if (!xformOp.suffix.empty()) {
      ss += ":" + xformOp.suffix;
    }

    toks.push_back(value::token(ss));
  }

  return toks;
}

value::float3 transform(const value::matrix4d &m, const value::float3 &p) {
  value::float3 tx{float(m.m[3][0]), float(m.m[3][1]), float(m.m[3][2])};
  // MatTy, VecTy, VecBaseTy, vecN
  return value::MultV<value::matrix4d, value::float3, float, 3>(m, p) + tx;
}

value::vector3f transform(const value::matrix4d &m, const value::vector3f &p) {
  value::vector3f tx{float(m.m[3][0]), float(m.m[3][1]), float(m.m[3][2])};
  return value::MultV<value::matrix4d, value::vector3f, float, 3>(m, p) + tx;
}

value::normal3f transform(const value::matrix4d &m, const value::normal3f &p) {
  value::normal3f tx{float(m.m[3][0]), float(m.m[3][1]), float(m.m[3][2])};
  return value::MultV<value::matrix4d, value::normal3f, float, 3>(m, p) + tx;
}
value::point3f transform(const value::matrix4d &m, const value::point3f &p) {
  value::point3f tx{float(m.m[3][0]), float(m.m[3][1]), float(m.m[3][2])};
  return value::MultV<value::matrix4d, value::point3f, float, 3>(m, p) + tx;
}
value::double3 transform(const value::matrix4d &m, const value::double3 &p) {
  value::double3 tx{m.m[3][0], m.m[3][1], m.m[3][2]};
  return value::MultV<value::matrix4d, value::double3, double, 3>(m, p) + tx;
}
value::vector3d transform(const value::matrix4d &m, const value::vector3d &p) {
  value::vector3d tx{m.m[3][0], m.m[3][1], m.m[3][2]};
  value::vector3d v =
      value::MultV<value::matrix4d, value::vector3d, double, 3>(m, p);
  v.x += tx.x;
  v.y += tx.y;
  v.z += tx.z;
  return v;
}
value::normal3d transform(const value::matrix4d &m, const value::normal3d &p) {
  value::normal3d tx{m.m[3][0], m.m[3][1], m.m[3][2]};
  value::normal3d v =
      value::MultV<value::matrix4d, value::normal3d, double, 3>(m, p);
  v.x += tx.x;
  v.y += tx.y;
  v.z += tx.z;
  return v;
}
value::point3d transform(const value::matrix4d &m, const value::point3d &p) {
  value::point3d tx{m.m[3][0], m.m[3][1], m.m[3][2]};
  value::point3d v =
      value::MultV<value::matrix4d, value::point3d, double, 3>(m, p);
  v.x += tx.x;
  v.y += tx.y;
  v.z += tx.z;
  return v;
}

value::float3 transform_dir(const value::matrix4d &m, const value::float3 &p) {
  // MatTy, VecTy, VecBaseTy, vecN
  return value::MultV<value::matrix4d, value::float3, float, 3>(m, p);
}

value::vector3f transform_dir(const value::matrix4d &m,
                              const value::vector3f &p) {
  return value::MultV<value::matrix4d, value::vector3f, float, 3>(m, p);
}

value::normal3f transform_dir(const value::matrix4d &m,
                              const value::normal3f &p) {
  return value::MultV<value::matrix4d, value::normal3f, float, 3>(m, p);
}
value::point3f transform_dir(const value::matrix4d &m,
                             const value::point3f &p) {
  return value::MultV<value::matrix4d, value::point3f, float, 3>(m, p);
}
value::double3 transform_dir(const value::matrix4d &m,
                             const value::double3 &p) {
  return value::MultV<value::matrix4d, value::double3, double, 3>(m, p);
}
value::vector3d transform_dir(const value::matrix4d &m,
                              const value::vector3d &p) {
  return value::MultV<value::matrix4d, value::vector3d, double, 3>(m, p);
}
value::normal3d transform_dir(const value::matrix4d &m,
                              const value::normal3d &p) {
  return value::MultV<value::matrix4d, value::normal3d, double, 3>(m, p);
}
value::point3d transform_dir(const value::matrix4d &m,
                             const value::point3d &p) {
  return value::MultV<value::matrix4d, value::point3d, double, 3>(m, p);
}

value::matrix4d upper_left_3x3_only(const value::matrix4d &m) {
  value::matrix4d dst;

  memcpy(dst.m, m.m, sizeof(double) * 4 * 4);

  dst.m[0][3] = 0.0;
  dst.m[0][3] = 0.0;
  dst.m[0][3] = 0.0;

  dst.m[3][0] = 0.0;
  dst.m[3][1] = 0.0;
  dst.m[3][2] = 0.0;

  dst.m[3][3] = 1.0;

  return dst;
}

// -------------------------------------------------------------------------------
// From pxrUSD
//

//
// Copyright 2016 Pixar
//
// Licensed under the Apache License, Version 2.0 (the "Apache License")
// with the following modification; you may not use this file except in
// compliance with the Apache License and the following modification to it:
// Section 6. Trademarks. is deleted and replaced with:
//
// 6. Trademarks. This License does not grant permission to use the trade
//    names, trademarks, service marks, or product names of the Licensor
//    and its affiliates, except as required to comply with Section 4(c) of
//    the License and to reproduce the content of the NOTICE file.
//
// You may obtain a copy of the Apache License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the Apache License with the above modification is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied. See the Apache License for the specific
// language governing permissions and limitations under the Apache License.
//
////////////////////////////////////////////////////////////////////////

/*
 * Given 3 basis vectors tx, ty, tz, orthogonalize and optionally normalize
 * them.
 *
 * This uses an iterative method that is very stable even when the vectors
 * are far from orthogonal (close to colinear).  The number of iterations
 * and thus the computation time does increase as the vectors become
 * close to colinear, however.
 *
 * If the iteration fails to converge, returns false with vectors as close to
 * orthogonal as possible.
 */
bool orthonormalize_basis(value::double3 &tx, value::double3 &ty,
                          value::double3 &tz, const bool normalize,
                          const double eps) {
  value::double3 ax, bx, cx, ay, by, cy, az, bz, cz;

  if (normalize) {
    tx = vnormalize(tx);
    ty = vnormalize(ty);
    tz = vnormalize(tz);
    ax = tx;
    ay = ty;
    az = tz;
  } else {
    ax = vnormalize(tx);
    ay = vnormalize(ty);
    az = vnormalize(tz);
  }

  /* Check for colinear vectors. This is not only a quick-out: the
   * error computation below will evaluate to zero if there's no change
   * after an iteration, which can happen either because we have a good
   * solution or because the vectors are colinear.   So we have to check
   * the colinear case beforehand, or we'll get fooled in the error
   * computation.
   */
  if (math::is_close(ax, ay, eps) || math::is_close(ax, az, eps) ||
      math::is_close(ay, az, eps)) {
    return false;
  }

  constexpr int kMAX_ITERS = 20;
  int iter;
  for (iter = 0; iter < kMAX_ITERS; ++iter) {
    bx = tx;
    by = ty;
    bz = tz;

    bx = bx - vdot(ay, bx) * ay;
    bx = bx - vdot(az, bx) * az;

    by = by - vdot(ax, by) * ax;
    by = by - vdot(az, by) * az;

    bz = bz - vdot(ax, bz) * ax;
    bz = bz - vdot(ay, bz) * ay;

    cx = 0.5 * (tx + bx);
    cy = 0.5 * (ty + by);
    cz = 0.5 * (tz + bz);

    if (normalize) {
      cx = vnormalize(cx);
      cy = vnormalize(cy);
      cz = vnormalize(cz);
    }

    value::double3 xDiff = tx - cx;
    value::double3 yDiff = ty - cy;
    value::double3 zDiff = tz - cz;

    double error = vdot(xDiff, xDiff) + vdot(yDiff, yDiff) + vdot(zDiff, zDiff);

    // error is squared, so compare to squared tolerance
    if (error < (eps * eps)) {
      break;
    }

    tx = cx;
    ty = cy;
    tz = cz;

    ax = tx;
    ay = ty;
    az = tz;

    if (!normalize) {
      ax = vnormalize(ax);
      ax = vnormalize(ay);
      ax = vnormalize(az);
    }
  }

  return iter < kMAX_ITERS;
}

/*
 * Make the matrix orthonormal in place using an iterative method.
 * It is potentially slower if the matrix is far from orthonormal (i.e. if
 * the row basis vectors are close to colinear) but in the common case
 * of near-orthonormality it should be just as fast.
 *
 * The translation part is left intact.  If the translation is represented as
 * a homogenous coordinate (i.e. a non-unity lower right corner), it is divided
 * out.
 */
value::matrix4d orthonormalize(const value::matrix4d &m, bool *result_valid) {
  value::matrix4d ret = value::matrix4d::identity();

  // orthogonalize and normalize row vectors
  value::double3 r0{m.m[0][0], m.m[0][1], m.m[0][2]};
  value::double3 r1{m.m[1][0], m.m[1][1], m.m[1][2]};
  value::double3 r2{m.m[2][0], m.m[2][1], m.m[2][2]};
  bool result = orthonormalize_basis(r0, r1, r2, true);
  ret.m[0][0] = r0[0];
  ret.m[0][1] = r0[1];
  ret.m[0][2] = r0[2];
  ret.m[1][0] = r1[0];
  ret.m[1][1] = r1[1];
  ret.m[1][2] = r1[2];
  ret.m[2][0] = r2[0];
  ret.m[2][1] = r2[1];
  ret.m[2][2] = r2[2];

  // divide out any homogeneous coordinate - unless it's zero
  const double min_vector_length = 1e-10;  //
  if (!math::is_close(ret.m[3][3], 1.0,
                      std::numeric_limits<double>::epsilon()) &&
      !math::is_close(ret.m[3][3], 0.0, min_vector_length)) {
    ret.m[3][0] /= ret.m[3][3];
    ret.m[3][1] /= ret.m[3][3];
    ret.m[3][2] /= ret.m[3][3];
    ret.m[3][3] = 1.0;
  }

  if (result_valid) {
    (*result_valid) = result;
    // TF_WARN("OrthogonalizeBasis did not converge, matrix may not be "
    //               "orthonormal.");
  }

  return ret;
}

// End pxrUSD
// -------------------------------------------------------------------------------

value::matrix4d trs_angle_xyz(
  const value::double3 &translation,
  const value::double3 &rotation_angles_xyz,
  const value::double3 &scale) {

  value::matrix4d m{value::matrix4d::identity()};

  XformEvaluator eval;
  eval.RotateX(rotation_angles_xyz[0]);
  eval.RotateY(rotation_angles_xyz[1]);
  eval.RotateZ(rotation_angles_xyz[2]);

  auto ret = eval.result();
  if (!ret) {
    // This should not happend though.
    return m;
  }
  value::matrix4d rMat = ret.value();

  value::matrix4d tMat{value::matrix4d::identity()};
  tMat.m[3][0] = translation[0];
  tMat.m[3][1] = translation[1];
  tMat.m[3][2] = translation[2];

  value::matrix4d sMat{value::matrix4d::identity()};
  sMat.m[0][0] = scale[0];
  sMat.m[1][1] = scale[1];
  sMat.m[2][2] = scale[2];
  
  m = sMat * rMat * tMat;

  return m;
}

//
// Build matrix from T R S.
//
// Rotation is given by 3 vectors axis(orthonormalized inside trs()).
// 
value::matrix4d trs_rot_axis(
  const value::double3 &translation,
  const value::double3 &rotation_x_axis,
  const value::double3 &rotation_y_axis,
  const value::double3 &rotation_z_axis,
  const value::double3 &scale) {

  value::matrix4d m{value::matrix4d::identity()};

  value::matrix4d rMat{value::matrix4d::identity()};
  rMat.m[0][0] = rotation_x_axis[0];
  rMat.m[0][1] = rotation_x_axis[1];
  rMat.m[0][2] = rotation_x_axis[2];
  rMat.m[1][0] = rotation_y_axis[0];
  rMat.m[1][1] = rotation_y_axis[1];
  rMat.m[1][2] = rotation_y_axis[2];
  rMat.m[2][0] = rotation_z_axis[0];
  rMat.m[2][1] = rotation_z_axis[1];
  rMat.m[2][2] = rotation_z_axis[2];

  bool result_valid{true};
  value::matrix4d orMat = orthonormalize(rMat, &result_valid);
  // TODO: report error when orthonormalize failed.

  value::matrix4d tMat{value::matrix4d::identity()};
  tMat.m[3][0] = translation[0];
  tMat.m[3][1] = translation[1];
  tMat.m[3][2] = translation[2];

  value::matrix4d sMat{value::matrix4d::identity()};
  sMat.m[0][0] = scale[0];
  sMat.m[1][1] = scale[1];
  sMat.m[2][2] = scale[2];
  
  m = sMat * orMat * tMat;

  return m;
}


}  // namespace tinyusdz
