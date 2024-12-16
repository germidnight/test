/*
 * Модуль отображения интерфейса пользователя
 */
#pragma once
#include <iosfwd>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace menu {
class Menu;
}

namespace app {
class UseCases;
}

namespace ui {
namespace detail {

struct AddBookParams {
    std::string title;
    std::string author_id;
    int publication_year = 0;
    std::vector<std::string> tags;
};

struct AuthorInfo {
    std::string id;
    std::string name;
};

struct BookInfo {
    std::string id;
    std::string title;
    int publication_year;
};

struct BookFullInfo {
    std::string id;
    std::string author_id;
    std::string title;
    int publication_year;
    std::string author_name;
    std::vector<std::string> tags;
};

enum class AuthorEnteredAs {
    REJECT,
    ID,
    NAME
};

}  // namespace detail

class View {
public:
    View(menu::Menu& menu, app::UseCases& use_cases, std::istream& input, std::ostream& output);

private:
    bool AddAuthor(std::istream& cmd_input) const;
    bool DeleteAuthor(std::istream& cmd_input) const;
    bool EditAuthor(std::istream& cmd_input) const;

    bool AddBook(std::istream& cmd_input) const;
    bool DeleteBook(std::istream& cmd_input) const;
    bool EditBook(std::istream& cmd_input) const;
    bool ShowAuthors() const;
    bool ShowBooks() const;
    bool ShowAuthorBooks(std::istream& cmd_input) const;
    bool ShowBook(std::istream& cmd_input) const;

    std::optional<detail::AddBookParams> GetBookParams(std::istream& cmd_input) const;
    detail::BookFullInfo GetEditBookParams(const detail::BookFullInfo& old_book) const;
    std::optional<std::string> SelectAuthor() const;
    std::optional<std::string> SelectBook() const;
    std::optional<size_t> SelectFromBooks(const std::vector<detail::BookFullInfo>& books) const;

    std::vector<detail::AuthorInfo> GetAuthors() const;
    void CheckAuthorPresenceByName(const std::string& author_name) const;
    std::vector<detail::BookFullInfo> GetBooks() const;
    std::vector<detail::BookInfo> GetAuthorBooks(const std::string& author_id) const;
    detail::BookFullInfo GetBookById(const std::string& book_id) const;
    std::vector<detail::BookFullInfo> GetBookByTitle(const std::string& book_title) const;

    std::pair<detail::AuthorEnteredAs, std::string> GetAuthorParams(std::istream& cmd_input) const;

    menu::Menu& menu_;
    app::UseCases& use_cases_;
    std::istream& input_;
    std::ostream& output_;
};

}  // namespace ui