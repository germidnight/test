/*
 * Модуль представления данных
 * Представлены основные объекты для взаимодействия и хранения:
 * - автор (Author) - соответствует таблице "authors" в СУБД
 * - книга (Book) - соответствует таблице "books" в СУБД
 *
 * а также интерфейсы для взаимодействия с модулем хранения:
 * - AuthorRepository - запись, чтение в таблицу "authors" в СУБД
 * - BookRepository - запись, чтение в таблицу "books" в СУБД
 * Интерфейсы реализованы в модуле хранения
 */
#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include "../util/tagged_uuid.h"

namespace domain {

namespace detail {
struct AuthorTag {};
struct BookTag {};
}  // namespace detail

/* ---------------------------- Author ---------------------------- */

using AuthorId = util::TaggedUUID<detail::AuthorTag>;

class Author {
public:
    Author(AuthorId id, std::string name)
        : id_(std::move(id))
        , name_(std::move(name)) {
    }

    const AuthorId& GetId() const noexcept {
        return id_;
    }

    const std::string& GetName() const noexcept {
        return name_;
    }

private:
    AuthorId id_;
    std::string name_;
};

class AuthorRepository {
public:
    virtual void Save(const Author& author) = 0;
    virtual std::string GetName(const AuthorId& id) = 0;
    virtual std::string GetID(const std::string& name) = 0;
    virtual std::vector<Author> Show() = 0;
    virtual void Delete(const AuthorId& id) = 0;
    virtual void Delete(const std::string& name) = 0;
    virtual void Edit(const Author& new_author) = 0;
    virtual void Edit(const std::string& old_name, const std::string& new_name) = 0;

protected:
    ~AuthorRepository() = default;
};

/* ---------------------------- Book ---------------------------- */

using BookId = util::TaggedUUID<detail::BookTag>;

class Book {
public:
    Book(BookId book_id, AuthorId author_id, std::string title, uint64_t year,
         std::vector<std::string> tags = {})
            : id_(std::move(book_id))
            , author_id_(std::move(author_id))
            , title_(std::move(title))
            , publication_year_(year)
            , tags_(std::move(tags)) {}

    const BookId& GetId() const noexcept {
        return id_;
    }

    const AuthorId& GetAuthorId() const noexcept {
        return author_id_;
    }

    const std::string& GetTitle() const noexcept {
        return title_;
    }

    uint64_t GetPublicationYear() const noexcept {
        return publication_year_;
    }

    void AddTag(std::string tag) {
        tags_.emplace_back(std::move(tag));
    }

    const std::vector<std::string>& GetTags() const noexcept {
        return tags_;
    }

private:
    BookId id_;
    AuthorId author_id_;
    std::string title_;
    uint64_t publication_year_;
    std::vector<std::string> tags_;
};

class BookRepository {
public:
    virtual void Save(const Book& book) = 0;
    virtual std::vector<Book> ShowAll() = 0;
    virtual std::vector<Book> ShowByAuthor(const AuthorId& author_id) = 0;
    virtual Book ShowInfoByID(const BookId& book_id) = 0;
    virtual std::vector<Book> ShowInfoByTitle(const std::string& book_title) = 0;
    virtual void Delete(const BookId& id) = 0;
    virtual void Edit(const Book& new_book) = 0;

protected:
    ~BookRepository() = default;
};

}  // namespace domain
