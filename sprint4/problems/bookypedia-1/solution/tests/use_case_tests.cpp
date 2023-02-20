#include <catch2/catch_test_macros.hpp>

#include "../src/app/use_cases_impl.h"
#include "../src/domain/author.h"

namespace {

struct MockAuthorRepository : domain::AuthorRepository {
    domain::Authors saved_authors;

    void Save(const domain::Author& author) override {
        saved_authors.emplace_back(author);
    }

    domain::Authors GetAllAuthors() override {
        return saved_authors;
    }
};

struct MockBookRepository : domain::BookRepository {
    domain::Books saved_books;

    void Save(const domain::Book& book) override {
        saved_books.emplace_back(book);
    }

    domain::Books GetAllBooks() override {
        return saved_books;
    }

    domain::Books GetBooksByAuthorId(const domain::AuthorId& id) override {
        assert("TODO: implement");
    }
};

struct Fixture {
    MockAuthorRepository authors;
    MockBookRepository books;
};

}  // namespace

SCENARIO_METHOD(Fixture, "Book Adding") {
    GIVEN("Use cases") {
        app::UseCasesImpl use_cases {authors, books};

        WHEN("Adding an author") {
            const auto author_name = "Joanne Rowling";
            use_cases.AddAuthor(author_name);

            THEN("author with the specified name is saved to repository") {
                REQUIRE(authors.saved_authors.size() == 1);
                CHECK(authors.saved_authors.at(0).GetName() == author_name);
                CHECK(
                    authors.saved_authors.at(0).GetId() != domain::AuthorId {}
                );
            }
        }
    }
}
