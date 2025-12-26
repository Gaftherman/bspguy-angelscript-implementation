#pragma once
#include "Polygon3D.h"
#include <vector>
#include <unordered_set>

struct PolyOctant {
    vec3 min;
    vec3 max;
    vector<Polygon3D*> polygons;
    PolyOctant* children[8]; // Eight children octants

    PolyOctant(vec3 min, vec3 max);

    ~PolyOctant();

    void removePolygon(Polygon3D* polygon);
};

class PolygonOctree {
public:
    PolyOctant* root;
    int maxDepth;

    PolygonOctree(const vec3& min, const vec3& max, int depth);

    ~PolygonOctree();

    void insertPolygon(Polygon3D* polygon);

    void removePolygon(Polygon3D* polygon);

    bool isPolygonInOctant(Polygon3D* polygon, PolyOctant* node);

    void getPolysInRegion(Polygon3D* poly, vector<bool>& regionPolys);

    unordered_set<int> getPolysInRegion(Polygon3D* poly);

private:
    void buildOctree(PolyOctant* node, int currentDepth);

    void getPolysInRegion(PolyOctant* node, Polygon3D* poly, int currentDepth, vector<bool>& regionPolys);

    void getPolysInRegion(PolyOctant* node, Polygon3D* poly, int currentDepth, unordered_set<int>& regionPolys);

    void insertPolygon(PolyOctant* node, Polygon3D* polygon, int currentDepth);
};