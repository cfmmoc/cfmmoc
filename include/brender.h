/**
    `cfMMOC` library for terrain rendering using OGRE (https://github.com/cfmmoc/cfmmoc/)
    Licensed under The GNU General Public License v3.0 (GPLv3)
    Any modification, re-utilization or copy of the source or binary format in other software or publications should mention a CITATION of this library.
    Copyright (c) 2016-2018 by Authors (Jin Yan, Guanghong Gong, Ni Li and Luhao Xiao)
**/

#ifndef __cfMMOCback_H__
#define __cfMMOCback_H__

#include "SdkSample.h"
#include "bload.h"
#include "bcount.h"
#include "rqts.h"

using namespace Ogre;
using namespace OgreBites;

/**
  @brief  A class for scene management and terrain rendering in back-end process.
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

class cfMMOCback : public SdkSample
{
public:

	/**
	@remarks
		initialize parameters
	**/
	cfMMOCback();
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
        	initialize scene manager and resources for back-end process
    	**/
	void setupBackSceneMgr();
	/**
    	@remarks
        	cleanup scene manager, related resource and memory for back-end process
    	**/
	void cleanupBackSceneMgr();

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
    	**/
	void createBackTile(Ogre::String filename);
	/**
    	@remarks
        	unload given tile
	@par
		filename	name of given tile
    	**/
	void unloadBackTile(Ogre::String filename);

	/**
    	@remarks
        	initialize discrete values for pseudo colors for rendering
    	**/
	void initColorTable();
	/**
    	@remarks
        	assign an unallocated color for given tile
	@par
		filename	name of given tile
    	**/
	Ogre::ColourValue getAnUnusedColor(Ogre::String filename);
	/**
    	@remarks
        	retrieve color which is not used for given tile
	@par
		filename	name of given tile
    	**/
	void retrieveColor(Ogre::String filename);
	/**
    	@remarks
        	retrieve image from GPU, and copy it to memory
    	**/
	void retrieveTexture();
	
	// folder name for simplified mesh
	Ogre::String mSimMeshFolder;
	// discrete values used for pseudo colors
	unsigned char mColorTable[16];
	// a map from predefined color to tile's name in string
	HashMap<unsigned int, Ogre::String> mHashColor2Tile;
	// a copy of map from predefined color to tile's name in string
	HashMap<unsigned int, Ogre::String> mAnotherHashColor2Tile;
	// a map from predefined color to pixel coverage in pixel
	HashMap<unsigned int, unsigned int> mHashColor2Pixel;
	// a map from a tile to summed pixel coverage of its children
	HashMap<Ogre::String, unsigned int> mHashTiles2Pixel;
	// a map from a tile to the number of its children in the scene
	HashMap<Ogre::String, unsigned int> mHashTiles2Count;
	// a map from a tile to the number of same material that is already used
	HashMap<Ogre::String, unsigned int> mHashMatCount;
	// splitting threshold in pixel
	int mSplitThresh;
	// merging threadhold in pixel
	int mMergeThresh;

	// pointer to scene manager
	Ogre::SceneManager* mBackSceneMgr;
	// pointer to camera
	Ogre::Camera* mBackCamera;
	// pointer to render window
	Ogre::RenderWindow* mBackWindow;
	// pointer to render to texture
	Ogre::TexturePtr mBackTex;
	// pointer to memory corresponding to render to texture
	unsigned char *mBackMemTex;
	// field of view of viewport along y direction
	Ogre::Real mBackFovY;

	// pointer to tile fetching object for back-end process
	BBackLoadThread* mBackThread;
	// initial point of camera and the point of look target
	Ogre::Vector3 mInitEye, mInitTarget;
	// distance of near and far clip plane
	Ogre::Vector2 mClipDist;
	// width and height of render to texuter
	Ogre::Vector2 mBackResolution;
	// all tiles in the scene
	std::vector<Ogre::String> mRenderedTiles;
	// level of log detail
	Ogre::LoggingLevel mLoggingLevel;
	// style of camera movement
	OgreBites::CameraStyle mCameraStyle;
	// indicate if initial tiles (first six tiles covering the whole Earth) load is done
	bool mInitLoadOver;

	// a thread to pixel counting
	Poco::Thread mCountingThread;
	// pixel counting implementation
    	CountingThread *mCountingWorker;

	// shared memory and related variables for sync camera position and orientation
    	void *mSharedMemCam;
    	struct SHARED_CAM_DOF *mSharedSttCam;
    	int mSharedIDCam;
    	Ogre::Vector3 mCamPosFromForeend;
    	Ogre::Vector3 mCamDirFromForeend;
    	Ogre::Quaternion mCamQuatFromForeend;
    	bool mOverFromForeend;

	// shared memory and related variables for sync visibility of tiles
    	void *mSharedMemVis;
    	struct SHARED_TILE_VIS *mSharedSttVis;
    	int mSharedIDVis;

	// shared memory and related variables of sync status of fore-end process
    	void *mSharedMemFed;
    	struct SHARED_FED_BACK *mSharedSttFed;
    	int mSharedIDFed;

	// a pointer to a class encapsulated operation of restricted quadtrees
    	libRQTS *mRQTS;
};

#endif
