#include "use_cases_impl.h"

#include "../domain/author.h"

namespace app {
using namespace domain;

AuthorId UseCasesImpl::AddAuthor(const std::string& name) {
    AuthorId id = AuthorId::New();
    authors_.Save({id, name});
    return id;
}

std::string UseCasesImpl::GetAuthorName(const std::string& id) {
    return authors_.GetName(domain::AuthorId::FromString(id));
}

std::string UseCasesImpl::GetAuthorID(const std::string& name) {
    return authors_.GetID(name);
}

std::vector<Author> UseCasesImpl::ShowAuthors() {
    return authors_.Show();
}

void UseCasesImpl::DeleteAuthorByID(const std::string& id) {
    authors_.Delete(AuthorId::FromString(id));
}

void UseCasesImpl::DeleteAuthorByName(const std::string& name) {
    authors_.Delete(name);
}

void UseCasesImpl::EditAuthorByID(const std::string& id,
                                  const std::string& new_name) {
    authors_.Edit({AuthorId::FromString(id), new_name});
}
void UseCasesImpl::EditAuthorByName(const std::string& old_name,
                          const std::string& new_name) {
    authors_.Edit(old_name, new_name);
}

void UseCasesImpl::AddBook(const std::string& author_id,
                           const std::string& title,
                           uint64_t year,
                           const std::vector<std::string>& tags) {
    books_.Save({BookId::New(),
                 AuthorId::FromString(author_id),
                 title,
                 year,
                 tags});
}

void UseCasesImpl::DeleteBook(const std::string& id) {
    books_.Delete(domain::BookId::FromString(id));
}

void UseCasesImpl::EditBook(const std::string& id,
                            const std::string& author_id,
                            const std::string& title,
                            uint64_t publication_year,
                            const std::vector<std::string>& tags) {
    books_.Edit({BookId::FromString(id),
                 AuthorId::FromString(author_id),
                 std::move(title),
                 publication_year,
                 std::move(tags)});
}

std::vector<domain::Book> UseCasesImpl::ShowAllBooks() {
    return books_.ShowAll();
}
std::vector<domain::Book> UseCasesImpl::ShowAuthorBooks(const std::string& author_id) {
    return books_.ShowByAuthor(AuthorId::FromString(author_id));
}

domain::Book UseCasesImpl::ShowBookInfoByID(const std::string& book_id) {
    return books_.ShowInfoByID(BookId::FromString(book_id));
}
std::vector<domain::Book> UseCasesImpl::ShowBookInfoByTitle(const std::string& book_title) {
    return books_.ShowInfoByTitle(book_title);
}

}  // namespace app
