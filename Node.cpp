#include"Node.h"
using namespace std;
namespace Opengl
{
    void Node::MarkDirty()
    {
        dirty_ = true;
        for (auto it:child) {
            it->MarkDirty();

        }
    }
    Node::Node() :
        dirty_(false),
        parent_(nullptr),
        enabled_(true),
        id_(0),
        position_(Vector3::ZERO),
        rotation_(Quaternion::IDENTITY),
        scale_(Vector3::ONE),
        worldRotation_(Quaternion::IDENTITY)
{

}
    Node::Node(Vector3 position, Quaternion rotation, Vector3 scale):
        dirty_(false),
        parent_(nullptr),
        enabled_(true),
        id_(0),
        position_(position),
        rotation_(rotation),
        scale_(scale)   
    {
        UpdateWorldTransform();
    }
Node::~Node()
{
    RemoveAllChildren();

}
void Node::SetPosition(const Vector3& position)
{
    position_ = position;
    MarkDirty();

}

void Node::SetRotation(const Quaternion& rotation)
{
    rotation_ = rotation;
    MarkDirty();
}

void Node::SetDirection(const Vector3& direction)
{
    SetRotation(Quaternion(Vector3::FORWARD, direction));
    MarkDirty();
}

void Node::SetScale(float scale)
{
    SetScale(Vector3(scale, scale, scale));
    MarkDirty();
}

void Node::SetScale(const Vector3& scale)
{
    scale_ = scale;
    if (scale_.x_ == 0.0f)
        scale_.x_ = M_EPSILON;
    if (scale_.y_ == 0.0f)
        scale_.y_ = M_EPSILON;
    if (scale_.z_ == 0.0f)
        scale_.z_ = M_EPSILON;
    MarkDirty();
}

void Node::SetTransform(const Vector3& position, const Quaternion& rotation)
{
    position_ = position;
    rotation_ = rotation;
    MarkDirty();
}

void Node::SetTransform(const Vector3& position, const Quaternion& rotation, float scale)
{
    SetTransform(position, rotation, Vector3(scale, scale, scale));
    MarkDirty();
}

void Node::SetTransform(const Vector3& position, const Quaternion& rotation, const Vector3& scale)
{
    position_ = position;
    rotation_ = rotation;
    scale_ = scale;
    MarkDirty();
   
}

void Node::SetTransform(const Matrix3x4& matrix)
{
    SetTransform(matrix.Translation(), matrix.Rotation(), matrix.Scale());
    MarkDirty();
}

void Node::SetWorldPosition(const Vector3& position)
{
    SetPosition((!parent_) ? position : parent_->GetWorldTransform().Inverse() * position);
    MarkDirty();
}

void Node::SetWorldRotation(const Quaternion& rotation)
{
    SetRotation((!parent_) ? rotation : parent_->GetWorldRotation().Inverse() * rotation);

}

void Node::SetWorldDirection(const Vector3& direction)
{
    Vector3 localDirection = ( !parent_) ? direction : parent_->GetWorldRotation().Inverse() * direction;
    SetRotation(Quaternion(Vector3::FORWARD, localDirection));
    MarkDirty();
}

void Node::SetWorldScale(float scale)
{
    SetWorldScale(Vector3(scale, scale, scale));
    MarkDirty();
}

void Node::SetWorldScale(const Vector3& scale)
{
    SetScale((!parent_) ? scale : scale / parent_->GetWorldScale());
    MarkDirty();
}

void Node::SetWorldTransform(const Vector3& position, const Quaternion& rotation)
{
    SetWorldPosition(position);
    SetWorldRotation(rotation);
    MarkDirty();
}

void Node::SetWorldTransform(const Vector3& position, const Quaternion& rotation, float scale)
{
    SetWorldPosition(position);
    SetWorldRotation(rotation);
    SetWorldScale(scale);
    MarkDirty();
}

void Node::SetWorldTransform(const Vector3& position, const Quaternion& rotation, const Vector3& scale)
{
    SetWorldPosition(position);
    SetWorldRotation(rotation);
    SetWorldScale(scale);
    MarkDirty();
}

void Node::SetWorldTransform(const Matrix3x4& worldTransform)
{
    SetWorldTransform(worldTransform.Translation(), worldTransform.Rotation(), worldTransform.Scale());
    MarkDirty();
}

void Node::Translate(const Vector3& delta, TransformSpace space)
{
    switch (space)
    {
    case TS_LOCAL:
        // Note: local space translation disregards local scale for scale-independent movement speed
        position_ += rotation_ * delta;//???
        break;

    case TS_PARENT:
        position_ += delta;
        break;

    case TS_WORLD:
        position_ += ( !parent_) ? delta : parent_->GetWorldTransform().Inverse() * Vector4(delta, 0.0f);
        break;
    }
    MarkDirty();

}

void Node::Rotate(const Quaternion& delta, TransformSpace space)
{
    switch (space)
    {
    case TS_LOCAL:
        rotation_ = (rotation_ * delta).Normalized();
        break;

    case TS_PARENT:
        rotation_ = (delta * rotation_).Normalized();
        break;

    case TS_WORLD:
        if ( !parent_)
            rotation_ = (delta * rotation_).Normalized();
        else
        {
            Quaternion worldRotation = GetWorldRotation();
            rotation_ = rotation_ * worldRotation.Inverse() * delta * worldRotation;
        }
        break;
    }
    MarkDirty();
}

void Node::RotateAround(const Vector3& point, const Quaternion& delta, TransformSpace space)
{
    Vector3 parentSpacePoint;
    Quaternion oldRotation = rotation_;
    switch (space)
    {
    case TS_LOCAL:
        parentSpacePoint = GetTransform() * point;
        rotation_ = (rotation_ * delta).Normalized();
        break;

    case TS_PARENT:
        parentSpacePoint = point;
        rotation_ = (delta * rotation_).Normalized();
        break;

    case TS_WORLD:
        if (!parent_)
        {
            parentSpacePoint = point;
            rotation_ = (delta * rotation_).Normalized();
        }
        else
        {
            parentSpacePoint = parent_->GetWorldTransform().Inverse() * point;
            Quaternion worldRotation = GetWorldRotation();
            rotation_ = rotation_ * worldRotation.Inverse() * delta * worldRotation;
        }
        break;
    }
    Vector3 oldRelativePos = oldRotation.Inverse() * (position_ - parentSpacePoint);
    position_ = rotation_ * oldRelativePos + parentSpacePoint;
    MarkDirty();
}

void Node::Yaw(float angle, TransformSpace space)
{
    Rotate(Quaternion(angle, Vector3::UP), space);
    MarkDirty();
}

void Node::Pitch(float angle, TransformSpace space)
{
    Rotate(Quaternion(angle, Vector3::RIGHT), space);
    MarkDirty();
}

void Node::Roll(float angle, TransformSpace space)
{
    Rotate(Quaternion(angle, Vector3::FORWARD), space);
    MarkDirty();
}

bool Node::LookAt(const Vector3& target, const Vector3& up, TransformSpace space)
{
    Vector3 worldSpaceTarget;

    switch (space)
    {
    case TS_LOCAL:
        worldSpaceTarget = GetWorldTransform() * target;
        break;

    case TS_PARENT:
        worldSpaceTarget = ( !parent_) ? target : parent_->GetWorldTransform() * target;
        break;

    case TS_WORLD:
        worldSpaceTarget = target;
        break;
    }
    Vector3 lookDir = worldSpaceTarget - GetWorldPosition();
    // Check if target is very close, in that case can not reliably calculate lookat direction
    if (lookDir.Equals(Vector3::ZERO))
        return false;
    Quaternion newRotation;
    // Do nothing if setting look rotation failed
    if (!newRotation.FromLookRotation(lookDir, up))
        return false;
    SetWorldRotation(newRotation);
    MarkDirty();
    return true;
}

void Node::Scale(float scale)
{
    Scale(Vector3(scale, scale, scale));
}

void Node::Scale(const Vector3& scale)
{
    scale_ *= scale;
    MarkDirty();
}

void Node::SetEnabled(bool enable)
{
    SetEnabled(enable, false, true);
}

void Node::SetDeepEnabled(bool enable)
{
    SetEnabled(enable, true, false);
}

void Node::ResetDeepEnabled()
{
    SetEnabled(enabledPrev_, false, false);

}

void Node::SetEnabledRecursive(bool enable)
{
    SetEnabled(enable, true, true);
}
void Node::AddChild(Node* node, unsigned index)
{
    node->id_ = index;
    node->parent_ = this;
    child.push_back(node);
}

void Node::RemoveChild(Node* node)
{
    if (!node)
        return;
    int i;
    for (i = 0; i < child.size(); i++) {
        if (child[i] == node) {
            child[i]->parent_ = nullptr;
            break;
        }
    }
    for (int k = i; k < child.size()-1; k++) {
        child[k] = child[k + 1];
    }
    if(i<child.size())child.pop_back();
}

void Node::RemoveAllChildren()
{
    RemoveChildren(true, true, true);
}

void Node::RemoveChildren(bool removeReplicated, bool removeLocal, bool recursive)
{
    
}


Node* Node::Clone(CreateMode mode)
{
   
    return nullptr;
}

void Node::Remove()
{
    if (parent_)
        parent_->RemoveChild(this);
}

void Node::SetParent(Node* parent)
{
    if (parent)
    {
        Matrix3x4 oldWorldTransform = GetWorldTransform();
        parent->AddChild(this);
        {
            Matrix3x4 newTransform = parent->GetWorldTransform().Inverse() * oldWorldTransform;
            SetTransform(newTransform.Translation(), newTransform.Rotation(), newTransform.Scale());
        }
    }
}

Vector3 Node::GetSignedWorldScale() const
{
    if (dirty_)
        UpdateWorldTransform();

    return worldTransform_.SignedScale(worldRotation_.RotationMatrix());
}

Vector3 Node::LocalToWorld(const Vector3& position) const
{
    return GetWorldTransform() * position;
}

Vector3 Node::LocalToWorld(const Vector4& vector) const
{
    return GetWorldTransform() * vector;
}

Vector2 Node::LocalToWorld2D(const Vector2& vector) const
{
    Vector3 result = LocalToWorld(Vector3(vector));
    return Vector2(result.x_, result.y_);
}

Vector3 Node::WorldToLocal(const Vector3& position) const
{
    return GetWorldTransform().Inverse() * position;
}

Vector3 Node::WorldToLocal(const Vector4& vector) const
{
    return GetWorldTransform().Inverse() * vector;
}

Vector2 Node::WorldToLocal2D(const Vector2& vector) const
{
    Vector3 result = WorldToLocal(Vector3(vector));
    return Vector2(result.x_, result.y_);
}

unsigned Node::GetNumChildren(bool recursive) const
{
    if (!recursive)
        return child.size();
    int ans = child.size();
    for (auto it : child) {
        ans += it->GetNumChildren(true);
    }
    return ans;
}
Node* Node::GetChild(unsigned index) const
{
    return nullptr;
}
Node* Node::GetChild(const char* name, bool recursive) const
{
    return nullptr;
}
bool Node::IsReplicated() const
{
    return false;
}
bool Node::IsChildOf(Node* node) const
{
    Node* parent = parent_;
    while (parent)
    {
        if (parent == node)
            return true;
        parent = parent->parent_;
    }
    return false;
}


void Node::SetID(unsigned id)
{
    id_ = id;
}



Node* Node::CreateChild(unsigned id, CreateMode mode, bool temporary)
{
   
    return nullptr;
}


unsigned Node::GetNumPersistentChildren() const
{
    unsigned ret = 0;
    return ret;
}


void Node::SetEnabled(bool enable, bool recursive, bool storeSelf)
{
   
}

void Node::UpdateWorldTransform() const
{
    Matrix3x4 transform = GetTransform();
    // Assume the root node (scene) has identity transform
    if (!parent_)
    {
        worldTransform_ = transform;
        worldRotation_ = rotation_;
    }
    else
    {
        worldTransform_ = parent_->GetWorldTransform() * transform;
        worldRotation_ = parent_->GetWorldRotation() * rotation_;
    }
    dirty_ = false;
}

void Node::RemoveChild()
{
 
}

void Node::GetChildrenRecursive() const
{
   
}
}
