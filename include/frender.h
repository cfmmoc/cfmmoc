/**
    `cfMMOC` library for terrain rendering using OGRE (https://github.com/cfmmoc/cfmmoc/)
    Licensed under The GNU General Public License v3.0 (GPLv3)
    Any modification, re-utilization or copy of the source or binary format in other software or publications should mention a CITATION of this library.
    Copyright (c) 2016-2018 by Authors (Jin Yan, Guanghong Gong, Ni Li and Luhao Xiao)
**/

#ifndef __cfMMOCfore_H__
#define __cfMMOCfore_H__

#include "SdkSample.h"
#include "fload.h"
#include "rqts.h"

using namespace Ogre;
using namespace OgreBites;

/**
  @brief  A class for scene management and terrain rendering in fore-end process.
**/

/**
@remarks
	struct of camera position and orientation for inter-process communication
**/
struct SHARED_CAM_DOF
{
    int mWritable;
    Ogre::Vector3 mCamPos;
    Ogre::Vector3 mCamDir;
    Ogre::Quaternion mCamQuat;
};

/**
@remarks
	struct of sync status from fore-end process to back-end process
**/
struct SHARED_FED_BACK
{
    int mWritable;
    bool mForeOver;
};

/**
@remarks
	struct of tiles' visibility for inter-process communication
**/
struct SHARED_TILE_VIS
{
    int mWritable;
    bool mVisible[1024];
    char mFilename[1024][32];
    unsigned int mPixel[1024];
    unsigned int mLength;
};

class cfMMOCfore : public SdkSample
{
public:

	/**
	@remarks
		initialize parameters
	**/
	cfMMOCfore();
	/**
    	@remarks
        	rendering main loop
    	@par
		evt	FrameEvent object
    	**/
	bool frameRenderingQueued(const Ogre::FrameEvent& evt);
	/**
    	@remarks
        	keyboard/mouse input event processing funcs
    	@par
		evt	FrameEvent object
    	**/
#if 1
	bool keyPressed(const OIS::KeyEvent& evt);
	bool keyReleased(const OIS::KeyEvent& evt);
	bool mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID id);
	bool mouseReleased(const OIS::MouseEvent& evt, OIS::MouseButtonID id);
	bool mouseMoved(const OIS::MouseEvent& evt);
#endif // 1

protected:

	/**
    	@remarks
        	initialize scene manager, shared memories and resources
    	**/
	void setupContent();
	/**
    	@remarks
        	cleanup scene manager, shared memories and resources
    	**/
	void cleanupContent();

	/**
    	@remarks
        	fire fetch request of tiles, 
		check all tiles that is already fetched, 
		load and split or merge them
    	**/
	void checkLoadReq();
	/**
    	@remarks
        	split given tile
	@par
		filename	name of given tile
    	**/
	void splitTile(Ogre::String filename);
	/**
    	@remarks
        	split children of given tile
	@par
		filename	name of given tile
    	**/
	void mergeTile(Ogre::String filename);

	/**
    	@remarks
        	load given tile into memory
	@par
		filename	name of given tile
    	**/
	void loadTile(Ogre::String filename);
	/**
    	@remarks
        	create given tile
	@par
		filename	name of given tile
		mask		bitwise variable indicates the density of triangles on boundaries
				0000b means low density
				1111b means high density
    	**/
	void createTile(Ogre::String filename, unsigned char mask = 0);
	/**
    	@remarks
        	unload given tile
	@par
		filename	name of given tile
		full		boolean indicates fully remove the all corresponding meshes
				(external mesh and boundary adaptive mesh), material and texture
				or just remove boundary adaptive mesh and material
    	**/
	void unloadTile(Ogre::String filename, bool full = true);

	// folder name for fore-end specified mesh
	Ogre::String mComMeshFolder;
	// folder name for texture
	Ogre::String mTexFolder;
	// extension for texture
	Ogre::String mTextureExtension;
	
	// a map from predefined color to tile's name in string
	HashMap<unsigned int, Ogre::String> mHashColor2Tile;
	// a map from a tile to the number of same material that is already used
	HashMap<Ogre::String, unsigned int> mHashMatCount;

	// pointer to tile fetching object for fore-end process
	FBackLoadThread* mBackThread;
	// initial point of camera and the point of look target
	Ogre::Vector3 mInitEye, mInitTarget;
	// distance of near and far clip plane
	Ogre::Vector2 mClipDist;
	// all tiles in the scene
	std::vector<Ogre::String> mRenderedTiles;
	// level of log detail
	Ogre::LoggingLevel mLoggingLevel;
	// style of camera movement
	OgreBites::CameraStyle mCameraStyle;
	// indicate if initial tiles (first six tiles covering the whole Earth) load is done
	bool mInitLoadOver;

	// shared memory and related variables for sync camera position and orientation
        void *mSharedMemCam;
        struct SHARED_CAM_DOF *mSharedSttCam;
        int mSharedIDCam;
        bool mForeOver;

	// shared memory and related variables for sync visibility of tiles
        void *mSharedMemVis;
        struct SHARED_TILE_VIS *mSharedSttVis;
        int mSharedIDVis;
	// a copy of visibility of tiles
        struct SHARED_TILE_VIS mTilesVisbleReg;

	// shared memory and related variables of sync status of fore-end process
        void *mSharedMemFed;
        struct SHARED_FED_BACK *mSharedSttFed;
        int mSharedIDFed;

	// a pointer to a class encapsulated operation of restricted quadtrees
        libRQTS *mRQTS;

	// starting point of the orbit around the Earth
        float artorbit;
};

#endif
