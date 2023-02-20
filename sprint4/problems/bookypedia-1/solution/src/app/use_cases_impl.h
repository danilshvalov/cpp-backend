#pragma once
#include "../domain/author_fwd.h"
#include "use_cases.h"

namespace app {

class UseCasesImpl : public UseCases {
  public:
    explicit UseCasesImpl(
        domain::AuthorRepository& authors,
        domain::BookRepository& books
    ) :
        authors_(authors),
        books_(books) {}

    void AddAuthor(const std::string& name) override;
    domain::Authors GetAllAuthors() override;

    void AddBook(
        const domain::AuthorId& author_id,
        const std::string& title,
        int year
    ) override;
    domain::Books GetAllBooks() override;
    domain::Books GetBooksByAuthorId(const domain::AuthorId& id) override;

  private:
    domain::AuthorRepository& authors_;
    domain::BookRepository& books_;
};

}  // namespace app
