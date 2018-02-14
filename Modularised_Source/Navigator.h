#ifndef NAVIGATOR_H
#define NAVIGATOR_H

#include "chai3d.h"
#include "DeformModel.h"

using namespace chai3d;
using namespace std;

class Navigator{
public:
	// a camera to render the world in the window display
	cCamera* camera;
	cMultiMesh* drillTool;
	// a frequency counter to measure the simulation haptic rate
	cFrequencyCounter freqCounterHaptics;
	
	Navigator();
	Navigator(cWorld* world);
	void initEndoscope(cWorld* world, string meshPath, string scopePath);
	void switchTool(cWorld* world);
	void rotateCamera(int dx, int dy);
	void translateCamera(int dx, int dy);
	void updateRadius(double a_offsetY);
	void cameraView(int width, int height);
	void endoscopeView(int width, int height);
	void setEndoscopePos(cVector3d pos, cMatrix3d rot);
	void initHaptic();
	void addDrillTool(string path);
	void startThread();
	void switchPerf();
	void closeHaptic();
	void addDeformPointer(DeformModel x);
	double getMaxStiffness();

	// this function contains the main haptics simulation loop
	void updateHaptics(void);

private:
	// a camera attached to the endocope object
	cCamera* cameraEndoScope;

	// a virtual object
	cMultiMesh* meshEndoscope;

	cDirectionalLight *light_scope;

	// a haptic device handler
	cHapticDeviceHandler* handler;

	// a pointer to the current haptic device
	cGenericHapticDevicePtr hapticDevice;

	// haptic device model
	double deviceRadius;

	// force scale factor
	double deviceForceScale;

	// scale factor between the device workspace and cursor workspace
	double workspaceScaleFactor;

	// desired workspace radius of the virtual cursor
	double cursorWorkspaceRadius;

	// stiffness properties between the haptic device tool and the model (GEM)
	double stiffness;

	cHapticDeviceInfo hapticDeviceInfo;

	bool switchtool;
	bool perforate;
	bool simulationRunning;
	bool simulationFinished;

	//Pointer to the Deformable model
	DeformModel deformModel;

};
#endif