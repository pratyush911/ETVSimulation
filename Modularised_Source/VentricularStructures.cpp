#include "VentricularStructures.h"

//default constructor
VentricularStructures::VentricularStructures(){}

VentricularStructures::VentricularStructures(string ventPath, string ventNorm, string tVein, string cPlexis, cWorld* world){
	// 1. Ventricles
    m_meshVentricles = new cMultiMesh();
    world->addChild(m_meshVentricles);
    m_meshVentricles->setUseCulling(true, true);
    bool fileload = m_meshVentricles->loadFromFile(ventPath);
    if (!fileload)
    	throw -1;
    m_meshVentricles->scaleXYZ(1 /235.0, 1 / 235.0, 1 / 235.0);
    // compute a boundary box
    m_meshVentricles->computeBoundaryBox(true);
    m_meshVentricles->m_meshes->at(0)->m_material->setWhite();
    m_meshVentricles->m_meshes->at(0)->m_material->setShininess(100);
    m_meshVentricles->m_meshes->at(0)->m_material->setTextureLevel(0.9);
    cNormalMapPtr normalMap_LateralVentricle = cNormalMap::create();
    fileload = normalMap_LateralVentricle->loadFromFile(ventNorm);
    if (!fileload)
    	throw -2;
    m_meshVentricles->m_meshes->at(0)->m_normalMap = normalMap_LateralVentricle;
    m_meshVentricles->computeBTN();
    m_meshVentricles->setLocalPos(cVector3d(0.19, -0.04, -0.24));


    // 2. Thalamostriate_Vein
    m_meshThalastriateVein = new cMultiMesh();
    world->addChild(m_meshThalastriateVein);
    m_meshThalastriateVein->setUseCulling(true, true);
    fileload = m_meshThalastriateVein->loadFromFile(tVein);
    if (!fileload)
        throw -3;
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
    fileload = m_meshChoroidPlexus->loadFromFile(cPlexis);
    if (!fileload)
    	throw -4;
    m_meshChoroidPlexus->scaleXYZ(1 /235.0, 1 / 235.0, 1 / 235.0);
    // compute a boundary box
    m_meshChoroidPlexus->computeBoundaryBox(true);
    m_meshChoroidPlexus->m_meshes->at(0)->m_material->setWhite();
    m_meshChoroidPlexus->m_meshes->at(0)->m_material->setShininess(100);
    m_meshChoroidPlexus->m_meshes->at(0)->m_material->setTextureLevel(0.9);
    m_meshChoroidPlexus->computeBTN();
    m_meshChoroidPlexus->setLocalPos(cVector3d(0.19, -0.04, -0.24));
}

void VentricularStructures::setShader(cShaderProgramPtr programShader){
	m_meshVentricles->setShaderProgram(programShader);
}

void VentricularStructures::translate(float dx, float dy, float dz){
	cVector3d d = m_meshVentricles->getLocalPos();
    d = cVector3d(d.x() + dx, d.y() + dy, d.z() + dz);
    m_meshVentricles->setLocalPos(d);

    d = m_meshThalastriateVein->getLocalPos();
    d = cVector3d(d.x() + dx, d.y() + dy, d.z() + dz);
    m_meshThalastriateVein->setLocalPos(d);


    d = m_meshChoroidPlexus->getLocalPos();
    d = cVector3d(d.x() + dx, d.y() + dy, d.z() + dz);
    m_meshChoroidPlexus->setLocalPos(d);
}