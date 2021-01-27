#ifndef __HTTP_HEADER__
#define __HTTP_HEADER__

#include <string>
#include <mutex>
#include <thread>
#include <list>
#include <memory>
#include <map>

namespace pd2hook
{

	void download_blt();
	struct HTTPItem;

	typedef void(*HTTPCallback)(std::unique_ptr<HTTPItem> httpItem);
	typedef void(*HTTPProgress)(void* data, long progress, long total);

	struct HTTPItem
	{
		HTTPCallback call = nullptr;
		HTTPProgress progress = nullptr;
		std::string url;
		std::string httpContents;
		std::map<std::string, std::string> responseHeaders;
		int errorCode;
		int httpStatusCode;
		void* data = nullptr;

		long byteprogress = 0;
		long bytetotal = 0;
	};

	class HTTPManager
	{
	private:
		HTTPManager();

	public:
		~HTTPManager();

		void init_locks();

		static HTTPManager* GetSingleton();

		void SSL_Lock(int lockno);
		void SSL_Unlock(int lockno);

		void LaunchHTTPRequest(std::unique_ptr<HTTPItem> callback);
	private:
		std::unique_ptr<std::mutex[]> openssl_locks;
		int numLocks = 0;
		std::list<std::unique_ptr<std::thread>> threadList;
	};
}

#endif // __HTTP_HEADER__