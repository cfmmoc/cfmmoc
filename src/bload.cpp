/**
    `cfMMOC` library for terrain rendering using OGRE (https://github.com/cfmmoc/cfmmoc/)
    Licensed under The GNU General Public License v3.0 (GPLv3)
    Any modification, re-utilization or copy of the source or binary format in other software or publications should mention a CITATION of this library.
    Copyright (c) 2016-2018 by Authors (Jin Yan, Guanghong Gong, Ni Li and Luhao Xiao)
**/

#include "bload.h"
#include "rqts.h"

/**
  @brief  A class for fetch tile from server in back-end thread.
**/

BBackLoadThread::BBackLoadThread(Ogre::String meshFD)
{
	mLoadType = 3;
	mSimMeshFolder = meshFD;
}

BBackLoadThread::~BBackLoadThread()
{

}

void BBackLoadThread::fireRequestByName(Ogre::String reqname, bool preparation)
{
		Ogre::BackgroundProcessTicket t0, t1, t2;
		Ogre::String dirname, filename, purefilename, serverfilename;

		filename = replacefile(reqname) + Ogre::String(".mesh");
        	serverfilename = mSimMeshFolder + replacefile(reqname);
		MeshPtr pMesh = static_cast<MeshPtr>(MeshManager::getSingleton().create(serverfilename + Ogre::String(".mesh"),
            		ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME));
		pMesh->setBackgroundLoaded(true);
		if (preparation)
			t1 = ResourceBackgroundQueue::getSingleton().prepare("Mesh", pMesh->getName(), ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME, false, 0, 0, 0);
		else
			t1 = ResourceBackgroundQueue::getSingleton().load("Mesh", pMesh->getName(), ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME, false, 0, 0, 0);
		mTicketsMesh.push_back(t1);
		mTicketFilename[t1] = reqname;

}

void BBackLoadThread::operationCompleted(BackgroundProcessTicket ticket, const BackgroundProcessResult& result)
{

}

std::vector<Ogre::String> BBackLoadThread::getLoadedFilename()
{
	std::vector<Ogre::String> ret;

	switch (mLoadType)
	{
	case 1:
		if (mTicketsTex.size() > 0)
		{
			for (int i = 0; i < mTicketsTex.size(); i++)
			{
				Ogre::BackgroundProcessTicket ticket = mTicketsTex[i];
				if (ticket != 0 && ResourceBackgroundQueue::getSingleton().isProcessComplete(ticket))
				{
					mTicketsTex[i] = 0;
				}
			}
		}
		break;
	case 2:
		if (mTicketsMesh.size() > 0)
		{
			for (int i = 0; i < mTicketsMesh.size(); i++)
			{
				Ogre::BackgroundProcessTicket ticket = mTicketsMesh[i];
				if (ticket != 0 && ResourceBackgroundQueue::getSingleton().isProcessComplete(ticket))
				{
					mTicketsMesh[i] = 0;
				}
			}
		}
		break;
	case 3:
		if (mTicketsMesh.size() > 0)
		{
			for (int i = 0; i < mTicketsMesh.size(); i++)
			{
				Ogre::BackgroundProcessTicket t1 = mTicketsMesh[i];
				bool check;
				{
					check = (t1 != 0 &&
						ResourceBackgroundQueue::getSingleton().isProcessComplete(t1));
				}
				if (check)
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
		break;
	}

	return ret;
}

bool BBackLoadThread::isQueueEmpty()
{
	return mTicketFilename.empty();
}
