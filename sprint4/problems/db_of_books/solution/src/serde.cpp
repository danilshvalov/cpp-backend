#include "serde.h"

namespace serde {

namespace keys {
    namespace Book {
        constexpr json::string_view id = "id";
        constexpr json::string_view title = "title";
        constexpr json::string_view author = "author";
        constexpr json::string_view year = "year";
        constexpr json::string_view isbn = "ISBN";
    }  // namespace Book
}  // namespace keys

model::Book ParseBook(const json::object& object) {
    model::Book book {
        .title = std::string(object.at(keys::Book::title).as_string()),
        .author = std::string(object.at(keys::Book::author).as_string()),
        .year = static_cast<int>(object.at(keys::Book::year).as_int64()),
    };

    if (auto it = object.find(keys::Book::id); it != object.end()) {
        book.id = it->value().as_int64();
    }

    if (auto it = object.find(keys::Book::isbn);
        it != object.end() && !it->value().is_null()) {
        book.isbn = std::string(it->value().as_string());
    }

    return book;
}

json::value SerializeBook(const model::Book& book) {
    json::object object({
        {keys::Book::id, book.id},
        {keys::Book::title, book.title},
        {keys::Book::author, book.author},
        {keys::Book::year, book.year},
    });

    if (book.isbn) {
        object[keys::Book::isbn] = *book.isbn;
    } else {
        object[keys::Book::isbn] = nullptr;
    }

    return object;
}

json::value SerializeBooks(const std::vector<model::Book>& books) {
    json::array array;
    array.reserve(books.size());

    for (const auto& book : books) {
        array.push_back(SerializeBook(book));
    }

    return array;
}
}  // namespace serde
