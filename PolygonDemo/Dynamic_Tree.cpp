#include"Dynamic_Tree.h"
#include <string.h>

DynamicTree::DynamicTree()
{
    m_root = -1;
    //ʹ��˳������ģ������
    m_nodeCapacity = 16;
    m_nodeCount = 0;
    m_nodes = (TreeNode*)malloc(m_nodeCapacity * sizeof(TreeNode));
    memset(m_nodes, 0, m_nodeCapacity * sizeof(TreeNode));
    //Ϊ���нڵ㽨������
    for (int i = 0; i < m_nodeCapacity - 1; ++i)
    {
        m_nodes[i].next = i + 1;
        m_nodes[i].height = -1;
    }
    m_nodes[m_nodeCapacity - 1].next = -1;
    m_nodes[m_nodeCapacity - 1].height = -1;
    m_freeList = 0;

    m_insertionCount = 0;
}

DynamicTree::~DynamicTree()
{

    free(m_nodes);
}

//��m_nodes������freelist�������һ���ڵ㣨������������
int DynamicTree::AllocateNode()
{

    //���ݲ���
    if (m_freeList == -1)
    {
        assert(m_nodeCount == m_nodeCapacity);


        //�����еĿ��ÿ��нڵ�Ϊ�գ�m_nodes����һ��
        TreeNode* oldNodes = m_nodes;
        m_nodeCapacity *= 2;
        m_nodes = (TreeNode*)malloc(m_nodeCapacity * sizeof(TreeNode));
        memcpy(m_nodes, oldNodes, m_nodeCount * sizeof(TreeNode));
        free(oldNodes);


        for (int i = m_nodeCount; i < m_nodeCapacity - 1; ++i)
        {
            m_nodes[i].next = i + 1;
            m_nodes[i].height = -1;
        }
        m_nodes[m_nodeCapacity - 1].next = -1;
        m_nodes[m_nodeCapacity - 1].height = -1;
        m_freeList = m_nodeCount;
    }


    int nodeId = m_freeList;
    m_freeList = m_nodes[nodeId].next;
    m_nodes[nodeId].parent = -1;
    m_nodes[nodeId].child1 = -1;
    m_nodes[nodeId].child2 = -1;
    m_nodes[nodeId].height = 0;
    m_nodes[nodeId].userData = nullptr;
    ++m_nodeCount;
    return nodeId;
}


//�����ýڵ㷵����m_nodes�����ӵ�freelist��
void DynamicTree::FreeNode(int nodeId)
{
    assert(0 <= nodeId && nodeId < m_nodeCapacity);
    assert(0 < m_nodeCount);
    m_nodes[nodeId].next = m_freeList;
    m_nodes[nodeId].height = -1;
    m_freeList = nodeId;
    --m_nodeCount;
}

//�ӽڵ���з���һ�����нڵ㣬���루��������id�������г�ΪҶ�ӽڵ�
int DynamicTree::CreateProxy(const AABB& aabb, struct Polygon* userData)
{
    int proxyId = AllocateNode();


    Vec2 r(0.1, 0.1);
    m_nodes[proxyId].aabb.lowerBound = aabb.lowerBound - r;
    m_nodes[proxyId].aabb.upperBound = aabb.upperBound + r;
    m_nodes[proxyId].userData = userData;
    m_nodes[proxyId].height = 0;
    InsertLeaf(proxyId);

    return proxyId;
}
//������Ҷ�ڵ�������Ƴ��������ظ��ڵ��
void DynamicTree::DestroyProxy(int proxyId)
{
    assert(0 <= proxyId && proxyId < m_nodeCapacity);
    assert(m_nodes[proxyId].IsLeaf());

    RemoveLeaf(proxyId);
    FreeNode(proxyId);
}

//��һ���ڵ���뵽����
void DynamicTree::InsertLeaf(int leaf)
{
    ++m_insertionCount;
    //���ڵ�Ϊ�գ�����Ϊ���ڵ㲢����
    if (m_root == -1)
    {
        m_root = leaf;
        m_nodes[m_root].parent = -1;
        return;
    }

    //Ϊ����Ҷ�ڵ��ҵ�һ����ѵ�index,Ҳ�����ֵܽڵ�(������С�Ľڵ�)
    AABB leafAABB = m_nodes[leaf].aabb;
    int index = m_root;
    while (m_nodes[index].IsLeaf() == false)
    {
        int child1 = m_nodes[index].child1;
        int child2 = m_nodes[index].child2;

        float area = m_nodes[index].aabb.GetPerimeter();

        AABB combinedAABB;
        combinedAABB.Combine(m_nodes[index].aabb, leafAABB);
        float combinedArea = combinedAABB.GetPerimeter();

        float cost = 2.0f * combinedArea;

        float inheritanceCost = 2.0f * (combinedArea - area);

        //��child1��Ϊ�ֵܽڵ�Ļ���
        float cost1;
        //����child1�Ƿ���Ҷ�ӽڵ㣬������
        if (m_nodes[child1].IsLeaf())
        {
            AABB aabb;
            aabb.Combine(leafAABB, m_nodes[child1].aabb);
            cost1 = aabb.GetPerimeter() + inheritanceCost;
        }
        else
        {
            AABB aabb;
            aabb.Combine(leafAABB, m_nodes[child1].aabb);
            float oldArea = m_nodes[child1].aabb.GetPerimeter();
            float newArea = aabb.GetPerimeter();
            cost1 = (newArea - oldArea) + inheritanceCost;
        }

        //��child2��Ϊ�ֵܽڵ�Ļ���
        float cost2;
        if (m_nodes[child2].IsLeaf())
        {
            AABB aabb;
            aabb.Combine(leafAABB, m_nodes[child2].aabb);
            cost2 = aabb.GetPerimeter() + inheritanceCost;
        }
        else
        {
            AABB aabb;
            aabb.Combine(leafAABB, m_nodes[child2].aabb);
            float oldArea = m_nodes[child2].aabb.GetPerimeter();
            float newArea = aabb.GetPerimeter();
            cost2 = newArea - oldArea + inheritanceCost;
        }

        if (cost < cost1 && cost < cost2)
        {
            break;
        }

        if (cost1 < cost2)
        {
            index = child1;
        }
        else
        {
            index = child2;
        }
    }

    int sibling = index;
    int oldParent = m_nodes[sibling].parent;
    int newParent = AllocateNode();
    m_nodes[newParent].parent = oldParent;
    m_nodes[newParent].userData = nullptr;
    m_nodes[newParent].aabb.Combine(leafAABB, m_nodes[sibling].aabb);
    m_nodes[newParent].height = m_nodes[sibling].height + 1;

    if (oldParent != -1)
    {

        if (m_nodes[oldParent].child1 == sibling)
        {
            m_nodes[oldParent].child1 = newParent;
        }
        else
        {
            m_nodes[oldParent].child2 = newParent;
        }

        m_nodes[newParent].child1 = sibling;
        m_nodes[newParent].child2 = leaf;
        m_nodes[sibling].parent = newParent;
        m_nodes[leaf].parent = newParent;
    }
    else
    {
        m_nodes[newParent].child1 = sibling;
        m_nodes[newParent].child2 = leaf;
        m_nodes[sibling].parent = newParent;
        m_nodes[leaf].parent = newParent;
        m_root = newParent;
    }

    //����������ߺ�AABB
    index = m_nodes[leaf].parent;
    while (index != -1)
    {
        index = Balance(index);

        int child1 = m_nodes[index].child1;
        int child2 = m_nodes[index].child2;

        assert(child1 != -1);
        assert(child2 != -1);

        m_nodes[index].height = 1 + Max(m_nodes[child1].height, m_nodes[child2].height);
        m_nodes[index].aabb.Combine(m_nodes[child1].aabb, m_nodes[child2].aabb);

        index = m_nodes[index].parent;
    }

}
//�Ƴ�һ��Ҷ�ڵ�
void DynamicTree::RemoveLeaf(int leaf)
{
    if (leaf == m_root)
    {
        m_root = -1;
        return;
    }

    int parent = m_nodes[leaf].parent;
    int grandParent = m_nodes[parent].parent;
    int sibling;
    //�ҵ��ֵܽڵ�
    if (m_nodes[parent].child1 == leaf)
    {
        sibling = m_nodes[parent].child2;
    }
    else
    {
        sibling = m_nodes[parent].child1;
    }

    if (grandParent != -1)
    {
        if (m_nodes[grandParent].child1 == parent)
        {
            m_nodes[grandParent].child1 = sibling;
        }
        else
        {
            m_nodes[grandParent].child2 = sibling;
        }
        m_nodes[sibling].parent = grandParent;
        FreeNode(parent);


        int index = grandParent;
        while (index != -1)
        {
            index = Balance(index);

            int child1 = m_nodes[index].child1;
            int child2 = m_nodes[index].child2;

            m_nodes[index].aabb.Combine(m_nodes[child1].aabb, m_nodes[child2].aabb);
            m_nodes[index].height = 1 + Max(m_nodes[child1].height, m_nodes[child2].height);

            index = m_nodes[index].parent;
        }
    }
    else
    {
        m_root = sibling;
        m_nodes[sibling].parent = -1;
        FreeNode(parent);
    }
}

//ƽ�����ĸ߶�
int DynamicTree::Balance(int iA)
{
    assert(iA != -1);

    TreeNode* A = m_nodes + iA;
    if (A->IsLeaf() || A->height < 2)
    {
        return iA;
    }

    int iB = A->child1;
    int iC = A->child2;
    assert(0 <= iB && iB < m_nodeCapacity);
    assert(0 <= iC && iC < m_nodeCapacity);

    TreeNode* B = m_nodes + iB;
    TreeNode* C = m_nodes + iC;

    int balance = C->height - B->height;

    if (balance > 1)
    {
        int iF = C->child1;
        int iG = C->child2;
        TreeNode* F = m_nodes + iF;
        TreeNode* G = m_nodes + iG;
        assert(0 <= iF && iF < m_nodeCapacity);
        assert(0 <= iG && iG < m_nodeCapacity);

        C->child1 = iA;
        C->parent = A->parent;
        A->parent = iC;
        if (C->parent != -1)
        {
            if (m_nodes[C->parent].child1 == iA)
            {
                m_nodes[C->parent].child1 = iC;
            }
            else
            {
                assert(m_nodes[C->parent].child2 == iA);
                m_nodes[C->parent].child2 = iC;
            }
        }
        else
        {
            m_root = iC;
        }

        if (F->height > G->height)
        {
            C->child2 = iF;
            A->child2 = iG;
            G->parent = iA;
            A->aabb.Combine(B->aabb, G->aabb);
            C->aabb.Combine(A->aabb, F->aabb);

            A->height = 1 + Max(B->height, G->height);
            C->height = 1 + Max(A->height, F->height);
        }
        else
        {
            C->child2 = iG;
            A->child2 = iF;
            F->parent = iA;
            A->aabb.Combine(B->aabb, F->aabb);
            C->aabb.Combine(A->aabb, G->aabb);

            A->height = 1 + Max(B->height, F->height);
            C->height = 1 + Max(A->height, G->height);
        }

        return iC;
    }

    if (balance < -1)
    {
        int iD = B->child1;
        int iE = B->child2;
        TreeNode* D = m_nodes + iD;
        TreeNode* E = m_nodes + iE;
        assert(0 <= iD && iD < m_nodeCapacity);
        assert(0 <= iE && iE < m_nodeCapacity);

        B->child1 = iA;
        B->parent = A->parent;
        A->parent = iB;

        if (B->parent != -1)
        {
            if (m_nodes[B->parent].child1 == iA)
            {
                m_nodes[B->parent].child1 = iB;
            }
            else
            {
                assert(m_nodes[B->parent].child2 == iA);
                m_nodes[B->parent].child2 = iB;
            }
        }
        else
        {
            m_root = iB;
        }

        if (D->height > E->height)
        {
            B->child2 = iD;
            A->child1 = iE;
            E->parent = iA;
            A->aabb.Combine(C->aabb, E->aabb);
            B->aabb.Combine(A->aabb, D->aabb);

            A->height = 1 + Max(C->height, E->height);
            B->height = 1 + Max(A->height, D->height);
        }
        else
        {
            B->child2 = iE;
            A->child1 = iD;
            D->parent = iA;
            A->aabb.Combine(C->aabb, D->aabb);
            B->aabb.Combine(A->aabb, E->aabb);

            A->height = 1 + Max(C->height, D->height);
            B->height = 1 + Max(A->height, E->height);
        }

        return iB;
    }

    return iA;
}









