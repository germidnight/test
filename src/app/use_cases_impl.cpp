#include "use_cases_impl.h"

#include "../domain/author.h"

namespace app {
using namespace domain;

void UseCasesImpl::AddAuthor(const std::string& name) {
    authors_.Save({AuthorId::New(), name});
}

std::vector<Author> UseCasesImpl::ShowAuthors() {
    return authors_.Show();
}

void UseCasesImpl::AddBook(const std::string& author_id,
                         const std::string& title,
                         uint64_t year) {
    books_.Save({BookId::New(),
                 AuthorId::FromString(author_id),
                 title,
                 year});
}
std::vector<domain::Book> UseCasesImpl::ShowAllBooks() {
    return books_.ShowAll();
}
std::vector<domain::Book> UseCasesImpl::ShowAuthorBooks(const std::string& author_id) {
    return books_.ShowByAuthor(AuthorId::FromString(author_id));
}

}  // namespace app
