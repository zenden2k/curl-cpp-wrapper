# curl-cpp-wrapper

A very simple HTTP client (C++ 11 wrapper for libcurl). It requires [libcurl](https://curl.se).

## Usage

Just add NetworkClient.cpp and NetworkClient.h files to your project.

Do a GET request:
```cpp
#include "NetworkClient.h"

NetworkClient nc;
nc.doGet("http://google.com/?q=" + nc.urlEncode("Smelly cat"));
std::cout << nc.responseBody();
```

Do a POST request (using method chaining):
```cpp
nc.setUrl("https://www.googleapis.com/oauth2/v3/token")
    .addQueryParam("refresh_token", refreshToken)
    .addQueryParam("client_id", clientId) 
    .addQueryParam("client_secret", clientSecret)
    .addQueryParam("grant_type", "refresh_token")
    .doPost();
std::cout << nc.responseBody();
```

Do a POST request with RAW data:
```cpp
nc.setUrl("https://www.googleapis.com/oauth2/v3/token");
nc.doPost("param1=value&param=value");
```

Downloading a file:
```cpp
NetworkClient nc;
nc.setOutputFile("d:\\image.png"); // file path should be UTF-8 encoded on Windows
nc.doGet("http://i.imgur.com/DDf2wbJ.png");
```

Uploading a file:
```cpp
NetworkClient nc;
std::string fileName = "c:\\test\\file.txt"; // file path should be UTF-8 encoded on Windows
nc.setUrl("http://takebin.com/action");
nc.addQueryParamFile("file", fileName, "file.txt", "");
nc.addQueryParam("fileDesc", "cool file");

nc.doUploadMultipartData();
if (nc.responseCode() == 200)  {
    std::cout << nc.responseBody();
}
```

Do a PUT request:
```cpp
nc.setMethod("PUT");
nc.setUrl("https://www.googleapis.com/drive/v2/files/" + id);
nc.addQueryHeader("Authorization", "Basic ...");
nc.addQueryHeader("Content-Type", "application/json");
std::string postData = "{\"title\" = \"SmellyCat.jpg\"}";
nc.doUpload("", postData);
```

Uploading a file to FTP:
```cpp
std::string fileName = "c:\\test\\file.txt";  // file path should be UTF-8 encoded on Windows
nc.setUrl("ftp://example.com");
nc.setMethod("PUT");
nc.doUpload(fileName, "");
```

Using proxy:
```cpp
nc.setProxy("127.0.0.1", "8888", CURLPROXY_HTTP); // CURLPROXY_HTTP, CURLPROXY_HTTPS,CURLPROXY_SOCKS4, CURLPROXY_SOCKS4A, CURLPROXY_SOCKS5, CURLPROXY_SOCKS5_HOSTNAME
```
Custom request header:
```
nc.addQueryHeader("Content-Type", "application/json");
```
## Attention

**On Windows, enable Unicode support when building libcurl** — otherwise, file uploads may fail.

If using OpenSSL (not WinSSL), ensure the `curl-ca-bundle.crt` file is in your app’s binary directory. https://curl.se/docs/caextract.html

