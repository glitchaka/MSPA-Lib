#pragma once

#include <type_traits>
#include <utility>
#include <variant>

#include "mspa/base/error.hpp"

namespace mspa {

template <typename T>
class Result {
public:
    Result(const T& value) : storage_(value) {}
    Result(T&& value) : storage_(std::move(value)) {}
    Result(const Error& error) : storage_(error) {}
    Result(Error&& error) : storage_(std::move(error)) {}

    [[nodiscard]] bool has_value() const noexcept {
        return std::holds_alternative<T>(storage_);
    }

    [[nodiscard]] explicit operator bool() const noexcept {
        return has_value();
    }

    [[nodiscard]] T* value_if() noexcept {
        return std::get_if<T>(&storage_);
    }

    [[nodiscard]] const T* value_if() const noexcept {
        return std::get_if<T>(&storage_);
    }

    [[nodiscard]] Error* error_if() noexcept {
        return std::get_if<Error>(&storage_);
    }

    [[nodiscard]] const Error* error_if() const noexcept {
        return std::get_if<Error>(&storage_);
    }

private:
    std::variant<T, Error> storage_;
};

template <>
class Result<void> {
public:
    Result() = default;
    Result(const Error& error) : error_(error), ok_(false) {}
    Result(Error&& error) : error_(std::move(error)), ok_(false) {}

    [[nodiscard]] bool has_value() const noexcept { return ok_; }
    [[nodiscard]] explicit operator bool() const noexcept { return ok_; }

    [[nodiscard]] const Error* error_if() const noexcept {
        return ok_ ? nullptr : &error_;
    }

private:
    Error error_{};
    bool ok_{true};
};

inline Error make_error(ErrorCode code, std::string details = {}) {
    return Error{code, std::move(details)};
}

} // namespace mspa
