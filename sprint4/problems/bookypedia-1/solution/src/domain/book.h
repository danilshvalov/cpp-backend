#pragma once
#include <string>
#include <vector>

#include "../util/tagged_uuid.h"
#include "author.h"

namespace domain {

using BookId = util::TaggedUUID<class BookTag>;

class Book {
  public:
    Book(
        BookId id,
        AuthorId author_id,
        std::string title,
        int publication_year
    ) :
        id_(std::move(id)),
        author_id_(std::move(author_id)),
        title_(std::move(title)),
        publication_year_(std::move(publication_year)) {}

    const BookId& GetId() const noexcept {
        return id_;
    }

    const AuthorId& GetAuthorId() const noexcept {
        return author_id_;
    }

    const std::string& GetTitle() const noexcept {
        return title_;
    }

    const int GetPublicationYear() const noexcept {
        return publication_year_;
    }

  private:
    BookId id_;
    AuthorId author_id_;
    std::string title_;
    int publication_year_;
};

using Books = std::vector<Book>;

class BookRepository {
  public:
    virtual void Save(const Book& author) = 0;
    virtual Books GetAllBooks() = 0;
    virtual Books GetBooksByAuthorId(const AuthorId& id) = 0;

  protected:
    ~BookRepository() = default;
};

}  // namespace domain
