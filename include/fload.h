#pragma once

#include "Ogre.h"
using namespace Ogre;

class FBackLoadThread : public Ogre::ResourceBackgroundQueue::Listener
{
public:
	FBackLoadThread(Ogre::String texExt, Ogre::String meshFD, Ogre::String texFD);
	~FBackLoadThread();

	void fireRequestByName(Ogre::String reqname, bool preparation, bool loadTex);
	void operationCompleted(BackgroundProcessTicket ticket, const BackgroundProcessResult& result);
	unsigned long getMilliseconds();
	std::vector<Ogre::String> getLoadedFilename(bool loadTex);
	bool isQueueEmpty();

protected:
	Ogre::Timer mTimer;
	unsigned long mMilliseconds;
	unsigned int mLoadType;
	Ogre::StringVector mLoadList;
	std::map<Ogre::BackgroundProcessTicket, Ogre::String> mTicketFilename;
	std::vector<Ogre::BackgroundProcessTicket> mTicketsBackMesh;
	std::vector<Ogre::BackgroundProcessTicket> mTicketsMesh;
	std::vector<Ogre::BackgroundProcessTicket> mTicketsTex;
	Ogre::String mTextureExtension;
	Ogre::String mComMeshFolder;
	Ogre::String mTexFolder;
};
