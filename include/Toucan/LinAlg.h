#pragma once

#include <array>
#include <cmath>
#include <cassert>
#include <type_traits>
#include <ostream>
#include <ios>
#include <limits>

namespace Toucan {

/***** Matrix *****/

template<typename scalar_t>
constexpr scalar_t normalized_sensitivity = 20*std::numeric_limits<scalar_t>::epsilon();

template<typename scalar_t, int rows, int columns>
class Matrix {
public:
	// Static constructors
	static constexpr Matrix<scalar_t, rows, columns> Zero();
	static constexpr Matrix<scalar_t, rows, columns> Ones();
	static constexpr Matrix<scalar_t, rows, columns> Identity();
	
	static constexpr Matrix<scalar_t, rows, columns> UnitX();
	static constexpr Matrix<scalar_t, rows, columns> UnitY();
	static constexpr Matrix<scalar_t, rows, columns> UnitZ();
	
	static constexpr Matrix<scalar_t, rows, columns> UnitN(int n);
	
	// Constructors
	constexpr Matrix<scalar_t, rows, columns>();
	
	template<typename... scalar_t_var>
	explicit constexpr Matrix<scalar_t, rows, columns>(const scalar_t_var&... v);
	
	// Matrix shape
	[[nodiscard]] constexpr int number_of_rows() const;
	[[nodiscard]] constexpr int number_of_columns() const;
	[[nodiscard]] constexpr int number_of_elements() const;
	
	// Norms
	[[nodiscard]] constexpr inline scalar_t squared_norm() const;
	[[nodiscard]] constexpr inline scalar_t norm() const;
	[[nodiscard]] constexpr inline bool is_normalized() const;
	constexpr void normalize();
	[[nodiscard]] constexpr inline Matrix<scalar_t, rows, columns> normalized() const;
	
	// Mathematical functions
	[[nodiscard]] constexpr scalar_t trace() const;
	[[nodiscard]] constexpr scalar_t dot_product(const Matrix<scalar_t, rows, columns>& rhs) const;
	[[nodiscard]] constexpr Matrix<scalar_t, rows, columns> cross_product(const Matrix<scalar_t, rows, columns>& rhs) const;
	[[nodiscard]] constexpr Matrix<scalar_t, columns, rows> transpose() const;
	
	// Accessors
	constexpr scalar_t& x();
	constexpr scalar_t& y();
	constexpr scalar_t& z();
	
	constexpr const scalar_t& x() const;
	constexpr const scalar_t& y() const;
	constexpr const scalar_t& z() const;
	
	constexpr inline scalar_t& operator()(int index);
	constexpr inline const scalar_t& operator()(int index) const;
	
	constexpr inline scalar_t& operator()(int row, int column);
	constexpr inline const scalar_t& operator()(int row, int column) const;
	
	[[nodiscard]] scalar_t* data();
	[[nodiscard]] const scalar_t* data() const;
	
private:
	scalar_t m_data[rows * columns]; // Row major
};

/*** Static constructors implementation ***/
template<typename scalar_t, int rows, int columns>
constexpr inline Matrix<scalar_t, rows, columns> Matrix<scalar_t, rows, columns>::Zero() {
	Matrix<scalar_t, rows, columns> matrix;
	for (int row_index = 0; row_index < rows; ++row_index) {
		for (int column_index = 0; column_index < columns; ++column_index) {
			matrix(row_index, column_index) = scalar_t(0);
		}
	}
	
	return matrix;
}

template<typename scalar_t, int rows, int columns>
constexpr inline Matrix<scalar_t, rows, columns> Matrix<scalar_t, rows, columns>::Ones() {
	Matrix<scalar_t, rows, columns> matrix;
	for (int row_index = 0; row_index < rows; ++row_index) {
		for (int column_index = 0; column_index < columns; ++column_index) {
			matrix(row_index, column_index) = scalar_t(1);
		}
	}
	
	return matrix;
}

template<typename scalar_t, int rows, int columns>
constexpr inline Matrix<scalar_t, rows, columns> Matrix<scalar_t, rows, columns>::Identity() {
	Matrix<scalar_t, rows, columns> matrix;
	for (int row_index = 0; row_index < rows; ++row_index) {
		for (int column_index = 0; column_index < columns; ++column_index) {
			if (row_index == column_index) {
				matrix(row_index, column_index) = scalar_t(1);
			} else {
				matrix(row_index, column_index) = scalar_t(0);
			}
		}
	}
	
	return matrix;
}

template<typename scalar_t, int rows, int columns>
constexpr inline Matrix<scalar_t, rows, columns> Matrix<scalar_t, rows, columns>::UnitX() {
	static_assert((rows >= 1 and rows <= 3 and columns == 1) or (rows == 1 and columns >= 1 and columns <= 3), "UnitX constructor may only be used on vector types of appropriate size.");
	Matrix<scalar_t, rows, columns> v = Matrix<scalar_t, rows, columns>::Zero();
	v.x() = scalar_t(1);
	return v;
}

template<typename scalar_t, int rows, int columns>
constexpr inline Matrix<scalar_t, rows, columns> Matrix<scalar_t, rows, columns>::UnitY() {
	static_assert((rows >= 2  and rows <= 3 and columns == 1) or (rows == 1 and columns >= 2 and columns <= 3), "UnitY constructor may only be used on vector types of appropriate size.");
	Matrix<scalar_t, rows, columns> v = Matrix<scalar_t, rows, columns>::Zero();
	v.y() = scalar_t(1);
	return v;
}

template<typename scalar_t, int rows, int columns>
constexpr inline Matrix<scalar_t, rows, columns> Matrix<scalar_t, rows, columns>::UnitZ() {
	static_assert((rows >= 3  and rows <= 3 and columns == 1) or (rows == 1 and columns >= 3 and columns <= 3), "UnitZ constructor may only be used on vector types of appropriate size.");
	Matrix<scalar_t, rows, columns> v = Matrix<scalar_t, rows, columns>::Zero();
	v.z() = scalar_t(1);
	return v;
}

template<typename scalar_t, int rows, int columns>
constexpr inline Matrix<scalar_t, rows, columns> Matrix<scalar_t, rows, columns>::UnitN(int n) { // TODO(Matias): Maybe let n be a template parameter instead?
	static_assert(rows == 1 or columns == 1, "UnitN constructor may only be used on vector types of appropriate size.");
	Matrix<scalar_t, rows, columns> v = Matrix<scalar_t, rows, columns>::Zero();
	v(n) = scalar_t(1);
	return v;
}

/*** Constructors implementation ***/
template<typename scalar_t, int rows, int columns>
constexpr inline Matrix<scalar_t, rows, columns>::Matrix() {
	for (int row_index = 0; row_index < rows; ++row_index) {
		for (int column_index = 0; column_index < columns; ++column_index) {
			// TODO(Matias): Investigate if memset is faster.
			this->operator()(row_index, column_index) = scalar_t(0);
		}
	}
}

template<typename scalar_t, int rows, int columns>
template<typename... scalar_t_var>
constexpr inline Matrix<scalar_t, rows, columns>::Matrix(const scalar_t_var& ... v) :
m_data{static_cast<scalar_t>(v)...} {
	static_assert(sizeof...(v) == rows*columns, "Number of constructor parameters does not match the size of the matrix");
}

/*** Matrix shape implementation ***/
template<typename scalar_t, int rows, int columns> constexpr inline int Matrix<scalar_t, rows, columns>::number_of_rows() const { return rows; }
template<typename scalar_t, int rows, int columns> constexpr inline int Matrix<scalar_t, rows, columns>::number_of_columns() const { return columns; }
template<typename scalar_t, int rows, int columns> constexpr inline int Matrix<scalar_t, rows, columns>::number_of_elements() const { return rows*columns; }

/*** Norms implementation ***/
template<typename scalar_t, int rows, int columns>
constexpr inline scalar_t Matrix<scalar_t, rows, columns>::squared_norm() const {
	scalar_t squared_sum = scalar_t(0.0);
	for (int row_index = 0; row_index < rows; ++row_index) {
		for (int column_index = 0; column_index < columns; ++column_index) {
			squared_sum += this->operator()(row_index, column_index) * this->operator()(row_index, column_index);
		}
	}
	return squared_sum;
};

template<typename scalar_t, int rows, int columns>
constexpr inline scalar_t Matrix<scalar_t, rows, columns>::norm() const {
	static_assert(std::is_floating_point_v<scalar_t>, "The `norm` method may only be called on floating point types. Consider using `squared_norm` instead.");
	return std::sqrt(squared_norm());
}

template<typename scalar_t, int rows, int columns>
constexpr inline bool Matrix<scalar_t, rows, columns>::is_normalized() const {
	return std::abs(norm() - scalar_t(1)) <= normalized_sensitivity<scalar_t>;
}


template<typename scalar_t, int rows, int columns>
constexpr inline void Matrix<scalar_t, rows, columns>::normalize() {
	scalar_t multiplier = scalar_t(1.0)/this->norm();
	for (int row_index = 0; row_index < rows; ++row_index) {
		for (int column_index = 0; column_index < columns; ++column_index) {
			this->operator()(row_index, column_index) *= multiplier;
		}
	}
}

template<typename scalar_t, int rows, int columns>
constexpr inline Matrix<scalar_t, rows, columns> Matrix<scalar_t, rows, columns>::normalized() const {
	Matrix<scalar_t, rows, columns> normalized_matrix = *this;
	normalized_matrix.normalize();
	return normalized_matrix;
}

/*** Mathematical functions implementation ***/
template<typename scalar_t, int rows, int columns>
constexpr inline scalar_t Matrix<scalar_t, rows, columns>::trace() const {
	static_assert(rows == columns, "Trace can only be called on square matrices");
	scalar_t sum = scalar_t(0);
	for (int index = 0; index < rows; ++index) {
		sum += this->operator()(index, index);
	}
	return sum;
}

template<typename scalar_t, int rows, int columns>
constexpr inline scalar_t Matrix<scalar_t, rows, columns>::dot_product(const Matrix<scalar_t, rows, columns>& rhs) const {
	scalar_t sum = scalar_t(0);
	for (int row_index = 0; row_index < rows; ++row_index) {
		for (int column_index = 0; column_index < columns; ++column_index) {
			sum += this->operator()(row_index, column_index) * rhs(row_index, column_index);
		}
	}
	return sum;
}
template<typename scalar_t, int rows, int columns>
constexpr inline Matrix<scalar_t, rows, columns> Matrix<scalar_t, rows, columns>::cross_product(const Matrix<scalar_t, rows, columns>& rhs) const {
	static_assert((rows == 3 and columns == 1) or (rows == 1 and columns == 3), "Cross product is only defined for Vector3 or RowVector3");
	Matrix<scalar_t, rows, columns> product;
	product.x() = this->y()*rhs.z() - this->z()*rhs.y();
	product.y() = this->z()*rhs.x() - this->x()*rhs.z();
	product.z() = this->x()*rhs.y() - this->y()*rhs.x();
	return product;
}

template<typename scalar_t, int rows, int columns>
constexpr inline Matrix<scalar_t, columns, rows> Matrix<scalar_t, rows, columns>::transpose() const {
	Matrix<scalar_t, columns, rows> transpose;
	for (int row_index = 0; row_index < rows; ++row_index) {
		for (int column_index = 0; column_index < columns; ++column_index) {
			transpose(column_index, row_index) = this->operator()(row_index, column_index);
		}
	}
	return transpose;
}

/*** Accessors implementation ***/
template<typename scalar_t, int rows, int columns>
constexpr inline scalar_t& Matrix<scalar_t, rows, columns>::x() {
	static_assert((rows >= 1 and columns == 1) or (rows == 1 and columns >= 1), "Named accessors should only be used on vector types of appropriate size.");
	return this->operator()(0, 0);
}

template<typename scalar_t, int rows, int columns>
constexpr inline scalar_t& Matrix<scalar_t, rows, columns>::y() {
	static_assert((rows >= 2 and columns == 1) or (rows == 1 and columns >= 2), "Named accessors should only be used on vector types of appropriate size.");
	if constexpr (rows == 1) { // Row vector
		return this->operator()(0, 1);
	} else { // Column vector
		return this->operator()(1, 0);
	}
}

template<typename scalar_t, int rows, int columns>
constexpr inline scalar_t& Matrix<scalar_t, rows, columns>::z() {
	static_assert((rows >= 3 and columns == 1) or (rows == 1 and columns >= 3), "Named accessors should only be used on vector types of appropriate size.");
	if constexpr (rows == 1) { // Row vector
		return this->operator()(0, 2);
	} else { // Column vector
		return this->operator()(2, 0);
	}
}

template<typename scalar_t, int rows, int columns>
constexpr inline const scalar_t& Matrix<scalar_t, rows, columns>::x() const {
	static_assert((rows >= 1 and columns == 1) or (rows == 1 and columns >= 1), "Named accessors should only be used on vector types of appropriate size.");
	return this->operator()(0, 0);
}

template<typename scalar_t, int rows, int columns>
constexpr inline const scalar_t& Matrix<scalar_t, rows, columns>::y() const {
	static_assert((rows >= 2 and columns == 1) or (rows == 1 and columns >= 2), "Named accessors should only be used on vector types of appropriate size.");
	if constexpr (rows == 1) { // Row vector
		return this->operator()(0, 1);
	} else { // Column vector
		return this->operator()(1, 0);
	}
}

template<typename scalar_t, int rows, int columns>
constexpr inline const scalar_t& Matrix<scalar_t, rows, columns>::z() const {
	static_assert((rows >= 3 and columns == 1) or (rows == 1 and columns >= 3), "Named accessors should only be used on vector types of appropriate size.");
	if constexpr (rows == 1) { // Row vector
		return this->operator()(0, 2);
	} else { // Column vector
		return this->operator()(2, 0);
	}
}

template<typename scalar_t, int rows, int columns>
constexpr inline scalar_t& Matrix<scalar_t, rows, columns>::operator()(int index) {
	static_assert(rows == 1 or columns == 1, "Single parameter parenthesis operator should only be used on vectors.");
	if constexpr (rows == 1) { // Row vector
		assert(index < columns);
		return this->operator()(0, index);
	} else { // Column vector
		assert(index < rows);
		return this->operator()(index, 0);
	}
}

template<typename scalar_t, int rows, int columns>
constexpr inline const scalar_t& Matrix<scalar_t, rows, columns>::operator()(int index) const {
	static_assert(rows == 1 or columns == 1, "Single parameter parenthesis operator should only be used on vectors.");
	if constexpr (rows == 1) { // Row vector
		assert(index < columns);
		return this->operator()(0, index);
	} else { // Column vector
		assert(index < rows);
		return this->operator()(index, 0);
	}
}

template<typename scalar_t, int rows, int columns>
constexpr inline scalar_t& Matrix<scalar_t, rows, columns>::operator()(int row, int column) {
	assert(row < rows and column < columns);
	return m_data[row*columns + column];
}

template<typename scalar_t, int rows, int columns>
constexpr inline const scalar_t& Matrix<scalar_t, rows, columns>::operator()(int row, int column) const {
	assert(row < rows and column < columns);
	return m_data[row*columns + column];
}

template<typename scalar_t, int rows, int columns>
inline scalar_t* Matrix<scalar_t, rows, columns>::data() {
	return &m_data[0];
}

template<typename scalar_t, int rows, int columns>
const inline scalar_t* Matrix<scalar_t, rows, columns>::data() const {
	return &m_data[0];
}

/*** Operator implementations ***/

/** Scalar Matrix operators **/
template<typename scalar_t, int rows, int columns>
constexpr inline Matrix<scalar_t, rows, columns> operator-(const Matrix<scalar_t, rows, columns>& matrix) {
	Matrix<scalar_t, rows, columns> result;
	for (int row_index = 0; row_index < rows; ++row_index) {
		for (int column_index = 0; column_index < columns; ++column_index) {
			result(row_index, column_index) = - matrix(row_index, column_index);
		}
	}
	return result;
}

template<typename scalar_t, int rows, int columns>
constexpr inline Matrix<scalar_t, rows, columns> operator+(const Matrix<scalar_t, rows, columns>& lhs, const scalar_t& rhs) {
	Matrix<scalar_t, rows, columns> result;
	for (int row_index = 0; row_index < rows; ++row_index) {
		for (int column_index = 0; column_index < columns; ++column_index) {
			result(row_index, column_index) = lhs(row_index, column_index) + rhs;
		}
	}
	return result;
}

template<typename scalar_t, int rows, int columns>
constexpr inline Matrix<scalar_t, rows, columns> operator+(const scalar_t& lhs, const Matrix<scalar_t, rows, columns>& rhs) {
	Matrix<scalar_t, rows, columns> result;
	for (int row_index = 0; row_index < rows; ++row_index) {
		for (int column_index = 0; column_index < columns; ++column_index) {
			result(row_index, column_index) = lhs + rhs(row_index, column_index);
		}
	}
	return result;
}

template<typename scalar_t, int rows, int columns>
constexpr inline Matrix<scalar_t, rows, columns> operator*(const Matrix<scalar_t, rows, columns>& lhs, const scalar_t& rhs) {
	Matrix<scalar_t, rows, columns> result;
	for (int row_index = 0; row_index < rows; ++row_index) {
		for (int column_index = 0; column_index < columns; ++column_index) {
			result(row_index, column_index) = lhs(row_index, column_index) * rhs;
		}
	}
	return result;
}

template<typename scalar_t, int rows, int columns>
constexpr inline Matrix<scalar_t, rows, columns> operator*(const scalar_t& lhs, const Matrix<scalar_t, rows, columns>& rhs) {
	Matrix<scalar_t, rows, columns> result;
	for (int row_index = 0; row_index < rows; ++row_index) {
		for (int column_index = 0; column_index < columns; ++column_index) {
			result(row_index, column_index) = lhs * rhs(row_index, column_index);
		}
	}
	return result;
}

/** Matrix Matrix operators **/
template<typename scalar_t, int rows, int columns>
constexpr inline Matrix<scalar_t, rows, columns> operator+(const Matrix<scalar_t, rows, columns>& lhs, const Matrix<scalar_t, rows, columns>& rhs) {
	Matrix<scalar_t, rows, columns> result;
	for (int row_index = 0; row_index < rows; ++row_index) {
		for (int column_index = 0; column_index < columns; ++column_index) {
			result(row_index, column_index) = lhs(row_index, column_index) + rhs(row_index, column_index);
		}
	}
	return result;
}

template<typename scalar_t, int rows, int common, int columns>
constexpr inline Matrix<scalar_t, rows, columns> operator*(const Matrix<scalar_t, rows, common>& lhs, const Matrix<scalar_t, common, columns>& rhs) {
	Matrix<scalar_t, rows, columns> result;
	for (int row_index = 0; row_index < rows; ++row_index) {
		for (int column_index = 0; column_index < columns; ++column_index) {
			scalar_t sum = scalar_t(0.0);
			for (int common_index = 0; common_index < common; ++common_index) {
				sum += lhs(row_index, common_index) * rhs(common_index, column_index);
			}
			result(row_index, column_index) = sum;
		}
	}
	return result;
}

template<typename scalar_t, int rows, int columns>
constexpr inline bool operator==(const Matrix<scalar_t, rows, columns>& lhs, const Matrix<scalar_t, rows, columns>& rhs) {
	
	for (int row_index = 0; row_index < rows; ++row_index) {
		for (int column_index = 0; column_index < columns; ++column_index) {
			if (lhs(row_index, column_index) != rhs(row_index, column_index)) { return false; }
		}
	}
	return true;
}

template<typename scalar_t, int rows, int columns>
constexpr inline bool operator!=(const Matrix<scalar_t, rows, columns>& lhs, const Matrix<scalar_t, rows, columns>& rhs) {
	
	for (int row_index = 0; row_index < rows; ++row_index) {
		for (int column_index = 0; column_index < columns; ++column_index) {
			if (lhs(row_index, column_index) != rhs(row_index, column_index)) { return true; }
		}
	}
	return false;
}

template<typename scalar_t, int rows, int columns>
inline std::ostream& operator<<(std::ostream& out, const Matrix<scalar_t, rows, columns>& matrix) {
	out << std::fixed;
	out << "Matrix: " << rows << "x" << columns << '\n';
	
	for (int row_index = 0; row_index < rows; ++row_index) {
		out << "| ";
		for (int column_index = 0; column_index < columns; ++column_index) {
			out << matrix(row_index, column_index) << " ";
			if (column_index == columns - 1) {
				out << "|";
				if (row_index != rows - 1) {
					out << '\n';
				}
			}
		}
	}
	out << '\n';
	out.unsetf(std::ios::fixed);
	return out;
}

/*** Type definitions ***/

template<typename scalar_t, int rows> using Vector = Matrix<scalar_t, rows, 1>;

template<typename scalar_t> using Vector2 = Vector<scalar_t, 2>;
template<typename scalar_t> using Vector3 = Vector<scalar_t, 3>;
template<typename scalar_t> using Vector4 = Vector<scalar_t, 4>;

using Vector2f = Vector2<float>;
using Vector3f = Vector3<float>;
using Vector4f = Vector4<float>;

using Vector2d = Vector2<double>;
using Vector3d = Vector3<double>;
using Vector4d = Vector4<double>;

using Vector2i = Vector2<int>;
using Vector3i = Vector3<int>;
using Vector4i = Vector4<int>;

template<typename scalar_t, int columns> using RowVector = Matrix<scalar_t, 1, columns>;

template<typename scalar_t> using RowVector2 = RowVector<scalar_t, 2>;
template<typename scalar_t> using RowVector3 = RowVector<scalar_t, 3>;
template<typename scalar_t> using RowVector4 = RowVector<scalar_t, 4>;

using RowVector2f = RowVector2<float>;
using RowVector3f = RowVector3<float>;
using RowVector4f = RowVector4<float>;

using RowVector2d = RowVector2<double>;
using RowVector3d = RowVector3<double>;
using RowVector4d = RowVector4<double>;

using RowVector2i = RowVector2<int>;
using RowVector3i = RowVector3<int>;
using RowVector4i = RowVector4<int>;

template<typename scalar_t> using Matrix2 = Matrix<scalar_t, 2, 2>;
template<typename scalar_t> using Matrix3 = Matrix<scalar_t, 3, 3>;
template<typename scalar_t> using Matrix4 = Matrix<scalar_t, 4, 4>;

using Matrix2f = Matrix2<float>;
using Matrix3f = Matrix3<float>;
using Matrix4f = Matrix4<float>;

using Matrix2d = Matrix2<double>;
using Matrix3d = Matrix3<double>;
using Matrix4d = Matrix4<double>;

using Matrix2i = Matrix2<int>;
using Matrix3i = Matrix3<int>;
using Matrix4i = Matrix4<int>;

/*** Rotation matrices ***/
template<typename scalar_t>
constexpr inline Matrix2<scalar_t> create_2d_rotation_matrix(scalar_t angle) {
	const scalar_t c = std::cos(angle);
	const scalar_t s = std::sin(angle);
	
	return Matrix2<scalar_t>(
		c, -s,
		s,  c
	);
}

template<typename scalar_t>
constexpr inline Matrix3<scalar_t> create_3d_rotation_matrix_x(scalar_t angle) {
	const scalar_t c = std::cos(angle);
	const scalar_t s = std::sin(angle);
	
	return Matrix3<scalar_t>(
		scalar_t(1), scalar_t(0), scalar_t(0),
		scalar_t(0),           c,          -s,
		scalar_t(0),           s,           c
	);
}

template<typename scalar_t>
constexpr inline Matrix3<scalar_t> create_3d_rotation_matrix_y(scalar_t angle) {
	const scalar_t c = std::cos(angle);
	const scalar_t s = std::sin(angle);
	
	return Matrix3<scalar_t>(
		          c, scalar_t(0),           s,
		scalar_t(0), scalar_t(1), scalar_t(0),
		         -s, scalar_t(0),           c
	);
}

template<typename scalar_t>
constexpr inline Matrix3<scalar_t> create_3d_rotation_matrix_z(scalar_t angle) {
	const scalar_t c = std::cos(angle);
	const scalar_t s = std::sin(angle);
	
	return Matrix3<scalar_t>(
		          c,          -s, scalar_t(0),
		          s,           c, scalar_t(0),
		scalar_t(0), scalar_t(0), scalar_t(1)
	);
}

/***** Diagonal Matrix *****/

template<typename scalar_t, int diagonal_size>
class DiagonalMatrix {
public:
	// Static constructors
	static constexpr DiagonalMatrix<scalar_t, diagonal_size> Zero();
	static constexpr DiagonalMatrix<scalar_t, diagonal_size> Identity();
	
	// Constructors
	constexpr DiagonalMatrix<scalar_t, diagonal_size>();
	explicit constexpr DiagonalMatrix<scalar_t, diagonal_size>(const Vector<scalar_t, diagonal_size>& diagonal);
	
	template<typename... scalar_t_var>
	explicit constexpr DiagonalMatrix<scalar_t, diagonal_size>(const scalar_t_var&... v);
	
	// Matrix shape
	[[nodiscard]] constexpr int number_of_rows() const;
	[[nodiscard]] constexpr int number_of_columns() const;
	[[nodiscard]] constexpr int number_of_elements() const;
	
	// Accessors
	Vector<scalar_t, diagonal_size> get_diagonal();
	const Vector<scalar_t, diagonal_size>& get_diagonal() const;
	
	[[nodiscard]]constexpr Matrix<scalar_t, diagonal_size, diagonal_size> create_dense_matrix() const;
	
	constexpr inline scalar_t& operator()(int diagonal_index);
	constexpr inline const scalar_t& operator()(int diagonal_index) const;
	
	[[nodiscard]] scalar_t* data();
	[[nodiscard]] const scalar_t* data() const;
	
private:
	Vector<scalar_t, diagonal_size> m_diagonal;
};

/*** Static constructors implementation ***/

template<typename scalar_t, int diagonal_size>
constexpr inline DiagonalMatrix<scalar_t, diagonal_size> DiagonalMatrix<scalar_t, diagonal_size>::Zero() {
	return DiagonalMatrix<scalar_t, diagonal_size>(Vector<scalar_t, diagonal_size>::Zero());
}

template<typename scalar_t, int diagonal_size>
constexpr inline DiagonalMatrix<scalar_t, diagonal_size> DiagonalMatrix<scalar_t, diagonal_size>::Identity() {
	return DiagonalMatrix<scalar_t, diagonal_size>(Vector<scalar_t, diagonal_size>::Ones());
}

/*** Constructor implementation ***/

template<typename scalar_t, int diagonal_size>
constexpr inline DiagonalMatrix<scalar_t, diagonal_size>::DiagonalMatrix() {
	for (int diagonal_index = 0; diagonal_index < diagonal_size; ++diagonal_index) {
		m_diagonal(diagonal_index) = 0;
	}
}

template<typename scalar_t, int diagonal_size>
constexpr inline DiagonalMatrix<scalar_t, diagonal_size>::DiagonalMatrix(const Vector<scalar_t, diagonal_size>& diagonal) :
m_diagonal{diagonal} { }

template<typename scalar_t, int diagonal_size>
template<typename... scalar_t_var>
constexpr inline DiagonalMatrix<scalar_t, diagonal_size>::DiagonalMatrix(const scalar_t_var& ... v) :
m_diagonal(v...) {
	static_assert(sizeof...(v) == diagonal_size, "Number of constructor parameters does not match the size of the matrix");
}

/*** Matrix shape implementation ***/
template<typename scalar_t, int diagonal_size> constexpr inline int DiagonalMatrix<scalar_t, diagonal_size>::number_of_rows() const { return diagonal_size; }
template<typename scalar_t, int diagonal_size> constexpr inline int DiagonalMatrix<scalar_t, diagonal_size>::number_of_columns() const { return diagonal_size; }
template<typename scalar_t, int diagonal_size> constexpr inline int DiagonalMatrix<scalar_t, diagonal_size>::number_of_elements() const { return diagonal_size*diagonal_size; }

/*** Accessors implementation ***/

template<typename scalar_t, int diagonal_size> inline Vector<scalar_t, diagonal_size> DiagonalMatrix<scalar_t, diagonal_size>::get_diagonal() { return m_diagonal; }
template<typename scalar_t, int diagonal_size> const inline Vector<scalar_t, diagonal_size>& DiagonalMatrix<scalar_t, diagonal_size>::get_diagonal() const { return m_diagonal; }

template<typename scalar_t, int diagonal_size>
constexpr inline Matrix<scalar_t, diagonal_size, diagonal_size> DiagonalMatrix<scalar_t, diagonal_size>::create_dense_matrix() const {
	Matrix<scalar_t, diagonal_size, diagonal_size> dense_matrix = Matrix<scalar_t, diagonal_size, diagonal_size>::Zero();
	for (int diagonal_index = 0; diagonal_index < diagonal_size; ++diagonal_index) {
		dense_matrix(diagonal_index, diagonal_index) = m_diagonal(diagonal_index);
	}
	return dense_matrix;
}

template<typename scalar_t, int diagonal_size>
constexpr inline scalar_t& DiagonalMatrix<scalar_t, diagonal_size>::operator()(int diagonal_index) {
	return m_diagonal(diagonal_index);
}

template<typename scalar_t, int diagonal_size>
constexpr inline const scalar_t& DiagonalMatrix<scalar_t, diagonal_size>::operator()(int diagonal_index) const {
	return m_diagonal(diagonal_index);
}

template<typename scalar_t, int diagonal_size>
inline scalar_t* DiagonalMatrix<scalar_t, diagonal_size>::data() { return m_diagonal.data(); }

template<typename scalar_t, int diagonal_size>
const inline scalar_t* DiagonalMatrix<scalar_t, diagonal_size>::data() const { return m_diagonal.data(); }

/*** operator implementations ***/

/** Scalar DiagonalMatrix operators **/

template<typename scalar_t, int diagonal_size>
constexpr inline DiagonalMatrix<scalar_t, diagonal_size> operator-(const DiagonalMatrix<scalar_t, diagonal_size>& diagonal_matrix) {
	DiagonalMatrix<scalar_t, diagonal_size> result;
	for (int diagonal_index = 0; diagonal_index < diagonal_size; ++diagonal_index) {
		result(diagonal_index) = -diagonal_matrix(diagonal_index);
	}
	return result;
}

template<typename scalar_t, int diagonal_size>
constexpr inline DiagonalMatrix<scalar_t, diagonal_size> operator+(const DiagonalMatrix<scalar_t, diagonal_size>& lhs, const scalar_t& rhs) {
	DiagonalMatrix<scalar_t, diagonal_size> result;
	for (int diagonal_index = 0; diagonal_index < diagonal_size; ++diagonal_index) {
		result(diagonal_index) = lhs(diagonal_index) + rhs;
	}
	return result;
}

template<typename scalar_t, int diagonal_size>
constexpr inline DiagonalMatrix<scalar_t, diagonal_size> operator+(const scalar_t& lhs, const DiagonalMatrix<scalar_t, diagonal_size>& rhs) {
	DiagonalMatrix<scalar_t, diagonal_size> result;
	for (int diagonal_index = 0; diagonal_index < diagonal_size; ++diagonal_index) {
		result(diagonal_index) = lhs + rhs(diagonal_index);
	}
	return result;
}

template<typename scalar_t, int diagonal_size>
constexpr inline DiagonalMatrix<scalar_t, diagonal_size> operator*(const DiagonalMatrix<scalar_t, diagonal_size>& lhs, const scalar_t& rhs) {
	DiagonalMatrix<scalar_t, diagonal_size> result;
	for (int diagonal_index = 0; diagonal_index < diagonal_size; ++diagonal_index) {
		result(diagonal_index) = lhs(diagonal_index) * rhs;
	}
	return result;
}

template<typename scalar_t, int diagonal_size>
constexpr inline DiagonalMatrix<scalar_t, diagonal_size> operator*(const scalar_t& lhs, const DiagonalMatrix<scalar_t, diagonal_size>& rhs) {
	DiagonalMatrix<scalar_t, diagonal_size> result;
	for (int diagonal_index = 0; diagonal_index < diagonal_size; ++diagonal_index) {
		result(diagonal_index) = lhs * rhs(diagonal_index);
	}
	return result;
}

/** DiagonalMatrix Matrix operators **/

template<typename scalar_t, int diagonal_size>
constexpr inline DiagonalMatrix<scalar_t, diagonal_size> operator+(const DiagonalMatrix<scalar_t, diagonal_size>& lhs, const DiagonalMatrix<scalar_t, diagonal_size>& rhs) {
	DiagonalMatrix<scalar_t, diagonal_size> result;
	for (int diagonal_index = 0; diagonal_index < diagonal_size; ++diagonal_index) {
		result(diagonal_index) = lhs(diagonal_index) + rhs(diagonal_index);
	}
	return result;
}

template<typename scalar_t, int diagonal_size>
constexpr inline Matrix<scalar_t, diagonal_size, diagonal_size> operator+(const Matrix<scalar_t, diagonal_size, diagonal_size>& lhs, const DiagonalMatrix<scalar_t, diagonal_size>& rhs) {
	Matrix<scalar_t, diagonal_size, diagonal_size> result = lhs;
	for (int diagonal_index = 0; diagonal_index < diagonal_size; ++diagonal_index) {
		result(diagonal_index, diagonal_index) += rhs(diagonal_index);
	}
	return result;
}

template<typename scalar_t, int diagonal_size>
constexpr inline Matrix<scalar_t, diagonal_size, diagonal_size> operator+(const DiagonalMatrix<scalar_t, diagonal_size>& lhs, const Matrix<scalar_t, diagonal_size, diagonal_size>& rhs) {
	Matrix<scalar_t, diagonal_size, diagonal_size> result = rhs;
	for (int diagonal_index = 0; diagonal_index < diagonal_size; ++diagonal_index) {
		result(diagonal_index, diagonal_index) += lhs(diagonal_index);
	}
	return result;
}

template<typename scalar_t, int diagonal_size>
constexpr inline DiagonalMatrix<scalar_t, diagonal_size> operator*(const DiagonalMatrix<scalar_t, diagonal_size>& lhs, const DiagonalMatrix<scalar_t, diagonal_size>& rhs) {
	DiagonalMatrix<scalar_t, diagonal_size> result;
	for (int diagonal_index = 0; diagonal_index < diagonal_size; ++diagonal_index) {
		result(diagonal_index) = lhs(diagonal_index) * rhs(diagonal_index);
	}
	return result;
}

template<typename scalar_t, int diagonal_size, int rows>
constexpr inline Matrix<scalar_t, rows, diagonal_size> operator*(const Matrix<scalar_t, rows, diagonal_size>& lhs, const DiagonalMatrix<scalar_t, diagonal_size>& rhs) {
	Matrix<scalar_t, rows, diagonal_size> result = lhs;
	for (int row_index = 0; row_index < rows; ++row_index) {
		for (int diagonal_index = 0; diagonal_index < diagonal_size; ++diagonal_index) {
			result(row_index, diagonal_index) *= rhs(diagonal_index);
		}
	}
	return result;
}

template<typename scalar_t, int diagonal_size, int columns>
constexpr inline Matrix<scalar_t, diagonal_size, columns> operator*(const DiagonalMatrix<scalar_t, diagonal_size>& lhs, const Matrix<scalar_t, diagonal_size, columns>& rhs) {
	Matrix<scalar_t, diagonal_size, diagonal_size> result = rhs;
	for (int diagonal_index = 0; diagonal_index < diagonal_size; ++diagonal_index) {
		for (int column_index = 0; column_index < columns; ++column_index) {
			result(diagonal_index, column_index) *= lhs(diagonal_index);
		}
	}
	return result;
}

/** Type definitions **/

template<typename scalar_t> using DiagonalMatrix2 = DiagonalMatrix<scalar_t, 2>;
template<typename scalar_t> using DiagonalMatrix3 = DiagonalMatrix<scalar_t, 3>;
template<typename scalar_t> using DiagonalMatrix4 = DiagonalMatrix<scalar_t, 4>;

using DiagonalMatrix2f = DiagonalMatrix2<float>;
using DiagonalMatrix3f = DiagonalMatrix3<float>;
using DiagonalMatrix4f = DiagonalMatrix4<float>;

using DiagonalMatrix2d = DiagonalMatrix2<double>;
using DiagonalMatrix3d = DiagonalMatrix3<double>;
using DiagonalMatrix4d = DiagonalMatrix4<double>;

using DiagonalMatrix2i = DiagonalMatrix2<int>;
using DiagonalMatrix3i = DiagonalMatrix3<int>;
using DiagonalMatrix4i = DiagonalMatrix4<int>;

/***** Quaternion ******/

template<typename scalar_t>
struct Quaternion {
	// Static constructors
	static constexpr Quaternion<scalar_t> Identity();
	
	// Constructors
	constexpr Quaternion();
	constexpr Quaternion(const scalar_t& w, const scalar_t& x, const scalar_t& y, const scalar_t& z);
	constexpr explicit Quaternion(const Vector4<scalar_t>& parameter_vector);
	constexpr Quaternion(const Vector3<scalar_t>& axis, const scalar_t& angle);
	explicit constexpr Quaternion(const Matrix3<scalar_t>& rotation_matrix);
	
	// Norms
	[[nodiscard]] constexpr scalar_t squared_norm() const;
	[[nodiscard]] constexpr scalar_t norm() const;
	[[nodiscard]] constexpr bool is_normalized() const;
	constexpr void normalize();
	[[nodiscard]] constexpr inline Quaternion<scalar_t> normalized() const;
	
	
	// Conversions
	[[nodiscard]] constexpr Quaternion<scalar_t> conjugate() const;
	[[nodiscard]] constexpr Matrix3<scalar_t> rotation_matrix() const;
	[[nodiscard]] constexpr Matrix4<scalar_t> transformation_matrix() const;
	
	// Member variables
	scalar_t w;
	scalar_t x;
	scalar_t y;
	scalar_t z;
};

/*** Static constructors implementation ***/
template<typename scalar_t>
constexpr inline Quaternion<scalar_t> Quaternion<scalar_t>::Identity() {
	Quaternion<scalar_t> q(scalar_t(1), scalar_t(0), scalar_t(0), scalar_t(0));
	return q;
}

/*** Constructors implementation ***/
template<typename scalar_t>
constexpr inline Quaternion<scalar_t>::Quaternion() :
w{1}, x{0}, y{0}, z{0}{
	static_assert(std::is_floating_point_v<scalar_t>, "Quaternions are only defined for floating point types.");
}

template<typename scalar_t>
constexpr inline Quaternion<scalar_t>::Quaternion(const scalar_t& w, const scalar_t& x, const scalar_t& y, const scalar_t& z) :
w{w}, x{x}, y{y}, z{z}{
	static_assert(std::is_floating_point_v<scalar_t>, "Quaternions are only defined for floating point types.");
}

template<typename scalar_t>
constexpr inline Quaternion<scalar_t>::Quaternion(const Vector4<scalar_t>& parameter_vector) :
w{parameter_vector(0)}, x{parameter_vector(1)}, y{parameter_vector(2)}, z{parameter_vector(3)}
{
	static_assert(std::is_floating_point_v<scalar_t>, "Quaternions are only defined for floating point types.");
}

template<typename scalar_t>
constexpr inline Quaternion<scalar_t>::Quaternion(const Vector3<scalar_t>& axis, const scalar_t& angle) {
	static_assert(std::is_floating_point_v<scalar_t>, "Quaternions are only defined for floating point types.");
	const Vector3<scalar_t> axis_normalized = axis.normalized();
	const float s = std::sin(scalar_t(0.5) * angle);
	w = std::cos(scalar_t(0.5) * angle);
	x = axis_normalized.x() * s;
	y = axis_normalized.y() * s;
	z = axis_normalized.z() * s;
}

template<typename scalar_t>
constexpr inline Quaternion<scalar_t>::Quaternion(const Matrix3<scalar_t>& rotation_matrix) {
	static_assert(std::is_floating_point_v<scalar_t>, "Quaternions are only defined for floating point types.");
	const auto& r = rotation_matrix;
	
	if (const scalar_t trace = r.trace() + scalar_t(1); trace > std::numeric_limits<scalar_t>::epsilon()) {
		const float s = scalar_t(0.5)/std::sqrt(trace);
		w = scalar_t(0.25) / s;
		x = (r(2, 1) - r(1, 2)) * s;
		y = (r(0, 2) - r(2, 0)) * s;
		z = (r(1, 0) - r(0, 1)) * s;
	} else if ((r(0, 0) > r(1, 1)) and (r(0, 0) > r(2, 2))) {
		const float s = 2*std::sqrt(scalar_t(1) + r(0, 0) - r(1, 1) - r(2, 2));
		w = (r(2, 1) - r(1, 2)) / s;
		x = scalar_t(0.25) * s;
		y = (r(0, 1) + r(1, 0)) / s;
		z = (r(0, 2) + r(2, 0)) / s;
	} else if (r(1, 1) > r(2, 2)) {
		const float s = 2*std::sqrt(scalar_t(1) + r(1, 1) - r(0, 0) - r(2, 2));
		w = (r(0, 2) - r(2, 0)) / s;
		x = (r(0, 1) - r(1, 0)) / s;
		y = scalar_t(0.25) * s;
		z = (r(1, 2) + r(2, 1)) / s;
	} else {
		const float s = 2*std::sqrt(scalar_t(1) + r(2, 2) - r(0, 0) - r(1, 1));
		w = (r(1, 0) - r(0, 1)) / s;
		x = (r(0, 2) - r(2, 0)) / s;
		y = (r(1, 2) - r(2, 1)) / s;
		z = scalar_t(0.25) * s;
	}
}
/*** Norms implementation ***/
template<typename scalar_t>
constexpr inline scalar_t Quaternion<scalar_t>::squared_norm() const {
	return w*w + x*x + y*y + z*z;
}

template<typename scalar_t>
constexpr inline scalar_t Quaternion<scalar_t>::norm() const {
	return std::sqrt(squared_norm());
}

template<typename scalar_t>
constexpr inline bool Quaternion<scalar_t>::is_normalized() const {
	return (scalar_t(1) - std::sqrt(w*w + x*x + y*y + z*z)) <= normalized_sensitivity<scalar_t>;
}

template<typename scalar_t>
constexpr inline void Quaternion<scalar_t>::normalize() {
	const scalar_t multiplier = scalar_t(1)/std::sqrt(w*w + x*x + y*y + z*z);
	w *= multiplier;
	x *= multiplier;
	y *= multiplier;
	z *= multiplier;
}

template<typename scalar_t>
constexpr inline Quaternion<scalar_t> Quaternion<scalar_t>::normalized() const {
	const scalar_t multiplier = scalar_t(1)/std::sqrt(x*x + y*y + z*z + w*w);
	return Quaternion<scalar_t>(x*multiplier, y*multiplier, z*multiplier, w*multiplier);
}

/*** Conversions implementations ***/

template<typename scalar_t>
constexpr inline Quaternion<scalar_t> Quaternion<scalar_t>::conjugate() const {
	return Quaternion<scalar_t>(w, -x, -y, -z);
}

template<typename scalar_t>
constexpr inline Matrix3<scalar_t> Quaternion<scalar_t>::rotation_matrix() const {
	return Matrix3<scalar_t>(
		scalar_t(1) - scalar_t(2)*(y*y + z*z),               scalar_t(2)*(x*y - z*w),               scalar_t(2)*(x*z + y*w),
		              scalar_t(2)*(x*y + z*w), scalar_t(1) - scalar_t(2)*(x*x + z*z),               scalar_t(2)*(y*z - x*w),
		              scalar_t(2)*(x*z - y*w),               scalar_t(2)*(y*z + x*w), scalar_t(1) - scalar_t(2)*(x*x + y*y)
	);
}

template<typename scalar_t>
constexpr inline Matrix4<scalar_t> Quaternion<scalar_t>::transformation_matrix() const {
	return Matrix4<scalar_t>(
			scalar_t(1) - scalar_t(2)*(y*y + z*z),               scalar_t(2)*(x*y - z*w),               scalar_t(2)*(x*z + y*w), scalar_t(0),
			              scalar_t(2)*(x*y + z*w), scalar_t(1) - scalar_t(2)*(x*x + z*z),               scalar_t(2)*(y*z - x*w), scalar_t(0),
			              scalar_t(2)*(x*z - y*w),               scalar_t(2)*(y*z + x*w), scalar_t(1) - scalar_t(2)*(x*x + y*y), scalar_t(0),
			                          scalar_t(0),                           scalar_t(0),                           scalar_t(0), scalar_t(1)
	);
}

/*** Operator implementations ***/
template<typename scalar_t>
constexpr inline Quaternion<scalar_t> operator*(const Quaternion<scalar_t>& lhs, const Quaternion<scalar_t>& rhs) {
	return Quaternion<scalar_t>(
			lhs.w*rhs.w - lhs.x*rhs.x - lhs.y*rhs.y - lhs.z*rhs.z,
			lhs.w*rhs.x + lhs.x*rhs.w + lhs.y*rhs.z - lhs.z*rhs.y,
			lhs.w*rhs.y - lhs.x*rhs.z + lhs.y*rhs.w + lhs.z*rhs.x,
			lhs.w*rhs.z + lhs.x*rhs.y - lhs.y*rhs.x + lhs.z*rhs.w
	);
}

template<typename scalar_t>
constexpr inline Vector3<scalar_t> operator*(const Quaternion<scalar_t>& orientation, const Vector3<scalar_t>& point) {
	Vector3<scalar_t> u(orientation.x, orientation.y, orientation.z);
	const scalar_t& s = orientation.w;
	return scalar_t(2)*u.dot_product(point)*u + (s*s - u.dot_product(u))*point + scalar_t(2)*s*u.cross_product(point);
}

template<typename scalar_t>
inline std::ostream& operator<<(std::ostream& out, const Quaternion<scalar_t>& quaternion) {
	out << std::fixed;
	out << "Quaternion: " << '\n';
	
	out << "(" << quaternion.w << " + " << quaternion.x << "*i + " << quaternion.y << "*j + " << quaternion.z << "*k)\n";
	
	out.unsetf(std::ios::fixed);
	return out;
}

/*** Type definitions ***/
using Quaternionf = Quaternion<float>;
using Quaterniond = Quaternion<double>;


/***** Rigid Transform 2D *****/

template<typename scalar_t>
struct RigidTransform2D {
	// Static constructors
	static constexpr RigidTransform2D<scalar_t> Identity();
	
	// Constructors
	constexpr RigidTransform2D();
	constexpr explicit RigidTransform2D(const scalar_t& rotation, const Vector2<scalar_t>& translation = Vector2<scalar_t>::Zero());
	
	// Accessors
	[[nodiscard]] constexpr RigidTransform2D<scalar_t> inverse() const;
	[[nodiscard]] constexpr Matrix3<scalar_t> transformation_matrix() const;
	[[nodiscard]] constexpr Matrix4<scalar_t> transformation_matrix_3d() const;
	
	scalar_t rotation;
	Vector2<scalar_t> translation;
};

/*** Static constructor implementation ***/

template<typename scalar_t>
constexpr inline RigidTransform2D<scalar_t> RigidTransform2D<scalar_t>::Identity() {
	return RigidTransform2D<scalar_t>();
}

/*** Constructors implementation ***/

template<typename scalar_t>
constexpr inline RigidTransform2D<scalar_t>::RigidTransform2D() :
rotation{0}, translation{Vector2<scalar_t>::Zero()} { }

template<typename scalar_t>
constexpr inline RigidTransform2D<scalar_t>::RigidTransform2D(const scalar_t& rotation, const Vector2<scalar_t>& translation) :
rotation{rotation}, translation{translation} { }

template<typename scalar_t>
constexpr inline RigidTransform2D<scalar_t> RigidTransform2D<scalar_t>::inverse() const {
	return RigidTransform2D<scalar_t>(-rotation, -create_2d_rotation_matrix(-rotation) * translation);
}

template<typename scalar_t>
constexpr inline Matrix3<scalar_t> RigidTransform2D<scalar_t>::transformation_matrix() const {
	const scalar_t c = std::cos(rotation);
	const scalar_t s = std::sin(rotation);
	
	return Matrix3<scalar_t>(
		          c,          -s, translation.x(),
		          s,           c, translation.y(),
		scalar_t(0), scalar_t(0),     scalar_t(1)
	);
}

template<typename scalar_t>
constexpr inline Matrix4<scalar_t> RigidTransform2D<scalar_t>::transformation_matrix_3d() const {
	const scalar_t c = std::cos(rotation);
	const scalar_t s = std::sin(rotation);
	
	return Matrix4<scalar_t>(
		          c,          -s, scalar_t(0), translation.x(),
		          s,           c, scalar_t(0), translation.y(),
		scalar_t(0), scalar_t(0), scalar_t(1),     scalar_t(0),
		scalar_t(0), scalar_t(0), scalar_t(0),     scalar_t(1)
	);
}

template<typename scalar_t>
constexpr inline Vector2<scalar_t> operator*(const RigidTransform2D<scalar_t>& pose, const Vector2<scalar_t>& point) {
	return create_2d_rotation_matrix(pose.rotation) * point + pose.translation;
}

template<typename scalar_t>
constexpr inline RigidTransform2D<scalar_t> operator*(const RigidTransform2D<scalar_t>& lhs, const RigidTransform2D<scalar_t>& rhs) {
	return RigidTransform2D<scalar_t>(
			lhs.rotation + rhs.rotation, lhs.translation + create_2d_rotation_matrix(lhs.rotation) * rhs.translation
	);
}

using RigidTransform2Df = RigidTransform2D<float>;
using RigidTransform2Dd = RigidTransform2D<double>;

/***** Scaled Transform 2D *****/

template<typename scalar_t>
struct ScaledTransform2D {
	// Static constructors
	static constexpr ScaledTransform2D<scalar_t> Identity();
	
	// Constructor
	constexpr ScaledTransform2D();
	constexpr explicit ScaledTransform2D(
			const scalar_t& rotation,
			const Vector2<scalar_t>& translation = Vector2<scalar_t>::Zero(),
			const Vector2<scalar_t>& scale = Vector2<scalar_t>::Ones()
	);
	
	// Accessors
	[[nodiscard]] constexpr Matrix3<scalar_t> transformation_matrix() const;
	[[nodiscard]] constexpr Matrix3<scalar_t> transformation_matrix_inverse() const;
	[[nodiscard]] constexpr Matrix4<scalar_t> transformation_matrix_3d() const;
	[[nodiscard]] constexpr Matrix4<scalar_t> transformation_matrix_inverse_3d() const;
	
	scalar_t rotation;
	Vector2<scalar_t> translation;
	Vector2<scalar_t> scale;
};

/*** Static constructor implementation ***/

template<typename scalar_t>
constexpr inline ScaledTransform2D<scalar_t> ScaledTransform2D<scalar_t>::Identity() {
	return ScaledTransform2D<scalar_t>();
}

/*** Constructor implementation ***/

template<typename scalar_t>
constexpr inline ScaledTransform2D<scalar_t>::ScaledTransform2D() :
rotation{0}, translation{Vector2<scalar_t>::Zero()}, scale{Vector2<scalar_t>::Ones()} { }

template<typename scalar_t>
constexpr inline ScaledTransform2D<scalar_t>::ScaledTransform2D(const scalar_t& rotation, const Vector2<scalar_t>& translation, const Vector2<scalar_t>& scale) :
rotation{rotation}, translation{translation}, scale{scale} { }

/*** Accessors implementation ***/

template<typename scalar_t>
constexpr inline Matrix3<scalar_t> ScaledTransform2D<scalar_t>::transformation_matrix() const {
	const Matrix2<scalar_t> r = create_2d_rotation_matrix(rotation);
	const auto& x = translation.x();
	const auto& y = translation.y();
	const auto& sx = scale.x();
	const auto& sy = scale.y();
	
	return Matrix3<scalar_t>(
			 r(0, 0)*sx,  r(0, 1)*sy,           x,
			 r(1, 0)*sx,  r(1, 1)*sy,           y,
			scalar_t(0), scalar_t(0), scalar_t(1)
	);
}

template<typename scalar_t>
constexpr inline Matrix3<scalar_t> ScaledTransform2D<scalar_t>::transformation_matrix_inverse() const {
	const Matrix2<scalar_t> r = create_2d_rotation_matrix(rotation);
	const auto& x = translation.x();
	const auto& y = translation.y();
	const auto& sx_r = scalar_t(1) / scale.x();
	const auto& sy_r = scalar_t(1) / scale.y();
	
	return Matrix3<scalar_t>(
			r(0, 0)*sx_r,  r(1, 0)*sx_r, - r(0, 0)*x*sx_r - r(1, 0)*y*sx_r,
			r(0, 1)*sy_r,  r(1, 1)*sy_r, - r(0, 1)*x*sy_r - r(1, 1)*y*sy_r,
			 scalar_t(0),   scalar_t(0),                       scalar_t(1)
	);
}

template<typename scalar_t>
constexpr inline Matrix4<scalar_t> ScaledTransform2D<scalar_t>::transformation_matrix_3d() const {
	const Matrix2<scalar_t> r = create_2d_rotation_matrix(rotation);
	const auto& x = translation.x();
	const auto& y = translation.y();
	const auto& sx = scale.x();
	const auto& sy = scale.y();
	
	return Matrix4<scalar_t>(
			 r(0, 0)*sx,  r(0, 1)*sy, scalar_t(0),           x,
			 r(1, 0)*sx,  r(1, 1)*sy, scalar_t(0),           y,
			scalar_t(0), scalar_t(0), scalar_t(1), scalar_t(0),
			scalar_t(0), scalar_t(0), scalar_t(0), scalar_t(1)
	);
}

template<typename scalar_t>
constexpr inline Matrix4<scalar_t> ScaledTransform2D<scalar_t>::transformation_matrix_inverse_3d() const {
	const Matrix2<scalar_t> r = create_2d_rotation_matrix(rotation);
	const auto& x = translation.x();
	const auto& y = translation.y();
	const auto& sx_r = scalar_t(1) / scale.x();
	const auto& sy_r = scalar_t(1) / scale.y();
	
	return Matrix4<scalar_t>(
			r(0, 0)*sx_r, r(1, 0)*sx_r, scalar_t(0), - r(0, 0)*x*sx_r - r(1, 0)*y*sx_r,
			r(0, 1)*sy_r, r(1, 1)*sy_r, scalar_t(0), - r(0, 1)*x*sy_r - r(1, 1)*y*sy_r,
			scalar_t(0),   scalar_t(0), scalar_t(1),                       scalar_t(0),
			scalar_t(0),   scalar_t(0), scalar_t(0),                       scalar_t(1)
	);
}

template<typename scalar_t>
constexpr inline Vector2<scalar_t> operator*(const ScaledTransform2D<scalar_t>& scaled_transform_2d, const Vector2<scalar_t>& point) {
	const auto& px = point.x();
	const auto& py = point.y();
	const auto& sx = scaled_transform_2d.scale.x();
	const auto& sy = scaled_transform_2d.scale.y();
	return create_2d_rotation_matrix(scaled_transform_2d.rotation) * Vector2<scalar_t>(sx*px, sy*py) + scaled_transform_2d.translation;
}

using ScaledTransform2Df = ScaledTransform2D<float>;
using ScaledTransform2Dd = ScaledTransform2D<double>;

/***** Rigid Transform 3D *****/

template<typename scalar_t>
struct RigidTransform3D {
	// Static constructors
	static constexpr RigidTransform3D<scalar_t> Identity();
	
	// Constructors
	constexpr RigidTransform3D();
	constexpr RigidTransform3D(const Quaternion<scalar_t>& orientation, const Vector3<scalar_t>& translation);
	
	// Accessors
	[[nodiscard]] constexpr inline RigidTransform3D<scalar_t> inverse() const;
	[[nodiscard]] constexpr inline Matrix4<scalar_t> transformation_matrix() const;
	
	Quaternion<scalar_t> orientation = Quaternion<scalar_t>::Identity();
	Vector3<scalar_t> translation = Vector3<scalar_t>::Zero();
};

/*** Static constructors implementation ***/

template<typename scalar_t>
constexpr inline RigidTransform3D<scalar_t> RigidTransform3D<scalar_t>::Identity() {
	return RigidTransform3D<scalar_t>();
}

/*** Constructors implementation ***/

template<typename scalar_t>
constexpr inline RigidTransform3D<scalar_t>::RigidTransform3D() :
orientation{Quaternion<scalar_t>::Identity()}, translation{Vector3<scalar_t>::Zero()} { }

template<typename scalar_t>
constexpr inline RigidTransform3D<scalar_t>::RigidTransform3D(const Quaternion<scalar_t>& orientation, const Vector3<scalar_t>& translation) :
orientation{orientation}, translation{translation} { }

template<typename scalar_t>
constexpr inline RigidTransform3D<scalar_t> RigidTransform3D<scalar_t>::inverse() const {
	return RigidTransform3D<scalar_t>(
			orientation.conjugate(),
			-(orientation.conjugate() * translation)
	);
}

template<typename scalar_t>
constexpr inline Matrix4<scalar_t> RigidTransform3D<scalar_t>::transformation_matrix() const {
	const Matrix3<scalar_t> r = orientation.rotation_matrix();
	return Matrix4<scalar_t>(
			    r(0, 0),     r(0, 1),     r(0, 2), translation.x(),
			    r(1, 0),     r(1, 1),     r(1, 2), translation.y(),
			    r(2, 0),     r(2, 1),     r(2, 2), translation.z(),
			scalar_t(0), scalar_t(0), scalar_t(0),     scalar_t(1)
	);
}

template<typename scalar_t>
constexpr inline RigidTransform3D<scalar_t> operator*(const RigidTransform3D<scalar_t>& lhs, const RigidTransform3D<scalar_t>& rhs) {
	return RigidTransform3D<scalar_t>(
			lhs.orientation * rhs.orientation,
			lhs.translation + lhs.orientation*rhs.translation
	);
}

template<typename scalar_t>
constexpr inline RigidTransform3D<scalar_t> operator*(const RigidTransform3D<scalar_t>& rigid_transform, const Vector3<scalar_t>& point) {
	return rigid_transform.orientation * point + rigid_transform.translation;
}

using RigidTransform3Df = RigidTransform3D<float>;
using RigidTransform3Dd = RigidTransform3D<double>;


/***** Scaled Transform 3D *****/

template<typename scalar_t>
struct ScaledTransform3D {
	// Static constructors
	static constexpr ScaledTransform3D<scalar_t> Identity();
	
	// Constructor
	constexpr ScaledTransform3D();
	constexpr explicit ScaledTransform3D(
			const Quaternion<scalar_t>& orientation,
			const Vector3<scalar_t>& translation = Vector3<scalar_t>::Zero(),
			const Vector3<scalar_t>& scale = Vector3<scalar_t>::Ones()
	);
	
	// Accessors
	[[nodiscard]] constexpr Matrix4<scalar_t> transformation_matrix() const;
	[[nodiscard]] constexpr Matrix4<scalar_t> transformation_matrix_inverse() const;
	
	Quaternion<scalar_t> orientation;
	Vector3<scalar_t> translation;
	Vector3<scalar_t> scale;
};

/*** Static constructors implementation ***/

template<typename scalar_t>
constexpr inline ScaledTransform3D<scalar_t> ScaledTransform3D<scalar_t>::Identity() {
	return ScaledTransform3D<scalar_t>();
}

/*** Constructors implementation ***/

template<typename scalar_t>
constexpr inline ScaledTransform3D<scalar_t>::ScaledTransform3D() :
orientation{Quaternion<scalar_t>::Identity()}, translation{Vector3<scalar_t>::Zero()}, scale{Vector3<scalar_t>::Ones()} { }

template<typename scalar_t>
constexpr inline ScaledTransform3D<scalar_t>::ScaledTransform3D(const Quaternion<scalar_t>& orientation, const Vector3<scalar_t>& translation, const Vector3<scalar_t>& scale) :
orientation{orientation}, translation{translation}, scale{scale} { }

/*** Accessors implementation ***/

template<typename scalar_t>
constexpr inline Matrix4<scalar_t> ScaledTransform3D<scalar_t>::transformation_matrix() const {
	const Matrix3<scalar_t> r = orientation.rotation_matrix();
	const auto& x = translation.x();
	const auto& y = translation.y();
	const auto& z = translation.z();
	const auto& sx = scale.x();
	const auto& sy = scale.y();
	const auto& sz = scale.z();
	
	return Matrix4<scalar_t>(
			 r(0, 0)*sx,  r(0, 1)*sy,  r(0, 2)*sz,           x,
			 r(1, 0)*sx,  r(1, 1)*sy,  r(1, 2)*sz,           y,
			 r(2, 0)*sx,  r(2, 1)*sy,  r(2, 2)*sz,           z,
			scalar_t(0), scalar_t(0), scalar_t(0), scalar_t(1)
	);
}

template<typename scalar_t>
constexpr inline Matrix4<scalar_t> ScaledTransform3D<scalar_t>::transformation_matrix_inverse() const {
	const Matrix3<scalar_t> r = orientation.rotation_matrix();
	const auto& x = translation.x();
	const auto& y = translation.y();
	const auto& z = translation.z();
	const auto& sx_r = scalar_t(1) / scale.x();
	const auto& sy_r = scalar_t(1) / scale.y();
	const auto& sz_r = scalar_t(1) / scale.z();
	
	return Matrix4<scalar_t>(
			r(0, 0)*sx_r,  r(1, 0)*sx_r,  r(2, 0)*sx_r, - r(0, 0)*x*sx_r - r(1, 0)*y*sx_r - r(2, 0)*z*sx_r,
			r(0, 1)*sy_r,  r(1, 1)*sy_r,  r(2, 1)*sy_r, - r(0, 1)*x*sy_r - r(1, 1)*y*sy_r - r(2, 1)*z*sy_r,
			r(0, 2)*sz_r,  r(1, 2)*sz_r,  r(2, 2)*sz_r, - r(0, 2)*x*sz_r - r(1, 2)*y*sz_r - r(2, 2)*z*sz_r,
			 scalar_t(0),   scalar_t(0),   scalar_t(0),                                        scalar_t(1)
	);
}

template<typename scalar_t>
constexpr inline Vector3<scalar_t> operator*(const ScaledTransform3D<scalar_t>& scaled_transform_3d, const Vector3<scalar_t>& point) {
	const auto& px = point.x();
	const auto& py = point.y();
	const auto& pz = point.z();
	const auto& sx = scaled_transform_3d.scale.x();
	const auto& sy = scaled_transform_3d.scale.y();
	const auto& sz = scaled_transform_3d.scale.z();
	return scaled_transform_3d.orientation * Vector3<scalar_t>(sx*px, sy*py, sz*pz) + scaled_transform_3d.translation;
}

using ScaledTransform3Df = ScaledTransform3D<float>;
using ScaledTransform3Dd = ScaledTransform3D<double>;

}
