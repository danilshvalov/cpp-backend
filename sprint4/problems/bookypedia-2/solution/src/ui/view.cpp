#include "view.h"

#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/join.hpp>
#include <cassert>
#include <iostream>

#include "../app/use_cases.h"
#include "../menu/menu.h"
#include "../util/string.h"

using namespace std::literals;
namespace ph = std::placeholders;

namespace ui {
namespace detail {

    std::ostream& operator<<(std::ostream& out, const AuthorInfo& author) {
        out << author.name;
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const BookInfo& book) {
        out << book.title << " by " << book.author_name << ", "
            << book.publication_year;
        return out;
    }

}  // namespace detail

template<typename T>
void PrintVector(std::ostream& out, const std::vector<T>& vector) {
    int i = 1;
    for (auto& value : vector) {
        out << i++ << " " << value << std::endl;
    }
}

View::View(
    menu::Menu& menu,
    app::UseCases& use_cases,
    std::istream& input,
    std::ostream& output
) :
    menu_ {menu},
    use_cases_ {use_cases},
    input_ {input},
    output_ {output} {
    menu_.AddAction(
        "AddAuthor"s,
        "name"s,
        "Adds author"s,
        std::bind(&View::AddAuthor, this, ph::_1)
    );
    menu_.AddAction(
        "EditAuthor"s,
        "name"s,
        "Edits author"s,
        std::bind(&View::EditAuthor, this, ph::_1)
    );
    menu_.AddAction(
        "DeleteAuthor"s,
        "name"s,
        "Deletes author"s,
        std::bind(&View::DeleteAuthor, this, ph::_1)
    );
    menu_.AddAction(
        "AddBook"s,
        "<pub year> <title>"s,
        "Adds book"s,
        std::bind(&View::AddBook, this, ph::_1)
    );
    menu_.AddAction(
        "EditBook"s,
        "title"s,
        "Edits book"s,
        std::bind(&View::EditBook, this, ph::_1)
    );
    menu_.AddAction(
        "DeleteBook"s,
        "name"s,
        "Deletes book"s,
        std::bind(&View::DeleteBook, this, ph::_1)
    );
    menu_.AddAction(
        "ShowAuthors"s,
        {},
        "Show authors"s,
        std::bind(&View::ShowAuthors, this)
    );
    menu_.AddAction(
        "ShowBooks"s,
        {},
        "Show books"s,
        std::bind(&View::ShowBooks, this)
    );
    menu_.AddAction(
        "ShowBook"s,
        "name"s,
        "Show book info"s,
        std::bind(&View::ShowBook, this, ph::_1)
    );
    menu_.AddAction(
        "ShowAuthorBooks"s,
        {},
        "Show author books"s,
        std::bind(&View::ShowAuthorBooks, this)
    );
}

bool View::AddAuthor(std::istream& cmd_input) const {
    try {
        std::string name;
        std::getline(cmd_input, name);
        boost::algorithm::trim(name);
        if (name.empty()) {
            throw std::runtime_error("Author name cannot be empty");
        }
        use_cases_.AddAuthor(std::move(name));
    } catch (const std::exception&) {
        output_ << "Failed to add author"sv << std::endl;
    }
    return true;
}

std::optional<std::string> View::ReadAuthor(std::istream& cmd_input) const {
    std::string name;
    std::getline(cmd_input, name);
    boost::algorithm::trim(name);

    if (name.empty()) {
        return std::nullopt;
    }

    auto author = use_cases_.GetAuthorByName(name);
    if (!author) {
        throw std::runtime_error("Author not found");
    }

    return author->GetId().ToString();
}

std::optional<std::string> View::ReadAuthor() const {
    output_ << "Enter author name or empty line to select from list:"
            << std::endl;

    std::string name;
    std::getline(input_, name);
    boost::algorithm::trim(name);

    if (name.empty()) {
        return SelectAuthor();
    }

    auto author = use_cases_.GetAuthorByName(name);
    if (!author) {
        output_ << "No author found. Do you want to add " + name + " (y/n)?"
                << std::endl;
        std::string answer;
        std::getline(input_, answer);
        boost::algorithm::trim(answer);

        if (answer != "y" && answer != "Y") {
            throw std::runtime_error("Author not found");
        }

        use_cases_.AddAuthor(std::move(name));
    }

    author = use_cases_.GetAuthorByName(name);
    if (!author) {
        throw std::runtime_error("Author not found");
    }

    return author->GetId().ToString();
}

std::optional<detail::BookInfo>
View::SelectBook(const std::vector<detail::BookInfo>& books) const {
    PrintVector(output_, books);
    output_ << "Enter the book # or empty line to cancel:" << std::endl;

    std::string str;
    if (!std::getline(input_, str) || str.empty()) {
        return std::nullopt;
    }

    size_t book_idx;
    try {
        book_idx = std::stoi(str);
    } catch (const std::exception&) {
        throw std::runtime_error("Invalid book num");
    }

    --book_idx;
    if (book_idx < 0 or book_idx >= books.size()) {
        throw std::runtime_error("Invalid book num");
    }

    return books[book_idx];
}

std::optional<detail::BookInfo> View::SelectBook() const {
    return SelectBook(GetBooks());
}

std::optional<detail::BookInfo> View::ReadBookByTitle(std::istream& cmd_input
) const {
    std::string title;
    std::getline(cmd_input, title);
    boost::algorithm::trim(title);

    if (title.empty()) {
        return SelectBook();
    }

    auto books = GetBooks();
    std::erase_if(books, [&](const auto& book) { return book.title != title; });

    if (books.empty()) {
        return std::nullopt;
    }
    if (books.size() == 1) {
        return books.front();
    }

    return SelectBook(books);
}

std::string View::ReadName() const {
    output_ << "Enter new name:" << std::endl;
    std::string name;
    std::getline(input_, name);
    boost::algorithm::trim(name);

    if (name.empty()) {
        throw std::runtime_error("Name cannot be empty");
    }

    return name;
}

bool View::EditAuthor(std::istream& cmd_input) const {
    try {
        std::string author_id;
        if (auto id = ReadAuthor(cmd_input)) {
            author_id = *id;
        } else if (auto id = SelectAuthor()) {
            author_id = *id;
        } else {
            return true;
        }

        use_cases_.EditAuthorName(
            domain::AuthorId::FromString(author_id),
            ReadName()
        );
    } catch (const std::exception&) {
        output_ << "Failed to edit author"sv << std::endl;
    }
    return true;
}

bool View::DeleteAuthor(std::istream& cmd_input) const {
    try {
        std::string author_id;
        if (auto id = ReadAuthor(cmd_input)) {
            author_id = *id;
        } else if (auto id = SelectAuthor()) {
            author_id = *id;
        } else {
            return true;
        }

        use_cases_.DeleteAuthor(domain::AuthorId::FromString(author_id));
    } catch (const std::exception&) {
        output_ << "Failed to delete author"sv << std::endl;
    }
    return true;
}

bool View::AddBook(std::istream& cmd_input) const {
    try {
        if (auto params = GetBookParams(cmd_input)) {
            use_cases_.AddBook(
                domain::AuthorId::FromString(std::move(params->author_id)),
                std::move(params->title),
                params->publication_year,
                std::move(params->tags)
            );
        }
    } catch (const std::exception& e) {
        output_ << "Failed to add book"sv << std::endl;
    }
    return true;
}

bool View::DeleteBook(std::istream& cmd_input) const {
    try {
        if (auto book = ReadBookByTitle(cmd_input)) {
            use_cases_.DeleteBook(domain::BookId::FromString(book->id));
        }
    } catch (const std::exception&) {
        output_ << "Failed to delete book"sv << std::endl;
    }
    return true;
}

bool View::ShowAuthors() const {
    PrintVector(output_, GetAuthors());
    return true;
}

bool View::ShowBooks() const {
    PrintVector(output_, GetBooks());
    return true;
}

bool View::ShowBook(std::istream& cmd_input) const {
    try {
        if (auto book = ReadBookByTitle(cmd_input)) {
            output_ << "Title: " << book->title << std::endl;
            output_ << "Author: " << book->author_name << std::endl;
            output_ << "Publication year: " << book->publication_year
                    << std::endl;
            if (!book->tags.empty()) {
                output_ << "Tags: " << boost::algorithm::join(book->tags, ", ")
                        << std::endl;
            }
        }
    } catch (const std::exception&) {
        throw std::runtime_error("Failed to Show Book");
    }
    return true;
}

bool View::ShowAuthorBooks() const {
    // TODO: handle error
    try {
        if (auto author_id = SelectAuthor()) {
            PrintVector(output_, GetAuthorBooks(*author_id));
        }
    } catch (const std::exception&) {
        throw std::runtime_error("Failed to Show Books");
    }
    return true;
}

std::optional<std::vector<std::string>> View::ReadTags(std::string_view message
) const {
    output_ << message << std::endl;

    std::string raw_tags;
    std::getline(input_, raw_tags);

    if (raw_tags.empty()) {
        return std::nullopt;
    }

    std::vector<std::string> tags;
    boost::split(tags, raw_tags, boost::is_any_of(","));
    std::transform(tags.begin(), tags.end(), tags.begin(), [](const auto& tag) {
        return util::TrimExtraSpaces(tag);
    });
    std::erase_if(tags, [](const auto& tag) { return tag.empty(); });
    std::sort(tags.begin(), tags.end());
    tags.erase(std::unique(tags.begin(), tags.end()), tags.end());

    return tags;
}

std::optional<detail::AddBookParams> View::GetBookParams(std::istream& cmd_input
) const {
    detail::AddBookParams params;

    cmd_input >> params.publication_year;
    std::getline(cmd_input, params.title);
    boost::algorithm::trim(params.title);

    if (params.title.empty()) {
        throw std::runtime_error("Author name cannot be empty");
    }

    auto author_id = ReadAuthor();
    if (!author_id) {
        return std::nullopt;
    }
    params.author_id = std::move(*author_id);

    auto tags = ReadTags();
    if (tags) {
        params.tags = std::move(*tags);
    }

    return params;
}

std::optional<std::string> View::SelectAuthor() const {
    output_ << "Select author:" << std::endl;
    auto authors = GetAuthors();
    PrintVector(output_, authors);
    output_ << "Enter author # or empty line to cancel" << std::endl;

    std::string str;
    if (!std::getline(input_, str) || str.empty()) {
        return std::nullopt;
    }

    int author_idx;
    try {
        author_idx = std::stoi(str);
    } catch (std::exception const&) {
        throw std::runtime_error("Invalid author num");
    }

    --author_idx;
    if (author_idx < 0 or author_idx >= authors.size()) {
        throw std::runtime_error("Invalid author num");
    }

    return authors[author_idx].id;
}

std::vector<detail::AuthorInfo> View::GetAuthors() const {
    std::vector<detail::AuthorInfo> authors;
    for (const auto& author : use_cases_.GetAllAuthors()) {
        authors.emplace_back(author.GetId().ToString(), author.GetName());
    }
    return authors;
}

std::vector<detail::BookInfo> View::GetBooks() const {
    const auto& books = use_cases_.GetAllBooks();
    std::vector<detail::BookInfo> result;
    result.reserve(books.size());

    for (const auto& book : books) {
        const auto& author = use_cases_.GetAuthorById(book.GetAuthorId());
        result.emplace_back(
            book.GetId().ToString(),
            book.GetTitle(),
            book.GetPublicationYear(),
            author.GetName(),
            book.GetTags()
        );
    }

    std::sort(
        result.begin(),
        result.end(),
        [](const auto& lhs, const auto& rhs) {
            return std::tie(lhs.title, lhs.author_name, lhs.publication_year) <
                std::tie(rhs.title, rhs.author_name, rhs.publication_year);
        }
    );

    return result;
}

std::vector<detail::BookInfo> View::GetAuthorBooks(const std::string& id
) const {
    auto author_id = domain::AuthorId::FromString(id);
    const auto& author = use_cases_.GetAuthorById(author_id);
    const auto& books = use_cases_.GetBooksByAuthorId(author_id);

    std::vector<detail::BookInfo> result;
    result.reserve(books.size());

    for (const auto& book : books) {
        result.emplace_back(
            book.GetId().ToString(),
            book.GetTitle(),
            book.GetPublicationYear(),
            author.GetName(),
            book.GetTags()
        );
    }
    return result;
}

std::string View::ReadBookTitle(std::string default_title) const {
    output_ << "Enter new title or empty line to use the current one ("
            << default_title << "):" << std::endl;

    std::string str;
    if (!std::getline(input_, str) || str.empty()) {
        return default_title;
    }

    boost::algorithm::trim(str);
    return str;
}

int View::ReadBookPublicationYear(int default_year) const {
    output_ << "Enter publication year or empty line to use the current one ("
            << default_year << "):" << std::endl;

    std::string str;
    if (!std::getline(input_, str) || str.empty()) {
        return default_year;
    }

    try {
        return std::stoi(str);
    } catch (const std::exception&) {
        throw std::runtime_error("Invalid publication year");
    }
}

std::vector<std::string>
View::ReadBookTags(const std::vector<std::string>& default_tags) const {
    auto tags = ReadTags(
        "Enter tags (current tags: " +
        boost::algorithm::join(default_tags, ", ") + "):"
    );

    if (!tags) {
        return default_tags;
    }
    return *tags;
}

bool View::EditBook(std::istream& cmd_input) const {
    try {
        if (auto book = ReadBookByTitle(cmd_input)) {
            book->title = ReadBookTitle(book->title);
            book->publication_year =
                ReadBookPublicationYear(book->publication_year);
            book->tags = ReadBookTags(book->tags);
            use_cases_.EditBook(
                domain::BookId::FromString(book->id),
                book->title,
                book->publication_year,
                book->tags
            );
        } else {
            output_ << "Book not found"sv << std::endl;
        }
    } catch (const std::exception&) {
        output_ << "Failed to edit book"sv << std::endl;
    }
    return true;
}

}  // namespace ui
