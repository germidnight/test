#pragma once
#include "sdk.h"
#include "logging_handler.h"

// boost.beast будет использовать std::string_view вместо boost::string_view
#define BOOST_BEAST_USE_STD_STRING_VIEW

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

#include <string_view>

namespace http_server {

namespace net = boost::asio;
using tcp = net::ip::tcp;
namespace beast = boost::beast;
namespace http = beast::http;
namespace sys = boost::system;
using namespace std::literals;

//void ReportError(beast::error_code ec, std::string_view what);

// ---------------------------------------- SessionBase ----------------------------------------
class SessionBase {
public:
    // Запрещаем копирование и присваивание объектов SessionBase и его наследников
    SessionBase(const SessionBase &) = delete;
    SessionBase &operator=(const SessionBase &) = delete;

    void Run();
protected:
    using HttpRequest = http::request<http::string_body>;

    explicit SessionBase(tcp::socket &&socket) : stream_(std::move(socket)) {}
    ~SessionBase() = default;

    template <typename Body, typename Fields>
    void Write(http::response<Body, Fields>&& response) {
        // Запись выполняется асинхронно, поэтому response перемещаем в область кучи
        auto safe_response = std::make_shared<http::response<Body, Fields>>(std::move(response));

        auto self = GetSharedThis();
        http::async_write(stream_, *safe_response,
                [safe_response, self](beast::error_code ec, std::size_t bytes_written){
                    self->OnWrite(safe_response->need_eof(), ec, bytes_written);
                });
    }

    // tcp_stream содержит внутри себя сокет и добавляет поддержку таймаутов
    beast::tcp_stream stream_;

private:
    // Асинхронное чтение запроса
    void Read();
    void OnRead(beast::error_code ec, [[maybe_unused]] std::size_t bytes_read);

    // Закрыть сокет
    void Close();

    void OnWrite(bool close, beast::error_code ec, [[maybe_unused]] std::size_t bytes_written);

    // Обработку запроса делегируем подклассу
    virtual void HandleRequest(HttpRequest && request) = 0;
    virtual std::shared_ptr<SessionBase> GetSharedThis() = 0;

    beast::flat_buffer buffer_;
    HttpRequest request_;
};

// ---------------------------------------- Session ----------------------------------------
/*
 * Внутри Session управление по очереди получают методы:
 * Read → OnRead → HandleRequest → Write → OnWrite, а затем снова Read.
 * Здесь нет рекурсии, так как методы OnRead и OnWrite вызываются асинхронно —
 * их вызывает не сама программа, а операционная система и Boost.Asio.
 */
template <typename RequestHandler>
class Session : public SessionBase, public std::enable_shared_from_this<Session<RequestHandler>> {
public:
    template <typename Handler>
    Session(tcp::socket &&socket, Handler &&request_handler)
        : SessionBase(std::move(socket))
        , request_handler_(std::forward<Handler>(request_handler)) {}

private:
    std::shared_ptr<SessionBase> GetSharedThis() override {
        return this->shared_from_this();
    }

    /* Вызывается в Session::OnRead() и вызывает обработчик запроса из декоратора logging_handler::LoggingRequestHandler() */
    void HandleRequest(HttpRequest &&request) override {
        /* Захватываем умный указатель на текущий объект Session в лямбде,
         * чтобы продлить время жизни сессии до вызова лямбды.
         * Используется generic-лямбда функция, способная принять response произвольного типа */
        request_handler_(std::move(request), [self = this->shared_from_this()](auto &&response) {
            self->Write(std::move(response));
        }, stream_.socket().remote_endpoint());
    }

    RequestHandler request_handler_;
};

// ---------------------------------------- Listener ----------------------------------------
template <typename RequestHandler>
class Listener : public std::enable_shared_from_this<Listener<RequestHandler>> {
public:
    template <typename Handler>
    Listener(net::io_context &ioc, const tcp::endpoint &endpoint, Handler &&request_handler)
                : ioc_(ioc)
                // Обработчики асинхронных операций acceptor_ будут вызываться в своём strand
                , acceptor_(net::make_strand(ioc))
                , request_handler_(std::forward<Handler>(request_handler)) {
        // Открываем acceptor, используя протокол (IPv4 или IPv6), указанный в endpoint
        acceptor_.open(endpoint.protocol());
        /* После закрытия TCP-соединения сокет некоторое время может считаться занятым,
         * чтобы компьютеры могли обменяться завершающими пакетами данных.
         * Однако это может помешать повторно открыть сокет в полузакрытом состоянии.
         * Флаг reuse_address разрешает открыть сокет, когда он "наполовину закрыт" */
        acceptor_.set_option(net::socket_base::reuse_address(true));
        // Привязываем acceptor к адресу и порту endpoint
        acceptor_.bind(endpoint);
        /* Переводим acceptor в состояние, в котором он способен принимать новые соединения
         * Благодаря этому новые подключения будут помещаться в очередь ожидающих соединений */
        acceptor_.listen(net::socket_base::max_listen_connections);

        // Сервер запущен
        logging_handler::LogStartServer(endpoint);
    }

    void Run() {
        DoAccept();
    }

private:
    void AsyncRunSession(tcp::socket &&socket) {
        std::make_shared<Session<RequestHandler>>(std::move(socket), request_handler_)->Run();
    }

    void DoAccept() {
        acceptor_.async_accept(
            /* Передаём последовательный исполнитель, в котором будут вызываться обработчики
             * асинхронных операций сокета */
            net::make_strand(ioc_),
            /* С помощью bind_front_handler создаём обработчик, привязанный к методу OnAccept
             * текущего объекта.
             * Так как Listener — шаблонный класс, нужно подсказать компилятору, что
             * shared_from_this — метод класса, а не свободная функция.
             * Для этого вызываем его, используя this
             * Этот вызов bind_front_handler аналогичен
             * namespace ph = std::placeholders;
             * std::bind(&Listener::OnAccept, this->shared_from_this(), ph::_1, ph::_2) */
            beast::bind_front_handler(&Listener::OnAccept, this->shared_from_this()));
    }

    // Метод socket::async_accept создаст сокет и передаст его в OnAccept
    void OnAccept(beast::error_code ec, tcp::socket socket) {
        if (ec) {
            //return ReportError(ec, "accept"sv);
	    logging_handler::LogNetworkError(ec.value(), ec.message(), "accept"sv);
        }
        // Асинхронно обрабатываем сессию
        AsyncRunSession(std::move(socket));
        // Принимаем новое соединение
        DoAccept();
    }

    net::io_context &ioc_;
    tcp::acceptor acceptor_;
    /* Для обработки запроса нужно вызвать функцию request_handler_, переданную в Session.
     * Эта функция принимает два параметра:
     * - Rvalue-ссылку на запрос.
     * - Функцию, отправляющую ответ клиенту. В качестве этой функции используется обобщённая лямбда-функция —
     * аналог шаблонной функции. Это позволит обработчику передавать в лямбда-функцию ответ произвольного типа,
     * что пригодится в будущем. */
    RequestHandler request_handler_;
};

/* Для запуска сервера вспомогательная функция ServeHttp. */
template <typename RequestHandler>
void ServeHttp(net::io_context& ioc, tcp::endpoint endpoint, RequestHandler&& handler) {
    /* При помощи decay_t исключим ссылки из типа RequestHandler,
     * чтобы Listener хранил RequestHandler по значению */
    using MyListener = Listener<std::decay_t<RequestHandler>>;
    std::make_shared<MyListener>(ioc, endpoint, std::forward<RequestHandler>(handler))->Run();
}

}  // namespace http_server
