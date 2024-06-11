#include <iostream>
#include "include/curl/curl.h"

struct MemoryStruct {
	char *memory = 0;
	size_t size = 0;
};

static size_t WriteDataCallback(void *contents, size_t size, size_t nmemb, void *userp) {
	size_t realsize = size * nmemb;
	MemoryStruct *mem = (struct MemoryStruct *)userp;

	char *ptr = (char*)realloc(mem->memory, mem->size + realsize + 1);
	if(!ptr) {
		printf("not enough memory (realloc returned NULL)\n");
		return 0;
	}

	mem->memory = ptr;
	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;

	return realsize;
}

class CurlWrapper {
public:
	MemoryStruct GetData() {
		return m_data;
	}
	CurlWrapper() {
		m_handle = curl_easy_init();
		if (m_handle == 0) {
			std::cout << "HTTP request failed\n";
			exit(-1);
		}
		m_data.memory = (char*)malloc(1);
	}
	~CurlWrapper() {
		curl_easy_cleanup(m_handle);
		if (m_data.memory) free(m_data.memory);
	}
	bool MakeRequest(const char* url) {
		m_data.size = 0;
		curl_easy_setopt(m_handle, CURLOPT_URL, url);
		curl_easy_setopt(m_handle, CURLOPT_WRITEFUNCTION, WriteDataCallback);
		curl_easy_setopt(m_handle, CURLOPT_WRITEDATA, (void *)&m_data);
		auto result = curl_easy_perform(m_handle);
		if (result != CURLE_OK) {
			std::cout << "Error: " << curl_easy_strerror(result) << std::endl;
			return false;
		}
		return true;
	}
private:
	CURL* m_handle;
	MemoryStruct m_data;
};