#ifndef VENTRICULAR_H
#define VENTRICULAR_H

#include "chai3d.h"
#include <fstream>

using namespace chai3d;
using namespace std;

class VentricularStructures{
public:
	VentricularStructures();
	VentricularStructures(string ventPath, string ventNorm, string tVein, string cPlexis, cWorld* world);
	void setShader(cShaderProgramPtr programShader);
	void translate(float dx, float dy, float dz);
	
private:
	cMultiMesh* m_meshVentricles;
	cMultiMesh* m_meshChoroidPlexus;
	cMultiMesh* m_meshThalastriateVein;
};

#endif