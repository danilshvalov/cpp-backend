#pragma once
#include <string>
#include <vector>

#include "../util/tagged_uuid.h"
#include "author.h"

namespace domain {

using BookId = util::TaggedUUID<class BookTag>;
using Tags = std::vector<std::string>;

class Book {
  public:
    Book(
        BookId id,
        AuthorId author_id,
        std::string title,
        int publication_year,
        Tags tags
    ) :
        id_(std::move(id)),
        author_id_(std::move(author_id)),
        title_(std::move(title)),
        publication_year_(std::move(publication_year)),
        tags_(std::move(tags)) {}

    const BookId& GetId() const noexcept {
        return id_;
    }

    const AuthorId& GetAuthorId() const noexcept {
        return author_id_;
    }

    const std::string& GetTitle() const noexcept {
        return title_;
    }

    void SetTitle(std::string title) {
        title_ = std::move(title);
    }

    const int GetPublicationYear() const noexcept {
        return publication_year_;
    }

    void SetPublicationYear(int publication_year) {
        publication_year_ = publication_year;
    }

    const Tags& GetTags() const noexcept {
        return tags_;
    }

    void SetTags(Tags tags) {
        tags_ = std::move(tags);
    }

  private:
    BookId id_;
    AuthorId author_id_;
    std::string title_;
    int publication_year_;
    Tags tags_;
};

using Books = std::vector<Book>;

class BookRepository {
  public:
    virtual void Save(const Book& author) = 0;
    virtual void Edit(
        const BookId& id,
        const std::string& title,
        int publication_year,
        const Tags& tags
    ) = 0;
    virtual void Delete(const BookId& id) = 0;
    virtual Books GetAllBooks() = 0;
    virtual Books GetBooksByAuthorId(const AuthorId& id) = 0;

  protected:
    ~BookRepository() = default;
};

}  // namespace domain
