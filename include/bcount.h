/**
    `cfMMOC` library for terrain rendering using OGRE (https://github.com/cfmmoc/cfmmoc/)
    Licensed under The GNU General Public License v3.0 (GPLv3)

    Any modification, re-utilization or copy of the source or binary format in other software or publications should mention a CITATION of this library.

    Copyright (c) 2016-2018 by Authors (Jin Yan, Guanghong Gong, Ni Li and Luhao Xiao)
**/

#ifndef COUNTINGTHREAD_H
#define COUNTINGTHREAD_H

#include <Poco/Runnable.h>
#include "SdkSample.h"
#include "rqts.h"

using namespace Ogre;

/**
  @brief  A thread for counting each pixel of an image to obtain pixel coverage of each tile.
**/

class CountingThread : public Poco::Runnable
{
    public:
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
        CountingThread(int x, int y, int split, int merge, libRQTS* rqts);
        virtual ~CountingThread();
    /**
    @remarks
        main loop of the counting thread
    **/
        virtual void run();
    /**
    @remark
        copy image from external
    @par
        *texture  a point to an external image
    **/
        void copyTexture(unsigned char *texture);
    /**
    @remark
        return if or not the counting thread is idle
    **/
        bool isSleeping();
    /**
    @remark
        return counting results
    **/
        HashMap<unsigned int, unsigned int> getCountingResults();
    /**
    @remark
        return a copy of map from color to tile name in string
    **/
        HashMap<unsigned int, Ogre::String> getAnotherHashColor2Tile();
    /**
    @remark
        update the sync status of fore-end process
    @par
        over	sync status of fore-end process
    **/
        bool updateOverState(bool over);
    /**
    @remark
        sync the map from color to tile from main thread
    @par
        hct	the map from color to tile in main thread
    **/
        void fillinHashMaps(HashMap<unsigned int, Ogre::String> hct);
    /**
    @remark
        count each pixel to obtain pixel coverage of each tile
    **/
        void countingTexture();
    /**
    @remark
        find the maximum pixel coverage of tile
    **/
        Ogre::String findMaxTile4Split();
    /**
    @remark
        find tiles whose pixel coverage is less than a given threashold, 
        and to guarantee that the number of children of found tiles must be 4
    **/
        std::vector<Ogre::String> findTiles4Merge();
    protected:
    private:
        // memory to store image for pixel conuting
        unsigned char *mBackMemTex;
        // mX = image width, mY = and height
        unsigned int mX, mY;
        // indicating if this thread is idle
        bool mSleeping;
        // splitting threshold in pixel
        int mSplitThresh;
        // merging threadhold in pixel
        int mMergeThresh;
        // a map from predefined color to tile's name in string
        HashMap<unsigned int, Ogre::String> mHashColor2Tile;
        // a map from predefined color to pixel coverage in pixel
        HashMap<unsigned int, unsigned int> mHashColor2Pixel;
        // a map from a tile to summed pixel coverage of its children
        HashMap<Ogre::String, unsigned int> mHashTiles2Pixel;
        // a map from a tile to the number of its children in the scene
        HashMap<Ogre::String, unsigned int> mHashTiles2Count;
        // sync status of the fore-end process
        bool mOverFromForeend;
        // a pointer to a class encapsulated operation of restricted quadtrees
        libRQTS* mRQTS;

};

#endif // COUNTINGTHREAD_H
