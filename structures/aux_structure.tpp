/*****************************************************************************************
 *              MIT License                                                              *
 *                                                                                       *
 * Copyright (c) 2020 Gianmarco Cherchi, Marco Livesu, Riccardo Scateni e Marco Attene   *
 *                                                                                       *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this  *
 * software and associated documentation files (the "Software"), to deal in the Software *
 * without restriction, including without limitation the rights to use, copy, modify,    *
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to    *
 * permit persons to whom the Software is furnished to do so, subject to the following   *
 * conditions:                                                                           *
 *                                                                                       *
 * The above copyright notice and this permission notice shall be included in all copies *
 * or substantial portions of the Software.                                              *
 *                                                                                       *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,   *
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A         *
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT    *
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION     *
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE        *
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                *
 *                                                                                       *
 * Authors:                                                                              *
 *      Gianmarco Cherchi (g.cherchi@unica.it)                                           *
 *      https://people.unica.it/gianmarcocherchi/                                        *
 *                                                                                       *
 *      Marco Livesu (marco.livesu@ge.imati.cnr.it)                                      *
 *      http://pers.ge.imati.cnr.it/livesu/                                              *
 *                                                                                       *
 *      Riccardo Scateni (riccardo@unica.it)                                             *
 *      https://people.unica.it/riccardoscateni/                                         *
 *                                                                                       *
 *      Marco Attene (marco.attene@ge.imati.cnr.it)                                      *
 *      https://www.cnr.it/en/people/marco.attene/                                       *
 *                                                                                       *
 * ***************************************************************************************/

#ifndef AUX_STRUCTURE_TPP
#define AUX_STRUCTURE_TPP

#include "aux_structure.h"

inline void AuxiliaryStructure::initFromTriangleSoup(TriangleSoup &ts)
{
    num_original_vtx = ts.numVerts();
    num_original_tris = ts.numTris();

    coplanar_tris.resize(ts.numTris());


    tri2pts.resize(ts.numTris());
    edge2pts.resize(ts.numEdges());
    tri2segs.resize(ts.numTris());

    num_intersections = 0;
    num_tpi = 0;

    for(uint v_id = 0; v_id < ts.numVerts(); v_id++)
    {
        std::pair<uint, bool> ins = addVertexInSortedList(std::make_pair(ts.vert(v_id), v_id));
        assert(ins.second && "Error: Duplicated vertex in starting mesh");
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

inline const std::vector<uint> &AuxiliaryStructure::triangleIntersectionList(const uint &t_id) const
{
    assert(t_id < tri2tri.size());
    return tri2tri[t_id];
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

inline std::vector< std::vector<uint> > &AuxiliaryStructure::intersectionList()
{
    return tri2tri;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

inline void AuxiliaryStructure::clearStructure()
{
    coplanar_tris.clear();
    tri2pts.clear();
    edge2pts.clear();
    tri2tri.clear();
    tri2segs.clear();
    seg2tris.clear();
    sorted_vtx.clear();
    visited_pockets.clear();
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

inline bool AuxiliaryStructure::addVertexInTriangle(const uint &t_id, const uint &v_id)
{
    assert(t_id < tri2pts.size());
    return tri2pts[t_id].insert(v_id).second;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

inline bool AuxiliaryStructure::addVertexInEdge(const uint &e_id, const uint &v_id)
{
    assert(e_id < edge2pts.size());
    return edge2pts[e_id].insert(v_id).second;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

inline bool AuxiliaryStructure::addSegmentInTriangle(const uint &t_id, const UIPair &seg)
{
    assert(t_id < tri2segs.size());
    UIPair key_seg = uniquePair(seg);
    return  tri2segs[t_id].insert(key_seg).second;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

inline void AuxiliaryStructure::addTrianglesInSegment(const UIPair &seg, const std::vector<uint> &triangles)
{
    UIPair key_seg = uniquePair(seg);
    std::set<uint> tris;

    auto f = seg2tris.find(key_seg);

    if(f != seg2tris.end())
        tris = f->second;

    for(uint t : triangles)
        tris.insert(t);

    seg2tris[key_seg] = tris;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

inline void AuxiliaryStructure::addTrianglesInSegment(const UIPair &seg, const std::set<uint> &triangles)
{
    const std::vector<uint> triangles_vec(triangles.begin(), triangles.end());
    addTrianglesInSegment(seg, triangles_vec);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

inline void AuxiliaryStructure::splitSegmentInSubSegments(const uint &orig_v0, const uint &orig_v1, const uint &midpoint)
{
    std::set<uint> tris = segmentTrianglesList(std::make_pair(orig_v0, orig_v1));

    UIPair sub_seg0 = uniquePair(std::make_pair(orig_v0, midpoint));
    UIPair sub_seg1 = uniquePair(std::make_pair(midpoint, orig_v1));

    seg2tris[sub_seg0] = tris;
    seg2tris[sub_seg1] = tris;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

inline void AuxiliaryStructure::addCoplanarTriangles(const uint &ta, const uint &tb)
{
    assert(ta != tb);
    assert(ta < coplanar_tris.size() && tb < coplanar_tris.size());

    coplanar_tris[ta].push_back(tb);
    coplanar_tris[tb].push_back(ta);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

inline const std::vector<uint> &AuxiliaryStructure::coplanarTriangles(const uint &t_id) const
{
    assert(t_id < coplanar_tris.size());
    return coplanar_tris[t_id];
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

inline bool AuxiliaryStructure::triangleHasCoplanars(const uint &t_id) const
{
    assert(t_id < coplanar_tris.size());
    return (coplanar_tris[t_id].size() > 0);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

inline bool AuxiliaryStructure::triangleHasIntersections(const uint &t_id) const
{
    assert(t_id < tri2tri.size());
    return tri2tri[t_id].size() > 0;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

inline const std::vector< std::vector<uint> > &AuxiliaryStructure::intersectionList() const
{
    return tri2tri;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

inline const std::set<uint> &AuxiliaryStructure::trianglePointsList(const uint &t_id) const
{
    assert(t_id < tri2pts.size());
    return tri2pts[t_id];
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

inline const std::set<uint> &AuxiliaryStructure::edgePointsList(const uint &e_id) const
{
    assert(e_id < edge2pts.size());
    return edge2pts[e_id];
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

inline const std::set<UIPair> &AuxiliaryStructure::triangleSegmentsList(const uint &t_id) const
{
    assert(t_id < tri2segs.size());
    return tri2segs[t_id];
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

inline const std::set<uint> &AuxiliaryStructure::segmentTrianglesList(const UIPair &seg) const
{
    UIPair key_seg = uniquePair(seg);

    auto res = seg2tris.find(key_seg);
    assert(res != seg2tris.end());

    return res->second;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

inline std::pair<uint, bool> AuxiliaryStructure::addVertexInSortedList(const std::pair<const genericPoint*, uint> &vtx_pair)
{
    std::pair<uint, bool> res;
    auto ins = sorted_vtx.insert(vtx_pair);

    res.second = ins.second;
    if(ins.second == false) // vertex not added (already present)
        res.first = ins.first->second;

    return res;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

inline bool AuxiliaryStructure::addVisitedPolygonPocket(const std::set<uint> &polygon)
{
    return visited_pockets.insert(polygon).second;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

inline uint AuxiliaryStructure::numIntersections() const
{
    if(num_intersections < 0) return 0;
    return static_cast<uint>(num_intersections);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

inline void AuxiliaryStructure::incrementNumTPI(const uint &num)
{
    num_tpi += num;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

inline void AuxiliaryStructure::incrementNumIntersections(const uint &num)
{
    num_intersections += num;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

inline uint AuxiliaryStructure::numTPI() const
{
    return num_tpi;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

inline UIPair AuxiliaryStructure::uniquePair(const UIPair &uip) const
{
    if(uip.first < uip.second) return  uip;
    return std::make_pair(uip.second, uip.first);
}


#endif // AUX_STRUCTURE_TPP
