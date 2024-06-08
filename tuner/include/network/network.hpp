#pragma onace

#include <string>
#include "HappyHTTP/happyhttp.h"
#include <functional>

class netowrk { 
private:
    happyhttp::Connection conn;
    volatile bool success = false;
    std::function<void(const happyhttp::Response* r, void* userdata)> begin;
    std::function<void(const happyhttp::Response* r, void* userdata, const unsigned char* data, int numbytes)> recv;
    std::function<void(const happyhttp::Response* r, void* userdata)> end;

    std::string result;
    void OnBegin(const happyhttp::Response* r)
    {
        // printf( "BEGIN (%d %s)\n", r->getstatus(), r->getreason() );
        result = "";
        success = false;
    }
    void OnData(const happyhttp::Response* r, const unsigned char* data, int n )
    {
        // fwrite( data,1,n, stdout );
        // 얌전히 N 값이 들어오면 N 만 복사하셈;;
        // Release 에서 뭔 오류가 날지 모름
        for (int i = 0; i < n; i++) { 
            result += (char)(data[i]);
        }
    }

    void OnComplete(const happyhttp::Response* r)
    {
        // printf( "COMPLETE (%d bytes [ %s ])\n", count, result.c_str());
        success = true;
    }

public:
    netowrk() = delete;

    netowrk(std::string ip, int port) : conn(ip.c_str(), port) { 
        begin = [&](const happyhttp::Response* r, void* userdata) {
            this->OnBegin(r);
        };
        recv = [&](const happyhttp::Response* r, void* userdata, const unsigned char* data, int numbytes) {
            this->OnData(r, data, numbytes);
        };
        end = [&](const happyhttp::Response* r, void* userdata) {
            this->OnComplete(r);
        };
        
	    conn.setcallbacks(begin, recv, end, nullptr);
    }

    void request(std::string path, std::string body) {
        success = false;

        conn.request("POST", path.c_str(), nullptr, (unsigned char*)body.c_str(), body.size());
        while(conn.outstanding()) {
		    conn.pump();
        }
    }  

    std::string recv_body() const { 
        return result;
    }

    bool wait() const {
        while (!success){
        }
        return true;
    }

};