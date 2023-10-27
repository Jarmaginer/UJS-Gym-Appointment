#include <iostream>
#include <windows.h>
#include <conio.h> // Add this header file
#include <stdlib.h>
#include <string>
#include <curl/curl.h>
#include <vector>
#include <regex>
#include <iomanip>
#include <ctime>
#include <json/json.h>
#include <locale>
#include <codecvt>

#define LOGIN_URL (std::string)"https://pass.ujs.edu.cn/cas/login?service=http%3A%2F%2Fgym.ujs.edu.cn%2Findex.php%2Findex%2Findex%2Flogin%3Ftype%3D1"

std::string GBKToUTF8(const std::string &gbkString) {
    int len = MultiByteToWideChar(CP_ACP, 0, gbkString.c_str(), -1, NULL, 0);
    wchar_t *wideStr = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, gbkString.c_str(), -1, wideStr, len);

    len = WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, NULL, 0, NULL, NULL);
    char *utf8Str = new char[len];
    WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, utf8Str, len, NULL, NULL);

    std::string result(utf8Str);
    delete[] wideStr;
    delete[] utf8Str;
    return result;
}


inline size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response) {
        size_t totalSize = size * nmemb;
        response->append((char*)contents, totalSize);
        return totalSize;
}



std::string POST(const std::string &url, const std::string &postParams, const std::vector<std::string>& headers)
    {

        std::cout<<"[POST]" << url << postParams << std::endl;
        // curl初始化  
        CURL *curl = curl_easy_init();
        // curl返回值 
        CURLcode res;
        std::string response;
        if (curl)
        {
            // set params
            //设置curl的请求头
            struct curl_slist* headerList = nullptr;
            for (const auto& header : headers) {
                headerList = curl_slist_append(headerList, header.c_str());
            }
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);

            //不接收响应头数据0代表不接收 1代表接收
            curl_easy_setopt(curl, CURLOPT_HEADER, 0);

            //设置请求为post请求
            curl_easy_setopt(curl, CURLOPT_POST, 1);
            curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");
            //设置请求的URL地址
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            //设置post请求的参数
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postParams.c_str());

            //设置ssl验证
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);

            //CURLOPT_VERBOSE的值为1时，会显示详细的调试信息
            curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);

            curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);

            //设置数据接收和写入函数
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);

            curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);

            //设置超时时间
            curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 0.1);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 1);

            // 开启post请求
            res = curl_easy_perform(curl);
        }
        //释放curl 
        curl_easy_cleanup(curl);
        if (res != CURLE_OK) {
            return "";
        }
        return response;
    }

std::string GET(const std::string& url, const std::vector<std::string>& headers) {

        std::cout<<"[GET]" << url  << std::endl;
        CURL* curl;
        CURLcode res;
        std::string response;
        long httpCode = 0;

        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();
        if(curl) {
            // 设置URL
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            // 设置请求头
            struct curl_slist* headerList = nullptr;
            for (const auto& header : headers) {
                headerList = curl_slist_append(headerList, header.c_str());
            }
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);
            curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");
            // 设置回调函数来处理响应数据
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);
            // 发送请求
            res = curl_easy_perform(curl);
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
            response = std::to_string(httpCode) + " " + response;
            // 清理资源
            curl_easy_cleanup(curl);
            curl_slist_free_all(headerList);

            if(res != CURLE_OK) {
                fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            }
        }
        curl_global_cleanup();
        return response;
    }

std::string getDate() {
    std::time_t currentTime = std::time(nullptr);

    std::tm* localTime = std::localtime(&currentTime);

    // 格式化日期为 %Y-%m-%d
    std::stringstream ss;
    ss << std::put_time(localTime, "%Y-%m-%d");

    std::string formattedDate = ss.str();

    
    return formattedDate;
}

inline std::string toTwoDigits(int num) {
    if (num >= 1 && num <= 9) {
        std::ostringstream oss;
        oss << "0" << num;
        return oss.str();
    } else {
        return std::to_string(num);
    }
}


class Gym{
public:
    std::string areaid = GBKToUTF8("未设置");
    std::string cookie;
    std::string token = GBKToUTF8("未登录，请先登录");
    std::string dateid = GBKToUTF8("未设置");
    
    std::vector<std::string> headers = {
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7",
        "Accept-Encoding: gzip, deflate, br",
        "Accept-Language: en,zh-CN;q=0.9,zh;q=0.8,en-GB;q=0.7,en-US;q=0.6",
        "Connection: keep-alive",
        "Host: pass.ujs.edu.cn",
        "Referer: http://gym.ujs.edu.cn/",
        "Sec-Fetch-Dest: document",
        "Sec-Fetch-Mode: navigate",
        "Sec-Fetch-Site: cross-site",
        "Sec-Fetch-User: ?1",
        "Upgrade-Insecure-Requests: 1",
        "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/118.0.0.0 Safari/537.36 Edg/118.0.2088.46",
        "sec-ch-ua: \"Chromium\";v=\"118\", \"Microsoft Edge\";v=\"118\", \"Not=A?Brand\";v=\"99\"",
        "sec-ch-ua-mobile: ?0",
        "sec-ch-ua-platform: \"Windows\""
    };

    std::vector<std::string> headers2 = {
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7",
        "Accept-Encoding: gzip, deflate",
        "Accept-Language: en,zh-CN;q=0.9,zh;q=0.8,en-GB;q=0.7,en-US;q=0.6",
        "Connection: keep-alive",
        "Host: gym.ujs.edu.cn",
        "Upgrade-Insecure-Requests: 1",
        "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/118.0.0.0 Safari/537.36 Edg/118.0.2088.46"
    };

    std::vector<std::string> headers3 = {
        "Accept: application/json, text/javascript, */*; q=0.01",
        "Accept-Encoding: gzip, deflate",
        "Accept-Language: en,zh-CN;q=0.9,zh;q=0.8,en-GB;q=0.7,en-US;q=0.6",
        "Connection: keep-alive",
        "Content-Length: 41",
        "Content-Type: application/x-www-form-urlencoded; charset=UTF-8",
        "Host: gym.ujs.edu.cn",
        "Origin: http://gym.ujs.edu.cn",
        "Referer: http://gym.ujs.edu.cn/index.php/index/yuyue/list.html?item_id=2",
        "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/118.0.0.0 Safari/537.36 Edg/118.0.2088.57",
        "X-Requested-With: XMLHttpRequest"
    };

        std::vector<std::string> headers4 = {
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7",
        "Accept-Encoding: gzip, deflate",
        "Accept-Language: en,zh-CN;q=0.9,zh;q=0.8,en-GB;q=0.7,en-US;q=0.6",
        "Cache-Control: max-age=0",
        "Connection: keep-alive",
        "Content-Length: 77",
        "Content-Type: application/x-www-form-urlencoded",
        "Host: gym.ujs.edu.cn",
        "Origin: http://gym.ujs.edu.cn",
        "Referer: http://gym.ujs.edu.cn/index.php/index/yuyue/list.html?item_id=2",
        "Upgrade-Insecure-Requests: 1",
        "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/118.0.0.0 Safari/537.36 Edg/118.0.2088.57"
    };
    void setCookie(std::string cookie){

        this->cookie = cookie;
    }


std::string extractToken(const std::string& input) {
        std::regex pattern("(ST-[a-zA-Z0-9-]+)-cas");

        std::smatch match;
        if (std::regex_search(input, match, pattern)) {
            return match.str();
}

        return ""; 
    }



std::vector<std::string> extractArea(const std::string& input) {
    std::regex pattern("area-id=\"(\\d+)\">(.*?)<");
    std::vector<std::string> matches;
    std::cout << input << std::endl;
    auto words_begin = std::sregex_iterator(input.begin(), input.end(), pattern);
    auto words_end = std::sregex_iterator();

    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
        std::smatch match = *i;
        std::string areaId = match[1];
        std::string content = match[2];
        std::string content2 = content;

        std::cout << areaId << std::endl;
        areaId = toTwoDigits(std::stoi(areaId));
        std::string status = GetTimesolt(areaId);

        std::string result = "area-id=" + areaId + ": " + content + " " + findAvailableTimes(status);
        // Remove newline characters from the result string
        result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
        matches.push_back(result);
    }

    return matches;
}


    std::string Login(){
        std::cout << "正在尝试登录.... " << std::endl;
        headers.push_back((std::string)"Cookie: CASTGC="+cookie);
        std::string response = GET(LOGIN_URL, headers);
        this->token = extractToken(response);
        std::string url2 = (std::string)"http://gym.ujs.edu.cn/index.php/index/index/login?type=1&ticket="+token;
        std::string url3 = (std::string)"http://gym.ujs.edu.cn/index.php/index/index/login?type=1";
        headers.pop_back();
        std::cout << token << std::endl;
        //已经确认这里的token过期了
        headers2.push_back((std::string)"Cookie: PHPSESSID="+token);
        std::string response2 = GET(url2, headers2);
        std::string response3 = GET(url3, headers2);
        return response2 + response3;
    }

    
    std::string GetTimesolt(std::string areaid){
        headers3.push_back((std::string)"Cookie: PHPSESSID="+token);
        std::string URL = "http://gym.ujs.edu.cn/index.php/index/item/timeslot.html";
        std::string Date = getDate();
        std::cout << "requesting:"<<"areaid="+(areaid)+"@"+Date <<std::endl;
        std::string response = POST(URL, "item_id=3&area_id="+areaid+"&date_time="+Date, headers3);
        std::cout << "response:" << response << std::endl;
        return response;
    }

    std::string GetOrderID(){
        Json::CharReaderBuilder reader;
        Json::Value root;

        std::string errs;

        headers3.push_back((std::string)"Cookie: PHPSESSID="+token);
        std::string URL = "http://gym.ujs.edu.cn/index.php/index/item/timeslot.html";
        std::string Date = getDate();
        std::string response = POST(URL, "item_id=3&area_id="+areaid+"&date_time="+Date, headers3);
        std::istringstream stream(response);
        Json::parseFromStream(reader, stream, &root, &errs);
        std::string orderid = root["order_num"].asString();
        return orderid;
    }


    struct TimeSlot {
        std::string start_time;
        std::string end_time;
    };


std::string findAvailableTimes(const std::string& jsonString) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    Json::CharReaderBuilder reader;
    Json::Value root;
    std::istringstream stream(jsonString);
    std::string errs;
    Json::parseFromStream(reader, stream, &root, &errs);

    if (root["code"].asInt() == 1) {
        std::vector<std::string> availableTimes;
        const auto& data = root["data"];
        for (const auto& entry : data) {
            if (entry["check"].asString() != "disable") {
                availableTimes.push_back(entry["time"].asString()+"("+entry["id"].asString()+")");
            }
        }

        if (availableTimes.empty()) {
            return converter.to_bytes(L"无可用时间\n");
        } else {

            std::wstring result = std::to_wstring(availableTimes.size()) + L"个可用时间：";
            for (const auto& time : availableTimes) {
                result += converter.from_bytes(time) + L"   ";
            }
            return converter.to_bytes(result);
        }
    } else {
        return converter.to_bytes(L"无可用时间\n");
    }
}


    int setAreaID(std::string areaid){
        this->areaid = areaid;
        return 0;
    }

    int setDateID(std::string dateid){
        this->dateid = dateid;
        return 0;
    }

    int PrintAreaID() {
        headers2.push_back((std::string)"Cookie: PHPSESSID=" + token);
        //std::string URL = "http://gym.ujs.edu.cn/index.php/index/yuyue/list.html?item_id=3";
        //std::string response = GET(URL, headers2);
        //SetConsoleOutputCP(CP_UTF8);

        std::string response = GBKToUTF8(R"(<div class="office-time-con" style="width: 1000px">
                                        <div class="fenhang">
                                          <div class="office-time-item area_  checked" area-id="4">一楼塑胶1</div>
                                          <div class="office-time-item area_" area-id="5">一楼塑胶2</div>
                                          <div class="office-time-item area_" area-id="6">一楼塑胶3</div>
                                          </div>
                                          <div class="fenhang">
                                          <div class="office-time-item area_" area-id="7">一楼木质4</div>
                                          <div class="office-time-item area_" area-id="8">一楼塑胶5</div>
                                          <div class="office-time-item area_" area-id="9">一楼塑胶6</div>
                                          </div>
                                          <div class="fenhang">
                                          <div class="office-time-item area_" area-id="10">一楼塑胶7</div>
                                          <div class="office-time-item area_" area-id="11">一楼木质8</div>
                                          <div class="office-time-item area_" area-id="12">二楼塑胶1</div>
                                          </div>
                                          <div class="fenhang">
                                          <div class="office-time-item area_" area-id="13">二楼塑胶2</div>
                                          <div class="office-time-item area_" area-id="14">二楼塑胶3</div>
                                          <div class="office-time-item area_" area-id="15">二楼塑胶4</div>
                                          </div>
                                          <div class="fenhang">
                                          <div class="office-time-item area_" area-id="16">二楼塑胶5</div>
                                          <div class="office-time-item area_" area-id="17">二楼塑胶6</div>
                                          <div class="office-time-item area_" area-id="18">二楼塑胶7</div>
                                          </div>
                                          <div class="fenhang">
                                          <div class="office-time-item area_" area-id="19">二楼塑胶8</div>
                                          <div class="office-time-item area_" area-id="20">二楼塑胶9</div>
                                          <div class="office-time-item area_" area-id="21">二楼塑胶10</div>
                                          </div>
                                          <div class="fenhang">
                                          <div class="office-time-item area_" area-id="22">二楼塑胶11</div>
                                          <div class="office-time-item area_" area-id="23">二楼塑胶12</div>
                                    </div>
              </div>)");
        std::vector<std::string> matches = extractArea(response);
        for (const auto& match : matches) {
            std::cout << match << std::endl;
        }
        return 0;
    }


    std::string Pay(){
        headers4.push_back((std::string)"Cookie: PHPSESSID="+token);
        std::string URL = "http://gym.ujs.edu.cn/index.php/index/pay/index.html";
        std::string response = POST(URL, "data_id="+dateid+"&item_id=2&area_id="+areaid+"&date_time="+getDate()+"&order_id="+GetOrderID(), headers4);
        return response;
    }

    std::string GetInf(){
        headers2.push_back((std::string)"Cookie: PHPSESSID="+token);
        std::string URL = "http://gym.ujs.edu.cn/index.php/index/user/index.html?page=1";
        std::string response = GET(URL,headers2);
        return response;
    }
};



using namespace std;


void gotoxy(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    string cas_cookie;

    const int numOptions = 6;
    string options[numOptions] = {GBKToUTF8("开抢"),GBKToUTF8("登录"), GBKToUTF8("设置待抢场馆信息"),GBKToUTF8("获取场地信息"),GBKToUTF8("获取预约信息"), GBKToUTF8("退出程序")};
    int selectedOption = 1;
    Gym gym;
    while (true) {
        system("cls"); // 清屏
        cout << "token="<<gym.token <<"  areaid="<< gym.areaid <<"  dataid="<< gym.dateid  << endl;
        cout << "------------------------------------- " << endl;
        // 输出选项
        for (int i = 0; i < numOptions; ++i) {
            if (i == selectedOption) {
                cout << "-> " << options[i] << endl;
            } else {
                cout << "   " << options[i] << endl;
            }
        }

        // 等待用户输入
        char input = _getch();

        // 处理用户输入
        switch (input) {
            case 72: // 上箭头
                selectedOption = (selectedOption - 1 + numOptions) % numOptions;
                break;
            case 80: // 下箭头
                selectedOption = (selectedOption + 1) % numOptions;
                break;
            case 13: // 回车键
                system("cls");
                switch (selectedOption){
                    case 0:
                        gym.Pay();
                        break;
                    case 1:
                        cout << GBKToUTF8("正在登录...\n\n提供你的cas-cookie：");
                        cin >> cas_cookie;
                        gym.setCookie(cas_cookie);
                        gym.Login();
                        break;
                    case 3:
                        gym.PrintAreaID();
                        break;
                    case 4:
                        cout << gym.GetInf() << endl;
                        break;
                    case 2:
                        cout << GBKToUTF8("设置area_id和data_id\n") << endl;
                        cout << "area_id: "<< endl;
                        cin >> gym.areaid;
                        gym.setAreaID(gym.areaid);
                        cout << "data_id: " << endl;
                        cin >> gym.dateid;
                        gym.setDateID(gym.dateid);

                        break;
                    case 5:
                        return 0;
                        break;
                }
                _getch(); // 等待用户按任意键继续
                break;
        }
    }

    return 0;
}
