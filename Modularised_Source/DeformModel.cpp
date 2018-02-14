#include "DeformModel.h"

//default constructor
DeformModel::DeformModel(){}

DeformModel::DeformModel(cWorld* world, string path){
	defWorld = new cGELWorld();
    world->addChild(defWorld);
    m_meshThirdVentricles = new cGELMesh();
    defWorld->m_gelMeshes.push_front(m_meshThirdVentricles);

    bool fileload = m_meshThirdVentricles->loadFromFile(path);
    if (!fileload)
      	throw -1;
    
    m_meshThirdVentricles->scaleXYZ(1 /235.0, 1 / 235.0, 1 / 235.0);
    m_meshThirdVentricles->m_meshes->at(0)->m_material->setWhite();
    m_meshThirdVentricles->m_meshes->at(0)->m_material->setShininess(100);
    m_meshThirdVentricles->m_meshes->at(0)->m_material->setTextureLevel(0.9);
    m_meshThirdVentricles->setLocalPos(cVector3d(0.19, -0.04, -0.24));

    normalMap_ThirdVentricle = cNormalMap::create();
    m_meshThirdVentricles->computeBTN();
    m_meshThirdVentricles->computeBoundaryBox(true);

    // build dynamic vertices
    m_meshThirdVentricles->buildVertices();

    // setup default values for nodes
    cGELSkeletonNode::s_default_radius        = 0.007;  // [m]
    cGELSkeletonNode::s_default_kDampingPos   = 10;
    cGELSkeletonNode::s_default_kDampingRot   = 0.6;
    cGELSkeletonNode::s_default_mass          = 0.2; // [kg]
    cGELSkeletonNode::s_default_showFrame     = false;
    cGELSkeletonNode::s_default_color.set(0.6, 0.6, 0.0);
    cGELSkeletonNode::s_default_useGravity      = false;
    cGELSkeletonNode::s_default_gravity.set(0.00, 0.00, 0.00);
    radius = cGELSkeletonNode::s_default_radius;

    

}

// create dynamic model (GEM)
void DeformModel::BuildDynamicModel(string path)
{

    // use internal skeleton as deformable model
    m_meshThirdVentricles->m_useSkeletonModel = true;

    //load dynamic model
    ifstream fin(path + "/mesh_models/Sampled-Points-withedges.obj");  
    string linestr;
    string type;
    double x_1,y_1,z_1;
    cGELSkeletonNode* newNode;
    while ( std::getline(fin, linestr) ){
        stringstream ss(linestr);
        ss>>type;
        if(type=="v"){
        	ss>>x_1;
        	ss>>y_1;
        	ss>>z_1;
        	newNode = new cGELSkeletonNode();
        	m_meshThirdVentricles->m_nodes.push_front(newNode);
    		newNode->m_pos.set(x_1 / 235.0  , y_1 / 235.0 , z_1 / 235.0 );
    		dynamicNodes.push_back(newNode);
        }else{
        	break;
        }
    }



    //BOUNDARY POINTS
    ifstream fin2(path + "/mesh_models/BoundaryPoints_ThirdVentricle.txt");  
    while ( std::getline(fin2, linestr) ){
        stringstream ss(linestr);
        int x;
        ss>>x;
        //cout << "fixed" << endl;
        dynamicNodes[x - 1]->m_fixed = true;
    }







    // setup default values for links
    cGELSkeletonLink::s_default_kSpringElongation = 250; // [N/m]
    cGELSkeletonLink::s_default_kSpringFlexion    = 0.5;   // [Nm/RAD]
    cGELSkeletonLink::s_default_kSpringTorsion    = 0.1;   // [Nm/RAD]
    cGELSkeletonLink::s_default_color.set(0.2, 0.2, 1.0);

    cGELSkeletonLink* newLink;
    int node1, node2;
    while ( std::getline(fin, linestr) ){
        stringstream ss(linestr);
        ss>>type;
        if(type=="e"){
        	ss>>node1;
        	ss>>node2;
        	newLink = new cGELSkeletonLink(dynamicNodes[node1 -1], dynamicNodes[node2 - 1]);
            newLink->m_enabled = false;
        	m_meshThirdVentricles->m_links.push_front(newLink);
        }else{
        	break;
        }
    }
    // cGELSkeletonLink* newLink;
    // int node1, node2;
    // while ( std::getline(fin, linestr) ){
    //     stringstream ss(linestr);
    //     ss>>type;
    //     if(type=="e"){
    //     	ss>>node1;
    //     	ss>>node2;
    //     	newLink = new cGELSkeletonLink(dynamicNodes[node1-1], dynamicNodes[node2-1]);
    //     	m_meshThirdVentricles->m_links.push_front(newLink);
    //     }else{
    //     	break;
    //     }
    // }
}

void DeformModel::initSkeleton(){

    // connect skin (mesh) to skeleton (GEM)
    m_meshThirdVentricles->connectVerticesToSkeleton(true);

    // show/hide underlying dynamic skeleton model
    m_meshThirdVentricles->m_showSkeletonModel = false;

    // int counter1 = 0;
    // int counter2 = 0;
    // for(i = m_meshThirdVentricles->m_nodes.begin(); i != m_meshThirdVentricles->m_nodes.end(); ++i)
    // {
    //     if (counter1 <= num)
    //     {
    //         if (counter2 > 3)
    //         {
    //             cGELSkeletonNode* nextItem = *i;
    //             cGELSkeletonNode* newNode = new cGELSkeletonNode();
    //             newNode->m_fixed = true;
    //             newNode->m_pos = nextItem->m_pos;
    //             cGELSkeletonLink* newLink = new cGELSkeletonLink(nextItem, newNode); m_meshThirdVentricles->m_links.push_front(newLink);
    //             newLink->m_kSpringElongation = 5;
    //             counter2 = 0;
    //         }
    //         counter2++;
    //         counter1++;
    //     }
    // }
}

cVector3d DeformModel::perforate(cVector3d pos, double deviceRadius, double stiffness, double time){
	// clear all external forces
    defWorld->clearExternalForces();

    // compute reaction forces
    cVector3d force(0.0, 0.0, 0.0);
    list<cGELSkeletonNode*>::iterator i;
    for(i = m_meshThirdVentricles->m_nodes.begin(); i != m_meshThirdVentricles->m_nodes.end(); ++i)
    {
        cGELSkeletonNode* nextItem = *i;

        cVector3d nodePos = nextItem->m_pos + cVector3d(0.19, -0.04, -0.24);
        // if(i== m_meshThirdVentricles->m_nodes.begin())
        	// cout<<nodePos.x()<<"******"<<endl;
        cVector3d f = computeForce(pos, 0, nodePos, deviceRadius+nextItem->m_radius, stiffness);
        force.add(f);
        cVector3d tmpfrc = -1.0 * f;
        nextItem->setExternalForce(tmpfrc);
    }
    // // integrate dynamics
    defWorld->updateDynamics(time);
    // cout << m_meshThirdVentricles->m_pos.length() << endl;
    // cout<<force.x() << " " << force.y() << " " << force.z() <<endl;
    return force;
}

cVector3d DeformModel::computeForce(const cVector3d& a_cursor,
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

void DeformModel::attachToWorld(cWorld* world){
    world->addChild(defWorld);
}

void DeformModel::removeFromWorld(cWorld* world){
    world->removeChild(defWorld);
}

void DeformModel::translate(float dx, float dy, float dz){
	cVector3d d = m_meshThirdVentricles->getLocalPos();
    d = cVector3d(d.x() + dx, d.y() + dy, d.z() + dz);
    m_meshThirdVentricles->setLocalPos(d);
}

void DeformModel::updateSkins(bool value){
	defWorld->updateSkins(value);
}

void DeformModel::renderSpheres(cWorld* world)
{
	// cShapeSphere* sphere;
	// list<cGELSkeletonNode*>::iterator i;
	// for(i = m_meshThirdVentricles->m_nodes.begin(); i != m_meshThirdVentricles->m_nodes.end(); ++i)
 //    {	
 //    	cGELSkeletonNode* nextItem = *i;

 //        cVector3d nodePos = nextItem->m_pos;
 //        sphere = new cShapeSphere(0.002);
 //        defWorld->addChild(sphere);
 //        sphere->setLocalPos(nodePos + cVector3d(0.19, -0.04, -0.24));
 //        sphere->m_material->setRedFireBrick();
 //    }




    //BOUNDARY
    // ifstream fin("/home/icy/Desktop/VR-Simulation/ETV-Simulator/Data/mesh_models/BoundaryPoints_ThirdVentricle.txt");  
    // string linestr;
    // cShapeSphere* sphere;
    // while ( std::getline(fin, linestr) ){
    //     stringstream ss(linestr);
    //     int x;
    //     ss>>x;
    //     sphere = new cShapeSphere(0.002);
    //     defWorld->addChild(sphere);
    //     sphere->setLocalPos(dynamicNodes[x-1]->m_pos + cVector3d(0.19, -0.04, -0.24));
    //     sphere->m_material->setRedFireBrick();
    // }




//DYNAMIC NODES:-
 //    for(int i = 0; i < dynamicNodes.size(); i++)
 //    {
 //        sphere = new cShapeSphere(0.002);
 //        defWorld->addChild(sphere);
 //        sphere->setLocalPos(dynamicNodes[i]->m_pos);// - cVector3d(0.19, -0.04, -0.24));
 //        sphere->m_material->setRedFireBrick();
 //    }
}
