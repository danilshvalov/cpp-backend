#include <catch2/catch_test_macros.hpp>
#include <algorithm>

#include "../src/app/use_cases_impl.h"
#include "../src/domain/author.h"

namespace {

struct MockAuthorRepository : domain::AuthorRepository {
    domain::Authors saved_authors;

    void Save(const domain::Author& author) override {
        saved_authors.emplace_back(author);
    }

    domain::Author GetAuthorById(const domain::AuthorId& id) override {
        auto it = std::find_if(
            saved_authors.begin(),
            saved_authors.end(),
            [&](const auto& author) { return author.GetId() == id; }
        );

        assert(it != saved_authors.end());

        return *it;
    }

    std::optional<domain::Author> GetAuthorByName(const std::string& name
    ) override {
        auto it = std::find_if(
            saved_authors.begin(),
            saved_authors.end(),
            [&](const auto& author) { return author.GetName() == name; }
        );

        if (it == saved_authors.end()) {
            return std::nullopt;
        }
        return *it;
    }

    void EditAuthorName(const domain::AuthorId& id, const std::string& name)
        override {
        auto it = std::find_if(
            saved_authors.begin(),
            saved_authors.end(),
            [&](const auto& a) { return a.GetId() == id; }
        );

        assert(it != saved_authors.end());

        it->SetName(name);
    }

    void Delete(const domain::AuthorId& id) override {
        std::erase_if(saved_authors, [&](const auto& author) {
            return author.GetId() == id;
        });
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

    void Edit(
        const domain::BookId& id,
        const std::string& title,
        int publication_year,
        const domain::Tags& tags
    ) override {
        auto it = std::find_if(
            saved_books.begin(),
            saved_books.end(),
            [&](const auto& book) { return book.GetId() == id; }
        );
        assert(it != saved_books.end());
        it->SetTitle(title);
        it->SetPublicationYear(publication_year);
        it->SetTags(tags);
    }

    void Delete(const domain::BookId& id) override {
        std::erase_if(saved_books, [&](const auto& book) {
            return book.GetId() == id;
        });
    }

    domain::Books GetAllBooks() override {
        return saved_books;
    }

    domain::Books GetBooksByAuthorId(const domain::AuthorId& id) override {
        domain::Books books = saved_books;
        std::erase_if(books, [&](const auto& book) {
            return book.GetAuthorId() != id;
        });
        return books;
    }
};

struct MockUnitOfWork : app::UnitOfWork {
    MockAuthorRepository& authors;
    MockBookRepository& books;

    MockUnitOfWork(MockAuthorRepository& authors, MockBookRepository& books) :
        authors(authors),
        books(books) {}

    virtual void Commit() {}

    virtual domain::AuthorRepository& Authors() {
        return authors;
    }

    virtual domain::BookRepository& Books() {
        return books;
    }
};

struct MockUnitOfWorkFactory : app::UnitOfWorkFactory {
    MockAuthorRepository& authors;
    MockBookRepository& books;

    MockUnitOfWorkFactory(
        MockAuthorRepository& authors,
        MockBookRepository& books
    ) :
        authors(authors),
        books(books) {}

    virtual app::UnitOfWorkHolder CreateUnitOfWork() {
        return std::make_unique<MockUnitOfWork>(authors, books);
    }
};

struct Fixture {
    MockAuthorRepository authors;
    MockBookRepository books;
    MockUnitOfWorkFactory unit_factory {authors, books};
};

}  // namespace

SCENARIO_METHOD(Fixture, "Book Adding") {
    GIVEN("Use cases") {
        app::UseCasesImpl use_cases {unit_factory};

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
