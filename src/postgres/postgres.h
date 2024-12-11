/*
 * Модуль хранения. Отвечает за запись и чтение данных в СУБД PostgreSQL
 */
#pragma once
#include <pqxx/connection>
#include <pqxx/transaction>
#include <string>
#include <vector>

#include "../domain/author.h"

namespace postgres {

class AuthorRepositoryImpl : public domain::AuthorRepository {
public:
    explicit AuthorRepositoryImpl(pqxx::connection& connection)
        : connection_{connection} {
    }

    void Save(const domain::Author& author) override;
    std::vector<domain::Author> Show() override;

private:
    pqxx::connection& connection_;
};

class BookRepositoryImpl : public domain::BookRepository {
public:
    explicit BookRepositoryImpl(pqxx::connection& connection)
                : connection_(connection) {}

    void Save(const domain::Book& book) override;
    std::vector<domain::Book> ShowAll() override;
    std::vector<domain::Book> ShowByAuthor(const domain::AuthorId& author_id) override;

private:
    pqxx::connection& connection_;
};

class Database {
public:
    explicit Database(pqxx::connection connection);

    AuthorRepositoryImpl& GetAuthors() & {
        return authors_;
    }

    BookRepositoryImpl& GetBooks() & {
        return books_;
    }

private:
    pqxx::connection connection_;
    AuthorRepositoryImpl authors_{connection_};
    BookRepositoryImpl books_{connection_};
};

}  // namespace postgres