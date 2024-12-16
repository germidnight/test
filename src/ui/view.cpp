#include "view.h"

#include <algorithm>
#include <boost/algorithm/string/trim.hpp>
#include <cassert>
#include <iostream>
#include <set>
#include <utility>

#include "../app/use_cases.h"
#include "../menu/menu.h"

using namespace std::literals;
namespace ph = std::placeholders;

namespace ui {
namespace detail {

std::ostream& operator<<(std::ostream& out, const AuthorInfo& author) {
    out << author.name;
    return out;
}

std::ostream& operator<<(std::ostream& out, const BookInfo& book) {
    out << book.title << ", " << book.publication_year;
    return out;
}

std::ostream& operator<<(std::ostream& out, const BookFullInfo& book) {
    out << book.title << " by " << book.author_name << ", " << book.publication_year;
    return out;
}

}  // namespace detail

std::ostream &operator<<(std::ostream &out, const std::vector<std::string> &tags) {
    bool fisrt_time = true;
    for (const std::string &tag : tags) {
        if (!fisrt_time) {
            out << ", "s;
        } else {
            fisrt_time = false;
        }
        out << tag;
    }
    return out;
}

template <typename T>
void PrintVector(std::ostream& out, const std::vector<T>& vector) {
    int i = 1;
    for (auto& value : vector) {
        out << i++ << " " << value << std::endl;
    }
}

void PrintFullInfoOfBook(std::ostream& out, const detail::BookFullInfo& book_info) {
    out << "Title: "sv << book_info.title << std::endl;
    out << "Author: "sv << book_info.author_name << std::endl;
    out << "Publication year: "sv << book_info.publication_year << std::endl;
    if (!book_info.tags.empty()) {
        out << "Tags: "sv << book_info.tags << std::endl;
    }
}

std::vector<std::string> SplitIntoWords(const std::string& text, char delim) {
    const size_t max_len_of_tag = 30;
    std::set<std::string> words;
    std::string word;
    for (const char c : text) {
        if (c == delim) {
            boost::algorithm::trim(word);
            if (!word.empty()) {
                if (word.length() > max_len_of_tag) {
                    word.resize(max_len_of_tag);
                }
                words.insert(std::move(word));
            }
            word.clear();
        } else {
            if ((c != ' ') || (word.back() != ' ')) {
                word += c;
            }
        }
    }
    boost::algorithm::trim(word);
    if (!word.empty()) {
        words.insert(std::move(word));
    }
    return {words.begin(), words.end()};
}

View::View(menu::Menu& menu, app::UseCases& use_cases, std::istream& input, std::ostream& output)
    : menu_{menu}
    , use_cases_{use_cases}
    , input_{input}
    , output_{output} {
    menu_.AddAction(  //
        "AddAuthor"s, "name"s, "Adds author"s, std::bind(&View::AddAuthor, this, ph::_1)
        // ����
        // [this](auto& cmd_input) { return AddAuthor(cmd_input); }
    );
    menu_.AddAction("AddBook"s, "<pub year> <title>"s, "Adds book"s,
                    std::bind(&View::AddBook, this, ph::_1));
    menu_.AddAction("ShowAuthors"s, {}, "Show authors"s, std::bind(&View::ShowAuthors, this));
    menu_.AddAction("ShowBooks"s, {}, "Show books"s, std::bind(&View::ShowBooks, this));
    menu_.AddAction("ShowAuthorBooks"s, {}, "Show author books"s,
                    std::bind(&View::ShowAuthorBooks, this, ph::_1));
    menu_.AddAction("DeleteAuthor"s, "<author_name>"s, "Delete author and his books"s,
                    std::bind(&View::DeleteAuthor, this, ph::_1));
    menu_.AddAction("EditAuthor"s, "<author_name>"s, "Edit authors name"s,
                    std::bind(&View::EditAuthor, this, ph::_1));
    menu_.AddAction("ShowBook"s, "<book_title>"s, "Shows book info"s,
                    std::bind(&View::ShowBook, this, ph::_1));
    menu_.AddAction("DeleteBook"s, "<title>"s, "Delete book"s,
                    std::bind(&View::DeleteBook, this, ph::_1));
    menu_.AddAction("EditBook"s, "<title>"s, "Edit book"s,
                    std::bind(&View::EditBook, this, ph::_1));
}

bool View::AddAuthor(std::istream& cmd_input) const {
    try {
        std::string name;
        std::getline(cmd_input, name);
        boost::algorithm::trim(name);
        if (name.empty()) {
            throw std::runtime_error("Empty author"s);
        }
        use_cases_.AddAuthor(std::move(name));
    } catch (const std::exception&) {
        output_ << "Failed to add author"sv << std::endl;
    }
    return true;
}

bool View::DeleteAuthor(std::istream& cmd_input) const {
    try {
        auto author = GetAuthorParams(cmd_input);
        if (author.first == detail::AuthorEnteredAs::NAME) {
            use_cases_.DeleteAuthorByName(author.second);
        } else if (author.first == detail::AuthorEnteredAs::ID) {
            use_cases_.DeleteAuthorByID(author.second);
        } else {
            throw std::runtime_error("Author not found"s);
        }
    } catch(const std::exception&) {
        output_ << "Failed to delete author"sv << std::endl;
    }
    return true;
}

bool View::EditAuthor(std::istream &cmd_input) const {
    try {
        auto author = GetAuthorParams(cmd_input);
        if (author.first == detail::AuthorEnteredAs::REJECT) {
            return true;
        }
        if (author.first == detail::AuthorEnteredAs::NAME) {
            CheckAuthorPresenceByName(author.second);
        }
        output_ << "Enter new name:"sv << std::endl;
        std::string new_name;
        std::getline(input_, new_name);
        boost::algorithm::trim(new_name);
        if (new_name.empty()) {
            throw std::runtime_error("Empty author"s);
        }
        if (author.first == detail::AuthorEnteredAs::NAME) {
            use_cases_.EditAuthorByName(author.second, new_name);
        } else {
            use_cases_.EditAuthorByID(author.second, new_name);
        }
    } catch (const std::exception &) {
        output_ << "Failed to edit author"sv << std::endl;
    }
    return true;
}
/* Получение автора:
 * 1) Проверяется задание имени автора в команде
 * 2) Если в команде автор не задан, предлагается выбрать автора из списка*/
std::pair<detail::AuthorEnteredAs, std::string> View::GetAuthorParams(std::istream &cmd_input) const {
    std::string author_name;

    std::getline(cmd_input, author_name);
    boost::algorithm::trim(author_name);
    if (!author_name.empty()) {
        return {detail::AuthorEnteredAs::NAME, author_name};
    }
    if (auto author_id = SelectAuthor()) {
        return {detail::AuthorEnteredAs::ID, author_id.value()};
    }
    return {detail::AuthorEnteredAs::REJECT, {}};
}

void View::CheckAuthorPresenceByName(const std::string& author_name) const {
    auto authors = GetAuthors();
    auto it = std::find_if(authors.begin(), authors.end(),
                [&author_name] (const detail::AuthorInfo& author) {
                    return (author.name == author_name);
                });
    if (it == authors.end()) {
        throw std::runtime_error("No such author"s);
    }
}

bool View::AddBook(std::istream& cmd_input) const {
    try {
        if (auto params = GetBookParams(cmd_input)) {
            use_cases_.AddBook(params->author_id,
                               params->title,
                               params->publication_year,
                               params->tags);
        }
    } catch (const std::exception&) {
        output_ << "Failed to add book"sv << std::endl;
    }
    return true;
}

/* Удаление книги
 * 1) Получаем параметры со ввода команды (title)
 * 2) Если книга (title) не введена, предлагаем выбрать из списка
 * 3) Если книга (title) введена ищем книгу/книги
 * 4) Если с указанным названием найдено несколько книг, предлагаем выбрать какую удалить */
bool View::DeleteBook(std::istream& cmd_input) const {
    try {
        std::string title;
        std::getline(cmd_input, title);
        if (title.empty()) {
            if (auto id = SelectBook()) {
                use_cases_.DeleteBook(id.value());
                return true;
            }
        } else {
            boost::algorithm::trim(title);
            auto books = GetBookByTitle(title);
            if(books.size() == 0) {
                output_ << "Book not found"s << std::endl;
            } else if (books.size() == 1) {
                use_cases_.DeleteBook(books[0].id);
            } else {
                if (auto book_idx = SelectFromBooks(books)) {
                    use_cases_.DeleteBook(books[book_idx.value()].id);
                }
            }
        }
    } catch(const std::exception&) {
        output_ << "Failed to delete book"sv << std::endl;
    }
    return true;
}

bool View::EditBook(std::istream& cmd_input) const {
    try {
        detail::BookFullInfo new_book;
        std::string title;
        std::getline(cmd_input, title);
        boost::algorithm::trim(title);
        if (title.empty()) {
            if (auto id = SelectBook()) {
                auto old_book = GetBookById(id.value());
                new_book = GetEditBookParams(old_book);
            } else {
                throw std::runtime_error("Book not found"s);
            }
        } else {
            auto books = GetBookByTitle(title);
            if (books.size() == 0) {
                throw std::runtime_error("Book not found"s);
            } else if (books.size() == 1) {
                new_book = GetEditBookParams(books[0]);
            } else {
                if (auto book_idx = SelectFromBooks(books)) {
                    new_book = GetEditBookParams(books[book_idx.value()]);
                }
            }
        }
        use_cases_.EditBook(new_book.id,
                            new_book.author_id,
                            new_book.title,
                            new_book.publication_year,
                            new_book.tags);
    } catch(const std::exception&) {
        output_ << "Book not found"sv << std::endl;
    }
    return true;
}

bool View::ShowAuthors() const {
    PrintVector(output_, GetAuthors());
    return true;
}

bool View::ShowBooks() const {
    PrintVector(output_, GetBooks());
    return true;
}

bool View::ShowAuthorBooks(std::istream& cmd_input) const {
    try {
        std::string title;
        std::getline(cmd_input, title);
        boost::algorithm::trim(title);
        if (title.empty()) {
            if (auto author_id = SelectAuthor()) {
                PrintVector(output_, GetAuthorBooks(*author_id));
            }
        } else {
            std::string author_id = use_cases_.GetAuthorID(title);
            PrintVector(output_, GetAuthorBooks(author_id));
        }
    } catch (const std::exception&) {
        throw std::runtime_error("Failed to Show Books");
    }
    return true;
}
/* Отображение полной иформации о книге:
 * 1) Проверяем ввёл ли пользователь название книги
 * 2) Если нет, то выводим список книг на выбор и выводим информацию по выбранной книге
 * 3) Если название книги введено, ищем её
 * 4) Если книга одна, то выводим информацию по выбранной книге
 * 5) Если книг с указанным названием больше одной, то просим выбрать по какой именно нужна справка
 * и выводим её*/
bool View::ShowBook(std::istream& cmd_input) const {
    try {
        std::string title;
        std::getline(cmd_input, title);
        boost::algorithm::trim(title);
        if (title.empty()) {
            if (auto id = SelectBook()) {
                PrintFullInfoOfBook(output_, GetBookById(id.value()));
            }
        } else {
            auto books = GetBookByTitle(title);
            if (books.size() == 0) {
                return true;
            } else if (books.size() == 1) {
                PrintFullInfoOfBook(output_, books[0]);
            } else {
                if (auto book_idx = SelectFromBooks(books)) {
                    PrintFullInfoOfBook(output_, books[book_idx.value()]);
                }
            }
        }
    } catch(const std::exception&) {}
    return true;
}

/* Получение параметров добавления книги
 * 1) Получаем параметры со ввода команды (title, publication_year)
 * 2) Просим ввести автора
 * 3) Если автор введён ищем автора, если автора не нашли предлагаем добавить
 * 4) Если пользователь согласился добавить автора, то добавляем нового автора и сохраняем id автора
 * 5) Если автор найден, сохраняем id автора
 * 6) Если автор не введён, предлагаем выбрать из списка
 * 7) Предлагаем ввести теги для книги */
std::optional<detail::AddBookParams> View::GetBookParams(std::istream& cmd_input) const {
    detail::AddBookParams params;

    cmd_input >> params.publication_year;
    std::getline(cmd_input, params.title);
    boost::algorithm::trim(params.title);
    if (params.title.empty()) {
        throw std::runtime_error("Title is empty"s);
    }

    output_ << "Enter author name or empty line to select from list:"sv << std::endl;
    std::string author_name;
    std::getline(input_, author_name);
    if (!author_name.empty()) {
        boost::algorithm::trim(author_name);
        auto authors = GetAuthors();
        auto author_it = std::find_if(authors.begin(), authors.end(),
                                      [&author_name](const ui::detail::AuthorInfo &author_info) {
                                          return (author_info.name == author_name);
                                      });
        if (author_it == authors.end()) {
            output_ << "No author found. Do you want to add "s + author_name + " (y/n)?"s << std::endl;
            std::string answer;
            std::getline(input_, answer);
            if ((answer != "y") && (answer != "Y")) {
                throw std::runtime_error("There is no author in base"s);
            }
            params.author_id = use_cases_.AddAuthor(std::move(author_name)).ToString();
        } else {
            params.author_id = author_it->id;
        }
    } else {
        auto author_id = SelectAuthor();
        if (not author_id.has_value())
            return std::nullopt;
        else {
            params.author_id = author_id.value();
        }
    }

    output_ << "Enter tags (comma separated):"sv << std::endl;
    std::string tags_raw;
    std::getline(input_, tags_raw);
    params.tags = SplitIntoWords(tags_raw, ',');
    return params;
}

detail::BookFullInfo View::GetEditBookParams(const detail::BookFullInfo& old_book) const {
    detail::BookFullInfo new_book;

    new_book = std::move(old_book);

    output_ << "Enter new title or empty line to use the current one ("s
            << old_book.title << "):"s << std::endl;
    std::string new_title;
    std::getline(input_, new_title);
    boost::algorithm::trim(new_title);
    if (!new_title.empty()) {
        new_book.title = std::move(new_title);
    }

    output_ << "Enter publication year or empty line to use the current one ("s
            << old_book.publication_year << "):"s << std::endl;
    std::string new_year;
    std::getline(input_, new_year);
    boost::algorithm::trim(new_year);
    if (!new_year.empty()) {
        try {
            new_book.publication_year = std::stoul(new_year);
        } catch (const std::exception&) {
            new_book.publication_year = old_book.publication_year;
        }
    }

    output_ << "Enter tags (current tags: "s << old_book.tags << "):"s << std::endl;
    std::string new_tags;
    std::getline(input_, new_tags);
    //if (!new_tags.empty()) {
        new_book.tags = SplitIntoWords(new_tags, ',');
    //}
    return new_book;
}

std::optional<std::string> View::SelectAuthor() const {
    output_ << "Select author:" << std::endl;
    auto authors = GetAuthors();
    PrintVector(output_, authors);
    output_ << "Enter author # or empty line to cancel" << std::endl;

    std::string str;
    if (!std::getline(input_, str) || str.empty()) {
        return std::nullopt;
    }

    int author_idx;
    try {
        author_idx = std::stoi(str);
    } catch (const std::exception&) {
        throw std::runtime_error("Invalid author num");
    }

    --author_idx;
    if (author_idx < 0 or author_idx >= authors.size()) {
        throw std::runtime_error("Invalid author num");
    }

    return authors[author_idx].id;
}

std::optional<std::string> View::SelectBook() const {
    auto books = GetBooks();
    PrintVector(output_, books);
    output_ << "Enter the book # or empty line to cancel:" << std::endl;

    std::string str;
    if (!std::getline(input_, str) || str.empty()) {
        return std::nullopt;
    }

    int book_idx;
    try {
        book_idx = std::stoi(str);
    } catch (const std::exception&) {
        throw std::runtime_error("Invalid book num");
    }

    --book_idx;
    if (book_idx < 0 or book_idx >= books.size()) {
        throw std::runtime_error("Invalid book num");
    }
    return books[book_idx].id;
}

std::optional<size_t> View::SelectFromBooks(const std::vector<detail::BookFullInfo>& books) const {
    PrintVector(output_, books);
    output_ << "Enter the book # or empty line to cancel:" << std::endl;

    std::string str;
    if (!std::getline(input_, str) || str.empty()) {
        return std::nullopt;
    }

    int book_idx;
    try {
        book_idx = std::stoi(str);
    } catch (const std::exception &) {
        throw std::runtime_error("Invalid book num");
    }

    --book_idx;
    if (book_idx < 0 or book_idx >= books.size()) {
        throw std::runtime_error("Invalid book num");
    }
    return static_cast<size_t>(book_idx);
}

std::vector<detail::AuthorInfo> View::GetAuthors() const {
    std::vector<detail::AuthorInfo> dst_authors;

    for (auto& author : use_cases_.ShowAuthors()) {
        dst_authors.emplace_back(std::move(author.GetId().ToString()), std::move(author.GetName()));
    }
    return dst_authors;
}

std::vector<detail::BookFullInfo> View::GetBooks() const {
    std::vector<detail::BookFullInfo> dst_books;

    for (auto &book : use_cases_.ShowAllBooks()) {
        std::string author_name;
        try {
            author_name = use_cases_.GetAuthorName(book.GetAuthorId().ToString());
        } catch (const std::exception&) {
            continue;
        }
        dst_books.emplace_back(std::move(book.GetId().ToString()),
                               std::move(book.GetAuthorId().ToString()),
                               std::move(book.GetTitle()),
                               book.GetPublicationYear(),
                               std::move(author_name));
    }
    return dst_books;
}

std::vector<detail::BookInfo> View::GetAuthorBooks(const std::string& author_id) const {
    std::vector<detail::BookInfo> dst_books;
    std::string author_name = use_cases_.GetAuthorName(author_id);
    for (auto& book : use_cases_.ShowAuthorBooks(author_id)) {
        dst_books.emplace_back(std::move(book.GetId().ToString()),
                               std::move(book.GetTitle()),
                               book.GetPublicationYear());
    }
    return dst_books;
}

detail::BookFullInfo View::GetBookById(const std::string& book_id) const {
    auto book = use_cases_.ShowBookInfoByID(book_id);
    return {std::move(book.GetId().ToString()),
            book.GetAuthorId().ToString(),
            std::move(book.GetTitle()),
            static_cast<int>(book.GetPublicationYear()),
            use_cases_.GetAuthorName(book.GetAuthorId().ToString()),
            std::move(book.GetTags())};
}
std::vector<detail::BookFullInfo> View::GetBookByTitle(const std::string& book_title) const {
    std::vector<detail::BookFullInfo> dst_books;

    for (auto& book : use_cases_.ShowBookInfoByTitle(book_title)) {
        dst_books.emplace_back(std::move(book.GetId().ToString()),
                               book.GetAuthorId().ToString(),
                               std::move(book.GetTitle()),
                               book.GetPublicationYear(),
                               use_cases_.GetAuthorName(book.GetAuthorId().ToString()),
                               std::move(book.GetTags()));
    }
    return dst_books;
}

}  // namespace ui
