#pragma once
#include "graph.hpp"
#include "../lib/json.hpp"
#include <queue>
#include <vector>
#include <algorithm>

using json = nlohmann::json;
using namespace std;

// Step structure for visualization
struct DijkstraStep {
    int stepNum;
    int currentNode;
    string action;
    string explanation;
    vector<bool> visited;
    vector<int> distances;
    vector<int> previous;
    vector<int> currentQueue;  // For visualization
    
    DijkstraStep() : stepNum(0), currentNode(-1) {}
    
    json toJSON(const Graph& g) const {
        json j;
        j["step"] = stepNum;
        j["node"] = currentNode;
        j["action"] = action;
        j["explanation"] = explanation;
        j["visited"] = visited;
        
        // Convert distances (INF to -1 for JSON)
        vector<int> distCopy = distances;
        for (auto& d : distCopy) {
            if (d == INF) d = -1;
        }
        j["distances"] = distCopy;
        j["previous"] = previous;
        j["queue"] = currentQueue;
        
        return j;
    }
};

class DijkstraVisualizer {
private:
    const Graph& graph;
    vector<DijkstraStep> steps;
    int stepNum;
    
    void recordStep(int currentNode, const string& action, const string& explanation,
                   const vector<bool>& visited, const vector<int>& distances,
                   const vector<int>& previous, const vector<int>& queue) {
        DijkstraStep step;
        step.stepNum = stepNum++;
        step.currentNode = currentNode;
        step.action = action;
        step.explanation = explanation;
        step.visited = visited;
        step.distances = distances;
        step.previous = previous;
        step.currentQueue = queue;
        steps.push_back(step);
    }
    
    
public:
    DijkstraVisualizer(const Graph& g) : graph(g), stepNum(0) {}
    
    json findPath(int start, int end) {
        int n = graph.size();
        vector<int> dist(n, INF);
        vector<bool> visited(n, false);
        vector<int> previous(n, -1);
        
        // Priority queue: pair<distance, node>
        priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;
        
        dist[start] = 0;
        pq.push({0, start});
        
        // Initial step
        vector<int> queueViz = {start};
        recordStep(start, 
                  "Starting at " + graph.getNode(start).name,
                  "Initialize distance to start node as 0, all others as infinity. "
                  "Add start node to priority queue.",
                  visited, dist, previous, queueViz);
        

        
        while (!pq.empty()) {
            int u = pq.top().second;
            int currentDist = pq.top().first;
            pq.pop();
            
            if (visited[u]) continue;
            
            visited[u] = true;
            
            // Build current queue visualization
            priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> tempPQ = pq;
            queueViz.clear();
            while (!tempPQ.empty()) {
                queueViz.push_back(tempPQ.top().second);
                tempPQ.pop();
            }
            
            // Record visit step
            string action = "Visiting " + graph.getNode(u).name;
            string explanation = "Selected " + graph.getNode(u).name + 
                               " as it has the minimum distance (" + to_string(dist[u]) + 
                               "m) among unvisited nodes. Mark it as visited.";
            recordStep(u, action, explanation, visited, dist, previous, queueViz);
            
            // Relax edges
            for (int v = 0; v < n; v++) {
                int weight = graph.getWeight(u, v);
                if (weight > 0 && !visited[v]) {
                    int newDist = dist[u] + weight;
                    
                    if (newDist < dist[v]) {
                        dist[v] = newDist;
                        previous[v] = u;
                        pq.push({newDist, v});
                        
                        // Record relaxation step
                        action = "Relaxing edge to " + graph.getNode(v).name;
                        explanation = "Found shorter path to " + graph.getNode(v).name + 
                                    " via " + graph.getNode(u).name + ". " +
                                    "Updated distance: " + to_string(dist[v]) + "m " +
                                    "(previous: " + to_string(currentDist + weight) + "m).";
                        
                        tempPQ = pq;
                        queueViz.clear();
                        while (!tempPQ.empty()) {
                            queueViz.push_back(tempPQ.top().second);
                            tempPQ.pop();
                        }
                        
                        recordStep(u, action, explanation, visited, dist, previous, queueViz);
                    }
                }
            }
            
            // If we reached the destination, we can stop
            if (u == end) {
                recordStep(end,
                          "Reached destination: " + graph.getNode(end).name,
                          "Found shortest path! Total distance: " + to_string(dist[end]) + "m",
                          visited, dist, previous, queueViz);
                break;
            }
        }
        
        // Reconstruct path
        vector<int> path;
        if (dist[end] != INF) {
            for (int v = end; v != -1; v = previous[v]) {
                path.push_back(v);
            }
            reverse(path.begin(), path.end());
        }
        
        // Build JSON response
        json result;
        result["algorithm"] = "dijkstra";
        result["start"] = start;
        result["end"] = end;
        result["startName"] = graph.getNode(start).name;
        result["endName"] = graph.getNode(end).name;
        result["distance"] = (dist[end] == INF) ? -1 : dist[end];
        result["path"] = path;
        result["steps"] = json::array();
        
        for (const auto& step : steps) {
            result["steps"].push_back(step.toJSON(graph));
        }
        
        // Add complexity info
        result["complexity"] = {
            {"time", "O((V + E) log V)"},
            {"space", "O(V)"},
            {"description", "Using min-heap priority queue"}
        };
        
        return result;
    }
};

// Main API function
json getDijkstraPath(const Graph& g, int start, int end) {
    DijkstraVisualizer viz(g);
    return viz.findPath(start, end);
}