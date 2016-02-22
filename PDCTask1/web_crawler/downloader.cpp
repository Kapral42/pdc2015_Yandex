/*
* Класс downloader - загружает страницу в string буфер по поданой извне ссылке
* и возвращает shared pointer на буфер
*/

#include "downloader.h"

// public

downloader::downloader(int const& timeout) : timeout(timeout)
{
	CURLcode code(CURLE_FAILED_INIT);
    curl = curl_easy_init();

    if (curl) {
        if (!(CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, string_write))
            && CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L))
            && CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L))
            && CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout))))
        { throw("ERROR: curl init."); }
    } else { throw("ERROR: curl init."); }
    url = std::make_shared<std::string>(*(new std::string));
}

downloader::downloader(downloader const&) = delete;

std::shared_ptr<std::string> downloader::download(std::shared_ptr<std::string> new_url)
{
	if (*url != *new_url){
    	url = new_url;
    	str_content = new std::string("");
		CURLcode code(CURLE_FAILED_INIT);
		if (!CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_URL, url.get()->c_str()))
			|| !CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, str_content))) {
			throw("ERROR: curl update url");
		}
		if(get_page()){
	    	*str_content = "";
		}
	}
    content = std::make_shared<std::string>(*str_content);
	return content;

}

downloader::~downloader()
{
	curl_easy_cleanup(curl);
}

// private

int downloader::get_page()
{
    CURLcode res = curl_easy_perform(curl);
    if (res == CURLE_OK)
       	return 0;
    return 1;
}

size_t downloader::string_write(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}
