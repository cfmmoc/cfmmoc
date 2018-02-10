#include "bload.h"
#include "rqts.h"

BBackLoadThread::BBackLoadThread(Ogre::String texExt, Ogre::String meshFD)
{
	mTimer.reset();
	mMilliseconds = 0;
	mLoadType = 3;
	mTextureExtension = texExt;
	mSimMeshFolder = meshFD;
}

BBackLoadThread::~BBackLoadThread()
{

}

void BBackLoadThread::fireRequestByName(Ogre::String reqname, bool preparation, bool loadTex)
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

		if (loadTex)
		{
			filename = replacefile(reqname) + mTextureExtension;
			TexturePtr pTex = static_cast<TexturePtr>(TextureManager::getSingleton().create(serverfilename + mTextureExtension,
                ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME));
			pTex->setBackgroundLoaded(true);
			if (preparation)
				t2 = ResourceBackgroundQueue::getSingleton().prepare("Texture", pTex->getName(), ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME, false, 0, 0, 0);
			else
				t2 = ResourceBackgroundQueue::getSingleton().load("Texture", pTex->getName(), ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME, false, 0, 0, 0);
			mTicketsTex.push_back(t2);
		}

		mTicketFilename[t1] = reqname;

}

void BBackLoadThread::operationCompleted(BackgroundProcessTicket ticket, const BackgroundProcessResult& result)
{

}

unsigned long BBackLoadThread::getMilliseconds()
{
	return mMilliseconds;
}

std::vector<Ogre::String> BBackLoadThread::getLoadedFilename(bool loadTex)
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
					mMilliseconds = mTimer.getMilliseconds();
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
					mMilliseconds = mTimer.getMilliseconds();
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
				if (loadTex)
				{
					Ogre::BackgroundProcessTicket t2 = mTicketsTex[i];
					check = (t1 != 0 && t2 != 0 &&
						ResourceBackgroundQueue::getSingleton().isProcessComplete(t1) &&
						ResourceBackgroundQueue::getSingleton().isProcessComplete(t2));
				}
				else
				{
					check = (t1 != 0 &&
						ResourceBackgroundQueue::getSingleton().isProcessComplete(t1));
				}
				if (check)
				{
					std::map<Ogre::BackgroundProcessTicket, Ogre::String>::iterator iter = mTicketFilename.find(t1);
					if (iter != mTicketFilename.end())
					{
						mMilliseconds = mTimer.getMilliseconds();
						Ogre::String filename = (*iter).second;
						ret.push_back(filename);

						MeshPtr pMesh = static_cast<MeshPtr>(MeshManager::getSingleton().getByName(
							mSimMeshFolder + replacefile(filename) + Ogre::String(".mesh"), ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME));
						pMesh->setBackgroundLoaded(false);

						if (loadTex)
						{
							TexturePtr pTex = static_cast<TexturePtr>(TextureManager::getSingleton().getByName(
								replacefile(filename) + mTextureExtension, ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME));
							pTex->setBackgroundLoaded(false);
						}

						mTicketFilename.erase(iter);
						mTicketsMesh[i] = 0;
						if (loadTex)
						{
							mTicketsTex[i] = 0;
						}
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
