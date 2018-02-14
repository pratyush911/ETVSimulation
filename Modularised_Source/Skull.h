#ifndef SKULL_H
#define SKULL_H

#include "chai3d.h"
#include <fstream>

using namespace chai3d;
using namespace std;

class Skull{
public:
	cVoxelObject* m_voxelBrainSkull;

	Skull();
	Skull(string model, string colormap, double maxStiffness, cWorld* world);
	void updateOpacity(float dx);

private:
	cMultiImagePtr m_imagesBrainSkull;
	cTexture3dPtr m_textureBrainSkull;
	cImagePtr    m_lutBrainSkull;
	float voxelBrainSkullOpacityThreshold;

};

#endif