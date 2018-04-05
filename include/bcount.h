/**
  cfMMOC library for terrain rendering using OGRE
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
        initialize parameters
    @par
        x       image width
        y       image height
        split    threshold for splitting a tile
        merge    threshold for merging four tiles
        *rqts    a class encapsulated operation of restricted quadtrees
    **/
        CountingThread(int x, int y, int split, int merge, libRQTS* rqts);
        virtual ~CountingThread();
        virtual void run();
        void copyTexture(unsigned char *texture);
        bool isSleeping();
        HashMap<unsigned int, unsigned int> getCountingResults(unsigned int *ret_count,
            unsigned int *ret_pixel, unsigned int *b_count);
        HashMap<unsigned int, Ogre::String> getAnotherHashColor2Tile();
        void fillinHashMaps(HashMap<unsigned int, Ogre::String> hct);//,
        bool updateOverState(bool over);
        void countingTexture();
        Ogre::String findMaxTile4Split();
        std::vector<Ogre::String> findTiles4Merge();
    protected:
    private:
        Ogre::Timer mTimer;
        unsigned char *mBackMemTex;
        unsigned int mX, mY;
        bool mSleeping;
        int mSplitThresh;
        int mMergeThresh;
        int mMaxPixel;
        HashMap<unsigned int, Ogre::String> mHashColor2Tile;
        HashMap<unsigned int, unsigned int> mHashColor2Pixel;
        HashMap<Ogre::String, unsigned int> mHashTiles2Pixel;
        HashMap<Ogre::String, unsigned int> mHashTiles2Count;
        bool mOverFromForeend;

        unsigned int mBlackCount;
        libRQTS* mRQTS;

};

#endif // COUNTINGTHREAD_H
