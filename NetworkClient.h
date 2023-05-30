/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2015 Sergey Svistunov (zenden2k@gmail.com)

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
#include <vector>

#include <curl/curl.h>
//#include <curl/types.h>
#ifdef NETWORKCLIENT_USEMUTEX
#include <mutex>
#endif

#include "Core/Utils/CoreUtils.h"

using NString = std::string;

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

    NetworkClient(void);
    ~NetworkClient(void);
    NetworkClient(NetworkClient const&) = delete;
    void operator=(NetworkClient const& x) = delete;

    /**
     * Adds a parameter to the POST request with the name and value
     */
    void addQueryParam(const NString& name, const NString& value);

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
     * The method is similar to the HTML form element - <input type = "file">.
    */
    void addQueryParamFile(const NString& name, const NString& fileName, const NString& displayName = "",
                           const NString& contentType = "");

    /**
     * Sets the value of the HTTP request header. To delete a header, pass in an empty string. To set an empty value, pass "\n".
     */
    void addQueryHeader(const NString& name, const NString& value);

    /**
     * Sets the URL for the next request.
    */
    void setUrl(const NString& url);

    /**
     * Performs a POST request.
     * @param data - the request body (for example, "param1=value&param2=value2").
     *  If data is an empty string, the parameters previously set using the addQueryParam() function are used.
     */
    bool doPost(const NString& data = "");

    /**
     * Sends a request to the address set by the function setUrl as parameters and files encoded in the
     * MULTIPART/FORM-DATA format. Similar to sending a form with a file from a web page.
     *
     */
    bool doUploadMultipartData();

    /**
     * Sending a file or data directly in the body of a POST request
     */
    bool doUpload(const NString& fileName, const NString& data);
    bool doGet(const std::string& url = "");
    std::string responseBody() const;
    int responseCode() const;
    NString errorString() const;
    void setUserAgent(const NString& userAgentStr);
    NString responseHeaderText() const;
    NString responseHeaderByName(const NString& name) const;
    NString responseHeaderByIndex(int index, NString& name) const;
    int responseHeaderCount() const;
    void setProgressCallback(curl_progress_callback func, void* data);
    NString urlEncode(const NString& str);
    NString getCurlResultString() const;
    void setCurlOption(int option, const NString& value);
    void setCurlOptionInt(int option, long value);
    void setMethod(const NString& str);
    void setProxy(const NString& host, int port, int type);
    void setProxyUserPassword(const NString& username, const NString password);
    void setReferer(const NString& str);
    void setOutputFile(const NString& str);
    void setUploadBufferSize(const int size);
    void setChunkOffset(double offset);
    void setChunkSize(double size);
    void setTreatErrorsAsWarnings(bool treat);
    int getCurlResult();
    CURL* getCurlHandle();
    static void Uninitialize();
    void enableResponseCodeChecking(bool enable);
private:
    enum CallBackFuncType { funcTypeBody, funcTypeHeader };

    struct CallBackData
    {
        CallBackFuncType funcType;
        NetworkClient* nmanager;
    };

    struct CustomHeaderItem
    {
        NString name;
        NString value;
    };

    struct QueryParam
    {
        bool isFile;
        NString name;
        NString value; // also filename
        NString displayName;
        NString contentType;
    };

    static size_t read_callback(void* ptr, size_t size, size_t nmemb, void* stream);
    static int ProgressFunc(void* clientp, double dltotal, double dlnow, double ultotal, double ulnow);
    static int private_static_writer(char* data, size_t size, size_t nmemb, void* buffer_in);
    int private_writer(char* data, size_t size, size_t nmemb);
    int private_header_writer(char* data, size_t size, size_t nmemb);
    size_t private_read_callback(void* ptr, size_t size, size_t nmemb, void* stream);
    static int set_sockopts(void* clientp, curl_socket_t sockfd, curlsocktype purpose);
    bool private_apply_method();
    void private_parse_headers();
    void private_cleanup_before();
    void private_cleanup_after();
    bool private_on_finish_request();
    void private_initTransfer();
    void private_checkResponse();
    static void curl_cleanup();

    int m_UploadBufferSize;
    CURL* curl_handle;
    FILE* m_hOutFile;
    std::string m_OutFileName;
    FILE* m_uploadingFile;
    std::string m_uploadData;
    ActionType m_currentActionType;
    int m_nUploadDataOffset;
    CallBackData m_bodyFuncData;
    curl_progress_callback m_progressCallbackFunc;
    CallBackData m_headerFuncData;
    NString m_url;
    void* m_progressData;
    CURLcode curl_result;
    int64_t m_CurrentFileSize;
    int64_t m_currentUploadDataSize;
    std::vector<QueryParam> m_QueryParams;
    std::vector<CustomHeaderItem> m_QueryHeaders;
    std::vector<CustomHeaderItem> m_ResponseHeaders;
    std::string internalBuffer;
    std::string m_headerBuffer;
    NString m_userAgent;
    char m_errorBuffer[CURL_ERROR_SIZE];
    std::string m_method;
    struct curl_slist* chunk_;
    bool enableResponseCodeChecking_;
    int64_t chunkOffset_;
    int64_t chunkSize_;
    bool treatErrorsAsWarnings_;
#ifdef NETWORKCLIENT_USEMUTEX
        static std::mutex _mutex;
#endif
    static bool _curl_init;
    static bool _is_openssl;
    static std::string certFileName;
};

#endif
