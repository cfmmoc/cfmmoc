/**
	This snippet is direved from http://wiki.ogre3d.org/URLArchive
	Right reserved for jacmoe, Invader Zim and other contributors
**/

#ifndef __cURLArchive_H__
#define __cURLArchive_H__

#include <OgrePrerequisites.h>

#include <OgreArchive.h>
#include <OgreArchiveFactory.h>
#include <OgreArchive.h>

/** Specialisation of the Archive class to allow reading of files from a URL.
*/
class cURLArchive : public Ogre::Archive
{
public:
	cURLArchive(const Ogre::String& name, const Ogre::String& archType );
	~cURLArchive();

	/// @copydoc Archive::isCaseSensitive
	bool isCaseSensitive(void) const { return true; }

	/// @copydoc Archive::load
	void load();
	/// @copydoc Archive::unload
	void unload();

	/// @copydoc Archive::open
	Ogre::DataStreamPtr open(const Ogre::String& filename, bool readOnly = true) const;

	time_t getModifiedTime(const Ogre::String& filename);

	/// @copydoc Archive::list
	Ogre::StringVectorPtr list(bool recursive = true, bool dirs = false);

	/// @copydoc Archive::listFileInfo
	Ogre::FileInfoListPtr listFileInfo(bool recursive = true, bool dirs = false);

	/// @copydoc Archive::find
	Ogre::StringVectorPtr find(const Ogre::String& pattern, bool recursive = true,
		bool dirs = false);

	/// @copydoc Archive::findFileInfo
	Ogre::FileInfoListPtr findFileInfo(const Ogre::String& pattern, bool recursive = true,
		bool dirs = false) const;

	/// @copydoc Archive::exists
	bool exists(const Ogre::String& filename);

private:
	void * mCurl;
	mutable void * mBuffer;
	std::string mLastURL;
	size_t mSize;
};

/** Specialisation of ArchiveFactory for cURLArchive files. */
class _OgrePrivate cURLArchiveFactory : public Ogre::ArchiveFactory
{
public:
	virtual ~cURLArchiveFactory() {}
	/// @copydoc FactoryObj::getType
	const Ogre::String& getType(void) const;
	/// @copydoc FactoryObj::createInstance
	Ogre::Archive *createInstance( const Ogre::String& name, bool readOnly =false )
	{
		return new cURLArchive(name, "cURL");
	}
	Ogre::Archive *createInstance( const Ogre::String& name )
	{
		return new cURLArchive(name, "cURL");
	}
	/// @copydoc FactoryObj::destroyInstance
	void destroyInstance( Ogre::Archive* arch) { delete arch; }
};

#endif
