#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <cstring>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>
#include <curl/curl.h>

#define RESET   "\033[0m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define RED     "\033[31m"
#define CYAN    "\033[36m"
#define BOLD    "\033[1m"

// shared state across threads
std::vector<std::string> wordlist;
std::atomic<size_t> next_index{0}; 
std::atomic<int> checked{0};
std::atomic<int> found{0};
std::mutex print_mutex;             
std::string target;

long ck(CURL* curl, const std::string& url) {
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
    CURLcode res = curl_easy_perform(curl);
    long code = -1;
    if (res == CURLE_OK)
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
    return code;
}

std::string color_for_code(long code) {
    if (code >= 200 && code < 300) return GREEN;
    if (code >= 300 && code < 400) return CYAN;
    if (code >= 400 && code < 500) return YELLOW;
    return RED;
}


void worker() {
    CURL* curl = curl_easy_init();
    if (!curl) return;

    while (true) {
        size_t i = next_index.fetch_add(1); 
        if (i >= wordlist.size()) break;  
        const std::string& word = wordlist[i];
        long code = ck(curl, target + "/" + word);
        checked++;

        if (code == -1 || code == 404) continue;

        found++;
        std::lock_guard<std::mutex> lock(print_mutex); 
        std::cout << "│ " << color_for_code(code)
                  << std::setw(5) << code << RESET
                  << " │ "
                  << std::left << std::setw(42) << word
                  << " │\n";
    }

    curl_easy_cleanup(curl);
}

void print_banner() {
    std::cout << BOLD << CYAN;
    std::cout << "     _ _                            _            \n";
    std::cout << "  __| (_)_ ____      ____ _ _ __ __| | ___ _ __  \n";
    std::cout << " / _` | | '__\\ \\ /\\ / / _` | '__/ _` |/ _ \\ '_ \\ \n";
    std::cout << "| (_| | | |   \\ V  V / (_| | | | (_| |  __/ | | |\n";
    std::cout << " \\__,_|_|_|    \\_/\\_/ \\__,_|_|  \\__,_|\\___|_| |_|\n";
    std::cout << RESET;
    std::cout << "                              directory bruteforcer\n\n";
}

void print_usage(const char* prog) {
    print_banner();
    std::cerr << "Usage: " << prog << " -u <url> -w <wordlist> [-t threads]\n\n";
    std::cerr << "Options:\n";
    std::cerr << "  -u <url>        target URL\n";
    std::cerr << "  -w <path>       path to wordlist file\n";
    std::cerr << "  -t <num>        number of threads (default: 10)\n";
    std::cerr << "  -h              show this help menu\n\n";
    std::cerr << "Example:\n";
    std::cerr << "  " << prog << " -u https://pentest-ground.com -w wordlist.txt -t 20\n";
}

int main(int argc, char* argv[]) {
    std::string wordlist_path;
    int thread_count = 10;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-u") == 0 && i + 1 < argc) {
            target = argv[++i];
        } else if (strcmp(argv[i], "-w") == 0 && i + 1 < argc) {
            wordlist_path = argv[++i];
        } else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
            thread_count = std::atoi(argv[++i]);
        } else if (strcmp(argv[i], "-h") == 0) {
            print_usage(argv[0]);
            return 0;
        }
    }

    if (target.empty() || wordlist_path.empty()) {
        std::cerr << "Error: -u and -w are both required.\n\n";
        print_usage(argv[0]);
        return 1;
    }

    if (!target.empty() && target.back() == '/') {
        target.pop_back();
    }

    std::ifstream file(wordlist_path);
    if (!file) {
        std::cerr << "Failed to open wordlist: " << wordlist_path << "\n";
        return 1;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) wordlist.push_back(line);
    }

    curl_global_init(CURL_GLOBAL_DEFAULT);

    print_banner();
    std::cout << BOLD << "Target:   " << RESET << target << "\n";
    std::cout << BOLD << "Wordlist: " << RESET << wordlist_path << " (" << wordlist.size() << " words)\n";
    std::cout << BOLD << "Threads:  " << RESET << thread_count << "\n\n";

    std::cout << "┌───────┬────────────────────────────────────────────┐\n";
    std::cout << "│ Code  │ Path                                       │\n";
    std::cout << "├───────┼────────────────────────────────────────────┤\n";

    auto start = std::chrono::steady_clock::now();

    // spin up threads
    std::vector<std::thread> threads;
    for (int i = 0; i < thread_count; i++) {
        threads.emplace_back(worker);
    }
    for (auto& t : threads) {
        t.join(); // wait for all threads to finish
    }

    auto end = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(end - start).count();

    std::cout << "└───────┴────────────────────────────────────────────┘\n\n";
    std::cout << BOLD << "Summary:\n" << RESET;
    std::cout << "  Checked:  " << checked.load() << " paths\n";
    std::cout << "  Found:    " << GREEN << found.load() << RESET << " results\n";
    std::cout << "  Time:     " << std::fixed << std::setprecision(2) << elapsed << "s\n";
    std::cout << "  Speed:    " << std::fixed << std::setprecision(1)
               << (checked.load() / (elapsed > 0 ? elapsed : 1)) << " req/s\n";

    curl_global_cleanup();
    return 0;
}