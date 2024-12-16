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

class UnitOfWork {
public:
    explicit UnitOfWork(pqxx::connection& connection)
                : connection_{connection}{}
    void AddAuthor(const domain::Author& author);
    std::string GetAuthorName(const domain::AuthorId& id);
    std::string GetAuthorID(const std::string& id);
    std::vector<domain::Author> ShowAuthors();
    void DeleteAuthor(const domain::AuthorId& id);
    void DeleteAuthor(const std::string& name);
    void EditAuthor(const domain::Author& new_author);
    void EditAuthor(const std::string& old_name, const std::string& new_name);

    void AddBook(const domain::Book& book);
    std::vector<domain::Book> ShowAllBooks();
    std::vector<domain::Book> ShowBooksByAuthor(const domain::AuthorId& author_id);
    domain::Book ShowBookInfoByID(const domain::BookId& book_id);
    std::vector<domain::Book> ShowBookInfoByTitle(const std::string& book_title);
    void DeleteBook(const domain::BookId& id);
    void EditBook(const domain::Book& new_book);

private:
    pqxx::connection& connection_;
};

class AuthorRepositoryImpl : public domain::AuthorRepository {
public:
    explicit AuthorRepositoryImpl(pqxx::connection& connection)
        : unit_of_work_{connection} {
    }

    void Save(const domain::Author& author) override;
    std::string GetName(const domain::AuthorId& id) override;
    std::string GetID(const std::string& name) override;
    std::vector<domain::Author> Show() override;
    void Delete(const domain::AuthorId& id) override;
    void Delete(const std::string& name) override;
    void Edit(const domain::Author& new_author) override;
    void Edit(const std::string& old_name, const std::string& new_name) override;

private:
    UnitOfWork unit_of_work_;
};

class BookRepositoryImpl : public domain::BookRepository {
public:
    explicit BookRepositoryImpl(pqxx::connection& connection)
                : unit_of_work_{connection} {}

    void Save(const domain::Book& book) override;
    std::vector<domain::Book> ShowAll() override;
    std::vector<domain::Book> ShowByAuthor(const domain::AuthorId& author_id) override;
    domain::Book ShowInfoByID(const domain::BookId& book_id) override;
    std::vector<domain::Book> ShowInfoByTitle(const std::string& book_title) override;
    void Delete(const domain::BookId& id) override;
    void Edit(const domain::Book& new_book) override;

private:
    UnitOfWork unit_of_work_;
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