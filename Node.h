#pragma once
#include "Matrix3x4.h"
#include <memory>
#include<vector>
namespace Opengl
{
class Node;

enum CreateMode
{
    REPLICATED = 0,
    LOCAL = 1
};

enum TransformSpace
{
    TS_LOCAL = 0,
    TS_PARENT,
    TS_WORLD
};

class  Node 
{

public:
    void MarkDirty();
     Node();
     Node(Vector3 position, Quaternion rotation,Vector3 scale);
     ~Node();
    void SetPosition(const Vector3& position);

   
    void SetPosition2D(const Vector2& position) { SetPosition(Vector3(position)); }

  
    void SetPosition2D(float x, float y) { SetPosition(Vector3(x, y, 0.0f)); }

  
    void SetRotation(const Quaternion& rotation);

    void SetRotation2D(float rotation) { SetRotation(Quaternion(rotation)); }

   
    void SetDirection(const Vector3& direction);
   
    void SetScale(float scale);
   
    void SetScale(const Vector3& scale);
    
    void SetScale2D(const Vector2& scale) { SetScale(Vector3(scale, 1.0f)); }
    
    void SetScale2D(float x, float y) { SetScale(Vector3(x, y, 1.0f)); }

    void SetTransform(const Vector3& position, const Quaternion& rotation);
   
    void SetTransform(const Vector3& position, const Quaternion& rotation, float scale);
 
    void SetTransform(const Vector3& position, const Quaternion& rotation, const Vector3& scale);
 
    void SetTransform(const Matrix3x4& matrix);

   
    void SetTransform2D(const Vector2& position, float rotation) { SetTransform(Vector3(position), Quaternion(rotation)); }

   
    void SetTransform2D(const Vector2& position, float rotation, float scale)
    {
        SetTransform(Vector3(position), Quaternion(rotation), scale);
    }

   
    void SetTransform2D(const Vector2& position, float rotation, const Vector2& scale)
    {
        SetTransform(Vector3(position), Quaternion(rotation), Vector3(scale, 1.0f));
    }

   
    void SetWorldPosition(const Vector3& position);

   
    void SetWorldPosition2D(const Vector2& position) { SetWorldPosition(Vector3(position)); }

   
    void SetWorldPosition2D(float x, float y) { SetWorldPosition(Vector3(x, y, 0.0f)); }

   
    void SetWorldRotation(const Quaternion& rotation);

   
    void SetWorldRotation2D(float rotation) { SetWorldRotation(Quaternion(rotation)); }

  
    void SetWorldDirection(const Vector3& direction);
   
    void SetWorldScale(float scale);
   
    void SetWorldScale(const Vector3& scale);

   
    void SetWorldScale2D(const Vector2& scale) { SetWorldScale(Vector3(scale, 1.0f)); }

    void SetWorldScale2D(float x, float y) { SetWorldScale(Vector3(x, y, 1.0f)); }

    void SetWorldTransform(const Vector3& position, const Quaternion& rotation);
    void SetWorldTransform(const Vector3& position, const Quaternion& rotation, float scale);
    void SetWorldTransform(const Vector3& position, const Quaternion& rotation, const Vector3& scale);
    void SetWorldTransform(const Matrix3x4& worldTransform);

  
    void SetWorldTransform2D(const Vector2& position, float rotation)
    {
        SetWorldTransform(Vector3(position), Quaternion(rotation));
    }

   
    void SetWorldTransform2D(const Vector2& position, float rotation, float scale)
    {
        SetWorldTransform(Vector3(position), Quaternion(rotation), scale);
    }

    
    void SetWorldTransform2D(const Vector2& position, float rotation, const Vector2& scale)
    {
        SetWorldTransform(Vector3(position), Quaternion(rotation), Vector3(scale, 1.0f));
    }

    
    void Translate(const Vector3& delta, TransformSpace space = TS_LOCAL);

   
    void Translate2D(const Vector2& delta, TransformSpace space = TS_LOCAL) { Translate(Vector3(delta), space); }

   
    void Rotate(const Quaternion& delta, TransformSpace space = TS_LOCAL);

    
    void Rotate2D(float delta, TransformSpace space = TS_LOCAL) { Rotate(Quaternion(delta), space); }

   
    void RotateAround(const Vector3& point, const Quaternion& delta, TransformSpace space = TS_LOCAL);

  
    void RotateAround2D(const Vector2& point, float delta, TransformSpace space = TS_LOCAL)
    {
        RotateAround(Vector3(point), Quaternion(delta), space);
    }

   
    void Pitch(float angle, TransformSpace space = TS_LOCAL);
   
    void Yaw(float angle, TransformSpace space = TS_LOCAL);
    
    void Roll(float angle, TransformSpace space = TS_LOCAL);
    bool LookAt(const Vector3& target, const Vector3& up = Vector3::UP, TransformSpace space = TS_WORLD);
   
    void Scale(float scale);

    void Scale(const Vector3& scale);

    void Scale2D(const Vector2& scale) { Scale(Vector3(scale, 1.0f)); }
    
    void SetEnabled(bool enable);
   
    void SetDeepEnabled(bool enable);
    
    void ResetDeepEnabled();
    void SetEnabledRecursive(bool enable);

    void AddChild(Node* node, unsigned index = M_MAX_UNSIGNED);
  
    void RemoveChild(Node* node);
   
    void RemoveAllChildren();
   
    void RemoveChildren(bool removeReplicated, bool removeLocal, bool recursive);
  
    Node* Clone(CreateMode mode = REPLICATED);
     void Remove();
   
    void SetParent(Node* parent);


    
    unsigned GetID() const { return id_; }
    bool IsReplicated() const;


   
    Node* GetParent() const { return parent_; }


  
    bool IsChildOf(Node* node) const;

   
    bool IsEnabled() const { return enabled_; }

  
    bool IsEnabledSelf() const { return enabledPrev_; }

  
    const Vector3& GetPosition() const { return position_; }

  
    Vector2 GetPosition2D() const { return Vector2(position_.x_, position_.y_); }

   
    const Quaternion& GetRotation() const { return rotation_; }

 
    float GetRotation2D() const { return rotation_.RollAngle(); }

   
    Vector3 GetDirection() const { return rotation_ * Vector3::FORWARD; }

   
    Vector3 GetUp() const { return rotation_ * Vector3::UP; }

  
    Vector3 GetRight() const { return rotation_ * Vector3::RIGHT; }

   
    const Vector3& GetScale() const { return scale_; }


    Vector2 GetScale2D() const { return Vector2(scale_.x_, scale_.y_); }

   
    Matrix3x4 GetTransform() const { return Matrix3x4(position_, rotation_, scale_); }

    Vector3 GetWorldPosition() const
    {
        if (dirty_)
            UpdateWorldTransform();
        return worldTransform_.Translation();
    }

 
    Vector2 GetWorldPosition2D() const
    {
        Vector3 worldPosition = GetWorldPosition();
        return Vector2(worldPosition.x_, worldPosition.y_);
    }

   
    Quaternion GetWorldRotation() const
    {
        if (dirty_)
            UpdateWorldTransform();
        return worldRotation_;
    }

   
    float GetWorldRotation2D() const
    {
        return GetWorldRotation().RollAngle();
    }

  
    Vector3 GetWorldDirection() const
    {
        if (dirty_)
            UpdateWorldTransform();
        return worldRotation_ * Vector3::FORWARD;
    }

  
    Vector3 GetWorldUp() const
    {
        if (dirty_)
            UpdateWorldTransform();
        return worldRotation_ * Vector3::UP;
    }

  
    Vector3 GetWorldRight() const
    {
        if (dirty_)
            UpdateWorldTransform();

        return worldRotation_ * Vector3::RIGHT;
    }

   
    Vector3 GetWorldScale() const
    {
        if (dirty_)
            UpdateWorldTransform();

        return worldTransform_.Scale();
    }

    
    Vector3 GetSignedWorldScale() const;

  
    Vector2 GetWorldScale2D() const
    {
        Vector3 worldScale = GetWorldScale();
        return Vector2(worldScale.x_, worldScale.y_);
    }

   
    const Matrix3x4& GetWorldTransform() const
    {
        if (dirty_)
            UpdateWorldTransform();

        return worldTransform_;
    }

  
    Vector3 LocalToWorld(const Vector3& position) const;
  
    Vector3 LocalToWorld(const Vector4& vector) const;
   
    Vector2 LocalToWorld2D(const Vector2& vector) const;
    
    Vector3 WorldToLocal(const Vector3& position) const;
   
    Vector3 WorldToLocal(const Vector4& vector) const;
   
    Vector2 WorldToLocal2D(const Vector2& vector) const;

   
    bool IsDirty() const { return dirty_; }


    unsigned GetNumChildren(bool recursive = false) const;

 

    Node* GetChild(unsigned index) const;

    Node* GetChild(const char* name, bool recursive = false) const;



    void SetID(unsigned id);

 
    Node* CreateChild(unsigned id, CreateMode mode, bool temporary = false);
 
    unsigned GetNumPersistentChildren() const;

  

protected:

private:
  
    void SetEnabled(bool enable, bool recursive, bool storeSelf);
  
  
    void UpdateWorldTransform() const;
   
    void RemoveChild();
  
    void GetChildrenRecursive() const;
   
    mutable Matrix3x4 worldTransform_;
  
    mutable bool dirty_;
 
    bool enabled_;
   
    bool enabledPrev_;
    
protected:



private:

    Node* parent_;
    std::vector<Node*>child;
 
    unsigned id_;

    Vector3 position_;

    Quaternion rotation_;

    Vector3 scale_;

    mutable Quaternion worldRotation_;

protected:

};
}
