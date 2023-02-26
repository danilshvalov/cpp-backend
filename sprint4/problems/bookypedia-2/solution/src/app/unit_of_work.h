#pragma once

#include <memory>

#include "../domain/author_fwd.h"
#include "../domain/book_fwd.h"

namespace app {

struct UnitOfWork {
    virtual void Commit() = 0;
    virtual domain::AuthorRepository& Authors() = 0;
    virtual domain::BookRepository& Books() = 0;
    virtual ~UnitOfWork() = default;
};

using UnitOfWorkHolder = std::unique_ptr<UnitOfWork>;

class UnitOfWorkFactory {
  public:
    virtual UnitOfWorkHolder CreateUnitOfWork() = 0;

  protected:
    ~UnitOfWorkFactory() = default;
};

}  // namespace app
