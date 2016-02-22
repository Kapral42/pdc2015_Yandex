/*
* Класс парсер - реализует поиск контента по string буферу ссылке
* составляет список встретившихся ссылок
*/
#include "parser.h"

// public

parser::parser()
{ }

parser::parser(parser const&) = delete;

int parser::pars(std::shared_ptr<std::string> new_content)
{
	if(!new_content){
		throw("ERROR: curl read");
		return 1;
	}
	content = new_content;
	find_herfs();
	return 0;

}

parser::~parser()
{ }

void parser::content_to_file(std::string const& fname)
{
	std::ofstream out(fname);
	out << *content;
	out.close();
}

void parser::content_to_file(std::string const& fname,
							 std::shared_ptr<std::string> new_content)
{
	std::ofstream out(fname);
	out << *new_content;
	out.close();
}

void parser::print_urls()
{
	std::for_each(found_urls.begin(), found_urls.end(), [](std::shared_ptr<std::string> &s_ptr){
		std::cout << *s_ptr << std::endl;
	});
}

std::shared_ptr<std::string> parser::get_url(size_t const& i)
{
	if (i < found_urls.size())
		return (found_urls[i]);
	throw("ERROR: curl read");
}

size_t parser::get_size()
{
	return found_urls.size();
}

// private

const bool parser::check_url(std::string const& link)
{
	std::regex check("^(https?:\\/\\/)?([\\da-z\\.-]+)\\.([a-z\\.]{2,6})([\\/\\w \\-]*)*\\/?$");
	if(regex_match(link, check))
		return true;
	return false;
}

size_t parser::find_herfs()
{
	size_t iter = 0;
	while (true) {
		iter = content.get()->find(" href=\"", iter);
		if (iter == std::string::npos)
			break;
		iter += 7;
		size_t substr_size = content.get()->find('\"', iter) - iter;
		std::string *sub_s = new std::string;
		*sub_s = content.get()->substr(iter, substr_size);
		if (check_url(*sub_s)){
			found_urls.push_back(std::make_shared<std::string>(*sub_s));
		}
	}
	return found_urls.size();
}

