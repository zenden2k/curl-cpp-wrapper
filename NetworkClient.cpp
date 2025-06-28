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

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "NetworkClient.h"

#include <cstring>
#include <cstdio>
#include <algorithm>
#include <iostream>

#if defined(_WIN32) || defined(__CYGWIN__)
#include <Windows.h>
#endif

namespace NetworkClientInternal {

void SplitString(const std::string& str, const std::string& delimiters, std::vector<std::string>& tokens,
                 int maxCount = -1) {
    // Skip delimiters at beginning.
    std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    std::string::size_type pos = str.find_first_of(delimiters, lastPos);
    int counter = 0;
    while (std::string::npos != pos || std::string::npos != lastPos) {
        counter++;
        if (counter == maxCount) {
            tokens.push_back(str.substr(lastPos, str.length()));
            break;
        } else

            // Found a token, add it to the vector.
            tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}

std::string TrimString(const std::string& str) {
    std::string res;
    // Trim Both leading and trailing spaces
    size_t startpos = str.find_first_not_of(" \t\r\n");
    // Find the first character position after excluding leading blank spaces
    size_t endpos = str.find_last_not_of(" \t\r\n"); // Find the first character position from reverse af

    // if all spaces or empty return an empty string
    if ((std::string::npos == startpos) || (std::string::npos == endpos)) {
        res.clear();
    } else
        res = str.substr(startpos, endpos - startpos + 1);
    return res;
}

std::string StrToLower(const std::string& str) {
    std::string s1 = str;
    for (size_t i = 0; i < s1.length(); i++)
        s1[i] = static_cast<char>(::tolower(s1[i]));
    return s1;
}

#ifdef _WIN32
std::wstring StrToWide(const std::string& str, UINT codePage) {
    std::wstring ws;
    int n = MultiByteToWideChar(codePage, 0, str.c_str(), str.size() + 1, /*dst*/nullptr, 0);
    if (n) {
        ws.reserve(n);
        ws.resize(n - 1);
        if (MultiByteToWideChar(codePage, 0, str.c_str(), str.size() + 1, /*dst*/&ws[0], n) == 0)
            ws.clear();
    }
    return ws;
}

std::wstring Utf8ToWide(const std::string& str) {
    return StrToWide(str, CP_UTF8);
}

std::string WideToStr(const std::wstring& ws, UINT codePage)
{
    // prior to C++11 std::string and std::wstring were not guaranteed to have their memory be contiguous,
    // although all real-world implementations make them contiguous
    std::string str;
    int srcLen = static_cast<int>(ws.size());
    int n = WideCharToMultiByte(codePage, 0, ws.c_str(), srcLen + 1, nullptr, 0, /*defchr*/nullptr, nullptr);
    if (n) {
        str.reserve(n);
        str.resize(n - 1);
        if (WideCharToMultiByte(codePage, 0, ws.c_str(), srcLen + 1, &str[0], n, /*defchr*/nullptr, nullptr) == 0)
            str.clear();
    }
    return str;
}

std::string WideToUtf8(const std::wstring& str)
{
    return WideToStr(str, CP_UTF8);
}

std::string Utf8ToSystemLocale(const std::string& str)
{
    std::wstring wideStr = Utf8ToWide(str);
    return WideToStr(wideStr, CP_ACP);
}
#endif

FILE* Fopen(const char* filename, const char* mode) {
#ifdef _MSC_VER
    return _wfopen(Utf8ToWide(filename).c_str(), Utf8ToWide(mode).c_str());
#else
    return fopen(filename, mode);
#endif
}

int Fseek64( FILE* stream, int64_t  offset, int origin) {
#ifdef _MSC_VER
    return _fseeki64(stream, offset, origin);
#else
    return fseeko(stream, offset, origin);
#endif
}

int64_t Ftell64(FILE* a)
{
#ifdef __CYGWIN__
    return ftell(a);
#elif defined (_WIN32)
    return _ftelli64(a);
#else
    return ftello(a);
#endif
}

struct CurlInitializer {
    std::string certFileName;

    CurlInitializer() {
        curl_global_init(CURL_GLOBAL_ALL);
#ifdef _WIN32
        wchar_t buffer[1024] = {0};
        if (GetModuleFileNameW(nullptr, buffer, 1024) != 0) {
            int len = lstrlenW(buffer);
            for (int i = len; i >= 0; i--) {
                if (buffer[i] == '\\') {
                    buffer[i + 1] = 0;
                    break;
                }
            }

            curl_version_info_data* versionInfo = curl_version_info(CURLVERSION_NOW);
            #ifdef CURL_VERSION_UNICODE
            if (versionInfo->features & CURL_VERSION_UNICODE ) {
                certFileName = WideToUtf8(buffer);
            }
            else
            #endif
            {
                certFileName = WideToStr(buffer, CP_ACP);
            }
            certFileName += "curl-ca-bundle.crt";
        }
#endif
    }

    ~CurlInitializer() {
        curl_global_cleanup();
    }
};
}

NetworkClient::NetworkClient():
    outFile_(nullptr),
    uploadingFile_(nullptr),
    currentActionType_(atNone),
    uploadDataOffset_(0),
    progressCallback_(nullptr),
    progressData_(nullptr),
    curlResult_(CURLE_OK),
    currentFileSize_(-1),
    currentUploadDataSize_(0),
    chunk_(nullptr),
    chunkOffset_(-1),
    chunkSize_(-1),
    curlWinUnicode_(false)
{
    static NetworkClientInternal::CurlInitializer initializer;

    *errorBuffer_ = 0;
    curlHandle_ = curl_easy_init();
    bodyFuncData_.funcType = funcTypeBody;
    bodyFuncData_.nmanager = this;
    uploadBufferSize_ = 65536;
    headerFuncData_.funcType = funcTypeHeader;
    headerFuncData_.nmanager = this;

    curl_easy_setopt(curlHandle_, CURLOPT_COOKIELIST, "");
    setUserAgent("Mozilla/5.0");

    curl_easy_setopt(curlHandle_, CURLOPT_WRITEFUNCTION, private_static_writer);
    curl_easy_setopt(curlHandle_, CURLOPT_WRITEDATA, &bodyFuncData_);
    curl_easy_setopt(curlHandle_, CURLOPT_WRITEHEADER, &headerFuncData_);
    curl_easy_setopt(curlHandle_, CURLOPT_ERRORBUFFER, errorBuffer_);

    curl_easy_setopt(curlHandle_, CURLOPT_PROGRESSFUNCTION, &private_progress_func);
    curl_easy_setopt(curlHandle_, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curlHandle_, CURLOPT_PROGRESSDATA, this);
    curl_easy_setopt(curlHandle_, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curlHandle_, CURLOPT_ENCODING, "");
    curl_easy_setopt(curlHandle_, CURLOPT_SOCKOPTFUNCTION, &set_sockopts);
    curl_easy_setopt(curlHandle_, CURLOPT_SOCKOPTDATA, this);

#if defined(_WIN32)
    curl_version_info_data* versionInfo = curl_version_info(CURLVERSION_NOW);

    if (versionInfo->features & CURL_VERSION_SSL && strstr(versionInfo->ssl_version, "Schannel") == nullptr
        && strstr(versionInfo->ssl_version, "WinSSL") == nullptr
        ) {
        curl_easy_setopt(curlHandle_, CURLOPT_CAINFO, initializer.certFileName.c_str());
    }
    #ifdef CURL_VERSION_UNICODE
    curlWinUnicode_ = versionInfo->features & CURL_VERSION_UNICODE;

    if (!curlWinUnicode_) {
        std::cerr << "CURL should be compiled with Unicode support on Windows" << std::endl;
    }
    #endif
#endif
    curl_easy_setopt(curlHandle_, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curlHandle_, CURLOPT_SSL_VERIFYHOST, 2L);

    //We want the referrer field set automatically when following locations
    curl_easy_setopt(curlHandle_, CURLOPT_AUTOREFERER, 1L);
    curl_easy_setopt(curlHandle_, CURLOPT_BUFFERSIZE, 32768L);
    curl_easy_setopt(curlHandle_, CURLOPT_VERBOSE, 0L);
}

NetworkClient::~NetworkClient() {
    curl_easy_setopt(curlHandle_, CURLOPT_PROGRESSFUNCTION, nullptr);
    curl_easy_cleanup(curlHandle_);
}

int NetworkClient::set_sockopts(void* clientp, curl_socket_t sockfd, curlsocktype purpose) {
#ifdef _WIN32
    // See http://support.microsoft.com/kb/823764
    auto* nm = static_cast<NetworkClient*>(clientp);
    int val = nm->uploadBufferSize_ + 32;
    setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, reinterpret_cast<const char*>(&val), sizeof(val));
#endif
    return 0;
}

size_t NetworkClient::private_static_writer(char* data, size_t size, size_t nmemb, void* buffer_in) {
    auto cbd = static_cast<CallBackData*>(buffer_in);
    NetworkClient* nm = cbd->nmanager;
    if (nm) {
        if (cbd->funcType == funcTypeBody) {
            return nm->private_writer(data, size, nmemb);
        }
        else
            return nm->private_header_writer(data, size, nmemb);
    }
    return 0;
}

NetworkClient& NetworkClient::setProxy(const std::string& host, int port, int type) {
    curl_easy_setopt(curlHandle_, CURLOPT_PROXY, host.c_str());
    curl_easy_setopt(curlHandle_, CURLOPT_PROXYPORT, static_cast<long>(port));
    curl_easy_setopt(curlHandle_, CURLOPT_PROXYTYPE, static_cast<long>(type));
    curl_easy_setopt(curlHandle_, CURLOPT_NOPROXY, "");
    return *this;
}

NetworkClient& NetworkClient::setProxyUserPassword(const std::string& username, const std::string& password) {
    if (username.empty() && password.empty()) {
        curl_easy_setopt(curlHandle_, CURLOPT_PROXYUSERPWD, nullptr);
        curl_easy_setopt(curlHandle_, CURLOPT_PROXYAUTH, nullptr);
    }
    else {
        std::string authStr = urlEncode(username) + ":" + urlEncode(password);
        curl_easy_setopt(curlHandle_, CURLOPT_PROXYUSERPWD, authStr.c_str());
        curl_easy_setopt(curlHandle_, CURLOPT_PROXYAUTH, CURLAUTH_ANY);
    }
    return *this;
}

size_t NetworkClient::private_writer(char* data, size_t size, size_t nmemb) {
    if (!outFileName_.empty()) {
        if (!outFile_)
            if (!(outFile_ = NetworkClientInternal::Fopen(outFileName_.c_str(), "wb")))
                return 0;
        fwrite(data, size, nmemb, outFile_);
    }
    else
        internalBuffer_.append(data, size * nmemb);
    return size * nmemb;
}

size_t NetworkClient::private_header_writer(char* data, size_t size, size_t nmemb) {
    headerBuffer_.append(data, size * nmemb);
    return size * nmemb;
}

size_t NetworkClient::private_progress_func(void* clientp, double dltotal, double dlnow, double ultotal, double ulnow) {
    auto nm = static_cast<NetworkClient*>(clientp);
    if (nm && nm->progressCallback_) {
        if (nm->chunkOffset_ >= 0 && nm->chunkSize_ > 0 && nm->currentActionType_ == atUpload) {
            ultotal = static_cast<double>(nm->currentFileSize_);
            ulnow = static_cast<double>(nm->chunkOffset_) + ulnow;
        }
        else if (((ultotal <= 0 && nm->currentFileSize_ > 0)) && nm->currentActionType_ == atUpload)
            ultotal = static_cast<double>(nm->currentFileSize_);
        return nm->progressCallback_(nm->progressData_, dltotal, dlnow, ultotal, ulnow);
    }
    return 0;
}

NetworkClient& NetworkClient::setMethod(const std::string& str) {
    method_ = str;
    return *this;
}

NetworkClient& NetworkClient::addQueryParam(const std::string& name, const std::string& value) {
    QueryParam newParam;
    newParam.name = name;
    newParam.value = value;
    newParam.isFile = false;
    queryParams_.push_back(newParam);
    return *this;
}

NetworkClient& NetworkClient::addQueryParamFile(const std::string& name, const std::string& fileName, const std::string& displayName,
                                      const std::string& contentType) {
    QueryParam newParam;
    newParam.name = name;
    newParam.value = fileName;
    newParam.isFile = true;
    newParam.contentType = contentType;
    newParam.displayName = displayName;
    queryParams_.push_back(newParam);
    return *this;
}

NetworkClient& NetworkClient::setUrl(const std::string& url) {
    url_ = url;
    curl_easy_setopt(curlHandle_, CURLOPT_URL, url.c_str());
    return *this;
}

bool NetworkClient::doUploadMultipartData() {
    if (method_.empty()) {
        setMethod("POST");
    }
    
    private_init_transfer();
    private_apply_method();

    struct curl_httppost* formpost = nullptr;
    struct curl_httppost* lastptr = nullptr;

    for (const auto& it : queryParams_) {
        if (it.isFile) {
            std::string fileName = it.value;
#ifdef _WIN32
            if (!curlWinUnicode_) {
                fileName = NetworkClientInternal::Utf8ToSystemLocale(it.value);
            }
#endif

            if (it.contentType.empty())
                curl_formadd(&formpost,
                             &lastptr,
                             CURLFORM_COPYNAME, it.name.c_str(),
                             CURLFORM_FILENAME, it.displayName.c_str(),
                             CURLFORM_FILE, fileName.c_str(),
                             CURLFORM_END);
            else
                curl_formadd(&formpost,
                             &lastptr,
                             CURLFORM_COPYNAME, it.name.c_str(),
                             CURLFORM_FILENAME, it.displayName.c_str(),
                             CURLFORM_FILE, fileName.c_str(),
                             CURLFORM_CONTENTTYPE, it.contentType.c_str(),
                             CURLFORM_END);

        } else {
            curl_formadd(&formpost,
                         &lastptr,
                         CURLFORM_COPYNAME, it.name.c_str(),
                         CURLFORM_COPYCONTENTS, it.value.c_str(),
                         CURLFORM_END);
        }
    }
    
    curl_easy_setopt(curlHandle_, CURLOPT_HTTPPOST, formpost);
    currentActionType_ = atUpload;
    curlResult_ = curl_easy_perform(curlHandle_);
    curl_formfree(formpost);
    return private_on_finish_request();
}

bool NetworkClient::private_on_finish_request() {
    private_cleanup_after();
    private_parse_headers();
    if (curlResult_ != CURLE_OK) {
        return false; //fail
    }

    return true;
}

std::string NetworkClient::responseBody() const {
    return internalBuffer_;
}

int NetworkClient::responseCode() const {
    long result = -1;
    curl_easy_getinfo(curlHandle_, CURLINFO_RESPONSE_CODE, &result);
    return result;
}

NetworkClient& NetworkClient::addQueryHeader(const std::string& name, const std::string& value) {
    queryHeaders_.emplace_back(name, value);
    return *this;
}

bool NetworkClient::doGet(const std::string& url) {
    if (!url.empty())
        setUrl(url);

    private_init_transfer();
    if (!private_apply_method())
        curl_easy_setopt(curlHandle_, CURLOPT_HTTPGET, 1);
    currentActionType_ = atGet;
    curlResult_ = curl_easy_perform(curlHandle_);
    return private_on_finish_request();

}

bool NetworkClient::doPost(const std::string& data) {
    private_init_transfer();
    if (!private_apply_method())
        curl_easy_setopt(curlHandle_, CURLOPT_POST, 1L);
    std::string postData;

    for (const auto& it: queryParams_) {
        if (!it.isFile) {
            postData += urlEncode(it.name) + "=" + urlEncode(it.value) + "&";
        }
    }

    if(data.empty()) {
        curl_easy_setopt(curlHandle_, CURLOPT_POSTFIELDS, postData.c_str());
        curl_easy_setopt(curlHandle_, CURLOPT_POSTFIELDSIZE, static_cast<long>(postData.length()));
    }
    else {
        curl_easy_setopt(curlHandle_, CURLOPT_POSTFIELDS, (const char*)data.data());
        curl_easy_setopt(curlHandle_, CURLOPT_POSTFIELDSIZE, (long)data.length());
    }

    currentActionType_ = atPost;
    curlResult_ = curl_easy_perform(curlHandle_);
    return private_on_finish_request();
}

std::string NetworkClient::urlEncode(const std::string& str) {
    char* encoded = curl_easy_escape(curlHandle_, str.c_str(), str.length());
    std::string res = encoded;
    curl_free(encoded);
    return res;
}

std::string NetworkClient::errorString() const {
    return errorBuffer_;
}

NetworkClient& NetworkClient::setUserAgent(const std::string& userAgentStr) {
    userAgent_ = userAgentStr;
    return *this;
}

void NetworkClient::private_init_transfer() {
    private_cleanup_before();
    curl_easy_setopt(curlHandle_, CURLOPT_USERAGENT, userAgent_.c_str());

    chunk_ = nullptr;

    for (const auto& it: queryHeaders_) {
        if (it.value == "\n") {
            chunk_ = curl_slist_append(chunk_, (it.name + ";" + it.value).c_str());
        } else {
            chunk_ = curl_slist_append(chunk_, (it.name + ": " + it.value).c_str());
        }
    }

    curl_easy_setopt(curlHandle_, CURLOPT_HTTPHEADER, chunk_);
}

std::string NetworkClient::responseHeaderText() const {
    return headerBuffer_;
}

NetworkClient& NetworkClient::setProgressCallback(curl_progress_callback func, void* data) {
    progressCallback_ = func;
    progressData_ = data;
    return *this;
}

void NetworkClient::private_parse_headers() {
    std::vector<std::string> headers;
    NetworkClientInternal::SplitString(headerBuffer_, "\n", headers);

    for (const auto& it: headers) {
        std::vector<std::string> thisHeader;
        NetworkClientInternal::SplitString(it, ":", thisHeader, 2);

        if (thisHeader.size() == 2) {
            CustomHeaderItem chi;
            chi.name = NetworkClientInternal::TrimString(thisHeader[0]);
            chi.value = NetworkClientInternal::TrimString(thisHeader[1]);
            responseHeaders_.push_back(chi);
        }
    }
}

std::string NetworkClient::responseHeaderByName(const std::string& name) const {
    std::string lowerName = NetworkClientInternal::StrToLower(name);

    for (const auto& it: responseHeaders_) {
        if (NetworkClientInternal::StrToLower(it.name) == lowerName) {
            return it.value;
        }
    }
    return std::string();
}

size_t NetworkClient::responseHeaderCount() const {
    return responseHeaders_.size();
}

std::string NetworkClient::responseHeaderByIndex(int index, std::string& name) const {
    name = responseHeaders_[index].name;
    return responseHeaders_[index].value;
}

void NetworkClient::private_cleanup_before() {
    addQueryHeader("Expect", "");
    responseHeaders_.clear();
    internalBuffer_.clear();
    headerBuffer_.clear();
    curl_easy_setopt(curlHandle_, CURLOPT_READFUNCTION, nullptr);
    curl_easy_setopt(curlHandle_, CURLOPT_SEEKFUNCTION, nullptr);
    curl_easy_setopt(curlHandle_, CURLOPT_READDATA, stdin);
}

void NetworkClient::private_cleanup_after() {
    currentActionType_ = atNone;
    queryHeaders_.clear();
    queryParams_.clear();
    if (outFile_) {
        fclose(outFile_);
        outFile_ = nullptr;
    }
    outFileName_.clear();
    method_.clear();

    curl_easy_setopt(curlHandle_, CURLOPT_POSTFIELDS, nullptr);
    curl_easy_setopt(curlHandle_, CURLOPT_MIMEPOST, nullptr);
    curl_easy_setopt(curlHandle_, CURLOPT_INFILESIZE_LARGE, static_cast<curl_off_t>(-1));
    curl_easy_setopt(curlHandle_, CURLOPT_POSTFIELDSIZE_LARGE, static_cast<curl_off_t>(-1));
    curl_easy_setopt(curlHandle_, CURLOPT_HTTPHEADER, nullptr);
    curl_easy_setopt(curlHandle_, CURLOPT_CUSTOMREQUEST, nullptr);
    curl_easy_setopt(curlHandle_, CURLOPT_UPLOAD, 0L);
    curl_easy_setopt(curlHandle_, CURLOPT_NOBODY, 0L);
    curl_easy_setopt(curlHandle_, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curlHandle_, CURLOPT_READFUNCTION, nullptr);
    curl_easy_setopt(curlHandle_, CURLOPT_SEEKFUNCTION, nullptr);
    curl_easy_setopt(curlHandle_, CURLOPT_SEEKDATA, nullptr);
    curl_easy_setopt(curlHandle_, CURLOPT_READDATA, stdin);

    uploadData_.clear();
    uploadingFile_ = nullptr;
    chunkOffset_ = -1;
    chunkSize_ = -1;
    uploadDataOffset_ = 0;
    /*curl_easy_setopt(curl_handle, CURLOPT_READFUNCTION, 0L);
    curl_easy_setopt(curl_handle, CURLOPT_READDATA, 0L);*/
    if (chunk_) {
        curl_slist_free_all(chunk_);
        chunk_ = nullptr;
    }
}

size_t NetworkClient::read_callback(void* ptr, size_t size, size_t nmemb, void* stream) {
    auto nm = static_cast<NetworkClient*>(stream);
    if (!nm) return 0;
    return nm->private_read_callback(ptr, size, nmemb, stream);
}

size_t NetworkClient::private_read_callback(void* ptr, size_t size, size_t nmemb, void*) {
    size_t retcode;
    size_t wantsToRead = size * nmemb;
    if (uploadingFile_) {
        int64_t pos = NetworkClientInternal::Ftell64(uploadingFile_);
        if (chunkOffset_ >= 0 && pos >= chunkOffset_ + currentUploadDataSize_) {
            return 0;
        }
        retcode = fread(ptr, size, nmemb, uploadingFile_);
        if (chunkOffset_ != -1) {
            retcode = std::min<int64_t>((int64_t)retcode, chunkOffset_ + currentUploadDataSize_ - pos);
        }
    } else {
        size_t canRead = std::min<size_t>(uploadData_.size() - uploadDataOffset_, wantsToRead);
        memcpy(ptr, uploadData_.c_str(), canRead);
        uploadDataOffset_ += canRead;
        retcode = canRead;
    }
    return retcode;
}

int NetworkClient::private_seek_callback(void *userp, curl_off_t offset, int origin) {
    auto* nc = static_cast<NetworkClient*>(userp);

    if (nc->uploadingFile_) {
        int64_t newOffset = offset;
        int newOrigin = origin;
        if (origin == SEEK_SET && nc->chunkOffset_ != -1) {
            newOffset = nc->chunkOffset_ + offset;
        } else if (origin == SEEK_END) {
            // not implemented
            return CURL_SEEKFUNC_CANTSEEK;
        }
        return NetworkClientInternal::Fseek64(nc->uploadingFile_, newOffset, newOrigin);
    } else {
        if (origin == SEEK_SET) {
            if (offset < 0 || offset>= nc->uploadData_.size()) {
                return CURL_SEEKFUNC_CANTSEEK;
            }
            nc->uploadDataOffset_ = offset;
        } else if (origin == SEEK_CUR) {
            nc->uploadDataOffset_ += offset;
        } else {
            // not implemented
            return CURL_SEEKFUNC_CANTSEEK;
        }
        return CURL_SEEKFUNC_OK;
    }
}

bool NetworkClient::doUpload(const std::string& fileName, const std::string& data) {
    if (!fileName.empty()) {
        uploadingFile_ = NetworkClientInternal::Fopen(fileName.c_str(), "rb"); 
        if (!uploadingFile_) {
            return false; /* can't continue */
        }
        if (fseek(uploadingFile_, 0, SEEK_END) == 0) {
            currentFileSize_ = NetworkClientInternal::Ftell64(uploadingFile_);
            NetworkClientInternal::Fseek64(uploadingFile_, 0, SEEK_SET);
        } else {
            fclose(uploadingFile_);
            uploadingFile_ = nullptr;
            //currentFileSize_ = NetworkClientInternal::GetBigFileSize(fileName);
            return false;
        }
        
        currentUploadDataSize_ = currentFileSize_;
        if (currentFileSize_ < 0) {
            fclose(uploadingFile_);
            uploadingFile_ = nullptr;
            return false;
        }
        if (chunkSize_ > 0 && chunkOffset_ >= 0) {
            currentUploadDataSize_ = chunkSize_;
            if (NetworkClientInternal::Fseek64(uploadingFile_, chunkOffset_, SEEK_SET)) {
            }
        }
        currentActionType_ = atUpload;
    } else {
        uploadDataOffset_ = 0;
        uploadData_ = data;
        currentFileSize_ = data.length();
        currentUploadDataSize_ = currentFileSize_;
        currentActionType_ = atPost;
    }
    private_init_transfer();
    curl_easy_setopt(curlHandle_, CURLOPT_POSTFIELDS, nullptr);
    if (!private_apply_method()) {
        curl_easy_setopt(curlHandle_, CURLOPT_POST, 1L);
    }
    curl_easy_setopt(curlHandle_, CURLOPT_READFUNCTION, read_callback);
    curl_easy_setopt(curlHandle_, CURLOPT_READDATA, this);
    curl_easy_setopt(curlHandle_, CURLOPT_SEEKFUNCTION, private_seek_callback);
    curl_easy_setopt(curlHandle_, CURLOPT_SEEKDATA, this);

    curl_easy_setopt(curlHandle_, CURLOPT_POSTFIELDSIZE_LARGE, static_cast<curl_off_t>(currentUploadDataSize_));

    curl_easy_setopt(curlHandle_, CURLOPT_INFILESIZE_LARGE, static_cast<curl_off_t>(currentUploadDataSize_));

    curlResult_ = curl_easy_perform(curlHandle_);
    if (uploadingFile_)
        fclose(uploadingFile_);
    bool res = private_on_finish_request();
    return res;
}

bool NetworkClient::private_apply_method() {
    curl_easy_setopt(curlHandle_, CURLOPT_CUSTOMREQUEST,nullptr);
    curl_easy_setopt(curlHandle_, CURLOPT_UPLOAD, 0L);
    if (method_ == "POST")
        curl_easy_setopt(curlHandle_, CURLOPT_POST, 1L);
    else if (method_ == "GET")
        curl_easy_setopt(curlHandle_, CURLOPT_HTTPGET, 1L);
    else if (method_ == "PUT")
        curl_easy_setopt(curlHandle_, CURLOPT_UPLOAD, 1L);
    else if (!method_.empty()) {
        curl_easy_setopt(curlHandle_, CURLOPT_CUSTOMREQUEST, method_.c_str());
    } else {
        curl_easy_setopt(curlHandle_, CURLOPT_CUSTOMREQUEST, nullptr);
        return false;
    }
    return true;
}

NetworkClient& NetworkClient::setReferer(const std::string& str) {
    curl_easy_setopt(curlHandle_, CURLOPT_REFERER, str.c_str());
    return *this;
}

int NetworkClient::getCurlResult() const {
    return curlResult_;
}

CURL* NetworkClient::getCurlHandle() {
    return curlHandle_;
}

NetworkClient& NetworkClient::setOutputFile(const std::string& str) {
    outFileName_ = str;
    return *this;
}

NetworkClient& NetworkClient::setUploadBufferSize(int size) {
    uploadBufferSize_ = size;
    return *this;
}

NetworkClient& NetworkClient::setChunkOffset(int64_t offset) {
    chunkOffset_ = offset;
    return *this;
}

NetworkClient& NetworkClient::setChunkSize(int64_t size) {
    chunkSize_ = size;
    return *this;
}

std::string NetworkClient::getCurlResultString() const {
    return curl_easy_strerror(curlResult_);
}

NetworkClient& NetworkClient::setCurlOption(int option, const std::string& value) {
    curl_easy_setopt(curlHandle_, static_cast<CURLoption>(option), value.c_str());
    return *this;
}

NetworkClient& NetworkClient::setCurlOptionInt(int option, long value) {
    curl_easy_setopt(curlHandle_, static_cast<CURLoption>(option), value);
    return *this;
}
