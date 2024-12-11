#include "postgres.h"

#include <pqxx/zview.hxx>

namespace postgres {

using namespace std::literals;
using pqxx::operator"" _zv;

/* ---------------------------- Author ---------------------------- */

void AuthorRepositoryImpl::Save(const domain::Author& author) {
    // Пока каждое обращение к репозиторию выполняется внутри отдельной транзакции
    // В будущих уроках вы узнаете про паттерн Unit of Work, при помощи которого сможете несколько
    // запросов выполнить в рамках одной транзакции.
    // Вы также может самостоятельно почитать информацию про этот паттерн и применить его здесь.
    pqxx::work work{connection_};
    work.exec_params(
        R"(
INSERT INTO authors (id, name) VALUES ($1, $2)
ON CONFLICT (id) DO UPDATE SET name=$2;
)"_zv,
        author.GetId().ToString(), author.GetName());
    work.commit();
}

std::vector<domain::Author> AuthorRepositoryImpl::Show() {
    std::vector<domain::Author> authors;
    pqxx::read_transaction r(connection_);
    const auto get_authors_sql = R"(SELECT id, name FROM authors ORDER BY name ASC;)"_zv;

    for (auto [id, author] : r.query<std::string, std::string>(get_authors_sql)) {
        authors.emplace_back(domain::AuthorId::FromString(std::move(id)),
                             std::move(author));
    }
    return authors;
}

/* ---------------------------- Book ---------------------------- */

void BookRepositoryImpl::Save(const domain::Book &book) {
    pqxx::work work{connection_};
    work.exec_params(R"(
INSERT INTO books (id, author_id, title, publication_year) VALUES($1, $2, $3, $4);
    )"_zv,
    book.GetId().ToString(), book.GetAuthorId().ToString(), book.GetTitle(), book.GetPublicationYear());
    work.commit();
}

std::vector<domain::Book> BookRepositoryImpl::ShowAll() {
    std::vector<domain::Book> books;
    pqxx::read_transaction r(connection_);
    const auto show_books_sql = R"(SELECT id, author_id, title, publication_year FROM books ORDER BY title ASC;)"_zv;

    for (auto [id, author_id, title, year] : r.query<std::string, std::string, std::string, uint64_t>(show_books_sql)) {
        books.emplace_back(domain::BookId::FromString(std::move(id)),
                           domain::AuthorId::FromString(std::move(author_id)),
                           std::move(title),
                           year);
    }
    return books;
}

std::vector<domain::Book> BookRepositoryImpl::ShowByAuthor(const domain::AuthorId& author_id) {
    std::vector<domain::Book> books;
    pqxx::read_transaction r(connection_);
    std::string query_str = "SELECT id, author_id, title, publication_year FROM books WHERE author_id = '" +
                            author_id.ToString() + "' ORDER BY title ASC;";

    for (auto [id, author_id, title, year] : r.query<std::string, std::string, std::string, uint64_t>(
                        pqxx::zview(query_str))) {
        books.emplace_back(domain::BookId::FromString(std::move(id)),
                           domain::AuthorId::FromString(std::move(author_id)),
                           std::move(title),
                           year);
    }
    return books;
}

/* ---------------------------- Database ---------------------------- */

Database::Database(pqxx::connection connection)
    : connection_{std::move(connection)} {
    pqxx::work work{connection_};
    work.exec(R"(
CREATE TABLE IF NOT EXISTS authors (
    id UUID CONSTRAINT author_id_constraint PRIMARY KEY,
    name varchar(100) UNIQUE NOT NULL
);
)"_zv);
    work.exec(R"(
CREATE TABLE IF NOT EXISTS books (
    id UUID PRIMARY KEY,
    author_id UUID NOT NULL,
    title varchar(100) NOT NULL,
    publication_year integer CHECK (publication_year > 0));
    )"_zv);

    // коммитим изменения
    work.commit();
}

}  // namespace postgres