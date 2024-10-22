#include "request_handler.h"
#include "json_loader.h"

#include <optional>

namespace http_handler {

    using namespace std::literals;

    // Создаёт StringResponse с заданными параметрами
    StringResponse MakeStringResponse(http::status status,
                                    std::string_view body,
                                    unsigned http_version,
                                    bool keep_alive,
                                    std::string_view content_type,
                                    size_t length = 0) {
        StringResponse response(status, http_version);
        response.set(http::field::content_type, content_type);
        if (status == http::status::method_not_allowed) {
            response.set("Allow"sv, "GET"sv);
        }
        response.body() = body;
        if (length == 0) {
            length = response.body().size();
        }
        response.content_length(length);
        response.keep_alive(keep_alive);
        return response;
    }

    // Обрабатываем запрос
    std::variant<StringResponse, FileResponse> RequestHandler::HandleRequest(StringRequest &&req) {
        const std::string api_base_str = "/api/";

        if (req.method() == http::verb::get) {
            std::string req_str{DecodeURI(req.target())};

            if (req_str.starts_with(api_base_str)) {
                // запрашивается REST API
                return ReturnAPIResponse(req_str, req.version(), req.keep_alive());
            } else {
                // Запрашивается файл
                std::filesystem::path path{req_str};
                path = path.lexically_normal();

                return ReturnFile(path, req.version(), req.keep_alive());
            }
        } else {
            return MakeStringResponse(http::status::method_not_allowed, "Invalid method"sv,
                            req.version(), req.keep_alive(), ContentType::JSON);
        }
    }

    StringResponse RequestHandler::ReturnAPIResponse(std::string_view req_str, unsigned int version, bool keep_alive) {
        const std::string command1_str = "/api/v1/maps";
        const std::string command2_str = "/api/v1/maps/";
        const size_t command2_symbols = command2_str.length();

        const auto text_response = [version, keep_alive](http::status status, std::string_view text, size_t length = 0) {
            return MakeStringResponse(status, text, version, keep_alive, ContentType::JSON, length);
        };

        if (req_str.starts_with(command2_str)) {
            // запрашивается карта с заданным id
            std::string_view map_id = req_str.substr(command2_symbols);
            std::optional<std::string> map_result = json_loader::GetMap(model::Map::Id{std::string(map_id)}, game_);
            if (map_result.has_value()) {
                return text_response(http::status::ok, map_result.value());
            } else {
                // Запрашиваемый id карты не найден
                return text_response(http::status::not_found, json_loader::GetMapNotFoundString());
            }
        } else if (req_str.compare(command1_str) == 0) {
            // запрашивается список карт
            return text_response(http::status::ok, json_loader::GetListOfMaps(game_));
        } else {
            // Неправильный запрос
            return text_response(http::status::bad_request, json_loader::GetBadRequestString());
        }
    }

    std::variant<StringResponse, FileResponse> RequestHandler::ReturnFile(std::filesystem::path file_path,
                                                                        unsigned int version, bool keep_alive) {
        const auto error_response = [version, keep_alive](http::status status, std::string_view text) {
            return MakeStringResponse(status, text, version, keep_alive, ContentType::PLAIN);
        };
        http::response<http::file_body> response(http::status::ok, version);
        size_t file_size = 0;

        if (file_path.string().back() == '/') {
            file_path.append("index.html");
        }
        file_path = root_path_ / file_path.relative_path();

        switch (isSafePath(root_path_, file_path)) {
        case 0: {
            http::file_body::value_type file;
            boost::system::error_code ec;
            file.open(file_path.string().c_str(), beast::file_mode::read, ec);
            if (ec) {
                return error_response(http::status::not_found, "There is no access to the specified file"sv);
            }
            response.body() = std::move(file);
            break;
        }
        case 1: // ошибка - искомый файл не найден
            return error_response(http::status::not_found, "The specified file was not found"sv);
        case 2: // ошибка - выход за пределы разрешённой директории
            return error_response(http::status::bad_request, "Invalid request"sv);
        }
        // возвращаем файл
        response.prepare_payload(); // заполняет заголовки Content-Length и Transfer-Encoding
        response.set(http::field::content_type, GetContentType(file_path.extension().string()));
        response.keep_alive(keep_alive);
        return response;
    }

    // Возвращает Content-Type по расширению файла
    std::string_view GetContentType(std::string file_extension) {
        if (!file_extension.empty()) {
            file_extension = file_extension.substr(1);
        }
        FileExt f_ext;
        if (f_ext.extensions.count(file_extension) == 1) {
            return f_ext.extensions.at(file_extension);
        }
        return ContentType::BINARY;
    }

    /* Декодирует URI строку, при запуске необходимо отлавливать исключение (std::stoul):
     * - std::invalid_argument - невозможно сконвертировать  */
    std::string DecodeURI(const std::string_view uri_string) {
        std::string result = "";
        for (int i = 0; i < uri_string.length(); ++i) {
            if (uri_string[i] == '%') {
                unsigned long x = std::stoul(std::string("0x").append(uri_string.substr(i + 1, 2)), nullptr, 16);
                result += static_cast<char>(x);
                i += 2;
            } else {
                result += uri_string[i];
            }
        }
        return result;
    }

    /* Проверка пути запрашиваемого файла, возвращаемые значения:
     * - 0 - всё хорошо, файл найден,
     * - 1 - ошибка - искомый файл не найден,
     * - 2 - ошибка - выход за пределы разрешённой директории */
    int isSafePath(const std::filesystem::path& norm_root, const std::filesystem::path& check) {
        std::error_code ec;
        auto const norm_check = std::filesystem::canonical(check, ec);
        if (ec) {
            return 1;
        }

        auto it = std::search(norm_check.begin(), norm_check.end(),
                               norm_root.begin(), norm_root.end());

        if (it == norm_check.begin()) {
            return 0;
        }
        return 2;
    }

}  // namespace http_handler
