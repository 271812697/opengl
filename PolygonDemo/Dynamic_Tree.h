#pragma once
#include <assert.h>
#include<stack>
#include"Vertex.h"
#include"Polygon.h"

struct  TreeNode
{
    bool IsLeaf() const
    {
        return child1 == -1;
    }
    AABB aabb;
    //Ҷ�ڵ�ʱ��������Ķ����ָ��
    struct Polygon* userData;

    //���ϵĽڵ�ʹ��parent��δ����Ľڵ�ʹ��next
    union
    {
        int parent;
        int next;
    };

    int child1;
    int child2;
    // leaf = 0, free node = -1
    int height;
};
class DynamicTree
{
public:

    DynamicTree();

    ~DynamicTree();

    //Ϊһ������δ���һ������ڵ㣬������뵽���г�ΪҶ�ӽڵ�
    int CreateProxy(const AABB& aabb, struct Polygon* userData);

    void DestroyProxy(int proxyId);

    void* GetUserData(int proxyId) const;

    //��ѯ����һ������ڵ��AABB
    const AABB& GetFatAABB(int proxyId) const;

    //���ƽ������������нڵ��AABB
    std::vector<AABB> GetAllAABB();

    //���ҳ�����������polygn�ཻ�Ķ���Σ����ཻ�������ans��
    bool Query(std::vector<struct Polygon>& ans,  struct Polygon& ploygn) ;

private:

    int AllocateNode();
    void FreeNode(int node);
    void InsertLeaf(int node);
    void RemoveLeaf(int node);
    int Balance(int index);
    int m_root;
    TreeNode* m_nodes;
    int m_nodeCount;
    int m_nodeCapacity;
    int m_freeList;
    int m_insertionCount;
};

inline void* DynamicTree::GetUserData(int proxyId) const
{
    assert(0 <= proxyId && proxyId < m_nodeCapacity);
    return m_nodes[proxyId].userData;
}

inline const AABB& DynamicTree::GetFatAABB(int proxyId) const
{
    assert(0 <= proxyId && proxyId < m_nodeCapacity);
    return m_nodes[proxyId].aabb;
}

inline std::vector<AABB> DynamicTree::GetAllAABB()
{
    std::vector<AABB>ans;
    for (int i = 0; i < m_nodeCount; i++) {
        ans.push_back(m_nodes[i].aabb);
    }
    return ans;
}


inline bool DynamicTree::Query(std::vector<struct Polygon>& ans,  struct Polygon& polygon) 
{
    //ʹ��ջ������������
    std::stack<int> s;
    s.push(m_root);
    AABB aabb = polygon.GetAABB();
  
    while (!s.empty())
    {
        int nodeId = s.top();
        s.pop();
        if (nodeId == -1)
        {
            continue;
        }
        const TreeNode* node = m_nodes + nodeId;
        //�뵱ǰ�ڵ��AABB�ཻ�Ž��б�������
        if (TestOverlap(node->aabb, aabb))
        {
            //Ҷ�ӽڵ�����ཻ����
            if (node->IsLeaf())
            {
                if (node->userData != nullptr && node->userData != &polygon) {
                    std::vector<struct Polygon>temp;
                    if (IntersectionPolygons(*node->userData, polygon, temp)) {
                        ans.insert(ans.end(), temp.begin(), temp.end());
                    }
                }
            }
            else
            {
                //������������
                s.push(node->child1);
                s.push(node->child2);
            }
        }
    }
    return ans.size() != 0;
}

