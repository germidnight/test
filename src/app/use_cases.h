/*
 * Интерфейс взаимодействия с модулем представления данных
 */
#pragma once

#include <string>
#include <vector>

#include "../domain/author.h"

namespace app {

class UseCases {
public:
    virtual domain::AuthorId AddAuthor(const std::string& name) = 0;
    virtual std::string GetAuthorName(const std::string& id) = 0;
    virtual std::string GetAuthorID(const std::string& name) = 0;
    virtual std::vector<domain::Author> ShowAuthors() = 0;
    virtual void DeleteAuthorByID(const std::string& id) = 0;
    virtual void DeleteAuthorByName(const std::string& name) = 0;
    virtual void EditAuthorByID(const std::string& id,
                                const std::string& new_name) = 0;
    virtual void EditAuthorByName(const std::string& old_name,
                                  const std::string& new_name) = 0;

    virtual void AddBook(const std::string& author_id,
                         const std::string& title,
                         uint64_t year,
                         const std::vector<std::string>& tags) = 0;
    virtual void DeleteBook(const std::string& id) = 0;
    virtual void EditBook(const std::string& id,
                          const std::string& author_id,
                          const std::string& title,
                          uint64_t publication_year,
                          const std::vector<std::string>& tags) = 0;

    virtual std::vector<domain::Book> ShowAllBooks() = 0;
    virtual std::vector<domain::Book> ShowAuthorBooks(const std::string& author_id) = 0;
    virtual domain::Book ShowBookInfoByID(const std::string& book_id) = 0;
    virtual std::vector<domain::Book> ShowBookInfoByTitle(const std::string& book_title) = 0;

protected:
    ~UseCases() = default;
};

}  // namespace app
