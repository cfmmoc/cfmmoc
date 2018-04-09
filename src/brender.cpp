#include <curl/curl.h>
#include "brender.h"
#include <sys/shm.h>

cfMMOCback::cfMMOCback()
{
	mInfo["Title"] = "cfMMOC-Back";
	mInfo["Description"] = "A Demo of cfMMOC.";
	mInfo["Thumbnail"] = "cfmmoc-back.png";
	mInfo["Category"] = "cfMMOC Terrain";
	mBackSceneMgr = 0;
	mBackCamera = 0;
	mBackWindow = 0;

	mInitLoadOver = false;
	mPreparation = true;
	mBackFramework = true;
	mRetrieveTex = true;
	mCountingTex = true;
	mInitEye = Ogre::Vector3(-2296831, -3654685, 4765826);
	mInitTarget = Ogre::Vector3(2049684, -3919982, 4667978);
	mClipDist = Ogre::Vector2(100, 20000000);
	mBackResolution = Ogre::Vector2(384, 210);
	mBackFovY = 60;
	mLoggingLevel = LL_LOW;
	mCameraStyle = CS_FREELOOK;
	mYawAxis = Vector3::UNIT_Y;

	mSplitThresh = 6000;
	mMergeThresh = 2000;

	mSharedMemCam = NULL;

	mSharedMemVis = NULL;

	mSharedMemFed = NULL;
	mOverFromForeend = false;

	curl_global_init(CURL_GLOBAL_DEFAULT);

	mVisDispCount = 0;
}

void cfMMOCback::initColorTable()
{
	for(int i = 0; i < 16; i++)
	{
		mColorTable[i] = 7 + i * 16;
	}
}

Ogre::ColourValue cfMMOCback::getAnUnusedColor(Ogre::String filename)
{
	for (unsigned int num = 1; num < 4096; num++)
	{
		unsigned int id = 4095 - num;
		HashMap<unsigned int, Ogre::String>::iterator it = mHashColor2Tile.find(id);
		if (it == mHashColor2Tile.end())
		{
			mHashColor2Tile[id] = filename;
			unsigned int red = mColorTable[(num >> 0) & 15];
			unsigned int green = mColorTable[(num >> 4) & 15];
			unsigned int blue = mColorTable[(num >> 8) & 15];
			return Ogre::ColourValue(red / 255.0, green / 255.0, blue / 255.0);
		}
	}
	return Ogre::ColourValue(Ogre::Math::RangeRandom(0, 1), Ogre::Math::RangeRandom(0, 1), Ogre::Math::RangeRandom(0, 1), Ogre::Math::RangeRandom(0, 1));
}

void cfMMOCback::retrieveColor(Ogre::String filename)
{
	for (unsigned int num = 1; num < 4096; num++)
	{
		unsigned int id = 4095 - num;
		HashMap<unsigned int, Ogre::String>::iterator it = mHashColor2Tile.find(id);
		if (it != mHashColor2Tile.end() && it->second == filename)
		{
			mHashColor2Tile.erase(mHashColor2Tile.find(id));
			break;
		}
	}
}

void cfMMOCback::retrieveTexture()
{
	HardwarePixelBufferSharedPtr mTexBuf = mBackTex->getBuffer();

	mTexBuf->lock(HardwareBuffer::HBL_READ_ONLY);

	unsigned char* data = (unsigned char*)mTexBuf->getCurrentLock().data;

	memcpy(mBackMemTex, data, (unsigned int)(mBackResolution.x * mBackResolution.y * 4));

	mTexBuf->unlock();
}

void cfMMOCback::checkLoadReq()
{
    Ogre::String name = mRQTS->checkfile();
    if (!name.empty())
    {
        mBackThread->fireRequestByName(name, mPreparation);
    }
            std::vector<Ogre::String> filelist = mBackThread->getLoadedFilename();
			std::vector<Ogre::String>::iterator iter;
			for (iter = filelist.begin(); iter != filelist.end(); iter++)
			{
				Ogre::String filename = *iter;
				if (mPreparation)
				{
					loadTile(filename);

					Ogre::String name = mRQTS->getfile();
					if (!name.empty())
					{
                        if (name != "m")
                        {
                            splitTile(name);
                        }
                        else
                        {
                            mergeTile(filename);
                        }
					}
				}
			}
}

void cfMMOCback::splitTile(Ogre::String filename)
{

	for (std::vector<Ogre::String>::iterator iter = mRenderedTiles.begin();
		iter != mRenderedTiles.end(); iter++)
	{
		if (*iter == filename)
		{
			mRenderedTiles.erase(iter);
			break;
		}
	}
	if (mBackRender)
	{
        unloadBackTile(filename);
        retrieveColor(filename);
    }

	Ogre::String sub_name[4] = {"/q", "/r", "/s", "/t"};

	for (int i = 0; i < 4; i++)
	{
		mRenderedTiles.push_back(filename + sub_name[i]);
		if (mBackRender)
		{
            createBackTile(filename + sub_name[i]);
		}
	}

    mRQTS->split(filename);

}

void cfMMOCback::mergeTile(Ogre::String filename)
{

	Ogre::String sub_name[4] = {"/q", "/r", "/s", "/t"};

	for (int i = 0; i < 4; i++)
	{
		for (std::vector<Ogre::String>::iterator iter = mRenderedTiles.begin();
			iter != mRenderedTiles.end(); iter++)
		{
			if (*iter == filename + sub_name[i])
			{
				mRenderedTiles.erase(iter);
				break;
			}
		}
		if (mBackRender)
		{
            unloadBackTile(filename + sub_name[i]);
            retrieveColor(filename + sub_name[i]);
		}
	}

	mRenderedTiles.push_back(filename);
	if (mBackRender)
	{
        createBackTile(filename);
	}

    mRQTS->merge(filename);

}

void cfMMOCback::loadTile(Ogre::String filename)
{
	MeshPtr pMesh = static_cast<MeshPtr>(MeshManager::getSingleton().getByName(
		mSimMeshFolder + replacefile(filename) + Ogre::String(".mesh"), ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME));
	pMesh->load();
}

void cfMMOCback::unloadBackTile(Ogre::String filename)
{
    const HardwareBuffer::LockOptions DEST_LOCK_TYPE = HardwareBuffer::HBL_NORMAL;

    {
        {

            SceneNode *node = mBackSceneMgr->getSceneNode(Ogre::String("back_") + filename);
            node->detachAllObjects();
            mBackSceneMgr->destroySceneNode(Ogre::String("back_") + filename);
            mBackSceneMgr->destroyEntity(Ogre::String("back_") + filename);
            MeshPtr pMesh;
        	pMesh = static_cast<MeshPtr>(MeshManager::getSingleton().getByName(
        		mSimMeshFolder + replacefile(filename) + Ogre::String(".mesh"), ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME));
        	pMesh->unload();
            ResourcePtr pRes;
        	pRes = static_cast<ResourcePtr>(pMesh);
        	MeshManager::getSingleton().remove(pRes);
            pMesh = static_cast<MeshPtr>(MeshManager::getSingleton().getByName(
                Ogre::String("back_") + filename + Ogre::String(".mesh"), ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME));
            pMesh->unload();
            pRes = static_cast<ResourcePtr>(pMesh);
            MeshManager::getSingleton().remove(pRes);
        }
    }
    {
		MaterialPtr mat = static_cast<MaterialPtr>(MaterialManager::getSingleton().getByName(filename +
            Ogre::StringConverter::toString(mHashMatCount[filename])));
		mat->unload();
		ResourcePtr pRes;
		pRes = static_cast<ResourcePtr>(mat);
		MaterialManager::getSingleton().remove(pRes);
	}
}

void cfMMOCback::createBackTile(Ogre::String filename)
{

    const unsigned int BACKMESH_SUBID = 0;
    const HardwareBuffer::LockOptions DEST_LOCK_TYPE = HardwareBuffer::HBL_DISCARD;

	MeshPtr pMesh = static_cast<MeshPtr>(MeshManager::getSingleton().getByName(
		mSimMeshFolder + replacefile(filename) + Ogre::String(".mesh"), ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME));

	MeshPtr pCbMesh;
	SubMesh* pSubMesh;
	HardwareVertexBufferSharedPtr pVertexBuf;

	{
        pCbMesh = MeshManager::getSingleton().createManual(
            Ogre::String("back_") + filename + Ogre::String(".mesh"), ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME);
        pSubMesh = pCbMesh->createSubMesh("0");
        pSubMesh->vertexData = new VertexData();
        pSubMesh->vertexData->vertexCount = pMesh->getSubMesh(BACKMESH_SUBID)->vertexData->vertexCount;
        pSubMesh->vertexData->vertexDeclaration->addElement(0, 0, VET_FLOAT3, VES_POSITION);
        pSubMesh->vertexData->vertexDeclaration->addElement(0, 12, VET_FLOAT3, VES_TANGENT);

        pVertexBuf = HardwareBufferManager::getSingleton().createVertexBuffer(
            pSubMesh->vertexData->vertexDeclaration->getVertexSize(0),
            pMesh->getSubMesh(BACKMESH_SUBID)->vertexData->vertexCount,
            HardwareBuffer::HBU_STATIC_WRITE_ONLY);
        pSubMesh->vertexData->vertexBufferBinding->setBinding(0, pVertexBuf);
	}

	float* pSrcVertex = static_cast<float*>(
		pMesh->getSubMesh(BACKMESH_SUBID)->vertexData->vertexBufferBinding->getBuffer(0)->lock(
		HardwareBuffer::HBL_DISCARD));
	float* pDstVertex = static_cast<float*>(
		pSubMesh->vertexData->vertexBufferBinding->getBuffer(0)->lock(0,
		pMesh->getSubMesh(BACKMESH_SUBID)->vertexData->vertexBufferBinding->getBuffer(0)->getSizeInBytes(),
		DEST_LOCK_TYPE));
	memcpy(pDstVertex, pSrcVertex,
		pMesh->getSubMesh(BACKMESH_SUBID)->vertexData->vertexBufferBinding->getBuffer(0)->getSizeInBytes());
	pMesh->getSubMesh(BACKMESH_SUBID)->vertexData->vertexBufferBinding->getBuffer(0)->unlock();
	pSubMesh->vertexData->vertexBufferBinding->getBuffer(0)->unlock();

	size_t indexCount = pMesh->getSubMesh(BACKMESH_SUBID)->indexData->indexCount;
    {
       	pSubMesh->indexData->indexCount = indexCount;
        pSubMesh->indexData->indexBuffer =
            HardwareBufferManager::getSingleton().createIndexBuffer(
            HardwareIndexBuffer::IT_16BIT, indexCount,
            HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
    }
	unsigned short* pDstIndices = static_cast<unsigned short*>(
		pSubMesh->indexData->indexBuffer->lock(0,
		pMesh->getSubMesh(BACKMESH_SUBID)->indexData->indexBuffer->getSizeInBytes(),
		DEST_LOCK_TYPE));
	unsigned short* pSrcIndices= static_cast<unsigned short*>(
        pMesh->getSubMesh(BACKMESH_SUBID)->indexData->indexBuffer->lock(HardwareBuffer::HBL_DISCARD));
	memcpy(pDstIndices, pSrcIndices,
		pMesh->getSubMesh(BACKMESH_SUBID)->indexData->indexBuffer->getSizeInBytes());
	pMesh->getSubMesh(BACKMESH_SUBID)->indexData->indexBuffer->unlock();
	pSubMesh->indexData->indexBuffer->unlock();
	pSubMesh->useSharedVertices = false;

	pCbMesh->_setBounds(pMesh->getBounds());
	pCbMesh->load();

    {
        Entity *ent = mBackSceneMgr->createEntity(Ogre::String("back_") + filename,
            Ogre::String("back_") + filename + Ogre::String(".mesh"));
        SceneNode *node = mBackSceneMgr->getRootSceneNode()->createChildSceneNode(Ogre::String("back_") + filename);
        node->attachObject(ent);
        MaterialPtr basemat = Ogre::MaterialManager::getSingletonPtr()->getByName("tb_terra_sph_simple");
        HashMap<Ogre::String, unsigned int>::iterator matit = mHashMatCount.find(filename);
        if (matit == mHashMatCount.end())
        {
            mHashMatCount[filename] = 0;
        }
        else
        {
            mHashMatCount[filename]++;
        }
        MaterialPtr mat = basemat->clone(filename +
            Ogre::StringConverter::toString(mHashMatCount[filename]));
        Ogre::ColourValue color = getAnUnusedColor(filename);
			mat.getPointer()->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setColourOperationEx(
			LBX_SOURCE1, LBS_MANUAL, LBS_CURRENT, color);
		ent->setMaterial(mat);
    }

}

bool cfMMOCback::frameRenderingQueued(const Ogre::FrameEvent& evt)
{

    if(mSharedSttCam->mWritable != 0)
    {
        mCamPosFromForeend = mSharedSttCam->mCamPos;
        mCamDirFromForeend = mSharedSttCam->mCamDir;
        mCamQuatFromForeend = mSharedSttCam->mCamQuat;
        mSharedSttCam->mWritable = 0;
    }

    if(mSharedSttFed->mWritable != 0)
    {
        if (!mOverFromForeend)
        {
            mOverFromForeend = mSharedSttFed->mForeOver;
            mSharedSttFed->mWritable = 0;
        }
    }

    mRQTS->sendfile();

    if (mSharedSttVis->mWritable != 1)
    {
        mSharedSttVis->mLength = 0;
        for (HashMap<unsigned int, Ogre::String>::iterator it = mAnotherHashColor2Tile.begin();
            it != mAnotherHashColor2Tile.end(); it++)
        {
            int color = it->first;
            Ogre::String name = it->second;
            mSharedSttVis->mVisible[mSharedSttVis->mLength] = mHashColor2Pixel[color] > 15;
            mSharedSttVis->mPixel[mSharedSttVis->mLength] = mHashColor2Pixel[color];
            strcpy(mSharedSttVis->mFilename[mSharedSttVis->mLength],
                name.c_str());
            mSharedSttVis->mLength++;
        }
        unsigned int count = 0;
        for (unsigned int i = 0; i < mSharedSttVis->mLength; i++)
        {
            if (mSharedSttVis->mVisible[i])
                count++;
        }
        mVisCacheCount = count;
        if (mVisCacheCount != 0)
            mSharedSttVis->mWritable = 1;
        else
            mSharedSttVis->mLength = 0;
    }

	if (mRetrieveTex)
	{
        static unsigned int temp_delay = 0;
        if (temp_delay < 300)
        {
            temp_delay++;
        }
        if (temp_delay > 240)
        {
            retrieveTexture();
        }
		if (mCountingTex)
		{
            if (mRQTS->isdone())
            {
                if (mCountingWorker->isSleeping())
                {
                    mHashColor2Pixel = mCountingWorker->getCountingResults();
                    mAnotherHashColor2Tile = mCountingWorker->getAnotherHashColor2Tile();
                    mCountingWorker->fillinHashMaps(mHashColor2Tile);
                    if (mCountingWorker->updateOverState(mOverFromForeend))
                    {
                        mOverFromForeend = false;
                    }
                }
                if (mRQTS->isdone() && mCountingWorker->isSleeping())
                {
                    mCountingWorker->copyTexture(mBackMemTex);
                }
            }
		}
	}

	{
        Ogre::Vector3 campos = mCamPosFromForeend;
		mBackCamera->setPosition(campos + campos.normalisedCopy() * 100);
		Ogre::Vector3 camdir = mCamDirFromForeend;
		Ogre::Vector3 camtoearth = -mCamPosFromForeend;
		camdir.normalise();
		camtoearth.normalise();
		Ogre::Quaternion camquat = mCamQuatFromForeend;
		if (camdir.dotProduct(camtoearth) < 0)
		{
			Ogre::Vector3 camnormal = camdir.crossProduct(camtoearth);
			camnormal.normalise();
			Ogre::Vector3 camhori = camtoearth.crossProduct(camnormal);
			camhori.normalise();
			camquat = camdir.getRotationTo(camhori);
			camquat = camquat * mCamQuatFromForeend;
		}
		mBackCamera->setOrientation(camquat);
	}
	if (mCamPosFromForeend.length() < 1000)
	{
        mBackCamera->setPosition(10000000, 0, 0);
        mBackCamera->lookAt(12000000, 0, 0);
	}

	if (mInitLoadOver)
	{
        if (!mRQTS->isdone())
		{
			checkLoadReq();
		}
	}
	else
	{
		if (mBackThread->isQueueEmpty())
		{
			mInitLoadOver = true;
		}
		else
		{
			std::vector<Ogre::String> filelist = mBackThread->getLoadedFilename();
			std::vector<Ogre::String>::iterator iter;
			for (iter = filelist.begin(); iter != filelist.end(); iter++)
			{
				Ogre::String filename = *iter;
				if (mPreparation)
				{
					loadTile(filename);
				}
                createBackTile(filename);
				mRenderedTiles.push_back(filename);
			}
		}
	}

//	return true;
	return SdkSample::frameRenderingQueued(evt);  // don't forget the parent class updates!
}

bool cfMMOCback::keyPressed(const OIS::KeyEvent& evt)
{
    return true;
}

bool cfMMOCback::keyReleased(const OIS::KeyEvent& evt)
{
    return true;
}

bool cfMMOCback::mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
{
    return true;
}

bool cfMMOCback::mouseReleased(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
{
    return true;
}

bool cfMMOCback::mouseMoved(const OIS::MouseEvent& evt)
{
    return true;
}

void cfMMOCback::setupBackSceneMgr()
{
    mBackSceneMgr = mSceneMgr;
    mBackCamera = mCamera;

	mBackCamera->setPosition(mInitEye);
	mBackCamera->lookAt(mInitTarget);
	mBackCamera->setNearClipDistance(mClipDist.x);
	mBackCamera->setFarClipDistance(mClipDist.y);
	mBackCamera->setFOVy(Ogre::Degree(mBackFovY));

	mBackTex = TextureManager::getSingleton().createManual("BackTexture",
		ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, TEX_TYPE_2D,
		mBackResolution.x, mBackResolution.y, 0, PF_R8G8B8, TU_RENDERTARGET);

	RenderTarget* rtt = mBackTex->getBuffer()->getRenderTarget();
	rtt->addViewport(mBackCamera);

	mBackMemTex = new unsigned char[(unsigned int)(mBackResolution.x * mBackResolution.y * 4)];

    mCountingWorker = new CountingThread(mBackResolution.x, mBackResolution.y, mSplitThresh, mMergeThresh, mRQTS);
	mCountingThread.start(*mCountingWorker);
}

void cfMMOCback::cleanupBackSceneMgr()
{
	if (mBackSceneMgr)
	{
		mSceneMgr->destroyAllCameras();
		mBackSceneMgr->clearScene();
		Ogre::Root::getSingleton().destroySceneManager(mBackSceneMgr);
		mBackSceneMgr = 0;
	}
	if (mBackWindow)
	{
        mBackWindow->removeAllViewports();
	}
	if (mBackFramework)
	{
        delete [] mBackMemTex;
	}
}

void cfMMOCback::setupContent()
{

	mSharedIDCam = shmget((key_t)1234, sizeof(struct SHARED_CAM_DOF), 0666|IPC_CREAT);
	mSharedMemCam = shmat(mSharedIDCam, 0, 0);
	mSharedSttCam = (struct SHARED_CAM_DOF*)mSharedMemCam;

	mSharedIDVis = shmget((key_t)1558, sizeof(struct SHARED_TILE_VIS), 0666|IPC_CREAT);
	mSharedMemVis = shmat(mSharedIDVis, 0, 0);
	mSharedSttVis = (struct SHARED_TILE_VIS*)mSharedMemVis;
	mSharedSttVis->mLength = 0;

	mSharedIDFed = shmget((key_t)1678, sizeof(struct SHARED_FED_BACK), 0666|IPC_CREAT);
	mSharedMemFed = shmat(mSharedIDFed, (void*)0, 0);
	mSharedSttFed = (struct SHARED_FED_BACK*)mSharedMemFed;

	mWindow->resize(mBackResolution.x, mBackResolution.y);
	mRQTS = new libRQTS();

	if (mBackFramework)
	{
        setupBackSceneMgr();
	}

	mCameraMan->setStyle(mCameraStyle);
	mTrayMgr->hideCursor();
	mTrayMgr->hideLogo();
	mTrayMgr->hideFrameStats();

	Ogre::LogManager::getSingleton().setLogDetail(mLoggingLevel);

	initColorTable();

	mBackThread = new BBackLoadThread(mTextureExtension, mSimMeshFolder);

	mBackThread->fireRequestByName(Ogre::String("u"), mPreparation);
	mBackThread->fireRequestByName(Ogre::String("v"), mPreparation);
	mBackThread->fireRequestByName(Ogre::String("w"), mPreparation);
	mBackThread->fireRequestByName(Ogre::String("x"), mPreparation);
	mBackThread->fireRequestByName(Ogre::String("y"), mPreparation);
	mBackThread->fireRequestByName(Ogre::String("z"), mPreparation);

    mCamera->setFixedYawAxis(false);
    mCamera->setNearClipDistance(mClipDist.x);
    mCamera->setFarClipDistance(mClipDist.y);

}

void cfMMOCback::cleanupContent()
{

    delete mRQTS;

    shmdt(mSharedMemCam);
    shmctl(mSharedIDCam, IPC_RMID, 0);

    shmdt(mSharedMemVis);
    shmctl(mSharedIDVis, IPC_RMID, 0);

    shmdt(mSharedMemFed);
    shmctl(mSharedIDFed, IPC_RMID, 0);

	cleanupBackSceneMgr();

	delete mBackThread;

	curl_global_cleanup();
}
