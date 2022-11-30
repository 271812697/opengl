#pragma once
#include"Vector3.h"
#include"Matrix3.h"
namespace Opengl
{
class  Quaternion
{
public:
    Quaternion() noexcept
       :w_(1.0f),
        x_(0.0f),
        y_(0.0f),
        z_(0.0f)
    {
    }
    Quaternion(const Quaternion& quat) noexcept
       :w_(quat.w_),
        x_(quat.x_),
        y_(quat.y_),
        z_(quat.z_)
    {
    }
    Quaternion(float w, float x, float y, float z) noexcept
       :w_(w),
        x_(x),
        y_(y),
        z_(z)
    {
    }

    /// Construct from a float array.
    explicit Quaternion(const float* data) noexcept
       :w_(data[0]),
        x_(data[1]),
        y_(data[2]),
        z_(data[3])
    {
    }

    /// Construct from an angle (in degrees) and axis.
    Quaternion(float angle, const Vector3& axis) noexcept
    {
        FromAngleAxis(angle, axis);
    }

    /// Construct from an angle (in degrees, for Urho2D).
    explicit Quaternion(float angle) noexcept
    {
        FromAngleAxis(angle, Vector3::FORWARD);
    }

    /// Construct from Euler angles (in degrees). Equivalent to Y*X*Z.
    Quaternion(float x, float y, float z) noexcept
    {
        FromEulerAngles(x, y, z);
    }

    /// Construct from Euler angles (in degrees).
    explicit Quaternion(const Vector3& angles) noexcept
    {
        FromEulerAngles(angles.x_, angles.y_, angles.z_);
    }

    /// Construct from the rotation difference between two direction vectors.
    Quaternion(const Vector3& start, const Vector3& end) noexcept
    {
        FromRotationTo(start, end);
    }

    /// Construct from orthonormal axes.
    Quaternion(const Vector3& xAxis, const Vector3& yAxis, const Vector3& zAxis) noexcept
    {
        FromAxes(xAxis, yAxis, zAxis);
    }

    /// Construct from a rotation matrix.
    explicit Quaternion(const Matrix3& matrix) noexcept
    {
        FromRotationMatrix(matrix);
    }
    Quaternion& operator =(const Quaternion& rhs) noexcept
    {
        w_ = rhs.w_;
        x_ = rhs.x_;
        y_ = rhs.y_;
        z_ = rhs.z_;
        return *this;
    }

    /// Add-assign a quaternion.
    Quaternion& operator +=(const Quaternion& rhs)
    {
        w_ += rhs.w_;
        x_ += rhs.x_;
        y_ += rhs.y_;
        z_ += rhs.z_;
        return *this;
    }
    Quaternion& operator *=(float rhs)
    {

        w_ *= rhs;
        x_ *= rhs;
        y_ *= rhs;
        z_ *= rhs;
        return *this;
    }

    /// Test for equality with another quaternion without epsilon.
    bool operator ==(const Quaternion& rhs) const
    {
        return w_ == rhs.w_ && x_ == rhs.x_ && y_ == rhs.y_ && z_ == rhs.z_;
    }

    /// Test for inequality with another quaternion without epsilon.
    bool operator !=(const Quaternion& rhs) const { return !(*this == rhs); }

    /// Multiply with a scalar.
    Quaternion operator *(float rhs) const
    {
        return Quaternion(w_ * rhs, x_ * rhs, y_ * rhs, z_ * rhs);
    }

    /// Return negation.
    Quaternion operator -() const
    {
        return Quaternion(-w_, -x_, -y_, -z_);
    }

    /// Add a quaternion.
    Quaternion operator +(const Quaternion& rhs) const
    {
        return Quaternion(w_ + rhs.w_, x_ + rhs.x_, y_ + rhs.y_, z_ + rhs.z_);
    }

    /// Subtract a quaternion.
    Quaternion operator -(const Quaternion& rhs) const
    {
        return Quaternion(w_ - rhs.w_, x_ - rhs.x_, y_ - rhs.y_, z_ - rhs.z_);
    }

    /// Multiply a quaternion.
    Quaternion operator *(const Quaternion& rhs) const
    {
        return Quaternion(
            w_ * rhs.w_ - x_ * rhs.x_ - y_ * rhs.y_ - z_ * rhs.z_,
            w_ * rhs.x_ + x_ * rhs.w_ + y_ * rhs.z_ - z_ * rhs.y_,
            w_ * rhs.y_ + y_ * rhs.w_ + z_ * rhs.x_ - x_ * rhs.z_,
            w_ * rhs.z_ + z_ * rhs.w_ + x_ * rhs.y_ - y_ * rhs.x_
        );
    }

    /// Multiply a Vector3.
    Vector3 operator *(const Vector3& rhs) const
    {

        Vector3 qVec(x_, y_, z_);
        Vector3 cross1(qVec.CrossProduct(rhs));
        Vector3 cross2(qVec.CrossProduct(cross1));

        return rhs + 2.0f * (cross1 * w_ + cross2);
    }

    /// Define from an angle (in degrees) and axis.
    void FromAngleAxis(float angle, const Vector3& axis);
    /// Define from Euler angles (in degrees). Equivalent to Y*X*Z.
    void FromEulerAngles(float x, float y, float z);
    /// Define from the rotation difference between two direction vectors.
    void FromRotationTo(const Vector3& start, const Vector3& end);
    /// Define from orthonormal axes.
    void FromAxes(const Vector3& xAxis, const Vector3& yAxis, const Vector3& zAxis);
    /// Define from a rotation matrix.
    void FromRotationMatrix(const Matrix3& matrix);
    /// Define from a direction to look in and an up direction. Return true if successful, or false if would result in a NaN, in which case the current value remains.
    bool FromLookRotation(const Vector3& direction, const Vector3& up = Vector3::UP);

    /// Normalize to unit length.
    void Normalize()
    {
        float lenSquared = LengthSquared();
        if (!Opengl::Equals(lenSquared, 1.0f) && lenSquared > 0.0f)
        {
            float invLen = 1.0f / sqrtf(lenSquared);
            w_ *= invLen;
            x_ *= invLen;
            y_ *= invLen;
            z_ *= invLen;
        }
    }

    /// Return normalized to unit length.
    Quaternion Normalized() const
    {

        float lenSquared = LengthSquared();
        if (!Opengl::Equals(lenSquared, 1.0f) && lenSquared > 0.0f)
        {
            float invLen = 1.0f / sqrtf(lenSquared);
            return *this * invLen;
        }
        else
            return *this;
    }

    /// Return inverse.
    Quaternion Inverse() const
    {
        float lenSquared = LengthSquared();
        if (lenSquared == 1.0f)
            return Conjugate();
        else if (lenSquared >= M_EPSILON)
            return Conjugate() * (1.0f / lenSquared);
        else
            return IDENTITY;
    }

    /// Return squared length.
    float LengthSquared() const
    {
        return w_ * w_ + x_ * x_ + y_ * y_ + z_ * z_;
    }

    /// Calculate dot product.
    float DotProduct(const Quaternion& rhs) const
    {
        return w_ * rhs.w_ + x_ * rhs.x_ + y_ * rhs.y_ + z_ * rhs.z_;
    }

    /// Test for equality with another quaternion with epsilon.
    bool Equals(const Quaternion& rhs) const
    {
        return Opengl::Equals(w_, rhs.w_) && Opengl::Equals(x_, rhs.x_) && Opengl::Equals(y_, rhs.y_) && Opengl::Equals(z_, rhs.z_);
    }

    /// Return whether any element is NaN.
    bool IsNaN() const { return Opengl::IsNaN(w_) || Opengl::IsNaN(x_) || Opengl::IsNaN(y_) || Opengl::IsNaN(z_); }

    /// Return whether any element is Inf.
    bool IsInf() const { return Opengl::IsInf(w_) || Opengl::IsInf(x_) || Opengl::IsInf(y_) || Opengl::IsInf(z_); }

    /// Return conjugate.
    Quaternion Conjugate() const
    {
        return Quaternion(w_, -x_, -y_, -z_);
    }

    /// Return Euler angles in degrees.
    /// @property
    Vector3 EulerAngles() const;
    /// Return yaw angle in degrees.
    /// @property{get_yaw}
    float YawAngle() const;
    /// Return pitch angle in degrees.
    /// @property{get_pitch}
    float PitchAngle() const;
    /// Return roll angle in degrees.
    /// @property{get_roll}
    float RollAngle() const;
    /// Return rotation axis.
    /// @property
    Vector3 Axis() const;
    /// Return rotation angle.
    /// @property
    float Angle() const;
    /// Return the rotation matrix that corresponds to this quaternion.
    /// @property
    Matrix3 RotationMatrix() const;
    /// Spherical interpolation with another quaternion.
    Quaternion Slerp(const Quaternion& rhs, float t) const;
    /// Normalized linear interpolation with another quaternion.
    Quaternion Nlerp(const Quaternion& rhs, float t, bool shortestPath = false) const;

    /// Return float data.
    const float* Data() const { return &w_; }

    /// W coordinate.
    float w_;
    /// X coordinate.
    float x_;
    /// Y coordinate.
    float y_;
    /// Z coordinate.
    float z_;

    static const Quaternion IDENTITY;
};

}
