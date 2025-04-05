#include <string>

class HttpClient {
    public:
        HttpClient();
        ~HttpClient();
    
        bool get(const std::string& url, std::string& outBody);
    
    private:
        bool getPlain(const std::string& host, uint16_t port, const std::string& path, std::string& outBody);
        bool getTls(const std::string& host, uint16_t port, const std::string& path, std::string& outBody);
        std::string extractBody(const std::string& response);
    };
    