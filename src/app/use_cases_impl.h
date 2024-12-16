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

    domain::AuthorId AddAuthor(const std::string& name) override;
    std::string GetAuthorName(const std::string& id) override;
    std::string GetAuthorID(const std::string& name) override;
    std::vector<domain::Author> ShowAuthors() override;
    void DeleteAuthorByID(const std::string& id) override;
    void DeleteAuthorByName(const std::string& name) override;
    void EditAuthorByID(const std::string& id,
                        const std::string& new_name) override;
    void EditAuthorByName(const std::string& old_name,
                          const std::string& new_name) override;

    void AddBook(const std::string& author_id,
                 const std::string& title,
                 uint64_t year,
                 const std::vector<std::string>& tags) override;
    void DeleteBook(const std::string& id) override;
    void EditBook(const std::string& id,
                  const std::string& author_id,
                  const std::string& title,
                  uint64_t publication_year,
                  const std::vector<std::string>& tags) override;

    std::vector<domain::Book> ShowAllBooks() override;
    std::vector<domain::Book> ShowAuthorBooks(const std::string& author_id) override;
    domain::Book ShowBookInfoByID(const std::string& book_id) override;
    std::vector<domain::Book> ShowBookInfoByTitle(const std::string& book_title) override;

private:
    domain::AuthorRepository& authors_;
    domain::BookRepository& books_;
};

}  // namespace app
