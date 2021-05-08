#include "Rope.h"
#include <QFile>
#include <QString>
#include <QTextStream>
#include <QDir>


static const float PI = 3.141592653589f;

static void printVector(float x, float y, float z, bool endLine) {
	
	cout << x << ", " << y << ", " << z;
	if (endLine) {
		cout << endl;
	}
}


Rope::Rope() {

	vector<Voxel*> connected_component;
	vector<Voxel*> block;
	root = new Node();
	
	thinnedXY = false;
	thinnedXZ = false;
	thinnedYZ = false;
	allThinned = false;

	currentX = 0;
	currentY = 0;
	currentZ = 0;

	pushedXY = false;
	pushedXZ = false;
	pushedYZ = false;

	changing = false;

    counter = 0;

    controlPoints = vector<glm::vec3>();
    interpPoints = vector<glm::vec4>();
}

void Rope::printVoxels() {
    int count = 0;
    for (int i = 0; i < buffX; i++) {
        for (int j = 0; j < buffY; j++) {
            for (int k = 0; k < buffZ; k++) {
                Voxel* v = voxelBuffer[i][j][k];
                if (v->density == 1) {
                    cout << "[" << i << "], " << "[" << j << "], " << "[" << k << "]" << endl;
                    cout << "Position: " << "(" << v->position.x << ", " << v->position.y << ", " << v->position.z << ")" << endl;
                    count++;
                }
            }
        }
    }
    cout << "Count: " << count << endl;
}

vector<glm::vec3> Rope::bSplineCurver() {
    vector<glm::vec4> controlPoints;
    Voxel* firstVoxel = nullptr;

    Voxel* currentVoxel = nullptr;
    Voxel* prevVoxel = nullptr;

    // get a filled voxel
    for (int i = 0; i < buffX; i++) {
        for (int j = 0; j < buffY; j++) {
            for (int k = 0; k < buffZ; k++) {
                Voxel* v = voxelBuffer[i][j][k];
                if (v->density == 1) {
                    currentVoxel = v;
                    firstVoxel = v;
                }
            }
        }
    }

    bool firstTime = true;


    while (currentVoxel != firstVoxel || firstTime) {
        vector<Voxel*> neighbors = getNeighbors(currentVoxel);
        if (neighbors.size() > 2) {
            std::cout << "NEIGHBORS MORE THAN 2: #" << neighbors.size() << std::endl;
        }
        Voxel* vNext = nullptr;
        // what if there are more than 2 neigbors?
        for (Voxel* v : neighbors) {
            if (v != prevVoxel) {
                vNext = v;
            }
        }

        glm::vec4 centerPoint = (vNext->position + currentVoxel->position) / 2.f;
        controlPoints.push_back(centerPoint);

        if (currentVoxel == firstVoxel) {
            firstTime = false;
        }

        prevVoxel = currentVoxel;
        currentVoxel = vNext;
    }

    vector<glm::vec3> inputPoints;
    for (int i = 0; i < (int) controlPoints.size(); i++) {
        inputPoints.push_back(glm::vec3(controlPoints[i]));
    }

    // add starting point to connect it back together?
    inputPoints.push_back(glm::vec3(controlPoints[0]));

    return inputPoints;
}

void Rope::writeOut(vector<glm::vec3> inputPoints) {
    /* Writing to File */
    QString file = "../../../../qt-knots/controlPoints.txt";
    QFile outputFile(file);
    outputFile.open(QIODevice::WriteOnly | QIODevice::Append);

    if(!outputFile.isOpen())
    {
        //alert that file did not open
        std::cout << "FILE WRITING ERROR" << std::endl;
    }
    QTextStream outStream(&outputFile);
    QString str = "Knot Step \n";
    outStream << str;
    for (glm::vec3 p : inputPoints) {
        QString x = QString::number(p.x);
        QString y = QString::number(p.y);
        QString z = QString::number(p.z);
        QString point = x + " " + y + " " + z + "\n";
        outStream << point;
    }
    outputFile.close();

    this->controlPoints = inputPoints;
}


void Rope::getCurve(vector<glm::vec3> inputPoints) {

    /* De Boor's Methodology: */
    BSpline* curve = new BSpline();
    int degree = 3;
    std::vector<int> knots = {};
    vector<glm::vec4> interpNodes;

    for(double t = 0; t < 1; t += 0.001) {
        glm::vec3 tick = curve->deBoor(t, degree, inputPoints, knots);
        interpNodes.push_back(glm::vec4(tick, 1.f));
        // cout << "[" << tick.x << "], " << "[" << tick.y << "], " << "[" << tick.z << "]" << endl;
    }
    this->interpPoints = interpNodes;

    delete curve;

    /* this->interpPoints = controlPoints;
    vector<glm::vec4> interpNodes;

    for (int i = 0; i < (int) controlPoints.size(); i = i + 3) {
        Curve* curve = new BSpline();
        curve->set_steps(100); // generate 100 interpolate points between the last 4 way points
        int p1 = i;
        int p2 = (i + 1) % controlPoints.size();
        int p3 = (i + 2) % controlPoints.size();
        int p4 = (i + 3) % controlPoints.size();

        if (i == (int) controlPoints.size() - 2) {

        }

        curve->add_way_point(Vector(controlPoints[p1].x, controlPoints[p1].y, controlPoints[p1].z));
        curve->add_way_point(Vector(controlPoints[p2].x, controlPoints[p2].y, controlPoints[p2].z));
        curve->add_way_point(Vector(controlPoints[p3].x, controlPoints[p3].y, controlPoints[p3].z));
        curve->add_way_point(Vector(controlPoints[p4].x, controlPoints[p4].y, controlPoints[p4].z));


        for (int i = 0; i < curve->node_count(); ++i) {
            // std::cout << "node #" << i << ": " << curve->node(i).toString() << " (length so far: " << curve->length_from_starting_point(i) << ")" << std::endl;
            glm::vec4 node = glm::vec4(curve->node(i).x, curve->node(i).y, curve->node(i).z, 1.f);
            interpNodes.push_back(node);
        }
    }
    this->controlPoints = interpNodes;*/
}

// Simulate a timestep of the rope
void Rope::simulate() {
	
	counter++;
	cout << "Counter: " << counter << endl;

	// Keep thinning until we can't anymore
	while (!allThinned) {
		allThinned = thinAll();
	}

	int count = 0;
	for (int i = 0; i < buffX; i++) {
		for (int j = 0; j < buffY; j++) {
			for (int k = 0; k < buffZ; k++) {
				Voxel* v = voxelBuffer[i][j][k];
				if (v->density == 1) {
					count++;
				}
				if (count > 8) {
					break;
				}
			}
		}
	}
	if (count == 8) {
		return;
	}

	if (!pushedXY) {

		////cout << "	We are on z = " << currentZ << endl;
		//bool blah = pushXY(currentZ);

		//while (!blah && currentZ <= buffZ - 1) {
		//	blah = pushXY(currentZ);
		//	currentZ++;
		//}
		//if (currentZ > (buffZ - 1)) {
		//	currentZ = buffZ - 1;
		//	pushedXY = true;
		//	cout << "Done with the xy pushes" << endl;

		//}


		bool blah = pushXY();
		if (!blah) {
			pushedXY = true;
		}


	}

	if (pushedXY && !pushedYZ) {

	//	////cout << "	We are on x = " << currentX << endl;
	//	//bool blah = pushYZ(currentX);
	//	//while (!blah && currentX <= buffX - 1) {
	//	//	blah = pushYZ(currentX);
	//	//	currentX++;
	//	//}
	//	//if (currentX > (buffX - 1)) {
	//	//	currentX = buffX - 1;
	//	//	pushedYZ = true;
	//	//	cout << "Done with the yz pushes" << endl;

	//	//}

		bool blah = pushYZ();
		if (!blah) {
			pushedYZ = true;
		}
	}

	if (pushedXY && pushedYZ && !pushedXZ) {

	//	////cout << "	We are on y = " << currentY << endl;
	//	//bool blah = pushXZ(currentY);
	//	//while (!blah && currentY <= buffY - 1) {
	//	//	blah = pushXZ(currentY);
	//	//	currentY++;
	//	//}
	//	//if (currentY > (buffY - 1)) {
	//	//	currentY = buffY - 1;
	//	//	pushedXZ = true;
	//	//	cout << "Done with the xz pushes" << endl;

	//	//}

		bool blah = pushXZ();
		if (!blah) {
			pushedXZ = true;
		}
	}

	if (pushedXY && pushedXZ && pushedYZ) {
		
		if (changing) {
		
			pushedXY = false;
			pushedXZ = false;
			pushedYZ = false;

			changing = false;

        // If it's not changing anymore, double the resolution
		} else {

			cout << "Time to double the resolution!" << endl;
			doubleResolution();
			changing = true;

		}
    }

    // generation of curve TODO:: do this when not changing = false or something?
    /*vector<glm::vec3> newControlPoints = bSplineCurver();
    std::cout << "Number of Points: " << newControlPoints.size() << std::endl;

    vector<glm::vec3> differentPoints;
    for (glm::vec3 x : this->controlPoints) {
        if(std::find(newControlPoints.begin(), newControlPoints.end(), x) != newControlPoints.end()) {
            // contains x

        } else {
            // does not contain x
            differentPoints.push_back(x);
        }
    }

    //std::cout << "Different Points: " << differentPoints.size() << std::endl;
    for (glm::vec3 p : differentPoints) {
        //std::cout << "Old Point: ";
        //printVector(p.x, p.y, p.z, true);

        glm::vec3 closest = newControlPoints[0];
        glm::vec3 secondClosest = newControlPoints[1];
        // get the 2 points in newControlPoints that is closest to each old point that has disappeared
        for (int i = 2; i < (int) newControlPoints.size(); i++) {
            glm::vec3 newP = newControlPoints[i];
            if (glm::distance(p, newP) < glm::distance(p, closest)) {
                secondClosest = closest;
                closest = newP;
            }
            else if (glm::distance(p, newP) < glm::distance(p, secondClosest)) {
                secondClosest = newP;
            }
        }
    }

    getCurve(newControlPoints);

    this->controlPoints = newControlPoints;*/
}

// ******************* XY Plane ********************** //

bool Rope::thinXY() {

	return thin(2);

}

bool Rope::pushXY() {
	return push(2);
}

// ******************* XZ Plane ********************** //

bool Rope::thinXZ() {

	return thin(1);
}

bool Rope::pushXZ() {
	return push(1);
}

// ******************* YZ Plane ********************** //
bool Rope::thinYZ() {

	return thin(0);

}

bool Rope::pushYZ() {
	return push(0);
}

// ******************* Generalized operations ********************** //

bool Rope::checkExternal() {

	for (int x = 0; x < buffX; x++) {
		for (int y = 0; y < buffY; y++) {
			for (int z = 0; z < buffZ; z++) {

				Voxel* thisVoxel = voxelBuffer[x][y][z];
				
				if (thisVoxel->density == 0) {
					continue;
				}

				vector<Voxel*> neighbors = getNeighbors(thisVoxel);

				if (neighbors.size() < 2) {
					thisVoxel->clearVoxel();
					return true;
				}
			
			}
		}
	}

	return false;

}

bool Rope::thin(int third) {

	int blockCounter = 0;

	int axis1, axis2;
	int dim1, dim2, dim3;
	
	// Set the axes
	if (third == 2) {
		axis1 = 0;
		axis2 = 1;
		dim1 = buffX;
		dim2 = buffY;
		dim3 = buffZ;
	} else if (third == 1) {
		axis1 = 0;
		axis2 = 2;
		dim1 = buffX;
		dim2 = buffZ;
		dim3 = buffY;
	} else {
		axis1 = 1;
		axis2 = 2;
		dim1 = buffY;
		dim2 = buffZ;
		dim3 = buffX;
	}

	Voxel* v1;
	Voxel* v2;
	Voxel* v3;
	Voxel* v4;

	for (int j = 0; j < dim2 - 1; j++) {
		for (int i = 0; i < dim1 - 1; i++) {
			for (int thirdValue = 0; thirdValue < dim3; thirdValue++) {

				//  [ ][  ][  ][ ][ ][ ][ ]
				//  [ ][v1][v2][ ][ ][ ][ ] 
				//  [ ][v3][v4][ ][ ][ ][ ]
				//  [ ][  ][  ][ ][ ][ ][ ]

				if (third == 2) {
				
					v1 = voxelBuffer[i][j][thirdValue];
					v2 = voxelBuffer[i + 1][j][thirdValue];
					v3 = voxelBuffer[i][j + 1][thirdValue];
					v4 = voxelBuffer[i + 1][j + 1][thirdValue];

				} else if (third == 1) {
				
					v1 = voxelBuffer[i][thirdValue][j];
					v2 = voxelBuffer[i + 1][thirdValue][j];
					v3 = voxelBuffer[i][thirdValue][j + 1];
					v4 = voxelBuffer[i + 1][thirdValue][j + 1];

			
				} else {

					v1 = voxelBuffer[thirdValue][i][j];
					v2 = voxelBuffer[thirdValue][i + 1][j];
					v3 = voxelBuffer[thirdValue][i][j + 1];
					v4 = voxelBuffer[thirdValue][i + 1][j + 1];

				}

				// Initialize everything in the block to be false
				v1->block = false;
				v2->block = false;
				v3->block = false;
				v4->block = false;

				// Continue if the block is not a 2x2 block
				if (v1->density != 1 || v2->density != 1 || v3->density != 1 || v4->density != 1) {
					continue;
				}

				v1->block = true;
				v2->block = true;
				v3->block = true;
				v4->block = true;

				// Potential block
				vector<Voxel*> component; 

				component.push_back(v1);
				component.push_back(v2);
				component.push_back(v3);
				component.push_back(v4);
					
				Voxel* neighbor;

				for (int i = 0; i < 4; i++) {

					Voxel* thisVoxel = component[i];

					int vX = thisVoxel->x;
					int vY = thisVoxel->y;
					int vZ = thisVoxel->z;
					
					// Neighbors not in the block
					int neighbors = 0;

					//  [  ][n2][  ][ ][ ][ ][ ]
					//  [n1][x ][n3][ ][ ][ ][ ] 
					//  [  ][n4][  ][ ][ ][ ][ ]
					//  [  ][  ][  ][ ][ ][ ][ ]

					Voxel* neighbor;

					// Left neighbor
					if (vX > 0) {
						neighbor = voxelBuffer[vX - 1][vY][vZ];
						if (neighbor->density == 1 && neighbor->block == false) {
							neighbors++;
						}
					}

					// Top neighbor
					if (vY > 0) {
						neighbor = voxelBuffer[vX][vY - 1][vZ];
						if (neighbor->density == 1 && neighbor->block == false) {
							neighbors++;
						}
					}

					// Right neighbor
					if (vX < buffX - 1) {
						neighbor = voxelBuffer[vX + 1][vY][vZ];
						if (neighbor->density == 1 && neighbor->block == false) {
							neighbors++;
						}
					}

					// Bottom neighbor
					if (vY < buffY - 1) {
						neighbor = voxelBuffer[vX][vY + 1][vZ];
						if (neighbor->density == 1 && neighbor->block == false) {
							neighbors++;
						}
					}

					// Back neighbor
					if (vZ > 0) {
						neighbor = voxelBuffer[vX][vY][vZ - 1];
						if (neighbor->density == 1 && neighbor->block == false) {
							neighbors++;
						}
					}

					// Forward neighbor
					if (vZ < buffZ - 1) {
						neighbor = voxelBuffer[vX][vY][vZ + 1];
						if (neighbor->density == 1 && neighbor->block == false) {
							neighbors++;
						}
					}

					// If the voxel has no neighbors outside of its block
					if (neighbors == 0) {
						thisVoxel->clearVoxel();
						return true;
					}
					

				} // end for the block

			}// end for thirdValue

		} // end for j

	} // end for i

	// We make it down here if there are no 2x2 blocks left to break up
	return false;

}

bool Rope::pushDir(int sign, int third) {

	bool externalsExist = checkExternal();
	while (externalsExist == true) {
		externalsExist = checkExternal();
	}

	int dim1; // axis 1 of the plane we're looking at
	int dim2; // axis 2 of the plane we're looking at
	int dim3; // axis along which we are moving
	Voxel* thisVoxel;

	// Plane is the XY plane
	if (third == 2) {

		dim1 = buffX;
		dim2 = buffY;
		dim3 = buffZ;

	// In the XZ plane
	} else if (third == 1) {

		dim1 = buffX;
		dim2 = buffZ;
		dim3 = buffY;

	// In the YZ plane
	} else {

		dim1 = buffY;
		dim2 = buffZ;
		dim3 = buffX;

	}

	if (sign > 0) {


		for (int i = 0; i < dim3 - 2; i++) {			
			for (int j = 0; j < dim1; j++) {
				for (int k = 0; k < dim2; k++) {

					if (third == 2) {
						thisVoxel = voxelBuffer[j][k][i];
					} else if (third == 1) {
						thisVoxel = voxelBuffer[j][i][k];
					} else {
						thisVoxel = voxelBuffer[i][j][k];
					}

					connected_component.clear();

					if (thisVoxel->density == 1) {
						
						endpoints = 0;
						fillCC(thisVoxel, third);

						int componentSize = connected_component.size();

						bool change = checkComponent(sign, third);
						if (change) {

                            // generation of curve TODO:: do this when not changing = false or something?
                            vector<glm::vec3> newControlPoints = bSplineCurver();
                            if (newControlPoints.size() != this->controlPoints.size()) {
                                std::cout << "Number of Points: " << newControlPoints.size() << std::endl;
                                writeOut(newControlPoints);
                            }

							return true;
						}

					}
				}
			}
		}

	} else {

		for (int i = dim3 - 1; i > 1; i--) {

			connected_component.clear();

			for (int j = 0; j < dim1; j++) {
				for (int k = 0; k < dim2; k++) {
				
					if (third == 2) {
						thisVoxel = voxelBuffer[j][k][i];
					} else if (third == 1) {
						thisVoxel = voxelBuffer[j][i][k];
					} else {
						thisVoxel = voxelBuffer[i][j][k];
					}

					connected_component.clear();

					if (thisVoxel->density == 1) {
						
						endpoints = 0;
						fillCC(thisVoxel, third);

						bool change = checkComponent(sign, third);
						if (change) {

                            // generation of curve TODO:: do this when not changing = false or something?
                            vector<glm::vec3> newControlPoints = bSplineCurver();
                            if (newControlPoints.size() != this->controlPoints.size()) {
                                std::cout << "Number of Points: " << newControlPoints.size() << std::endl;
                                writeOut(newControlPoints);
                            }

							return true;
						}

					}
				}
			}

		}

	} // end if sign is negative

	return false;

}

void Rope::fillCC(Voxel* v, int plane) {

	if (endpoints == 2) {
		return;
	}

	vector<Voxel*> neighbors = getPlanarNeighbors(v, plane);

	// If v is an endpoint
	if (neighbors.size() == 1) {

		if (endpoints == 0) {
			connected_component.insert(connected_component.begin(), v);
			//cout << "Adding an endpoint: (" << v->x << ", " << v->y << ", " << v->z << ")" << endl;
		} else {
			if (!isMember(v)) {
				connected_component.push_back(v);
				//cout << "Adding an endpoint: (" << v->x << ", " << v->y << ", " << v->z << ")" << endl;
			}
		}
		endpoints++;
		//cout << "	And now we've found " << endpoints << " endpoints of the thing" << endl;
	}

	for (int i = 0; i < neighbors.size(); i++) {
			
		Voxel* thisNeighbor = neighbors[i];

		if (!isMember(thisNeighbor)) {
			connected_component.push_back(thisNeighbor);
			//cout << "Just added (" << thisNeighbor->x << ", " << thisNeighbor->y << ", " << thisNeighbor->z << ")" << endl;
			fillCC(thisNeighbor, plane);
		}

	}


}

// Is the voxel already a member of the cc?
bool Rope::isMember(Voxel *v) {

	for (int i = 0; i < connected_component.size(); i++) {
		Voxel* thisVoxel = connected_component[i];
		if (v->my_equals(thisVoxel)) {
			return true;
		}
	}
	return false;

}

bool Rope::checkComponent(int sign, int axis) {

	if (connected_component.size() < 3) {
		return false;
	}

	Voxel* first = connected_component.at(0);
	Voxel* last  = connected_component.at(connected_component.size() - 1);

	//first->setColor(1, 0, 1);
	//last->setColor(1, 0, 1);

	// If we're going in the x direction
	if (axis == 0) {

		// Check the edges
		if (voxelBuffer[first->x + (sign)][first->y][first->z]->density == 0) {
			return false;
		} 
		if (voxelBuffer[last->x + (sign)][last->y][last->z]->density == 0) {
			return false;
		}

		// Check the inside ones
		for (int i = 1; i < connected_component.size() - 1; i++) {
			
			Voxel* thisVoxel = connected_component.at(i);

			if (voxelBuffer[thisVoxel->x + (sign)][thisVoxel->y][thisVoxel->z]->density == 1) {
				return false;
			}
			if (voxelBuffer[thisVoxel->x + (2 * sign)][thisVoxel->y][thisVoxel->z]->density == 1) {
				return false;
			}

			//// Check for edge adjacencies too
            //Voxel* nextOne  = voxelBuffer[thisVoxel->x + (sign)][thisVoxel->y][thisVoxel->z];
            //Voxel* nextNext = voxelBuffer[thisVoxel->x + (2 * sign)][thisVoxel->y][thisVoxel->z];
            //
            //vector<Voxel*> nextOneNeighbors = getNeighbors(nextOne);
            //if (nextOneNeighbors.size() > 2) {
            //	return false;
            //}
            //vector<Voxel*> nextNextNeighbors = getNeighbors(nextNext);
            //if (nextNextNeighbors.size() > 2) {
            //	return false;
            //}

		}

		// If we've made it this far, we can push it
		for (int i = 0; i < connected_component.size(); i++) {

			Voxel* thisVoxel = connected_component.at(i);

			voxelBuffer[thisVoxel->x + sign][thisVoxel->y][thisVoxel->z]->density = 1;
			voxelBuffer[thisVoxel->x + sign][thisVoxel->y][thisVoxel->z]->node->hasGeometry = true;

			thisVoxel->clearVoxel();

			changing = true;

		}

		return true;

	// If we're going in the y direction
	} else if (axis == 1) {

		// Check the edges
		if (voxelBuffer[first->x][first->y + sign][first->z]->density == 0) {
			return false;
		} 
		if (voxelBuffer[last->x][last->y + sign][last->z]->density == 0) {
			return false;
		}

		// Check the inside ones
		for (int i = 1; i < connected_component.size() - 1; i++) {
			
			Voxel* thisVoxel = connected_component.at(i);

			if (voxelBuffer[thisVoxel->x][thisVoxel->y + sign][thisVoxel->z]->density == 1) {
				return false;
			}
			if (voxelBuffer[thisVoxel->x][thisVoxel->y + (2 * sign)][thisVoxel->z]->density == 1) {
				return false;
			}


			//// Check for edge adjacencies too
            //Voxel* nextOne  = voxelBuffer[thisVoxel->x][thisVoxel->y + sign][thisVoxel->z];
            //Voxel* nextNext = voxelBuffer[thisVoxel->x][thisVoxel->y + (2 * sign)][thisVoxel->z];
            //
            //vector<Voxel*> nextOneNeighbors = getNeighbors(nextOne);
            //if (nextOneNeighbors.size() > 2) {
            //	return false;
            //}
            //vector<Voxel*> nextNextNeighbors = getNeighbors(nextNext);
            //if (nextNextNeighbors.size() > 2) {
            //	return false;
            //}
		}

		// If we've made it this far, we can push it
		for (int i = 0; i < connected_component.size(); i++) {

			Voxel* thisVoxel = connected_component.at(i);

			voxelBuffer[thisVoxel->x][thisVoxel->y + sign][thisVoxel->z]->density = 1;
			voxelBuffer[thisVoxel->x][thisVoxel->y + sign][thisVoxel->z]->node->hasGeometry = true;

			thisVoxel->clearVoxel();

			changing = true;

		}

		return true;

	// If we're going in the z direction
	} else if (axis == 2) {

		// Check the edges
		if (voxelBuffer[first->x][first->y][first->z + sign]->density == 0) {
			return false;
		} 
		if (voxelBuffer[last->x][last->y][last->z + sign]->density == 0) {
			return false;
		}

		// Check the inside ones
		for (int i = 1; i < connected_component.size() - 1; i++) {
			
			Voxel* thisVoxel = connected_component.at(i);

			if (voxelBuffer[thisVoxel->x][thisVoxel->y][thisVoxel->z + sign]->density == 1) {
				return false;
			}
			if (voxelBuffer[thisVoxel->x][thisVoxel->y][thisVoxel->z + (sign * 2)]->density == 1) {
				return false;
			}

			//// Check for edge adjacencies too
            //Voxel* nextOne  = voxelBuffer[thisVoxel->x][thisVoxel->y][thisVoxel->z + sign];
            //Voxel* nextNext = voxelBuffer[thisVoxel->x][thisVoxel->y][thisVoxel->z + (sign * 2)];
            //
            //vector<Voxel*> nextOneNeighbors = getNeighbors(nextOne);
            //if (nextOneNeighbors.size() > 2) {
            //	return false;
            //}
            //vector<Voxel*> nextNextNeighbors = getNeighbors(nextNext);
            //if (nextNextNeighbors.size() > 2) {
            //	return false;
            //}


		}


		// If we've made it this far, we can push it
		for (int i = 0; i < connected_component.size(); i++) {

			Voxel* thisVoxel = connected_component.at(i);

			voxelBuffer[thisVoxel->x][thisVoxel->y][thisVoxel->z + sign]->density = 1;
			voxelBuffer[thisVoxel->x][thisVoxel->y][thisVoxel->z + sign]->node->hasGeometry = true;

			thisVoxel->clearVoxel();

			changing = true;

		}


		return true;

	}

}

bool Rope::push(int third) {

	int axis1, axis2;

	if (third == 2) {
		axis1 = 0;
		axis2 = 1;
	} else if (third == 1) {
		axis1 = 0;
		axis2 = 2;
	} else {
		axis1 = 1;
		axis2 = 2;
	}

	bool changing = pushDir(1, axis2);
	while (changing) {
        changing = pushDir(1, axis2);
		return true;
	}

	changing = pushDir(-1, axis1);
	while (changing) {

		changing = pushDir(-1, axis1);
		return true;
	}

	changing = pushDir(-1, axis2);
	while (changing) {
        changing = pushDir(-1, axis2);

		return true;
	}

	changing = pushDir(1, axis1);
	while (changing) {
        changing = pushDir(1, axis1);

		return true;
	}

	return false;


}

void Rope::setup() {

	// Assume it isn't thinned
	thinnedXY = false;
	thinnedXZ = false;
	thinnedYZ = false;

	root->children.clear();
	
	//Sphere* sphere2 = new Sphere();
	//sphere2->geoColor = color;
	//sphere2->refreshColor();
	//root->geometry = sphere2;
	root->hasGeometry = false;

	root->setXTrans(-1.5);
	root->setYTrans(2);
	root->transform();

	// Set up the spheres in the voxel buffer
	for (int z = buffZ - 1; z >= 0; z--) {
	//for (int z = 0; z < buffZ; z++) {
		for (int y = 0; y < buffY; y++) {

			for (int x = buffX - 1; x >= 0; x--) {
			//for (int x = 0; x < buffX; x++) {
			

				Voxel* voxel = voxelBuffer[x][y][z];
				voxel->block = false;

				float vX = sX * (x);
				float vY = sY * -(y);
				float vZ = sZ * (z);

				voxel->position = glm::vec4(vX, vY, vZ, 1.0f);

				Node* cubeNode = new Node();

				//Sphere* cube = new Sphere;
				Cube* cube = new Cube();

				cube->geoColor = color;

				cube->refreshColor();
				cubeNode->geometry = cube;

				cubeNode->hasGeometry = (voxel->density == 1);

				cubeNode->setXScale(sX);
				cubeNode->setYScale(sY);
				cubeNode->setZScale(sZ);

				cubeNode->setXTrans(vX);
				cubeNode->setYTrans(vY);
				cubeNode->setZTrans(vZ);

				cubeNode->transform();

				root->addChild(cubeNode);
				cubeNode->parent = root;

				voxel->node = cubeNode;

			}


		}

	}

}

vector<Voxel*> Rope::getNeighbors(Voxel* v) {

	int vX = v->x;
	int vY = v->y;
	int vZ = v->z;

	vector<Voxel*> neighbors;
	Voxel* neighbor;

	// Left neighbor
	if (vX > 0) {
		neighbor = voxelBuffer[vX - 1][vY][vZ];
		if (neighbor->density == 1) {
			neighbors.push_back(neighbor);
		}
	}

	// Top neighbor
	if (vY > 0) {
		neighbor = voxelBuffer[vX][vY - 1][vZ];
		if (neighbor->density == 1) {
			neighbors.push_back(neighbor);
		}
	}

	// Right neighbor
	if (vX < buffX - 1) {
		neighbor = voxelBuffer[vX + 1][vY][vZ];
		if (neighbor->density == 1) {
			neighbors.push_back(neighbor);
		}
	}

	// Bottom neighbor
	if (vY < buffY - 1) {
		neighbor = voxelBuffer[vX][vY + 1][vZ];
		if (neighbor->density == 1) {
			neighbors.push_back(neighbor);
		}
	}

	// Back neighbor
	if (vZ > 0) {
		neighbor = voxelBuffer[vX][vY][vZ - 1];
		if (neighbor->density == 1) {
			neighbors.push_back(neighbor);
		}
	}

	// Forward neighbor
	if (vZ < buffZ - 1) {
		neighbor = voxelBuffer[vX][vY][vZ + 1];
		if (neighbor->density == 1) {
			neighbors.push_back(neighbor);
		}
	}

	return neighbors;

}

vector<Voxel*> Rope::getPlanarNeighbors(Voxel* v, int plane) {
	
	// plane = 0 --> yz plane
	// plane = 1 --> xz plane
	// plane = 2 --> xy plane

	int vX = v->x;
	int vY = v->y;
	int vZ = v->z;

	vector<Voxel*> neighbors;
	Voxel* neighbor;

	if (plane != 0) {

		// Left neighbor
		if (vX > 0) {
			neighbor = voxelBuffer[vX - 1][vY][vZ];
			if (neighbor->density == 1) {
				neighbors.push_back(neighbor);
			}
		}

		// Right neighbor
		if (vX < buffX - 1) {
			neighbor = voxelBuffer[vX + 1][vY][vZ];
			if (neighbor->density == 1) {
				neighbors.push_back(neighbor);
			}
		}

	}

	if (plane != 1) {

		// Top neighbor
		if (vY > 0) {
			neighbor = voxelBuffer[vX][vY - 1][vZ];
			if (neighbor->density == 1) {
				neighbors.push_back(neighbor);
			}
		}

		// Bottom neighbor
		if (vY < buffY - 1) {
			neighbor = voxelBuffer[vX][vY + 1][vZ];
			if (neighbor->density == 1) {
				neighbors.push_back(neighbor);
			}
		}

	}

	if (plane != 2) {

		// Back neighbor
		if (vZ > 0) {
			neighbor = voxelBuffer[vX][vY][vZ - 1];
			if (neighbor->density == 1) {
				neighbors.push_back(neighbor);
			}
		}

		// Forward neighbor
		if (vZ < buffZ - 1) {
			neighbor = voxelBuffer[vX][vY][vZ + 1];
			if (neighbor->density == 1) {
				neighbors.push_back(neighbor);
			}
		}

	}

	return neighbors;
}

bool Rope::thinAll() {

	// Thin in xy until we can't anymore
	if (!thinnedXY) {
		thinnedXY = !(thinXY());
	}
	
	// If we can thin in XZ, we need to set thinnedXY = false again
	if (!thinnedXZ) {
		bool check = thinXZ();
		if (check) {
			thinnedXY = false;
		} else {
			thinnedXZ = true;
		}
	}

	// If we can thin in YZ, we need to set thinnedXY and thinnedXZ = false again
	if (!thinnedYZ) {
		bool check = thinYZ();
		if (check) {
			thinnedXY = false;
			thinnedXZ = false;
		} else {
			thinnedYZ = true;
		}
	}


	// If we're all thinned, pick off the extra ones
	if (thinnedXY && thinnedXZ && thinnedYZ) {

		// Look for solo voxels *first*
		for (int x = 0; x < buffX; x++) {
			for (int y = 0; y < buffY; y++) {
				for (int z = 0; z < buffZ; z++) {

					Voxel* thisVoxel = voxelBuffer[x][y][z];

					if (thisVoxel->density == 0) {
						continue;
					}

					// Neighbors
					vector<Voxel*> neighbors = getNeighbors(thisVoxel);

					int neighborsCount = neighbors.size();

					if (neighborsCount <= 1) {
						thisVoxel->clearVoxel();
						return false;
					}


				}
			}
		}


		cout << "Finished one pass of pinching all the solos off!" << endl;

		// Make sure everything only has 2 neighbors
		for (int x = 0; x < buffX; x++) {
			for (int y = 0; y < buffY; y++) {
				for (int z = 0; z < buffZ; z++) {

					Voxel* thisVoxel = voxelBuffer[x][y][z];
					int vX = thisVoxel->x;
					int vY = thisVoxel->y;
					int vZ = thisVoxel->z;
				
					if (thisVoxel->density == 0) {
						continue;
					}

					// Neighbors
					vector<Voxel*> neighbors = getNeighbors(thisVoxel);
					int neighborsCount = neighbors.size();

					if (neighborsCount > 2) {

						//thisVoxel->setColor(1, 1, 0);

						for (int i = 0; i < neighbors.size(); i++) {

							Voxel* thisNeighbor = neighbors[i];
							//thisNeighbor->setColor(0, 1, 0);

							vector<Voxel*> neighborNeighbors = getNeighbors(thisNeighbor);

							for (int j = 0; j < neighborNeighbors.size(); j++) {
							
								Voxel* thisNeighborNeighbor = neighborNeighbors[j];

								// If we haven't seen this voxel before, look at its neighbors
								if (!thisNeighborNeighbor->my_equals(thisVoxel)) {

									//thisNeighborNeighbor->setColor(1, 0, 0);

									// If we haven't seen this neighbor before
									vector<Voxel*> thirdNeighbors = getNeighbors(thisNeighborNeighbor);

									if (thirdNeighbors.size() > 2) {
										//cout << "		2nd neighbor has 3 neighbors, so we're deleting 1st neighbor" << endl;
										thisNeighbor->clearVoxel();
										return false;

									}

									for (int k = 0; k < thirdNeighbors.size(); k++) {

										Voxel* third = thirdNeighbors[k];

										if (!third->my_equals(thisNeighbor) && !third->my_equals(thisNeighborNeighbor)) {

											//third->setColor(1, 1, 1);

											vector<Voxel*> fourthNeighbors = getNeighbors(third);

											if (fourthNeighbors.size() > 2) {
												//cout << "		3rd neighbor has 3 neighbors, so we're deleting 1st and 2nd neighbor" << endl;
												thisNeighbor->clearVoxel();
												thisNeighborNeighbor->clearVoxel();
												return false;

											} // end if fourthNeighbors.size > 2

											for (int m = 0; m < fourthNeighbors.size(); m++) {

												Voxel* fourth = fourthNeighbors[m];

												if (!fourth->my_equals(thisVoxel) && !fourth->my_equals(thisNeighbor) && !fourth->my_equals(thisNeighborNeighbor)) {

													//fourth->setColor(0.5, 0, 1);

													vector<Voxel*> fifthNeighbors = getNeighbors(fourth);

													if (fifthNeighbors.size() > 2) {
														//cout << "		4th neighbor has 3 neighbors, so we're deleting 1st, 2nd and third neighbors" << endl;
														thisNeighbor->clearVoxel();
														thisNeighborNeighbor->clearVoxel();
														third->clearVoxel();
														return false;

													} // end if fourthNeighbors.size > 2

												}


											}

										} // end if third != thisNeighbor and third != thisNeighborNeighbor


									} // end for neighbors of thisNeighborNeighbor


								} // end if thisNeighborNeighbor != thisVoxel

							} // end for neighbors of thisNeighbor


						} // end for neighbors of thisVoxel

						//return true;

					} else {
						//thisVoxel->setColor(0.2, 0.6, 1);
					}

				} // end for z
			} // end for y
		} // end for x


	}  else {
		
		// There's still some thinning to do
		return false;
		
	}
	
	// end if all 3 thinned bools are true
	// Make sure everything is 2-connected
		for (int x = 0; x < buffX; x++) {
			for (int y = 0; y < buffY; y++) {
				for (int z = 0; z < buffZ; z++) {

					Voxel* thisVoxel = voxelBuffer[x][y][z];

					if (thisVoxel->density == 0) {
						continue;
					}

					// Neighbors
					vector<Voxel*> neighbors = getNeighbors(thisVoxel);

					int neighborsCount = neighbors.size();

					if (neighborsCount != 2) {
						cout << "### We made it all the way here and there's still something wrong!! ###" << endl;
						cout << "	Dis dude has " << neighborsCount << " neighbors fam" << endl;
						//thisVoxel->setColor(1, 0, 1);
						//return false;
					}

				}
			}
		}

	cout << "All thinned!" << endl;
	return true;

}

// Double the resolution of the buffer
void Rope::doubleResolution() {

	// First we find the min and max extent of the rope in every direction, so we only expand the buffer to we have to
	int minX_extent = INT_MAX;
	int minY_extent = INT_MAX;
	int minZ_extent = INT_MAX;

	int maxX_extent = 0;
	int maxY_extent = 0;
	int maxZ_extent = 0;

	for (int z = 0; z < buffZ; z++) {
		for (int y = 0; y < buffY; y++) {
			for (int x = 0; x < buffX; x++) {
				
				Voxel* thisVoxel = voxelBuffer[x][y][z];

				if (thisVoxel->density == 1) {
				
					minX_extent = min(minX_extent, x);
					minY_extent = min(minY_extent, y);
					minZ_extent = min(minZ_extent, z);

					maxX_extent = max(maxX_extent, x);
					maxY_extent = max(maxY_extent, y);
					maxZ_extent = max(maxZ_extent, z);


				}
					
			}
		}
	}

	int new_buffX = (maxX_extent - minX_extent + 1) * 2;
	int new_buffY = (maxY_extent - minY_extent + 1) * 2;
	int new_buffZ = (maxZ_extent - minZ_extent + 1) * 2;

	Voxel ****newBuffer;
	newBuffer = new Voxel***[new_buffX];
	for (int k = 0; k < new_buffX; k++) {
		newBuffer[k] = new Voxel**[new_buffY];
	}

	for (int k = 0; k < new_buffX; k++) {
		for (int w = 0; w < new_buffY; w++) {
			newBuffer[k][w] = new Voxel*[new_buffZ];
		}
	}

	// Fill in the voxels
	for (int z = 0; z < new_buffZ; z++) {
		for (int y = 0; y < new_buffY; y++) {
			for (int x = 0; x < new_buffX; x++) {
			
				// The corresponding voxel in the original buffer
				int xCoord = minX_extent + (x/2);
				int yCoord = minY_extent + (y/2);
				int zCoord = minZ_extent + (z/2);
				
				Voxel* corresp = voxelBuffer[xCoord][yCoord][zCoord];
				Voxel* voxel = new Voxel(corresp->density);
				voxel->x = x;
				voxel->y = y;
				voxel->z = z;

				newBuffer[x][y][z] = voxel;

			}

		}
	}


	for (int z = 0; z < buffZ; z++) {
		for (int y = 0; y < buffY; y++) {
			for (int x = 0; x < buffX; x++) {
				delete voxelBuffer[x][y][z];	
			}
		}
	}

	// Swap the buffers & run setup
	voxelBuffer = newBuffer;

	buffX = new_buffX;
	buffY = new_buffY;
	buffZ = new_buffZ;

	setup();
	thinnedXY = false;
	thinnedXZ = false;
	thinnedYZ = false;
	allThinned = false;
}
