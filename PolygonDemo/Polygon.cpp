#include<iostream>
#include"Polygon.h"
/*
* �жϵ��Ƿ��ڶ������
* v ����
* lines ����εı�
*
*/

bool VertexInPolygon(Vertex v, std::vector<LineSegment>lines) {
    float x = v.x;
    float y = v.y;
    int cnt = 0;
    for (auto& line : lines) {
        //��պ��ڶ���εı���
        if (Intersection(v, line)) {
            return true;
        }
        float minY = Min(line.p1.y, line.p2.y);
        float maxY = Max(line.p1.y, line.p2.y);
        //ȥ�����Ķ˵㣬���Դ��������߶ι���������
        if (y >= minY && y < maxY) {
            float interX = (y - line.p1.y) * (line.p2.x - line.p1.x) / (line.p2.y - line.p1.y) + line.p1.x;
            if (x > interX) {
                cnt++;
            }
        }
    }
    return cnt % 2 != 0;
}


/*
* ��һ�����������һ��������������еĶ���
* polygon ���Ķ����
* lines ��һ������εı�
* ���䷨�ж�һ����������еĶ����Ƿ�����һ��������ڣ����������еĽ��
*
*/

std::vector<Vertex> PolygonInPolygon(struct Polygon polygon, std::vector<LineSegment>lines) {
    std::vector<Vertex>ans;
    for (auto& v : polygon.m_vertices) {
        if (VertexInPolygon(v, lines)) {
            ans.push_back(v);
        }
    }
    return ans;
}


/*
* �ж϶���a��b�ֱ���center���ɵļн�
*/

bool VertexCmp(const Vertex& a, const Vertex& b, const Vertex& center) {

    //����OA������OB�Ĳ��
    int det = (a.x - center.x) * (b.y - center.y) - (b.x - center.x) * (a.y - center.y);
    float angle_a = atan2(a.y - center.y, a.x - center.x);
    float angle_b = atan2(b.y - center.y, b.x - center.x);
    return angle_a < angle_b;
}


/*
* ��������͹����εĽ�����
* polygon1��polygon2��Ϊ͹�����
* ����������ε��߶εĽ��㣬���������һ�����������Ķ���
* �ռ���Ϻ����򣬷��ؽ����Ķ����
*
*/

std::vector<Vertex> IntersectionPolygon(struct Polygon polygon1, struct Polygon polygon2) {
    std::vector<LineSegment>line1 = polygon1.Edges();
    std::vector<LineSegment>line2 = polygon2.Edges();
    //����������߶��ཻ�Ľ���
    std::vector<Vertex> ans = IntersectionPloygonLines(line1, line2);

    //��������εĻ�������Ķ���
    std::vector<Vertex> l1 = PolygonInPolygon(polygon1, line2);
    std::vector<Vertex> l2 = PolygonInPolygon(polygon2, line1);
    ans.insert(ans.end(), l1.begin(), l1.end());
    ans.insert(ans.end(), l2.begin(), l2.end());
    //ð������������װ��һ��͹�����
    if (ans.size() != 0) {
        Vertex center = { 0,0 };
        float x = 0, y = 0;
        for (int i = 0; i < ans.size(); i++) {
            x += ans[i].x;
            y += ans[i].y;
        }
        center.x = x / (ans.size());
        center.y = y / (ans.size());
        for (int i = 0; i < ans.size(); i++) {
            for (int j = 0; j < ans.size() - 1; j++) {
                if (VertexCmp(ans[j], ans[j + 1], center)) {
                    Vertex tmp = ans[j];
                    ans[j] = ans[j + 1];
                    ans[j + 1] = tmp;
                }
            }
        }
    }
    return ans;
}


/*
* �ж϶���ε�������˳ʱ�뻹����ʱ��
*/

bool PolygonIsAntiClockwise(std::vector<Vertex>& vertices) {
    int n = vertices.size();
    //�����������
    if (n < 3)
        return true;
    float s = vertices[0].y * (vertices[n - 1].x - vertices[1].x);
    for (int i = 1; i < n; ++i)
        s += vertices[i].y * (vertices[i - 1].x - vertices[((i + 1) >= n ? 0 : (i + 1))].x);
    s = s * 0.5;
    return s > 0.f;
}


/*
* �õ��������β����������
*/

std::vector<Vertex> GetVectorArrByPointsArr(const std::vector<Vertex> _points)
{
    std::vector<Vertex> t_res;
    int t_pointsNum = _points.size();
    if (t_pointsNum > 1) {
        Vertex t_p1;
        Vertex t_p2;
        for (int i = 0; i < _points.size(); i++) {
            t_p1 = _points[i];
            if (i == t_pointsNum - 1) {
                t_p2 = _points[0];
            }
            else {
                t_p2 = _points[i + 1];
            }
            t_res.push_back(t_p2 - t_p1);
        }
    }
    return t_res;
}


/*
* �ж������߶��Ƿ��ཻ�����ཻ�����뽻��
*/

bool LineIntersect3D(const Vertex& line1P1, const Vertex& line1P2, const Vertex& line2P1, const Vertex& line2P2, Vertex& intersectP)
{
    Vertex v1 = line1P2 - line1P1;
    Vertex v2 = line2P2 - line2P1;

    if (abs(v1.x * v2.y - v1.y * v2.x) < 0.0001) {
        // ����ƽ��
        return false;
    }

    Vertex startPointSeg = line2P1 - line1P1;
    float vecS1 = v1.x * v2.y - v1.y * v2.x;
    float vecS2 = startPointSeg.x * v2.y - startPointSeg.y * v2.x;

    // ���������ֵ�����õ������Ϊ����������������߸�����������������������������������
    if (abs(vecS1 * vecS1) < 1e-5f) {
        return false;
    }
    float num2 = vecS2 / vecS1;
    intersectP = line1P1 + num2 * v1;
    Vertex v11 = intersectP - line2P1;
    //�Ƿ����߶�2�ӳ�����
    if ((v11.Size() / v2.Size()) > 1)
        return false;
    return true;
}


/*
* ����������Χ�ռ�����εĶ��㣬���ڰ�����ηָ
*
*/

std::vector<Vertex> GetPointsByIndexRange(const std::vector<Vertex> _points, int startIndex, int endIndex)
{
    std::vector<Vertex> pts;
    if (startIndex <= endIndex) {
        int idx = startIndex;
        while (idx <= endIndex && idx < _points.size()) {
            pts.push_back(_points[idx]);
            idx++;
        }
    }
    else {
        int idx = startIndex;
        while (idx < _points.size()) {
            pts.push_back(_points[idx]);
            idx++;
        }
        idx = 0;
        while (idx <= endIndex && idx < _points.size()) {
            pts.push_back(_points[idx]);
            idx++;
        }
    }
    return pts;
}


/*
*�԰�����ν��зָ�����ҵ��ĵ�һ���������ӳ��������󽻵㣬������λ���������
*/
bool GetRayIntersectionOfVecInVecArr(const std::vector<Vertex> _dirs, const std::vector<Vertex> _points,
    const int _vecIndex, const int _beginIndex, const int _endIndex, std::vector<Vertex>& _dividePolygonA,
    std::vector<Vertex>& _dividePolygonB)
{
    int t_dirsNum = _dirs.size(); //��������
    int t_pointsNum = _points.size(); //��ĸ���

    if (t_dirsNum > 3 && t_pointsNum > 3) {
        if (t_dirsNum == t_pointsNum) {
            if (_beginIndex >= 0 && _beginIndex < t_dirsNum) {
                if (_endIndex >= 0 && _endIndex < t_dirsNum) {
                    int t_indexNew; //����ͷ��Ӧ����±�
                    if (_vecIndex == (t_dirsNum - 1)) //Ϊ���������һ������
                        t_indexNew = 0;
                    else
                        t_indexNew = _vecIndex + 1;

                    Vertex t_beginA = _points[_vecIndex]; //������һ���������
                    float k = 1e14;
                    Vertex ExtendedVector = k * _dirs[_vecIndex];
                    Vertex t_endA = t_beginA + ExtendedVector; //���߶δ�������

                    Vertex t_intersectionPoint;
                    for (int j = _beginIndex; j <= _endIndex; j++) {
                        if (j != _vecIndex && j != t_indexNew) { //��������_vecIndex�Ķ˵�
                            Vertex t_beginB = _points[j];
                            if (LineIntersect3D(t_beginA, t_endA, t_beginB, t_beginB + _dirs[j], t_intersectionPoint)) {
                                //���ָ�Ķ���ε���ӵ�
                                std::cout << "���������±�: " << _vecIndex << ",  �ཻ�����±�: " << j << std::endl;

                                _dividePolygonA = GetPointsByIndexRange(_points, t_indexNew, j);
                                _dividePolygonA.push_back(t_intersectionPoint);

                                std::cout << "_dividePolygonA������������� " << _dividePolygonA.size() << std::endl;
                                _dividePolygonB = GetPointsByIndexRange(_points, j, t_indexNew - 1);
                                if (_dividePolygonB.size() > 0) {
                                    _dividePolygonB[0] = t_intersectionPoint;
                                }
                                std::cout << "_dividePolygonB������������� " << _dividePolygonB.size() << std::endl;

                                return true;
                            }
                        }
                    }
                }
            }
        }
    }
    return false;
}

/*
* ��������ηָ��͹�����
* �ȼ���Ƿ��ǰ�����Σ���������зָ����
*/
std::vector<std::vector<Vertex>> DividePolygonIfConcave(std::vector<Vertex> _points, bool _antiClockwise)
{
    std::vector<std::vector<Vertex>> polygonDividedArr;
    int t_pointsNum = _points.size();
    if (t_pointsNum < 3) {
        return polygonDividedArr;
    }
    else if (t_pointsNum == 3) {
        polygonDividedArr.push_back(_points);
        return polygonDividedArr;
    }
    else if (t_pointsNum > 3) {
        std::vector<Vertex> _dividePolygonA;
        std::vector<Vertex> _dividePolygonB;
        Vertex t_p1;
        Vertex t_p2;
        // ���㼯ת��Ϊ��β��ӵ�������
        std::vector<Vertex> t_dirs = GetVectorArrByPointsArr(_points);

        bool t_divideResult = false;
        //��һ�������±꣬Ҳ��ǰһ�������յ���±�
        int t_indexNew = 0;
        for (int i = 0; i < t_dirs.size(); i++) {
            t_p1 = t_dirs[i];
            if (i == t_dirs.size() - 1) {
                t_p2 = t_dirs[0];
                t_indexNew = 0;
            }
            else {
                t_p2 = t_dirs[i + 1];
                t_indexNew = i + 1;
            }

            double t_rotateDir = (t_p1.x * t_p2.y - t_p1.y * t_p2.x);
            //�����ǰ������
            if ((t_rotateDir < -0.0f && _antiClockwise == true) || (t_rotateDir > 0.0f && _antiClockwise == false)) {
                std::cout << "�������" << std::endl;
                //��ָ��
                t_divideResult = GetRayIntersectionOfVecInVecArr(t_dirs, _points, i, i, t_pointsNum - 1, _dividePolygonA, _dividePolygonB);
                if (t_divideResult == false) {
                    t_divideResult = GetRayIntersectionOfVecInVecArr(t_dirs, _points, i, 0, i - 1, _dividePolygonA, _dividePolygonB);
                }
                if (t_divideResult == false) {
                    std::cout << "�߶�" << i << " û�еõ��ָ��" << std::endl;
                }
                break;
            }
        }

        if (t_divideResult == false) {
            polygonDividedArr.push_back(_points);
        }
        else {
            if (_dividePolygonA.size() > 2) {
                std::vector<std::vector<Vertex>>temp = DividePolygonIfConcave(_dividePolygonA, _antiClockwise);
                polygonDividedArr.insert(polygonDividedArr.end(), temp.begin(), temp.end());
            }
            if (_dividePolygonB.size() > 2) {
                std::vector<std::vector<Vertex>>temp = DividePolygonIfConcave(_dividePolygonB, _antiClockwise);
                polygonDividedArr.insert(polygonDividedArr.end(), temp.begin(), temp.end());
            }
        }
    }
    return polygonDividedArr;
}

/*
*
* �Ӷ���εĶ���ָ���������Σ����ȼ����Ӧ�ĳ���
*
*/
std::vector<std::vector<Vertex>>DividePolygonFromOrderVertexs(std::vector<Vertex> _points)
{

    std::vector<std::vector<Vertex>> polygonDividedArr;
    if (_points.size() >= 3) {
        // �ж϶�����Ƿ���ʱ��
        bool _antiClockwise = PolygonIsAntiClockwise(_points);
        // ��ʼ�ָ�
        polygonDividedArr = DividePolygonIfConcave(_points, _antiClockwise); //�ݹ�

        if (polygonDividedArr.size() > 0) {
            std::cout << "�ָ�����θ�����" << polygonDividedArr.size() << std::endl;
        }
    }
    else {
        std::cout << "����ͼ�β��Ƕ����~~~~~~~~~~~" << std::endl;

    }
    return polygonDividedArr;
}

/*
* ���������������εĽ�����
* polygon1��polygon2��Ϊ��������
*
*
*/
bool IntersectionPolygons(struct Polygon& polygon1, struct Polygon& polygon2, std::vector<struct Polygon>&ans) {
    //�ȶ��������ν��зָ��������ηָ��͹�����
    auto p1 = DividePolygonFromOrderVertexs(polygon1.m_vertices);
    auto p2 = DividePolygonFromOrderVertexs(polygon2.m_vertices);
    
    for (auto it1 : p1) {
        for (auto it2 : p2) {
            std::vector<Vertex> temp = IntersectionPolygon({ it1 }, { it2 });
            if (temp.size() != 0) {
                ans.push_back({ temp });
            }
        }
    }
    
    return ans.size()!=0;
}


