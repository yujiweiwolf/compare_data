#pragma once
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <thread>
#include <memory>
#include <mutex>
#include <x/x.h>
#include <atomic>
#include "coral/coral.h"
#include "feeder/feeder.h"
#include "coral/wal_reader.h"
#include "config.h"

namespace co {
    using namespace std;

    struct FullDate {
        shared_ptr<map<int64_t, MemQTickBody>> mmap_tick;
        shared_ptr<map<int64_t, MemQEtfTick>> mmap_etf;
        shared_ptr<map<string, MemQOrder>> mmap_order;  // key is order_no + "_" + order_type
        shared_ptr<map<int64_t, MemQKnock>> mmap_knock;
        MemQTickHead contract;
    };

    struct StatisticsData {
        int right_tick_num = 0;
        int new_tick_num = 0;
        int tick_diff = 0;
        int etf_diff = 0;
        int tick_miss = 0;
        int order_diff = 0;
        int order_miss = 0;
        int knock_diff = 0;
        int knock_miss = 0;
    };

    class CompareAllCode {
    public:
        CompareAllCode();
        ~CompareAllCode();
        void Init();
    private:
        void ReadWalFile(const string& file, unordered_map<std::string, shared_ptr<FullDate>>& data);
        void ReadMemFile(const string& file, unordered_map<std::string, shared_ptr<FullDate>>& data);

        void CompareContract(MemQTickHead* right, MemQTickHead* data);
        void CompareTick(MemQTickBody* right, MemQTickBody* data);
        void CompareEtf(MemQEtfTick* right, MemQEtfTick* data);
        void CompareOrder(MemQOrder* right, MemQOrder* data);
        void CompareKnock(MemQKnock* right, MemQKnock* data);
        bool IsNeedInstrument(const string& code);


        void CheckSingleCodeData(MemQTickBody* tick, MemQTickHead* contract);
    private:
        std::vector<std::string> miss_code_;
        std::vector<std::string> diff_code_;
        x::MMapReader feeder_reader_;

        unordered_map<std::string, shared_ptr<FullDate>> right_data_;
        unordered_map<std::string, shared_ptr<FullDate>> new_data_;

        unordered_map<std::string, shared_ptr<StatisticsData>> statistics_data_;
        atomic<int64_t> mem_num_;
        atomic<bool> stop_flag_;
        std::shared_ptr<std::thread> monitor_thread_;
        std::set<string> compare_instrument_;
        int64_t compare_date_ = 0;
    };
}
