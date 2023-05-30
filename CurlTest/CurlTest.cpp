// CurlTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include "../NetworkClient.h"

int main()
{
    NetworkClient nc;
    nc.setOutputFile("D:\\response.html");
    //nc.doGet("https://google.com/" + nc.urlEncode("Smelly cat"));
    nc.doGet("https://google.com/");

    std::cout << nc.responseHeaderByName("Content-Length") << std::endl;
    std::cout << "Response code " << nc.responseCode() << std::endl;
    std::cout << nc.errorString() << std::endl;

    NetworkClient nc2;
    nc2.setProxy("127.0.0.1", 8888, CURLPROXY_HTTP);

    nc2.setCurlOption(CURLOPT_USERNAME, "user");
    nc2.setCurlOption(CURLOPT_PASSWORD, "");
    nc2.setMethod("PUT");
    nc2.setUrl("http://webdav.test/test2.jpg");
    nc2.doUpload("d:\\Develop\\animated-webp-supported.webp", "");

    std::cout << "Response code" << nc2.responseCode() << std::endl;
    std::cout << nc2.getCurlResultString() << std::endl;
}
