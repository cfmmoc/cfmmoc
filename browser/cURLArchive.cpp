/**
	This snippet is direved from http://wiki.ogre3d.org/URLArchive
	Right reserved for jacmoe, Invader Zim and other contributors
**/

#include <OgreStableHeaders.h>
#include "cURLArchive.h"
#include <OgreLogManager.h>
#include <OgreException.h>
#include <OgreConfigFile.h>
#include <OgreStringVector.h>
#include <OgreRoot.h>
#include <OgreDataStream.h>

#include <curl/curl.h>

using namespace Ogre;

//-----------------------------------------------------------------------
cURLArchive::cURLArchive(const String& name, const String& archType )
	: Archive(name, archType), mCurl(0), mBuffer(0)
{
}
//-----------------------------------------------------------------------
cURLArchive::~cURLArchive()
{
	unload();
}
//-----------------------------------------------------------------------
struct MemoryStruct {
	unsigned  char * memory;
	size_t size;
};

static CURLcode
	fetch_url(CURL * curl, void *& buffer, size_t& size)
{
	MemoryStruct data = { 0 };
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
	CURLcode res = curl_easy_perform(curl);
	buffer = data.memory;
	size = data.size;
	return res;
}

static size_t
	write_memory_callback(void *ptr, size_t size, size_t nmemb, void *data)
{
	size_t realsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *)data;

	assert(mem->memory!=NULL || mem->size==0);
	mem->memory = (unsigned char *)realloc(mem->memory, mem->size + realsize + 1);
	if (mem->memory) {
		memcpy(&(mem->memory[mem->size]), ptr, realsize);
		mem->size += realsize;
		mem->memory[mem->size] = 0;
	}
	return realsize;
}

void
	cURLArchive::load()
{
	mCurl = curl_easy_init();
	curl_easy_setopt(mCurl, CURLOPT_WRITEFUNCTION, write_memory_callback);
}
//-----------------------------------------------------------------------
void
	cURLArchive::unload()
{
	if (mCurl) {
		curl_easy_cleanup(mCurl);
		mCurl = 0;
	}
	if (mBuffer) {
		free(mBuffer);
		mBuffer = 0;
	}
}
//-----------------------------------------------------------------------
DataStreamPtr
	cURLArchive::open(const String& filename, bool readOnly) const
{
	void * buffer;
	size_t size;
	std::string url = mName + filename;

	if (mBuffer==NULL || url!=mLastURL) {
		curl_easy_setopt(mCurl, CURLOPT_URL, url.c_str());

		CURLcode res = fetch_url(mCurl, buffer, size);

		long statLong = 0;
		if (CURLE_OK == res) {
			curl_easy_getinfo(mCurl, CURLINFO_HTTP_CODE, &statLong);
		}
		if (statLong!=200 || buffer==NULL) {
			OGRE_EXCEPT(Exception::ERR_FILE_NOT_FOUND,
				"Could not open resource: " + filename, "RDBArchive::open");
		}
	} else {
		// recycle the data we got in the call to exist()
		buffer = mBuffer;
		size = mSize;
		mBuffer = NULL;
	}

	MemoryDataStream stream(buffer, size, false);
	DataStreamPtr ptr(new MemoryDataStream(stream, true));
	free(buffer);
	return ptr;
}

time_t cURLArchive::getModifiedTime(const Ogre::String& filename)
{
	return time_t(0);
}

//-----------------------------------------------------------------------
StringVectorPtr cURLArchive::list(bool recursive, bool dirs)
{
	// directory change requires locking due to saved returns
	return StringVectorPtr(new StringVector());
}
//-----------------------------------------------------------------------
FileInfoListPtr cURLArchive::listFileInfo(bool recursive, bool dirs)
{
	return FileInfoListPtr(new FileInfoList());
}
//-----------------------------------------------------------------------
StringVectorPtr cURLArchive::find(const String& pattern,
								 bool recursive, bool dirs)
{
	return StringVectorPtr(new StringVector());
}
//-----------------------------------------------------------------------
FileInfoListPtr cURLArchive::findFileInfo(const Ogre::String& pattern,
										 bool recursive, bool dirs) const
{
	return FileInfoListPtr(new FileInfoList());
}
//-----------------------------------------------------------------------
bool
	cURLArchive::exists(const String& filename)
{
	std::string url = mName + filename;

	if (url==mLastURL) {
		return true;
	}

	mLastURL = url;
	if (mBuffer) {
		free(mBuffer);
		mBuffer = 0;
	}

	curl_easy_setopt(mCurl, CURLOPT_URL, url.c_str());

	CURLcode res = fetch_url(mCurl, mBuffer, mSize);

	long statLong = 0;
	if (CURLE_OK == res) {
		curl_easy_getinfo(mCurl, CURLINFO_HTTP_CODE, &statLong);
	}
	if (statLong!=200) {
		if (mBuffer) free(mBuffer);
		mBuffer = NULL;
	}

	return (statLong==200);
}
//-----------------------------------------------------------------------
const String& cURLArchiveFactory::getType(void) const
{
	static String name = "cURL";
	return name;
}
