// Copyright 2022 Fancapital Inc.  All rights reserved.
#include <x/x.h>
#include "./config.h"
#include <boost/filesystem.hpp>
#include "yaml-cpp/yaml.h"

namespace co {
    Config* Config::instance_ = nullptr;

    Config* Config::Instance() {
        static std::once_flag flag;
        std::call_once(flag, [&]() {
            if (instance_ == nullptr) {
                instance_ = new Config();
                instance_->Init();
            }
        });
        return instance_;
    }
    void Config::Init() {
        auto getStr = [&](const YAML::Node& node, const std::string& name) {
            try {
                return node[name] && !node[name].IsNull() ? node[name].as<std::string>() : "";
            } catch (std::exception& e) {
                LOG_ERROR << "load configuration failed: name = " << name << ", error = " << e.what();
                throw std::runtime_error(e.what());
            }
        };
        auto getInt = [&](const YAML::Node& node, const std::string& name, const int64_t& default_value = 0) {
            try {
                return node[name] && !node[name].IsNull() ? node[name].as<int64_t>() : default_value;
            } catch (std::exception& e) {
                LOG_ERROR << "load configuration failed: name = " << name << ", error = " << e.what();
                throw std::runtime_error(e.what());
            }
        };
        auto filename = x::FindFile("feeder.yaml");
        YAML::Node root = YAML::LoadFile(filename);
        opt_ = FeedOptions::Load(filename);

        auto feeder = root["compare_all"];
        right_data_ = getStr(feeder, "right_data");
        new_data_ = getStr(feeder, "new_data");
        compare_instrument_ = getStr(feeder, "compare_instrument");

        stringstream ss;
        ss << "+-------------------- configuration begin --------------------+" << endl;
        ss << opt_->ToString() << endl;
        ss << endl;
        ss << "  right_data: " << right_data_ << endl
           << "  new_data: " << new_data_ << endl
           << "  compare_instrument: " << compare_instrument_ << endl
           ;
        ss << "+-------------------- configuration end   --------------------+";
        LOG_INFO << endl << ss.str();
    }
}  // namespace co
