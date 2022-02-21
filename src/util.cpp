/*!
util.cpp

Copyright (c) 2022 Kosuke Nakai

This software is released under the MIT License.
http://opensource.org/licenses/mit-license.php
*/
#include"util.h"
#include<sstream>
#include <Shlwapi.h>
#include <openssl/aes.h>
#include<openssl/evp.h>

#pragma comment(lib, "Shlwapi.lib")

using namespace std;

//•¶Žš—ñ‚Ìsplit‹@”\
std::vector<std::string> split(std::string str, char del){
    int first = 0;
    int last = str.find_first_of(del);
    std::vector<std::string> result;
    while(first < str.size()){
        std::string subStr(str, first, last - first);
        result.push_back(subStr);
        first = last + 1;
        last = str.find_first_of(del, first);
        if(last == std::string::npos){
            last = str.size();
        }
    }
    return result;
}


std::vector<std::vector<std::string> >
csv2vector(std::string csv, int ignore_line_num,char del){
    std::stringstream ss;
    std::string reading_line_buffer;
    csv = replace(csv, "  ", " ");
    ss << csv;

    //Å‰‚Ìignore_line_nums‚ð‹ó“Ç‚Ý‚·‚é
    for(int line = 0; line < ignore_line_num; line++){
        getline(ss, reading_line_buffer);
        if(ss.eof()) break;
    }
    //“ñŽŸŒ³‚Ìvector‚ðì¬
    std::vector<std::vector<std::string> > data;
    while(std::getline(ss, reading_line_buffer)){
        if(reading_line_buffer.size() == 0) break;
        std::vector<std::string> temp_data;
        temp_data = split(reading_line_buffer, del);
        data.push_back(temp_data);
    }
    return data;
}


unsigned char* Encrypt(const char* key, const char* data, const size_t datalen, const unsigned char* iv, unsigned char* dest, const size_t destlen){
    EVP_CIPHER_CTX* en = EVP_CIPHER_CTX_new();
    int f_len = 0;
    int c_len = destlen;


    memset(dest, 0x00, destlen);


    EVP_CIPHER_CTX_init(en);
    EVP_EncryptInit_ex(en, EVP_aes_128_cbc(), NULL, (unsigned char*)key, iv);


    EVP_EncryptUpdate(en, dest, &c_len, (unsigned char*)data, datalen);
    //EVP_EncryptFinal_ex(en, (unsigned char *)(dest + c_len), &f_len);

    EVP_CIPHER_CTX_cleanup(en);


    return dest;
}

unsigned char* Decrypt(const char* key, const unsigned char* data, const size_t datalen, const unsigned char* iv,unsigned char* dest, const size_t destlen){
    EVP_CIPHER_CTX* de = EVP_CIPHER_CTX_new();
    int f_len = 0;
    int p_len = datalen;


    memset(dest, 0x00, destlen);


    EVP_CIPHER_CTX_init(de);
    EVP_DecryptInit_ex(de, EVP_aes_128_cbc(), NULL, (unsigned char*)key, iv);


    EVP_DecryptUpdate(de, (unsigned char*)dest, &p_len, data, datalen);
    //EVP_DecryptFinal_ex(de, (unsigned char *)(dest + p_len), &f_len);


    EVP_CIPHER_CTX_cleanup(de);


    return dest;
}

union CRYPT_UNION{
    struct{
        char hostname[512];
        char user[256];
        char password[256];
    };
    unsigned char data[1024];
};

string pack_space(string src){
    string a, b = src;
    while(1){
        a = replace(b, "  ", " ");
        if(a == b) break;
        b = a;
    }
    return b;
}

int ReadSetting(char* filename, CONNECT_INFO& ci){
    unsigned char crypt[1024] = {0};
    CRYPT_UNION cu;
    FILE* fp;

    if(PathFileExistsA(filename)){
        if(!fopen_s(&fp, filename, "rb")){
            fread(crypt, 1024, 1, fp);
            Decrypt("remote_performace_monitor_key", crypt, sizeof(crypt), (const unsigned char*)"initial_vector", cu.data, sizeof(cu.data));
            ci.hostname = cu.hostname;
            ci.user = cu.user;
            ci.pass = cu.password;
            fclose(fp);
        }
    } else{
        ci.hostname = "localhost";
        ci.user = "user";
        ci.pass = "password";
        WriteSetting(filename, ci);
    }
    
    return 0;
}

int WriteSetting(char* filename, CONNECT_INFO& ci){
    CRYPT_UNION cu;
    unsigned char crypt[1024];

    memset(cu.data, 0, sizeof(cu));
    strcpy_s(cu.hostname, ci.hostname.c_str());
    strcpy_s(cu.user, ci.user.c_str());
    strcpy_s(cu.password, ci.pass.c_str());
    Encrypt("remote_performace_monitor_key", (const char*)cu.data, sizeof(cu.data),(const unsigned char*)"initial_vector", crypt, sizeof(crypt));
    FILE* fp;
    if(!fopen_s(&fp, filename, "wb")){
        fwrite(crypt, sizeof(crypt), 1, fp);
        fclose(fp);
    }
    return 0;
}

int KillCmd(SSH& ssh, string pid){
    string cmd = "kill -9 ";
    char buffer[64] = {0};
    cmd += pid;
    if(ssh.connect_flag){
        ssh.ExecCmd((char*)cmd.c_str(), buffer);
        string delspace = replace(buffer, " ", "");
        return 0;
    }
    return -1;
}

