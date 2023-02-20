#include "use_cases_impl.h"

#include "../domain/author.h"
#include "../domain/book.h"

namespace app {
using namespace domain;

void UseCasesImpl::AddAuthor(const std::string& name) {
    authors_.Save({AuthorId::New(), name});
}

Authors UseCasesImpl::GetAllAuthors() {
    return authors_.GetAllAuthors();
}

void UseCasesImpl::AddBook(
    const AuthorId& author_id,
    const std::string& title,
    int year
) {
    books_.Save({BookId::New(), author_id, title, year});
}

Books UseCasesImpl::GetAllBooks() {
    return books_.GetAllBooks();
}

Books UseCasesImpl::GetBooksByAuthorId(const AuthorId& id) {
    return books_.GetBooksByAuthorId(id);
}

}  // namespace app
