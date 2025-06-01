#ifndef __BFC_CONFIGURATION_PARSER_HPP__
#define __BFC_CONFIGURATION_PARSER_HPP__

#include <optional>
#include <utility>
#include <fstream>
#include <sstream>
#include <string>
#include <regex>
#include <map>

namespace bfc
{

class args_map : public std::map<std::string, std::string>
{
public:
    args_map() = default;

    template<typename T>
    std::optional<T> as(const std::string_view& p_key) const
    {
        auto findit = find(p_key.data());
        if (findit == end())
        {
            return std::nullopt;
        }

        T rv;
        std::istringstream iss(findit->second);
        iss >> rv;
        if (iss.fail())
        {
            return std::nullopt;
        }

        return rv;
    }

    std::optional<std::string> arg(const std::string_view& p_key) const
    {
        auto findit = find(p_key.data());
        if (findit == end())
        {
            return std::nullopt;
        }
        return findit->second;
    }
};

class configuration_parser : public args_map
{
public:
    void load_line(const std::string& line)
    {
        static const std::regex config_fmt("^(.+?)[ ]*=[ ]{0,1}(.*?)$");
        std::smatch match;
        if (std::regex_match(line, match, config_fmt))
        {
            try_emplace(match[1].str(), match[2].str());
        }
    }

    void load(std::string file)
    {
        std::ifstream in(file);
        if (!in.is_open()) 
        {
            return;
        }

        std::string line;
        while (std::getline(in, line))
        {
            load_line(line);
        }
    }
};

} // namespace bfc

#endif // __BFC_CONFIGURATION_PARSER_HPP__
