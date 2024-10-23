#pragma once
#include "model.h"

#define BOOST_BEAST_USE_STD_STRING_VIEW

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

#include <algorithm>
#include <filesystem>
#include <iterator>
#include <map>
#include <optional>
#include <string_view>
#include <variant>

namespace http_handler {

namespace beast = boost::beast;
namespace http = beast::http;

// Запрос, тело которого представлено в виде строки
using StringRequest = http::request<http::string_body>;
// Ответ, тело которого представлено в виде строки
using StringResponse = http::response<http::string_body>;
// Ответ, тело которого представлено в виде файла
using FileResponse = http::response<http::file_body>;

using namespace std::literals;

struct ContentType {
    ContentType() = delete;
    constexpr static std::string_view BINARY    = "application/octet-stream"sv;
    constexpr static std::string_view JSON      = "application/json"sv;
    constexpr static std::string_view XML       = "application/xml"sv;
    constexpr static std::string_view MP3       = "audio/mpeg"sv;
    constexpr static std::string_view BMP       = "image/bmp"sv;
    constexpr static std::string_view GIF       = "image/gif"sv;
    constexpr static std::string_view JPG       = "image/jpeg"sv;
    constexpr static std::string_view SVG       = "image/svg+xml"sv;
    constexpr static std::string_view PNG       = "image/png"sv;
    constexpr static std::string_view TIFF      = "image/tiff"sv;
    constexpr static std::string_view ICO       = "image/vnd.microsoft.icon"sv;
    constexpr static std::string_view CSS       = "text/css"sv;
    constexpr static std::string_view HTML      = "text/html"sv;
    constexpr static std::string_view JS        = "text/javascript"sv;
    constexpr static std::string_view PLAIN     = "text/plain"sv;
};

struct FileExt {
    const std::map<std::string, std::string_view> extensions = {
        {"htm", ContentType::HTML}, {"html", ContentType::HTML},
        {"css", ContentType::CSS}, {"txt", ContentType::PLAIN},
        {"js", ContentType::JS}, {"json", ContentType::JSON},
        {"xml", ContentType::XML}, {"png", ContentType::PNG},
        {"jpg", ContentType::JPG}, {"jpe", ContentType::JPG}, {"jpeg", ContentType::JPG},
        {"gif", ContentType::GIF}, {"bmp", ContentType::BMP}, {"ico", ContentType::ICO},
        {"tif", ContentType::TIFF}, {"tiff", ContentType::TIFF},
        {"svg", ContentType::SVG}, {"svgz", ContentType::SVG},
        {"mp3", ContentType::MP3}};
};

class RequestHandler {
public:
    explicit RequestHandler(model::Game& game, std::filesystem::path root_dir)
        : game_{game}, root_path_(root_dir) {}

    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;

    /* Эта версия обработчика нужна при упразднении декоратора логирования
    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        // Обработать запрос request и отправить ответ, используя send
        auto answer = HandleRequest(std::forward<StringRequest>(req));
        if (std::holds_alternative<StringResponse>(answer)) {
            send(std::get<StringResponse>(answer));
        } else if (std::holds_alternative<FileResponse>(answer)) {
            send(std::get<FileResponse>(answer));
        }
    }*/
    // При использовании декоратора часть функционала ушла в logger
    template <typename Body, typename Allocator>
    std::variant<StringResponse, FileResponse> operator()(http::request<Body, http::basic_fields<Allocator>> &&req) {
        return HandleRequest(std::forward<StringRequest>(req));
    }

private:
    std::variant<StringResponse, FileResponse> HandleRequest(StringRequest &&req);

    StringResponse ReturnAPIResponse(std::string_view req_str, unsigned int version, bool keep_alive);
    std::variant<StringResponse, FileResponse> ReturnFile(std::filesystem::path file_path,
                                                        unsigned int version, bool keep_alive);

    model::Game& game_;
    std::filesystem::path root_path_;
};

StringResponse MakeStringResponse(http::status status,
                                  std::string_view body,
                                  unsigned http_version,
                                  bool keep_alive,
                                  std::string_view content_type,
                                  size_t length);

std::string_view GetContentType(std::string file_extension);
std::string DecodeURI(const std::string_view uri_string);
int isSafePath(const std::filesystem::path& norm_root, const std::filesystem::path& child);

}  // namespace http_handler
