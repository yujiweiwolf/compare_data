// Copyright 2022 Fancapital Inc.  All rights reserved.
#pragma once
#include <vector>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <mutex>
#include "feeder/feeder.h"

namespace co {
class Config {
 public:
    static Config* Instance();

    inline std::shared_ptr<FeedOptions> opt() {
        return opt_;
    }
    string right_data() {
        return right_data_;
    }
    string new_data() {
        return new_data_;
    }
    string compare_instrument() {
        return compare_instrument_;
    }

protected:
    Config() = default;
    ~Config() = default;
    Config(const Config&) = delete;
    const Config& operator=(const Config&) = delete;

    void Init();

 private:
    static Config* instance_;
    std::shared_ptr<FeedOptions> opt_;
    string right_data_;
    string new_data_;
    string compare_instrument_;
};
}  // namespace co
