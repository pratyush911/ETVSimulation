#include "Skull.h"

//deafult constructor
Skull::Skull(){}

Skull::Skull(string model, string colormap, double maxStiffness, cWorld* world){
	voxelBrainSkullOpacityThreshold = 1.0;

	m_voxelBrainSkull = new cVoxelObject();
    m_voxelBrainSkull->setLocalPos(0.0, 0.0, 0.0);
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
    int filesloaded = m_imagesBrainSkull->loadFromFiles(model, "png", 512);
    if (filesloaded == 0)
    {
        cout << "Failed to load volume data.Make sure folder contains files in the form 0XXX.png" << endl;
        throw -1;
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
    bool fileLoaded = m_lutBrainSkull->loadFromFile(colormap);
    if (!fileLoaded)
    {
        cout << "Failed to load colormap.Make sure folder contains colormap.png" << endl;
        throw -2;
    }
    m_voxelBrainSkull->m_colorMap->setImage(m_lutBrainSkull);
    // set haptic properties
    m_voxelBrainSkull->m_material->setStiffness(0.2 * maxStiffness);
    m_voxelBrainSkull->m_material->setStaticFriction(0.0);
    m_voxelBrainSkull->m_material->setDynamicFriction(0.0);
    m_voxelBrainSkull->rotateAboutLocalAxisDeg(1, 0, 0, 90);
    //m_voxelBrainSkull->createAABBCollisionDetector(0.01);
    world->addChild(m_voxelBrainSkull);
}

void Skull::updateOpacity(float dx){
	voxelBrainSkullOpacityThreshold += dx;
    m_voxelBrainSkull->setOpacityThreshold(voxelBrainSkullOpacityThreshold);
}