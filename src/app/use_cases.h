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
    virtual void AddAuthor(const std::string& name) = 0;
    virtual std::vector<domain::Author> ShowAuthors() = 0;

    virtual void AddBook(const std::string& author_id,
                         const std::string& title,
                         uint64_t year) = 0;
    virtual std::vector<domain::Book> ShowAllBooks() = 0;
    virtual std::vector<domain::Book> ShowAuthorBooks(const std::string& author_id) = 0;

protected:
    ~UseCases() = default;
};

}  // namespace app
