#pragma once
#include <cstdint>

class InteractionResult {
public:
    enum class Result : int32_t {
        SUCCESS = 1,
        SWING = 2,
    };

public:
    int mResult;

    InteractionResult() = default;
    InteractionResult(Result r) : mResult(static_cast<int32_t>(r)) {}
    InteractionResult(const InteractionResult&) = default;
    InteractionResult& operator=(const InteractionResult&) = default;

    operator Result() const { return static_cast<Result>(mResult); }
};