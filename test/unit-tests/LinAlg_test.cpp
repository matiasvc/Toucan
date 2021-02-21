#include <catch2/catch.hpp>

#include <Toucan/LinAlg.h>

#include <iostream>
#include <cmath>

#include <Eigen/Core>
#include <Eigen/Geometry>

template<typename scalar_type> using EigenVector2 = Eigen::Matrix<scalar_type, 2, 1>;
template<typename scalar_type> using EigenVector3 = Eigen::Matrix<scalar_type, 3, 1>;
template<typename scalar_type> using EigenVector4 = Eigen::Matrix<scalar_type, 4, 1>;


// Toucan-Eigen matrix equality operator
template<typename scalar_type, int rows, int columns>
bool operator==(const Toucan::Matrix<scalar_type, rows, columns>& toucan_matrix, const Eigen::Matrix<scalar_type, rows, columns>& eigen_matrix) {
	
	for (int row_index = 0; row_index < rows; ++row_index) {
		for (int column_index = 0; column_index < columns; ++column_index) {
			if (toucan_matrix(row_index, column_index) != eigen_matrix(row_index, column_index)) {
				return false;
			}
		}
	}
	
	return true;
}

// Toucan-Eigen matrix inequality operator
template<typename scalar_type, int rows, int columns>
bool operator!=(const Toucan::Matrix<scalar_type, rows, columns>& toucan_matrix, const Eigen::Matrix<scalar_type, rows, columns>& eigen_matrix) {
	
	for (int row_index = 0; row_index < rows; ++row_index) {
		for (int column_index = 0; column_index < columns; ++column_index) {
			if (toucan_matrix(row_index, column_index) != eigen_matrix(row_index, column_index)) {
				return true;
			}
		}
	}
	
	return false;
}

TEMPLATE_TEST_CASE("Matrix static constructors", "[matrix][static_constructor]", float, double, int) {
	
	SECTION("Zero") {
		Toucan::Matrix4<TestType> matrix_zero_toucan = Toucan::Matrix4<TestType>::Zero();
		Eigen::Matrix<TestType, 4, 4> matrix_zero_eigen = Eigen::Matrix<TestType, 4, 4>::Zero();
		REQUIRE((matrix_zero_toucan == matrix_zero_eigen));
	}
	
	SECTION("Ones") {
		Toucan::Matrix4<TestType> matrix_ones_toucan = Toucan::Matrix4<TestType>::Ones();
		Eigen::Matrix<TestType, 4, 4> matrix_ones_eigen = Eigen::Matrix<TestType, 4, 4>::Ones();
		
		REQUIRE((matrix_ones_toucan == matrix_ones_eigen));
	}
	
	SECTION("Identity") {
		Toucan::Matrix4<TestType> matrix_identity_toucan = Toucan::Matrix4<TestType>::Identity();
		Eigen::Matrix<TestType, 4, 4> matrix_identity_eigen = Eigen::Matrix<TestType, 4, 4>::Identity();
		
		REQUIRE((matrix_identity_toucan == matrix_identity_eigen));
	}
	
	SECTION("UnitX") {
		Toucan::Vector2<TestType> v2 = Toucan::Vector2<TestType>::UnitX();
		REQUIRE(v2.x() == Approx(1));
		REQUIRE(v2.y() == Approx(0));
		
		Toucan::Vector3<TestType> v3 = Toucan::Vector3<TestType>::UnitX();
		REQUIRE(v3.x() == Approx(1));
		REQUIRE(v3.y() == Approx(0));
		REQUIRE(v3.z() == Approx(0));
	}
	
	SECTION("UnitX RowVector") {
		Toucan::RowVector2<TestType> v2 = Toucan::RowVector2<TestType>::UnitX();
		REQUIRE(v2.x() == Approx(1));
		REQUIRE(v2.y() == Approx(0));
		
		Toucan::RowVector3<TestType> v3 = Toucan::RowVector3<TestType>::UnitX();
		REQUIRE(v3.x() == Approx(1));
		REQUIRE(v3.y() == Approx(0));
		REQUIRE(v3.z() == Approx(0));
	}
	
	SECTION("UnitY") {
		Toucan::Vector2<TestType> v2 = Toucan::Vector2<TestType>::UnitY();
		REQUIRE(v2.x() == Approx(0));
		REQUIRE(v2.y() == Approx(1));
		
		Toucan::Vector3<TestType> v3 = Toucan::Vector3<TestType>::UnitY();
		REQUIRE(v3.x() == Approx(0));
		REQUIRE(v3.y() == Approx(1));
		REQUIRE(v3.z() == Approx(0));
	}
	
	SECTION("UnitY RowVector") {
		Toucan::RowVector2<TestType> v2 = Toucan::RowVector2<TestType>::UnitY();
		REQUIRE(v2.x() == Approx(0));
		REQUIRE(v2.y() == Approx(1));
		
		Toucan::RowVector3<TestType> v3 = Toucan::RowVector3<TestType>::UnitY();
		REQUIRE(v3.x() == Approx(0));
		REQUIRE(v3.y() == Approx(1));
		REQUIRE(v3.z() == Approx(0));
	}
	
	SECTION("UnitZ") {
		Toucan::Vector3<TestType> v3 = Toucan::Vector3<TestType>::UnitZ();
		REQUIRE(v3.x() == Approx(0));
		REQUIRE(v3.y() == Approx(0));
		REQUIRE(v3.z() == Approx(1));
	}
	
	SECTION("UnitZ RowVector") {
		Toucan::RowVector3<TestType> v3 = Toucan::RowVector3<TestType>::UnitZ();
		REQUIRE(v3.x() == Approx(0));
		REQUIRE(v3.y() == Approx(0));
		REQUIRE(v3.z() == Approx(1));
	}
	
	SECTION("UnitN") {
		Toucan::Matrix<TestType, 32, 1> v = Toucan::Matrix<TestType, 32, 1>::UnitN(10);
		for (int index = 0; index < 32; ++index) {
			if (index == 10) {
				REQUIRE(v(index) == Approx(1));
			} else {
				REQUIRE(v(index) == Approx(0));
			}
		}
	}
	
	SECTION("UnitN RowVector") {
		Toucan::Matrix<TestType, 1, 32> v = Toucan::Matrix<TestType, 1, 32>::UnitN(10);
		for (int index = 0; index < 32; ++index) {
			if (index == 10) {
				REQUIRE(v(index) == Approx(1));
			} else {
				REQUIRE(v(index) == Approx(0));
			}
		}
	}
}

TEMPLATE_TEST_CASE("Matrix default constructor", "[matrix][constructor]", float, double, int) {
	Toucan::Vector3<TestType> v3;
	REQUIRE(v3.x() == Approx(0));
	REQUIRE(v3.y() == Approx(0));
	REQUIRE(v3.z() == Approx(0));
}


TEMPLATE_TEST_CASE("Matrix constructors", "[matrix][constructor]", float, double, int) {
	
	Toucan::Matrix3<TestType> matrix_toucan(
		TestType(1), TestType(2), TestType(3),
		TestType(4), TestType(5), TestType(6),
		TestType(7), TestType(8), TestType(9)
	);
	Eigen::Matrix<TestType, 3, 3> matrix_eigen;
	matrix_eigen << TestType(1), TestType(2), TestType(3),
	                TestType(4), TestType(5), TestType(6),
	                TestType(7), TestType(8), TestType(9);
	
	REQUIRE((matrix_toucan == matrix_eigen));
	
	// TODO(Matias): Check that a invalid number of parameters to a Toucan Matrix constructor fails at compile time.
}

TEMPLATE_TEST_CASE("Matrix constructors 2", "[matrix][constructor]", float, double, int) {
	Toucan::Matrix<TestType, 2, 2> m2_zero(
			TestType(0), TestType(0),
			TestType(0), TestType(0)
	);
	
	Toucan::Matrix<TestType, 2, 2> m2_ones(
			TestType(1), TestType(1),
			TestType(1), TestType(1)
	);
	
	Toucan::Matrix<TestType, 2, 2> m2_identity(
			TestType(1), TestType(0),
			TestType(0), TestType(1)
	);
	
	
	SECTION("Zero") {
		REQUIRE(Toucan::Matrix<TestType, 2, 2>::Zero() == m2_zero);
		REQUIRE_FALSE(Toucan::Matrix<TestType, 2, 2>::Zero() != m2_zero);
		
		REQUIRE_FALSE(Toucan::Matrix<TestType, 2, 2>::Zero() == m2_ones);
		REQUIRE(Toucan::Matrix<TestType, 2, 2>::Zero() != m2_ones);
		
		REQUIRE_FALSE(Toucan::Matrix<TestType, 2, 2>::Zero() == m2_identity);
		REQUIRE(Toucan::Matrix<TestType, 2, 2>::Zero() != m2_identity);
	}
	SECTION("Ones") {
		REQUIRE_FALSE(Toucan::Matrix<TestType, 2, 2>::Ones() == m2_zero);
		REQUIRE(Toucan::Matrix<TestType, 2, 2>::Ones() != m2_zero);
		
		REQUIRE(Toucan::Matrix<TestType, 2, 2>::Ones() == m2_ones);
		REQUIRE_FALSE(Toucan::Matrix<TestType, 2, 2>::Ones() != m2_ones);
		
		REQUIRE_FALSE(Toucan::Matrix<TestType, 2, 2>::Ones() == m2_identity);
		REQUIRE(Toucan::Matrix<TestType, 2, 2>::Ones() != m2_identity);
	}
	SECTION("Identity") {
		REQUIRE_FALSE(Toucan::Matrix<TestType, 2, 2>::Identity() == m2_zero);
		REQUIRE(Toucan::Matrix<TestType, 2, 2>::Identity() != m2_zero);
		
		REQUIRE_FALSE(Toucan::Matrix<TestType, 2, 2>::Identity() == m2_ones);
		REQUIRE(Toucan::Matrix<TestType, 2, 2>::Identity() != m2_ones);
		
		REQUIRE(Toucan::Matrix<TestType, 2, 2>::Identity() == m2_identity);
		REQUIRE_FALSE(Toucan::Matrix<TestType, 2, 2>::Identity() != m2_identity);
		
	}
}

TEMPLATE_TEST_CASE("Matrix shape", "[matrix][shape]", float, double, int) {
	Toucan::Vector2<TestType> v2;
	STATIC_REQUIRE(v2.number_of_rows() == 2);
	STATIC_REQUIRE(v2.number_of_columns() == 1);
	STATIC_REQUIRE(v2.number_of_elements() == 2);
	
	Toucan::Vector3<TestType> v3;
	STATIC_REQUIRE(v3.number_of_rows() == 3);
	STATIC_REQUIRE(v3.number_of_columns() == 1);
	STATIC_REQUIRE(v3.number_of_elements() == 3);
	
	Toucan::Vector4<TestType> v4;
	STATIC_REQUIRE(v4.number_of_rows() == 4);
	STATIC_REQUIRE(v4.number_of_columns() == 1);
	STATIC_REQUIRE(v4.number_of_elements() == 4);
	
	Toucan::RowVector2<TestType> v2_row;
	STATIC_REQUIRE(v2_row.number_of_rows() == 1);
	STATIC_REQUIRE(v2_row.number_of_columns() == 2);
	STATIC_REQUIRE(v2_row.number_of_elements() == 2);
	
	Toucan::RowVector3<TestType> v3_row;
	STATIC_REQUIRE(v3_row.number_of_rows() == 1);
	STATIC_REQUIRE(v3_row.number_of_columns() == 3);
	STATIC_REQUIRE(v3_row.number_of_elements() == 3);
	
	Toucan::RowVector4<TestType> v4_row;
	STATIC_REQUIRE(v4_row.number_of_rows() == 1);
	STATIC_REQUIRE(v4_row.number_of_columns() == 4);
	STATIC_REQUIRE(v4_row.number_of_elements() == 4);
	
	Toucan::Matrix<TestType, 4, 8> m1;
	STATIC_REQUIRE(m1.number_of_rows() == 4);
	STATIC_REQUIRE(m1.number_of_columns() == 8);
	STATIC_REQUIRE(m1.number_of_elements() == 32);
}

TEMPLATE_TEST_CASE("Matrix norms", "[matrix][norm]", float, double) {
	Toucan::Vector2<TestType> v2_ones = Toucan::Vector2<TestType>::Ones();
	REQUIRE(v2_ones.norm() == Approx(std::sqrt(2)));
	
	Toucan::Vector3<TestType> v3_ones = Toucan::Vector3<TestType>::Ones();
	REQUIRE(v3_ones.norm() == Approx(std::sqrt(3)));
	
	Toucan::Matrix<TestType, 2, 2> m2_ones = Toucan::Matrix<TestType, 2, 2>::Ones();
	REQUIRE(m2_ones.norm() == Approx(std::sqrt(4)));
	
	Toucan::Vector4<TestType> v4(TestType(1), TestType(2), TestType(3), TestType(4));
	const TestType v4_squared_sum = 1+4+9+16;
	const TestType v4_sum = std::sqrt(v4_squared_sum);
	REQUIRE(v4.norm() == Approx(std::sqrt(v4_squared_sum)));
	
	Toucan::Vector4<TestType> v4_normalized = v4.normalized();
	REQUIRE(v4_normalized(0) == Approx(1.0/v4_sum));
	REQUIRE(v4_normalized(1) == Approx(2.0/v4_sum));
	REQUIRE(v4_normalized(2) == Approx(3.0/v4_sum));
	REQUIRE(v4_normalized(3) == Approx(4.0/v4_sum));
	
	v4.normalize();
	REQUIRE(v4(0) == Approx(1.0/v4_sum));
	REQUIRE(v4(1) == Approx(2.0/v4_sum));
	REQUIRE(v4(2) == Approx(3.0/v4_sum));
	REQUIRE(v4(3) == Approx(4.0/v4_sum));
}

TEMPLATE_TEST_CASE("Matrix squared norms", "[matrix][norms]", float, double, int) {
	Toucan::Vector2<TestType> v2_ones = Toucan::Vector2<TestType>::Ones();
	REQUIRE(v2_ones.squared_norm() == Approx(2));
	
	Toucan::Vector3<TestType> v3_ones = Toucan::Vector3<TestType>::Ones();
	REQUIRE(v3_ones.squared_norm() == Approx(3));
	
	Toucan::Matrix<TestType, 2, 2> m2_ones = Toucan::Matrix<TestType, 2, 2>::Ones();
	REQUIRE(m2_ones.squared_norm() == Approx(4));
	
	Toucan::Vector4<TestType> v4(TestType(1), TestType(2), TestType(3), TestType(4));
	const TestType v4_squared_sum = 1+4+9+16;
	REQUIRE(v4.squared_norm() == Approx(v4_squared_sum));
}

TEMPLATE_TEST_CASE("Matrix math functions", "[matrix][math]", float, double, int) {
	
	SECTION("Trace") {
		Toucan::Matrix<TestType, 4, 4> m4(
				TestType(1), TestType(2), TestType(3), TestType(4),
				TestType(5), TestType(6), TestType(7), TestType(8),
				TestType(9), TestType(10), TestType(11), TestType(12),
				TestType(13), TestType(14), TestType(15), TestType(16)
		);
		
		REQUIRE(m4.trace() == Approx(1+6+11+16));
	}
	
	Toucan::Vector3<TestType> v1(TestType(1), TestType(2), TestType(3));
	Toucan::Vector3<TestType> v2(TestType(4), TestType(5), TestType(6));
	
	SECTION("Dot product") {
		TestType dot_product = v1.dot_product(v2);
		REQUIRE(dot_product == Approx(32));
	}
	
	SECTION("Cross product") {
		Toucan::Vector3<TestType> cross_product = v1.cross_product(v2);
		REQUIRE(cross_product.x() == Approx(-3));
		REQUIRE(cross_product.y() == Approx(6));
		REQUIRE(cross_product.z() == Approx(-3));
	}
}

TEMPLATE_TEST_CASE("Vector accessors", "[matrix][accessor]", float, double, int) {
	Toucan::Vector2<TestType> v2;
	v2.x() = TestType(1);
	v2.y() = TestType(2);
	
	REQUIRE(v2.x() == TestType(1));
	REQUIRE(v2.y() == TestType(2));
	
	Toucan::Vector3<TestType> v3;
	v3.x() = TestType(1);
	v3.y() = TestType(2);
	v3.z() = TestType(3);
	
	REQUIRE(v3.x() == TestType(1));
	REQUIRE(v3.y() == TestType(2));
	REQUIRE(v3.z() == TestType(3));
}

TEMPLATE_TEST_CASE("Vector const accessors", "[matrix][accessor]", float, double, int) {
	const Toucan::Vector2<TestType> v2(TestType(1), TestType(2));
	
	REQUIRE(v2.x() == TestType(1));
	REQUIRE(v2.y() == TestType(2));
	
	const Toucan::Vector3<TestType> v3(TestType(1), TestType(2), TestType(3));
	
	REQUIRE(v3.x() == TestType(1));
	REQUIRE(v3.y() == TestType(2));
	REQUIRE(v3.z() == TestType(3));
}

TEMPLATE_TEST_CASE("Row Vector accessors", "[matrix][accessor]", float, double, int) {
	Toucan::RowVector2<TestType> v2;
	v2.x() = TestType(1);
	v2.y() = TestType(2);
	
	REQUIRE(v2.x() == TestType(1));
	REQUIRE(v2.y() == TestType(2));
	
	Toucan::RowVector3<TestType> v3;
	v3.x() = TestType(1);
	v3.y() = TestType(2);
	v3.z() = TestType(3);
	
	REQUIRE(v3.x() == TestType(1));
	REQUIRE(v3.y() == TestType(2));
	REQUIRE(v3.z() == TestType(3));
}

TEMPLATE_TEST_CASE("Row Vector const accessors", "[matrix][accessor]", float, double, int) {
	const Toucan::RowVector2<TestType> v2(TestType(1), TestType(2));
	
	REQUIRE(v2.x() == TestType(1));
	REQUIRE(v2.y() == TestType(2));
	
	const Toucan::RowVector3<TestType> v3(TestType(1), TestType(2), TestType(3));
	
	REQUIRE(v3.x() == TestType(1));
	REQUIRE(v3.y() == TestType(2));
	REQUIRE(v3.z() == TestType(3));
}


TEMPLATE_TEST_CASE("Rotation matrices", "[matrix]", float, double) {
	constexpr TestType angle_1 = TestType(7)*M_PI/TestType(3);
	constexpr TestType angle_2 = TestType(5)*M_PI/TestType(3);
	constexpr TestType angle_3 = TestType(4)*M_PI/TestType(3);
	
	const Eigen::Matrix<TestType, 3, 3> r1_eigen =
			(Eigen::AngleAxis<TestType>(angle_1, EigenVector3<TestType>::UnitX()) *
			 Eigen::AngleAxis<TestType>(angle_2, EigenVector3<TestType>::UnitY()) *
			 Eigen::AngleAxis<TestType>(angle_3, EigenVector3<TestType>::UnitZ())).toRotationMatrix();
	
	const EigenVector3<TestType> v1_eigen(TestType(1), TestType(2), TestType(3));
	const EigenVector3<TestType> v1_eigen_transformed = r1_eigen * v1_eigen;
	
	const Toucan::Matrix3<TestType> r1_toucan =
			Toucan::create_3d_rotation_matrix_x(angle_1) *
			Toucan::create_3d_rotation_matrix_y(angle_2) *
			Toucan::create_3d_rotation_matrix_z(angle_3);
	
	const Toucan::Vector3<TestType> v1_toucan(TestType(1), TestType(2), TestType(3));
	const Toucan::Vector3<TestType> v1_toucan_transformed = r1_toucan * v1_toucan;
	
	REQUIRE(v1_toucan_transformed.x() == Approx(v1_eigen_transformed.x()));
	REQUIRE(v1_toucan_transformed.y() == Approx(v1_eigen_transformed.y()));
	REQUIRE(v1_toucan_transformed.z() == Approx(v1_eigen_transformed.z()));
}

TEMPLATE_TEST_CASE("Diagonal Matrix constructor", "[matrix]", float, double, int) {
	const Toucan::DiagonalMatrix3<TestType> d1;
	REQUIRE(d1(0) == Approx(0));
	REQUIRE(d1(1) == Approx(0));
	REQUIRE(d1(2) == Approx(0));
	
	const Toucan::DiagonalMatrix3<TestType> d2(TestType(1), TestType(2), TestType(3));
	REQUIRE(d2(0) == Approx(1));
	REQUIRE(d2(1) == Approx(2));
	REQUIRE(d2(2) == Approx(3));
}

TEMPLATE_TEST_CASE("Diagonal Matrix shape functions", "[matrix]", float, double, int) {
	const Toucan::DiagonalMatrix2<TestType> d2;
	REQUIRE(d2.number_of_rows() == 2);
	REQUIRE(d2.number_of_columns() == 2);
	REQUIRE(d2.number_of_elements() == 2*2);
	
	const Toucan::DiagonalMatrix3<TestType> d3;
	REQUIRE(d3.number_of_rows() == 3);
	REQUIRE(d3.number_of_columns() == 3);
	REQUIRE(d3.number_of_elements() == 3*3);
	
	const Toucan::DiagonalMatrix4<TestType> d4;
	REQUIRE(d4.number_of_rows() == 4);
	REQUIRE(d4.number_of_columns() == 4);
	REQUIRE(d4.number_of_elements() == 4*4);
}

TEMPLATE_TEST_CASE("Diagonal Matrix get diagonal functions", "[matrix]", float, double, int) {
	//const Toucan::DiagonalMatrix2<TestType> d2(TestType(1), TestType(2));
}

TEMPLATE_TEST_CASE("Quaternion default constructor", "[quaternion][constructor]", float, double) {
	const Toucan::Quaternion<TestType> q;
	REQUIRE(q.w == Approx(1));
	REQUIRE(q.x == Approx(0));
	REQUIRE(q.y == Approx(0));
	REQUIRE(q.z == Approx(0));
}

TEMPLATE_TEST_CASE("Quaternion 4 scalar constructor", "[quaternion][constructor]", float, double) {
	const Toucan::Quaternion<TestType> q(TestType(1), TestType(2), TestType(3), TestType(4));
	REQUIRE(q.w == Approx(1));
	REQUIRE(q.x == Approx(2));
	REQUIRE(q.y == Approx(3));
	REQUIRE(q.z == Approx(4));
}

TEMPLATE_TEST_CASE("Quaternion parameter vector constructor", "[quaternion][constructor]", float, double) {
	const Toucan::Vector4<TestType> v4(TestType(1), TestType(2), TestType(3), TestType(4));
	const Toucan::Quaternion<TestType> q(v4);
	REQUIRE(q.w == Approx(1));
	REQUIRE(q.x == Approx(2));
	REQUIRE(q.y == Approx(3));
	REQUIRE(q.z == Approx(4));
}

TEMPLATE_TEST_CASE("Quaternion axis angle constructor", "[quaternion][constructor]", float, double) {
	const Toucan::Vector3<TestType> axis1(TestType(0.506979), TestType(0.253490), TestType(0.823842));
	const TestType angle1 = 1.845;
	
	const Toucan::Quaternion<TestType> q1(axis1, angle1);
	REQUIRE(q1.w == Approx(0.6038293));
	REQUIRE(q1.x == Approx(0.4041198));
	REQUIRE(q1.y == Approx(0.2020603));
	REQUIRE(q1.z == Approx(0.6566956));
	
	const Toucan::Vector3<TestType> axis2(TestType(0.857921), TestType(-0.509391), TestType(-0.067025));
	const TestType angle2 = -2.271;
	
	const Toucan::Quaternion<TestType> q2(axis2, angle2);
	REQUIRE(q2.w == Approx(0.4216791));
	REQUIRE(q2.x == Approx(-0.7779157));
	REQUIRE(q2.y == Approx(0.4618878));
	REQUIRE(q2.z == Approx(0.0607746));
}

TEMPLATE_TEST_CASE("Quaternion rotation matrix constructor", "[quaternion][constructor]", float, double) {
	const Toucan::Matrix3<TestType> rotation_matrix1(
			TestType(0.0558452), TestType(-0.6297508), TestType(0.7747872),
			TestType(0.9563771), TestType(-0.1891237), TestType(-0.2226545),
			TestType(0.2867475), TestType(0.7534229), TestType(0.5917177)
	);
	
	const Toucan::Quaternion<TestType> q1(rotation_matrix1);
	REQUIRE(q1.w == Approx(0.6038293));
	REQUIRE(q1.x == Approx(0.4041198));
	REQUIRE(q1.y == Approx(0.2020603));
	REQUIRE(q1.z == Approx(0.6566956));
	
	const Toucan::Matrix3<TestType> rotation_matrix2(
			TestType(0.5659322), TestType(-0.7698743), TestType(0.2949819),
			TestType(-0.6673648), TestType(-0.2176927), TestType(0.7122037),
			TestType(-0.4840919), TestType(-0.5999195), TestType(-0.6369864)
	);
	
	const Toucan::Quaternion<TestType> q2(rotation_matrix2);
	REQUIRE(q2.w == Approx(0.4216791));
	REQUIRE(q2.x == Approx(-0.7779157));
	REQUIRE(q2.y == Approx(0.4618878));
	REQUIRE(q2.z == Approx(0.0607746));
}


TEMPLATE_TEST_CASE("Quaternion operators", "[quaternion][operator]", float, double) {
	const Toucan::Quaternion<TestType> q1(0.6038293, 0.4041198, 0.2020603, 0.6566956);
	const Toucan::Quaternion<TestType> q2(0.4216791, -0.7779157, 0.4618878, 0.0607746);
	
	constexpr TestType eps = 1e-4;
	
	SECTION("Quaternion-Quaternion composition") {
		const Toucan::Quaternion<TestType> q_1_2 = q1*q2;
		REQUIRE(q_1_2.w == Approx(0.435754).epsilon(eps));
		REQUIRE(q_1_2.x == Approx(-0.590359).epsilon(eps));
		REQUIRE(q_1_2.y == Approx(-0.17130).epsilon(eps));
		REQUIRE(q_1_2.z == Approx(0.657456).epsilon(eps));
		
		const Toucan::Quaternion<TestType> q_2_1 = q2*q1;
		REQUIRE(q_2_1.w == Approx(0.435754).epsilon(eps));
		REQUIRE(q_2_1.x == Approx(-0.00827987).epsilon(eps));
		REQUIRE(q_2_1.y == Approx(0.89952).epsilon(eps));
		REQUIRE(q_2_1.z == Approx(-0.0302316).epsilon(eps));
	}
	
	SECTION("Quaternion-Point composition") {
		const Eigen::Quaternion<TestType> q1_eig(q2.w, q2.x, q2.y, q2.z);
		
		const Toucan::Vector3<TestType> p(TestType(8.91), TestType(-42.8), TestType(0.25));
		
		const Toucan::Vector3<TestType> p1 = q1 * p;
		REQUIRE(p1.x() == Approx(27.6446).epsilon(eps));
		REQUIRE(p1.y() == Approx(16.5602).epsilon(eps));
		REQUIRE(p1.z() == Approx(-29.5437).epsilon(eps));
		
		const Toucan::Vector3<TestType> p2 = q2 * p;
		REQUIRE(p2.x() == Approx(38.0668).epsilon(eps));
		REQUIRE(p2.y() == Approx(3.54908).epsilon(eps));
		REQUIRE(p2.z() == Approx(21.204).epsilon(eps));
	}
}

TEMPLATE_TEST_CASE("Scaled Transform 2D inverse", "", float, double) {
	Toucan::ScaledTransform2D<TestType> t(
			TestType(0.123),
			Toucan::Vector2<TestType>(TestType(1.0), TestType(2.0)),
			Toucan::Vector2<TestType>(TestType(0.5), TestType(2.0))
	);
	
	SECTION("3x3 Transformation matrix") {
		const Toucan::Matrix3<TestType> t_matrix = t.transformation_matrix();
		const Toucan::Matrix3<TestType> t_matrix_inverse = t.transformation_matrix_inverse();
		const Toucan::Matrix3<TestType> identity_1 = t_matrix * t_matrix_inverse;
		const Toucan::Matrix3<TestType> identity_2 = t_matrix_inverse * t_matrix;
		
		REQUIRE(identity_1(0, 0) == Approx(1));
		REQUIRE(identity_1(0, 1) == Approx(0));
		REQUIRE(identity_1(0, 2) == Approx(0));
		REQUIRE(identity_1(1, 0) == Approx(0));
		REQUIRE(identity_1(1, 1) == Approx(1));
		REQUIRE(identity_1(1, 2) == Approx(0));
		REQUIRE(identity_1(2, 0) == Approx(0));
		REQUIRE(identity_1(2, 1) == Approx(0));
		REQUIRE(identity_1(2, 2) == Approx(1));
		
		REQUIRE(identity_2(0, 0) == Approx(1));
		REQUIRE(identity_2(0, 1) == Approx(0));
		REQUIRE(identity_2(0, 2) == Approx(0));
		REQUIRE(identity_2(1, 0) == Approx(0));
		REQUIRE(identity_2(1, 1) == Approx(1));
		REQUIRE(identity_2(1, 2) == Approx(0));
		REQUIRE(identity_2(2, 0) == Approx(0));
		REQUIRE(identity_2(2, 1) == Approx(0));
		REQUIRE(identity_2(2, 2) == Approx(1));
	}
	
	SECTION("4x4 Transformation matrix") {
		const Toucan::Matrix4<TestType> t_matrix = t.transformation_matrix_3d();
		const Toucan::Matrix4<TestType> t_matrix_inverse = t.transformation_matrix_inverse_3d();
		const Toucan::Matrix4<TestType> identity_1 = t_matrix * t_matrix_inverse;
		const Toucan::Matrix4<TestType> identity_2 = t_matrix_inverse * t_matrix;
		
		REQUIRE(identity_1(0, 0) == Approx(1));
		REQUIRE(identity_1(0, 1) == Approx(0));
		REQUIRE(identity_1(0, 2) == Approx(0));
		REQUIRE(identity_1(0, 3) == Approx(0));
		REQUIRE(identity_1(1, 0) == Approx(0));
		REQUIRE(identity_1(1, 1) == Approx(1));
		REQUIRE(identity_1(1, 2) == Approx(0));
		REQUIRE(identity_1(1, 3) == Approx(0));
		REQUIRE(identity_1(2, 0) == Approx(0));
		REQUIRE(identity_1(2, 1) == Approx(0));
		REQUIRE(identity_1(2, 2) == Approx(1));
		REQUIRE(identity_1(2, 3) == Approx(0));
		REQUIRE(identity_1(3, 0) == Approx(0));
		REQUIRE(identity_1(3, 1) == Approx(0));
		REQUIRE(identity_1(3, 2) == Approx(0));
		REQUIRE(identity_1(3, 3) == Approx(1));
		
		REQUIRE(identity_2(0, 0) == Approx(1));
		REQUIRE(identity_2(0, 1) == Approx(0));
		REQUIRE(identity_2(0, 2) == Approx(0));
		REQUIRE(identity_2(0, 3) == Approx(0));
		REQUIRE(identity_2(1, 0) == Approx(0));
		REQUIRE(identity_2(1, 1) == Approx(1));
		REQUIRE(identity_2(1, 2) == Approx(0));
		REQUIRE(identity_2(1, 3) == Approx(0));
		REQUIRE(identity_2(2, 0) == Approx(0));
		REQUIRE(identity_2(2, 1) == Approx(0));
		REQUIRE(identity_2(2, 2) == Approx(1));
		REQUIRE(identity_2(2, 3) == Approx(0));
		REQUIRE(identity_2(3, 0) == Approx(0));
		REQUIRE(identity_2(3, 1) == Approx(0));
		REQUIRE(identity_2(3, 2) == Approx(0));
		REQUIRE(identity_2(3, 3) == Approx(1));
	}
	
}
TEMPLATE_TEST_CASE("Scaled Transform 3D inverse", "", float, double) {
	Toucan::ScaledTransform3D<TestType> t(
			Toucan::Quaternion<TestType>(Toucan::Vector3<TestType>(TestType(1), TestType(2), TestType(3)).normalized(), TestType(2*M_PI/3)),
			Toucan::Vector3<TestType>(TestType(1.0), TestType(2.0), TestType(3.0)),
			Toucan::Vector3<TestType>(TestType(0.5), TestType(2.0), TestType(3.25))
	);
	
	const Toucan::Matrix4<TestType> t_matrix = t.transformation_matrix();
	const Toucan::Matrix4<TestType> t_matrix_inverse = t.transformation_matrix_inverse();
	const Toucan::Matrix4<TestType> identity_1 = t_matrix * t_matrix_inverse;
	const Toucan::Matrix4<TestType> identity_2 = t_matrix_inverse * t_matrix;
	
	REQUIRE(identity_1(0, 0) == Approx(1));
	REQUIRE(identity_1(0, 1) == Approx(0));
	REQUIRE(identity_1(0, 2) == Approx(0));
	REQUIRE(identity_1(0, 3) == Approx(0));
	REQUIRE(identity_1(1, 0) == Approx(0));
	REQUIRE(identity_1(1, 1) == Approx(1));
	REQUIRE(identity_1(1, 2) == Approx(0));
	REQUIRE(identity_1(1, 3) == Approx(0));
	REQUIRE(identity_1(2, 0) == Approx(0));
	REQUIRE(identity_1(2, 1) == Approx(0));
	REQUIRE(identity_1(2, 2) == Approx(1));
	REQUIRE(identity_1(2, 3) == Approx(0));
	REQUIRE(identity_1(3, 0) == Approx(0));
	REQUIRE(identity_1(3, 1) == Approx(0));
	REQUIRE(identity_1(3, 2) == Approx(0));
	REQUIRE(identity_1(3, 3) == Approx(1));
	
	REQUIRE(identity_2(0, 0) == Approx(1));
	REQUIRE(identity_2(0, 1) == Approx(0));
	REQUIRE(identity_2(0, 2) == Approx(0));
	REQUIRE(identity_2(0, 3) == Approx(0));
	REQUIRE(identity_2(1, 0) == Approx(0));
	REQUIRE(identity_2(1, 1) == Approx(1));
	REQUIRE(identity_2(1, 2) == Approx(0));
	REQUIRE(identity_2(1, 3) == Approx(0));
	REQUIRE(identity_2(2, 0) == Approx(0));
	REQUIRE(identity_2(2, 1) == Approx(0));
	REQUIRE(identity_2(2, 2) == Approx(1));
	REQUIRE(identity_2(2, 3) == Approx(0));
	REQUIRE(identity_2(3, 0) == Approx(0));
	REQUIRE(identity_2(3, 1) == Approx(0));
	REQUIRE(identity_2(3, 2) == Approx(0));
	REQUIRE(identity_2(3, 3) == Approx(1));
	
}
