/*

    curl-cpp-wrapper (NetworkClient)

    Copyright 2023 Sergey Svistunov (zenden2k@gmail.com)

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

*/

#ifndef CURL_CPP_WRAPPER_NETWORK_CLIENT_H
#define CURL_CPP_WRAPPER_NETWORK_CLIENT_H

#include <string>
#include <utility>
#include <vector>

#include <curl/curl.h>

class NetworkClient
{
public:
    enum ActionType
    {
        atNone = 0,
        atPost,
        atUpload,
        atGet
    };

    NetworkClient();
    ~NetworkClient();
    NetworkClient(NetworkClient const&) = delete;
    void operator=(NetworkClient const& x) = delete;

    /**
     * Adds a parameter to the POST request with the name and value
     */
    NetworkClient& addQueryParam(const std::string& name, const std::string& value);

    /**
     * Adds a file parameter to the MULTIPART/DATA POST request.
     *
     * @param name is the name of the request parameter
     *
     * @param fileName is the physical path to the file
     *
     * @param displayName is the display name (the name that is transferred to the server does not contain a path),
     *
     * @param contentType is the mime file type, can be an empty string or obtained using the GetFileMimeType function).
     * The method is similar to the HTML form element - <input type="file">.
    */
    NetworkClient& addQueryParamFile(const std::string& name, const std::string& fileName, const std::string& displayName = "",
                           const std::string& contentType = "");

    /**
     * Sets the value of the HTTP request header. To delete a header, pass in an empty string.
     * To set an empty value, pass new line.
     */
    NetworkClient& addQueryHeader(const std::string& name, const std::string& value);

    /**
     * Sets the URL for the next request.
    */
    NetworkClient& setUrl(const std::string& url);

    /**
     * Performs a POST request.
     * @param data - the request body (for example, "param1=value&param2=value2").
     *  If data is an empty string, the parameters previously set using the addQueryParam() function are used.
     */
    bool doPost(const std::string& data = "");

    /**
     * Sends a request to the address set by the function setUrl as parameters and files encoded in the
     * MULTIPART/FORM-DATA format. Similar to sending a form with a file from a web page.
     *
     */
    bool doUploadMultipartData();

    /**
     * Sending a file or data directly in the body of a POST request
     */
    bool doUpload(const std::string& fileName, const std::string& data);
    bool doGet(const std::string& url = "");
    std::string responseBody() const;
    int responseCode() const;
    std::string errorString() const;
    NetworkClient& setUserAgent(const std::string& userAgentStr);
    std::string responseHeaderText() const;
    std::string responseHeaderByName(const std::string& name) const;
    std::string responseHeaderByIndex(int index, std::string& name) const;
    size_t responseHeaderCount() const;
    NetworkClient& setProgressCallback(curl_progress_callback func, void* data);
    std::string urlEncode(const std::string& str);
    std::string getCurlResultString() const;
    NetworkClient& setCurlOption(int option, const std::string& value);
    NetworkClient& setCurlOptionInt(int option, long value);
    NetworkClient& setMethod(const std::string& str);
    NetworkClient& setProxy(const std::string& host, int port, int type);
    NetworkClient& setProxyUserPassword(const std::string& username, const std::string& password);
    NetworkClient& setReferer(const std::string& str);
    NetworkClient& setOutputFile(const std::string& str);
    NetworkClient& setUploadBufferSize(int size);
    NetworkClient& setChunkOffset(int64_t offset);
    NetworkClient& setChunkSize(int64_t size);
    int getCurlResult() const;
    CURL* getCurlHandle();
private:
    enum CallBackFuncType { funcTypeBody, funcTypeHeader };

    struct CallBackData
    {
        CallBackFuncType funcType;
        NetworkClient* nmanager;
    };

    struct CustomHeaderItem
    {
        std::string name;
        std::string value;
        CustomHeaderItem() = default;
        CustomHeaderItem(std::string n, std::string v): name(std::move(n)), value(std::move(v)) {
            
        }
    };

    struct QueryParam
    {
        bool isFile;
        std::string name;
        std::string value; // also filename
        std::string displayName;
        std::string contentType;
    };

    static size_t read_callback(void* ptr, size_t size, size_t nmemb, void* stream);
    static size_t private_progress_func(void* clientp, double dltotal, double dlnow, double ultotal, double ulnow);
    static size_t private_static_writer(char* data, size_t size, size_t nmemb, void* buffer_in);
    size_t private_writer(char* data, size_t size, size_t nmemb);
    size_t private_header_writer(char* data, size_t size, size_t nmemb);
    size_t private_read_callback(void* ptr, size_t size, size_t nmemb, void* stream);
    static int private_seek_callback(void *userp, curl_off_t offset, int origin);
    static int set_sockopts(void* clientp, curl_socket_t sockfd, curlsocktype purpose);
    bool private_apply_method();
    void private_parse_headers();
    void private_cleanup_before();
    void private_cleanup_after();
    bool private_on_finish_request();
    void private_init_transfer();

    int uploadBufferSize_;
    CURL* curlHandle_;
    FILE* outFile_;
    std::string outFileName_;
    FILE* uploadingFile_;
    std::string uploadData_;
    ActionType currentActionType_;
    size_t uploadDataOffset_;
    CallBackData bodyFuncData_;
    curl_progress_callback progressCallback_;
    CallBackData headerFuncData_;
    std::string url_;
    void* progressData_;
    CURLcode curlResult_;
    int64_t currentFileSize_;
    int64_t currentUploadDataSize_;
    std::vector<QueryParam> queryParams_;
    std::vector<CustomHeaderItem> queryHeaders_;
    std::vector<CustomHeaderItem> responseHeaders_;
    std::string internalBuffer_;
    std::string headerBuffer_;
    std::string userAgent_;
    char errorBuffer_[CURL_ERROR_SIZE];
    std::string method_;
    struct curl_slist* chunk_;
    int64_t chunkOffset_;
    int64_t chunkSize_;
    bool curlWinUnicode_;
};

#endif
