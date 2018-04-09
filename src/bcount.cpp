/**
    `cfMMOC` library for terrain rendering using OGRE (https://github.com/cfmmoc/cfmmoc/)
    Licensed under The GNU General Public License v3.0 (GPLv3)

    Any modification, re-utilization or copy of the source or binary format in other software or publications should mention a CITATION of this library.

    Copyright (c) 2016-2018 by Authors (Jin Yan, Guanghong Gong, Ni Li and Luhao Xiao)
**/

/**
  @brief  A thread for counting each pixel of an image to obtain pixel coverage of each tile.
**/

#include "bcount.h"

/**
@remarks
    initialize parameters, allocate memory
@par
    x       image width
    y       image height
    split    threshold for splitting a tile
    merge    threshold for merging four tiles
    *rqts    a class encapsulated operation of restricted quadtrees
**/
CountingThread::CountingThread(int x, int y, int split, int merge, libRQTS* rqts) : Runnable()
{
    mX = x;
    mY = y;
    mBackMemTex = new unsigned char[(unsigned int)(mX * mY * 4)];
    mSplitThresh = split;
    mMergeThresh = merge;
    mSleeping = true;
    mOverFromForeend = false;
    mRQTS = rqts;
}

CountingThread::~CountingThread()
{
    delete [] mBackMemTex;
}

/**
@remark
    copy image from external
@par
    *texture  a point to an external image
**/
void CountingThread::copyTexture(unsigned char *texture)
{
    {
        memcpy(mBackMemTex, texture, (unsigned int)(mX * mY * 4));
        mSleeping = false;
    }
}

/**
@remark
    return if or not the counting thread is idle
    if mSleeping == true or mOverFromForeend == false 
    (mOverFromForeend indicates sync status of fore-end rendering process), 
    then return true
**/
bool CountingThread::isSleeping()
{
    return mSleeping || !mOverFromForeend;
}

/**
@remark
    return counting results
    mHashColor2Pixel represents a map from color for a given tile to pixel coverage of that tile
**/
HashMap<unsigned int, unsigned int> CountingThread::getCountingResults()
{
    return mHashColor2Pixel;
}

/**
@remark
    return a copy of map from color to tile name in string
    this map is maintained in this thread, the copy of the map is used externally
**/
HashMap<unsigned int, Ogre::String> CountingThread::getAnotherHashColor2Tile()
{
    return mHashColor2Tile;
}

/**
@remark
    update the sync status of fore-end process
    if mOverFromForeend == false, then update the status, and return true
@par
    over	the sync status of fore-end process
**/
bool CountingThread::updateOverState(bool over)
{
    bool updated = false;
    if (!mOverFromForeend)
    {
        mOverFromForeend = over;
        updated = true;
    }
    return updated;
}

/**
@remark
    sync the map from color to tile from main thread
@par
    hct	the map from color to tile in main thread
**/
void CountingThread::fillinHashMaps(HashMap<unsigned int, Ogre::String> hct)
{
    {
        mHashColor2Tile = hct;
    }
}

/**
@remark
    count each pixel to obtain pixel coverage of each tile
**/
void CountingThread::countingTexture()
{
    unsigned char r,g,b;

	unsigned int temp[4096];
	memset(temp, 0, sizeof(unsigned int) * 4096);
	mHashColor2Pixel.clear();
	mHashTiles2Pixel.clear();
	mHashTiles2Count.clear();

	/**
	    traverse each pixel, obtain its r,g,b value, 
	    get its corresponding id via a predefined map function from r,g,b to an integer, 
	    then accumulate the pixel count for given color
	**/
	for (unsigned int y = 0; y < mY; y++)
	{
		for (unsigned int x = 0; x < mX; x++)
		{
			r = mBackMemTex[(y * (unsigned int)(mX) + x) * 4 + 2];
			g = mBackMemTex[(y * (unsigned int)(mX) + x) * 4 + 1];
			b = mBackMemTex[(y * (unsigned int)(mX) + x) * 4];

			unsigned int id = ((255 - r) / 16) + (((255 - g) / 16) << 4) + (((255 - b) / 16) << 8);
			temp[id] ++;

		}
	}
	/**
	    traverse each predefined color, 
	    set the result of pixel counting to mHashColor2Pixel, 
	    mHashColor2Pixel is a map from predefined color to the result of pixel counting, 
	    get the parent of the tile corresponding to given predefined color, 
	    accumlate the number of pixels to mHashTiles2Pixel, 
	    mHashTiles2Pixel is a map from tile (actually, the parent tile) to the result of pixel counting, 
	    plus one to mHashTiles2Count, 
	    mHashTiles2Count is a map form tile (actually, the parent tile) to the number of children (in the scene) of that tile
	**/
	for (unsigned int num = 0; num < 4095; num++)
	{
		{
			mHashColor2Pixel[num] = temp[num];
			HashMap<unsigned int, Ogre::String>::iterator it = mHashColor2Tile.find(num);
			if (it != mHashColor2Tile.end())
			{
				Ogre::String tile = it->second;
				tile.erase(tile.end() - 1);
				if (tile != "")
				{
                    			tile.erase(tile.end() - 1);
                    			if (tile != "")
                    			{
                        			mHashTiles2Pixel[tile] += temp[num];
                        			mHashTiles2Count[tile]++;
                    			}
				}
			}
		}
	}
}

/**
@remark
    find the maximum pixel coverage of tile
**/
Ogre::String CountingThread::findMaxTile4Split()
{
    	bool found = false;
	Ogre::String found_tile = "";
	unsigned int found_count = 0;
	/**
		traverse each predefined color, 
		obtain the pixel coverage for given color, 
		compare coverages and find out the maximum of coverage among all tiles
	**/
	for (HashMap<unsigned int, unsigned int>::iterator it = mHashColor2Pixel.begin();
		it != mHashColor2Pixel.end(); it ++)
	{
		unsigned int color = it->first;
		unsigned int count = it->second;
		if (color != 0 && count > 0)
		{
			HashMap<unsigned int, Ogre::String>::iterator iter = mHashColor2Tile.find(color);
			/**
				string length of tile's name is less than 10
				means the maximum level of tile is 5
			**/
			if (iter != mHashColor2Tile.end() && iter->second.length() < 10)
			{
                		Ogre::String str = iter->second;
				{
					found = true;
					if (count > found_count)
					{
						found_count = count;
						found_tile = iter->second;
					}
				}
			}
		}
	}
	if (found_count < mSplitThresh)
        	found_tile = "";
	return found_tile;
}

/**
@remark
    find tiles whose pixel coverage is less than a given threashold, 
    and to guarantee that the number of children of found tiles must be 4
**/
std::vector<Ogre::String> CountingThread::findTiles4Merge()
{
    	std::vector<Ogre::String> found_tiles;
	for (HashMap<Ogre::String, unsigned int>::iterator it = mHashTiles2Pixel.begin();
		it != mHashTiles2Pixel.end(); it ++)
	{
		Ogre::String tile = it->first;
		unsigned int count = it->second;
		if (tile != "")
		{
			if (count < mMergeThresh && mHashTiles2Count[tile] == 4)
			{
				found_tiles.push_back(tile);
			}
		}
	}
	return found_tiles;
}

/**
@remarks
    main loop of the counting thread
**/
void CountingThread::run()
{
    while(1)
    {
	/**
	    to sleep for a period of time
	**/
        if (mSleeping || !mOverFromForeend)
        {
            Poco::Thread::sleep(50);
        }
	/**
	    count pixel coverage of tiles, 
	    then find out the tile with maximum coverage, and all tiles whose satisfy merging threshold
	    if there is/are tiles to merge and mRQTS is idle, 
	    then check the mergiblity of all tiles whose satisfy merging threshold
	    if there is a tile to split and mRQTS is idle, 
	    then check the splittability of the tile with maximum coverage
	**/
        else
        {
            countingTexture();
            Ogre::String tile1 = findMaxTile4Split();
            std::vector<Ogre::String> tiles2 = findTiles4Merge();

            if (!tiles2.empty() && mRQTS->isdone())
            {
                mRQTS->checkmerge(tiles2);
            }
            if (tile1 != "" && mRQTS->isdone())
            {
		mRQTS->checksplit(tile1);
            }
            mOverFromForeend = false;
            mSleeping = true;
        }
    }
}
