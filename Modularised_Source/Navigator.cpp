#include "Navigator.h"

//default constructor
Navigator::Navigator(){}

Navigator::Navigator(cWorld* world){
	// create a camera and insert it into the virtual world
    camera = new cCamera(world);
    world->addChild(camera);

    // position and orient the camera
    camera->set(cVector3d(1.4, 0.0, 0.0),    // camera position (eye)
                cVector3d(0.0, 0.0, 0.0),    // lookat position (target)
                cVector3d(0.0, 0.0, 1.0));   // direction of the "up" vector

    // set the near and far clipping planes of the camera
    // anything in front or behind these clipping planes will not be rendered
    camera->setClippingPlanes(0.001, 20.0);

    // set stereo mode
    camera->setStereoMode(C_STEREO_DISABLED);

    // set stereo eye separation and focal length (applies only if stereo is enabled)
    camera->setStereoEyeSeparation(0.03);
    camera->setStereoFocalLength(3.0);

    // set vertical mirrored display mode
    camera->setMirrorVertical(false);

    // set camera field of view
    camera->setFieldViewAngleDeg(60);


    camera->setSphericalAzimuthDeg(-37.5);
    camera->setSphericalPolarDeg(18);
    camera->setSphericalRadius(2.0);
}

void Navigator::initEndoscope(cWorld* world, string meshPath, string scopePath){
	// create a virtual mesh
    meshEndoscope = new cMultiMesh();
    world->addChild(meshEndoscope);

    // load an object file
    bool fileload = meshEndoscope->loadFromFile(meshPath);
    if (!fileload)
    	throw -1;

    // disable culling so that faces are rendered on both sides
    meshEndoscope->setUseCulling(false);

    // scale model
    meshEndoscope->scale(0.1);

    // use display list for faster rendering
    meshEndoscope->setUseDisplayList(true);

    // position object in scene
    meshEndoscope->rotateExtrinsicEulerAnglesDeg(0, 0, 0, C_EULER_ORDER_XYZ);

    cameraEndoScope = new cCamera(world);
    meshEndoscope->addChild(cameraEndoScope);

    // set the near and far clipping planes of the camera
    // anything in front or behind these clipping planes will not be rendered
    cameraEndoScope->setClippingPlanes(0.001, 100);


    //meshEndoscope->setShowFrame(true);
    cameraEndoScope->setLocalPos(cVector3d(-0.001, 0, 0.001));
    //m_meshThirdVentricles->setShowFrame(true);
    // ***************************************************************************************************************************


    // ************************************************ Light Source ****************************************************
    light_scope = new cDirectionalLight(world);
    meshEndoscope->addChild(light_scope);
    light_scope->setEnabled(true);
    light_scope->setDir(meshEndoscope->getLocalPos());


    // set lighting conditions
    light_scope->m_ambient.set(0.8f, 0.8f, 0.8f);
    light_scope->m_diffuse.set(0.8f, 0.8f, 0.8f);
    light_scope->m_specular.set(0.3f, 0.3f, 0.3f);

    cBackground* frontground = new cBackground();
    fileload = frontground->loadFromFile(scopePath);
    if (!fileload)
    	throw -2;
    cameraEndoScope->m_frontLayer->addChild(frontground);

    switchtool = true;
    perforate = false;
    simulationRunning = false;
    simulationFinished = true;
}

void Navigator::switchTool(cWorld* world){
	switchtool = !switchtool;
    if(switchtool){
        world->addChild(meshEndoscope);
        world->removeChild(drillTool);
    }
    else{
        world->addChild(drillTool);
        world->removeChild(meshEndoscope);
    }
}

void Navigator::rotateCamera(int dx, int dy){
	// compute new camera angles
    double azimuthDeg = camera->getSphericalAzimuthDeg() - 0.5 * dx;
    double polarDeg = camera->getSphericalPolarDeg() - 0.5 * dy;

    // assign new angles
    camera->setSphericalAzimuthDeg(azimuthDeg);
    camera->setSphericalPolarDeg(polarDeg);
}

void Navigator::translateCamera(int dx, int dy){
	cVector3d oldPos = camera->getLocalPos();
    cVector3d newPos = cVector3d(oldPos.x() + 0.001*dx, oldPos.y() + 0.001*dy, oldPos.z()-0.001*dy);
    //camera->setSphericalAzimuthReference(newPos);
    camera->setLocalPos(newPos);
}

void Navigator::updateRadius(double a_offsetY){
	double r = camera->getSphericalRadius();
    r = cClamp(r + 0.1 * a_offsetY, 0.001, 10.0);
    camera->setSphericalRadius(r);
}

void Navigator::cameraView(int width, int height){
	camera->renderView(width, height);
}

void Navigator::endoscopeView(int width, int height){
	cameraEndoScope->renderView(width, height);
}

void Navigator::setEndoscopePos(cVector3d pos, cMatrix3d rot){
	meshEndoscope->setLocalPos(pos);
    meshEndoscope->setLocalRot(rot);
}

void Navigator::initHaptic(){
    // create a haptic device handler
    handler = new cHapticDeviceHandler();

    // get access to the first available haptic device found
    handler->getDevice(hapticDevice, 0);

    // retrieve information about the current haptic device
    hapticDeviceInfo = hapticDevice->getSpecifications();

    // open connection to haptic device
    hapticDevice->open();

    hapticDevice->setEnableGripperUserSwitch(true);
    // desired workspace radius of the cursor
    cursorWorkspaceRadius = 1.0;

    // read the scale factor between the physical workspace of the haptic
    // device and the virtual workspace defined for the tool
    workspaceScaleFactor = cursorWorkspaceRadius / hapticDeviceInfo.m_workspaceRadius;


    // define a scale factor between the force perceived at the cursor and the
    // forces actually sent to the haptic device
    deviceForceScale = 0.15 * hapticDeviceInfo.m_maxLinearForce;

    // create a large sphere that represents the haptic device
    deviceRadius = 0.01;

    // interaction stiffness between tool and deformable model
    stiffness = 20;
}

void Navigator::addDrillTool(string path){
    // create a new mesh.
    drillTool = new cMultiMesh();
    // load a drill like mesh and attach it to the tool
    bool fileload = drillTool->loadFromFile(path);
    if (!fileload)
        throw -1;

    // resize tool mesh model
    drillTool->scale(0.0015);

    // remove the collision detector. we do not want to compute any
    // force feedback rendering on the object itself.
    drillTool->deleteCollisionDetector(true);

    // define a material property for the mesh
    cMaterial mat;
    mat.m_ambient.set(0.5f, 0.5f, 0.5f);
    mat.m_diffuse.set(0.8f, 0.8f, 0.8f);
    mat.m_specular.set(1.0f, 1.0f, 1.0f);
    drillTool->setMaterial(mat, true);
    drillTool->computeAllNormals();

    //device->setShowFrame(true);
    //drillTool->setShowFrame(true);
}

void Navigator::switchPerf(){
    perforate = !perforate;
}

void Navigator::closeHaptic(){
    // stop the simulation
    simulationRunning = false;

    // wait for graphics and haptics loops to terminate
    while (!simulationFinished) { cSleepMs(100); }

    hapticDevice->close();
    delete handler;
}

void Navigator::updateHaptics(void)
{

    cout << "thstarted" << endl;
    // initialize precision clock
    cPrecisionClock clock;
    clock.reset();

    // simulation in now running
    simulationRunning  = true;
    simulationFinished = false;

    // main haptic simulation loop
    while(simulationRunning)
    {
        // stop clock
        double time = cMin(0.001, clock.stop());

        // restart clock
        clock.start(true);

        // read position from haptic device
        cVector3d pos;
        cMatrix3d rot;
        hapticDevice->getPosition(pos);
        hapticDevice->getRotation(rot);
        pos.mul(workspaceScaleFactor);
        //cout << "workspaceScaleFactor " << workspaceScaleFactor << endl;
        //device->setLocalPos(pos);
        //_device->setLocalPos(pos);
        if(switchtool)
            setEndoscopePos(pos, rot);
        else
        {
            drillTool->setLocalPos(pos);
            drillTool->setLocalRot(camera->getLocalRot());
        }

        if(perforate)
        {
            cVector3d force = deformModel.perforate(pos, deviceRadius, stiffness, time);
            // scale force
            force.mul(deviceForceScale);
            // send forces to haptic device
            hapticDevice->setForce(force);
        }
        // signal frequency counter
        freqCounterHaptics.signal(1);
    }
    // exit haptics thread
    simulationFinished = true;
    cout << "threadFinished" << endl;
}

void Navigator::addDeformPointer(DeformModel x){
    deformModel = x;
}

double Navigator::getMaxStiffness(){
    // stiffness property
    double maxStiffness = hapticDeviceInfo.m_maxLinearStiffness / workspaceScaleFactor;
    return maxStiffness;
}