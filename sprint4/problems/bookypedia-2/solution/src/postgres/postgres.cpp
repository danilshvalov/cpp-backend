#include "postgres.h"

#include <pqxx/zview.hxx>
#include <pqxx/result>
#include <fmt/core.h>

namespace postgres {

using namespace std::literals;
using pqxx::operator"" _zv;

void AuthorRepositoryImpl::Save(const domain::Author& author) {
    // Пока каждое обращение к репозиторию выполняется внутри отдельной транзакции
    // В будущих уроках вы узнаете про паттерн Unit of Work, при помощи которого сможете несколько
    // запросов выполнить в рамках одной транзакции.
    // Вы также может самостоятельно почитать информацию про этот паттерн и применить его здесь.
    work_.exec_params(
        R"(
            INSERT INTO authors (id, name) VALUES ($1, $2)
            ON CONFLICT (id) DO UPDATE SET name=$2;
        )"_zv,
        author.GetId().ToString(),
        author.GetName()
    );
}

domain::Author AuthorRepositoryImpl::GetAuthorById(const domain::AuthorId& id) {
    const auto query_text = fmt::format(
        "SELECT id, name FROM authors WHERE id = {};",
        work_.quote(id.ToString())
    );
    const auto& [_, name] = work_.query1<std::string, std::string>(query_text);
    return domain::Author {id, name};
}

std::optional<domain::Author>
AuthorRepositoryImpl::GetAuthorByName(const std::string& name) {
    const auto query_text = fmt::format(
        "SELECT id, name FROM authors WHERE name = {};",
        work_.quote(name)
    );
    const auto& data = work_.query01<std::string, std::string>(query_text);
    if (!data) {
        return std::nullopt;
    }
    const auto& [id, author_name] = *data;
    return domain::Author {domain::AuthorId::FromString(id), author_name};
}

void AuthorRepositoryImpl::EditAuthorName(
    const domain::AuthorId& id,
    const std::string& name
) {
    work_.exec_params(
        R"(
        UPDATE authors
        SET name = $1
        WHERE id = $2
    )",
        name,
        id.ToString()
    );
}

void AuthorRepositoryImpl::Delete(const domain::AuthorId& id) {
    work_.exec_params("DELETE FROM authors WHERE id = $1", id.ToString());
}

domain::Authors AuthorRepositoryImpl::GetAllAuthors() {
    const auto query_text = "SELECT id, name FROM authors ORDER BY name"_zv;
    domain::Authors authors;

    for (const auto& [id, name] :
         work_.query<std::string, std::string>(query_text)) {
        authors.emplace_back(domain::AuthorId::FromString(id), name);
    }

    return authors;
}

void BookRepositoryImpl::Save(const domain::Book& book) {
    work_.exec_params(
        R"(
            INSERT INTO books (id, author_id, title, publication_year) VALUES ($1, $2, $3, $4)
            ON CONFLICT (id) DO UPDATE SET author_id=$2, title=$3, publication_year=$4;
        )"_zv,
        book.GetId().ToString(),
        book.GetAuthorId().ToString(),
        book.GetTitle(),
        book.GetPublicationYear()
    );

    for (const auto& tag : book.GetTags()) {
        work_.exec_params(
            "INSERT INTO tags (book_id, tag) VALUES($1, $2);",
            book.GetId().ToString(),
            tag
        );
    }
}

void BookRepositoryImpl::Edit(
    const domain::BookId& id,
    const std::string& title,
    int publication_year,
    const domain::Tags& tags
) {
    work_.exec_params(
        R"(
            UPDATE books
            SET title = $1, publication_year = $2
            WHERE id = $3
        )",
        title,
        publication_year,
        id.ToString()
    );
    work_.exec_params(
        R"(
            DELETE FROM tags
            WHERE book_id = $1
        )",
        id.ToString()
    );

    for (const auto& tag : tags) {
        work_.exec_params(
            "INSERT INTO tags (book_id, tag) VALUES($1, $2);",
            id.ToString(),
            tag
        );
    }
}

void BookRepositoryImpl::Delete(const domain::BookId& id) {
    work_.exec_params("DELETE FROM books WHERE id = $1", id.ToString());
}

domain::Tags BookRepositoryImpl::GetBookTags(const domain::BookId& id) {
    const auto query_text = fmt::format(
        R"(
            SELECT tag FROM tags
            WHERE book_id = {}
            ORDER BY tag;
        )",
        work_.quote(id.ToString())
    );

    domain::Tags tags;
    for (const auto& [tag] : work_.query<std::string>(query_text)) {
        tags.push_back(tag);
    }
    return tags;
}

domain::Books BookRepositoryImpl::GetAllBooks() {
    const auto query_text = R"(
        SELECT id, author_id, title, publication_year
        FROM books
        ORDER BY title
    )"_zv;
    domain::Books books;

    for (const auto& [id, author_id, title, publication_year] :
         work_.query<std::string, std::string, std::string, int>(query_text)) {
        auto book_id = domain::BookId::FromString(id);
        books.emplace_back(
            book_id,
            domain::AuthorId::FromString(author_id),
            title,
            publication_year,
            GetBookTags(book_id)
        );
    }

    return books;
}

domain::Books BookRepositoryImpl::GetBooksByAuthorId(const domain::AuthorId& id
) {
    const auto query_text = fmt::format(
        R"(
            SELECT id, author_id, title, publication_year
            FROM books
            WHERE author_id = {}
            ORDER BY publication_year, title;
        )",
        work_.quote(id.ToString())
    );
    domain::Books books;

    for (const auto& [id, author_id, title, publication_year] :
         work_.query<std::string, std::string, std::string, int>(query_text)) {
        auto book_id = domain::BookId::FromString(id);
        books.emplace_back(
            book_id,
            domain::AuthorId::FromString(author_id),
            title,
            publication_year,
            GetBookTags(book_id)
        );
    }

    return books;
}

Database::Database(pqxx::connection connection) :
    connection_ {std::move(connection)} {
    pqxx::work work {connection_};

    work.exec(R"(
        CREATE TABLE IF NOT EXISTS authors (
            id UUID CONSTRAINT author_id_constraint PRIMARY KEY,
            name varchar(100) UNIQUE NOT NULL
        );
    )"_zv);

    work.exec(R"(
        CREATE TABLE IF NOT EXISTS books (
            id UUID CONSTRAINT book_id_constraint PRIMARY KEY,
            author_id UUID NOT NULL,
            title varchar(100) NOT NULL,
            publication_year INTEGER NOT NULL
        );
    )"_zv);

    work.exec(R"(
        CREATE TABLE IF NOT EXISTS tags (
            book_id UUID references books(id) NOT NULL,
            tag varchar(30) NOT NULL
        );
    )"_zv);

    work.commit();
}

}  // namespace postgres
