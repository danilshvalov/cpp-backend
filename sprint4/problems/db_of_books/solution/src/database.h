#pragma once

#include "model.h"

#include <pqxx/connection>
#include <pqxx/transaction>
#include <pqxx/result>

namespace database {

using pqxx::operator"" _zv;

class BookTable {
  public:
    BookTable(pqxx::connection& connection) :
        connection_(connection),
        add_book_tag_("add_book"_zv) {
        CreateTable();
        PrepareAddBookCommand(add_book_tag_);
    }

    void CreateTable() {
        pqxx::work w(connection_);
        w.exec(
            R"(CREATE TABLE IF NOT EXISTS books (
                    id SERIAL PRIMARY KEY,
                    title varchar(100) NOT NULL,
                    author varchar(100) NOT NULL,
                    year integer NOT NULL,
                    ISBN char(13) UNIQUE
                );)"_zv
        );
        w.commit();
    }

    void AddBook(const model::Book& book) {
        pqxx::work w(connection_);
        w.exec_prepared(
            add_book_tag_,
            book.title,
            book.author,
            book.year,
            book.isbn
        );
        w.commit();
    };

    std::vector<model::Book> GetBooks() const {
        pqxx::read_transaction r(connection_);
        const auto query_text =
            "SELECT * FROM books ORDER BY year DESC, title ASC, author ASC, ISBN ASC";
        std::vector<model::Book> books;

        for (const auto& [id, title, author, year, isbn] :
             r.query<
                 size_t,
                 std::string,
                 std::string,
                 int,
                 std::optional<std::string>>(query_text)) {
            books.emplace_back(id, title, author, year, isbn);
        }

        return books;
    }

  private:
    void PrepareAddBookCommand(const pqxx::zview& tag) {
        connection_.prepare(
            tag,
            "INSERT INTO books (title, author, year, ISBN) VALUES ($1, $2, $3, $4)"_zv
        );
    };

    pqxx::connection& connection_;
    const pqxx::zview add_book_tag_;
};

}  // namespace database
