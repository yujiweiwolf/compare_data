#pragma once
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <thread>
#include <memory>
#include <mutex>
#include <x/x.h>
#include "coral/coral.h"
#include "feeder/feeder.h"
#include "coral/wal_reader.h"
#include "config.h"

namespace co {
    using namespace std;

    struct FullDate {
        shared_ptr<map<int64_t, MemQTickBody>> mmap_tick;
        shared_ptr<map<int64_t, MemQOrder>> mmap_order;
        shared_ptr<map<int64_t, MemQKnock>> mmap_knock;
        MemQTickHead contract;
    };

    struct StatisticsData {
        int tick_diff = 0;
        int tick_miss = 0;
        int order_diff = 0;
        int order_miss = 0;
        int knock_diff = 0;
        int knock_miss = 0;
    };

    class CompareOneCode {
    public:
        CompareOneCode();
        ~CompareOneCode();
        void Init();
    private:
        void ReadFile(const string& file, unordered_map<std::string, shared_ptr<FullDate>>& data);
        void ParseQTickFromWal(const string& line, unordered_map<std::string, shared_ptr<FullDate>>& data);
        void ParseQOrderFromWal(const string& line, unordered_map<std::string, shared_ptr<FullDate>>& data);
        void ParseQKnockFromWal(const string& line, unordered_map<std::string, shared_ptr<FullDate>>& data);

        void ParseQContractFromMem(const string& line, unordered_map<std::string, shared_ptr<FullDate>>& data);
        void ParseQTickFromMem(const string& line, unordered_map<std::string, shared_ptr<FullDate>>& data);
        void ParseQOrderFromMem(const string& line, unordered_map<std::string, shared_ptr<FullDate>>& data);
        void ParseQKnockFromMem(const string& line, unordered_map<std::string, shared_ptr<FullDate>>& data);

        void CompareContract(MemQTickHead* right, MemQTickHead* data);
        void CompareTick(MemQTickBody* right, MemQTickBody* data);
        void CompareOrder(MemQOrder* right, MemQOrder* data);
        void CompareKnock(MemQKnock* right, MemQKnock* data);
    private:
        std::vector<std::string> miss_code_;
        std::vector<std::string> diff_code_;
        x::MMapReader feeder_reader_;

        unordered_map<std::string, shared_ptr<FullDate>> right_data_;
        unordered_map<std::string, shared_ptr<FullDate>> new_data_;

        unordered_map<std::string, shared_ptr<StatisticsData>> statistics_data_;
    };
}
