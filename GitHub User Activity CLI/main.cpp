#define WIN32_LEAN_AND_MEAN
#include <windows.h>   // must be included before using GetTempPathA
#include <iostream>
#include <string>
#include <stdio.h>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <array>

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

// use jq to parse the JSON output from the GitHub API
std::string parseGitHubEventsWithJQ(const std::string& json) {
    char tempPath[MAX_PATH];
    DWORD pathLen = GetTempPathA(MAX_PATH, tempPath);
    if (pathLen == 0 || pathLen > MAX_PATH) {
        throw std::runtime_error("Failed to get temp path");
    }

    std::string tempFile = std::string(tempPath) + "github_events.json";
    
    FILE* file = fopen(tempFile.c_str(), "w");
    if (!file) throw std::runtime_error("Failed to create temp file");

    fputs(json.c_str(), file);
    fclose(file);

    // use jq to format the output
    std::string command = "jq -r \".[] | .type + \\\" on \\\" + (.repo.name // \\\"unknown repo\\\")\" \"" + tempFile + "\"";
    std::array<char, 128> buffer;
    std::string result;

    std::shared_ptr<FILE> pipe(popen(command.c_str(), "r"), pclose);
    if (!pipe) throw std::runtime_error("jq popen() failed!");

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += "- ";
        result += buffer.data();
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

        std::string parsed = parseGitHubEventsWithJQ(json);
        std::cout << "Recent GitHub Activity:" << parsed << std::endl;
    } catch (const std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}