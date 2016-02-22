/*
* class downloader: pass a reference(std::shared_ptr<std::string>)
* to obtain a buffer(std::shared_ptr<std::string>)
*/
#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <iostream>
#include <string>
#include <curl/curl.h>
#include <memory>

class downloader {
public:
	downloader(int const& timeout);
	downloader(downloader const&);
	std::shared_ptr<std::string> download(std::shared_ptr<std::string> new_url);
	~downloader();
private:
 	std::shared_ptr<std::string> url;
	std::shared_ptr<std::string> content;
	std::string* str_content;
	CURL* curl;
	long int timeout;

	int get_page();
	static size_t string_write(void *contents, size_t size, size_t nmemb, void *userp); 
};

#endif