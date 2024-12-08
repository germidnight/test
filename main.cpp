#include "sdk.h"

#include <boost/asio/signal_set.hpp>
#include <boost/asio/io_context.hpp>
#include <iostream>
#include <fstream>
#include <thread>

#include "command_line.h"
#include "http_server.h"
#include "json_loader.h"
#include "logging_handler.h"
#include "players.h"
#include "ticker.h"

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
    // отправляет логи в cerr
    boost::log::add_console_log(
        std::clog,
        keywords::format = &(logging_handler::LogFormatter),
        keywords::auto_flush = true);
    try {
        // 1. Разбираем опции запуска
        auto args = start_options::ParseCommandLine(argc, argv);
        if (!args) {
            return EXIT_SUCCESS;
        }

        // 2. Загружаем карту из файла и строим модель игры
        std::optional<std::string_view> autosave_file = std::nullopt;
        if ((args->autosave_period > 0) && !args->state_file.empty()) {
            autosave_file = args->state_file;
        }
        model::Game game = json_loader::LoadGame(args->config_file);
        players::Application app(game,
                                args->randomize_spawn_points,
                                args->test_mode,
                                args->tick_period,
                                autosave_file);

        // 3. Инициализируем io_context
        const unsigned num_threads = std::thread::hardware_concurrency();
        net::io_context ioc(num_threads);

        // 4. Добавляем асинхронный обработчик сигналов SIGINT и SIGTERM
        net::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&ioc](const boost::system::error_code& ec, [[maybe_unused]] int signal_number) {
            if (!ec) {
                ioc.stop();
            }
        });

        // 5. Открываем каталог со статическими файлами игры
        std::error_code ec;
        std::filesystem::path game_root_dir = std::filesystem::canonical(
                            (std::filesystem::current_path() / std::string{args->www_root}),
                            ec);
        if (ec) {
            throw std::runtime_error("Error opening given root directory: "
                                    + std::string(argv[1]) + " - " + ec.message());
        }

        // 6. Загружаем состояние игры (при необходимости и возможности)
        if (!args->state_file.empty()) {
            std::ifstream autosave_file(args->state_file, std::ios::in);
            if (autosave_file.is_open()) {
                std::stringstream stream;
                std::string file_str{std::istreambuf_iterator<char>(autosave_file), std::istreambuf_iterator<char>()};
                autosave_file.close();
                stream << file_str;
                players::DeserializeState(stream, app);
            }
        }

        // 7. strand для выполнения запросов к API
        auto api_strand = net::make_strand(ioc);

        // 8. Создаём и запускаем обработчик передвижений игровых персонажей по карте
        if (!args->test_mode) {
            std::shared_ptr<ticker::Ticker> time_sheduler = std::make_shared<ticker::Ticker>(api_strand,
                                std::chrono::duration_cast<std::chrono::milliseconds>(
                                    std::chrono::duration<double, std::milli>{app.GetTickPeriod() * 1s}),
                                [&app](std::chrono::milliseconds delta) {
                                        double tick_period = std::chrono::duration_cast<
                                                std::chrono::duration<double, std::milli>>(delta) / 1s;
                                        app.MoveDogs(tick_period);
                                });
            time_sheduler->Start();

            // 8.1 Создаём и запускаем обработчик сохранения состояния
            if (args->autosave_period > 0) {
                std::shared_ptr<ticker::Ticker> autosave_sheduller = std::make_shared<ticker::Ticker>(api_strand,
                                std::chrono::duration_cast<std::chrono::milliseconds>(
                                    std::chrono::duration<unsigned, std::milli>{args->autosave_period}),
                                    [&app, &args]([[maybe_unused]]std::chrono::milliseconds delta) {
                                        players::AutosaveState(app, args->state_file);
                                    });
                autosave_sheduller->Start();
            }
        }

        // 8.2. Создаём обработчик HTTP-запросов в куче, управляемый shared_ptr и связываем его с моделью игры
        auto handler = std::make_shared<http_handler::RequestHandler>(app, game_root_dir, api_strand);
        // 8.3. Создаём объект декоратора для логирования
        logging_handler::LoggingRequestHandler logger_handler(std::move(handler));

        // 9. Запустить обработчик HTTP-запросов, делегируя их обработчику запросов
        const auto address = net::ip::make_address("0.0.0.0");
        constexpr net::ip::port_type port = 8080;
        http_server::ServeHttp(ioc, {address, port}, logger_handler);

        // 10. Запускаем обработку асинхронных операций
        RunWorkers(std::max(1u, num_threads), [&ioc] {
            ioc.run();
        });

        // 11. Сохраняем состояние перед окончанием работы (при необходимости)
        if (!args->state_file.empty()) {
            players::AutosaveState(app, args->state_file);
        }

        logging_handler::LogStopServer(EXIT_SUCCESS, "");
    } catch (const std::domain_error& ex) {
        logging_handler::LogStopServer(EXIT_FAILURE, ex.what());
        return EXIT_FAILURE;
    } catch (const std::exception& ex) {
        logging_handler::LogStopServer(EXIT_FAILURE, ex.what());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
