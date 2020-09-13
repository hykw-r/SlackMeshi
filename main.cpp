#include<iostream>
#include<fstream>
#include<sstream>
#include<unordered_map>
#include<array>
#include<string>
#include<curl/curl.h>
#include<opencv2/opencv.hpp>

static CURL *curl;

/**
 * 文字列を区切り文字で分割する
 * @param str 文字列
 * @param del 区切り文字
 * @return std::array<std::string, 2>
 */
std::array<std::string, 2> split(std::string str, char del)
{
    int pos = str.find_first_of(del);
    std::string first = str.substr(0, pos);
    std::string second = str.substr(pos + 1);
    std::array<std::string, 2> result = {first, second};

    return result;
}

/**
 * slackAPIを叩く
 * @param token OAUTHトークン
 */
void post(std::string token, std::string emoji_name)
{
    std::string params = "token=" + token + "&profile=%7B%22status_emoji%22%3A%22%3A" + emoji_name + "%3A%22%7D";
    curl_easy_setopt(curl, CURLOPT_URL, "https://slack.com/api/users.profile.set");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)params.length());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, params.c_str());
    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_perform(curl);
}


int main(void)
{
    // configファイルの取得
    std::ifstream ifs("config.ini", std::ios::in);
    if (!ifs) {
        std::cerr << "Error: file not opened." << std::endl;
        return 1;
    }
    std::unordered_map<std::string, std::string> config_map;
    std::string tmp;
    while (getline(ifs, tmp)) {
        std::array<std::string, 2> key_value = split(tmp, '=');
        config_map[key_value[0]] = key_value[1];
    }

    // curl初期化
    curl = curl_easy_init();

    // カメラキャプチャと顔認識
    cv::Mat frame;
    cv::VideoCapture cap(0);
    std::vector<cv::Rect> faces;
    cv::CascadeClassifier cascade;
    cascade.load("./haarcascade_frontalface_alt.xml");
    while (1) {
        cap.read(frame);
        cascade.detectMultiScale(frame, faces, 1.1, 3, 0, cv::Size(20, 20));
        // 顔が認識されればステータスを仕事中にする
        if (faces.size() > 0) {
            post(config_map.at("OAUTH_ACCESS_TOKEN"), "male-technologist");
            cv::rectangle(frame, cv::Point(faces[0].x, faces[0].y), cv::Point(faces[0].x + faces[0].width, faces[0].y + faces[0].height), cv::Scalar(0, 0, 255), 3, 16);
        } else { // 認識されなければステータスを飯にする
            post(config_map.at("OAUTH_ACCESS_TOKEN"), "rice");
        }

        imshow("window", frame);
        cv::waitKey(1);
    }

    // curlクリア
    curl_easy_cleanup(curl);

    return 0;
}