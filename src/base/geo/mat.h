#pragma once

#include "vec.h"

namespace geo {

template <usize cols, usize rows>
struct GMat;

using Mat = GMat<2, 2>;
using Mat2x3 = GMat<2, 3>;
using Mat2x4 = GMat<2, 4>;
using Mat3x2 = GMat<3, 2>;
using Mat3 = GMat<3, 3>;
using Mat3x4 = GMat<3, 4>;
using Mat4x2 = GMat<4, 2>;
using Mat4x3 = GMat<4, 3>;
using Mat4 = GMat<4, 4>;

 // Stored in column-major order to match OpenGL
template <usize cols, usize rows>
struct GMat {
    GVec<GVec<float, rows>, cols> e;

    CE GMat () { }

    template <class... Args, std::enable_if_t<sizeof...(Args) == cols * rows, bool> = true>
    CE GMat (Args... args) {
        float es [cols * rows] {float(args)...};
        for (usize c = 0; c < cols; c++)
        for (usize r = 0; r < rows; r++) {
            e[c][r] = es[c * rows + r];
        }
    }

    template <class... Args, std::enable_if_t<sizeof...(Args) == cols, bool> = true>
    CE GMat (Args... args) : e{GVec<float, rows>{args}...} { }

    template <class T, usize n, std::enable_if_t<n == cols && n == rows, bool> = true>
    explicit CE GMat (GVec<T, n> diag) {
        for (usize i = 0; i < n; i++) {
            e[i][i] = diag[i];
        }
    }

    explicit CE GMat (float v) {
        for (usize i = 0; i < min(cols, rows); i++) {
            e[i][i] = v;
        }
    }

    CE GMat (NAN_t nan) {
        for (usize c = 0; c < cols; c++) {
            e[c] = nan;
        }
    }

    GVec<float, rows>& operator [] (usize c) { return e[c]; }
    const GVec<float, rows>& operator [] (usize c) const { return e[c]; }
    explicit CE operator bool () const {
        for (usize c = 0; c < cols; c++) {
            if (!e[c]) return false;
        }
        return true;
    }
};

///// OPERATORS

template <usize cols, usize rows>
CE bool operator == (const GMat<cols, rows>& a, const GMat<cols, rows>& b) {
    for (usize c = 0; c < cols; c++) {
        if (a[c] != b[c]) return false;
    }
    return true;
}
template <usize cols, usize rows>
CE bool operator != (const GMat<cols, rows>& a, const GMat<cols, rows>& b) {
    return !(a == b);
}

template <usize cols, usize rows>
CE GMat<cols, rows> operator - (const GMat<cols, rows>& a) {
    GMat<cols, rows> ret;
    for (usize c = 0; c < cols; c++)
    for (usize r = 0; r < rows; r++) {
        ret[c][r] = -a[c][r];
    }
    return ret;
}

template <usize cols, usize rows>
CE GMat<cols, rows> operator + (const GMat<cols, rows>& a, const GMat<cols, rows>& b) {
    GMat<cols, rows> ret;
    for (usize c = 0; c < cols; c++)
    for (usize r = 0; r < rows; r++) {
        ret[c][r] = a[c][r] + b[c][r];
    }
    return ret;
}

template <usize cols, usize rows>
CE GMat<cols, rows> operator - (const GMat<cols, rows>& a, const GMat<cols, rows>& b) {
    GMat<cols, rows> ret;
    for (usize c = 0; c < cols; c++)
    for (usize r = 0; r < rows; r++) {
        ret[c][r] = a[c][r] - b[c][r];
    }
    return ret;
}

template <usize cols, usize mid, usize rows>
CE GMat<cols, rows> operator * (const GMat<mid, rows>& a, const GMat<cols, mid>& b) {
    GMat<cols, rows> ret;
    for (usize c = 0; c < cols; c++)
    for (usize r = 0; r < rows; r++)
    for (usize m = 0; m < mid; m++) {
        ret[c][r] += a[m][r] * b[c][m];
    }
    return ret;
}

template <usize cols, usize rows>
CE GVec<float, cols> operator * (const GMat<cols, rows>& a, const GVec<float, rows>& b) {
    GVec<float, cols> ret;
    for (usize c = 0; c < cols; c++)
    for (usize r = 0; r < rows; r++) {
        ret[r] += a[c][r] * b[c];
    }
    return ret;
}

template <usize cols, usize rows>
CE GMat<cols, rows> operator * (const GMat<cols, rows>& a, float b) {
    GMat<cols, rows> ret;
    for (usize c = 0; c < cols; c++)
    for (usize r = 0; r < rows; r++) {
        ret[c][r] = a[c][r] * b;
    }
    return ret;
}

template <usize cols, usize rows>
CE GMat<cols, rows> operator * (float a, const GMat<cols, rows>& b) {
    GMat<cols, rows> ret;
    for (usize c = 0; c < cols; c++)
    for (usize r = 0; r < rows; r++) {
        ret[c][r] = a * b[c][r];
    }
    return ret;
}

#define GMAT_ASSIGN_OP(opeq, op) \
template <usize cols, usize rows, class T> \
CE GMat<cols, rows> operator opeq (GMat<cols, rows>& a, const T& b) { \
    return (a = a op b); \
}
GMAT_ASSIGN_OP(+=, +)
GMAT_ASSIGN_OP(-=, -)
GMAT_ASSIGN_OP(*=, *)

///// FUNCTIONS

template <usize cols, usize rows>
CE GMat<cols, rows> transpose (const GMat<rows, cols>& a) {
    GMat<cols, rows> ret;
    for (usize c = 0; c < cols; c++)
    for (usize r = 0; r < rows; r++) {
        ret[c][r] = a[r][c];
    }
    return ret;
}

template <usize cols, usize rows>
CE GMat<cols+1, rows> add_column (
    const GMat<cols, rows>& m,
    const GVec<float, rows>& v
) {
    GMat<cols+1, rows> ret;
    for (usize c = 0; c < cols; c++)
    for (usize r = 0; r < rows; r++) {
        ret[c][r] = m[c][r];
    }
    for (usize r = 0; r < rows; r++) {
        ret[cols][r] = v[r];
    }
    return ret;
}

template <usize cols, usize rows>
CE GMat<cols, rows+1> add_row (
    const GMat<cols, rows>& m,
    const GVec<float, cols>& v
) {
    GMat<cols, rows+1> ret;
    for (usize c = 0; c < cols; c++) {
        for (usize r = 0; r < rows; r++) {
            ret[c][r] = m[c][r];
        }
        ret[c][rows] = v[c];
    }
    return ret;
}

} // namespace geo

HACCABLE_TEMPLATE(
    HACCABLE_TEMPLATE_PARAMS(usize cols, usize rows),
    HACCABLE_TEMPLATE_TYPE(geo::GMat<cols, rows>),
    hcb::name([]{
        using namespace std::literals;
        if CE (cols == 2) {
            if CE (rows == 2) return "geo::Mat"sv;
            else if CE (rows == 3) return "geo::Mat2x3"sv;
            else if CE (rows == 4) return "geo::Mat2x4"sv;
        }
        else if CE (cols == 3) {
            if CE (rows == 2) return "geo::Mat3x2"sv;
            else if CE (rows == 3) return "geo::Mat3"sv;
            else if CE (rows == 4) return "geo::Mat3x4"sv;
        }
        else if CE (cols == 4) {
            if CE (rows == 2) return "geo::Mat4x2"sv;
            else if CE (rows == 3) return "geo::Mat4x3"sv;
            else if CE (rows == 4) return "geo::Mat4"sv;
        }
        static String r = "geo::GMat<" + std::to_string(cols) + ", " + std::to_string(rows) + ">";
        return Str(r);
    }),
    []{
        using namespace geo;
        if CE (cols == 2 && rows == 2) {
            return hcb::values(
                hcb::value(double(NAN), Mat(NAN)),
                hcb::value(0, Mat()),
                hcb::value(1, Mat(1)),
                hcb::value("flipx", Mat(-1, 0, 0, 1)),
                hcb::value("flipy", Mat(1, 0, 0, -1)),
                hcb::value("rotcw", Mat(0, -1, 1, 0)),
                hcb::value("rotccw", Mat(0, 1, -1, 0)),
                hcb::value("rot180", Mat(-1, 0, 0, -1))
            );
        }
        else {
            return hcb::values(
                hcb::value(double(NAN), GMat<cols, rows>(NAN)),
                hcb::value(0, &GMat<cols, rows>()),
                hcb::value(1, &GMat<cols, rows>(1))
            );
        }
    }(),
    hcb::length(hcb::template constant<usize>(cols)),
    hcb::elem_func([](geo::GMat<cols, rows>& v, usize i){
        if (i < cols) return hacc::Reference(&v[i]);
        else return hacc::Reference();
    })
)
