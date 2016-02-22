/*
* class parser: finds links in buffer(std::shared_ptr<std::string>),
* create a list of links(std::vector<std::shared_ptr<std::string>>)
*/
#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <memory>

class parser {
public:
	parser();
	parser(parser const&);
	int pars(std::shared_ptr<std::string> new_content);
	void content_to_file(std::string const& fname); 
	void content_to_file(std::string const& fname, 
						 std::shared_ptr<std::string> new_content); 
	void print_urls(); 
	std::shared_ptr<std::string> get_url(size_t const& i);
	size_t get_size();
	~parser();
private:
	std::shared_ptr<std::string> content;
	std::vector<std::shared_ptr<std::string>> found_urls;

	const bool check_url(std::string const& link);
	size_t find_herfs();
};

#endif