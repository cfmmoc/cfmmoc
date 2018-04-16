/**
    `cfMMOC` library for terrain rendering using OGRE (https://github.com/cfmmoc/cfmmoc/)
    Licensed under The GNU General Public License v3.0 (GPLv3)
    Any modification, re-utilization or copy of the source or binary format in other software or publications should mention a CITATION of this library.
    Copyright (c) 2016-2018 by Authors (Jin Yan, Guanghong Gong, Ni Li and Luhao Xiao)
**/

#include <curl/curl.h>
#include "brender.h"
#include <sys/shm.h>

/**
  @brief  A class for scene management and terrain rendering in back-end process.
**/

/**
@remarks
	initialize parameters
**/
cfMMOCback::cfMMOCback()
{
	mInfo["Title"] = "cfMMOC-Back";
	mInfo["Description"] = "A Demo of cfMMOC.";
	mInfo["Thumbnail"] = "cfmmoc-back.png";
	mInfo["Category"] = "cfMMOC Terrain";
	
	// init pointers
	mBackSceneMgr = NULL;
	mBackCamera = NULL;
	mBackWindow = NULL;
	
	// init folder name for simplified mesh
	mSimMeshFolder = "sim/";

	// init status of loading initial tiles
	mInitLoadOver = false;
	
	// init point of camera and the point of look target
	mInitEye = Ogre::Vector3(-2296831, -3654685, 4765826);
	mInitTarget = Ogre::Vector3(2049684, -3919982, 4667978);
	
	// init distance of near and far clip plane, width and height of render to texuter, 
	// and the field of view of viewport in y direction
	mClipDist = Ogre::Vector2(100, 20000000);
	mBackResolution = Ogre::Vector2(384, 210);
	mBackFovY = 60;
	
	// init level of log detail and the style of camera movement
	mLoggingLevel = LL_LOW;
	mCameraStyle = CS_FREELOOK;

	// init splitting and merging thresholds in pixel
	mSplitThresh = 6000;
	mMergeThresh = 2000;

	// init shared memories and the sync status of fore-end process
	mSharedMemCam = NULL;
	mSharedMemVis = NULL;
	mSharedMemFed = NULL;
	mOverFromForeend = false;

	// init libcurl
	curl_global_init(CURL_GLOBAL_DEFAULT);
}

/**
@remarks
      	initialize discrete values for pseudo colors for rendering
**/
void cfMMOCback::initColorTable()
{
	for(int i = 0; i < 16; i++)
	{
		mColorTable[i] = 7 + i * 16;
	}
}

/**
@remarks
       	assign an unallocated color for given tile
@par
	filename	name of given tile
**/
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

/**
@remarks
        retrieve color which is not used for given tile
@par
	filename	name of given tile
**/
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

/**
@remarks
       	retrieve image from GPU, and copy it to memory
**/
void cfMMOCback::retrieveTexture()
{
	HardwarePixelBufferSharedPtr mTexBuf = mBackTex->getBuffer();

	mTexBuf->lock(HardwareBuffer::HBL_READ_ONLY);

	unsigned char* data = (unsigned char*)mTexBuf->getCurrentLock().data;

	memcpy(mBackMemTex, data, (unsigned int)(mBackResolution.x * mBackResolution.y * 4));

	mTexBuf->unlock();
}

/**
@remarks
	fire fetch request of tiles, 
	check all tiles that is already fetched, 
	load and split or merge them
**/
void cfMMOCback::checkLoadReq()
{
    Ogre::String name = mRQTS->checkfile();
    if (!name.empty())
    {
        mBackThread->fireRequestByName(name);
    }
    /**
    	obtain all fetched tiles, traverse them to load and split or merge corresponding tile
    **/
    std::vector<Ogre::String> filelist = mBackThread->getLoadedFilename();
    std::vector<Ogre::String>::iterator iter;
    for (iter = filelist.begin(); iter != filelist.end(); iter++)
    {
	Ogre::String filename = *iter;
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

/**
@remarks
	split given tile
@par
	filename	name of given tile
**/
void cfMMOCback::splitTile(Ogre::String filename)
{
	/**
		traverse all tiles in the scene, 
		find given tile, delete it, unload corresponding tile and retrieve corresponding color
	**/
	for (std::vector<Ogre::String>::iterator iter = mRenderedTiles.begin();
		iter != mRenderedTiles.end(); iter++)
	{
		if (*iter == filename)
		{
			mRenderedTiles.erase(iter);
			break;
		}
	}
        unloadBackTile(filename);
        retrieveColor(filename);

	/**
		create children for given tile, 
		and execute the process of splitting
	**/
	Ogre::String sub_name[4] = {"/q", "/r", "/s", "/t"};
	for (int i = 0; i < 4; i++)
	{
		mRenderedTiles.push_back(filename + sub_name[i]);
                createBackTile(filename + sub_name[i]);
	}
    	mRQTS->split(filename);
}

/**
@remarks
	split children of given tile
@par
	filename	name of given tile
**/
void cfMMOCback::mergeTile(Ogre::String filename)
{
	/**
		traverse all tiles in the scene, 
		find children of given tile, delete them, unload corresponding tiles and retrieve corresponding colors
	**/
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
            unloadBackTile(filename + sub_name[i]);
            retrieveColor(filename + sub_name[i]);
	}

	/**
		create given tile, 
		and execute the process of merging
	**/
	mRenderedTiles.push_back(filename);
        createBackTile(filename);
    	mRQTS->merge(filename);
}

/**
@remarks
      	load given tile into memory
@par
	filename	name of given tile
**/
void cfMMOCback::loadTile(Ogre::String filename)
{
	MeshPtr pMesh = static_cast<MeshPtr>(MeshManager::getSingleton().getByName(
		mSimMeshFolder + replacefile(filename) + Ogre::String(".mesh"), ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME));
	pMesh->load();
}

/**
@remarks
       	unload given tile
@par
	filename	name of given tile
**/
void cfMMOCback::unloadBackTile(Ogre::String filename)
{
    const HardwareBuffer::LockOptions DEST_LOCK_TYPE = HardwareBuffer::HBL_NORMAL;

    /**
    	unload and delete given from scene
    **/
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
    /**
    	unload and delete material for given tile
    **/
    {
	MaterialPtr mat = static_cast<MaterialPtr>(MaterialManager::getSingleton().getByName(filename +
        Ogre::StringConverter::toString(mHashMatCount[filename])));
	mat->unload();
	ResourcePtr pRes;
	pRes = static_cast<ResourcePtr>(mat);
	MaterialManager::getSingleton().remove(pRes);
    }
}

/**
@remarks
       	create given tile
@par
	filename	name of given tile
**/
void cfMMOCback::createBackTile(Ogre::String filename)
{

    	const unsigned int BACKMESH_SUBID = 0;
    	const HardwareBuffer::LockOptions DEST_LOCK_TYPE = HardwareBuffer::HBL_DISCARD;

	MeshPtr pMesh = static_cast<MeshPtr>(MeshManager::getSingleton().getByName(
		mSimMeshFolder + replacefile(filename) + Ogre::String(".mesh"), ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME));

	MeshPtr pCbMesh;
	SubMesh* pSubMesh;
	HardwareVertexBufferSharedPtr pVertexBuf;

	/**
		create manual mesh (name with back_$filename$) and corresponding submesh,
		and copy vertex data from external loaded mesh (name with $filename$)
	**/
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

	/**
		copy index data from external loaded mesh (name with $filename$) 
		to manual mesh (name with back_$filename$)
	**/
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

	/**
		create entity from mesh, 
		create scene node and attach the entity
		create material from a base material named 'tb_terra_sph_simple'
		set the color by a unallocated color in the default pass in material
	**/
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

/**
@remarks
       	rendering main loop
@par
	evt	FrameEvent object
**/
bool cfMMOCback::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
    /**
    	sync camera position and orientation from fore-end process
    **/
    if(mSharedSttCam->mWritable != 0)
    {
        mCamPosFromForeend = mSharedSttCam->mCamPos;
        mCamDirFromForeend = mSharedSttCam->mCamDir;
        mCamQuatFromForeend = mSharedSttCam->mCamQuat;
        mSharedSttCam->mWritable = 0;
    }

    /**
    	sync the status of fore-end process, if current status mOverFromForeend == false
    **/
    if(mSharedSttFed->mWritable != 0)
    {
        if (!mOverFromForeend)
        {
            mOverFromForeend = mSharedSttFed->mForeOver;
            mSharedSttFed->mWritable = 0;
        }
    }

    /**
    	send splitting or merging request from back-end process to fore-end process
    **/
    mRQTS->sendfile();

    /**
    	send visibility of tiles from back-end process to fore-end process
    **/
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
        if (count != 0)
            mSharedSttVis->mWritable = 1;
        else
            mSharedSttVis->mLength = 0;
    }

	/**
		retrieve texture from GPU after a time delay relateive to the starting of the process
	**/
        static unsigned int temp_delay = 0;
        if (temp_delay < 300)
        {
            temp_delay++;
        }
        if (temp_delay > 240)
        {
            retrieveTexture();
        }
	/**
		if updating restricted quadtrees is done and counting thread is idle, 
		then count pixel coverage of retrieved texture
		pixel coverage (a map from color to coverage) is returned by getCountingResults
		a copy of map from color to tile name is returned by getAnotherHashColor2Tile
		and map from color to tile name in counting thread is updated by fillinHashMaps accordingly
		sync status of fore-end process in couting thread is also updated by updateOverState
		finally, image to count is transfered to counting thread
	**/
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

	/**
		camera position and orientation is updated
		while a slight modification of orientation is applied
		which garateens viewing direction is not point to sky
		camera porision is also changed slightly (100 meters higher)
	**/
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
	/**
		if invalid camera position received from fore-end porcess, 
		set camera position and look target point to fixed positions
	**/
	if (mCamPosFromForeend.length() < 1000)
	{
        	mBackCamera->setPosition(10000000, 0, 0);
        	mBackCamera->lookAt(12000000, 0, 0);
	}

	/**
		check if fetching of initial tiles is done
		if done, fire load requests, check load requests, and split or merge tiles
	**/
	if (mInitLoadOver)
	{
        	if (!mRQTS->isdone())
		{
			checkLoadReq();
		}
	}
	/**
		if not, wait initial tiles to be fetched
		load and create corresponding fetched tiles
		if all initial tiles are fetched, mark that initial fetching is done
	**/
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
				loadTile(filename);
                		createBackTile(filename);
				mRenderedTiles.push_back(filename);
			}
		}
	}

	return SdkSample::frameRenderingQueued(evt);  // don't forget the parent class updates!
}


/**
@remarks
       	keyboard/mouse input event processing funcs
@par
	evt	FrameEvent object
**/
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


/**
@remarks
       	initialize scene manager and resources for back-end process
**/
void cfMMOCback::setupBackSceneMgr()
{
	/**
		initialize scene manager and camera
		initialize initial camera position and orientation
		initialize near and far distances of clip plane and field of view along y-direction
	**/
    	mBackSceneMgr = mSceneMgr;
    	mBackCamera = mCamera;

	mBackCamera->setPosition(mInitEye);
	mBackCamera->lookAt(mInitTarget);
	mBackCamera->setNearClipDistance(mClipDist.x);
	mBackCamera->setFarClipDistance(mClipDist.y);
	mBackCamera->setFOVy(Ogre::Degree(mBackFovY));

	/**
		initialize texture and render target for pixel counting
		allocate correspoding memory
	**/
	mBackTex = TextureManager::getSingleton().createManual("BackTexture",
		ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, TEX_TYPE_2D,
		mBackResolution.x, mBackResolution.y, 0, PF_R8G8B8, TU_RENDERTARGET);

	RenderTarget* rtt = mBackTex->getBuffer()->getRenderTarget();
	rtt->addViewport(mBackCamera);

	mBackMemTex = new unsigned char[(unsigned int)(mBackResolution.x * mBackResolution.y * 4)];

	/**
		initialize counting thread
	**/
    	mCountingWorker = new CountingThread(mBackResolution.x, mBackResolution.y, mSplitThresh, mMergeThresh, mRQTS);
	mCountingThread.start(*mCountingWorker);
}

/**
@remarks
       	cleanup scene manager, related resource and memory for back-end process
**/
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
        delete [] mBackMemTex;
}

/**
@remarks
       	initialize scene manager, shared memories and resources
**/
void cfMMOCback::setupContent()
{
	/**
		initialize shared memories and related variables
	**/
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

	/**
		initialize render window and restricted quadtrees logics
		initialize scene manager and resources for back-end process
		initialize style of camera movement and camera related parameters
		initialize overlay element and level of log detail
	**/
	mWindow->resize(mBackResolution.x, mBackResolution.y);
	mRQTS = new libRQTS();

        setupBackSceneMgr();

	mCameraMan->setStyle(mCameraStyle);
    	mCamera->setFixedYawAxis(false);
    	mCamera->setNearClipDistance(mClipDist.x);
    	mCamera->setFarClipDistance(mClipDist.y);

	mTrayMgr->hideCursor();
	mTrayMgr->hideLogo();
	mTrayMgr->hideFrameStats();

	Ogre::LogManager::getSingleton().setLogDetail(mLoggingLevel);

	/**
		initialize discrete values for pseudo colors for rendering
		initialize tile fetching object, fire initial tiles to be fetched
	**/
	initColorTable();

	mBackThread = new BBackLoadThread(mSimMeshFolder);

	mBackThread->fireRequestByName(Ogre::String("u"));
	mBackThread->fireRequestByName(Ogre::String("v"));
	mBackThread->fireRequestByName(Ogre::String("w"));
	mBackThread->fireRequestByName(Ogre::String("x"));
	mBackThread->fireRequestByName(Ogre::String("y"));
	mBackThread->fireRequestByName(Ogre::String("z"));

}

/**
@remarks
       	cleanup scene manager, shared memories and resources
**/
void cfMMOCback::cleanupContent()
{
    /**
    	cleanup restricted quadtrees logics
	cleanup shared memories
	cleanup scene manager, related resource and memory for back-end process
	cleanup tile fetching object
	cleanup libcurl
    **/
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
