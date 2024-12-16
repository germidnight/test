#include <catch2/catch_test_macros.hpp>

#include "../src/app/use_cases_impl.h"
#include "../src/domain/author.h"

namespace {

struct MockAuthorRepository : domain::AuthorRepository {
    std::vector<domain::Author> saved_authors;

    void Save(const domain::Author& author) override {
        saved_authors.emplace_back(author);
    }
    std::vector<domain::Author> Show() override {
        return {};
    }
    std::string GetName(const domain::AuthorId &id) override {return {};}
    std::string GetID(const std::string &name) override { return {}; }
    void Delete(const domain::AuthorId &id) override {}
    void Delete(const std::string &name) override {}
    void Edit(const domain::Author &new_author) override {}
    void Edit(const std::string &old_name, const std::string &new_name) override {}
};

struct MockBookRepository : domain::BookRepository {
    std::vector<domain::Book> saved_books;

    void Save(const domain::Book& book) override {
        saved_books.emplace_back(book);
    }
    std::vector<domain::Book> ShowAll() override {
        return {};
    }
    std::vector<domain::Book> ShowByAuthor([[maybe_unused]] const domain::AuthorId &author_id) override {
        return {};
    }
    domain::Book ShowInfoByID(const domain::BookId &book_id) override {
        return {domain::BookId::New(),
                domain::AuthorId::New(),
                {},
                {},
                {}};
    }
    std::vector<domain::Book> ShowInfoByTitle(const std::string &book_title) override {return {};}
    void Delete(const domain::BookId &id) override {}
    void Edit(const domain::Book &new_book) override {}
};

struct Fixture {
    MockAuthorRepository authors;
    MockBookRepository books;
};

}  // namespace

SCENARIO_METHOD(Fixture, "Book Adding") {
    GIVEN("Use cases") {
        app::UseCasesImpl use_cases{authors, books};

        WHEN("Adding an author") {
            const auto author_name = "Joanne Rowling";
            use_cases.AddAuthor(author_name);

            THEN("author with the specified name is saved to repository") {
                REQUIRE(authors.saved_authors.size() == 1);
                CHECK(authors.saved_authors.at(0).GetName() == author_name);
                CHECK(authors.saved_authors.at(0).GetId() != domain::AuthorId{});
            }
        }
    }
}