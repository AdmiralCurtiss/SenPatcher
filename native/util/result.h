#pragma once

#include <cassert>
#include <variant>

namespace HyoutaUtils {
template<typename SuccessT, typename ErrorT>
struct Result {
public:
    Result(SuccessT value) : Value(std::move(value)) {}
    Result(ErrorT value) : Value(std::move(value)) {}
    Result(const Result& other) = default;
    Result(Result&& other) = default;
    Result& operator=(const Result& other) = default;
    Result& operator=(Result&& other) = default;
    ~Result() = default;

    bool IsError() const {
        return std::holds_alternative<ErrorT>(Value);
    }
    bool IsSuccess() const {
        return !IsError();
    }
    explicit operator bool() const {
        return IsSuccess();
    }

    const SuccessT& GetSuccessValue() const {
        assert(IsSuccess());
        return std::get<SuccessT>(Value);
    }
    SuccessT& GetSuccessValue() {
        assert(IsSuccess());
        return std::get<SuccessT>(Value);
    }

    const ErrorT& GetErrorValue() const {
        assert(IsError());
        return std::get<ErrorT>(Value);
    }
    ErrorT& GetErrorValue() {
        assert(IsError());
        return std::get<ErrorT>(Value);
    }

private:
    std::variant<ErrorT, SuccessT> Value;
};
} // namespace HyoutaUtils
