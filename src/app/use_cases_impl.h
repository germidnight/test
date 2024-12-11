/*
 * Реализация интерфейса взаимодействия с модулем представления данных
 * Примечание:
 * Интерфейсы для взаимодействия с модулем хранения (authors_, books_) реализованы в модуле хранения
 */
#pragma once
#include "../domain/author_fwd.h"
#include "use_cases.h"

namespace app {

class UseCasesImpl : public UseCases {
public:
    explicit UseCasesImpl(domain::AuthorRepository& authors, domain::BookRepository& books)
        : authors_{authors}
        , books_{books} {}

    void AddAuthor(const std::string& name) override;
    std::vector<domain::Author> ShowAuthors() override;

    void AddBook(const std::string& author_id,
                         const std::string& title,
                         uint64_t year) override;
    std::vector<domain::Book> ShowAllBooks() override;
    std::vector<domain::Book> ShowAuthorBooks(const std::string& author_id) override;

private:
    domain::AuthorRepository& authors_;
    domain::BookRepository& books_;
};

}  // namespace app
