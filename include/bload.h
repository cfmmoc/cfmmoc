/**
    `cfMMOC` library for terrain rendering using OGRE (https://github.com/cfmmoc/cfmmoc/)
    Licensed under The GNU General Public License v3.0 (GPLv3)
    Any modification, re-utilization or copy of the source or binary format in other software or publications should mention a CITATION of this library.
    Copyright (c) 2016-2018 by Authors (Jin Yan, Guanghong Gong, Ni Li and Luhao Xiao)
**/

#pragma once

#include "Ogre.h"
using namespace Ogre;

/**
  @brief  A class for fetch tile from server in back-end thread.
**/

class BBackLoadThread : public Ogre::ResourceBackgroundQueue::Listener
{
public:
	/**
    	@remarks
        	initialize parameters
    	@par
		meshFD		folder name for simplified mesh 
    	**/
	BBackLoadThread(Ogre::String meshFD);
	~BBackLoadThread();

	/**
	@remark
		fire a request to back-end thread
	@par
		reqname		file name of a tile
		preparation	indicate if request is prepared in back-edn thread
	**/
	void fireRequestByName(Ogre::String reqname, bool preparation);
	void operationCompleted(BackgroundProcessTicket ticket, const BackgroundProcessResult& result);
	/**
	@remark
		return tiles which is already fetched
	**/
	std::vector<Ogre::String> getLoadedFilename();
	/**
	@remark
		return true if fetch queue is empty
	**/
	bool isQueueEmpty();

protected:
	unsigned int mLoadType;
	Ogre::StringVector mLoadList;
	std::map<Ogre::BackgroundProcessTicket, Ogre::String> mTicketFilename;
	std::vector<Ogre::BackgroundProcessTicket> mTicketsBackMesh;
	std::vector<Ogre::BackgroundProcessTicket> mTicketsMesh;
	std::vector<Ogre::BackgroundProcessTicket> mTicketsTex;
	Ogre::String mTextureExtension;
	Ogre::String mSimMeshFolder;
};
