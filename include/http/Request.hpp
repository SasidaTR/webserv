#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <map>
#include <cstddef>

class Request {
private:
    std::string method;
    std::string target;
    std::string version;
    std::map<std::string, std::string> headers;
    std::string body;

    static std::string toLower(const std::string& s);
    std::string dechunkBody(const std::string& chunked);

public:
    bool parse(const std::string& raw);

    std::string getMethod() const { return method; }
    std::string getTarget() const { return target; }
    std::string getVersion() const { return version; }
    std::string getHeader(const std::string& key) const;
    const std::map<std::string, std::string>& getHeaders() const { return headers; }
    std::string getBody() const { return body; }
    bool isValidMethod() const;
    void debugPrint() const;

    bool hasHeader(const std::string& key) const;                        // case-insensitive key
    bool headerEquals(const std::string& key, const std::string& val) const; // case-insensitive key+value
    size_t contentLength() const;                                        // 0 if missing/invalid
    bool isChunked() const;                                              // Transfer-Encoding: chunked
    bool hasExpect100() const;                                           // Expect: 100-continue
};

#endif
