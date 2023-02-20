#pragma once
#include <pqxx/connection>
#include <pqxx/transaction>

#include "../domain/author.h"
#include "../domain/book.h"

namespace postgres {

class AuthorRepositoryImpl : public domain::AuthorRepository {
  public:
    explicit AuthorRepositoryImpl(pqxx::connection& connection) :
        connection_(connection) {}

    void Save(const domain::Author& author) override;
    domain::Authors GetAllAuthors() override;

  private:
    pqxx::connection& connection_;
};

class BookRepositoryImpl : public domain::BookRepository {
  public:
    explicit BookRepositoryImpl(pqxx::connection& connection) :
        connection_(connection) {}

    void Save(const domain::Book& book) override;
    domain::Books GetAllBooks() override;
    domain::Books GetBooksByAuthorId(const domain::AuthorId& id) override;

  private:
    pqxx::connection& connection_;
};

class Database {
  public:
    explicit Database(pqxx::connection connection);

    AuthorRepositoryImpl& GetAuthors() {
        return authors_;
    }

    BookRepositoryImpl& GetBooks() {
        return books_;
    }

  private:
    pqxx::connection connection_;
    AuthorRepositoryImpl authors_ {connection_};
    BookRepositoryImpl books_ {connection_};
};

}  // namespace postgres
