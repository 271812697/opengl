#pragma once
#include"Vertex.h"

/*
* ����εĶ���
*/
struct Polygon {
    Polygon() = default;
    std::vector<LineSegment> Edges() {
        std::vector<LineSegment>ans;
        for (int i = 0; i < m_vertices.size() - 1; i++) {
            ans.push_back({ m_vertices[i],m_vertices[i + 1] });
        }
        ans.push_back({ m_vertices[m_vertices.size() - 1],m_vertices[0] });
        return ans;
    }
    AABB GetAABB() {
        Vec2 lower{1e9,1e9}, upper{ -1e9,-1e9 };
        for (Vertex v : m_vertices) {
            lower = Min(v,lower);
            upper = Max(v,upper);
        }
        return AABB{ lower,upper };
    }
    std::vector<Vertex>m_vertices;

};

/*
* �жϵ��Ƿ��ڶ������
* v ����
* lines ����εı�
* 
*/
bool VertexInPolygon(Vertex v, std::vector<LineSegment>lines);

/*
* ��һ�����������һ��������������еĶ���
* polygon ���Ķ����
* lines ��һ������εı�
* ���䷨�ж�һ����������еĶ����Ƿ�����һ��������ڣ����������еĽ��
* 
*/

std::vector<Vertex> PolygonInPolygon(struct Polygon polygon, std::vector<LineSegment>lines);



/*
* ��������͹����εĽ�����
* polygon1��polygon2��Ϊ͹�����
* ����������ε��߶εĽ��㣬���������һ�����������Ķ���
* �ռ���Ϻ����򣬷��ؽ����Ķ����
* 
*/
std::vector<Vertex> IntersectionPolygon(struct Polygon polygon1, struct Polygon polygon2);

/*
* �ж϶���ε�������˳ʱ�뻹����ʱ��
*/
bool PolygonIsAntiClockwise(std::vector<Vertex>& vertices);


/*
* �õ��������β����������
*/
std::vector<Vertex> GetVectorArrByPointsArr(const std::vector<Vertex> _points);

/*
* �ж������߶��Ƿ��ཻ�����ཻ�����뽻��
*/
bool LineIntersect3D(const Vertex& line1P1, const Vertex& line1P2, const Vertex& line2P1, const Vertex& line2P2, Vertex& intersectP);

/*
* ����������Χ�ռ�����εĶ��㣬���ڰ�����ηָ
* 
*/
std::vector<Vertex> GetPointsByIndexRange(const std::vector<Vertex> _points, int startIndex, int endIndex);

/*
*�԰�����ν��зָ�����ҵ��ĵ�һ���������ӳ��������󽻵㣬������λ���������
*/
bool GetRayIntersectionOfVecInVecArr(const std::vector<Vertex> _dirs, const std::vector<Vertex> _points,
    const int _vecIndex, const int _beginIndex, const int _endIndex, std::vector<Vertex>& _dividePolygonA,
    std::vector<Vertex>& _dividePolygonB);

/*
* ��������ηָ��͹�����
* �ȼ���Ƿ��ǰ�����Σ���������зָ����
*/
std::vector<std::vector<Vertex>> DividePolygonIfConcave(std::vector<Vertex> _points, bool _antiClockwise);

/*
*
* �Ӷ���εĶ���ָ���������Σ����ȼ����Ӧ�ĳ���
* 
*/
std::vector<std::vector<Vertex>>DividePolygonFromOrderVertexs(std::vector<Vertex> _points);

/*
* ���������������εĽ�����
* polygon1��polygon2��Ϊ��������
* 
* 
*/
bool IntersectionPolygons(struct Polygon& polygon1, struct Polygon& polygon2, std::vector<struct Polygon>& ans);

