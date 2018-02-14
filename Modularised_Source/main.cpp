

//------------------------------------------------------------------------------
#include "chai3d.h"
//------------------------------------------------------------------------------
#include <GLFW/glfw3.h>
#include <fstream>
//------------------------------------------------------------------------------
#include "VentricularStructures.h"
#include "DeformModel.h"
#include "Navigator.h"
#include "Skull.h"
// #include "Skull.cpp"

#include <string>
//------------------------------------------------------------------------------
using namespace chai3d;
using namespace std;
//------------------------------------------------------------------------------

#define DATA_LOC string("/home/icy/Desktop/VR-Simulation/ETV-Simulator/Data")

//------------------------------------------------------------------------------
// GENERAL SETTINGS
//------------------------------------------------------------------------------

// stereo Mode
/*
    C_STEREO_DISABLED:            Stereo is disabled
    C_STEREO_ACTIVE:              Active stereo for OpenGL NVDIA QUADRO cards
    C_STEREO_PASSIVE_LEFT_RIGHT:  Passive stereo where L/R images are rendered next to each other
    C_STEREO_PASSIVE_TOP_BOTTOM:  Passive stereo where L/R images are rendered above each other
*/

// fullscreen mode
bool fullscreen = false;

// mirrored display
bool mirroredDisplay = false;


enum MouseStates
{
    MOUSE_IDLE,
    MOUSE_ROTATE_CAMERA,
    MOUSE_TRANSLATE_CAMERA
};


//------------------------------------------------------------------------------
// DECLARED VARIABLES
//------------------------------------------------------------------------------

// a world that contains all objects of the virtual environment
cWorld* world;

Navigator navigator;

// objects in the scene
// deformable world
DeformModel deformModel;

VentricularStructures vStructures;

Skull skull;

// haptic thread
cThread* hapticsThread;

// a colored background
cBackground* background;

// a font for rendering text
cFontPtr font;

// a label to display the rate [Hz] at which the simulation is running
cLabel* labelRates;

// a frequency counter to measure the simulation graphic rate
cFrequencyCounter freqCounterGraphics;

// a handle to window display context
GLFWwindow* window0 = NULL;
int width0  = 0;
int height0 = 0;

// a second window
GLFWwindow* window1 = NULL;
int width1 = 0;
int height1 = 0;

// swap interval for the display context (vertical synchronization)
int swapInterval = 1;

// mouse state
MouseStates mouseState = MOUSE_IDLE;

// last mouse position
double mouseX, mouseY;


//------------------------------------------------------------------------------
// DECLARED FUNCTIONS
//------------------------------------------------------------------------------

// callback when the window display is resized
void windowSizeCallback0(GLFWwindow* a_window, int a_width, int a_height);

// callback when the window display is resized
void windowSizeCallback1(GLFWwindow* a_window, int a_width, int a_height);

// callback when an error GLFW occurs
void errorCallback(int error, const char* a_description);

// callback when a key is pressed
void keyCallback(GLFWwindow* a_window, int a_key, int a_scancode, int a_action, int a_mods);

// callback to handle mouse click
void mouseButtonCallback(GLFWwindow* a_window, int a_button, int a_action, int a_mods);

// callback to handle mouse motion
void mouseMotionCallback(GLFWwindow* a_window, double a_posX, double a_posY);

// callback to handle mouse scroll
void mouseScrollCallback(GLFWwindow* a_window, double a_offsetX, double a_offsetY);

// this function renders the scene
void updateGraphics0(void);

// this function renders the endoscopic scene
void updateGraphics1(void);

// this function closes the application
void close(void);

void updateHaptics(void);

void translateModel(float dx, float dy, float dz);


int main(int argc, char* argv[])
{
    //--------------------------------------------------------------------------
    // INITIALIZATION
    //--------------------------------------------------------------------------

    cout << endl;
    cout << "-----------------------------------" << endl;
    cout << "CHAI3D" << endl;
    cout << "Demo: 08-shaders" << endl;
    cout << "Copyright 2003-2016" << endl;
    cout << "-----------------------------------" << endl << endl << endl;
    cout << "Keyboard Options:" << endl << endl;
    cout << "[f] - Enable/Disable full screen mode" << endl;
    cout << "[m] - Enable/Disable vertical mirroring" << endl;
    cout << "[q] - Exit application" << endl;
    cout << endl << endl;


    //--------------------------------------------------------------------------
    // OPEN GL - WINDOW DISPLAY
    //--------------------------------------------------------------------------

    // initialize GLFW library
    if (!glfwInit())
    {
        cout << "failed initialization" << endl;
        cSleepMs(1000);
        return 1;
    }

    // set error callback
    glfwSetErrorCallback(errorCallback);

    // compute desired size of window
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    int space = 10;
    int w = 0.8 * mode->height;
    int h = 0.5 * mode->height;
    int x0 = 0.5 * mode->width - w - space;
    int y0 = 0.5 * (mode->height - h);
    int x1 = 0.5 * mode->width + space;
    int y1 = 0.5 * (mode->height - h);

    // set OpenGL version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    // set active stereo mode
    glfwWindowHint(GLFW_STEREO, GL_FALSE);
    

    ////////////////////////////////////////////////////////////////////////////
    // SETUP WINDOW 0
    ////////////////////////////////////////////////////////////////////////////
    // create display context
    window0 = glfwCreateWindow(w, h, "Endoscopic Third Ventriculostomy - World View", NULL, NULL);
    if (!window0)
    {
        cout << "failed to create window" << endl;
        cSleepMs(1000);
        glfwTerminate();
        return 1;
    }

    // get width and height of window
    glfwGetWindowSize(window0, &width0, &height0);

    // set position of window
    glfwSetWindowPos(window0, x0, y0);

    // set key callback
    glfwSetKeyCallback(window0, keyCallback);

    // set resize callback
    glfwSetWindowSizeCallback(window0, windowSizeCallback0);

    // set current display context
    glfwMakeContextCurrent(window0);

    // sets the swap interval for the current display context
    glfwSwapInterval(swapInterval);

    // set mouse position callback
    glfwSetCursorPosCallback(window0, mouseMotionCallback);

    // set mouse button callback
    glfwSetMouseButtonCallback(window0, mouseButtonCallback);

    // set mouse scroll callback
    glfwSetScrollCallback(window0, mouseScrollCallback);


    ////////////////////////////////////////////////////////////////////////////
    // SETUP WINDOW 1
    ////////////////////////////////////////////////////////////////////////////
    // create display context and share GPU data with window 0
    window1 = glfwCreateWindow(w, h, "Endoscopic Third Ventriculostomy - Endoscopic View", NULL, window0);
    if (!window1)
    {
        cout << "failed to create window" << endl;
        cSleepMs(1000);
        glfwTerminate();
        return 1;
    }

    // get width and height of window
    glfwGetWindowSize(window1, &width1, &height1);

    // set position of window
    glfwSetWindowPos(window1, x1, y1);

    // set key callback
    glfwSetKeyCallback(window1, keyCallback);

    // set resize callback
    glfwSetWindowSizeCallback(window1, windowSizeCallback1);

    // set current display context
    glfwMakeContextCurrent(window1);

    // sets the swap interval for the current display context
    glfwSwapInterval(swapInterval);



#ifdef GLEW_VERSION
    // initialize GLEW library
    if (glewInit() != GLEW_OK)
    {
        cout << "failed to initialize GLEW library" << endl;
        glfwTerminate();
        return 1;
    }
#endif

    //--------------------------------------------------------------------------
    // WORLD - CAMERA - LIGHTING
    //--------------------------------------------------------------------------

    // create a new world.
    world = new cWorld();

    // set the background color of the environment
    // the color is defined by its (R,G,B) components.
    world->m_backgroundColor.setBlueAqua();

    //initialize Navigator with camera properties
    navigator = Navigator(world);

    navigator.initHaptic();



    //--------------------------------------------------------------------------
    // CREATE Deformable Third Ventricle
    //--------------------------------------------------------------------------
    try{
        deformModel = DeformModel(world, DATA_LOC + "/mesh_models/Third-Ventricle.obj");
    }
    catch(int x){
        printf("Error - 3D Model failed to load correctly.\n");
        close();
        return (-1);
    }
    deformModel.BuildDynamicModel(DATA_LOC);
    deformModel.initSkeleton();
    deformModel.renderSpheres(world);
    navigator.addDeformPointer(deformModel);



    // ************************************************************* Add the other ventricular structures *********************************************
    try{
    	vStructures = VentricularStructures(DATA_LOC + "/mesh_models/Ventricles.obj",
    		DATA_LOC + "/mesh_models/ventricle_texture_NORM.png",
    		DATA_LOC + "/mesh_models/Thalamostriate_Vein.obj",
    		DATA_LOC + "/mesh_models/Choroid_Plexus.obj",
    		world);
	}
	catch(int x){
		switch(x){
			case -1:
				printf("Error - 3D Lateral Model failed to load correctly.\n");
				break;

			case -2:
				printf("Error - Normal Map failed to load correctly.\n");
				break;

			case -3:
				printf("Error - 3D Thalamostriate_Vein Model failed to load correctly.\n");
				break;

			case -4:
				printf("Error - 3D Choroid_Plexus Model failed to load correctly.\n");
				break;
		}
		close();
        return (-1);
	}



    // **************************************************  Add the shaders and use the meshes for the rendering **************************************
    // CREATE SHADERS
    // create vertex shader
    cShaderPtr vertexShader = cShader::create(C_VERTEX_SHADER);
    bool fileload = vertexShader->loadSourceFile(DATA_LOC + "/shaders/bump.vert");

    // create fragment shader
    cShaderPtr fragmentShader = cShader::create(C_FRAGMENT_SHADER);
    fileload = fragmentShader->loadSourceFile(DATA_LOC + "/shaders/bump.frag");

    // create program shader
    cShaderProgramPtr programShader = cShaderProgram::create();

    // assign vertex shader to program shader
    programShader->attachShader(vertexShader);

    // assign fragment shader to program shader
    programShader->attachShader(fragmentShader);

    // assign program shader to object
    vStructures.setShader(programShader);

    // link program shader
    programShader->linkProgram();



    // set uniforms
    programShader->setUniformi("uColorMap", 0);
    programShader->setUniformi("uShadowMap", 0);
    programShader->setUniformi("uNormalMap", 2);
    programShader->setUniformf("uInvRadius", 0.1f);

    // *************************************** Add the volume and skull model *************************************************************************
    try{
    	skull = Skull(DATA_LOC + "/skull/png/brain-0",DATA_LOC + "/colormap/colormap_out.png", navigator.getMaxStiffness(), world);
    }
    catch(int x){
    	close();
    	return -1;

    }
    // ****************************************************************************************************************************************


    // ********************************************  Endoscopic Camera ***********************************************************
    try{
    	navigator.initEndoscope(world, DATA_LOC + "/endoscope.3ds", DATA_LOC + "/scope.png");
    }
    catch(int x){
    	if(x==-1)
    		cout << "Error - 3D Model Endoscope failed to load correctly." << endl;
    	else
    		cout << "Error - Scope Texture failed to load correctly." << endl;
        close();
        return (-1);
    }



    try{
    	navigator.addDrillTool(DATA_LOC + "/drill.3ds");
    }
    catch(int x){
    	printf("Error - 3D Model failed to load correctly.\n");
        close();
        return (-1);
    }
    


    //--------------------------------------------------------------------------
    // WIDGETS
    //--------------------------------------------------------------------------

    // create a font
    font = NEW_CFONTCALIBRI20();

    // create a label to display the haptic and graphic rate of the simulation
    labelRates = new cLabel(font);
    navigator.camera->m_frontLayer->addChild(labelRates);

    // set font color
    labelRates->m_fontColor.setWhite();

    // create a background
    background = new cBackground();
    navigator.camera->m_backLayer->addChild(background);

    // set background properties
    fileload = background->loadFromFile(DATA_LOC + "/Craniotomy_and_Trocar_placement_imgs/img_background.jpg");
    if (!fileload)
    {
        cout << "Error - Background Image failed to load correctly." << endl;
        close();
    }
    //--------------------------------------------------------------------------
    // START SIMULATION
    //--------------------------------------------------------------------------
    hapticsThread = new cThread();
    hapticsThread->start(updateHaptics, CTHREAD_PRIORITY_HAPTICS);

    // setup callback when application exits
    atexit(close);


    //--------------------------------------------------------------------------
    // MAIN GRAPHIC LOOP
    //--------------------------------------------------------------------------

    // call window size callback at initialization
    windowSizeCallback0(window0, width0, height0);
    windowSizeCallback1(window1, width1, height1);

    // main graphic loop
    while ((!glfwWindowShouldClose(window0)) && (!glfwWindowShouldClose(window1)))
    {
        ////////////////////////////////////////////////////////////////////////
        // RENDER WINDOW 0
        ////////////////////////////////////////////////////////////////////////

        // activate display context
        glfwMakeContextCurrent(window0);

        // get width and height of window
        glfwGetWindowSize(window0, &width0, &height0);

        // render graphics
        updateGraphics0();

        // swap buffers
        glfwSwapBuffers(window0);


        ////////////////////////////////////////////////////////////////////////
        // RENDER WINDOW 1
        ////////////////////////////////////////////////////////////////////////

        // activate display context
        glfwMakeContextCurrent(window1);

        // get width and height of window
        glfwGetWindowSize(window1, &width1, &height1);

        // render graphics
        updateGraphics1();

        // swap buffers
        glfwSwapBuffers(window1);

        // process events
        glfwPollEvents();

        // signal frequency counter
        freqCounterGraphics.signal(1);
    }

    // close window
    glfwDestroyWindow(window0);
    glfwDestroyWindow(window1);

    // terminate GLFW library
    glfwTerminate();

    // exit
    return 0;
}



//------------------------------------------------------------------------------

void windowSizeCallback0(GLFWwindow* a_window, int a_width, int a_height)
{
    // update window size
    width0  = a_width;
    height0 = a_height;
}

//------------------------------------------------------------------------------

void windowSizeCallback1(GLFWwindow* a_window, int a_width, int a_height)
{
    // update window size
    width1  = a_width;
    height1 = a_height;
}
//------------------------------------------------------------------------------

void errorCallback(int a_error, const char* a_description)
{
    cout << "Error: " << a_description << endl;
}

//------------------------------------------------------------------------------

void keyCallback(GLFWwindow* a_window, int a_key, int a_scancode, int a_action, int a_mods)
{
    // filter calls that only include a key press
    if ((a_action != GLFW_PRESS) && (a_action != GLFW_REPEAT))
    {
        return;
    }

    switch(a_key){
    	case GLFW_KEY_ESCAPE:
    	case GLFW_KEY_Q:
    		glfwSetWindowShouldClose(a_window, GLFW_TRUE);
    		break;

    	case GLFW_KEY_F:
    	{
    		// toggle state variable
        	fullscreen = !fullscreen;

        	// get handle to monitor
        	GLFWmonitor* monitor = glfwGetPrimaryMonitor();

       		// get information about monitor
        	const GLFWvidmode* mode = glfwGetVideoMode(monitor);

        	// set fullscreen or window mode
        	if (fullscreen)
        	{
            	glfwSetWindowMonitor(window0, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
            	glfwSwapInterval(swapInterval);
        	}
        	else
        	{
            	int w = 0.8 * mode->height;
            	int h = 0.5 * mode->height;
            	int x = 0.5 * (mode->width - w);
            	int y = 0.5 * (mode->height - h);
            	glfwSetWindowMonitor(window0, NULL, x, y, w, h, mode->refreshRate);
            	glfwSwapInterval(swapInterval);
        	}
        }
        	break;


        case GLFW_KEY_M:
        	mirroredDisplay = !mirroredDisplay;
        	navigator.camera->setMirrorVertical(mirroredDisplay);
        	break;

        case GLFW_KEY_1:
        	skull.updateOpacity(-0.03f);
        	break;

        case GLFW_KEY_2:
        	skull.updateOpacity(0.03f);
        	break;

        case GLFW_KEY_3:
        	world->addChild(skull.m_voxelBrainSkull);
	        break;

	    case GLFW_KEY_4:
        	world->removeChild(skull.m_voxelBrainSkull);
        	break;

        case GLFW_KEY_5:
        	world->removeChild(skull.m_voxelBrainSkull);
        	deformModel.attachToWorld(world);
        	world->addChild(skull.m_voxelBrainSkull);   
        	break;    

        case GLFW_KEY_6:
        	deformModel.removeFromWorld(world);
        	break;

        case GLFW_KEY_S:
        	navigator.switchTool(world);
            navigator.switchPerf();
        	break;
            
        case GLFW_KEY_Z:
        	translateModel(0.01f, 0.0f, 0.0f);
        	break;

        case GLFW_KEY_X:
        	translateModel(-0.01f, 0.0f, 0.0f);
        	break;

        case GLFW_KEY_C:
        	translateModel(0.0f, 0.01f, 0.0f);
        	break;

        case GLFW_KEY_V:
        	translateModel(0.0f, -0.01f, 0.0f);
        	break;

        case GLFW_KEY_B:
        	translateModel(0.01f, 0.0f, 0.01f);
        	break;

        case GLFW_KEY_N:
        	translateModel(0.01f, 0.0f, -0.01f);
        	break;
    }
}

//------------------------------------------------------------------------------

void close(void)
{
    // close haptic device
    navigator.closeHaptic();

    // delete resources
    delete hapticsThread;
    delete world;
}

//------------------------------------------------------------------------------

void mouseButtonCallback(GLFWwindow* a_window, int a_button, int a_action, int a_mods)
{
    if (a_button == GLFW_MOUSE_BUTTON_LEFT && a_action == GLFW_PRESS)
    {
        // store mouse position
        glfwGetCursorPos(window0, &mouseX, &mouseY);

        // update mouse state
        mouseState = MOUSE_ROTATE_CAMERA;
    }
    else if(a_button == GLFW_MOUSE_BUTTON_MIDDLE && a_action == GLFW_PRESS)
    {
        glfwGetCursorPos(window0, &mouseX, &mouseY);

        // update mouse state
        mouseState = MOUSE_TRANSLATE_CAMERA;
    }
    else
    {
        // update mouse state
        mouseState = MOUSE_IDLE;
    }
}

//------------------------------------------------------------------------------

void mouseMotionCallback(GLFWwindow* a_window, double a_posX, double a_posY)
{
    if (mouseState == MOUSE_ROTATE_CAMERA)
    {
        // compute mouse motion
        int dx = a_posX - mouseX;
        int dy = a_posY - mouseY;
        mouseX = a_posX;
        mouseY = a_posY;

        navigator.rotateCamera(dx, dy);
    }
    else if (mouseState == MOUSE_TRANSLATE_CAMERA)
    {
        // compute mouse motion
        int dx = a_posX - mouseX;
        int dy = a_posY - mouseY;
        mouseX = a_posX;
        mouseY = a_posY;

        navigator.translateCamera(dx, dy);
    }
}

//------------------------------------------------------------------------------

void mouseScrollCallback(GLFWwindow* a_window, double a_offsetX, double a_offsetY)
{
	navigator.updateRadius(a_offsetY);
}

//------------------------------------------------------------------------------
void updateGraphics0(void)
{
    /////////////////////////////////////////////////////////////////////
    // UPDATE WIDGETS
    /////////////////////////////////////////////////////////////////////

    // update haptic and graphic rate data
    labelRates->setText(cStr(freqCounterGraphics.getFrequency(), 0) + " Hz / " +
                        cStr(navigator.freqCounterHaptics.getFrequency(), 0) + " Hz");

    // update position of label
    labelRates->setLocalPos((int)(0.5 * (width0 - labelRates->getWidth())), 15);


    /////////////////////////////////////////////////////////////////////
    // RENDER SCENE
    /////////////////////////////////////////////////////////////////////
    deformModel.updateSkins(true);

    // update shadow maps (if any)
    world->updateShadowMaps(false, mirroredDisplay);


    navigator.cameraView(width0, height0);


    // wait until all GL commands are completed
    glFinish();

    // check for any OpenGL errors
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) cout << "Error: " << gluErrorString(err) << endl;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void updateGraphics1(void)
{

    /////////////////////////////////////////////////////////////////////
    // RENDER SCENE
    /////////////////////////////////////////////////////////////////////

    // update shadow maps (if any)
    world->updateShadowMaps(false, mirroredDisplay);


    navigator.endoscopeView(width1, height1);


    // wait until all GL commands are completed
    glFinish();

    // check for any OpenGL errors
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) cout << "Error: " << gluErrorString(err) << endl;
}
//----------------------------------------------------------------------------

void translateModel(float dx, float dy, float dz){	
    vStructures.translate(dx, dy, dz);
    deformModel.translate(dx, dy, dz);
}

void updateHaptics(void){
	navigator.updateHaptics();
}
