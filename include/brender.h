#ifndef __cfMMOCback_H__
#define __cfMMOCback_H__

#include "SdkSample.h"
#include "bload.h"
#include "bcount.h"
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

class cfMMOCback : public SdkSample
{
public:

	cfMMOCback();
	bool frameRenderingQueued(const Ogre::FrameEvent& evt);
#if 1
	bool keyPressed(const OIS::KeyEvent& evt);
	bool keyReleased(const OIS::KeyEvent& evt);
	bool mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID id);
	bool mouseReleased(const OIS::MouseEvent& evt, OIS::MouseButtonID id);
	bool mouseMoved(const OIS::MouseEvent& evt);
#endif // 1

protected:

	void setupBackSceneMgr();
	void cleanupBackSceneMgr();

	void setupContent();
	void cleanupContent();

	void checkLoadReq();
	void splitTile(Ogre::String filename);
	void mergeTile(Ogre::String filename);
	Ogre::String mSimMeshFolder = "sim/";

	void loadTile(Ogre::String filename);
	void createBackTile(Ogre::String filename);
	void unloadBackTile(Ogre::String filename);

	void initColorTable();
	Ogre::ColourValue getAnUnusedColor(Ogre::String filename);
	void retrieveColor(Ogre::String filename);
	void retrieveTexture();
	unsigned char mColorTable[16];
	HashMap<unsigned int, Ogre::String> mHashColor2Tile;
	HashMap<unsigned int, Ogre::String> mAnotherHashColor2Tile;
	HashMap<unsigned int, unsigned int> mHashColor2Pixel;
	HashMap<Ogre::String, unsigned int> mHashTiles2Pixel;
	HashMap<Ogre::String, unsigned int> mHashTiles2Count;
	HashMap<Ogre::String, unsigned int> mHashMatCount;
	int mSplitThresh;
	int mMergeThresh;

	Ogre::SceneManager* mBackSceneMgr;
	Ogre::Camera* mBackCamera;
	Ogre::RenderWindow* mBackWindow;
	Ogre::TexturePtr mBackTex;
	unsigned char *mBackMemTex;
	bool mRetrieveTex, mCountingTex;
	Ogre::Real mBackFovY;

	BBackLoadThread* mBackThread;
	bool mPreparation, mBackFramework, mBackRender;
	Ogre::Vector3 mInitEye, mInitTarget;
	Ogre::Vector2 mClipDist;
	Ogre::Vector2 mBackResolution;
	std::vector<Ogre::String> mPreparedTiles;
	std::vector<Ogre::String> mRenderedTiles;
	std::vector<Ogre::String> mUnloadedTiles;
	Ogre::LoggingLevel mLoggingLevel;
	OgreBites::CameraStyle mCameraStyle;
	Ogre::Vector3 mYawAxis;
	bool mInitLoadOver;

	Poco::Thread mCountingThread;
    CountingThread *mCountingWorker;

    void *mSharedMemCam;
    struct SHARED_CAM_DOF *mSharedSttCam;
    int mSharedIDCam;
    Ogre::Vector3 mCamPosFromForeend;
    Ogre::Vector3 mCamDirFromForeend;
    Ogre::Quaternion mCamQuatFromForeend;
    bool mOverFromForeend;

    void *mSharedMemVis;
    struct SHARED_TILE_VIS *mSharedSttVis;
    int mSharedIDVis;

    void *mSharedMemFed;
    struct SHARED_FED_BACK *mSharedSttFed;
    int mSharedIDFed;

    unsigned int mVisCacheCount;
    unsigned int mVisDispCount;

    libRQTS *mRQTS;
};

#endif
