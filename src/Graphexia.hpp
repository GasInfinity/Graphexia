#ifndef _GRAPHEXIA_APP_GRAPHEXIA__HPP_
#define _GRAPHEXIA_APP_GRAPHEXIA__HPP_

#include "Core.hpp"
#include "GraphView.hpp"
#include "GPXRenderer.hpp"

#include <Graphexia/Algo/Kruskal.hpp>
#include <Graphexia/Algo/BFS.hpp>
#include <Graphexia/Algo/DFS.hpp>

#include <sokol/sokol_app.h>
#include <nuklear/nuklear.h>


class Graphexia final {
public:
    Graphexia(); 
    
    void Init();
    void Update(f32 dt, nk_context* ctx);
    void Render();

    void Event(const sapp_event* event);
private:
    void ClearLastSelection();
    void Select(const SelectionType type, const usize id);
    void RequestSelectedDeletion();

    void AddVertex(f32x2 position);
    void AddEdge(usize from, usize to);
    void EraseVertex(usize id);
    void EraseEdge(usize id);

    void ChangeMode(GraphexiaMode mode);

    GraphView view;
    GPXRenderer renderer;

    GraphexiaMode mode;
    SelectionType selectionType;
    usize selectedId;
    f32x2 selectedVertexMouseOffset;

    bool movingCamera;
    bool currentlyDraggingVertex;
    f32x2 currentMouseWorldPosition;

    i32 savedSelectedRadius;
    i32 savedSelectedKComplete;

    i32 savedHavelHakimiSequenceLength;
    char savedHavelHakimiSequence[256];
    bool renderHakimiRandom;
    std::vector<usize> havelHakimiSequence;

    gpx::KruskalState kruskalState;

    i32 initialVertex, endVertex;
    gpx::BFSState bfsState;
    gpx::DFSState dfsState;
};

#endif
