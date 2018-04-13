/**
    `cfMMOC` library for terrain rendering using OGRE (https://github.com/cfmmoc/cfmmoc/)
    Licensed under The GNU General Public License v3.0 (GPLv3)
    Any modification, re-utilization or copy of the source or binary format in other software or publications should mention a CITATION of this library.
    Copyright (c) 2016-2018 by Authors (Jin Yan, Guanghong Gong, Ni Li and Luhao Xiao)
**/

#include "fload.h"

/**
  @brief  A class for fetching tiles from server for fore-end process.
**/

FBackLoadThread::FBackLoadThread(Ogre::String texExt, Ogre::String meshFD, Ogre::String texFD)
{
	mTextureExtension = texExt;
	mComMeshFolder = meshFD;
	mTexFolder = texFD;
}

FBackLoadThread::~FBackLoadThread()
{

}

/**
@remark
	fire a request to back-end thread
	create mesh and texture resource, 
	prepare/load them, and store the corresponding request tickets to 
	mTicketsMesh, mTicketsTex and mTicketFilename
@par
	reqname		file name of a tile
**/
void FBackLoadThread::fireRequestByName(Ogre::String reqname)
{
	Ogre::BackgroundProcessTicket t1, t2;
	Ogre::String dirname, filename, purefilename, serverfilename;
	filename = reqname + Ogre::String(".mesh");
        serverfilename = reqname;
	MeshPtr pMesh = static_cast<MeshPtr>(MeshManager::getSingleton().create(mComMeshFolder + serverfilename + Ogre::String(".mesh"),
            ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME));
	pMesh->setBackgroundLoaded(true);
	t1 = ResourceBackgroundQueue::getSingleton().prepare("Mesh", pMesh->getName(), ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME, false, 0, 0, 0);
	mTicketsMesh.push_back(t1);

	filename = reqname + mTextureExtension;
	TexturePtr pTex = static_cast<TexturePtr>(TextureManager::getSingleton().create(mTexFolder + serverfilename + mTextureExtension,
        ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME));
	pTex->setBackgroundLoaded(true);
	t2 = ResourceBackgroundQueue::getSingleton().prepare("Texture", pTex->getName(), ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME, false, 0, 0, 0);
	mTicketsTex.push_back(t2);

	mTicketFilename[t1] = serverfilename;
}

void FBackLoadThread::operationCompleted(BackgroundProcessTicket ticket, const BackgroundProcessResult& result)
{

}

/**
@remark
	return tiles which is already fetched
	traverse all tickets, check if the corresponding request is fetched
	if fetched, remove ticket from mTicketsMesh, mTicketsTex and mTicketFilename, and return tiles 
**/
std::vector<Ogre::String> FBackLoadThread::getLoadedFilename()
{
	std::vector<Ogre::String> ret;

	if (mTicketsMesh.size() > 0)
	{
		for (int i = 0; i < mTicketsMesh.size(); i++)
		{
			Ogre::BackgroundProcessTicket t1 = mTicketsMesh[i];
			Ogre::BackgroundProcessTicket t2 = mTicketsTex[i];
			if (t1 != 0 && t2 != 0 &&
				ResourceBackgroundQueue::getSingleton().isProcessComplete(t1) &&
				ResourceBackgroundQueue::getSingleton().isProcessComplete(t2))
			{
				std::map<Ogre::BackgroundProcessTicket, Ogre::String>::iterator iter = mTicketFilename.find(t1);
				if (iter != mTicketFilename.end())
				{
					Ogre::String filename = (*iter).second;
					ret.push_back(filename);
					MeshPtr pMesh = static_cast<MeshPtr>(MeshManager::getSingleton().getByName(
						mComMeshFolder + filename + Ogre::String(".mesh"), ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME));
					pMesh->setBackgroundLoaded(false);
					TexturePtr pTex = static_cast<TexturePtr>(TextureManager::getSingleton().getByName(
						mTexFolder + filename + mTextureExtension, ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME));
					pTex->setBackgroundLoaded(false);
					mTicketFilename.erase(iter);
					mTicketsMesh[i] = 0;
					mTicketsTex[i] = 0;
				}
			}
		}
	}

	return ret;
}

/**
@remark
	return true if fetch queue is empty
**/
bool FBackLoadThread::isQueueEmpty()
{
	return mTicketFilename.empty();
}
