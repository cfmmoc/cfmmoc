/**
    `cfMMOC` library for terrain rendering using OGRE (https://github.com/cfmmoc/cfmmoc/)
    Licensed under The GNU General Public License v3.0 (GPLv3)
    Any modification, re-utilization or copy of the source or binary format in other software or publications should mention a CITATION of this library.
    Copyright (c) 2016-2018 by Authors (Jin Yan, Guanghong Gong, Ni Li and Luhao Xiao)
**/

#include <curl/curl.h>
#include "frender.h"
#include <sys/shm.h>

/**
  @brief  A class for scene management and terrain rendering in fore-end process.
**/

/**
@remarks
	initialize parameters
**/
cfMMOCfore::cfMMOCfore()
{
	mInfo["Title"] = "cfMMOC-Fore";
	mInfo["Description"] = "A Demo of cfMMOC.";
	mInfo["Thumbnail"] = "cfmmoc-fore.png";
	mInfo["Category"] = "cfMMOC Terrain";
	
	// init folder name and texture extension for mesh and texture
	mComMeshFolder = "com/";
	mTexFolder = "tex/";
	mTextureExtension = ".dds";

	// init status of loading initial tiles
	mInitLoadOver = false;
	
	// init point of camera and the point of look target
	mInitEye = Ogre::Vector3(3000000, 700000, 5800000);
	mInitTarget = Ogre::Vector3(0, 0, 0);

	// init distance of near and far clip plane, 
	// level of log detail and the style of camera movement
	mClipDist = Ogre::Vector2(100, 20000000);
	mLoggingLevel = LL_LOW;
	mCameraStyle = CS_FREELOOK;

	// init shared memories, the sync status of fore-end process, 
	// and the length of the copy of visibility of tiles
	mSharedMemCam = NULL;
	mSharedMemVis = NULL;
	mSharedMemFed = NULL;
	mForeOver = false;
	mTilesVisbleReg.mLength = 0;

	// init libcurl
	curl_global_init(CURL_GLOBAL_DEFAULT);

	// init the starting point of the orbit around the Earth
	artorbit = 0.0;
}

/**
@remarks
	fire fetch request of tiles, 
	check all tiles that is already fetched, 
	load and split or merge them
**/
void cfMMOCfore::checkLoadReq()
{
    Ogre::String name = mRQTS->checkfile();
    if (!name.empty())
    {
        mBackThread->fireRequestByName(name);
    }
    /**
    	obtain all fetched tiles, traverse them to load and split or merge corresponding tile
	if updating restricted quadtrees is done, mark fore-end status (mForeOver) is true
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
		       /**
		       		crack fixing on boundary
				check four neighbor tiles, if density of triangle on boundary is needed to update, 
				then unload neighbor tile(s) and recreate the corresponding tile(s)
		       **/
                      for (unsigned int i = 0; i < 4; i++)
                      {
                                unsigned char mask = 0;
                                std::string neighbor = mRQTS->checkbound(name, i, &mask);
                                if (mask != 0)
                                {
					unloadTile(neighbor, false);
					createTile(neighbor, mask);
				}
                      }
                      splitTile(name);
                }
		else
		{
		      mergeTile(filename);
		}
                if (mRQTS->isdone())
		{
                      mForeOver = true;
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
void cfMMOCfore::splitTile(Ogre::String filename)
{
	/**
		unload given tile
		traverse all tiles in the scene, 
		find given tile, delete it
	**/
	unloadTile(filename);
	for (std::vector<Ogre::String>::iterator iter = mRenderedTiles.begin();
		iter != mRenderedTiles.end(); iter++)
	{
		if (*iter == filename)
		{
			mRenderedTiles.erase(iter);
			break;
		}
	}

	/**
		create children for given tile, 
		and execute the process of splitting
	**/
	Ogre::String sub_name[4] = {"/q", "/r", "/s", "/t"};
	for (int i = 0; i < 4; i++)
	{
		createTile(filename + sub_name[i]);
		mRenderedTiles.push_back(filename + sub_name[i]);
	}
	mRQTS->split(filename);
}

/**
@remarks
	split children of given tile
@par
	filename	name of given tile
**/
void cfMMOCfore::mergeTile(Ogre::String filename)
{
	/**
		unload children of given til
		traverse all tiles in the scene, 
		find children of given tile, delete them
	**/
	Ogre::String sub_name[4] = {"/q", "/r", "/s", "/t"};
	for (int i = 0; i < 4; i++)
	{
		unloadTile(filename + sub_name[i]);
		for (std::vector<Ogre::String>::iterator iter = mRenderedTiles.begin();
			iter != mRenderedTiles.end(); iter++)
		{
			if (*iter == filename + sub_name[i])
			{
				mRenderedTiles.erase(iter);
				break;
			}
		}
	}

	/**
		create given tile, 
		and execute the process of merging
	**/
	createTile(filename, 15);
	mRenderedTiles.push_back(filename);
        mRQTS->merge(filename);

}

/**
@remarks
      	load given tile into memory
@par
	filename	name of given tile
**/
void cfMMOCfore::loadTile(Ogre::String filename)
{
	MeshPtr pMesh = static_cast<MeshPtr>(MeshManager::getSingleton().getByName(
		mComMeshFolder + filename + Ogre::String(".mesh"), ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME));
	pMesh->load();
	TexturePtr pTex = static_cast<TexturePtr>(TextureManager::getSingleton().getByName(
		mTexFolder + filename + mTextureExtension, ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME));
	pTex->load();
}

/**
@remarks
       	unload given tile
@par
	filename	name of given tile
	full		boolean indicates fully remove the all corresponding meshes
			(external mesh and boundary adaptive mesh), material and texture
			or just remove boundary adaptive mesh and material
**/
void cfMMOCfore::unloadTile(Ogre::String filename, bool full)
{
	/**
    		unload and delete given from scene
	**/
	SceneNode *node = mSceneMgr->getSceneNode(filename);
	node->detachAllObjects();
	mSceneMgr->destroySceneNode(filename);
	mSceneMgr->destroyEntity(filename);
	MeshPtr pMesh;
	ResourcePtr pRes;
	/**
    		unload and delete external mesh
	**/
	if (full)
	{
	        pMesh = static_cast<MeshPtr>(MeshManager::getSingleton().getByName(
        	    mComMeshFolder + filename + Ogre::String(".mesh"), ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME));
        	pMesh->unload();
        	pRes = static_cast<ResourcePtr>(pMesh);
        	MeshManager::getSingleton().remove(pRes);
	}
    	/**
    		unload and delete boundary adaptive mesh
    	**/
	pMesh = static_cast<MeshPtr>(MeshManager::getSingleton().getByName(
		Ogre::String("combined_") + filename + Ogre::String(".mesh"), ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME));
	pMesh->unload();
	pRes = static_cast<ResourcePtr>(pMesh);
	MeshManager::getSingleton().remove(pRes);
    	/**
    		unload and delete material for given tile
    	**/
	{
		MaterialPtr mat = static_cast<MaterialPtr>(MaterialManager::getSingleton().getByName(filename +
            		Ogre::StringConverter::toString(mHashMatCount[filename])));
		mat->unload();
		pRes = static_cast<ResourcePtr>(mat);
		MaterialManager::getSingleton().remove(pRes);
	}
    	/**
    		unload and delete texture for given tile
    	**/
	if (full)
	{
		TexturePtr pTex = static_cast<TexturePtr>(TextureManager::getSingleton().getByName(
			mTexFolder + filename + mTextureExtension, ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME));
		pTex->unload();
		pRes = static_cast<ResourcePtr>(pTex);
		TextureManager::getSingleton().remove(pRes);
	}
}

/**
@remarks
       	create given tile
@par
	filename	name of given tile
	mask		bitwise variable indicates the density of triangles on boundaries
			0000b means low density
			1111b means high density
**/
void cfMMOCfore::createTile(Ogre::String filename, unsigned char mask)
{
	MeshPtr pMesh;
        pMesh = static_cast<MeshPtr>(MeshManager::getSingleton().getByName(
            mComMeshFolder + filename + Ogre::String(".mesh"), ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME));
	MeshPtr pCbMesh;

	/**
		create manual mesh (name with combined_$filename$) and corresponding submesh,
		and copy vertex data from external loaded mesh (name with $filename$)
	**/
	{
	        pCbMesh = MeshManager::getSingleton().createManual(
        	    Ogre::String("combined_") + filename + Ogre::String(".mesh"), ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME);
	}
	pCbMesh->sharedVertexData = new VertexData();
	pCbMesh->sharedVertexData->vertexCount = pMesh->sharedVertexData->vertexCount;
	pCbMesh->sharedVertexData->vertexDeclaration->addElement(0, 0, VET_FLOAT3, VES_POSITION);
	pCbMesh->sharedVertexData->vertexDeclaration->addElement(0, 12, VET_FLOAT2, VES_TEXTURE_COORDINATES);
	pCbMesh->sharedVertexData->vertexDeclaration->addElement(0, 21, VET_FLOAT3, VES_TANGENT);

	HardwareVertexBufferSharedPtr pVertexBuf;
	pVertexBuf = HardwareBufferManager::getSingleton().createVertexBuffer(
		pCbMesh->sharedVertexData->vertexDeclaration->getVertexSize(0),
		pMesh->sharedVertexData->vertexCount,
		HardwareBuffer::HBU_STATIC_WRITE_ONLY);
	pCbMesh->sharedVertexData->vertexBufferBinding->setBinding(0, pVertexBuf);
	float* pSrcVertex = static_cast<float*>(
		pMesh->sharedVertexData->vertexBufferBinding->getBuffer(0)->lock(
		HardwareBuffer::HBL_DISCARD));
	float* pDstVertex = static_cast<float*>(
		pCbMesh->sharedVertexData->vertexBufferBinding->getBuffer(0)->lock(
		HardwareBuffer::HBL_DISCARD));
	memcpy(pDstVertex, pSrcVertex,
		pMesh->sharedVertexData->vertexBufferBinding->getBuffer(0)->getSizeInBytes());
	pMesh->sharedVertexData->vertexBufferBinding->getBuffer(0)->unlock();
	pCbMesh->sharedVertexData->vertexBufferBinding->getBuffer(0)->unlock();

    int mask_offset[4] = {
        (mask & 2) ? 4:0,
        (mask & 1) ? 4:0,
        (mask & 8) ? 4:0,
        (mask & 4) ? 4:0};

	/**
		copy index data from external loaded mesh (name with $filename$) 
		to manual mesh (name with combined_$filename$)
	**/
	SubMesh* pSubMesh = pCbMesh->createSubMesh("0");
	size_t indexCount = pMesh->getSubMesh(0)->indexData->indexCount +
		pMesh->getSubMesh(1 + mask_offset[0])->indexData->indexCount +
		pMesh->getSubMesh(2 + mask_offset[1])->indexData->indexCount +
		pMesh->getSubMesh(3 + mask_offset[2])->indexData->indexCount +
		pMesh->getSubMesh(4 + mask_offset[3])->indexData->indexCount;
	pSubMesh->indexData->indexCount = indexCount;

	pSubMesh->indexData->indexBuffer =
		HardwareBufferManager::getSingleton().createIndexBuffer(
		HardwareIndexBuffer::IT_16BIT, indexCount,
		HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
	unsigned short* pDstIndices = static_cast<unsigned short*>(
		pSubMesh->indexData->indexBuffer->lock(HardwareBuffer::HBL_DISCARD));
	unsigned short* pSrcIndices= static_cast<unsigned short*>(
		pMesh->getSubMesh(0)->indexData->indexBuffer->lock(HardwareBuffer::HBL_DISCARD));
	memcpy(pDstIndices, pSrcIndices,
		pMesh->getSubMesh(0)->indexData->indexBuffer->getSizeInBytes());
	pMesh->getSubMesh(0)->indexData->indexBuffer->unlock();
	size_t indexOffset = pMesh->getSubMesh(0)->indexData->indexCount;
	/**
		manully process boundaries for given variable mask
	**/
	for (int i = 1; i <= 4; i++)
	{
		unsigned short* pSrcIndices= static_cast<unsigned short*>(
			pMesh->getSubMesh(i + mask_offset[i - 1])->indexData->indexBuffer->lock(HardwareBuffer::HBL_DISCARD));
		memcpy(pDstIndices + indexOffset, pSrcIndices,
			pMesh->getSubMesh(i + mask_offset[i - 1])->indexData->indexBuffer->getSizeInBytes());
		indexOffset += pMesh->getSubMesh(i + mask_offset[i - 1])->indexData->indexCount;
		pMesh->getSubMesh(i + mask_offset[i - 1])->indexData->indexBuffer->unlock();
	}
	pSubMesh->indexData->indexBuffer->unlock();
	pSubMesh->useSharedVertices = true;

	pCbMesh->_setBounds(pMesh->getBounds());
	pCbMesh->load();

	/**
		create entity from mesh, 
		create scene node and attach the entity
		create material from a base material named 'tb_terra_sph_simple'
		set the color by a unallocated color in the default pass in material
	**/
    	Entity *ent;
	{
		ent = mSceneMgr->createEntity(filename,
		    Ogre::String("combined_") + filename + Ogre::String(".mesh"));
		SceneNode *node = mSceneMgr->getRootSceneNode()->createChildSceneNode(filename);
		node->attachObject(ent);
		node->showBoundingBox(true);
	}
	{
        	MaterialPtr mat;
		MaterialPtr basemat = Ogre::MaterialManager::getSingletonPtr()->getByName("tb_terra_sph_complex");
		{
		    HashMap<Ogre::String, unsigned int>::iterator matit = mHashMatCount.find(filename);
		    if (matit == mHashMatCount.end())
		    {
			mHashMatCount[filename] = 0;
		    }
		    else
		    {
			mHashMatCount[filename]++;
		    }
		    mat = basemat->clone(filename + Ogre::StringConverter::toString(mHashMatCount[filename]));
		}
		mat.getPointer()->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName(
			mTexFolder + filename + mTextureExtension);
		ent->setMaterial(mat);
	}
}

/**
@remarks
       	rendering main loop
@par
	evt	FrameEvent object
**/
bool cfMMOCfore::frameRenderingQueued(const Ogre::FrameEvent& evt)
{

    if (mSharedSttCam->mWritable != 1)
    {
        mSharedSttCam->mCamPos = mCamera->getDerivedPosition();
        mSharedSttCam->mCamDir = mCamera->getDerivedDirection();
        mSharedSttCam->mCamQuat = mCamera->getDerivedOrientation();
        mSharedSttCam->mWritable = 1;
    }

    if (mSharedSttFed->mWritable != 1)
    {
        mSharedSttFed->mForeOver = mForeOver;
        mSharedSttFed->mWritable = 1;
    }

    if (mRQTS->recvfile())
        mForeOver = false;

    if(mSharedSttVis->mWritable != 0)
    {
        memcpy(&mTilesVisbleReg, mSharedSttVis, sizeof(struct SHARED_TILE_VIS));
        mSharedSttVis->mLength = 0;
        mSharedSttVis->mWritable = 0;
    }

    artorbit += evt.timeSinceLastFrame / 60;
    if (artorbit > 2 * Ogre::Math::PI)
        artorbit -= 2 * Ogre::Math::PI;
    float arty = Ogre::Math::PI / 3 * Ogre::Math::Sin(Ogre::Radian(2 * artorbit / 3));
    float alt = 13500000 + 6500000 * Ogre::Math::Cos(Ogre::Radian(artorbit * 6));
    Ogre::Vector3 artpos = alt * Ogre::Vector3(Ogre::Math::Cos(Ogre::Radian(arty)) * Ogre::Math::Cos(Ogre::Radian(artorbit)),
        Ogre::Math::Cos(Ogre::Radian(arty)) * Ogre::Math::Sin(Ogre::Radian(artorbit)), Ogre::Math::Sin(Ogre::Radian(arty)));

    mCamera->setPosition(artpos);

    float tarorbit = artorbit + 0.006;
    if (tarorbit > 2 * Ogre::Math::PI)
        tarorbit -= 2 * Ogre::Math::PI;
    arty = Ogre::Math::PI / 3 * Ogre::Math::Sin(Ogre::Radian(2 * tarorbit / 3));
    alt = 8000000 + 6000000 * Ogre::Math::Cos(Ogre::Radian(tarorbit * 6));
    artpos = alt * Ogre::Vector3(Ogre::Math::Cos(Ogre::Radian(arty)) * Ogre::Math::Cos(Ogre::Radian(tarorbit)),
        Ogre::Math::Cos(Ogre::Radian(arty)) * Ogre::Math::Sin(Ogre::Radian(tarorbit)), Ogre::Math::Sin(Ogre::Radian(arty)));
    mCamera->lookAt(artpos);

#if 1
    {
        for (unsigned int i = 0; i < mTilesVisbleReg.mLength; i++)
        {
            Ogre::String name(mTilesVisbleReg.mFilename[i]);
            if (mSceneMgr->hasSceneNode(name))
            {
                mSceneMgr->getSceneNode(name)->setVisible(mTilesVisbleReg.mVisible[i]);
            }
            else if (mTilesVisbleReg.mVisible[i])
            {
                Ogre::String tempname = name;
                if (tempname.length() > 2)
                {
                    tempname.erase(tempname.end() - 1);
                    tempname.erase(tempname.end() - 1);
                    if (mSceneMgr->hasSceneNode(tempname))
                    {
                        mSceneMgr->getSceneNode(tempname)->setVisible(true);
                    }
                    else
                    {
                        if (mSceneMgr->hasSceneNode(name + "/q"))
                            mSceneMgr->getSceneNode(name + "/q")->setVisible(true);
                        if (mSceneMgr->hasSceneNode(name + "/r"))
                            mSceneMgr->getSceneNode(name + "/r")->setVisible(true);
                        if (mSceneMgr->hasSceneNode(name + "/s"))
                            mSceneMgr->getSceneNode(name + "/s")->setVisible(true);
                        if (mSceneMgr->hasSceneNode(name + "/t"))
                            mSceneMgr->getSceneNode(name + "/t")->setVisible(true);
                    }
                }
            }
        }
    }
#endif

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
			mForeOver = true;
		}
		else
		{
			std::vector<Ogre::String> filelist = mBackThread->getLoadedFilename();
			std::vector<Ogre::String>::iterator iter;
			for (iter = filelist.begin(); iter != filelist.end(); iter++)
			{
				Ogre::String filename = *iter;
				loadTile(filename);
		                createTile(filename);
				mRenderedTiles.push_back(filename);
			}
		}
	}

    return SdkSample::frameRenderingQueued(evt);  // don't forget the parent class updates!
}

#if 1

bool cfMMOCfore::keyPressed(const OIS::KeyEvent& evt)
{
    return true;
}

bool cfMMOCfore::keyReleased(const OIS::KeyEvent& evt)
{
    return true;
}

bool cfMMOCfore::mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
{
    return true;
}

bool cfMMOCfore::mouseReleased(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
{
    return true;
}

bool cfMMOCfore::mouseMoved(const OIS::MouseEvent& evt)
{
    return true;
}

#endif // 1

void cfMMOCfore::setupContent()
{

    mSharedIDCam = shmget((key_t)1234, sizeof(struct SHARED_CAM_DOF), 0666|IPC_CREAT);
	mSharedMemCam = shmat(mSharedIDCam, (void*)0, 0);
	mSharedSttCam = (struct SHARED_CAM_DOF*)mSharedMemCam;

	mSharedIDVis = shmget((key_t)1558, sizeof(struct SHARED_TILE_VIS), 0666|IPC_CREAT);
	mSharedMemVis = shmat(mSharedIDVis, (void*)0, 0);
	mSharedSttVis = (struct SHARED_TILE_VIS*)mSharedMemVis;
	mSharedSttVis->mLength = 0;

	mSharedIDFed = shmget((key_t)1678, sizeof(struct SHARED_FED_BACK), 0666|IPC_CREAT);
	mSharedMemFed = shmat(mSharedIDFed, (void*)0, 0);
	mSharedSttFed = (struct SHARED_FED_BACK*)mSharedMemFed;

	mWindow->resize(1280, 700);

	mCameraMan->setStyle(mCameraStyle);
	mTrayMgr->hideCursor();
	mTrayMgr->hideLogo();
	mTrayMgr->hideFrameStats();

	Ogre::LogManager::getSingleton().setLogDetail(mLoggingLevel);

	mRQTS = new libRQTS();

	mBackThread = new FBackLoadThread(mTextureExtension, mComMeshFolder, mTexFolder);

	mBackThread->fireRequestByName(Ogre::String("u"));
	mBackThread->fireRequestByName(Ogre::String("v"));
	mBackThread->fireRequestByName(Ogre::String("w"));
	mBackThread->fireRequestByName(Ogre::String("x"));
	mBackThread->fireRequestByName(Ogre::String("y"));
	mBackThread->fireRequestByName(Ogre::String("z"));

    mCamera->setFixedYawAxis(false);
    mCamera->setNearClipDistance(mClipDist.x);
    mCamera->setFarClipDistance(mClipDist.y);

}

void cfMMOCfore::cleanupContent()
{

    delete mRQTS;

    shmdt(mSharedMemCam);
    shmctl(mSharedIDCam, IPC_RMID, 0);

    shmdt(mSharedMemVis);
    shmctl(mSharedIDVis, IPC_RMID, 0);

    shmdt(mSharedMemFed);
    shmctl(mSharedIDFed, IPC_RMID, 0);

	delete mBackThread;

	curl_global_cleanup();
}
