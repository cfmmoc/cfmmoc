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
  @brief  A class for fetching tile from server for fore-end thread.
**/

class FBackLoadThread : public Ogre::ResourceBackgroundQueue::Listener
{
public:
	/**
    	@remarks
        	initialize parameters
    	@par
		texExt		extension for texture files
		meshFD		folder name for meshes
		texFD		folder name for textures
    	**/
	FBackLoadThread(Ogre::String texExt, Ogre::String meshFD, Ogre::String texFD);
	~FBackLoadThread();

	/**
	@remark
		fire a request to back-end thread
	@par
		reqname		file name of a tile
	**/
	void fireRequestByName(Ogre::String reqname);
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
	// a map from request ticket to tile's name
	std::map<Ogre::BackgroundProcessTicket, Ogre::String> mTicketFilename;
	// a vector of all tickets of meshes
	std::vector<Ogre::BackgroundProcessTicket> mTicketsMesh;
	// a vector of all tickets of texctures
	std::vector<Ogre::BackgroundProcessTicket> mTicketsTex;
	// extension for texture files
	Ogre::String mTextureExtension;
	// folder name for meshes
	Ogre::String mComMeshFolder;
	// folder name for textures
	Ogre::String mTexFolder;
};
