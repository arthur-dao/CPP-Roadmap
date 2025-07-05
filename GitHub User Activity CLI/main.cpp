#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: github-activity <username>" << std::endl;
        return 1;
    }

    std::string username = argv[1];
    std::string url = "https://api.github.com/users/" + username + "/events";

    std::cout << "Fetching events for [" << username << "]" << " from [" << url << "]" << std::endl;

    return 0;
}