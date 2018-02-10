#ifndef COUNTINGTHREAD_H
#define COUNTINGTHREAD_H

#include <Poco/Runnable.h>
#include "SdkSample.h"
#include "rqts.h"

using namespace Ogre;

class CountingThread : public Poco::Runnable
{
    public:
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
