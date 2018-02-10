#pragma once

#include "Ogre.h"
using namespace Ogre;

class BBackLoadThread : public Ogre::ResourceBackgroundQueue::Listener
{
public:
	BBackLoadThread(Ogre::String texExt, Ogre::String meshFD);
	~BBackLoadThread();

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
	Ogre::String mSimMeshFolder;
};
