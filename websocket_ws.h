
#ifndef WEBSOCKET_WS_H
#define WEBSOCKET_WS_H

#include <string>
#include <vector>
#include <map>
#include <iosfwd>
#include "basefunc.h"
#include <openssl/sha.h>  //for SHA1
#include <arpa/inet.h>    //for ntohl
#include <string.h>


enum WsFrameType {
    Error_Frame = 0xFF00,
    InComplete_Frame = 0xFE00,

    Opening_Frame = 0x3300,
    Closeing_Frame = 0x3400,

    Incomplete_Text_Frame = 0x01,
    Incomplete_Binary_Frame = 0x02,

    Text_Frame = 0x81,
    Binary_Frame = 0x82,

    Ping_Frame = 0x19,
    Pong_Frame = 0x1A,
};

enum WSStatus {
    WS_STATUS_UNCONNECTED = 1,
    WS_STATUS_CONNECTED = 2,
};

class WebSocket 
{
public:
    WebSocket();
    //解析 WebSocket 的握手数据
    bool parseHandshake(const std::string& request);

    //应答 WebSocket 的握手
    std::string respondHandshake();

    //解析 WebSocket 的协议具体数据，客户端-->服务器
    int getWSFrameData(char* msg, int msgLen, std::vector<char>& outBuf, int* outLen);

    //封装 WebSocket 协议的数据，服务器-->客户端
    int makeWSFrameData(char* msg, int msgLen, std::vector<char>& outBuf);

    //封装 WebSocket 协议的数据头（二进制数据）
    static int makeWSFrameDataHeader(int len, std::vector<char>& header);

private:
    std::string websocketKey_;//握手中客户端发来的key
};

WebSocket::WebSocket()
{
}

bool WebSocket::parseHandshake(const std::string& request)
{
    // 解析WEBSOCKET请求头信息
    bool ret = false;
    std::istringstream stream(request.c_str());
    std::string reqType;
    std::getline(stream, reqType);
    if (reqType.substr(0, 4) != "GET ")
        return ret;

    std::string header;
    std::string::size_type pos = 0;
    while (std::getline(stream, header) && header != "\r")
    {
        header.erase(header.end() - 1);
        pos = header.find(": ", 0);
        if (pos != std::string::npos)
        {
            std::string key = header.substr(0, pos);
            std::string value = header.substr(pos + 2);
            if (key == "Sec-WebSocket-Key")
            {
                ret = true;
                websocketKey_ = value;
                break;
            }
        }
    }

    return ret;
}


std::string WebSocket::respondHandshake()
{
    // 算出WEBSOCKET响应信息
    std::string response = "HTTP/1.1 101 Switching Protocols\r\n";
    response += "Upgrade: websocket\r\n";
    response += "Connection: upgrade\r\n";
    response += "Sec-WebSocket-Accept: ";

    //使用请求传过来的KEY+协议字符串，先用SHA1加密然后使用base64编码算出一个应答的KEY
    const std::string magicKey("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
    std::string serverKey = websocketKey_ + magicKey;
    //LOG_INFO << "serverKey:" << serverKey;

    //SHA1
    unsigned char digest[SHA_DIGEST_LENGTH];
    SHA1((unsigned char*)serverKey.c_str(), serverKey.length(), (unsigned char*)&digest);
    //printf("DIGEST:"); for(int i=0; i<20; i++) printf("%02x ",digest[i]); printf("\n");

    //Base64
    char basestr[1024] = {0};
    base64_encode((char*)digest, SHA_DIGEST_LENGTH, basestr);

    //完整的握手应答
    response = response + std::string(basestr) + "\r\n";
    //LOG_INFO << "RESPONSE:" << response;

    return response;
}


int WebSocket::getWSFrameData(char* msg, int msgLen, std::vector<char>& outBuf, int* outLen)
{
    if(msgLen < 2)
        return INCOMPLETE_FRAME;

    uint8_t fin_ = 0;
    uint8_t opcode_ = 0;
    uint8_t mask_ = 0;
    uint8_t masking_key_[4] = {0,0,0,0};
    uint64_t payload_length_ = 0;
    int pos = 0;
    //FIN
    fin_ = (unsigned char)msg[pos] >> 7;
    //Opcode
    opcode_ = msg[pos] & 0x0f;
    pos++;
    //MASK
    mask_ = (unsigned char)msg[pos] >> 7;
    //Payload length
    payload_length_ = msg[pos] & 0x7f;
    pos++;
    if(payload_length_ == 126)
    {
        uint16_t length = 0;
        memcpy(&length, msg + pos, 2);
        pos += 2;
        payload_length_ = ntohs(length);
    }
    else if(payload_length_ == 127)
    {
        uint32_t length = 0;
        memcpy(&length, msg + pos, 4);
        pos += 4;
        payload_length_ = ntohl(length);
    }
    //Masking-key
    if(mask_ == 1)
    {
        for(int i = 0; i < 4; i++)
            masking_key_[i] = msg[pos + i];
        pos += 4;
    }
    //取出消息数据
    if (msgLen >= pos + payload_length_ )
    {
        //Payload data
        *outLen = pos + payload_length_;
        outBuf.clear();
        if(mask_ != 1)
        {
            char* dataBegin = msg + pos;
            outBuf.insert(outBuf.begin(), dataBegin, dataBegin+payload_length_);
        }
        else
        {
            for(uint i = 0; i < payload_length_; i++)
            {
                int j = i % 4;
                outBuf.push_back(msg[pos + i] ^ masking_key_[j]);
            }
        }
    }
    else
    {
        return INCOMPLETE_FRAME;
    }

//    printf("WEBSOCKET PROTOCOL\n"
//            "FIN: %d\n"
//            "OPCODE: %d\n"
//            "MASK: %d\n"
//            "PAYLOADLEN: %d\n"
//            "outLen:%d\n",
//            fin_, opcode_, mask_, payload_length_, *outLen);

    //断开连接类型数据包
    if ((int)opcode_ == 0x8)
        return -1;

    return 0;
}


int WebSocket::makeWSFrameData(char* msg, int msgLen, std::vector<char>& outBuf)
{
    std::vector<char> header;
    makeWSFrameDataHeader(msgLen, header);
    outBuf.insert(outBuf.begin(), header.begin(), header.end());
    outBuf.insert(outBuf.end(), msg, msg+msgLen);
    return 0;
}

int WebSocket::makeWSFrameDataHeader(int len, std::vector<char>& header)
{
    header.push_back((char)BINARY_FRAME);
    if(len <= 125)
    {
        header.push_back((char)len);
    }
    else if(len <= 65535)
    {
        header.push_back((char)126);//16 bit length follows
        header.push_back((char)((len >> 8) & 0xFF));// leftmost first
        header.push_back((char)(len & 0xFF));
    }
    else // >2^16-1 (65535)
    {
        header.push_back((char)127);//64 bit length follows

        // write 8 bytes length (significant first)
        // since msg_length is int it can be no longer than 4 bytes = 2^32-1
        // padd zeroes for the first 4 bytes
        for(int i=3; i>=0; i--)
        {
            header.push_back((char)0);
        }
        // write the actual 32bit msg_length in the next 4 bytes
        for(int i=3; i>=0; i--)
        {
            header.push_back((char)((len >> 8*i) & 0xFF));
        }
    }

    return 0;
}

#endif // WEBSOCKET_WS_H