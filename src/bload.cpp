/**
    `cfMMOC` library for terrain rendering using OGRE (https://github.com/cfmmoc/cfmmoc/)
    Licensed under The GNU General Public License v3.0 (GPLv3)
    Any modification, re-utilization or copy of the source or binary format in other software or publications should mention a CITATION of this library.
    Copyright (c) 2016-2018 by Authors (Jin Yan, Guanghong Gong, Ni Li and Luhao Xiao)
**/

#include "bload.h"
#include "rqts.h"

/**
  @brief  A class for fetching tiles from server for back-end process.
**/

/**
@remarks
initialize parameters
@par
meshFD		folder name for simplified mesh
**/
BBackLoadThread::BBackLoadThread(Ogre::String meshFD)
{
	mSimMeshFolder = meshFD;
}

BBackLoadThread::~BBackLoadThread()
{

}

/**
@remark
	fire a request to back-end thread
	create mesh resource, prepare/load it, and store the corresponding request ticket to mTicketsMesh and mTicketFilename
@par
	reqname		file name of a tile
**/
void BBackLoadThread::fireRequestByName(Ogre::String reqname)
{
	Ogre::BackgroundProcessTicket t1;
	Ogre::String dirname, filename, purefilename, serverfilename;
		filename = replacefile(reqname) + Ogre::String(".mesh");
       	serverfilename = mSimMeshFolder + replacefile(reqname);
	MeshPtr pMesh = static_cast<MeshPtr>(MeshManager::getSingleton().create(serverfilename + Ogre::String(".mesh"),
           	ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME));
	pMesh->setBackgroundLoaded(true);
	t1 = ResourceBackgroundQueue::getSingleton().prepare("Mesh", pMesh->getName(), ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME, false, 0, 0, 0);
	mTicketsMesh.push_back(t1);
	mTicketFilename[t1] = reqname;
}

void BBackLoadThread::operationCompleted(BackgroundProcessTicket ticket, const BackgroundProcessResult& result)
{

}

/**
@remark
	return tiles which is already fetched
	traverse all tickets, check if the corresponding request is fetched
	if fetched, remove ticket from mTicketsMesh and mTicketFilename, and return tiles 
**/
std::vector<Ogre::String> BBackLoadThread::getLoadedFilename()
{
	std::vector<Ogre::String> ret;

	if (mTicketsMesh.size() > 0)
	{
		for (int i = 0; i < mTicketsMesh.size(); i++)
		{
			Ogre::BackgroundProcessTicket t1 = mTicketsMesh[i];
			if (t1 != 0 && ResourceBackgroundQueue::getSingleton().isProcessComplete(t1))
			{
				std::map<Ogre::BackgroundProcessTicket, Ogre::String>::iterator iter = mTicketFilename.find(t1);
				if (iter != mTicketFilename.end())
				{
					Ogre::String filename = (*iter).second;
					ret.push_back(filename);
						MeshPtr pMesh = static_cast<MeshPtr>(MeshManager::getSingleton().getByName(
						mSimMeshFolder + replacefile(filename) + Ogre::String(".mesh"), ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME));
					pMesh->setBackgroundLoaded(false);
					mTicketFilename.erase(iter);
					mTicketsMesh[i] = 0;
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
bool BBackLoadThread::isQueueEmpty()
{
	return mTicketFilename.empty();
}
