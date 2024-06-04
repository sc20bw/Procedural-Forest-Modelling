#ifndef MAT44_HPP_E7187A26_469E_48AD_A3D2_63150F05A4CA
#define MAT44_HPP_E7187A26_469E_48AD_A3D2_63150F05A4CA

#include <cmath>
#include <cassert>
#include <cstdlib>

#include "vec3.hpp"
#include "vec4.hpp"

/** Mat44f: 4x4 matrix with floats
 *
 * See vec2f.hpp for discussion. Similar to the implementation, the Mat44f is
 * intentionally kept simple and somewhat bare bones.
 *
 * The matrix is stored in row-major order (careful when passing it to OpenGL).
 *
 * The overloaded operator () allows access to individual elements. Example:
 *    Mat44f m = ...;
 *    float m12 = m(1,2);
 *    m(0,3) = 3.f;
 *
 * The matrix is arranged as:
 *
 *   ⎛ 0,0  0,1  0,2  0,3 ⎞
 *   ⎜ 1,0  1,1  1,2  1,3 ⎟
 *   ⎜ 2,0  2,1  2,2  2,3 ⎟
 *   ⎝ 3,0  3,1  3,2  3,3 ⎠
 */
struct Mat44f
{
	float v[16];

	constexpr
	float& operator() (std::size_t aI, std::size_t aJ) noexcept
	{
		assert( aI < 4 && aJ < 4 );
		return v[aI*4 + aJ];
	}
	constexpr
	float const& operator() (std::size_t aI, std::size_t aJ) const noexcept
	{
		assert( aI < 4 && aJ < 4 );
		return v[aI*4 + aJ];
	}
};

// Identity matrix
constexpr Mat44f kIdentity44f = { {
	1.f, 0.f, 0.f, 0.f,
	0.f, 1.f, 0.f, 0.f,
	0.f, 0.f, 1.f, 0.f,
	0.f, 0.f, 0.f, 1.f
} };

// Common operators for Mat44f.
// Note that you will need to implement these yourself.

constexpr
Mat44f operator*( Mat44f const& aLeft, Mat44f const& aRight ) noexcept
{
	Mat44f aAnswer = { 
		aLeft.v[0]*aRight.v[0] + aLeft.v[1]*aRight.v[4] + aLeft.v[2]*aRight.v[8] + aLeft.v[3]*aRight.v[12], aLeft.v[0] * aRight.v[1] + aLeft.v[1] * aRight.v[5] + aLeft.v[2] * aRight.v[9] + aLeft.v[3] * aRight.v[13], aLeft.v[0] * aRight.v[2] + aLeft.v[1] * aRight.v[6] + aLeft.v[2] * aRight.v[10] + aLeft.v[3] * aRight.v[14], aLeft.v[0] * aRight.v[3] + aLeft.v[1] * aRight.v[7] + aLeft.v[2] * aRight.v[11] + aLeft.v[3] * aRight.v[15],
		aLeft.v[4] * aRight.v[0] + aLeft.v[5] * aRight.v[4] + aLeft.v[6] * aRight.v[8] + aLeft.v[7] * aRight.v[12], aLeft.v[4] * aRight.v[1] + aLeft.v[5] * aRight.v[5] + aLeft.v[6] * aRight.v[9] + aLeft.v[7] * aRight.v[13], aLeft.v[4] * aRight.v[2] + aLeft.v[5] * aRight.v[6] + aLeft.v[6] * aRight.v[10] + aLeft.v[7] * aRight.v[14], aLeft.v[4] * aRight.v[3] + aLeft.v[5] * aRight.v[7] + aLeft.v[6] * aRight.v[11] + aLeft.v[7] * aRight.v[15],
		aLeft.v[8] * aRight.v[0] + aLeft.v[9] * aRight.v[4] + aLeft.v[10] * aRight.v[8] + aLeft.v[11] * aRight.v[12], aLeft.v[8] * aRight.v[1] + aLeft.v[9] * aRight.v[5] + aLeft.v[10] * aRight.v[9] + aLeft.v[11] * aRight.v[13], aLeft.v[8] * aRight.v[2] + aLeft.v[9] * aRight.v[6] + aLeft.v[10] * aRight.v[10] + aLeft.v[11] * aRight.v[14], aLeft.v[8] * aRight.v[3] + aLeft.v[9] * aRight.v[7] + aLeft.v[10] * aRight.v[11] + aLeft.v[11] * aRight.v[15],
		aLeft.v[12] * aRight.v[0] + aLeft.v[13] * aRight.v[4] + aLeft.v[14] * aRight.v[8] + aLeft.v[15] * aRight.v[12], aLeft.v[12] * aRight.v[1] + aLeft.v[13] * aRight.v[5] + aLeft.v[14] * aRight.v[9] + aLeft.v[15] * aRight.v[13], aLeft.v[12] * aRight.v[2] + aLeft.v[13] * aRight.v[6] + aLeft.v[14] * aRight.v[10] + aLeft.v[15] * aRight.v[14], aLeft.v[12] * aRight.v[3] + aLeft.v[13] * aRight.v[7] + aLeft.v[14] * aRight.v[11] + aLeft.v[15] * aRight.v[15],
	};
	return aAnswer;
}

constexpr
Vec4f operator*( Mat44f const& aLeft, Vec4f const& aRight ) noexcept
{
	return {aLeft.v[0]*aRight.x + aLeft.v[1]*aRight.y + aLeft.v[2]*aRight.z + aLeft.v[3]*aRight.w, aLeft.v[4] * aRight.x + aLeft.v[5] * aRight.y + aLeft.v[6] * aRight.z + aLeft.v[7] * aRight.w, aLeft.v[8] * aRight.x + aLeft.v[9] * aRight.y + aLeft.v[10] * aRight.z + aLeft.v[11] * aRight.w, aLeft.v[12] * aRight.x + aLeft.v[13] * aRight.y + aLeft.v[14] * aRight.z + aLeft.v[15] * aRight.w, };
}

// Functions:

inline
Mat44f make_rotation_x( float aAngle ) noexcept
{
	return {
		1.f, 0.f, 0.f, 0.f,
		0.f, cos(aAngle), -sin(aAngle), 0.f,
		0.f, sin(aAngle), cos(aAngle), 0.f,
		0.f, 0.f, 0.f, 1.f 
	};
}


inline
Mat44f make_rotation_y( float aAngle ) noexcept
{
	return {
		cos(aAngle), 0.f, sin(aAngle), 0.f,
		0.f, 1.f, 0.f, 0.f,
		-sin(aAngle), 0.f, cos(aAngle), 0.f,
		0.f, 0.f, 0.f, 1.f
	};
}

inline
Mat44f make_rotation_z( float aAngle ) noexcept
{
	return {
		cos(aAngle), -sin(aAngle), 0.f, 0.f,
		sin(aAngle), cos(aAngle), 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f
	};
}

inline
Mat44f make_translation( Vec3f aTranslation ) noexcept
{
	return {
		1.f, 0.f, 0.f, aTranslation.x,
		0.f, 1.f, 0.f, aTranslation.y,
		0.f, 0.f, 1.f, aTranslation.z,
		0.f, 0.f, 0.f, 1.f
	};
}

inline
Mat44f make_scaling(float aSX, float aSY, float aSZ) noexcept
{
	//TODO: your implementation goes here
	return  { {
	aSX, 0.f, 0.f, 0.f,
	0.f, aSY, 0.f, 0.f,
	0.f, 0.f, aSZ, 0.f,
	0.f, 0.f, 0.f, 1.f
} };
}

inline
Mat44f make_perspective_projection( float aFovInRadians, float aAspect, float aNear, float aFar ) noexcept
{
	return {
		1.f/aAspect, 0.f, 0.f, 0.f,
		0.f, 1.f/tan(aFovInRadians / 2.f), 0.f, 0.f,
		0.f, 0.f, -1*((aFar+aNear)/(aFar-aNear)), -2*((aFar*aNear)/(aFar-aNear)),
		0.f, 0.f, -1.f, 1.f
	};
}



#endif // MAT44_HPP_E7187A26_469E_48AD_A3D2_63150F05A4CA
