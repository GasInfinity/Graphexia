#include <Graphexia/Algo/Hakimi.hpp>
#include <algorithm>
#include <cmath>
#include <numeric>

namespace gpx {
    struct SequenceVertex {
        usize id;
        usize degree;
    };

    bool IsGraphicSequence(std::span<usize> sequence) {
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

    Graph CreateFromGraphicSequence(std::span<usize> sequence) {
        Graph sequenceGraph(sequence.size());

        std::vector<SequenceVertex> vertices(sequence.size());

        for (usize i = 0; i < sequence.size(); ++i) {
            vertices[i] = SequenceVertex{i, sequence[i]}; 
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

                sequenceGraph.AddEdge(highestDegree.id, seqVertex.id);
                --seqVertex.degree;
            } 
        } while(vertices.size() > 0);

        return sequenceGraph;
    }
} // namespace gpx
