#include <Graphexia/algo/hakimi.hpp>
#include <algorithm>
#include <cmath>
#include <numbers>
#include <numeric>
#include <raylib.h>

namespace gpx {
    struct SequenceVertex {
        usize id;
        usize degree;
    };

    bool IsValidSequence(std::span<usize> sequence) {
        usize degreesSum = std::accumulate(sequence.begin(), sequence.end(), 0);

        // Invalid progression, the sum of all degrees is not even
        if(degreesSum & 1) {
            return false;
        }

        std::vector<usize> degrees(sequence.begin(), sequence.end());

        do {
            std::sort(degrees.begin(), degrees.end());

            usize lastDegreeIndex = degrees.size() - 1;
            usize highestDegree = degrees[lastDegreeIndex];
            degrees.pop_back();

            if(highestDegree == 0)
                return true;

            if(lastDegreeIndex < highestDegree)
                return false;

            usize firstDegreeAffected = lastDegreeIndex - highestDegree;
            for(usize i = lastDegreeIndex; i > firstDegreeAffected; --i) {
                usize index = i - 1;
                
                if(degrees[index] == 0)
                    return false;

                --degrees[index];
            }
        } while(degrees.size() > 0);

        return true;
    }

    void RenderSequence(Graph &graph, std::span<usize> sequence) {
        std::vector<SequenceVertex> vertices(sequence.size());

        f32 angleIncrement = 2 * std::numbers::pi / sequence.size();
        f32 currentAngle = 0;
        for (usize i = 0; i < sequence.size(); ++i) {
            i16 x = static_cast<i16>(50 * std::cos(currentAngle));
            i16 y = static_cast<i16>(50 * std::sin(currentAngle));
            vertices[i] = SequenceVertex{graph.AddVertex(x, y), sequence[i]}; 

            currentAngle += angleIncrement;
        }

        do {
            std::sort(vertices.begin(), vertices.end(), [](SequenceVertex l, SequenceVertex r) { return l.degree < r.degree; });

            usize lastDegreeIndex = vertices.size() - 1;
            SequenceVertex highestDegree = vertices[lastDegreeIndex];
            vertices.pop_back();

            usize firstDegreeAffected = lastDegreeIndex - highestDegree.degree;
            for(usize i = lastDegreeIndex; i > firstDegreeAffected; --i) {
                usize index = i - 1;
                SequenceVertex& seqVertex = vertices[index]; 

                graph.AddEdge(highestDegree.id, seqVertex.id);
                --seqVertex.degree;
            } 
        } while(vertices.size() > 0);
    }
} // namespace gpx
