// TODO: Use SDL3 then available

#include <Graphexia/graph.hpp>
#include <iostream>
#include <raylib.h>
#include <vector>
#include <string>

enum class GraphMode : u8 {
    EditVertices,
    DrawEdges,
};

void RenderGraph(const gpx::Graph& graph) {
    const std::vector<gpx::Vertex>& vertices = graph.Vertices();
    const std::vector<gpx::Edge>& edges = graph.Edges();

    for (const gpx::Edge& edge : edges) {
        const gpx::Vertex& from = vertices[edge.fromId]; 
        const gpx::Vertex& to = vertices[edge.toId]; 

        if(edge.fromId != edge.toId) {
            DrawLineEx({static_cast<f32>(from.x), static_cast<f32>(from.y)}, {static_cast<f32>(to.x), static_cast<f32>(to.y)}, 1, Color{255,255,255,255});
            continue;
        }

        DrawCircleLines(from.x - 3, from.y - 3, 8, {255, 255, 255, 255});
    }

    for (const gpx::Vertex& vertex : vertices) {
        DrawCircle(vertex.x, vertex.y, vertex.size, Color{255, 255, 255, 255}); 
    }
}

#include <Graphexia/graphtypes.hpp>

int main(void)
{
    gpx::Graph graph;

    InitWindow(1280, 720, "Graphexia");
    SetTargetFPS(60);

    GraphMode mode = GraphMode::EditVertices;
    usize selectedVertex = gpx::Graph::NoVertex;
    Vector2 movingVertexDifference = {};
    Camera2D camera = {{640, 360}, {}, 0, 1};
    while (!WindowShouldClose())
    {
        Vector2 mDt = GetMouseDelta();

        if(IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
            camera.offset.x += mDt.x;
            camera.offset.y += mDt.y;
        }

        camera.zoom += GetMouseWheelMove() * 0.04;

        if(IsKeyPressed(KEY_R)) {
            graph = gpx::Graph();
            selectedVertex = gpx::Graph::NoVertex;
        } else if(IsKeyPressed(KEY_V)) {
            selectedVertex = gpx::Graph::NoVertex;
            mode = GraphMode::EditVertices;
        } else if(IsKeyPressed(KEY_C)) {
            selectedVertex = gpx::Graph::NoVertex;
            mode = GraphMode::DrawEdges;
        } else if(IsKeyPressed(KEY_A)) {
            usize verticesSize = graph.Vertices().size();
            std::vector<usize> adjacencyMatrix = graph.GetAdjacencyMatrix();

            std::cout << "Adjacency matrix -----" << std::endl;
            for (usize i = 0; i < verticesSize; ++i) {
                usize currentRow = i * verticesSize;
                for (usize j = 0; j < verticesSize; ++j) {
                    std::cout << adjacencyMatrix[currentRow + j] << ' ';
                } 

                std::cout << '\n';
            }
            std::cout << "-----" << std::endl;
        } else if(IsKeyPressed(KEY_Q)) {
            usize verticesSize = graph.Vertices().size();
            usize edgesSize = graph.Edges().size();
            std::vector<gpx::IncidenceState> incidenceMatrix = graph.GetIncidenceMatrix();

            std::cout << "Incidence matrix -----" << std::endl;
            for (usize i = 0; i < verticesSize; ++i) {
                usize currentRow = i * edgesSize;
                for (usize j = 0; j < edgesSize; ++j) {
                    std::cout << static_cast<i16>(incidenceMatrix[currentRow + j]) << ' ';
                } 

                std::cout << '\n';
            }
            std::cout << "-----" << std::endl;
        } else if(IsKeyPressed(KEY_ONE)) {
            graph = gpx::Graph();
            gpx::RenderKComplete(graph, 10);
        } else if(IsKeyPressed(KEY_TWO)) {
            graph = gpx::Graph();
            gpx::RenderKComplete(graph, 14);
        } else if(IsKeyPressed(KEY_THREE)) {
            graph = gpx::Graph();
            gpx::RenderKComplete(graph, 18);
        } else if(IsKeyPressed(KEY_FOUR)) {
            graph = gpx::Graph();
            gpx::RenderKComplete(graph, 22);
        }
        
        Vector2 worldMousePosition = GetScreenToWorld2D(GetMousePosition(), camera);
        switch (mode) {
            case GraphMode::EditVertices: {
                if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    usize newSelected = graph.FindVertex(static_cast<i16>(worldMousePosition.x), static_cast<i16>(worldMousePosition.y));

                    if(newSelected == gpx::Graph::NoVertex && selectedVertex == gpx::Graph::NoVertex) {
                        graph.AddVertex(static_cast<i16>(worldMousePosition.x), static_cast<i16>(worldMousePosition.y));
                    }

                    selectedVertex = newSelected;

                    if(selectedVertex != gpx::Graph::NoVertex) {
                        const gpx::Vertex& vertex = graph.Vertices()[selectedVertex];

                        movingVertexDifference = {vertex.x - worldMousePosition.x, vertex.y - worldMousePosition.y};
                    }
                } else if(IsMouseButtonDown(MOUSE_BUTTON_LEFT) && selectedVertex != gpx::Graph::NoVertex) {
                    graph.MoveVertex(selectedVertex, worldMousePosition.x + movingVertexDifference.x, worldMousePosition.y + movingVertexDifference.y);
                }
                break;
            } 
            case GraphMode::DrawEdges: {
                if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    if(selectedVertex == gpx::Graph::NoVertex) {
                        selectedVertex = graph.FindVertex(worldMousePosition.x, worldMousePosition.y); 
                    } else {
                        usize nextSelectedNode = graph.FindVertex(worldMousePosition.x, worldMousePosition.y);

                        if(nextSelectedNode != gpx::Graph::NoVertex) {
                            graph.AddEdge(selectedVertex, nextSelectedNode); 
                            selectedVertex = nextSelectedNode;
                        }
                    }
                } else if(IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && selectedVertex != gpx::Graph::NoVertex) {
                    selectedVertex = gpx::Graph::NoVertex; 
                }
                break;
            }
        }

        BeginDrawing();
            ClearBackground(BLACK);

            BeginMode2D(camera);
                if(mode == GraphMode::DrawEdges && selectedVertex != gpx::Graph::NoVertex) {
                    const gpx::Vertex& v = graph.Vertices()[selectedVertex];
                    DrawLineEx({static_cast<f32>(v.x), static_cast<f32>(v.y)}, {static_cast<f32>(worldMousePosition.x), static_cast<f32>(worldMousePosition.y)}, 2.5, Color{255,255,255,255});
                }

                RenderGraph(graph);

                if(mode == GraphMode::EditVertices && selectedVertex != gpx::Graph::NoVertex) {
                    const gpx::Vertex& v = graph.Vertices()[selectedVertex];

                    std::string vertexInfo = "Vertex [" + std::to_string(v.id) + "]: degree [" + std::to_string(graph.DegreeOf(v.id)) + "]";
                    DrawText(vertexInfo.c_str(), v.x, v.y, 8, {200, 200, 200, 255});
                }
            EndMode2D();

            DrawText(mode == GraphMode::EditVertices ? "EditVertices" : "DrawEdges", 0, 0, 14, {0, 255, 0, 255});

            std::string verticesEdges = "Vertices: " + std::to_string(graph.Vertices().size()) + '\n' + "Edges: " + std::to_string(graph.Edges().size());
            DrawText(verticesEdges.c_str(), 0, 14, 14, {0, 255, 0, 255});
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
