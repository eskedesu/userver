#pragma once

#include <string>

#include <stdexcept>

USERVER_NAMESPACE_BEGIN

namespace storages::etcd {

/// @brief Base class for all etcd client exceptions
class EtcdError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

}

USERVER_NAMESPACE_END
