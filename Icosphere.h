#pragma once
#include <vector>
#include <cmath>
#include <map>
#include <utility>
#include <GL/glut.h>
#include "Renderable.h"

struct Point3D {
    double x, y, z;
    Point3D(double X, double Y, double Z) : x(X), y(Y), z(Z) {}
};

struct TriangleIndices {
    int v1, v2, v3;
    TriangleIndices(int a, int b, int c) : v1(a), v2(b), v3(c) {}
};

class Icosphere : public Renderable {
public:
    Icosphere(int recursionLevel = 2, double radius = 1.0);
    void create();
    void draw(float t = 0.0f) const override;
    const std::vector<Point3D>& getVertices() const { return vertices; }
    const std::vector<TriangleIndices>& getFaces() const { return faces; }

private:
    int recursionLevel;
    double radius;
    std::vector<Point3D> vertices;
    std::vector<TriangleIndices> faces;

    std::map<std::pair<int,int>, int> middlePointCache;

    int addVertex(const Point3D& p);
    int getMiddlePoint(int p1, int p2);
};


