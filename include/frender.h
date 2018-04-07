#ifndef __cfMMOCfore_H__
#define __cfMMOCfore_H__

#include "SdkSample.h"
#include "fload.h"
#include "rqts.h"

using namespace Ogre;
using namespace OgreBites;

struct SHARED_CAM_DOF
{
    int mWritable;
    Ogre::Vector3 mCamPos;
    Ogre::Vector3 mCamDir;
    Ogre::Quaternion mCamQuat;
};

struct SHARED_FED_BACK
{
    int mWritable;
    bool mForeOver;
};

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

	cfMMOCfore();
	bool frameRenderingQueued(const Ogre::FrameEvent& evt);
#if 1
	bool keyPressed(const OIS::KeyEvent& evt);
	bool keyReleased(const OIS::KeyEvent& evt);
	bool mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID id);
	bool mouseReleased(const OIS::MouseEvent& evt, OIS::MouseButtonID id);
	bool mouseMoved(const OIS::MouseEvent& evt);
#endif // 1

protected:

	void setupContent();
	void cleanupContent();

	void checkLoadReq();
	void splitTile(Ogre::String filename);
	void mergeTile(Ogre::String filename);
	Ogre::String mComMeshFolder = "com/";
	Ogre::String mTexFolder = "tex/";

	void loadTile(Ogre::String filename);
	void createTile(Ogre::String filename, unsigned char mask = 0);
	void unloadTile(Ogre::String filename, bool full = true);

	HashMap<unsigned int, Ogre::String> mHashColor2Tile;
	HashMap<unsigned int, Ogre::String> mAnotherHashColor2Tile;
	HashMap<unsigned int, unsigned int> mHashColor2Pixel;
	HashMap<Ogre::String, unsigned int> mHashTiles2Pixel;
	HashMap<Ogre::String, unsigned int> mHashTiles2Count;
	HashMap<Ogre::String, unsigned int> mHashMatCount;

	FBackLoadThread* mBackThread;
	bool mPreparation, mRenderTex, mHideAABB, mHideInvisible;
	Ogre::Vector3 mInitEye, mInitTarget;
	Ogre::Vector2 mClipDist;
	std::vector<Ogre::String> mRenderedTiles;
	Ogre::String mTextureExtension = ".dds";
	Ogre::LoggingLevel mLoggingLevel;
	OgreBites::CameraStyle mCameraStyle;
	Ogre::Vector3 mYawAxis;
	bool mInitLoadOver;

    void *mSharedMemCam;
    struct SHARED_CAM_DOF *mSharedSttCam;
    int mSharedIDCam;
    bool mForeOver;

    void *mSharedMemVis;
    struct SHARED_TILE_VIS *mSharedSttVis;
    int mSharedIDVis;

    struct SHARED_TILE_VIS mTilesVisbleReg;

    void *mSharedMemFed;
    struct SHARED_FED_BACK *mSharedSttFed;
    int mSharedIDFed;

    unsigned int mVisCacheCount;

    libRQTS *mRQTS;

    float artorbit = 0.0;
};

#endif
