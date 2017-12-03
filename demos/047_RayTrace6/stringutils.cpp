#include "stringutils.hpp"

std::string CStringUtils::TrimRight(const std::string& Source, const std::string& T)
{
  	std::string Str = Source;
  	return Str.erase(Str.find_last_not_of(T) + 1);
}

std::string CStringUtils::TrimLeft(const std::string& Source, const std::string& T)
{
  	std::string Str = Source;
  	return Str.erase(0 , Source.find_first_not_of(T));
}

std::string CStringUtils::Trim(const std::string& Source, const std::string& T)
{
  	std::string Str = Source;
  	return TrimLeft(TrimRight(Str , T), T);
}

