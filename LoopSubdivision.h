#ifndef __LoopSubdivision_h__
#define __LoopSubdivision_h__
#include <unordered_map>
#include <vector>
#include "Mesh.h"

inline void LoopSubdivision(TriangleMesh<3>& mesh)
{
	std::vector<Vector3>& old_vtx=mesh.Vertices();
	std::vector<Vector3i>& old_tri=mesh.Elements();
	std::vector<Vector3> new_vtx;		////vertex array for the new mesh
	std::vector<Vector3i> new_tri;		////element array for the new mesh

    new_vtx = old_vtx; // copy all the old vertices to the new_vtx array

    std::unordered_map<Vector2i,int> edge_odd_vtx_map;
    std::unordered_map<Vector2i,std::vector<int> > edge_tri_map;
    std::unordered_map<int,std::vector<int> > vtx_vtx_map;

    // step 1
    for (int t = 0; t < old_tri.size(); t++) {
        int v1 = old_tri[t][0];
        int v2 = old_tri[t][1];
        int v3 = old_tri[t][2];

        Vector2i e1 = Sorted(Vector2i(v1, v2));
        Vector2i e2 = Sorted(Vector2i(v2, v3));
        Vector2i e3 = Sorted(Vector2i(v3, v1));

        Vector3 mid1 = (old_vtx[v1] + old_vtx[v2]) / 2.0;
        Vector3 mid2 = (old_vtx[v2] + old_vtx[v3]) / 2.0;
        Vector3 mid3 = (old_vtx[v3] + old_vtx[v1]) / 2.0;

        new_vtx.push_back(mid1);
        new_vtx.push_back(mid2);
        new_vtx.push_back(mid3);

        if (edge_odd_vtx_map.find(e1) == edge_odd_vtx_map.end()) {
            edge_odd_vtx_map[e1] = new_vtx.size() - 3;
        }

        if (edge_odd_vtx_map.find(e2) == edge_odd_vtx_map.end()) {
            edge_odd_vtx_map[e2] = new_vtx.size() - 2;
        }

        if (edge_odd_vtx_map.find(e3) == edge_odd_vtx_map.end()) {
            edge_odd_vtx_map[e3] = new_vtx.size() - 1;
        }

        Vector3i tri1 = Vector3i(v1, edge_odd_vtx_map[e1], edge_odd_vtx_map[e3]);
        Vector3i tri2 = Vector3i(edge_odd_vtx_map[e1], v2, edge_odd_vtx_map[e2]);
        Vector3i tri3 = Vector3i(edge_odd_vtx_map[e3], edge_odd_vtx_map[e2], v3);
        Vector3i tri4 = Vector3i(edge_odd_vtx_map[e1], edge_odd_vtx_map[e2], edge_odd_vtx_map[e3]);

        new_tri.push_back(tri1);
        new_tri.push_back(tri2);
        new_tri.push_back(tri3);
        new_tri.push_back(tri4);
    }

    // step 2
    for (int k = 0; k < old_tri.size(); k++) {
        int v1 = old_tri[k][0];
        int v2 = old_tri[k][1];
        int v3 = old_tri[k][2];

        Vector2i es = Sorted(Vector2i(v1, v2));
        Vector2i ed = Sorted(Vector2i(v2, v3));
        Vector2i ef = Sorted(Vector2i(v3, v1));

        if (edge_tri_map.find(es) != edge_tri_map.end()) {
            edge_tri_map[es].push_back(k);
        } else {
            edge_tri_map[es] = {k};
        }
        if (edge_tri_map.find(ed) != edge_tri_map.end()) {
            edge_tri_map[ed].push_back(k);
        } else {
            edge_tri_map[ed] = {k};
        }
        if (edge_tri_map.find(ef) != edge_tri_map.end()) {
            edge_tri_map[ef].push_back(k);
        } else {
            edge_tri_map[ef] = {k};
        }
    }


    for (const auto& e : edge_odd_vtx_map) {
        Vector2i et = e.first;
        int odd_v = e.second;
        int t0 = et[0];
        int t1 = et[1];

        std::vector<int> tri = edge_tri_map[et];
        std::vector<int> opp_arr = {};

        for (int j = 0; j < tri.size(); j++) {
            for (int l = 0; l < old_tri[tri[j]].size(); l++) {
                if (old_tri[tri[j]][l] != t0 && old_tri[tri[j]][l] != t1) {
                    opp_arr.push_back(old_tri[tri[j]][l]);
                }
            }
        }
    
        int opp_v0 = opp_arr[0];
        int opp_v1 = opp_arr[1];
        new_vtx[odd_v] = (0.375 * (old_vtx[t0] + old_vtx[t1])) + (0.125 * (old_vtx[opp_v0] + old_vtx[opp_v1]));
    }

    // step 3
    for (const auto& tri : old_tri) {
        for (int i = 0; i < 3; i++) {
            int v1 = tri[i];
            int v2 = tri[(i + 1) % 3];
            vtx_vtx_map[v1].push_back(v2);
            vtx_vtx_map[v2].push_back(v1);
        }
    }

    for (int i = 0; i < old_vtx.size(); i++) {
        std::vector<int> neighbors = vtx_vtx_map[i];
        double n = neighbors.size();
        double beta = (n > 3) ? 3.0 / (8 * n) : 3.0 / 16.0;

        Vector3 sumOfNeighbors(0, 0, 0);
        for (int neighbor : neighbors) {
            sumOfNeighbors += new_vtx[neighbor];
        }

        Vector3 updatedPosition = new_vtx[i] * (1 - n * beta) + sumOfNeighbors * beta;
        new_vtx[i] = updatedPosition;
    }

    old_vtx = new_vtx;
    old_tri = new_tri;
}

#endif