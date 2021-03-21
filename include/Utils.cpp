#include <fstream>
#include "Utils.h"

Utils::Utils()
{
    //ctor
}

 void Utils::replaceAll(std::string &str, const std::string &from, const std::string &to) {
    if (from.empty())
        return;

    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}

int Utils::FileSize(std::string filePath) {
    struct stat result;

    int statResult = stat(filePath.c_str(), &result);
    if (statResult == 0) return result.st_size;
    return -1;
}

long Utils::FileModTime(std::string filePath) {
    struct stat result;
    int statResult = stat(filePath.c_str(), &result);
    if (statResult == 0) return result.st_mtime;
    return -1;
}

bool Utils::DirectoryExists(std::string filePath, bool create) {
    if (filePath == "")
        return true;

    bool result = std::filesystem::is_directory(filePath);

    if (create && !result) {
        std::filesystem::create_directory(filePath.c_str());
    }
    return result;
}

std::string Utils::TrimPathString(std::string input) {
    if (input.find("/") == std::string::npos && input.find("\\") == std::string::npos)
        return input;

    auto location = input.rfind("/");

    if (location == std::string::npos)
        location = input.rfind("\\");

    return input.substr(location+1);
}

int Utils::strCount(std::string input, char search) {
   int result = 0;

   for(auto i = 0; i < input.size(); i++) {
       if (input.at(i) == search)
           result++;
   }

   return result;
}
void Utils::padTo(std::string &str, const size_t num, const char paddingChar) {

        if(num > str.size())
            str.insert(0, num - str.size(), paddingChar);

}

std::vector<std::string> Utils::splitString(std::string input, const char splitChar) {
    std::vector<std::string> result;
    int count = strCount(input, splitChar);
    for (auto i = 0; i < count+1; i++) {
        int location = input.find(splitChar);
        result.push_back(input.substr(0, location));
        input = input.substr(location+1, input.size() - (location +1));
    }

    return result;
}
