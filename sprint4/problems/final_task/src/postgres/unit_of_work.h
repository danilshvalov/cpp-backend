#pragma once

#include "postgres/player_record.h"
#include "postgres/connection_pool.h"
#include "app/unit_of_work.h"

namespace postgres {

class UnitOfWorkImpl : public app::UnitOfWork {
  public:
    explicit UnitOfWorkImpl(ConnectionPool::ConnectionWrapper&& connection) :
        connection_(std::move(connection)),
        work_(*connection_),
        player_records_(work_) {}

    void Commit() override {
        work_.commit();
    }

    app::PlayerRecordRepository& PlayerRecords() override {
        return player_records_;
    }

  private:
    ConnectionPool::ConnectionWrapper connection_;
    pqxx::work work_;
    PlayerRecordRepositoryImpl player_records_;
};

class UnitOfWorkFactoryImpl : public app::UnitOfWorkFactory {
  public:
    explicit UnitOfWorkFactoryImpl(ConnectionPool& connection_pool) :
        connection_pool_(connection_pool) {}

    app::UnitOfWorkHolder CreateUnitOfWork() override {
        return std::make_unique<UnitOfWorkImpl>(connection_pool_.GetConnection()
        );
    }

  private:
    ConnectionPool& connection_pool_;
};
} // namespace postgres
