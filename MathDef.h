#pragma once
#include <cstdlib>
#include <cmath>
#include <limits>
#include <type_traits>
namespace Opengl
{
	static constexpr float M_GLPI = 3.14159265358979323846264338327950288f;
	static constexpr float M_HALF_PI = M_GLPI * 0.5f;
	static constexpr int M_MIN_INT = 0x80000000;
	static constexpr int M_MAX_INT = 0x7fffffff;
	static constexpr unsigned int M_MIN_UNSIGNED = 0x00000000;
	static constexpr unsigned int M_MAX_UNSIGNED = 0xffffffff;

	static constexpr float M_EPSILON = 0.000001f;
	static constexpr float M_LARGE_EPSILON = 0.00005f;
	static constexpr float M_MIN_NEARCLIP = 0.01f;
	static constexpr float M_MAX_FOV = 160.0f;
	static constexpr float M_LARGE_VALUE = 100000000.0f;
	static constexpr float M_INFINITY = std::numeric_limits<float>::infinity();
	static constexpr float M_DEGTORAD = M_GLPI / 180.0f;
	static constexpr float M_DEGTORAD_2 = M_GLPI / 360.0f;    // M_DEGTORAD / 2.f
	static constexpr float M_RADTODEG = 1.0f / M_DEGTORAD;
	
	 template <class T> inline T Pow(T x, T y) { return pow(x, y); }
	 template <class T>
	 T Clamp(T value, T min, T max);
	 template <class T>
	 inline T Abs(T value) { return value >= 0.0 ? value : -value; }

	 template <class T> inline T Acos(T x) { return M_RADTODEG * acos(Clamp(x, T(-1.0), T(1.0))); }
	 template <class T, class U>
	 inline T Min(T lhs, U rhs) { return lhs < rhs ? lhs : rhs; }
	 template <class T, class U>
	 inline T Max(T lhs, U rhs) { return lhs > rhs ? lhs : rhs; }
	 template <class T> inline T Floor(T x) { return floor(x); }
	 template <class T> inline T Round(T x) { return round(x); }
	 template <class T> inline T Ceil(T x) { return ceil(x); }
	 template <class T> inline int FloorToInt(T x) { return static_cast<int>(floor(x)); }
	 template <class T> inline int RoundToInt(T x) { return static_cast<int>(round(x)); }
	 template <class T> inline int CeilToInt(T x) { return static_cast<int>(ceil(x)); }
	 template <class T>
	 inline T Clamp(T value, T min, T max)
	 {
		 if (value < min)
			 return min;
		 else if (value > max)
			 return max;
		 else
			 return value;
	 }
	 template <class T> inline T Sin(T angle) { return sin(angle * M_DEGTORAD); }
	 template <class T> inline T Fract(T value) { return value - floor(value); }
	 /// @specialization{float}
	 template <class T>
	 inline bool Equals(T lhs, T rhs) { return lhs + std::numeric_limits<T>::epsilon() >= rhs && lhs - std::numeric_limits<T>::epsilon() <= rhs; }
	 template <class T> inline bool IsNaN(T value) { return std::isnan(value); }

	 /// Check whether a floating point value is positive or negative infinity.
	 template <class T> inline bool IsInf(T value) { return std::isinf(value); }
	 /// Return a representation of the specified floating-point value as a single format bit layout.
	 inline unsigned FloatToRawIntBits(float value)
	 {
		 unsigned u = *((unsigned*)&value);
		 return u;
	 }


}