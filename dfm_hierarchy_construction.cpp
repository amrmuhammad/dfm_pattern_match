#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <functional>

// --- Polygon and Layout Definitions ---

struct Point {
    double x, y;
    Point(double _x=0, double _y=0) : x(_x), y(_y) {}
};

struct Polygon {
    int id;
    std::string label; // e.g., shape type or feature type
    std::vector<Point> points;
    Point centroid() const {
        double cx = 0, cy = 0;
        for (const auto& p : points) {
            cx += p.x; cy += p.y;
        }
        return Point(cx/points.size(), cy/points.size());
    }
};

struct Edge {
    int from, to;
    Edge(int f, int t) : from(f), to(t) {}
};

struct Graph {
    std::vector<Polygon> nodes;
    std::vector<Edge> edges;

    // Adjacency list for quick access
    std::unordered_map<int, std::vector<int>> adj;

    void buildAdjacency() {
        adj.clear();
        for (const auto& e : edges) {
            adj[e.from].push_back(e.to);
            adj[e.to].push_back(e.from); // Assuming undirected graph
        }
    }
};

// --- VF2 Subgraph Isomorphism Algorithm Implementation ---

class VF2State {
public:
    const Graph& g1; // pattern graph (cell)
    const Graph& g2; // target graph (flat layout)
    std::unordered_map<int,int> core_1; // mapping from g1 nodes to g2 nodes
    std::unordered_map<int,int> core_2; // mapping from g2 nodes to g1 nodes
    std::unordered_set<int> mapped_1;
    std::unordered_set<int> mapped_2;

    VF2State(const Graph& pattern, const Graph& target)
        : g1(pattern), g2(target) {}

    bool isFeasiblePair(int n1, int n2) {
        // Check labels match
        if (g1.nodes[n1].label != g2.nodes[n2].label) return false;

        // Check adjacency consistency
        for (int adj1 : g1.adj.at(g1.nodes[n1].id)) {
            if (mapped_1.count(adj1)) {
                int mapped_adj2 = core_1[adj1];
                // Check if edge exists between n2 and mapped_adj2 in g2
                auto& neighbors = g2.adj.at(g2.nodes[n2].id);
                if (std::find(neighbors.begin(), neighbors.end(), mapped_adj2) == neighbors.end())
                    return false;
            }
        }
        return true;
    }

    void match(std::vector<std::unordered_map<int,int>>& results) {
        if (core_1.size() == g1.nodes.size()) {
            // Found a complete mapping
            results.push_back(core_1);
            return;
        }

        // Select next node in g1 to map
        int n1 = -1;
        for (const auto& node : g1.nodes) {
            if (mapped_1.count(node.id) == 0) {
                n1 = node.id;
                break;
            }
        }
        if (n1 == -1) return; // no node to map

        // Try all candidate nodes in g2
        for (const auto& node2 : g2.nodes) {
            int n2 = node2.id;
            if (mapped_2.count(n2) == 0 && isFeasiblePair(n1, n2)) {
                // Extend mapping
                core_1[n1] = n2;
                core_2[n2] = n1;
                mapped_1.insert(n1);
                mapped_2.insert(n2);

                match(results);

                // Backtrack
                core_1.erase(n1);
                core_2.erase(n2);
                mapped_1.erase(n1);
                mapped_2.erase(n2);
            }
        }
    }
};

// --- Hierarchy Construction ---

struct Cell {
    std::string name;
    Graph graph;
};

struct Instance {
    Cell* cell;
    double x_offset, y_offset;
};

struct Hierarchy {
    Cell top_cell;
    std::vector<Cell> cells;
    std::vector<Instance> instances;
};

// --- Utility Functions ---

// Compute bounding box centroid for placement offset
Point computeOffset(const Graph& flat, const std::unordered_map<int,int>& mapping, const Graph& pattern) {
    // Compute centroid of mapped nodes in flat
    double cx = 0, cy = 0;
    for (const auto& p : pattern.nodes) {
        int flat_id = mapping.at(p.id);
        const Polygon& flat_poly = flat.nodes[flat_id];
        Point c = flat_poly.centroid();
        cx += c.x;
        cy += c.y;
    }
    cx /= pattern.nodes.size();
    cy /= pattern.nodes.size();

    // Compute centroid of pattern nodes
    double pcx = 0, pcy = 0;
    for (const auto& p : pattern.nodes) {
        Point c = p.centroid();
        pcx += c.x;
        pcy += c.y;
    }
    pcx /= pattern.nodes.size();
    pcy /= pattern.nodes.size();

    return Point(cx - pcx, cy - pcy);
}

// Extract subgraph from flat graph given node IDs
Graph extractSubgraph(const Graph& flat, const std::unordered_set<int>& node_ids) {
    Graph sub;
    std::unordered_map<int,int> id_map; // old id -> new id
    int idx = 0;
    for (int id : node_ids) {
        for (const auto& n : flat.nodes) {
            if (n.id == id) {
                Polygon p = n;
                p.id = idx;
                sub.nodes.push_back(p);
                id_map[id] = idx++;
                break;
            }
        }
    }
    for (const auto& e : flat.edges) {
        if (node_ids.count(e.from) && node_ids.count(e.to)) {
            sub.edges.emplace_back(id_map[e.from], id_map[e.to]);
        }
    }
    sub.buildAdjacency();
    return sub;
}

// Remove nodes from flat graph that are part of instances
void removeNodes(Graph& flat, const std::unordered_set<int>& node_ids) {
    std::vector<Polygon> new_nodes;
    std::unordered_set<int> remove_set = node_ids;
    for (const auto& n : flat.nodes) {
        if (remove_set.count(n.id) == 0) new_nodes.push_back(n);
    }
    flat.nodes = new_nodes;

    std::vector<Edge> new_edges;
    for (const auto& e : flat.edges) {
        if (remove_set.count(e.from) == 0 && remove_set.count(e.to) == 0)
            new_edges.push_back(e);
    }
    flat.edges = new_edges;
    flat.buildAdjacency();
}

// --- Main ---

int main() {
    // Step 1: Create a flat layout graph with polygons and edges
    Graph flat;

    // Example polygons (IDs must be unique)
    flat.nodes = {
        {0, "rect", {{0,0},{1,0},{1,1},{0,1}}},
        {1, "rect", {{1,0},{2,0},{2,1},{1,1}}},
        {2, "rect", {{1,1},{2,1},{2,2},{1,2}}},
        {3, "rect", {{3,0},{4,0},{4,1},{3,1}}},
        {4, "rect", {{4,0},{5,0},{5,1},{4,1}}},
        {5, "rect", {{4,1},{5,1},{5,2},{4,2}}},
        {6, "rect", {{6,0},{7,0},{7,1},{6,1}}},
        {7, "rect", {{7,0},{8,0},{8,1},{7,1}}},
        {8, "rect", {{7,1},{8,1},{8,2},{7,2}}}
    };

    // Edges representing adjacency
    flat.edges = {
        {0,1},{1,2},
        {3,4},{4,5},
        {6,7},{7,8}
    };

    flat.buildAdjacency();

    // Step 2: Define a pattern graph (cell) to search for (e.g., the "L"-shape of 3 polygons)
    Graph pattern;
    pattern.nodes = {
        {0, "rect", {{0,0},{1,0},{1,1},{0,1}}},
        {1, "rect", {{1,0},{2,0},{2,1},{1,1}}},
        {2, "rect", {{1,1},{2,1},{2,2},{1,2}}}
    };
    pattern.edges = {
        {0,1},{1,2}
    };
    pattern.buildAdjacency();

    // Step 3: Run VF2 subgraph isomorphism to find all matches of pattern in flat graph
    VF2State vf2(pattern, flat);
    std::vector<std::unordered_map<int,int>> matches;
    vf2.match(matches);

    std::cout << "Found " << matches.size() << " instances of the pattern.\n";

    // Step 4: Extract the cell and instances
    Cell cell;
    cell.name = "L_CELL";
    {
        std::unordered_set<int> node_ids;
        for (const auto& kv : matches[0]) node_ids.insert(kv.first);
        cell.graph = extractSubgraph(pattern, node_ids);
    }

    std::vector<Instance> instances;
    for (const auto& mapping : matches) {
        Point offset = computeOffset(flat, mapping, pattern);
        instances.push_back({&cell, offset.x, offset.y});
    }

    // Step 5: Remove matched nodes from flat graph to avoid duplication
    for (const auto& mapping : matches) {
        std::unordered_set<int> ids_to_remove;
        for (const auto& kv : mapping) ids_to_remove.insert(kv.second);
        removeNodes(flat, ids_to_remove);
    }

    // Step 6: Build top-level cell
    Cell top_cell;
    top_cell.name = "TOP";
    top_cell.graph = flat;

    // Step 7: Output hierarchy info
    std::cout << "\nCell definition: " << cell.name << "\n";
    for (const auto& n : cell.graph.nodes) {
        std::cout << " Node " << n.id << " label=" << n.label << "\n";
    }
    for (const auto& e : cell.graph.edges) {
        std::cout << " Edge " << e.from << "->" << e.to << "\n";
    }

    std::cout << "\nInstances:\n";
    for (size_t i = 0; i < instances.size(); ++i) {
        std::cout << " Instance " << i << " of " << instances[i].cell->name
                  << " at offset (" << instances[i].x_offset << ", " << instances[i].y_offset << ")\n";
    }

    std::cout << "\nTop cell has " << top_cell.graph.nodes.size() << " unique polygons remaining.\n";

    return 0;
}

