# curl-cpp-wrapper

A very simple HTTP client (C++ wrapper for curl). Requires libcurl http://curl.haxx.se

If you are using Visual C++ your also have to install msinttypes (headers are provided in "include" directory).

Usage

Do a GET request:
```cpp
NetworkClient nc;
nc.doGet("http://google.com/?q=" + nc.urlEncode("Smelly cat"));
std::cout << nc.responseBody();
```

Do a POST request:
```cpp
nc.setUrl("https://www.googleapis.com/oauth2/v3/token");
nc.addQueryParam("refresh_token", refreshToken); 
nc.addQueryParam("client_id", clientId); 
nc.addQueryParam("client_secret", clientSecret); 
nc.addQueryParam("grant_type", "refresh_token"); 
nc.doPost();
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
nc.setOutputFile("d:\\image.png"); //only UTF-8 file names are supported on Windows
nc.doGet("http://i.imgur.com/DDf2wbJ.png");
```

Uploading a file:
```cpp
#include "NetworkClient.h"
#include "Core/Utils/CoreUtils.h"
NetworkClient nc;
std::string fileName = "c:\\test\\file.txt"; //only UTF-8 file names are supported on Windows
//std::string fileName = IuCoreUtils::SystemLocaleToUtf8(fileName); <-- if you want to upload a file with ANSI filename on Windows
nc.setUrl("http://takebin.com/action");
nc.addQueryParamFile("file", fileName, IuCoreUtils::ExtractFileName(FileName),"");
nc.addQueryParam("fileDesc", "cool file");

nc.doUploadMultipartData();
if ( nc.responseCode() == 200 )  {
		std::cout << nc.responseBody();
}
```

Do a PUT request:
```cpp
nc.setMethod("PUT");
nc.setUrl("https://www.googleapis.com/drive/v2/files/" + id);
nc.addQueryHeader("Authorization", "Basic ");
nc.addQueryHeader("Content-Type", "application/json");
std::string postData = "{\"title\" = \"SmellyCat.jpg\"}";
nc.doUpload("", postData);
```

Uploading a file to FTP:
```cpp
std::string fileName = "c:\\test\\file.txt";
nc.setUrl("ftp://example.com");
nc.setMethod("PUT");
nc.doUpload(fileName, "");
```

Using proxy:
```cpp
nc.setProxy("127.0.0.1", "8888", CURLPROXY_HTTP); // CURLPROXY_HTTP, CURLPROXY_SOCKS4, CURLPROXY_SOCKS4A, CURLPROXY_SOCKS5, CURLPROXY_SOCKS5_HOSTNAME
```
Custom request header:
```
nc.addQueryHeader("User-Agent", "Mozilla/5.0");
```

If you want to use HTTPS, you should compile libcurl with openssl support (you could also use WinSSL on Windows). 
If you compile libcurl for windows with OpenSSL support (instead of WinSSL), you should put "curl-ca-bundle.crt" file into your application's directory).
