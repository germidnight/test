#include "postgres.h"

#include <pqxx/pqxx>
#include <pqxx/zview.hxx>
#include <pqxx/result.hxx>

namespace postgres {

using namespace std::literals;
using pqxx::operator"" _zv;

/* ---------------------------- Author ---------------------------- */

void AuthorRepositoryImpl::Save(const domain::Author& author) {
    unit_of_work_.AddAuthor(author);
}
void AuthorRepositoryImpl::Delete(const domain::AuthorId& id) {
    unit_of_work_.DeleteAuthor(id);
}
void AuthorRepositoryImpl::Delete(const std::string& name) {
    unit_of_work_.DeleteAuthor(name);
}
void AuthorRepositoryImpl::Edit(const domain::Author& new_author){
    unit_of_work_.EditAuthor(new_author);
}
void AuthorRepositoryImpl::Edit(const std::string& old_name, const std::string& new_name) {
    unit_of_work_.EditAuthor(old_name, new_name);
}
std::string AuthorRepositoryImpl::GetName(const domain::AuthorId& id) {
    return unit_of_work_.GetAuthorName(id);
}
std::string AuthorRepositoryImpl::GetID(const std::string& name) {
    return unit_of_work_.GetAuthorID(name);
}
std::vector<domain::Author> AuthorRepositoryImpl::Show() {
    return unit_of_work_.ShowAuthors();
}

/* ---------------------------- Book ---------------------------- */

void BookRepositoryImpl::Save(const domain::Book& book) {
    unit_of_work_.AddBook(book);
}
void BookRepositoryImpl::Delete(const domain::BookId& id) {
    unit_of_work_.DeleteBook(id);
}
void BookRepositoryImpl::Edit(const domain::Book& new_book) {
    unit_of_work_.EditBook(new_book);
}
std::vector<domain::Book> BookRepositoryImpl::ShowAll() {
    return unit_of_work_.ShowAllBooks();
}
std::vector<domain::Book> BookRepositoryImpl::ShowByAuthor(const domain::AuthorId& author_id) {
    return unit_of_work_.ShowBooksByAuthor(author_id);
}
domain::Book BookRepositoryImpl::ShowInfoByID(const domain::BookId& book_id) {
    return unit_of_work_.ShowBookInfoByID(book_id);
}
std::vector<domain::Book> BookRepositoryImpl::ShowInfoByTitle(const std::string& book_title) {
    return unit_of_work_.ShowBookInfoByTitle(book_title);
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
    work.exec(R"(
CREATE TABLE IF NOT EXISTS book_tags (
    book_id UUID,
    tag varchar(30));
    )"_zv);
    work.commit();
}

/* ---------------------------- Unit Of Work ---------------------------- */

void UnitOfWork::AddAuthor(const domain::Author& author) {
    pqxx::work work{connection_};
    work.exec_params(
        R"(
INSERT INTO authors (id, name) VALUES ($1, $2)
ON CONFLICT (id) DO UPDATE SET name=$2;
)"_zv, author.GetId().ToString(), author.GetName());
    work.commit();
}

void UnitOfWork::DeleteAuthor(const domain::AuthorId& id){
    pqxx::work work{connection_};
    work.exec(R"(START TRANSACTION;)"_zv);
    work.exec_params(R"(
DELETE FROM book_tags
WHERE book_id IN (
    SELECT id FROM books
    WHERE author_id = $1
);)"_zv, id.ToString());
    work.exec_params(R"(DELETE FROM books WHERE author_id = $1;)"_zv, id.ToString());
    work.exec_params(R"(DELETE FROM authors WHERE id = $1;)"_zv, id.ToString());
    work.exec(R"(COMMIT;)"_zv);
    work.commit();
}
void UnitOfWork::DeleteAuthor(const std::string& name){
    pqxx::work work{connection_};
    work.exec(R"(START TRANSACTION;)"_zv);
    work.exec_params(R"(
DELETE FROM book_tags
WHERE book_id IN (
    SELECT id FROM books
    WHERE author_id IN (
        SELECT id FROM authors
        WHERE name = $1
    )
);)"_zv, name);
    work.exec_params(R"(
DELETE FROM books
WHERE author_id IN (
    SELECT id FROM authors
    WHERE name = $1
);)"_zv, name);
    pqxx::result res = work.exec_params0(R"(DELETE FROM authors WHERE name = $1;)"_zv, name);
    if (res.affected_rows() == 0) {
        throw std::runtime_error("No such author"s);
    }
    work.exec(R"(COMMIT;)"_zv);
    work.commit();
}

void UnitOfWork::EditAuthor(const domain::Author& new_author){
    pqxx::work work{connection_};
    work.exec_params(R"(
UPDATE authors SET name = $1 WHERE id = $2;
    )"_zv, new_author.GetName(), new_author.GetId().ToString());
    work.commit();
}
void UnitOfWork::EditAuthor(const std::string& old_name, const std::string& new_name) {
    pqxx::work work{connection_};
    work.exec_params(R"(
UPDATE authors SET name = $1 WHERE name = $2;
    )"_zv, new_name, old_name);
    work.commit();
}

std::string UnitOfWork::GetAuthorName(const domain::AuthorId& id) {
    pqxx::read_transaction r(connection_);
    std::string query_str = "SELECT name FROM authors WHERE id = '" +
                            id.ToString() + "';";
    return r.query_value<std::string>(pqxx::zview(query_str));
}
std::string UnitOfWork::GetAuthorID(const std::string& name) {
    pqxx::read_transaction r(connection_);
    std::string query_str = "SELECT id FROM authors WHERE name = '" + name + "';";
    return r.query_value<std::string>(pqxx::zview(query_str));
}
std::vector<domain::Author> UnitOfWork::ShowAuthors() {
    std::vector<domain::Author> authors;
    pqxx::read_transaction r(connection_);
    const auto get_authors_sql = R"(SELECT id, name FROM authors ORDER BY name ASC;)"_zv;

    for (auto [id, author] : r.query<std::string, std::string>(get_authors_sql)) {
        authors.emplace_back(domain::AuthorId::FromString(std::move(id)),
                             std::move(author));
    }
    return authors;
}


void UnitOfWork::AddBook(const domain::Book& book) {
    pqxx::work work{connection_};
    work.exec(R"(START TRANSACTION;)"_zv);
    work.exec_params(R"(
INSERT INTO books (id, author_id, title, publication_year) VALUES($1, $2, $3, $4);
    )"_zv, book.GetId().ToString(), book.GetAuthorId().ToString(), book.GetTitle(), book.GetPublicationYear());
    for (const std::string& tag : book.GetTags()) {
        work.exec_params(R"(
INSERT INTO book_tags (book_id, tag) VALUES($1, $2);
        )"_zv, book.GetId().ToString(), tag);
    }
    work.exec(R"(COMMIT;)"_zv);
    work.commit();
}

void UnitOfWork::DeleteBook(const domain::BookId& id) {
    pqxx::work work{connection_};
    work.exec(R"(START TRANSACTION;)"_zv);
    work.exec_params(R"(
DELETE FROM book_tags
WHERE book_id IN (
    SELECT id FROM books
    WHERE author_id = $1
);)"_zv, id.ToString());
    pqxx::result res = work.exec_params0(R"(DELETE FROM books WHERE id = $1;)"_zv, id.ToString());
    if (res.affected_rows() == 0) {
        throw std::runtime_error("No such book"s);
    }
    work.exec(R"(COMMIT;)"_zv);
    work.commit();
}

void UnitOfWork::EditBook(const domain::Book& new_book) {
    pqxx::work work{connection_};
    std::string book_id = new_book.GetId().ToString();
    work.exec(R"(START TRANSACTION;)"_zv);
    work.exec_params(R"(
UPDATE books SET title = $1, publication_year = $2 WHERE id = $3;
    )"_zv, new_book.GetTitle(), new_book.GetPublicationYear(), book_id);
    work.exec_params(R"(DELETE FROM book_tags WHERE book_id = $1;)"_zv, book_id);
    for (const std::string& tag : new_book.GetTags()) {
        work.exec_params(R"(
INSERT INTO book_tags (book_id, tag) VALUES($1, $2);
        )"_zv, book_id, tag);
    }
    work.exec(R"(COMMIT;)"_zv);
    work.commit();
}

std::vector<domain::Book> UnitOfWork::ShowAllBooks() {
    std::vector<domain::Book> books;
    pqxx::read_transaction r(connection_);
    const auto show_books_sql = R"(
SELECT books.id, author_id, title, publication_year
FROM books
JOIN authors ON books.author_id = authors.id
ORDER BY books.title, authors.name, books.publication_year;)"_zv;

    for (auto [id, author_id, title, year] :
                    r.query<std::string, std::string, std::string, uint64_t>(show_books_sql)) {
        books.emplace_back(domain::BookId::FromString(std::move(id)),
                           domain::AuthorId::FromString(std::move(author_id)),
                           std::move(title),
                           year);
    }
    return books;
}
std::vector<domain::Book> UnitOfWork::ShowBooksByAuthor(const domain::AuthorId& author_id){
    std::vector<domain::Book> books;
    pqxx::read_transaction r(connection_);
    std::string query_str = "SELECT id, author_id, title, publication_year FROM books WHERE author_id = '" +
                            author_id.ToString() + "' ORDER BY publication_year, title;";

    for (auto [id, author_id, title, year] : r.query<std::string, std::string, std::string, uint64_t>(
             pqxx::zview(query_str))) {
        books.emplace_back(domain::BookId::FromString(std::move(id)),
                           domain::AuthorId::FromString(std::move(author_id)),
                           std::move(title),
                           year);
    }
    return books;
}

domain::Book UnitOfWork::ShowBookInfoByID(const domain::BookId& book_id) {
    pqxx::read_transaction r(connection_);
    std::string query1_str = "SELECT id, author_id, title, publication_year FROM books WHERE id = '" +
                            book_id.ToString() + "' ORDER BY publication_year, title;";
    std::string query2_str = "SELECT tag FROM book_tags WHERE book_id = '" +
                             book_id.ToString() + "' ORDER BY tag ASC;";
    auto [id, author_id, title, year] =
                r.query1<std::string, std::string, std::string, uint64_t>(pqxx::zview(query1_str));
    domain::Book book(domain::BookId::FromString(std::move(id)),
                      domain::AuthorId::FromString(std::move(author_id)),
                      std::move(title),
                      year,
                      {});
    for (auto [tag] : r.query<std::string>(pqxx::zview(query2_str))) {
        book.AddTag(std::move(tag));
    }
    return book;
}
std::vector<domain::Book> UnitOfWork::ShowBookInfoByTitle(const std::string& book_title) {
    pqxx::read_transaction r(connection_);
    std::vector<domain::Book> books;
    std::string query1_str = "SELECT id, author_id, title, publication_year FROM books WHERE title = '" +
                             book_title + "' ORDER BY publication_year, title;";
    for (auto [id, author_id, title, year] :
                r.query<std::string, std::string, std::string, uint64_t>(pqxx::zview(query1_str))) {
        books.emplace_back(domain::BookId::FromString(std::move(id)),
                           domain::AuthorId::FromString(std::move(author_id)),
                           std::move(title),
                           year);
        std::string query2_str = "SELECT tag FROM book_tags WHERE book_id = '" +
                                 id + "' ORDER BY tag;";
        for (auto [tag] : r.query<std::string>(pqxx::zview(query2_str))) {
            books.back().AddTag(std::move(tag));
        }
    }
    return books;
}

}  // namespace postgres