#ifndef ROPE_H
#define ROPE_H

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "scene/Voxel.h"
#include "Node.h"
#include <BSpline.h>

class Rope {
public:

	int currentX, currentY, currentZ;

	Rope();

	int buffX, buffY, buffZ;			// Number of voxels in the x, y, z directions
	float scaleX, scaleY, scaleZ;		// Size in the x, y, z directions
	float sX, sY, sZ;					// Size of the individual spheres/cubes
	
	Node* root;							// The root node which we will use for transformations and 
	
	glm::mat4 transformation;
	glm::vec3 color;

	void setup();
	void simulate();
	
	// Voxel buffer
	Voxel ****voxelBuffer;

	// Stuff for thinning
	vector<Voxel*> connected_component;
	vector<Voxel*> block;
	vector<Voxel*> getNeighbors(Voxel* v);

	// XY plane operations
	bool thinXY();
	bool pushXY();

	// XZ plane operations
	bool thinXZ();
	bool pushXZ();

	// YZ plane operations
	bool thinYZ();
	bool pushYZ();
	
	// Thin, push and check component in one plane
	bool thin(int third);
	bool push(int third);
	bool pushDir(int, int);

	void fillCC(Voxel*, int);
	bool isMember(Voxel*);
	int endpoints;

	bool checkComponent(int, int);

	vector<Voxel*> getPlanarNeighbors(Voxel* v, int plane);

	// Has the rope been thinned?
	bool thinnedXY;
	bool thinnedXZ;
	bool thinnedYZ;
	bool allThinned;

	// Have we pushed in a plane?
	bool pushedXY;
	bool pushedXZ;
	bool pushedYZ;

	// Thin in all planes
	bool thinAll();
	bool changedThinning;

	bool changing;

	// Double the resolution (in every direction)
	void doubleResolution();

	int counter;

	// Are there any voxels with 0 or 1 neighbor?
	bool checkExternal();


    void printVoxels();
    vector<glm::vec3> bSplineCurver();
    void getCurve(vector<glm::vec3> inputPoints);
    vector<glm::vec3> controlPoints;
    vector<glm::vec4> interpPoints;

};

#endif
