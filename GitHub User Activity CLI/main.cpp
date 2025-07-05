#define WIN32_LEAN_AND_MEAN
#include <windows.h>   // must be included before using GetTempPathA
#include <iostream>
#include <string>
#include <stdio.h>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <array>
#include "json.hpp"
using json = nlohmann::json;

std::string fetchGitHubActivity(const std::string& url) {
    std::string command = "curl -s -H \"User-Agent: Cpp-App\" \"" + url + "\"";
    std::array<char, 128> buffer;
    std::string result;
    
    std::shared_ptr<FILE> pipe(popen(command.c_str(), "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    return result;
}

// native parser
std::string parseGitHubEvents(const std::string& jsonString) {
    std::string result;

    try {
        json j = json::parse(jsonString);
        for (const auto& event : j) {
            std::string type = event.value("type", "UnknownEvent");
            std::string repo = event.contains("repo") ? event["repo"].value("name", "unknown repo") : "unknown repo";
            
            if (type == "PushEvent") {
                result += "- Pushed commits to " + repo + "\n";
            } else if (type == "CreateEvent") {
                result += "- Created something in " + repo + "\n";   
            }
        }
    } catch (const std::exception& e) {
        result = "Error parsing JSON: ";
        result += e.what();
    }

    return result;
}


int main(int argc, char* argv[]) {
    // check if the number of arguments is correct
    if (argc != 2) {
        std::cerr << "Usage: github-activity <username>" << std::endl;
        return 1;
    }

    // get the username from the command line
    std::string username = argv[1];
    std::string url = "https://api.github.com/users/" + username + "/events";

    // std::string json = fetchGitHubActivity(url);
    // std::cout << "\nRaw JSON fetched:\n" << json.substr(0, 500) << "..." << std::endl;
    
    std::cout << "Fetching events for [" << username << "]" << " from [" << url << "]" << std::endl;
    try {
        std::string json = fetchGitHubActivity(url);

        if (json.find("Not Found") != std::string::npos) {
            std::cerr << "Error: User not found or GitHub API returned error" << std::endl;
            return 1;
        }

        std::string parsed = parseGitHubEvents(json);
        std::cout << "Recent GitHub Activity:" << parsed << std::endl;
    } catch (const std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}