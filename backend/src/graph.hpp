#pragma once
#include <vector>
#include <string>
#include <map>
#include <limits>

using namespace std;

const int INF= numeric_limits<int>::max();

//Node= a location on campus
struct Node{
    int id;
    string name;
    double x,y; //coordinates for the location
    string type; //"building","parking","food","leisure"

    Node(): id(0), x(0), y(0) {}
    Node(int id, string name, double x, double y, string type="building")
    : id(id), name(name), x(x), y(y), type(type){}
};

//Edge= a path between two locations
struct Edge{
    int from, to;
    int weight; //distance in meters
    string pathType;//"walkway","road","stairs"

    Edge(int f, int t, int w, string pt="walkway")
    :from(f), to(t), weight(w), pathType(pt){}
};

//Graph class
class Graph{
    private:
    vector<Node> nodes;
    vector<vector<int>> adjMatrix;
    vector<Edge> edges;
    map<string, int> nameToId;

    public:
    Graph(int size){
        adjMatrix.resize(size, vector<int>(size,0));
        nodes.reserve(size);
    }

    void addNode(int id, string name, double x, double y, string type="building"){
        nodes.push_back(Node(id, name, x, y, type));
        nameToId[name]= id;
    }

    void addEdge(int from, int to, int weight, string pathType="walkway"){
        if(from >= 0 && from < nodes.size() && to >= 0 && to < nodes.size()){
            adjMatrix[from][to] = weight;
            adjMatrix[to][from] = weight;
            edges.push_back(Edge(from, to, weight, pathType));
        }
    }

    int getNodeId(const string& name) const{
        auto it = nameToId.find(name);
        return (it != nameToId.end()) ? it->second : -1;
    }

    const Node& getNode(int id) const {
        return nodes[id];
    }

    int getWeight(int from, int to) const {
        return adjMatrix[from][to];
    }

    int size() const {
        return nodes.size();
    }

    const vector<Node>& getNodes() const {
        return nodes;
    }

    const vector<Edge>& getEdges() const{
        return edges;
    }

    vector<int> getNeighbors(int nodeId) const{
        vector<int> neighbors;
        for(int i=0; i< nodes.size(); i++){
            if(adjMatrix[nodeId][i]>0){
                neighbors.push_back(i);
            }
        }
        return neighbors;
    }
};

//Campus Graph with predefined data
Graph createCampusGraph(){
    Graph g(10);

    //Add Campus locations
    g.addNode(0, "Administration", 150, 150, "admin");
    g.addNode(1, "Library", 400, 150, "library");
    g.addNode(2, "Main Gate", 650, 150, "entrance");
    g.addNode(3, "Auditorium", 850, 150, "auditorium");
    g.addNode(4, "Cafeteria", 150, 350, "cafeteria");
    g.addNode(5, "Hostel", 400, 350, "hostel");
    g.addNode(6, "Parking Lot", 650, 350, "parking");
    g.addNode(7, "Football Ground", 850, 350, "sports");
    g.addNode(8, "Garden", 500, 550, "leisure");
    g.addNode(9, "Laboratory", 500, 650, "lab"); 

    // Add paths
    g.addEdge(0, 1, 150, "walkway"); // Admin to library
    g.addEdge(1, 2, 200, "walkway"); // Library to Gate
    g.addEdge(2, 3, 180, "walkway"); // Gate to auditorium
    g.addEdge(0, 4, 250, "walkway"); // Admin to Cafeteria
    g.addEdge(1, 5, 220, "walkway"); // Library to Hostel
    g.addEdge(2, 6, 200, "walkway"); // Gate to Parking Lot
    g.addEdge(3, 7, 160, "walkway"); // Auditorium to Ground
    g.addEdge(4, 5, 240, "road"); // Cafeteria to Hostel
    g.addEdge(5, 6, 230, "walkway"); // Hostel to Parking Lot
    g.addEdge(6, 7, 190, "walkway"); // Parking Lot to Ground
    g.addEdge(5, 8, 210, "walkway"); // Hostel to Garden
    g.addEdge(6, 8, 180, "walkway"); // Parking Lot to Garden
    g.addEdge(8, 9, 120, "walkway"); // Garden to Lab
    g.addEdge(7, 9, 400, "road"); // Ground to lab
    
    return g;
}