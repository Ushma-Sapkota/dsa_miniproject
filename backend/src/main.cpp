#include <iostream>
#include <string>
#include <cstring>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <sstream>

#include "graph.hpp"
#include "dijkstra.hpp"
#include "search.hpp"
#include "sort.hpp"
#include "utils.hpp"
#include "../lib/json.hpp"

using namespace std;
using json = nlohmann::json;

// Global campus graph
Graph campusGraph = createCampusGraph();

// Send HTTP response
void sendResponse(int clientSocket, const string& content, const string& contentType = "application/json") {
    ostringstream response;
    response << "HTTP/1.1 200 OK\r\n";
    response << "Content-Type: " << contentType << "\r\n";
    response << "Access-Control-Allow-Origin: *\r\n";
    response << "Content-Length: " << content.length() << "\r\n";
    response << "Connection: close\r\n";
    response << "\r\n";
    response << content;
    
    string responseStr = response.str();
    send(clientSocket, responseStr.c_str(), static_cast<int>(responseStr.length()), 0);
}

// Send 404 error
void send404(int clientSocket) {
    string content = "{\"error\": \"Endpoint not found\"}";
    ostringstream response;
    response << "HTTP/1.1 404 Not Found\r\n";
    response << "Content-Type: application/json\r\n";
    response << "Access-Control-Allow-Origin: *\r\n";
    response << "Content-Length: " << content.length() << "\r\n";
    response << "\r\n";
    response << content;
    
    string responseStr = response.str();
    send(clientSocket, responseStr.c_str(), static_cast<int>(responseStr.length()), 0);
}

// Handle API requests
void handleRequest(int clientSocket, const string& request) {
    string path = extractPath(request);
    string queryString = extractQueryString(request);
    map<string, string> params = parseQueryParams(queryString);
    
    cout << "Request: " << path << endl;
    
    try {
        // GET /api/graph - Return campus graph data
        if (path == "/api/graph") {
            json graphData;
            graphData["nodes"] = json::array();
            graphData["edges"] = json::array();
            
            for (const auto& node : campusGraph.getNodes()) {
                graphData["nodes"].push_back({
                    {"id", node.id},
                    {"name", node.name},
                    {"x", node.x},
                    {"y", node.y},
                    {"type", node.type}
                });
            }
            
            for (const auto& edge : campusGraph.getEdges()) {
                graphData["edges"].push_back({
                    {"from", edge.from},
                    {"to", edge.to},
                    {"weight", edge.weight},
                    {"type", edge.pathType}
                });
            }
            
            sendResponse(clientSocket, graphData.dump());
        }
        
        // GET /api/dijkstra?start=0&end=9
        else if (path == "/api/dijkstra") {
            int start = stoi(params["start"]);
            int end = stoi(params["end"]);
            
            json result = getDijkstraPath(campusGraph, start, end);
            sendResponse(clientSocket, result.dump());
        }
        
        // GET /api/search?query=Library
        else if (path == "/api/search") {
            string query = params["query"];
            
            json result = searchBuilding(campusGraph, query);
            sendResponse(clientSocket, result.dump());
        }
        
        // GET /api/sort?reference=0
        else if (path == "/api/sort") {
            int reference = stoi(params["reference"]);
            
            json result = sortLocationsByDistance(campusGraph, reference);
            sendResponse(clientSocket, result.dump());
        }
        
        // Unknown endpoint
        else {
            send404(clientSocket);
        }
    }
    catch (const exception& e) {
        json error;
        error["error"] = e.what();
        sendResponse(clientSocket, error.dump());
    }
}

int main() {
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed" << endl;
        WSACleanup();
        return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        cerr << "Error creating socket: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }
    
    // Allow socket reuse
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
    
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8080);
    
    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Error binding socket: " << WSAGetLastError() << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    
    if (listen(serverSocket, 10) == SOCKET_ERROR) {
        cerr << "Error listening: " << WSAGetLastError() << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    
    cout << "========================================" << endl;
    cout << "Campus Navigator Server" << endl;
    cout << "========================================" << endl;
    cout << "Server running on http://localhost:8080" << endl;
    cout << "Endpoints:" << endl;
    cout << "  GET /api/graph" << endl;
    cout << "  GET /api/dijkstra?start=0&end=9" << endl;
    cout << "  GET /api/search?query=Library" << endl;
    cout << "  GET /api/sort?reference=0" << endl;
    cout << "========================================" << endl;
    
    while (true) {
        sockaddr_in clientAddr;
        int clientLen = sizeof(clientAddr);
        
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientLen);
        
        if (clientSocket == INVALID_SOCKET) {
            cerr << "Error accepting connection: " << WSAGetLastError() << endl;
            continue;
        }
        
        char buffer[4096] = {0};
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        
        if (bytesRead > 0) {
            string request(buffer);
            handleRequest(clientSocket, request);
        }
        
        closesocket(clientSocket);
    }
    
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}