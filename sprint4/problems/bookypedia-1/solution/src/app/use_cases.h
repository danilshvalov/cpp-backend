#pragma once

#include <string>
#include <vector>

#include "../domain/book.h"
#include "../domain/author.h"

namespace app {

class UseCases {
  public:
    virtual void AddAuthor(const std::string& name) = 0;
    virtual domain::Authors GetAllAuthors() = 0;

    virtual void AddBook(
        const domain::AuthorId& author_id,
        const std::string& title,
        int year
    ) = 0;
    virtual domain::Books GetAllBooks() = 0;
    virtual domain::Books GetBooksByAuthorId(const domain::AuthorId& id) = 0;

  protected:
    ~UseCases() = default;
};

}  // namespace app
