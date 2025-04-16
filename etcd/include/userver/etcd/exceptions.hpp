#pragma once

#include <string>

#include <stdexcept>

USERVER_NAMESPACE_BEGIN

namespace etcd {

/// @brief Base class for all etcd client exceptions
class EtcdError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

/// @brief Etcd request error
class EtcdRequestError : public EtcdError {
public:
    using EtcdError::EtcdError;
};

}  // namespace etcd

USERVER_NAMESPACE_END
