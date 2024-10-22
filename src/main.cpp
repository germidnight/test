#include "sdk.h"
//
#include <boost/asio/signal_set.hpp>
#include <boost/asio/io_context.hpp>
#include <iostream>
#include <thread>

#include "json_loader.h"
#include "logging_handler.h"

using namespace std::literals;
namespace net = boost::asio;
namespace keywords = boost::log::keywords;

namespace {

// Запускает функцию fn на n потоках, включая текущий
template <typename Fn>
void RunWorkers(unsigned n, const Fn& fn) {
    n = std::max(1u, n);
    std::vector<std::jthread> workers;
    workers.reserve(n - 1);
    // Запускаем n-1 рабочих потоков, выполняющих функцию fn
    while (--n) {
        workers.emplace_back(fn);
    }
    fn();
}

}  // namespace

int main(int argc, const char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: game_server <game-config-json> <root_directory_for_game_data>"sv << std::endl;
        return EXIT_FAILURE;
    }
    // отправляет логи в cerr
    boost::log::add_console_log(
        std::clog,
        keywords::format = &(logging_handler::LogFormatter),
        keywords::auto_flush = true);
    try {
        // 1. Загружаем карту из файла и построить модель игры
        model::Game game = json_loader::LoadGame(argv[1]);

        // 2. Инициализируем io_context
        const unsigned num_threads = std::thread::hardware_concurrency();
        net::io_context ioc(num_threads);

        // 3. Добавляем асинхронный обработчик сигналов SIGINT и SIGTERM
        net::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&ioc](const boost::system::error_code& ec, [[maybe_unused]] int signal_number) {
            if (!ec) {
                ioc.stop();
            }
        });

        std::error_code ec;
        std::filesystem::path game_root_dir = std::filesystem::canonical(
                            (std::filesystem::current_path() / std::string{argv[2]}),
                            ec);
        if (ec) {
            throw std::runtime_error("Error opening given root directory: "
                                    + std::string(argv[1]) + " - " + ec.message());
        }
        // 4.1. Создаём обработчик HTTP-запросов и связываем его с моделью игры
        http_handler::RequestHandler handler{game, game_root_dir};
        // 4.2. Создаём объект декоратора для логирования
        logging_handler::LoggingRequestHandler logger_handler{handler};

        // 5. Запустить обработчик HTTP-запросов, делегируя их обработчику запросов
        const auto address = net::ip::make_address("0.0.0.0");
        constexpr net::ip::port_type port = 8080;
        http_server::ServeHttp(ioc, {address, port}, [&logger_handler](auto&& req, auto&& send,
                    const auto& client_endpoint) {
            logger_handler(std::forward<decltype(req)>(req), std::forward<decltype(send)>(send), client_endpoint);
        });

        // 6. Запускаем обработку асинхронных операций
        RunWorkers(std::max(1u, num_threads), [&ioc] {
            ioc.run();
        });

        logging_handler::LogStopServer(EXIT_SUCCESS, "");
    } catch (const std::exception& ex) {
        //std::cerr << ex.what() << std::endl;
        logging_handler::LogStopServer(EXIT_FAILURE, ex.what());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
