

//------------------------------------------------------------------------------
#include "chai3d.h"
//------------------------------------------------------------------------------
#include <GLFW/glfw3.h>
#include <fstream>
#include "GEL3D.h"
#include <math.h>
//------------------------------------------------------------------------------
using namespace chai3d;
using namespace std;
//------------------------------------------------------------------------------
#define PI 3.141592657
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
cStereoMode stereoMode = C_STEREO_DISABLED;

// fullscreen mode
bool fullscreen = false;

// mirrored display
bool mirroredDisplay = false;

bool switchtool = true;

bool perforate = true;
double rotate_x=0,rotate_y=0,rotate_z=0, rotate_val;

double skull_scale = 1.0;

bool rotateFlag = true;

cVector3d initialPos;
//cVector3d offsetPos(0.0853583, -0.86858, 0.513968);

//cVector3d virPoint1, virPoint2, virPoint3;
cVector3d realPoint1, realPoint2, realPoint3;

// realPoint1 = 0.000....;
// realPoint2 = 0.000....;
// realPoint3 = 0.000....;


cVector3d virPoint1(0.0324499, -0.030197, 0.0196008);
cVector3d virPoint2(0.00112148, -0.00480966, 0.0376946);
cVector3d virPoint3(-0.0271661, -0.0330365, 0.0254311);

enum MouseStates
{
    MOUSE_IDLE,
    MOUSE_ROTATE_CAMERA,
    MOUSE_TRANSLATE_CAMERA
};


void calibrate3d(cVector3d a, cVector3d b,  cVector3d c, cVector3d A, cVector3d B, cVector3d C);

//------------------------------------------------------------------------------
// DECLARED VARIABLES
//------------------------------------------------------------------------------

// a world that contains all objects of the virtual environment
cWorld* world;

// a camera to render the world in the window display
cCamera* camera;

// a camera attached to the endocope object
cCamera* cameraEndoScope;

// a virtual object
cMultiMesh* meshEndoscope;

// a light source
cPositionalLight *light;
cDirectionalLight *light_scope;

// a haptic device handler
cHapticDeviceHandler* handler;

// virtual drill mesh
cMultiMesh* drillTool;

// a pointer to the current haptic device
cGenericHapticDevicePtr hapticDevice;

// haptic device model
//cShapeSphere* device;
double deviceRadius;
//cToolCursor* _device;

// Updates on 26/2018
//double su = 1;

// objects in the scene
// deformable world
cGELWorld* defWorld;
cGELMesh* m_meshThirdVentricles;
cMultiMesh* m_meshVentricles;
cMultiMesh* m_meshChoroidPlexus;
cMultiMesh* m_meshThalastriateVein;
cMultiMesh* m_meshSkull;


cVoxelObject* m_voxelBrainSkull;
cMultiImagePtr m_imagesBrainSkull;
cTexture3dPtr m_textureBrainSkull;
cImagePtr    m_lutBrainSkull;

// force scale factor
double deviceForceScale;

// scale factor between the device workspace and cursor workspace
double workspaceScaleFactor;

// desired workspace radius of the virtual cursor
double cursorWorkspaceRadius;

// radius of the dynamic model sphere (GEM)
double radius;

// stiffness properties between the haptic device tool and the model (GEM)
double stiffness;

vector<cVector3d> dynamicNodes;
// dynamic nodes
cGELSkeletonNode* nodes[10][10];

// a colored background
cBackground* background;

// a font for rendering text
cFontPtr font;

// a label to display the rate [Hz] at which the simulation is running
cLabel* labelRates;

// a flag that indicates if the haptic simulation is currently running
bool simulationRunning = false;

// a flag that indicates if the haptic simulation has terminated
bool simulationFinished = true;

// a frequency counter to measure the simulation graphic rate
cFrequencyCounter freqCounterGraphics;

// a frequency counter to measure the simulation haptic rate
cFrequencyCounter freqCounterHaptics;

// haptic thread
cThread* hapticsThread;

// a handle to window display context
GLFWwindow* window0 = NULL;
int width0  = 0;
int height0 = 0;

// a second window
GLFWwindow* window1 = NULL;
int width1 = 0;
int height1 = 0;

cNormalMapPtr normalMap_test;

vector<cShapeSphere*> spheres;

// swap interval for the display context (vertical synchronization)
int swapInterval = 1;


float voxelBrainSkullOpacityThreshold = 1.0;
// mouse state
MouseStates mouseState = MOUSE_IDLE;

// last mouse position
double mouseX, mouseY;

//------------------------------------------------------------------------------
// DECLARED MACROS
//------------------------------------------------------------------------------
// convert to resource path
#define RESOURCE_PATH(p)    (char*)((resourceRoot+string(p)).c_str())


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

// this function contains the main haptics simulation loop
void updateHaptics(void);

// this function closes the application
void close(void);

// compute forces between tool and environment
cVector3d computeForce(const cVector3d& a_cursor,
                       double a_cursorRadius,
                       const cVector3d& a_spherePos,
                       double a_radius,
                       double a_stiffness);

// Build deformable model of huge cell
void BuildDynamicModel();
void renderSpheres();


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

    // parse first arg to try and locate resources
    string resourceRoot = string(argv[0]).substr(0,string(argv[0]).find_last_of("/\\")+1);


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
    if (stereoMode == C_STEREO_ACTIVE)
    {
        glfwWindowHint(GLFW_STEREO, GL_TRUE);
    }
    else
    {
        glfwWindowHint(GLFW_STEREO, GL_FALSE);
    }

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
    // read the CSV file
    //--------------------------------------------------------------------------

    std::ifstream fin("/home/vrl3/VR-Simulation/Data/mesh_models/sampled-points.obj");  //ifstream to read from
    std::string linestr;
    double x_1,y_1,z_1;
    while ( std::getline(fin, linestr) ) // first read
    {
        std::stringstream ss(linestr);
        ss>>x_1;
        ss>>y_1;
        ss>>z_1;
        dynamicNodes.push_back(cVector3d(x_1,y_1,z_1));
    }
    for(int i = 0 ; i < dynamicNodes.size(); i++)
    {
        dynamicNodes[i] = cVector3d(dynamicNodes[i].x()/235.0, dynamicNodes[i].y()/235.0, dynamicNodes[i].z()/235.0);
    }
    //**********************************************************************************************************


    //--------------------------------------------------------------------------
    // WORLD - CAMERA - LIGHTING
    //--------------------------------------------------------------------------

    // create a new world.
    world = new cWorld();

    // set the background color of the environment
    // the color is defined by its (R,G,B) components.
    world->m_backgroundColor.setBlueAqua();

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
    camera->setStereoMode(stereoMode);

    // set stereo eye separation and focal length (applies only if stereo is enabled)
    camera->setStereoEyeSeparation(0.0);//0.03
    camera->setStereoFocalLength(3.0);

    // set vertical mirrored display mode
    camera->setMirrorVertical(mirroredDisplay);

    // set camera field of view
    camera->setFieldViewAngleDeg(60);


    camera->setSphericalAzimuthDeg(-37.5);
    camera->setSphericalPolarDeg(18);
    camera->setSphericalRadius(2.0);


    //--------------------------------------------------------------------------
    // HAPTIC DEVICES / TOOLS
    //--------------------------------------------------------------------------

    // create a haptic device handler
    handler = new cHapticDeviceHandler();

    // get access to the first available haptic device found
    handler->getDevice(hapticDevice, 0);

    // retrieve information about the current haptic device
    cHapticDeviceInfo hapticDeviceInfo = hapticDevice->getSpecifications();

    // open connection to haptic device
    hapticDevice->open();

    hapticDevice->setEnableGripperUserSwitch(true);
    // desired workspace radius of the cursor
    cursorWorkspaceRadius = 1.0;

    // read the scale factor between the physical workspace of the haptic
    // device and the virtual workspace defined for the tool
    workspaceScaleFactor = cursorWorkspaceRadius / hapticDeviceInfo.m_workspaceRadius;


    // stiffness property
    double maxStiffness = hapticDeviceInfo.m_maxLinearStiffness / workspaceScaleFactor;

    // define a scale factor between the force perceived at the cursor and the
    // forces actually sent to the haptic device
    deviceForceScale = 0.15 * hapticDeviceInfo.m_maxLinearForce;

    // create a large sphere that represents the haptic device
    deviceRadius = 0.01;
    //device = new cShapeSphere(deviceRadius);
    //world->addChild(device);
    //device->m_material->m_ambient.set(1.0f, 0.4f, 0.4f, 0.5f);
    //device->m_material->m_diffuse.set(1.0f, 0.7f, 0.7f, 0.5f);
    // device->m_material->m_specular.set(1.0f, 1.0f, 1.0f, 0.5f);
    //device->m_material->setShininess(0);
    //device->setUseTransparency(true);
    //device->setShowEnabled(false);

//    _device = new cToolCursor(world);
//    world->addChild(_device);
//    _device->setHapticDevice(hapticDevice);
//    _device->setRadius(0.03);
//    _device->setWorkspaceRadius(1.0);
//    _device->enableDynamicObjects(true);
//    _device->setWaitForSmallForce(true);
//    //_device->setLocalPos(cVector3d(-0.001, 0, 0.001));
//    _device->start();

    //_device->setShowFrame(true);


    // interaction stiffness between tool and deformable model
    stiffness = 20;



    //--------------------------------------------------------------------------
    // CREATE Deformable Third Ventricle
    //--------------------------------------------------------------------------
    defWorld = new cGELWorld();
    world->addChild(defWorld);
    m_meshThirdVentricles = new cGELMesh();
    defWorld->m_gelMeshes.push_front(m_meshThirdVentricles);

    bool fileload = m_meshThirdVentricles->loadFromFile("/home/vrl3/VR-Simulation/Data/mesh_models/Third-Ventricle.obj");
    if (!fileload)
    {
        printf("Error - 3D Model failed to load correctly.\n");
        close();
        return (-1);
    }
    m_meshThirdVentricles->scaleXYZ(1 /235.0, 1 / 235.0, 1 / 235.0);
    m_meshThirdVentricles->m_meshes->at(0)->m_material->setWhite();
    m_meshThirdVentricles->m_meshes->at(0)->m_material->setShininess(100);
    m_meshThirdVentricles->m_meshes->at(0)->m_material->setTextureLevel(0.9);
    m_meshThirdVentricles->setLocalPos(cVector3d(0.19, -0.04, -0.24));
    cNormalMapPtr normalMap_ThirdVentricle = cNormalMap::create();
    //fileload = normalMap_ThirdVentricle->loadFromFile("/home/terminalx/vinkle/ETV-Simulation-Project/Data/mesh_models/ventricle_texture_NORM.png");
    //m_meshThirdVentricles->m_meshes->at(0)->m_normalMap = normalMap_ThirdVentricle;
    m_meshThirdVentricles->computeBTN();
    m_meshThirdVentricles->computeBoundaryBox(true);

    // setup default values for nodes
    cGELSkeletonNode::s_default_radius        = 0.04;//3;
    cGELSkeletonNode::s_default_kDampingPos   = 10;
    cGELSkeletonNode::s_default_kDampingRot   = 0.1;
    cGELSkeletonNode::s_default_mass          = 0.30;  // [kg]
    cGELSkeletonNode::s_default_showFrame     = false;
    cGELSkeletonNode::s_default_color.set(0.6, 0.6, 0.0);
    cGELSkeletonNode::s_default_useGravity      = false;
    cGELSkeletonNode::s_default_gravity.set(0.00, 0.00, -0.10);
    radius = cGELSkeletonNode::s_default_radius;

    // setup default values for links
    cGELSkeletonLink::s_default_kSpringElongation = 1000; // [N/m]
    cGELSkeletonLink::s_default_kSpringFlexion    = 0.1;   // [Nm/RAD]
    cGELSkeletonLink::s_default_kSpringTorsion    = 0.1;   // [Nm/RAD]
    cGELSkeletonLink::s_default_color.set(0.2, 0.2, 1.0);

    // build dynamic vertices
    m_meshThirdVentricles->buildVertices();

    // create dynamic model (GEM)
    BuildDynamicModel();

    // connect skin to skeleton
    m_meshThirdVentricles->connectVerticesToSkeleton(true);

    // show/hide underlying dynamic skeleton model
    m_meshThirdVentricles->m_showSkeletonModel = false;

    // use internal skeleton as deformable model
    m_meshThirdVentricles->m_useSkeletonModel = true;

    // create anchors
    cGELSkeletonLink::s_default_kSpringElongation = 5.0; // [N/m]
    list<cGELSkeletonNode*>::iterator i;
    int num = 0;
    for(i = m_meshThirdVentricles->m_nodes.begin(); i != m_meshThirdVentricles->m_nodes.end(); ++i)
    {
        num++;
    }

    int counter1 = 0;
    int counter2 = 0;
    for(i = m_meshThirdVentricles->m_nodes.begin(); i != m_meshThirdVentricles->m_nodes.end(); ++i)
    {
        if (counter1 <= num)
        {
            if (counter2 > 3)
            {
                cGELSkeletonNode* nextItem = *i;
                cGELSkeletonNode* newNode = new cGELSkeletonNode();
                newNode->m_fixed = true;
                newNode->m_pos = nextItem->m_pos;
                cGELSkeletonLink* newLink = new cGELSkeletonLink(nextItem, newNode); m_meshThirdVentricles->m_links.push_front(newLink);
                newLink->m_kSpringElongation = 5;
                counter2 = 0;
            }
            counter2++;
            counter1++;
        }
    }
    //renderSpheres();

    // ************************************************************* Add the other ventricular structures *********************************************
    // 1. Ventricles
    m_meshVentricles = new cMultiMesh();
    world->addChild(m_meshVentricles);
    m_meshVentricles->setUseCulling(true, true);
    fileload = m_meshVentricles->loadFromFile("/home/vrl3/VR-Simulation/Data/mesh_models/Ventricles.obj");
    if (!fileload)
    {
        printf("Error - 3D Lateral Model failed to load correctly.\n");
        close();
        return (-1);
    }
    m_meshVentricles->scaleXYZ(1 /235.0, 1 / 235.0, 1 / 235.0);
    // compute a boundary box
    m_meshVentricles->computeBoundaryBox(true);
    m_meshVentricles->m_meshes->at(0)->m_material->setWhite();
    m_meshVentricles->m_meshes->at(0)->m_material->setShininess(100);
    m_meshVentricles->m_meshes->at(0)->m_material->setTextureLevel(0.9);
    cNormalMapPtr normalMap_LateralVentricle = cNormalMap::create();
    fileload = normalMap_LateralVentricle->loadFromFile("/home/vrl3/VR-Simulation/Data/mesh_models/ventricle_texture_NORM.png");
    m_meshVentricles->m_meshes->at(0)->m_normalMap = normalMap_LateralVentricle;
    m_meshVentricles->computeBTN();
    m_meshVentricles->setLocalPos(cVector3d(0.19, -0.04, -0.24));


    // 2. Thalamostriate_Vein
    m_meshThalastriateVein = new cMultiMesh();
    world->addChild(m_meshThalastriateVein);
    m_meshThalastriateVein->setUseCulling(true, true);
    fileload = m_meshThalastriateVein->loadFromFile("/home/vrl3/VR-Simulation/Data/mesh_models/Thalamostriate_Vein.obj");
    if (!fileload)
    {
        printf("Error - 3D Thalamostriate_Vein Model failed to load correctly.\n");
        close();
        return (-1);
    }
    m_meshThalastriateVein->scaleXYZ(1 /235.0, 1 / 235.0, 1 / 235.0);
    // compute a boundary box
    m_meshThalastriateVein->computeBoundaryBox(true);
    m_meshThalastriateVein->m_meshes->at(0)->setUseTexture(false);
    m_meshThalastriateVein->m_meshes->at(0)->m_material->setRedDark();
    m_meshThalastriateVein->m_meshes->at(0)->m_material->setShininess(100);
    m_meshThalastriateVein->m_meshes->at(0)->m_material->setTextureLevel(0.9);
    m_meshThalastriateVein->computeBTN();
    m_meshThalastriateVein->getMesh(0)->setUseTransparency(true);
    m_meshThalastriateVein->getMesh(0)->setTransparencyLevel(0.8);
    m_meshThalastriateVein->setLocalPos(cVector3d(0.19, -0.04, -0.24));

    // 3. Choroid Plexus
    m_meshChoroidPlexus = new cMultiMesh();
    world->addChild(m_meshChoroidPlexus);
    m_meshChoroidPlexus->setUseCulling(true, true);
    fileload = m_meshChoroidPlexus->loadFromFile("/home/vrl3/VR-Simulation/Data/mesh_models/Choroid_Plexus.obj");
    if (!fileload)
    {
        printf("Error - 3D Choroid_Plexus Model failed to load correctly.\n");
        close();
        return (-1);
    }
    m_meshChoroidPlexus->scaleXYZ(1 /235.0, 1 / 235.0, 1 / 235.0);
    // compute a boundary box
    m_meshChoroidPlexus->computeBoundaryBox(true);
    m_meshChoroidPlexus->m_meshes->at(0)->m_material->setWhite();
    m_meshChoroidPlexus->m_meshes->at(0)->m_material->setShininess(100);
    m_meshChoroidPlexus->m_meshes->at(0)->m_material->setTextureLevel(0.9);
    m_meshChoroidPlexus->computeBTN();
    m_meshChoroidPlexus->setLocalPos(cVector3d(0.19, -0.04, -0.24));
    //m_meshChoroidPlexus->getMesh(0)->setUseTransparency(true);
    //m_meshChoroidPlexus->getMesh(0)->setTransparencyLevel(0.6);


    // compute collision detection algorithm
    //m_meshVentricles->createAABBCollisionDetector(0.01);

    // define a default stiffness for the object
    //m_meshVentricles->setStiffness(0.8 * maxStiffness, true);

    //m_meshVentricles->m_meshes->at(0)->setUseTexture(false);
    //m_meshVentricles->m_meshes->at(0)->m_material->setRedSalmon();
    //m_meshVentricles->m_meshes->at(0)->m_material->setShininess(100);
    //m_meshVentricles->m_meshes->at(0)->m_material->setTextureLevel(0.9);

    //m_meshVentricles->m_meshes->at(1)->m_material->setRedDark();
    //m_meshVentricles->m_meshes->at(1)->m_material->setShininess(100);
    //m_meshVentricles->m_meshes->at(1)->m_material->setTextureLevel(0.9);




    // make the outside of the tooth rendered in semi-transparent
    // m_meshVentricles->getMesh(0)->setUseTransparency(true);
    // m_meshVentricles->getMesh(0)->setTransparencyLevel(0.9);
    // m_meshVentricles->getMesh(1)->setUseTransparency(true);
    // m_meshVentricles->getMesh(1)->setTransparencyLevel(0.9);




    // **************************************************  Add the shaders and use the meshes for the rendering **************************************
    // CREATE SHADERS
    // create vertex shader
    cShaderPtr vertexShader = cShader::create(C_VERTEX_SHADER);
    fileload = vertexShader->loadSourceFile("/home/vrl3/VR-Simulation/Data/shaders/bump.vert");

    // create fragment shader
    cShaderPtr fragmentShader = cShader::create(C_FRAGMENT_SHADER);
    fileload = fragmentShader->loadSourceFile("/home/vrl3/VR-Simulation/Data/shaders/bump.frag");

    // create program shader
    cShaderProgramPtr programShader = cShaderProgram::create();

    // assign vertex shader to program shader
    programShader->attachShader(vertexShader);

    // assign fragment shader to program shader
    programShader->attachShader(fragmentShader);

    // assign program shader to object
    //m_meshThirdVentricles->setShaderProgram(programShader);
    m_meshVentricles->setShaderProgram(programShader);

    // link program shader
    programShader->linkProgram();



    // set uniforms
    programShader->setUniformi("uColorMap", 0);
    programShader->setUniformi("uShadowMap", 0);
    programShader->setUniformi("uNormalMap", 2);
    programShader->setUniformf("uInvRadius", 0.1f);

    // *************************************** Add the volume and skull model *************************************************************************
    m_voxelBrainSkull = new cVoxelObject();
    // m_voxelBrainSkull->setLocalPos(0.0, 0.0, 0.0);
    m_voxelBrainSkull->setLocalPos(0.0, -0.1, 0.2);
    m_voxelBrainSkull->m_minCorner.set(-0.5, -0.5, -0.5);
    m_voxelBrainSkull->m_maxCorner.set(0.5, 0.220703125, 0.5);
    m_voxelBrainSkull->m_minTextureCoord.set(0.0, 0.0, 0.0);
    m_voxelBrainSkull->m_maxTextureCoord.set(1.0, 1.0, 1.0);
    // set haptic properties
    //m_voxelBrainSkull->m_material->setStiffness(0.6 * maxStiffness);
    //m_voxelBrainSkull->m_material->setStaticFriction(0.4);
    //m_voxelBrainSkull->m_material->setDynamicFriction(0.4);
    // enable materials
    m_voxelBrainSkull->setUseMaterial(true);
    // set material
    m_voxelBrainSkull->m_material->setWhite();
    // set quality of graphic rendering
    m_voxelBrainSkull->setQuality(1.0);
    // set graphic rendering mode
    //m_voxelBrainSkull->setRenderingModeIsosurfaceColorMap();   // medium quality
    m_voxelBrainSkull->setRenderingModeDVRColorMap();      // high quality


    m_imagesBrainSkull = cMultiImage::create();
    m_textureBrainSkull = cTexture3d::create();
    m_lutBrainSkull = cImage::create();

    //!
    //! Load the data
    //int filesloaded = m_imagesBrainSkull->loadFromFiles("/home/vrl3/VR-Simulation/Data/skull/png/brain-0", "png", 512);
    int filesloaded = m_imagesBrainSkull->loadFromFiles("/home/vrl3/VR-Simulation/Data/Data_Model_printed/png/000", "png", 220);
	if (filesloaded == 0)
    {
        cout << "Failed to load volume data.Make sure folder contains files in the form 0XXX.png" << endl;
        close();
    }
    // assign volumetric image to texture
    m_textureBrainSkull->setImage(m_imagesBrainSkull);
    // assign texture to voxel object
    m_voxelBrainSkull->setTexture(m_textureBrainSkull);
    // initially select an isosurface corresponding to the bone/heart level
    m_voxelBrainSkull->setIsosurfaceValue(0.1f);
    m_voxelBrainSkull->setOpacityThreshold(voxelBrainSkullOpacityThreshold);
    // set optical density factor
    m_voxelBrainSkull->setOpticalDensity(1.2f);
    //bool fileLoaded = m_lutColorMap->loadFromFile("D:/data/colormap/colormap.png");
    bool fileLoaded = m_lutBrainSkull->loadFromFile("/home/vrl3/VR-Simulation/Data/colormap/colormap_out.png");
    if (!fileLoaded)
    {
        cout << "Failed to load colormap.Make sure folder contains colormap.png" << endl;
        close();
    }
    m_voxelBrainSkull->m_colorMap->setImage(m_lutBrainSkull);
    // set haptic properties
    m_voxelBrainSkull->m_material->setStiffness(0.2 * maxStiffness);
    m_voxelBrainSkull->m_material->setStaticFriction(0.0);
    m_voxelBrainSkull->m_material->setDynamicFriction(0.0);
    m_voxelBrainSkull->rotateAboutLocalAxisDeg(1, 0, 0, 90);
    //m_voxelBrainSkull->createAABBCollisionDetector(0.01);
    m_meshSkull = new cMultiMesh();
    m_voxelBrainSkull -> polygonize 	( 	m_meshSkull,-1.0,-1.0,-1.0);
    world->addChild(m_meshSkull);
    // m_meshSkull->setUseCulling(true, true);

    // m_meshSkull->scaleXYZ(1 /235.0, 1 / 235.0, 1 / 235.0);
    // compute a boundary box
    // m_meshSkull->computeBoundaryBox(true);
    // m_meshSkull->m_meshes->at(0)->m_material->setWhite();
    // m_meshSkull->m_meshes->at(0)->m_material->setShininess(100);
    // m_meshSkull->m_meshes->at(0)->m_material->setTextureLevel(0.9);
    // cNormalMapPtr normalMap_LateralVentricle = cNormalMap::create();
    // fileload = normalMap_LateralVentricle->loadFromFile("/home/vrl3/VR-Simulation/Data/mesh_models/ventricle_texture_NORM.png");
    // m_meshVentricles->m_meshes->at(0)->m_normalMap = normalMap_LateralVentricle;
    // m_meshSkull->computeBTN();
    // m_meshSkull->setLocalPos(cVector3d(0.19, -0.04, -0.24));
    // ****************************************************************************************************************************************


    // ********************************************  Endoscopic Camera ***********************************************************
    // create a virtual mesh
    meshEndoscope = new cMultiMesh();
    world->addChild(meshEndoscope);

	initialPos = meshEndoscope->getLocalPos();
	//cout<<"\n****************************************\n";
	//cout<<initialPos<<endl;
	//cout<<offsetPos<<endl;
	//offsetPos.sub(initialPos);
	//cout<<initialPos<<endl;
	//cout<<offsetPos<<endl;
	//cout<<"\n****************************************\n";

    // load an object file
    fileload = meshEndoscope->loadFromFile("/home/vrl3/VR-Simulation/Data/endoscope.3ds");
    if (!fileload)
    {
        cout << "Error - 3D Model Endoscope failed to load correctly." << endl;
        close();
        return (-1);
    }

    // disable culling so that faces are rendered on both sides
    meshEndoscope->setUseCulling(false);

    // scale model
    meshEndoscope->scale(0.1);

    // use display list for faster rendering
    meshEndoscope->setUseDisplayList(true);


    // position object in scene
    meshEndoscope->rotateAboutGlobalAxisDeg(0, 0, 0, C_EULER_ORDER_XYZ);
	cVector3d z_axis_1(0.0,0.0,1.0);
	meshEndoscope->rotateAboutGlobalAxisDeg(z_axis_1,-90.0);

    cameraEndoScope = new cCamera(world);
    meshEndoscope->addChild(cameraEndoScope);

    // set the near and far clipping planes of the camera
    // anything in front or behind these clipping planes will not be rendered
    cameraEndoScope->setClippingPlanes(0.001, 100);


    //meshEndoscope->setShowFrame(true);
    cameraEndoScope->setLocalPos(cVector3d(0.0, 0, 0.0));//(-0.001,0.0,0.001)
    //**cVector3d pos = meshEndoscope->getLocalPos();**//

	//**cameraEndoScope->setLocalPos(pos);**//
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

    // create a light source
    //    light = new cPositionalLight(world);

    //    // add light to world
    //    world->addChild(light);

    //    // enable light source
    //    light->setEnabled(true);
    //    //light->setDir(0.00570787, -0.076876, -1);
    //    light->setLocalPos(0.00570787, -0.076876, -0.0044498);

    //    // define the direction of the light beam
    //    //light->setDir();

    //    // set lighting conditions
    //    light->m_ambient.set(0.5f, 0.5f, 0.5f);
    //    light->m_diffuse.set(0.8f, 0.8f, 0.8f);
    //    light->m_specular.set(1.0f, 1.0f, 1.0f);



    // create a new mesh.
    drillTool = new cMultiMesh();
    //world->addChild(drillTool);
    // load a drill like mesh and attach it to the tool
    fileload = drillTool->loadFromFile("/home/vrl3/VR-Simulation/Data/drill.3ds");
    if (!fileload)
    {
        printf("Error - 3D Model failed to load correctly.\n");
        close();
        return (-1);
    }

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


    //--------------------------------------------------------------------------
    // WIDGETS
    //--------------------------------------------------------------------------

    // create a font
    font = NEW_CFONTCALIBRI20();

    // create a label to display the haptic and graphic rate of the simulation
    labelRates = new cLabel(font);
    camera->m_frontLayer->addChild(labelRates);

    // set font color
    labelRates->m_fontColor.setWhite();

    // create a background
    background = new cBackground();
    camera->m_backLayer->addChild(background);

    // set background properties
    fileload = background->loadFromFile("/home/vrl3/VR-Simulation/Data/Craniotomy_and_Trocar_placement_imgs/img_background.jpg");
    if (!fileload)
    {
        cout << "Error - Background Image failed to load correctly." << endl;
        close();
    }
    cBackground* frontground = new cBackground();
    cameraEndoScope->m_frontLayer->addChild(frontground);

    // load an texture map
    fileload = frontground->loadFromFile("/home/vrl3/VR-Simulation/Data/scope.png");
    if (!fileload)
    {
        cout << "Error - Image failed to load correctly." << endl;
        close();
        return (-1);
    }

    //--------------------------------------------------------------------------
    // START SIMULATION
    //--------------------------------------------------------------------------
    // create a thread which starts the main haptics rendering loop

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
        // RENDER WINDOW 1m_voxelBrainSkull->scaleXYZ(scaling_factor,scaling_factor,scaling_factor);
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

	/*
    // option - exit
    else if ((a_key == GLFW_KEY_ESCAPE) || (a_key == GLFW_KEY_Q))
    {
        glfwSetWindowShouldClose(a_window, GLFW_TRUE);
    }

    // option - toggle fullscreen
    else if (a_key == GLFW_KEY_F)
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

    // option - toggle vertical mirroring
    else if (a_key == GLFW_KEY_M)
    {
        mirroredDisplay = !mirroredDisplay;
        camera->setMirrorVertical(mirroredDisplay);
    }
    else if (a_key == GLFW_KEY_1)
    {
        voxelBrainSkullOpacityThreshold -= 0.03;
        m_voxelBrainSkull->setOpacityThreshold(voxelBrainSkullOpacityThrm_voxelBrainSkull->scaleXYZ(scaling_factor,scaling_factor,scaling_factor);eshold);
    }
    else if (a_key == GLFW_KEY_2)
    {
        voxelBrainSkullOpacityThreshold += 0.03;
        m_voxelBrainSkull->setOpacityThreshold(voxelBrainSkullOpacityThreshold);
    }
    else if (a_key == GLFW_KEY_3)
    {
        world->addChild(m_voxelBrainSkull);
    }
	*/
    else if (a_key == GLFW_KEY_4)
    {
        world->removeChild(m_meshSkull);
    }

    else if (a_key == GLFW_KEY_5)
    {
        world->removeChild(m_meshSkull);
        world->addChild(defWorld);
        world->addChild(m_meshSkull);
    }
    else if (a_key == GLFW_KEY_6)
    {
        world->removeChild(defWorld);
    }
    else if(a_key == GLFW_KEY_S)
    {
        switchtool = !switchtool;
        if(switchtool)
        {
            world->addChild(meshEndoscope);
            //*world->addChild(cameraEndoScope);//**
            world->removeChild(drillTool);
        }
        else
        {
            world->addChild(drillTool);
            //*world->removeChild(meshEndoscope);
            world->removeChild(cameraEndoScope);
        }
    }
    else if (a_key == GLFW_KEY_P)
    {
        perforate = !perforate;
    }
    else if (a_key == GLFW_KEY_RIGHT)
    {
        cVector3d d = m_meshVentricles->getLocalPos();
        d = cVector3d(d.x() + 0.01, d.y(), d.z());
        m_meshVentricles->setLocalPos(d);

        d = m_meshThirdVentricles->getLocalPos();
        d = cVector3d(d.x() + 0.01, d.y(), d.z());
        m_meshThirdVentricles->setLocalPos(d);

        d = m_meshThalastriateVein->getLocalPos();
        d = cVector3d(d.x() + 0.01, d.y(), d.z());
        m_meshThalastriateVein->setLocalPos(d);


        d = m_meshChoroidPlexus->getLocalPos();
        d = cVector3d(d.x() + 0.01, d.y(), d.z());
        m_meshChoroidPlexus->setLocalPos(d);

        d = m_meshSkull->getLocalPos();
        d = cVector3d(d.x() + 0.01, d.y(), d.z());
        //m_meshSkull->setLocalPos(d);
        cout << "d " << d << endl;
    }
    else if (a_key == GLFW_KEY_LEFT)
    {
        cVector3d d = m_meshVentricles->getLocalPos();
        d = cVector3d(d.x() - 0.01, d.y(), d.z());
        m_meshVentricles->setLocalPos(d);

        d = m_meshThirdVentricles->getLocalPos();
        d = cVector3d(d.x() - 0.01, d.y(), d.z());
        m_meshThirdVentricles->setLocalPos(d);

        d = m_meshThalastriateVein->getLocalPos();
        d = cVector3d(d.x() - 0.01, d.y(), d.z());
        m_meshThalastriateVein->setLocalPos(d);


        d = m_meshChoroidPlexus->getLocalPos();
        d = cVector3d(d.x() - 0.01, d.y(), d.z());
        m_meshChoroidPlexus->setLocalPos(d);

        d = m_meshSkull->getLocalPos();
        d = cVector3d(d.x() - 0.01, d.y(), d.z());
        //m_meshSkull->setLocalPos(d);
        cout << "d " << d << endl;
    }

    else if (a_key == GLFW_KEY_UP)
    {
        cVector3d d = m_meshVentricles->getLocalPos();
        d = cVector3d(d.x() , d.y() + 0.01, d.z());
        m_meshVentricles->setLocalPos(d);

        d = m_meshThirdVentricles->getLocalPos();
        d = cVector3d(d.x() , d.y() + 0.01, d.z());
        m_meshThirdVentricles->setLocalPos(d);

        d = m_meshThalastriateVein->getLocalPos();
        d = cVector3d(d.x(), d.y() + 0.01, d.z());
        m_meshThalastriateVein->setLocalPos(d);


        d = m_meshChoroidPlexus->getLocalPos();
        d = cVector3d(d.x() , d.y() + 0.01, d.z());
        m_meshChoroidPlexus->setLocalPos(d);

        d = m_meshSkull->getLocalPos();
        d = cVector3d(d.x() , d.y() + 0.01, d.z());
        //m_meshSkull->setLocalPos(d);
        cout << "d " << d << endl;
    }
    else if (a_key == GLFW_KEY_DOWN)
    {
        cVector3d d = m_meshVentricles->getLocalPos();
        d = cVector3d(d.x() , d.y() - 0.01, d.z());
        m_meshVentricles->setLocalPos(d);

        d = m_meshThirdVentricles->getLocalPos();
        d = cVector3d(d.x() , d.y() - 0.01, d.z());
        m_meshThirdVentricles->setLocalPos(d);

        d = m_meshThalastriateVein->getLocalPos();
        d = cVector3d(d.x() , d.y() - 0.01, d.z());
        m_meshThalastriateVein->setLocalPos(d);


        d = m_meshChoroidPlexus->getLocalPos();
        d = cVector3d(d.x() , d.y() - 0.01, d.z());
        m_meshChoroidPlexus->setLocalPos(d);

        d = m_meshSkull->getLocalPos();
        d = cVector3d(d.x() , d.y() - 0.01, d.z());
        //m_meshSkull->setLocalPos(d);
        cout << "d " << d << endl;
    }

    else if (a_key == GLFW_KEY_PAGE_UP)
    {
        cVector3d d = m_meshVentricles->getLocalPos();
        d = cVector3d(d.x() , d.y() , d.z() + 0.01);
        m_meshVentricles->setLocalPos(d);

        d = m_meshThirdVentricles->getLocalPos();
        d = cVector3d(d.x() , d.y() , d.z() + 0.01);
        m_meshThirdVentricles->setLocalPos(d);

        d = m_meshThalastriateVein->getLocalPos();
        d = cVector3d(d.x(), d.y() , d.z() + 0.01);
        m_meshThalastriateVein->setLocalPos(d);


        d = m_meshChoroidPlexus->getLocalPos();
        d = cVector3d(d.x() , d.y() , d.z() + 0.01);
        m_meshChoroidPlexus->setLocalPos(d);

        d = m_meshSkull->getLocalPos();
        d = cVector3d(d.x() , d.y() , d.z() + 0.01);
        //m_meshSkull->setLocalPos(d);

        cout << "d " << d << endl;
    }
    else if (a_key == GLFW_KEY_PAGE_DOWN)
    {
        cVector3d d = m_meshVentricles->getLocalPos();
        d = cVector3d(d.x() , d.y() , d.z() - 0.01);
        m_meshVentricles->setLocalPos(d);

        d = m_meshThirdVentricles->getLocalPos();
        d = cVector3d(d.x() , d.y() , d.z() - 0.01);
        m_meshThirdVentricles->setLocalPos(d);

        d = m_meshThalastriateVein->getLocalPos();
        d = cVector3d(d.x() , d.y() , d.z() - 0.01);
        m_meshThalastriateVein->setLocalPos(d);


        d = m_meshChoroidPlexus->getLocalPos();
        d = cVector3d(d.x() , d.y() , d.z() - 0.01);
        m_meshChoroidPlexus->setLocalPos(d);

        d = m_meshSkull->getLocalPos();
        d = cVector3d(d.x() , d.y() , d.z() - 0.01);
        //m_meshSkull->setLocalPos(d);
        cout << "d " << d << endl;
    }


	// checking rotation //Aditya
	else if (a_key == GLFW_KEY_R)
	{
    rotate_z = 0;
		rotate_x = 1;
		rotate_y = 0;
    rotate_val = 5;
		m_meshVentricles->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
		m_meshThirdVentricles->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
		m_meshThalastriateVein->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
    m_meshSkull -> rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
    m_meshChoroidPlexus->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
	}
		else if (a_key == GLFW_KEY_T)
	{
    rotate_z = 0;
		rotate_x = 1;
		rotate_y = 0;
    rotate_val = -5;
		// rotate_x = rotate_x-5;
		m_meshVentricles->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
		m_meshThirdVentricles->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
		m_meshThalastriateVein->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
    m_meshSkull -> rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
    m_meshChoroidPlexus->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
	}
		else if (a_key == GLFW_KEY_Y)
	{
    rotate_z = 0;
		rotate_x = 0;
		rotate_y = 1;
    rotate_val = 5;
		m_meshVentricles->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
		m_meshThirdVentricles->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
		m_meshThalastriateVein->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
		m_meshChoroidPlexus->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
    m_meshSkull -> rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
	}
		else if (a_key == GLFW_KEY_U)
	{
    rotate_z = 0;
    rotate_x = 0;
    rotate_y = 1;
    rotate_val = -5;
    m_meshVentricles->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
		m_meshThirdVentricles->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
		m_meshThalastriateVein->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
		m_meshChoroidPlexus->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
    m_meshSkull -> rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
	}
		else if (a_key == GLFW_KEY_I)
	{
    rotate_z = 1;
		rotate_x = 0;
		rotate_y = 0;
    rotate_val = 5;
		m_meshVentricles->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
		m_meshThirdVentricles->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
		m_meshThalastriateVein->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
		m_meshChoroidPlexus->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
    m_meshSkull -> rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
	}
	else if (a_key == GLFW_KEY_O)
	{
    rotate_z = 1;
		rotate_x = 0;
		rotate_y = 0;
    rotate_val = -5;
		m_meshVentricles->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
		m_meshThirdVentricles->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
		m_meshThalastriateVein->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
		m_meshChoroidPlexus->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
    m_meshSkull -> rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
	}
	// 	else if (a_key == GLFW_KEY_E)
	// {
	// 	rotate_z = 0;
	// 	rotate_x = 0;
	// 	rotate_y = 0;
	// // 	m_meshVentricles->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
	// // 	m_meshThirdVentricles->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
	// // 	m_meshThalastriateVein->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
	// // 	m_meshChoroidPlexus->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
	// }
	else if (a_key == GLFW_KEY_L)
	{
		cVector3d pos;
    cMatrix3d m1,m2,m3,m4;
		// hapticDevice->getPosition(pos);
    pos = m_meshVentricles->getLocalPos();
		cout<<"position :"<<pos<<endl;
    // cout<<m_meshVentricles->getLocalPos()<<endl;
    // cout<<m_meshThirdVentricles->getLocalPos()<<endl;
    // cout<<m_meshThalastriateVein->getLocalPos()<<endl;
    // cout<<m_meshChoroidPlexus->getLocalPos()<<endl;
    m1=m_meshVentricles->getLocalRot();
    cout<<"matrix1 : "<<endl<<m1.getRow(0)<<endl<<m1.getRow(1)<<endl<<m1.getRow(2)<<endl;
    m1=m_meshThirdVentricles->getLocalRot();
    cout<<"matrix1 : "<<endl<<m1.getRow(0)<<endl<<m1.getRow(1)<<endl<<m1.getRow(2)<<endl;
    m1=m_meshThalastriateVein->getLocalRot();
    cout<<"matrix1 : "<<endl<<m1.getRow(0)<<endl<<m1.getRow(1)<<endl<<m1.getRow(2)<<endl;
    m1=m_meshChoroidPlexus->getLocalRot();
    cout<<"matrix1 : "<<endl<<m1.getRow(0)<<endl<<m1.getRow(1)<<endl<<m1.getRow(2)<<endl;
    // cout<<m_meshThirdVentricles->getLocalRot()<<endl<<endl;
    // cout<<m_meshThalastriateVeinVentricles->getLocalRot()<<endl<<endl;
    // cout<<m_meshChoroidPlexus->getLocalRot()<<endl<<endl;
	}
  // else if (a_key == GLFW_KEY_7)
	// {
	// 	skull_scale += 0.5;
  //   m_meshSkull->scaleXYZ(skull_scale,skull_scale,skull_scale);
  //   m_meshVentricles->scaleXYZ(skull_scale,skull_scale,skull_scale);
	// 	cout<<"scale :"<<skull_scale<<endl;
	// }
  // else if (a_key == GLFW_KEY_8)
	// {
	// 	skull_scale -= 0.5;
  //   m_meshSkull->scaleXYZ(skull_scale,skull_scale,skull_scale);
  //   m_meshVentricles->scaleXYZ(skull_scale,skull_scale,skull_scale);
	// 	cout<<"scale :"<<skull_scale<<endl;
	// }
  // else if (a_key == GLFW_KEY_8)
	// {
	// 	rotateFlag = !(rotateFlag);
	// }
	/*
    else if (a_key == GLFW_KEY_P)
	{
	initialPos = me_Y: "<<rot.getCol1()<<endl;
	cout<<"ROTATION_Z: "<<rot.getCol2()<<endl;
	cout<<"\n*********************ENDOSCOPE ORIENTATION******************************************\n";
	}
	else if (a_key == GLFW_KEY_W)
    {
		cVector3d pos = meshEndoscope->getLocalPos();

		pos = cVector3d(pos.x() + 1 , pos.y() , pos.z());

		meshEndoscope->setLocalPos(pos);
    }
	else if (a_key == GLFW_KEY_S)
    {
		cVector3d pos = meshEndoscope->getLocalPos();

		pos = cVector3d(pos.x() - 1 , pos.y() , pos.z());

		meshEndoscope->setLocalPos(pos);
    }
	else if (a_key == GLFW_KEY_A)
    {
		cVector3d pos = meshEndoscope->getLocalPos();

		pos = cVector3d(pos.x() , pos.y() - 1 , pos.z());

		meshEndoscope->setLocalPos(pos);
    }
	else if (a_key == GLFW_KEY_D)
    {
		cVector3d posshEndoscope->getLocalPos();
	cout<<"\n****************************************\n";
	cout<<initialPos<<endl;
	cout<<offsetPos<<endl;
	offsetPos.sub(initialPos);
	cout<<initialPos<<endl;
	cout<<offsetPos<<endl;
	cout<<"\n****************************************\n";
	//cout<<"Orientation : "<<rotate_x<<" ,"<<rotate_y<<" ,"<<rotate_z<<endl;
	cVector3d pos = meshEndoscope->getLocalPos();
    cMatrix3d rot = meshEndoscope->getLocalRot();
	cout<<"\n*********************ENDOSCOPE ORIENTATION******************************************\n";
	cout<<"COORDINATES: "<<pos.get(0)<<", "<<pos.get(1)<<", "<<pos.get(2)<<", "<<endl;
	//cout<<"ROTATION_X: "<<rot.get(0,0)<<", "<<rot.get(0,1)<<", "<<rot.get(0,2)<<", "<<endl;
	//cout<<"ROTATION_Y: "<<rot.get(1,0)<<", "<<rot.get(1,1)<<", "<<rot.get(1,2)<<", "<<endl;
	//cout<<"ROTATION_Z: "<<rot.get(2,0)<<", "<<rot.get(2,1)<<", "<<rot.get(2,2)<<", "<<endl;
	cout<<"ROTATION_X: "<<rot.getCol0()<<endl;
	cout<<"ROTATION_Y: "<<rot.getCol1()<<endl;
	cout<<"ROTATION_Z: "<<rot.getCol2()<<endl;
	cout<<"\n*********************ENDOSCOPE ORIENTATION******************************************\n";
	}
	else if (a_key == GLFW_KEY_W)
    {
		cVector3d pos = meshEndoscope->getLocalPos();

		pos = cVector3d(pos.x() + 1 , pos.y() , pos.z());

		meshEndoscope->setLocalPos(pos);
    }
	else if (a_key == G// m_meshVentricles->setLocalPos(d);
    // m_meshThirdVentricles->setLocalPos(d);
    // m_meshThalastriateVein->setLocalPos(d);
    // m_meshChoroidPlexus->setLocalPos(d);
    // m_meshVentricles->setLocalRot(m);
    // m_meshThirdVentricles->setLocalRot(m);
    // m_meshThalastriateVein->setLocalRot(m);
    // m_meshChoroidPlexus->setLocalRot(m);LFW_KEY_S)
    {
		cVector3d pos = meshEndoscope->getLocalPos();

		pos = cVector3d(pos.x() - 1 , pos.y() , pos.z());

		meshEndoscope->setLocalPos(pos);
    }
	else if (a_key == GLFW_KEY_A)
    {
		cVector3d pos = meshEndoscope->getLocalPos();

		pos = cVector3d(pos.x() , pos.y() - 1 , pos.z());

		meshEndoscope->setLocalPos(pos);
    }
	else if (a_key == GLFW_KEY_D)
    {
		cVector3d poselse if (a_key == GLFW_KEY_R)
	{
		rotate_x = rotate_x+5;
		m_meshVentricles->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
		m_meshThirdVentricles->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
		m_meshThalastriateVein->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
		m_meshChoroidPlexus->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
	}
		else if (a_key == GLFW_KEY_T)
	{
		rotate_x = rotate_x-5;
		m_meshVentricles->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
		m_meshThirdVentricles->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
		m_meshThalastriateVein->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
		m_meshChoroidPlexus->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
	}
		else if (a_key == GLFW_KEY_Y)
	{
		rotate_y = rotate_y+5;
		m_meshVentricles->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
		m_meshThirdVentricles->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
		m_meshThalastriateVein->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
		m_meshChoroidPlexus->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
	}
		else if (a_key == GLFW_KEY_U)
	{
		rotate_y = rotate_y-5;
		m_meshVentricles->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
		m_meshThirdVentricles->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
		m_meshThalastriateVein->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
		m_meshChoroidPlexus->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
	}
		else if (a_key == GLFW_KEY_I)
	{
		rotate_z = rotate_z+5;
		m_meshVentricles->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
		m_meshThirdVentricles->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
		m_meshThalastriateVein->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
		m_meshChoroidPlexus->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
	}
	else if (a_key == GLFW_KEY_O)
	{
		rotate_z = rotate_z-5;
		m_meshVentricles->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
		m_meshThirdVentricles->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
		m_meshThalastriateVein->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
		m_meshChoroidPlexus->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
	}
		else if (a_key == GLFW_KEY_E)
	{
		rotate_z = 0;
		rotate_x = 0;
		rotate_y = 0;
		m_meshVentricles->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
		m_meshThirdVentricles->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
		m_meshThalastriateVein->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
		m_meshChoroidPlexus->rotateAboutGlobalAxisDeg(rotate_x, rotate_y, rotate_z, rotate_val);
	}
	else if (a_key == GLFW_KEY_L)
	{
		cVector3d pos;
    cMatrix3d m1,m2,m3,m4;
		hapticDevice->getPosition(pos);
		cout<<"position :"<<pos<<endl;
    // cout<<m_meshVentricles->getLocalPos()<<endl;
    // cout<<m_meshThirdVentricles->getLocalPos()<<endl;
    // cout<<m_meshThalastriateVein->getLocalPos()<<endl;
    // cout<<m_meshChoroidPlexus->getLocalPos()<<endl;
    m1=m_meshVentricles->getLocalRot();
    cout<<"matrix1 : "<<endl<<m1.getRow(0)<<endl<<m1.getRow(1)<<endl<<m1.getRow(2)<<endl;
    m1=m_meshThirdVentricles->getLocalRot();
    cout<<"matrix1 : "<<endl<<m1.getRow(0)<<endl<<m1.getRow(1)<<endl<<m1.getRow(2)<<endl;
    m1=m_meshThalastriateVein->getLocalRot();
    cout<<"matrix1 : "<<endl<<m1.getRow(0)<<endl<<m1.getRow(1)<<endl<<m1.getRow(2)<<endl;
    m1=m_meshChoroidPlexus->getLocalRot();
    cout<<"matrix1 : "<<endl<<m1.getRow(0)<<endl<<m1.getRow(1)<<endl<<m1.getRow(2)<<endl;
    // cout<<m_meshThirdVentricles->getLocalRot()<<endl<<endl;
    // cout<<m_meshThalastriateVeinVentricles->getLocalRot()<<endl<<endl;
    // cout<<m_meshChoroidPlexus->getLocalRot()<<endl<<endl;
	}
  else if (a_key == GLFW_KEY_7)
	{
		skull_scale += 0.5;
    m_meshSkull->scaleXYZ(skull_scale,skull_scale,skull_scale);
    m_meshVentricles->scaleXYZ(skull_scale,skull_scale,skull_scale);
		cout<<"scale :"<<skull_scale<<endl;
	}-0.0222979, 0.00372431, -0.109716
  else if (a_key == GLFW_KEY_8)
	{
		skull_scale -= 0.5;
    m_meshSkull->scaleXYZ(skull_scale,skull_scale,skull_scale);
    m_meshVentricles->scaleXYZ(skull_scale,skull_scale,skull_scale);
		cout<<"scale :"<<skull_scale<<endl;
	}
  else if (a_key == GLFW_KEY_8)
	{
		rotateFlag = !(rotateFlag);
	}
	/*
    else if (a_key == GLFW_KEY_P)
	{
	initialPos = meshEndoscope->getLocalPos();
	cout<<"\n****************************************\n";
	cout<<initialPos<<endl;
	cout<<offsetPos<<endl;
	offsetPos.sub(initialPos);
	cout<<initialPos<<endl;
	cout<<offsetPos<<endl;
	cout<<"\n****************************************\n";
	//cout<<"Orientation : "<<rotate_x<<" ,"<<rotate_y<<" ,"<<rotate_z<<endl;
	cVector3d pos = meshEndoscope->getLocalPos();
    cMatrix3d rot = meshEndoscope->getLocalRot();
	cout<<"\n*********************ENDOSCOPE ORIENTATION******************************************\n";
	cout<<"COORDINATES: "<<pos.get(0)<<", "<<pos.get(1)<<", "<<pos.get(2)<<", "<<endl;
	//cout<<"ROTATION_X: "<<rot.get(0,0)<<", "<<rot.get(0,1)<<", "<<rot.get(0,2)<<", "<<endl;
	//cout<<"ROTATION_Y: "<<rot.get(1,0)<<", "<<rot.get(1,1)<<", "<<rot.get(1,2)<<", "<<endl;
	//cout<<"ROTATION_Z: "<<rot.get(2,0)<<", "<<rot.get(2,1)<<", "<<rot.get(2,2)<<", "<<endl;
	cout<<"ROTATION_X: "<<rot.getCol0()<<endl;
	cout<<"ROTATION_Y: "<<rot.getCol1()<<endl;
	cout<<"ROTATION_Z: "<<rot.getCol2()<<endl;
	cout<<"\n*********************ENDOSCOPE ORIENTATION******************************************\n";
	}
	else if (a_key == GLFW_KEY_W)
    {
		cVector3d pos = meshEndoscope->getLocalPos();

		pos = cVector3d(pos.x() + 1 , pos.y() , pos.z());

		meshEndoscope->setLocalPos(pos);
    }
	else if (a_key == GLFW_KEY_S)
    {
		cVector3d pos = meshEndoscope->getLocalPos();

		pos = cVector3d(pos.x() - 1 , pos.y() , pos.z());

		meshEndoscope->setLocalPos(pos);
    }
	else if (a_key == GLFW_KEY_A)
    {
		cVector3d pos = meshEndoscope->getLocalPos();

		pos = cVector3d(pos.x() , pos.y() - 1 , pos.z());

		meshEndoscope->setLocalPos(pos);
    }
	else if (a_key == GLFW_KEY_D)
    {
		cVector3d pos = meshEndoscope->getLocalPos();

		pos = cVector3d(pos.x() , pos.y() + 1 , pos.z());

		meshEndoscope->setLocalPos(pos);
    }
	else if (a_key == GLFW_KEY_I)
    {
		cVector3d pos = meshEndoscope->getLocalPos();

		pos = cVector3d(pos.x() , pos.y() , pos.z() - 1);

		meshEndoscope->setLocalPos(pos);
    }
	else if (a_key == GLFW_KEY_O)
    {
		cVector3d pos = meshEndoscope->getLocalPos();

		pos = cVector3d(pos.x() , pos.y() , pos.z() - 1);

		meshEndoscope->setLocalPos(pos);
    }
*/

// Updates on Thursday 26/2018
else if (a_key == GLFW_KEY_EQUAL)
	{
		float su = 0.9;
		m_meshVentricles->scaleXYZ(su, su, su);
		m_meshThirdVentricles->scaleXYZ(su, su, su);
		m_meshThalastriateVein->scaleXYZ(su, su, su);
		m_meshChoroidPlexus->scaleXYZ(su, su, su);
    m_meshSkull -> scaleXYZ(su, su, su);
	}
// m_meshVentricles->setLocalPos(d);
    // m_meshThirdVentricles->setLocalPos(d);
    // m_meshThalastriateVein->setLocalPos(d);
    // m_meshChoroidPlexus->setLocalPos(d);
    // m_meshVentricles->setLocalRot(m);
    // m_meshThirdVentricles->setLocalRot(m);
    // m_meshThalastriateVein->setLocalRot(m);
    // m_meshChoroidPlexus->setLocalRot(m);
else if (a_key == GLFW_KEY_MINUS)
	{
		float su = 1.1;
		m_meshVentricles->scaleXYZ(su, su, su);
		m_meshThirdVentricles->scaleXYZ(su, su, su);
		m_meshThalastriateVein->scaleXYZ(su, su, su);
		m_meshChoroidPlexus->scaleXYZ(su, su, su);
    m_meshSkull -> scaleXYZ(su, su, su);
	}
  // -------------------------------------------------------

else if (a_key == GLFW_KEY_C)
	{
		calibrate3d(virPoint1, virPoint2, virPoint3, realPoint1, realPoint2, realPoint3);
    cVector3d d(-0.89, -0.26, -1.29);
    // cVector3d d2(-0.0222979, 0.00372431, -0.109716);
    cMatrix3d m(-0.933013, -0.340224, -0.117195,0.25, -0.847116, 0.468929,-0.258819, 0.408218, 0.875426);
    m_meshVentricles->setLocalPos(d);
    m_meshThirdVentricles->setLocalPos(d);
    m_meshThalastriateVein->setLocalPos(d);
    m_meshChoroidPlexus->setLocalPos(d);
    m_meshVentricles->setLocalRot(m);
    m_meshThirdVentricles->setLocalRot(m);
    m_meshThalastriateVein->setLocalRot(m);
    m_meshChoroidPlexus->setLocalRot(m);
    float su = 1/(1.1*1.1*1.1);
    m_meshVentricles->scaleXYZ(su, su, su);
		m_meshThirdVentricles->scaleXYZ(su, su, su);
		m_meshThalastriateVein->scaleXYZ(su, su, su);
		m_meshChoroidPlexus->scaleXYZ(su, su, su);
    m_meshSkull -> scaleXYZ(su, su, su);
    cVector3d d2(-0.349392, -0.130379, -0.77361);
    m_meshVentricles->setLocalPos(d2);
    m_meshThirdVentricles->setLocalPos(d2);
    m_meshThalastriateVein->setLocalPos(d2);
    m_meshChoroidPlexus->setLocalPos(d2);
	}
/*
else if (a_key == GLFW_KEY_1)
	{
		meshEndoscope->setLocalPos(virPoint1);
	}
else if (a_key == GLFW_KEY_2)
	{
		meshEndoscope->setLocalPos(virPoint2);
	}
else if (a_key == GLFW_KEY_3)
	{
		meshEndoscope->setLocalPos(virPoint3);
	}
*/
// -------------------------------------------------------
else if (a_key == GLFW_KEY_1)
	{
		// realPoint1 = meshEndoscope->getLocalPos();
		// cout<<"point 1 "<<realPoint1<<endl;
    // cVector3d realPoint1;
		hapticDevice->getPosition(realPoint1);
	  cout<<"point 1 "<<realPoint1<<endl;
	}
else if (a_key == GLFW_KEY_2)
	{
		// realPoint2 = meshEndoscope->getLocalPos();
		// cout<<"point 2 "<<realPoint2<<endl;
    // cVector3d realPoint2;
		hapticDevice->getPosition(realPoint2);
	  cout<<"point 2 "<<realPoint2<<endl;
  }
else if (a_key == GLFW_KEY_3)
	{
		// realPoint3 = meshEndoscope->getLocalPos();
		// cout<<"point 3 "<<realPoint3<<endl;
    // cVector3d realPoint3;
		hapticDevice->getPosition(realPoint3);
	  cout<<"point 3 "<<realPoint3<<endl;
	}
// else if (a_key == GLFW_KEY_V)
// 	{
// 		// realPoint3 = meshEndoscope->getLocalPos();
// 		// cout<<"point 3 "<<realPoint3<<endl;
//     // cVector3d realPoint3;
// 		hapticDevice->getPosition(realPoint3);
// 	  cout<<"point 3 "<<realPoint3<<endl;
// 	}


// Updates on Thirsday 26/2018 ends
}


//------------------------------------------------------------------------------

void close(void)
{
    // stop the simulation
    simulationRunning = false;

    // wait for graphics and haptics loops to terminate
    while (!simulationFinished) { cSleepMs(100); }

    // close haptic device
    hapticDevice->close();

    // delete resources
    delete hapticsThread;
    delete world;
    delete handler;
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

        // compute new camera angles
        double azimuthDeg = camera->getSphericalAzimuthDeg() - 0.5 * dx;
        double polarDeg = camera->getSphericalPolarDeg() - 0.5 * dy;

        // assign new angles
        camera->setSphericalAzimuthDeg(azimuthDeg);
        camera->setSphericalPolarDeg(polarDeg);
        //cout << "azimuthDeg " << azimuthDeg << endl;
        //cout << "polarDeg " << polarDeg << endl;
        // oriente tool with camera
        //tool->setLocalRot(camera->getLocalRot());
    }
    else if (mouseState == MOUSE_TRANSLATE_CAMERA)
    {
        // compute mouse motion
        int dx = a_posX - mouseX;
        int dy = a_posY - mouseY;
        mouseX = a_posX;
        mouseY = a_posY;
        cVector3d oldPos = camera->getLocalPos();
        cVector3d newPos = cVector3d(oldPos.x() + 0.001*dx, oldPos.y() + 0.001*dy, oldPos.z()-0.001*dy);
        //camera->setSphericalAzimuthReference(newPos);
        camera->setLocalPos(newPos);
    }
}

//------------------------------------------------------------------------------

void mouseScrollCallback(GLFWwindow* a_window, double a_offsetX, double a_offsetY)
{
    double r = camera->getSphericalRadius();
    r = cClamp(r + 0.1 * a_offsetY, 0.001, 10.0);
    camera->setSphericalRadius(r);
    //cout << "r " << r << endl;
}

//------------------------------------------------------------------------------
void updateGraphics0(void)
{
    /////////////////////////////////////////////////////////////////////
    // UPDATE WIDGETS
    /////////////////////////////////////////////////////////////////////

    // update haptic and graphic rate data
    labelRates->setText(cStr(freqCounterGraphics.getFrequency(), 0) + " Hz / " +
                        cStr(freqCounterHaptics.getFrequency(), 0) + " Hz");

    // update position of label
    labelRates->setLocalPos((int)(0.5 * (width0 - labelRates->getWidth())), 15);


    /////////////////////////////////////////////////////////////////////
    // RENDER SCENE
    /////////////////////////////////////////////////////////////////////

    defWorld->updateSkins(true);

    // update shadow maps (if any)
    world->updateShadowMaps(false, mirroredDisplay);


    camera->renderView(width0, height0);


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



    cameraEndoScope->renderView(width1, height1);


    // wait until all GL commands are completed
    glFinish();

    // check for any OpenGL errors
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) cout << "Error: " << gluErrorString(err) << endl;
}
//------------------------------------------------------------------------------


void updateHaptics(void)
{
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

	//cVector3d offsetPos(0.0853583, -0.86858, 0.513968);
	cVector3d temp(0.0,0.0,0.0);//change made on 13 march
	//temp.x(1);
	//rot.setCol0(temp);
	//temp.x(0);
	//temp.y(1);
    //rot.setCol1(temp);
	//temp.y(0);


	//cMatrix3d offsetRot(0.0497357, -0.886516, 0.460018, 0.964903, -0.0762627, -0.251291, 0, 0, 1);
	cVector3d z_axis(0.0,0.0,1.0);

	cVector3d zero(0.0, 0.0, 0.0);

  if(!(rotateFlag)){
	   temp.z(1);
	   rot.setCol2(temp);
  }

	pos.mul(workspaceScaleFactor);
        //cout << "workspaceScaleFactor " << workspaceScaleFactor << endl;
        //device->setLocalPos(pos);
        //device->setLocalPos(pos);
        if(switchtool)
        {
			//cout<<"\n------------------------------------------------------------------------------------------------\n";
			//cout<<offsetPos;
			//cout<<"\n------------------------------------------------------------------------------------------------\n";
			//pos.add(offsetPos);//update on
			//rot.mul(offsetRot);
            meshEndoscope->setLocalPos(pos);
            //**cameraEndoScope->setLocalPos(pos);**//
            meshEndoscope->setLocalRot(rot);
            //**cameraEndoScope->setLocalRot(rotCamera);**//
			//meshEndoscope->rotateAboutGlobalAxisDeg(z_axis,-90.0);//rotation to get endoscope right
        }
        else
        {
            drillTool->setLocalPos(pos);
            drillTool->setLocalRot(camera->getLocalRot());

        }

        if(perforate)
        {
            // clear all external forces
            defWorld->clearExternalForces();

            // compute reaction forces
            cVector3d force(0.0, 0.0, 0.0);
            list<cGELSkeletonNode*>::iterator i;
            for(i = m_meshThirdVentricles->m_nodes.begin(); i != m_meshThirdVentricles->m_nodes.end(); ++i)
            {
                cGELSkeletonNode* nextItem = *i;

                cVector3d nodePos = nextItem->m_pos;
                cVector3d f = computeForce(pos, 0, nodePos, deviceRadius+nextItem->m_radius, stiffness);
                force.add(f);
                cVector3d tmpfrc = -1.0 * f;
                nextItem->setExternalForce(tmpfrc);
            }
            // integrate dynamics
            defWorld->updateDynamics(time);
            // scale force
            force.mul(deviceForceScale);
            // send forces to haptic device
            hapticDevice->setForce(force);
        }
        // signal frequency counter
        freqCounterHaptics.signal(1);
	//==========================getting points of calibration==================
        // read user-switch status (button 0)
        bool button;
        hapticDevice->getUserSwitch(0, button);
	if(button){
		// hapticDevice->getPosition(pos);
		// hapticDevice->getRotation(rot);
		// cout<<"positon "<<pos<<endl;
	}
    }

    // exit haptics thread
    simulationFinished = true;
}
//------------------------------------------------------------------------------

cVector3d computeForce(const cVector3d& a_cursor,
                       double a_cursorRadius,
                       const cVector3d& a_spherePos,
                       double a_radius,
                       double a_stiffness)
{

    // In the following we compute the reaction forces between the tool and the
    // sphere.
    cVector3d force;
    force.zero();

    cVector3d vSphereCursor = a_cursor - a_spherePos;

    if (vSphereCursor.length() < 0.0000001)
    {
        return (force);
    }
    if (vSphereCursor.length() > (a_cursorRadius + a_radius))
    {
        return (force);
    }

    // compute penetration distance between tool and surface of sphere
    double penetrationDistance = (a_cursorRadius + a_radius) - vSphereCursor.length();
    cVector3d forceDirection = cNormalize(vSphereCursor);
    force = cMul( penetrationDistance * a_stiffness, forceDirection);

    return (force);
}

void renderSpheres()
{
    spheres.resize(dynamicNodes.size());
    for(int i = 0; i < dynamicNodes.size(); i++)
    {
        spheres[i] = new cShapeSphere(0.002);
        world->addChild(spheres[i]);
        spheres[i]->setLocalPos(dynamicNodes[i].x(), dynamicNodes[i].y(), dynamicNodes[i].z());
        spheres[i]->m_material->setRedFireBrick();
    }
}

void BuildDynamicModel()
{
    cGELSkeletonNode* newNode;
    cGELSkeletonNode* prevNode;
    cGELSkeletonLink* newLink;

    //-----------------------------------
    newNode = new cGELSkeletonNode(); m_meshThirdVentricles->m_nodes.push_front(newNode);
    newNode->m_pos.set(dynamicNodes[0].x(), dynamicNodes[0].y(), dynamicNodes[0].z());
    //-----------------------------------

    for(int i = 1; i < dynamicNodes.size(); i++)
    {
        prevNode = newNode; newNode = new cGELSkeletonNode(); m_meshThirdVentricles->m_nodes.push_front(newNode);
        newNode->m_pos.set(dynamicNodes[i].x(), dynamicNodes[i].y(), dynamicNodes[i].z());
        newLink = new cGELSkeletonLink(prevNode, newNode); m_meshThirdVentricles->m_links.push_front(newLink);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////CALIBRATION CODE STARTS HERE//////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

cVector3d normal_find(cVector3d a,cVector3d b,cVector3d c) {
cVector3d dir0((a.x() - b.x()), (a.y() - b.y()), (a.z() - b.z()) );
cVector3d dir1((c.x() - b.x()), (c.y() - b.y()), (c.z() - b.z()) );
cVector3d norm;
dir1.crossr(dir0, norm);

return norm;

}

cMatrix3d rotateNorm(cVector3d a,cVector3d b,cVector3d c,cVector3d A,cVector3d B,cVector3d C){ // a,b,c virtual skull ,ABC physical skull
	cVector3d unit1,unit2,cross,dot, cross_normalise;
	double sint,cost;

	unit1 = normal_find(a,b,c);
	unit2 = normal_find(A,B,C);
	unit1.normalize();
	unit2.normalize();
	unit1.crossr(unit2,cross);
	cross_normalise = cross;
	cross_normalise.normalize();
	cost = unit1.dot(unit2);
	sint = cross.x()/ cross_normalise.x();
	double v1, v2, v3;
	v1 = cross.x();
	v2 = cross.y();
	v3 = cross.z();
	cMatrix3d vx(0, -v3, v2, v3, 0, -v1, -v2, v1, 0);
	cMatrix3d i, vx_2, result;
	i.identity();
	vx.mulr(vx, vx_2);
	i.addr(vx, result);
	vx_2*=(1/1+cost);
	result.add(vx_2);

	return result;



}

double scale1(cVector3d a,cVector3d b, cVector3d c, cVector3d A,cVector3d B, cVector3d C){
	//double scale;
	//scale = (A.x() - B.x())/(a.x() - b.x());
	double distV;
	distV = cDistance(a, b);
  cout<<"Dist ab: "<<distV<<endl;
	double distR;
	distR = cDistance(A, B);
  cout<<"Dist AB: "<<distR<<endl;
	double scale_1 =  (distR/distV);
	distV = cDistance(c, b);
	distR = cDistance(C, B);
  cout<<"Dist cb: "<<distV<<endl;
  cout<<"Dist CB: "<<distR<<endl;
	double scale_2 =  (distR/distV);
	distV = cDistance(a, c);
	distR = cDistance(A, C);
  cout<<"Dist ac: "<<distV<<endl;
  cout<<"Dist AC: "<<distR<<endl;
	double scale_3 =  (distR/distV);
	return (scale_1 + scale_2 + scale_3)/3;
}


cVector3d translate1(cVector3d a,cVector3d A){
	cVector3d trans;
	A.subr(a,trans);
	return trans;
}

cMatrix3d rotatePlan(cVector3d A,cVector3d b,cVector3d B,cVector3d normal){
	cVector3d dir1,dir2;
	b.subr(A,dir1);
	B.subr(A,dir2);
	dir1.normalize();
	dir2.normalize();
	double cosvalue,angleradian;
	cosvalue = dir1.dot(dir2);
	angleradian = acos(cosvalue);
	cMatrix3d result;
	result = cRotAxisAngleRad(normal.x(),normal.y(),normal.z(),angleradian);
	return result;
}

//finding the normals



void calibrate3d(cVector3d a,cVector3d b,cVector3d c,cVector3d A,cVector3d B,cVector3d C){
	double scaling_factor;
	cMatrix3d rotate1,rotate2;
	cVector3d translate,normal, normal_v;
	scaling_factor = scale1(a,b,c,A,B,C);
	normal = normal_find(A,B,C);
  cout<<"Normal Real: "<<normal<<endl;

	//applying scale to the three points
	a = a*scaling_factor;
	b = b*scaling_factor;
	c = c*scaling_factor;

	rotate1 = rotateNorm(a,b,c,A,B,C);

	//applying the rotation 1 (aligning the normal vectors of the plane)
  //changing mulr to mul//undo
	rotate1.mulr(a,a);
	rotate1.mulr(b,b);
	rotate1.mulr(c,c);

  normal_v = normal_find(a,b,c);
  cout<<"Normal Virtual: "<<normal_v<<endl;

	translate = translate1(a,A);

	//applying the translation (making one point coincide with the physical skull's point)
	a.add(translate);
	b.add(translate);
	c.add(translate);
//changing a to A
	rotate2 = rotatePlan(A,b,B,normal);

	cout<<"Scaling Factor:    "<<scaling_factor<<endl;
	//Applying changes to the mesh
	 m_meshVentricles->scaleXYZ(scaling_factor, scaling_factor, scaling_factor);
	 m_meshThirdVentricles->scaleXYZ(scaling_factor, scaling_factor, scaling_factor);
	 m_meshThalastriateVein->scaleXYZ(scaling_factor, scaling_factor, scaling_factor);
	 m_meshChoroidPlexus->scaleXYZ(scaling_factor, scaling_factor, scaling_factor);
  m_meshSkull->scaleXYZ(scaling_factor,scaling_factor,scaling_factor);

	m_meshVentricles->setLocalTransform(rotate1);
	m_meshThirdVentricles->setLocalTransform(rotate1);
	m_meshThalastriateVein->setLocalTransform(rotate1);
	m_meshChoroidPlexus->setLocalTransform(rotate1);
  m_meshSkull->setLocalTransform(rotate1);


	m_meshVentricles->translate(translate);
	m_meshThirdVentricles->translate(translate);
	m_meshThalastriateVein->translate(translate);
	m_meshChoroidPlexus->translate(translate);
  m_meshSkull->translate(translate);

	m_meshVentricles->setLocalTransform(rotate2);
	m_meshThirdVentricles->setLocalTransform(rotate2);
	m_meshThalastriateVein->setLocalTransform(rotate2);
	m_meshChoroidPlexus->setLocalTransform(rotate2);
  m_meshSkull->setLocalTransform(rotate2);

}
