#ifndef DATA_TYPES
#define DATA_TYPES

typedef struct Edge {
	short from;
	short to;
	int length;
} Edge;

typedef struct Vertex {
	struct Vertex* leader;
} Vertex;

typedef struct InitializationResult {
	char statusCode;
	short verticesN;
	Vertex* vertices;
	int edgesN;
	Edge* edges;
} InitializationResult;

#endif