#include "Icosphere.h"

Icosphere::Icosphere(int recursion, double r) : recursionLevel(recursion), radius(r) {}

int Icosphere::addVertex(const Point3D& p) {
    // normaliser pour garder sur la sphère
    double length = std::sqrt(p.x*p.x + p.y*p.y + p.z*p.z);
    vertices.push_back(Point3D(radius * p.x / length,
                               radius * p.y / length,
                               radius * p.z / length));
    return vertices.size() - 1;
}

int Icosphere::getMiddlePoint(int p1, int p2) {
    std::pair<int,int> key = p1 < p2 ? std::make_pair(p1,p2) : std::make_pair(p2,p1);
    auto it = middlePointCache.find(key);
    if (it != middlePointCache.end()) return it->second;

    Point3D point(
        (vertices[p1].x + vertices[p2].x) / 2.0,
        (vertices[p1].y + vertices[p2].y) / 2.0,
        (vertices[p1].z + vertices[p2].z) / 2.0
    );

    int index = addVertex(point);
    middlePointCache[key] = index;
    return index;
}

void Icosphere::create() {
    middlePointCache.clear();
    vertices.clear();
    faces.clear();

    double t = (1.0 + std::sqrt(5.0)) / 2.0;

    addVertex(Point3D(-1,  t,  0));
    addVertex(Point3D( 1,  t,  0));
    addVertex(Point3D(-1, -t,  0));
    addVertex(Point3D( 1, -t,  0));
    addVertex(Point3D( 0, -1,  t));
    addVertex(Point3D( 0,  1,  t));
    addVertex(Point3D( 0, -1, -t));
    addVertex(Point3D( 0,  1, -t));
    addVertex(Point3D( t,  0, -1));
    addVertex(Point3D( t,  0,  1));
    addVertex(Point3D(-t,  0, -1));
    addVertex(Point3D(-t,  0,  1));

    faces = {
        {0,11,5},{0,5,1},{0,1,7},{0,7,10},{0,10,11},
        {1,5,9},{5,11,4},{11,10,2},{10,7,6},{7,1,8},
        {3,9,4},{3,4,2},{3,2,6},{3,6,8},{3,8,9},
        {4,9,5},{2,4,11},{6,2,10},{8,6,7},{9,8,1}
    };

    for (int i = 0; i < recursionLevel; i++) {
        std::vector<TriangleIndices> faces2;
        for (auto &tri : faces) {
            int a = getMiddlePoint(tri.v1, tri.v2);
            int b = getMiddlePoint(tri.v2, tri.v3);
            int c = getMiddlePoint(tri.v3, tri.v1);

            faces2.push_back({tri.v1, a, c});
            faces2.push_back({tri.v2, b, a});
            faces2.push_back({tri.v3, c, b});
            faces2.push_back({a, b, c});
        }
        faces = faces2;
    }
}

void Icosphere::draw(float t) const
{
    glBegin(GL_TRIANGLES);
    for (const auto& tri : faces) {
        const Point3D& p1 = vertices[tri.v1];
        const Point3D& p2 = vertices[tri.v2];
        const Point3D& p3 = vertices[tri.v3];

        // normales pour l’éclairage
        glNormal3f(p1.x, p1.y, p1.z);
        glVertex3f(p1.x, p1.y, p1.z);

        glNormal3f(p2.x, p2.y, p2.z);
        glVertex3f(p2.x, p2.y, p2.z);

        glNormal3f(p3.x, p3.y, p3.z);
        glVertex3f(p3.x, p3.y, p3.z);
    }
    glEnd();
}

