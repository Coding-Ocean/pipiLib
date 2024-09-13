#pragma once
#include <fstream>
#include <sstream>

class FILE_BUFFER {
    std::wistringstream stream;
    std::wstring token;
public:
    FILE_BUFFER(const wchar_t* filename) {
        std::wifstream file(filename);
        auto start = std::istreambuf_iterator<wchar_t>(file);
        auto end = std::istreambuf_iterator<wchar_t>();
        std::wstring content(start, end);
        stream.str(content);
    }
    std::wstring readString() { stream >> token; return token; }
    std::wstring readOnAssumption(const wchar_t* str) { stream >> token; assert(token == str); return token; }
    int readInt() { stream >> token; return std::stoi(token); }
    float readFloat() { stream >> token; return std::stof(token); }
    bool eof() { return stream.eof(); }
    void restart() { stream.clear(); stream.seekg(0); }
};

//class FILE_BUFFER{
//public:
//    FILE_BUFFER();
//    FILE_BUFFER(const char* fileName);
//    void operator= ( const char* fileName );
//    ~FILE_BUFFER();
//    void create(const char* fileName);
//    //バッファポインタを進める関数
//    const char* readString();
//    float readFloat();
//    int readInt();
//    unsigned readUnsigned();
//    void readOnAssumption( const char* s );
//    void skipNode();
//    void restart();
//    bool end();
//    //取得系
//    const char* buffer() const;
//    int size();
//    const char* bufferPointer() const;
//    const char* string() const;
//    operator const char*();
//    //オーバーロード
//    bool operator==( const char* str );
//    bool operator!=( const char* str );
//    bool operator==( char c );
//    bool operator!=( char c );
//private:
//    char* Buffer=0;//ファイルバッファ
//    int Size=0;//バッファサイズ
//    const char* BufferPointer=0;//ファイルバッファのポインタ
//    char String[ 256 ]="";//取り出した文字列
//};


