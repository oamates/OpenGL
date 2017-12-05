// Class declaration: Matrix

#ifndef DGMATRIX_H
#define DGMATRIX_H

#include "dgmath.hpp"

namespace Dg
{

template<size_t M, size_t N, typename Real> struct Matrix;
template<size_t M, size_t N, typename Real> Matrix<N, M, Real> Transpose(const Matrix<M, N, Real>&);
template<size_t M, size_t N, typename Real> Matrix<M, N, Real> operator * (Real, const Matrix<M, N, Real>&);

// Generic M * N matrix class.

template<size_t M, size_t N, typename Real> struct Matrix
{
    static_assert(M > 0 && N > 0, "Matrix cannot have a zero dimension.");

    Real m_V[M * N];

    template<size_t _M, size_t _N, typename T> friend class Matrix;

    // Default constructor, initialized to identity matrix.
    Matrix();
    ~Matrix() {}

    Matrix(Matrix const &);
    Matrix& operator=(Matrix const &);

    // Accessor i: row, j:column.
    Real& operator()(size_t m, size_t n);

    // Accessor i: row, j:column.
    Real operator()(size_t m, size_t n) const;

    // Accessor element by index.
    Real& operator [] (size_t i)
        { return m_V[i]; }
    Real operator [] (size_t i) const
        { return m_V[i]; }

    bool operator == (const Matrix&) const;
    bool operator != (const Matrix&) const;

    // Checks if all elements are below the tolerance.
    bool IsZero() const;

    // Set all elements to zero.
    void Zero();

    // Checks if matrix is equal to the identity, within some tolerance.
    bool IsIdentity() const;

    // Directly set the elements.
    void Set(Real const [M * N]);

    void SetRow(size_t m, Matrix<1, N, Real> const &);
    void GetRow(size_t m, Matrix<1, N, Real>&) const;
    void SetColumn(size_t n, Matrix<M, 1, Real> const &);
    void GetColumn(size_t n, Matrix<M, 1, Real>&) const;

    // Sets near-zero elements to 0.
    void Clean();

    // Set to identity matrix. For aquare martices only.
    void Identity();

    // Compute the transpose matrix.
    template<size_t _M, size_t _N, typename _Real> friend Matrix<_N, _M, _Real> Transpose(Matrix<_M, _N, _Real> const &);

    // Set matrix to its transpose. For square matrices only.
    Matrix& Transpose();

    Matrix operator+ (Matrix const &) const;
    Matrix& operator+= (Matrix const &);
    Matrix operator- (Matrix const &) const;
    Matrix& operator-= (Matrix const &);

    // Negate matrix.
    Matrix operator-() const;

    Matrix& operator*= (Matrix const &);

    template<size_t _M> Matrix<M, _M, Real> operator* (Matrix<N, _M, Real> const &) const;

    Matrix& operator*= (Real);
    Matrix operator* (Real) const;
    Matrix& operator/= (Real);
    Matrix operator/ (Real) const;

    template<size_t _M, size_t _N, typename _Real> friend Matrix<_M, _N, _Real> operator* (_Real, Matrix<_M, _N, _Real> const &);
};

//--------------------------------------------------------------------------------
//	@	Matrix::Matrix()
//--------------------------------------------------------------------------------
template<size_t M, size_t N, typename Real> Matrix<M, N, Real>::Matrix() { }

//--------------------------------------------------------------------------------
//	@	Matrix::operator()
//--------------------------------------------------------------------------------
template<size_t M, size_t N, typename Real> Real& Matrix<M, N, Real>::operator() (size_t a_m, size_t a_n)
    { return m_V[a_m * N + a_n]; }

//--------------------------------------------------------------------------------
//	@	Matrix::operator()
//--------------------------------------------------------------------------------
template<size_t M, size_t N, typename Real> Real Matrix<M, N, Real>::operator() (size_t a_m, size_t a_n) const
    { return m_V[a_m * N + a_n]; }

//--------------------------------------------------------------------------------
//	@	Matrix::operator=()
//--------------------------------------------------------------------------------
template<size_t M, size_t N, typename Real> Matrix<M, N, Real>& Matrix<M, N, Real>::operator = (const Matrix<M, N, Real>& a_other)
{
    for (size_t i = 0; i < M * N; ++i)
        m_V[i] = a_other.m_V[i];
    return *this;
}

//--------------------------------------------------------------------------------
//	@	Matrix::Matrix()
//--------------------------------------------------------------------------------
template<size_t M, size_t N, typename Real> Matrix<M, N, Real>::Matrix(const Matrix<M, N, Real>& a_other)
{
    for (size_t i = 0; i < M * N; ++i)
        m_V[i] = a_other.m_V[i];
}

//--------------------------------------------------------------------------------
//	@	Matrix::operator==()
//--------------------------------------------------------------------------------
template<size_t M, size_t N, typename Real> bool Matrix<M, N, Real>::operator == (const Matrix<M, N, Real>& a_other) const
{
    for (size_t i = 0; i < M * N; i++)
        if (!Dg::AreEqual(m_V[i], a_other.m_V[i]))
            return false;
    return true;
}

//--------------------------------------------------------------------------------
//	@	Matrix::operator!=()
//--------------------------------------------------------------------------------
template<size_t M, size_t N, typename Real> bool Matrix<M, N, Real>::operator != (const Matrix<M, N, Real>& a_other) const
{
    for (size_t i = 0; i < M * N; i++)
        if (!Dg::AreEqual(m_V[i], a_other.m_V[i]))
            return true;
    return false;
}

//--------------------------------------------------------------------------------
//	@	Matrix44Dg::IsZero()
//--------------------------------------------------------------------------------
template<size_t M, size_t N, typename Real> void Matrix<M, N, Real>::Zero()
{
    for (size_t i = 0; i < M * N; i++)
        m_V[i] = static_cast<Real>(0.0);
}

//--------------------------------------------------------------------------------
//	@	Matrix44Dg::IsZero()
//--------------------------------------------------------------------------------
template<size_t M, size_t N, typename Real> bool Matrix<M, N, Real>::IsZero() const
{
    for (size_t i = 0; i < M * N; i++)
        if (!Dg::IsZero(m_V[i]))
            return false;
    return true;
}

//--------------------------------------------------------------------------------
//	@	Matrix::IsIdentity()
//--------------------------------------------------------------------------------
template<size_t M, size_t N, typename Real> bool Matrix<M, N, Real>::IsIdentity() const
{
    static_assert(M == N, "Only square matrices can be identity");

    for (size_t m = 0; m < M - 1; ++m)
        for (size_t n = m + 1; n < N; ++n)
            if (!Dg::IsZero(m_V[m * N + n]) || !Dg::IsZero(m_V[n * M + m]))
                return false;

    for (size_t i = 0; i < M * N; i += (N + 1))
        if (!Dg::IsZero(static_cast<Real>(1.0) - m_V[i]))
            return false;

    return true;
}

//--------------------------------------------------------------------------------
//	@	Matrix::Set()
//--------------------------------------------------------------------------------
template<size_t M, size_t N, typename Real> void Matrix<M, N, Real>::Set(Real const a_data[M * N])
{
    for (size_t i = 0; i < M * N; ++i)
        m_V[i] = a_data[i];
}

//--------------------------------------------------------------------------------
//	@	Matrix::SetRow()
//--------------------------------------------------------------------------------
template<size_t M, size_t N, typename Real> void Matrix<M, N, Real>::SetRow(size_t a_m, Matrix<1, N, Real> const & a_row)
{
    a_m *= N;

    for (size_t i = 0; i < N; ++i)
        m_V[a_m +i] = a_row.m_V[i];
}

//--------------------------------------------------------------------------------
//	@	Matrix::GetRow()
//--------------------------------------------------------------------------------
template<size_t M, size_t N, typename Real> void Matrix<M, N, Real>::GetRow(size_t a_m, Matrix<1, N, Real>& a_out) const
{
    a_m *= N;
    for (size_t i = 0; i < N; ++i)
        a_out.m_V[i] = m_V[a_m + i];
}

//--------------------------------------------------------------------------------
//	@	Matrix::SetColumn()
//--------------------------------------------------------------------------------
template<size_t M, size_t N, typename Real> void Matrix<M, N, Real>::SetColumn(size_t a_n, Matrix<M, 1, Real> const & a_col)
{
    for (size_t i = 0; i < M; ++i)
        m_V[i * N + a_n] = a_col.m_V[i];
}

//--------------------------------------------------------------------------------
//	@	Matrix::GetColumn()
//--------------------------------------------------------------------------------
template<size_t M, size_t N, typename Real> void Matrix<M, N, Real>::GetColumn(size_t a_n, Matrix<M, 1, Real>& a_out) const
{
    for (size_t i = 0; i < M; ++i)
        a_out.m_V[i] = m_V[i * N + a_n];
}

//--------------------------------------------------------------------------------
//	@	Matrix::Clean()
//--------------------------------------------------------------------------------
template<size_t M, size_t N, typename Real> void Matrix<M, N, Real>::Clean()
{
    for (size_t i = 0; i < M * N; i++)
        if (Dg::IsZero(m_V[i]))
            m_V[i] = static_cast<Real>(0.0);
}

//--------------------------------------------------------------------------------
//	@	Matrix::Indentity()
//--------------------------------------------------------------------------------
template<size_t M, size_t N, typename Real> void Matrix<M, N, Real>::Identity()
{
    static_assert(M == N, "Only square matrices can be identity");

    for (size_t m = 0; m < M - 1; ++m)
        for (size_t n = m + 1; n < N; ++n)
            m_V[m * N + n] = m_V[n * M + m] = static_cast<Real>(0.0);

    for (size_t i = 0; i < M * N; i += (N + 1))
        m_V[i] = static_cast<Real>(1.0);
}

//--------------------------------------------------------------------------------
//	@	Transpose()
//-------------------------------------------------------------------------------
template<size_t M, size_t N, typename Real> Matrix<N, M, Real> Transpose(Matrix<M, N, Real> const & a_mat)
{
    Matrix<N, M, Real> result;
    for (size_t m = 0; m < M; ++m)
        for (size_t n = 0; n < N; ++n)
            result.m_V[n * M + m] = a_mat.m_V[m * N + n];
    return result;
}

//--------------------------------------------------------------------------------
//	@	Transpose()
//-------------------------------------------------------------------------------
template<size_t M, size_t N, typename Real> Matrix<M, N, Real>& Matrix<M, N, Real>::Transpose()
{
    static_assert(M == N, "Can only transpose a square matrix");
    for (size_t m = 0; m < M - 1; ++m)
        for (size_t n = m + 1; n < N; ++n)
        {
            Real temp = m_V[m * N + n];
            m_V[m * N + n] = m_V[n * N + m];
            m_V[n * N + m] = temp;
        }
    return *this;
}

//-------------------------------------------------------------------------------
//	@	Matrix::operator+()
//-------------------------------------------------------------------------------
template<size_t M, size_t N, typename Real> Matrix<M, N, Real> Matrix<M, N, Real>::operator + (Matrix<M, N, Real> const & a_other) const
{
    Matrix<M, N, Real> result;
    for (size_t i = 0; i < M * N; ++i)
        result.m_V[i] = m_V[i] + a_other.m_V[i];
    return result;
}

//-------------------------------------------------------------------------------
//	@	Matrix::operator+=()
//-------------------------------------------------------------------------------
template<size_t M, size_t N, typename Real> Matrix<M, N, Real>& Matrix<M, N, Real>::operator += (Matrix<M, N, Real> const & a_other)
{
    for (size_t i = 0; i < M * N; ++i)
        m_V[i] += a_other.m_V[i];
    return *this;
}

//-------------------------------------------------------------------------------
//	@	Matrix::operator-()
//-------------------------------------------------------------------------------
template<size_t M, size_t N, typename Real> Matrix<M, N, Real> Matrix<M, N, Real>::operator - (Matrix<M, N, Real> const & other) const
{
    Matrix<M, N, Real> result;
    for (size_t i = 0; i < M * N; ++i)
        result.m_V[i] = m_V[i] - other.m_V[i];
    return result;
}

//-------------------------------------------------------------------------------
//	@	Matrix::operator-=()
//-------------------------------------------------------------------------------
template<size_t M, size_t N, typename Real> Matrix<M, N, Real>& Matrix<M, N, Real>::operator -= (Matrix<M, N, Real> const & a_other)
{
    for (size_t i = 0; i < M * N; ++i)
        m_V[i] -= a_other.m_V[i];
    return *this;
}

//-------------------------------------------------------------------------------
//	@	Matrix::operator-()
//-------------------------------------------------------------------------------
template<size_t M, size_t N, typename Real> Matrix<M, N, Real> Matrix<M, N, Real>::operator - () const
{
    Matrix<M, N, Real> result;
    for (size_t i = 0; i < M * N; ++i)
        result.m_V[i] = -m_V[i];
    return result;
}

//-------------------------------------------------------------------------------
//	@	Matrix::operator*()
//-------------------------------------------------------------------------------
template<size_t M, size_t N, typename Real> template<size_t _M> Matrix<M, _M, Real> Matrix<M, N, Real>::operator*(Matrix<N, _M, Real> const & a_other) const
{
    Matrix<M, _M, Real> result;
    result.Zero();

    for (size_t m = 0; m < M; ++m)
        for (size_t _m = 0; _m < _M; ++_m)
            for (size_t n = 0; n < N; ++n)
                result.m_V[m * _M + _m] += (m_V[m * N + n] * a_other.m_V[n * _M + _m]);
    return result;
}

//-------------------------------------------------------------------------------
//	@	Matrix::operator*()
//-------------------------------------------------------------------------------
template<size_t M, size_t N, typename Real> Matrix<M, N, Real>& Matrix<M, N, Real>::operator *= (Matrix<M, N, Real> const & a_other)
{
    static_assert(M == N, "Can only assign to self if a square matrix");
    Matrix<M, N, Real> result;
    result.Zero();
    for (size_t m = 0; m < M; ++m)
        for (size_t n = 0; n < N; ++n)
            for (size_t i = 0; i < N; ++i)
                result.m_V[m * N + n] += (m_V[m * N + i] * a_other.m_V[i * N + n]);
    Set(result.m_V);
    return *this;
}

//-------------------------------------------------------------------------------
//	@	Matrix::operator*=()
//-------------------------------------------------------------------------------
template<size_t M, size_t N, typename Real> Matrix<M, N, Real>& Matrix<M, N, Real>::operator *= (Real a_scalar)
{
    for (size_t i = 0; i < M * N; ++i)
        m_V[i] *= a_scalar;
    return *this;
}

//-------------------------------------------------------------------------------
//	@	Matrix::operator*()
//-------------------------------------------------------------------------------
template<size_t M, size_t N, typename Real> Matrix<M, N, Real> operator * (Real a_scalar, Matrix<M, N, Real> const & matrix)
{
    Matrix<M, N, Real> result;
    for (size_t i = 0; i < M * N; ++i)
        result.m_V[i] = matrix.m_V[i] * a_scalar;
    return result;
}

//-------------------------------------------------------------------------------
//	@	Matrix::operator*()
//-------------------------------------------------------------------------------
template<size_t M, size_t N, typename Real> Matrix<M, N, Real> Matrix<M, N, Real>::operator * (Real a_scalar) const
{
    Matrix<M, N, Real> result;
    for (size_t i = 0; i < M * N; ++i)
        result.m_V[i] = m_V[i] * a_scalar;
    return result;
}

//-------------------------------------------------------------------------------
//	@	Matrix::operator/()
//-------------------------------------------------------------------------------
template<size_t M, size_t N, typename Real> Matrix<M, N, Real> Matrix<M, N, Real>::operator / (Real a_scalar) const
{
    Matrix<M, N, Real> result;
    for (size_t i = 0; i < M * N; ++i)
        result.m_V[i] = m_V[i] / a_scalar;
    return result;
}

//-------------------------------------------------------------------------------
//	@	Matrix::operator/=()
//-------------------------------------------------------------------------------
template<size_t M, size_t N, typename Real> Matrix<M, N, Real>& Matrix<M, N, Real>::operator /= (Real a_scalar)
{
    for (size_t i = 0; i < M * N; ++i)
        m_V[i] /= a_scalar;
    return *this;
}

}

#endif