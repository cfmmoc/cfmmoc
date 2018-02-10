#include "bcount.h"

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

void CountingThread::copyTexture(unsigned char *texture)
{
    {
        memcpy(mBackMemTex, texture, (unsigned int)(mX * mY * 4));
        mSleeping = false;
    }
}

bool CountingThread::isSleeping()
{
    return mSleeping || !mOverFromForeend;
}

HashMap<unsigned int, unsigned int> CountingThread::getCountingResults(unsigned int *ret_count,
    unsigned int *ret_pixel, unsigned int *b_count)
{
    *ret_count = mHashColor2Tile.size();
    *ret_pixel = mMaxPixel;
    *b_count = mBlackCount;
    return mHashColor2Pixel;
}

HashMap<unsigned int, Ogre::String> CountingThread::getAnotherHashColor2Tile()
{
    return mHashColor2Tile;
}

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

void CountingThread::fillinHashMaps(HashMap<unsigned int, Ogre::String> hct)//,
{
    {
        mHashColor2Tile = hct;
    }
}

void CountingThread::countingTexture()
{
    unsigned char r,g,b;

	unsigned int temp[4096];
	memset(temp, 0, sizeof(unsigned int) * 4096);
	mHashColor2Pixel.clear();
	mHashTiles2Pixel.clear();
	mHashTiles2Count.clear();

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
	mBlackCount = temp[4095];
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

Ogre::String CountingThread::findMaxTile4Split()
{
    bool found = false;
	Ogre::String found_tile = "";
	unsigned int found_count = 0;
	for (HashMap<unsigned int, unsigned int>::iterator it = mHashColor2Pixel.begin();
		it != mHashColor2Pixel.end(); it ++)
	{
		unsigned int color = it->first;
		unsigned int count = it->second;
		if (color != 0 && count > 0)
		{
			HashMap<unsigned int, Ogre::String>::iterator iter = mHashColor2Tile.find(color);
			if (iter != mHashColor2Tile.end() && iter->second.length() < 10)
			{
                Ogre::String str = iter->second;
				{
					found = true;
					if (count > found_count)
					{
						found_count = count;
						mMaxPixel = count;
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

void CountingThread::run()
{
    while(1)
    {
        if (mSleeping || !mOverFromForeend)
        {
            Poco::Thread::sleep(50);
        }
        else
        {
            mTimer.reset();
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
