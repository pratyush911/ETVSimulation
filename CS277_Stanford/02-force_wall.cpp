//===========================================================================
/*
    CS277 - Experimental Haptics
    Assignment1 - Force Ball
*/
//===========================================================================

//---------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
//---------------------------------------------------------------------------
#include "chai3d.h"
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// DECLARED CONSTANTS
//---------------------------------------------------------------------------


double wall_y = -0.015;
double ground_z = -0.01;
double stiffness = 0;
double stiffness_divisor = 10;
double force_vector_divisor = 2000 / stiffness_divisor;

// initial size (width/height) in pixels of the display window
const int WINDOW_SIZE_W         = 600;
const int WINDOW_SIZE_H         = 600;

// mouse menu options (right button)
const int OPTION_FULLSCREEN     = 1;
const int OPTION_WINDOWDISPLAY  = 2;


//---------------------------------------------------------------------------
// DECLARED VARIABLES
//---------------------------------------------------------------------------

// a world that contains all objects of the virtual environment
cWorld* world;

// a camera that renders the world in a window display
cCamera* camera;

// a light source to illuminate the objects in the virtual scene
cLight *light;

// width and height of the current window display
int displayW  = 0;
int displayH  = 0;

// a haptic device handler
cHapticDeviceHandler* handler;

// a pointer to the first haptic device detected on this computer
cGenericHapticDevice* hapticDevice = 0;

// a 3D cursor for the haptic device
cShapeSphere* cursor = 0;

// a line to display velocity of the haptic interface
cShapeLine* velocityVector;

// labels to show haptic device position and update rate
cLabel* positionLabel;
cLabel* rateLabel;
double rateEstimate = 0;

// material properties used to render the color of the cursors
cMaterial matCursorButtonON;
cMaterial matCursorButtonOFF;

// status of the main simulation haptics loop
bool simulationRunning = false;

// has exited haptics simulation thread
bool simulationFinished = false;

//Adding a sphere and a cube

int scene = 0;

//scene 0
cShapeSphere *sphere;
cMesh* wall;
cTexture2D* texture;
int vertices[6][4];

//scene 1
cMesh* ground;

//scene 2
cShapeSphere *sphere3;
cPrecisionClock *timer;
cLabel *gameInfoLabel;
cLabel *gameInfoLabel2;
cLabel *gameInfoLabel3;
cCamera *cam2;
cBitmap *bitmap;

//---------------------------------------------------------------------------
// DECLARED FUNCTIONS
//---------------------------------------------------------------------------

// callback when the window display is resized
void resizeWindow(int w, int h);

// callback when a keyboard key is pressed
void keySelect(unsigned char key, int x, int y);

// callback when the right mouse button is pressed to select a menu item
void menuSelect(int value);

// function called before exiting the application
void close(void);

// main graphics callback
void updateGraphics(void);

// main haptics loop
void updateHaptics(void);


//===========================================================================
/*

*/
//===========================================================================

int main(int argc, char* argv[])
{
    //-----------------------------------------------------------------------
    // INITIALIZATION
    //-----------------------------------------------------------------------

    printf ("\n");
    printf ("-----------------------------------\n");
    printf ("CS277 - Experimental Haptics\n");
    printf ("Example 02: Force Wall\n");
    printf ("Adam Leeper\n");
    printf ("January 2012, Stanford University\n");
    printf ("-----------------------------------\n");
	  printf ("Press key [n] to change scenes!");
    printf ("\n\n");

    //-----------------------------------------------------------------------
    // 3D - SCENEGRAPH
    //-----------------------------------------------------------------------

    // create a new world.
    world = new cWorld();

    // set the background color of the environment
    // the color is defined by its (R,G,B) components.
    world->setBackgroundColor(0.2, 0.2, 0.2);

    // create a camera and insert it into the virtual world
    camera = new cCamera(world);
    world->addChild(camera);

    // position and oriente the camera
    camera->set( cVector3d (0.2, 0.0, 0.0),    // camera position (eye)
                 cVector3d (0.0, 0.0, 0.0),    // lookat position (target)
                 cVector3d (0.0, 0.0, 1.0));   // direction of the "up" vector

    // set the near and far clipping planes of the camera
    // anything in front/behind these clipping planes will not be rendered
    camera->setClippingPlanes(0.01, 10.0);

    // create a light source and attach it to the camera
    light = new cLight(world);
    camera->addChild(light);                   // attach light to camera
    light->setEnabled(true);                   // enable light source
    light->setPos(cVector3d( 2.0, 0.5, 1.0));  // position the light source
    light->setDir(cVector3d(-2.0, 0.5, 1.0));  // define the direction of the light beam

    // create a label that shows the haptic loop update rate
    rateLabel = new cLabel();
    rateLabel->setPos(8, 24, 0);
    camera->m_front_2Dscene.addChild(rateLabel);



    //-----------------------------------------------------------------------
    // HAPTIC DEVICES / TOOLS
    //-----------------------------------------------------------------------

    // create a haptic device handler
    handler = new cHapticDeviceHandler();

    // read the number of haptic devices currently connected to the computer
    int numHapticDevices = handler->getNumDevices();

    // if there is at least one haptic device detected...
	cHapticDeviceInfo info;
    if (numHapticDevices)
    {
        // get a handle to the first haptic device
        handler->getDevice(hapticDevice);

        // open connection to haptic device
        hapticDevice->open();

		// initialize haptic device
		hapticDevice->initialize();

        // retrieve information about the current haptic device
       info = hapticDevice->getSpecifications();
       stiffness = info.m_maxForceStiffness / stiffness_divisor;

		//printf("max force: %lf",info.m_maxForce);

        // create a cursor with its radius set
        cursor = new cShapeSphere(0.003);

        // add cursor to the world
        world->addChild(cursor);

        // create a small line to illustrate velocity
        velocityVector = new cShapeLine(cVector3d(0,0,0), cVector3d(0,0,0));

        // add line to the world
        world->addChild(velocityVector);

        positionLabel = new cLabel();
        positionLabel->setPos(8, 8, 0);
        camera->m_front_2Dscene.addChild(positionLabel);
    }

//	 double workspaceScaleFactor = tool->getWorkspaceScaleFactor();
//	//maxStiffness
//	double stiffnessMax = info.m_maxForceStiffness / workspaceScaleFactor;



    // here we define the material properties of the cursor when the
    // user button of the device end-effector is engaged (ON) or released (OFF)

    // a light orange material color
    matCursorButtonOFF.m_ambient.set(0.5, 0.2, 0.0);
    matCursorButtonOFF.m_diffuse.set(1.0, 0.5, 0.0);
    matCursorButtonOFF.m_specular.set(1.0, 1.0, 1.0);

    // a blue material color
    matCursorButtonON.m_ambient.set(0.1, 0.1, 0.4);
    matCursorButtonON.m_diffuse.set(0.3, 0.3, 0.8);
    matCursorButtonON.m_specular.set(1.0, 1.0, 1.0);

	//-----------------------------------------------------------------------
    // Scene 0
    //-----------------------------------------------------------------------

	sphere = new cShapeSphere(0.015);
  //world->addChild(sphere);
	sphere->setPos(0.0, 0.00, 0.004);

	//square
  wall = new cMesh(world);
  world->addChild(wall);

  const double HALFSIZE = 0.04;
  wall->setPos(HALFSIZE/2, wall_y, 0.0);

    // face +y
    vertices[3][0] = wall->newVertex( HALFSIZE,   0, -HALFSIZE);
    vertices[3][1] = wall->newVertex(-HALFSIZE,   0, -HALFSIZE);
    vertices[3][2] = wall->newVertex(-HALFSIZE,   0,  HALFSIZE);
    vertices[3][3] = wall->newVertex( HALFSIZE,   0,  HALFSIZE);

    // create triangles
    for (int i=3; i<4; i++)
    {
        wall->newTriangle(vertices[i][0], vertices[i][1], vertices[i][2]);
        wall->newTriangle(vertices[i][0], vertices[i][2], vertices[i][3]);
    }

//	texture = new cTexture2D();
//    wall->setTexture(texture);
//    wall->setUseTexture(true);



    wall->m_material.m_ambient.set(0.5f, 0.5f, 0.5f, 1.0f);
    wall->m_material.m_diffuse.set(0.7f, 0.7f, 0.7f, 1.0f);
    wall->m_material.m_specular.set(1.0f, 1.0f, 1.0f, 1.0f);
    wall->m_material.m_emission.set(0.0f, 0.0f, 0.0f, 1.0f);

  wall->computeAllNormals();

  //wall->rotate(cVector3d(0,0,1), 0.24);
  wall->computeBoundaryBox(true);


	//-----------------------------------------------------------------------
    // Scene 1
    //-----------------------------------------------------------------------


  //square
  ground = new cMesh(world);
  world->addChild(ground);


  ground->setPos(0.0, 0.0, ground_z);


    // face +z
    vertices[5][0] = ground->newVertex( HALFSIZE,  -HALFSIZE,  0);
    vertices[5][1] = ground->newVertex( HALFSIZE,   HALFSIZE,  0);
    vertices[5][2] = ground->newVertex(-HALFSIZE,   HALFSIZE,  0);
    vertices[5][3] = ground->newVertex(-HALFSIZE,  -HALFSIZE,  0);

    // create triangles
    for (int i=5; i<6; i++)
    {
        ground->newTriangle(vertices[i][0], vertices[i][1], vertices[i][2]);
        ground->newTriangle(vertices[i][0], vertices[i][2], vertices[i][3]);
    }

//	texture = new cTexture2D();
//    ground->setTexture(texture);
//    ground->setUseTexture(true);



    ground->m_material.m_ambient.set(0.5f, 0.5f, 0.5f, 1.0f);
    ground->m_material.m_diffuse.set(0.7f, 0.7f, 0.7f, 1.0f);
    ground->m_material.m_specular.set(1.0f, 1.0f, 1.0f, 1.0f);
    ground->m_material.m_emission.set(0.0f, 0.0f, 0.0f, 1.0f);

  ground->computeAllNormals();

  //cube->rotate(cVector3d(0,0,1), 0.24);
  ground->computeBoundaryBox(true);
  ground->setShowEnabled(false, true);
	//-----------------------------------------------------------------------
    // Scene 2
    //-----------------------------------------------------------------------

	sphere3 = new cShapeSphere(0.03);
	world->addChild(sphere3);
	sphere3->setPos(-0.005, 0.00, 0.00);
	sphere3->setShowEnabled(false, true);
	sphere3->setWireMode(true, true);

	timer = new cPrecisionClock();

	gameInfoLabel = new cLabel();
    gameInfoLabel->setPos(WINDOW_SIZE_W / 2 - 150, WINDOW_SIZE_H * 4/ 5, 0);
    camera->m_front_2Dscene.addChild(gameInfoLabel);
	gameInfoLabel->setShowEnabled(false, true);
	//gameInfoLabel->scale(3.0, true);

	gameInfoLabel2 = new cLabel();
    gameInfoLabel2->setPos(20, 50, 0);
    camera->m_front_2Dscene.addChild(gameInfoLabel2);
	gameInfoLabel2->setShowEnabled(false, true);


	//other view
	texture = new cTexture2D();
    wall->setTexture(texture);
    wall->setUseTexture(false);

	bitmap = new cBitmap();
	camera->m_back_2Dscene.addChild(bitmap);
	double scalingFactor = 0.35;
	bitmap->setPos(0,(double)WINDOW_SIZE_H - (double)WINDOW_SIZE_H * scalingFactor,0);
	bitmap->setZoomHV(scalingFactor,scalingFactor);
	bitmap->setShowEnabled(false,true);

	//cam
	cam2 = new cCamera(world);
	cam2->set( cVector3d (0, 0.0, 0.08),    // camera position (eye)
				cVector3d (0.0, 0.0, 0.0),    // lookat position (target)
				cVector3d (-1.0, 0.0, 0.0));   // direction of the "up" vector
	cam2->setClippingPlanes(0.01, 10.0);



    //-----------------------------------------------------------------------
    // OPEN GL - WINDOW DISPLAY
    //-----------------------------------------------------------------------

    // initialize GLUT
    glutInit(&argc, argv);

    // retrieve the resolution of the computer display and estimate the position
    // of the GLUT window so that it is located at the center of the screen
    int screenW = glutGet(GLUT_SCREEN_WIDTH);
    int screenH = glutGet(GLUT_SCREEN_HEIGHT);
    int windowPosX = (screenW - WINDOW_SIZE_W) / 2;
    int windowPosY = (screenH - WINDOW_SIZE_H) / 2;

    // initialize the OpenGL GLUT window
    glutInitWindowPosition(windowPosX, windowPosY);
    glutInitWindowSize(WINDOW_SIZE_W, WINDOW_SIZE_H);
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    glutCreateWindow(argv[0]);
    glutDisplayFunc(updateGraphics);
    glutKeyboardFunc(keySelect);
    glutReshapeFunc(resizeWindow);
    glutSetWindowTitle("CHAI 3D");

    // create a mouse menu (right button)
    glutCreateMenu(menuSelect);
    glutAddMenuEntry("full screen", OPTION_FULLSCREEN);
    glutAddMenuEntry("window display", OPTION_WINDOWDISPLAY);
    glutAttachMenu(GLUT_RIGHT_BUTTON);


    //-----------------------------------------------------------------------
    // START SIMULATION
    //-----------------------------------------------------------------------

    // simulation in now running
    simulationRunning = true;

    // create a thread which starts the main haptics rendering loop
    cThread* hapticsThread = new cThread();
    hapticsThread->set(updateHaptics, CHAI_THREAD_PRIORITY_HAPTICS);

    // start the main graphics rendering loop
    glutMainLoop();

    // close everything
    close();

    // exit
    return (0);
}

//---------------------------------------------------------------------------

void resizeWindow(int w, int h)
{
    // update the size of the viewport
    displayW = w;
    displayH = h;

	  //printf("resize");
	  gameInfoLabel->setPos(w / 2 - 50, h * 4/ 5, 0);

	  double scalingFactor = 0.38;
	  bitmap->setPos(0,(double)h - (double) h * scalingFactor,0);

    glViewport(0, 0, displayW, displayH);


	  double txMin, txMax, tyMin, tyMax;
    if (displayW >= displayH)
    {
        double ratio = (double)displayW / (double)displayH;
        txMin = 0.5 * (ratio - 1.0) / ratio;
        txMax = 1.0 - txMin;
        tyMin = 0.0;
        tyMax = 1.0;
    }
    else
    {
        double ratio = (double)displayH / (double)displayW;
        txMin = 0.0;
        txMax = 1.0;
        tyMin = 0.5 * (ratio - 1.0) / ratio;
        tyMax = 1.0 - tyMin;
    }
}

//---------------------------------------------------------------------------

void keySelect(unsigned char key, int x, int y)
{
    // escape key
    if ((key == 27) || (key == 'x'))
    {
        // close everything
        close();

        // exit application
        exit(0);
    }

	if (key == 'n') {

		scene++;

    scene = scene % 2;


		switch (scene) {
			case 0:

				camera->set( cVector3d (0.2, 0.0, 0.0),    // camera position (eye)
							cVector3d (0.0, 0.0, 0.0),    // lookat position (target)
							cVector3d (0.0, 0.0, 1.0));   // direction of the "up" vector

				sphere->setShowEnabled(true, true);
        wall->setShowEnabled(true, true);
        wall->setUseTexture(false);

        ground->setShowEnabled(false, true);

				sphere3->setShowEnabled(false, true);
				bitmap->setShowEnabled(false,true);
				break;
			case 1:
        wall->setShowEnabled(false, true);

        ground->setShowEnabled(true, true);

				sphere3->setShowEnabled(false, true);
				break;

			case 2:

				camera->set( cVector3d (0.15, 0.0, 0.0),    // camera position (eye)
							cVector3d (0.0, 0.0, 0.01),    // lookat position (target)
							cVector3d (0.0, 0.0, 1.0));   // direction of the "up" vector


				sphere->setShowEnabled(false, true);
        //wall->setShowEnabled(true, true);
        //wall->setUseTexture(true);
        //wall->setPos(0.0,-0.035,0.05);

        ground->setShowEnabled(false, true);


				sphere3->setShowEnabled(true, true);
				bitmap->setShowEnabled(true,true);

				printf("Game rules: Don't touch the magnetic sphere or the magnetic line. \n");
				printf("Stay inside the big sphere at all times! \n");
				printf("Start game by pressing the main falcon button.\n");
				printf("Use the top view (top left corner) to help with orientation.\n");
				break;


			default:
				break;
		}



	}
}

//---------------------------------------------------------------------------

void menuSelect(int value)
{
    switch (value)
    {
        // enable full screen display
        case OPTION_FULLSCREEN:
            glutFullScreen();
            break;

        // reshape window to original size
        case OPTION_WINDOWDISPLAY:
            glutReshapeWindow(WINDOW_SIZE_W, WINDOW_SIZE_H);
            break;
    }
}

//---------------------------------------------------------------------------

void close(void)
{
    // stop the simulation
    simulationRunning = false;

    // wait for graphics and haptics loops to terminate
    while (!simulationFinished) { cSleepMs(100); }

    // close the haptic devices
    if (hapticDevice)
    {
        hapticDevice->close();
    }
}

//---------------------------------------------------------------------------

void updateGraphics(void)
{
    // update the label showing the position of the haptic device
    if (cursor)
    {
        cVector3d position = cursor->getPos() * 1000.0; // convert to mm
        char buffer[128];
        sprintf(buffer, "device position: (%.2lf, %.2lf, %.2lf) mm",
            position.x, position.y, position.z);
        positionLabel->m_string = buffer;
    }

    // update the label with the haptic refresh rate
    char buffer[128];
    sprintf(buffer, "haptic rate: %.0lf Hz", rateEstimate);
    rateLabel->m_string = buffer;

	//********** game *****************

	//gameInfoLabel->m_string = "TESTING TESTING";

	double currentTime = timer->getCurrentTimeSeconds();
	bool isGameOn = timer->on();

	gameInfoLabel->setShowEnabled(false, true);

	if (currentTime < 2 && currentTime > 0) {
		gameInfoLabel->setShowEnabled(true, true);
		gameInfoLabel->m_string = "Beware the magnetic killers!!!";

		timer->setTimeoutPeriodSeconds(0.02 + currentTime);


	} else {

	}

	static bool hasPrintedMeOnce = false;

	if (currentTime > 2 && isGameOn) { //game is going on

		gameInfoLabel2->setShowEnabled(true, true);

		char buffer[128];
		sprintf(buffer, "Score %.1lf", currentTime - 2);
		gameInfoLabel2->m_string = buffer;
		hasPrintedMeOnce = false;

	} else {
		gameInfoLabel2->setShowEnabled(false, true);
	}

	if (currentTime > 0 && !isGameOn) {
		//game ended
		gameInfoLabel->setShowEnabled(true, true);
		char buffer[128];
		sprintf(buffer, "You scored %.1lf points. Better luck next time!", currentTime - 2);
		gameInfoLabel->m_string = buffer;

		if (!hasPrintedMeOnce) {
			hasPrintedMeOnce = true;

			printf("You scored %.1lf points. Better luck next time!\n", currentTime - 2);
		}

	}

	if (scene == 2) {

		cam2->renderView(displayW, displayH);
		cam2->copyImageData(&bitmap->m_image);

	}




	//********** game end *************

    // render world

    texture->markForUpdate();
    camera->renderView(displayW, displayH);



    // Swap buffers
    glutSwapBuffers();

    // check for any OpenGL errors
    GLenum err;
    err = glGetError();
    if (err != GL_NO_ERROR) printf("Error:  %s\n", gluErrorString(err));

    // inform the GLUT window to call updateGraphics again (next frame)
    if (simulationRunning)
    {
        glutPostRedisplay();
    }

}

//---------------------------------------------------------------------------

void updateHaptics(void)
{
    // a clock to estimate the haptic simulation loop update rate
    cPrecisionClock pclock;
    pclock.setTimeoutPeriodSeconds(1.0);
    pclock.start(true);
    int counter = 0;

    // main haptic simulation loop
    while(simulationRunning)
    {
		if (!hapticDevice) continue;

		world->computeGlobalPositions(true);

        // read position of haptic device
        cVector3d newPosition;
        hapticDevice->getPosition(newPosition);

        // update position and orientation of cursor
        cursor->setPos(newPosition);

        // read linear velocity from device
        cVector3d linearVelocity;
        hapticDevice->getLinearVelocity(linearVelocity);
        cVector3d last_force;
        hapticDevice->getForce(last_force);

        // update the line showing force
        velocityVector->m_pointA = newPosition;
        velocityVector->m_pointB = newPosition + last_force/force_vector_divisor;

        // read user button status
        bool buttonStatus;
        hapticDevice->getUserSwitch(0, buttonStatus);

        // adjust the color of the cursor according to the status of
        // the user switch (ON = TRUE / OFF = FALSE)
        cursor->m_material = buttonStatus ? matCursorButtonON : matCursorButtonOFF;

        // compute a reaction force (a spring that pulls the device to the origin)
        // const double stiffness = 100; // [N/m]
        // cVector3d force = -stiffness * newPosition;
		cVector3d force = cVector3d();
		force.zero();

		if (scene == 0) {

			cVector3d c_pos = newPosition;

      double penetration = wall_y - c_pos.y;

      if(penetration > 0)
      {
        force.y = stiffness * penetration;
      }

		}
    else if (scene == 1)
    {

      cVector3d c_pos = newPosition;

      double penetration = ground_z - c_pos.z;

      if(penetration > 0)
      {
        if(c_pos.y <  0.01)
          force.z += stiffness * penetration;
        if(c_pos.y > -0.01)
          force.z += stiffness * penetration;
      }

		}

        // send computed force to haptic device

		double temp = force.length();

		static int displayCounter = 0;
		displayCounter++;

		//if (displayCounter % 100 == 0)
			//printf("f: %lf \n",temp);

        hapticDevice->setForce(force);

        // estimate the refresh rate
        ++counter;
        if (pclock.timeoutOccurred()) {
            pclock.stop();
            rateEstimate = counter;
            counter = 0;
            pclock.start(true);
        }
    }

    // exit haptics thread
    simulationFinished = true;
}

//---------------------------------------------------------------------------
