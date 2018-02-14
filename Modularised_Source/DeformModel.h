#ifndef DEFORMMODEL_H
#define DEFORMMODEL_H

#include "chai3d.h"
#include "GEL3D.h"
#include <fstream>

using namespace chai3d;
using namespace std;

class DeformModel{
public:
	DeformModel();
	DeformModel(cWorld* world, string path);
	void BuildDynamicModel(string path);
	void initSkeleton();
	cVector3d perforate(cVector3d pos, double deviceRadius, double stiffness, double time);
	void attachToWorld(cWorld* world);
	void removeFromWorld(cWorld* world);
	void translate(float dx, float dy, float dz);
	void updateSkins(bool value);
	void renderSpheres(cWorld* world);

private:
	cGELWorld* defWorld;
	cGELMesh* m_meshThirdVentricles;
	cNormalMapPtr normalMap_ThirdVentricle;
	double radius;
	vector<cGELSkeletonNode*> dynamicNodes;

	// compute forces between tool and environment
	cVector3d computeForce(const cVector3d& a_cursor,
                       double a_cursorRadius,
                       const cVector3d& a_spherePos,
                       double a_radius,
                       double a_stiffness);

};

#endif
