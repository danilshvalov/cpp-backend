#include "use_cases_impl.h"

#include "../domain/author.h"
#include "../domain/book.h"

namespace app {
using namespace domain;

void UseCasesImpl::AddAuthor(const std::string& name) {
    auto work = unit_factory_.CreateUnitOfWork();
    work->Authors().Save({AuthorId::New(), name});
    work->Commit();
}

Author UseCasesImpl::GetAuthorById(const AuthorId& id) {
    auto work = unit_factory_.CreateUnitOfWork();
    return work->Authors().GetAuthorById(id);
}

std::optional<Author> UseCasesImpl::GetAuthorByName(const std::string& name) {
    auto work = unit_factory_.CreateUnitOfWork();
    return work->Authors().GetAuthorByName(name);
}

void UseCasesImpl::EditAuthorName(
    const domain::AuthorId& id,
    const std::string& name
) {
    auto work = unit_factory_.CreateUnitOfWork();
    work->Authors().EditAuthorName(id, name);
    work->Commit();
}

void UseCasesImpl::DeleteAuthor(const domain::AuthorId& id) {
    auto work = unit_factory_.CreateUnitOfWork();
    work->Authors().Delete(id);
    work->Commit();
}

void UseCasesImpl::EditBook(
    const domain::BookId& id,
    const std::string& title,
    int year,
    const domain::Tags& tags
) {
    auto work = unit_factory_.CreateUnitOfWork();
    work->Books().Edit(id, title, year, tags);
    work->Commit();
}

void UseCasesImpl::DeleteBook(const domain::BookId& id) {
    auto work = unit_factory_.CreateUnitOfWork();
    work->Books().Delete(id);
    work->Commit();
}

Authors UseCasesImpl::GetAllAuthors() {
    auto work = unit_factory_.CreateUnitOfWork();
    return work->Authors().GetAllAuthors();
}

void UseCasesImpl::AddBook(
    const AuthorId& author_id,
    const std::string& title,
    int year,
    const Tags& tags
) {
    auto work = unit_factory_.CreateUnitOfWork();
    work->Books().Save({BookId::New(), author_id, title, year, tags});
    work->Commit();
}

Books UseCasesImpl::GetAllBooks() {
    auto work = unit_factory_.CreateUnitOfWork();
    return work->Books().GetAllBooks();
}

Books UseCasesImpl::GetBooksByAuthorId(const AuthorId& id) {
    auto work = unit_factory_.CreateUnitOfWork();
    return work->Books().GetBooksByAuthorId(id);
}

}  // namespace app
