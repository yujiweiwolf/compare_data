
#include <regex>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include "compare_all.h"
namespace po = boost::program_options;

#define WUCAI 0.000001

namespace co {
    CompareAllCode::CompareAllCode() {
    }

    CompareAllCode::~CompareAllCode() {
    }

    bool CompareAllCode::IsNeedInstrument(const string& code) {
        bool flag = true;
        if (!compare_instrument_.empty()) {
            if (auto it = compare_instrument_.find(code); it == compare_instrument_.end()) {
                flag = false;
            }
        }
        return flag;
    }

    void CompareAllCode::ReadWalFile(const string& file, unordered_map<std::string, shared_ptr<FullDate>>& data) {
        if (file.empty()) {
            return;
        }
        if (!boost::filesystem::exists(file)) {
            LOG_ERROR << "not exit file: " << file;
            return;
        }
        co::WALReader reader;
        reader.Open(file.c_str());
        while (true) {
            std::string raw;
            int64_t type = reader.Read(&raw);
            if (raw.empty()) {
                break;
            }
            switch (type) {
                case kFBPrefixQTick: {
                    shared_ptr<FullDate> full_data;
                    auto q = flatbuffers::GetRoot<co::fbs::QTick>(raw.data());
                    std::string std_code = q->code() ? q->code()->str() : "";
                    if (IsNeedInstrument(std_code)) {
                        std::string name = q->name() ? q->name()->str() : "";
                        std::string underlying_code = q->underlying_code() ? q->underlying_code()->str() : "";
                        auto it = data.find(std_code);
                        if (it == data.end()) {
                            full_data = std::make_shared<FullDate>();
                            memset(&full_data->contract, 0, sizeof(MemQContract));
                            strncpy(full_data->contract.code, std_code.c_str(), std_code.length());
                            full_data->contract.timestamp = q->timestamp();
                            full_data->contract.market = q->market();
                            full_data->contract.dtype = q->dtype();
                            full_data->contract.pre_close = q->pre_close();
                            full_data->contract.upper_limit = q->upper_limit() >= 900000 ? 0 : q->upper_limit();
                            full_data->contract.lower_limit = q->lower_limit() < 0.1 ? 0 : q->lower_limit();
                            full_data->contract.pre_settle = q->pre_settle();
                            full_data->contract.pre_open_interest = q->pre_open_interest();
                            strncpy(full_data->contract.underlying_code, underlying_code.c_str(), underlying_code.length());
                            full_data->contract.multiple = q->multiple();
                            full_data->contract.price_step = q->price_step();
                            full_data->contract.list_date = q->list_date();
                            full_data->contract.expire_date = q->expire_date();
                            full_data->contract.exercise_price = q->exercise_price();
                            full_data->contract.cp_flag = q->cp_flag();
                            full_data->contract.volume_unit = 0;

                            full_data->mmap_tick = std::make_shared<map<int64_t, MemQTick>>();
                            full_data->mmap_order = std::make_shared<map<int64_t, MemQOrder>>();
                            full_data->mmap_knock = std::make_shared<map<int64_t, MemQKnock>>();
                            data.insert(std::make_pair(std_code, full_data));
                        } else {
                            full_data = it->second;
                        }
                        int64_t timestamp = q->timestamp();
                        bool valid_flag = true;
                        int stamp = timestamp % 1000000000LL;
                        if (stamp < 91500000) {
                            valid_flag = false;
                        }
                        if (stamp > 1131000 && stamp < 1300000) {
                            valid_flag = false;
                        }
                        if (stamp > 150100000) {
                            valid_flag = false;
                        }
                        if (valid_flag && q->src() != 2) {
                            MemQTick tick;
                            memset(&tick, 0, sizeof(tick));
                            strncpy(tick.code, std_code.c_str(), std_code.length());
                            tick.timestamp = q->timestamp();
                            auto bps = q->bp();
                            auto bvs = q->bv();
                            auto aps = q->ap();
                            auto avs = q->av();

                            for (size_t j = 0; j < 10 && bps && bvs && j < bps->size() && j < bvs->size(); ++j) {
                                double bp = bps->Get(j);
                                int64_t bv = bvs->Get(j);
                                tick.bp[j] = bp;
                                tick.bv[j] = bv;
                            }
                            for (size_t j = 0; j < 10 && aps && avs && j < aps->size() && j < avs->size(); ++j) {
                                double ap = aps->Get(j);
                                int64_t av = avs->Get(j);
                                tick.ap[j] = ap;
                                tick.av[j] = av;
                            }
                            tick.new_price = q->new_price();
                            tick.new_volume = q->new_volume();
                            tick.new_amount = q->new_amount();
                            tick.sum_volume = q->sum_volume();
                            tick.sum_amount = q->sum_amount();
                            tick.new_bid_volume = q->new_bid_volume();
                            tick.new_bid_amount = q->new_bid_amount();
                            tick.new_ask_volume = q->new_ask_volume();
                            tick.new_ask_amount = q->new_ask_amount();
                            tick.state = q->status();
                            tick.open = q->open();
                            tick.high = q->high();
                            tick.low = q->low();
                            tick.close = q->close();
                            tick.settle = q->settle();
                            tick.open_interest = q->open_interest();
                            CheckSingleCodeData(&tick, &full_data->contract);
                            full_data->mmap_tick->insert(std::make_pair(timestamp, tick));
                        }
                    }
                    break;
                }
                case kFBPrefixQOrder: {
                    auto q = flatbuffers::GetRoot<co::fbs::QOrder>(raw.data());
                    string code = q->code() ? q->code()->str() : "";
                    int64_t order_no = q->order_no();
                    string std_code = q->code() ? q->code()->str() : "";
                    auto it = data.find(std_code);
                    if (it != data.end()) {
                        auto full_data = it->second;
                        MemQOrder order;
                        memset(&order, 0, sizeof(order));
                        strncpy(order.code, std_code.c_str(), std_code.length());
                        order.timestamp = q->timestamp();
                        order.order_no = order_no;
                        order.bs_flag = q->bs_flag();
                        order.order_type = q->order_type();
                        order.order_price = q->order_price();
                        order.order_volume = q->order_volume();
                        full_data->mmap_order->insert(std::make_pair(order_no, order));
                    }
                    break;
                }
                case kFBPrefixQKnock: {
                    auto q = flatbuffers::GetRoot<co::fbs::QKnock>(raw.data());
                    int64_t match_no = q->match_no();
                    string std_code = q->code() ? q->code()->str() : "";
                    auto it = data.find(std_code);
                    if (it != data.end()) {
                        auto full_data = it->second;
                        MemQKnock knock;
                        memset(&knock, 0, sizeof(knock));
                        strncpy(knock.code, std_code.c_str(), std_code.length());
                        knock.timestamp = q->timestamp();
                        knock.match_no = match_no;
                        knock.bid_order_no = q->bid_order_no();
                        knock.ask_order_no = q->ask_order_no();
                        knock.match_price = q->match_price();
                        knock.match_volume = q->match_volume();
                        full_data->mmap_knock->insert(std::make_pair(match_no, knock));
                    }
                    break;
                }
            }
        }

        LOG_INFO << "read wal file: " << file;
        for (auto it = data.begin(); it != data.end(); ++it) {
            LOG_INFO << "code: " << it->first << ", static pre_close: " << it->second->contract.pre_close
                     << ", upper_limit: " << it->second->contract.upper_limit
                     << ", lower_limit: " << it->second->contract.lower_limit
                     << ", tick num: " << it->second->mmap_tick->size()
                     << ", order num: " << it->second->mmap_order->size()
                     << ", knock num: " << it->second->mmap_knock->size();
            int64_t sum_volume = 0;
            int64_t sum_amout = 0;
            for (auto& itor : *it->second->mmap_knock) {
                sum_volume += itor.second.match_volume;
                sum_amout += itor.second.match_price * itor.second.match_volume;
            }
            if (sum_volume > 0 && sum_amout > 0) {
                for (auto& itor : *it->second->mmap_tick) {
                    int64_t stamp = itor.second.timestamp % 1000000000LL;
                    if (stamp > 150000000) {
                        MemQTick& tick = itor.second;
                        if ((tick.sum_volume != sum_volume) ||  fabs(tick.sum_amount - i2f(sum_amout)) > 100) {
                            LOG_ERROR << "code: " << tick.code << ", timestamp: " << tick.timestamp
                                      << ", sum_volume: " << tick.sum_volume
                                      << ", sum_amount: " << tick.sum_amount
                                      << ", 逐笔成交累计volume: " << sum_volume
                                      << ", 逐笔成交累计amount: " << i2f(sum_amout);
                        }
                        break;
                    }
                }
            }
        }
    }

    void CompareAllCode::ReadMemFile(const string& dir, unordered_map<std::string, shared_ptr<FullDate>>& recode) {
        x::MMapReader feeder_reader_;
        feeder_reader_.Open(dir, "meta");
        feeder_reader_.Open(dir, "data");
        shared_ptr<FullDate> full_data;
        const void* data = nullptr;
        while (true) {
            int32_t type = feeder_reader_.Next(&data);
            if (type == kMemTypeQContract) {
                MemQContract *contract = (MemQContract *) data;
                string std_code = contract->code;
                if (IsNeedInstrument(std_code)) {
                    auto it = recode.find(std_code);
                    if (it == recode.end()) {
                        full_data = std::make_shared<FullDate>();
                        memset(&full_data->contract, 0, sizeof(full_data->contract));
                        memcpy(&full_data->contract, contract, sizeof(MemQContract));
                        full_data->mmap_tick = std::make_shared<map<int64_t, MemQTick>>();
                        full_data->mmap_order = std::make_shared<map<int64_t, MemQOrder>>();
                        full_data->mmap_knock = std::make_shared<map<int64_t, MemQKnock>>();
                        recode.insert(std::make_pair(std_code, full_data));
                    } else {
                        full_data = it->second;
                        memcpy(&full_data->contract, contract, sizeof(MemQContract));
                        if (full_data->contract.upper_limit > 900000) {
                            full_data->contract.upper_limit = 0;
                        }
                        if (full_data->contract.lower_limit < 0.1) {
                            full_data->contract.lower_limit = 0;
                        }
                    }
                }
            } else if (type == kMemTypeQTick) {
                MemQTick *tick = (MemQTick *) data;
                string std_code = tick->code;
                auto it = recode.find(std_code);
                if (it == recode.end()) {
                    continue;
                } else {
                    full_data = it->second;
                }
                int64_t timestamp = tick->timestamp;
                int stamp = timestamp % 1000000000LL;
                bool valid_flag = true;
                if (stamp < 91500000) {
                    valid_flag = false;
                }
                if (stamp > 1131000 && stamp < 1300000) {
                    valid_flag = false;
                }
                if (stamp > 150100000) {
                    valid_flag = false;
                }
                if (valid_flag && tick->src != 2) {
                    MemQTick mem_tick;
                    memcpy(&mem_tick, tick, sizeof(MemQTick));
                    if (full_data->contract.volume_unit > 0) {
                        for (int i = 0; i < 10; i++) {
                            mem_tick.av[i] /= full_data->contract.volume_unit;
                            mem_tick.bv[i] /= full_data->contract.volume_unit;
                        }
                        mem_tick.sum_volume /= full_data->contract.volume_unit;
                        mem_tick.new_volume /= full_data->contract.volume_unit;
                    }
                    CheckSingleCodeData(&mem_tick, &full_data->contract);
                    full_data->mmap_tick->insert(std::make_pair(timestamp, mem_tick));
                }
            } else if (type == kMemTypeQOrder) {
                MemQOrder *order = (MemQOrder *) data;
                string std_code = order->code;
                auto it = recode.find(std_code);
                if (it != recode.end()) {
//                    if (strcmp(order->code, "600848.SH") == 0) {
//                        LOG_INFO << "From mem, QOrder{timestamp: " << order->timestamp
//                                 << ", code: " << order->code
//                                 << ", order_no: " << order->order_no
//                                 << ", bs_flag: " << (int)order->bs_flag
//                                 << ", order_type: " << (int)order->order_type
//                                 << ", order_price: " << order->order_price
//                                 << ", order_volume: " << order->order_volume
//                                 << "}";
//                    }
                    full_data = it->second;
                    int64_t order_no = order->order_no;
                    MemQOrder mem_order;
                    memcpy(&mem_order, order, sizeof(MemQOrder));
                    if (full_data->contract.volume_unit > 0) {
                        mem_order.order_volume /= full_data->contract.volume_unit;
                    }
                    full_data->mmap_order->insert(std::make_pair(order_no, mem_order));
                }
            } else if (type == kMemTypeQKnock) {
                MemQKnock *knock = (MemQKnock *) data;
//                if (strcmp(knock->code, "600848.SH") == 0) {
//                    LOG_INFO << "QKnock{timestamp: " << knock->timestamp
//                              << ", code: " << knock->code
//                              << ", match_no: " << knock->match_no
//                              << ", bid_order_no: " << knock->bid_order_no
//                              << ", ask_order_no: " << knock->ask_order_no
//                              << ", match_price: " << knock->match_price
//                              << ", match_volume: " << knock->match_volume
//                              << "}";
//                }
                string std_code = knock->code;
                auto it = recode.find(std_code);
                if (it != recode.end()) {
                    full_data = it->second;
                    int64_t match_no = knock->match_no;
                    MemQKnock mem_knock;
                    memcpy(&mem_knock, knock, sizeof(MemQKnock));
                    if (full_data->contract.volume_unit > 0) {
                        mem_knock.match_volume /= full_data->contract.volume_unit;
                    }
                    full_data->mmap_knock->insert(std::make_pair(match_no, mem_knock));
                }
            } else if (type == 0) {
                LOG_INFO << "stop read mmap file: " << dir;
                break;
            }
        }

        for (auto it = recode.begin(); it != recode.end(); ++it) {
            LOG_INFO << "code: " << it->first << ", static pre_close: " << it->second->contract.pre_close
                     << ", upper_limit: " << it->second->contract.upper_limit
                     << ", lower_limit: " << it->second->contract.lower_limit
                     << ", tick num: " << it->second->mmap_tick->size()
                     << ", order num: " << it->second->mmap_order->size()
                     << ", knock num: " << it->second->mmap_knock->size();
            int64_t sum_volume = 0;
            int64_t sum_amout = 0;
            for (auto& itor : *it->second->mmap_knock) {
                sum_volume += itor.second.match_volume;
                sum_amout += itor.second.match_price * itor.second.match_volume;
            }
            if (sum_volume > 0 && sum_amout > 0) {
                for (auto& itor : *it->second->mmap_tick) {
                    int64_t stamp = itor.second.timestamp % 1000000000LL;
                    if (stamp > 150000000) {
                        MemQTick& tick = itor.second;
                        if ((tick.sum_volume != sum_volume) ||  fabs(tick.sum_amount - i2f(sum_amout)) > 100) {
                            LOG_ERROR << "code: " << tick.code << ", timestamp: " << tick.timestamp
                                      << ", sum_volume: " << tick.sum_volume
                                      << ", sum_amount: " << tick.sum_amount
                                      << ", 逐笔成交累计volume: " << sum_volume
                                      << ", 逐笔成交累计amount: " << i2f(sum_amout);
                        }
                        break;
                    }
                }
            }
        }
    }

    void CompareAllCode::Init() {
        string need_instrument = Config::Instance()->compare_instrument();
        std::vector<string> temp;
        x::Split(&temp, need_instrument, "|");
        for (auto& it : temp) {
            compare_instrument_.insert(it);
        }
        {
            string right_file = Config::Instance()->right_data();
            size_t pos = right_file.find("wal");
            if (pos != right_file.npos) {
                ReadWalFile(right_file, right_data_);
            } else {
                ReadMemFile(right_file, right_data_);
            }
        }

        {
            string new_file = Config::Instance()->new_data();
            size_t pos = new_file.find("wal");
            if (pos != new_file.npos) {
                ReadWalFile(new_file, new_data_);
            } else {
                ReadMemFile(new_file, new_data_);
            }
        }

        for (auto& it : compare_instrument_) {
            if (auto itor = right_data_.find(it); itor == right_data_.end()) {
                LOG_ERROR << "base file " << Config::Instance()->right_data() << " not have code: " << it;
            }
            if (auto itor = new_data_.find(it); itor == new_data_.end()) {
                LOG_ERROR << "new file " << Config::Instance()->new_data() << " not have code: " << it;
            }
        }
        for (auto it = right_data_.begin(); it != right_data_.end(); ++it) {
            auto itor = new_data_.find(it->first);
            if (itor == new_data_.end()) {
                miss_code_.push_back(it->first);
                continue;
            }
            auto& right_contract = it->second->contract;
            auto right_tick = it->second->mmap_tick;
            auto right_order = it->second->mmap_order;
            auto right_knock = it->second->mmap_knock;

            auto& new_contract = itor->second->contract;
            auto new_tick = itor->second->mmap_tick;
            auto new_order = itor->second->mmap_order;
            auto new_knock = itor->second->mmap_knock;
            shared_ptr<StatisticsData> statistics;
            auto __it = statistics_data_.find(it->first);
            if (__it == statistics_data_.end()) {
                statistics = std::make_shared<StatisticsData>();
                statistics_data_.insert(std::make_pair(it->first, statistics));
            } else {
                statistics = __it->second;
            }
            statistics->right_tick_num = right_tick->size();
            statistics->new_tick_num = new_tick->size();
            CompareContract(&right_contract, &new_contract);
            // 比较 Tick
            {
                for (auto iter = right_tick->begin(); iter != right_tick->end(); ++iter) {
                    auto _it = new_tick->find(iter->first);
                    if (_it == new_tick->end()) {
                        statistics->tick_miss ++;
                        if (statistics->order_miss <= 10) {
                            MemQTick& tick = iter->second;
                            LOG_TRACE << "miss tick, code: " << tick.code << ", timestamp: " << tick.timestamp;
                        }
                    } else {
                        MemQTick& right_tick = iter->second;
                        MemQTick& new_tick = _it->second;
                        CompareTick(&right_tick, &new_tick);
                    }
                }
            }
            // 比较 Order
            {
//                if (it->first.compare("600848.SH") == 0) {
//                    int a = 0;
//                    for (auto iter = right_order->begin(); iter != right_order->end(); ++iter) {
//                        auto order = &iter->second;
//                        LOG_INFO << "Right QOrder{timestamp: " << order->timestamp
//                                  << ", code: " << order->code
//                                  << ", order_no: " << order->order_no
//                                  << ", bs_flag: " << (int)order->bs_flag
//                                  << ", order_type: " << (int)order->order_type
//                                  << ", order_price: " << order->order_price
//                                  << ", order_volume: " << order->order_volume
//                                  << "}";
//                    }
//                    for (auto iter = new_order->begin(); iter != new_order->end(); ++iter) {
//                        auto order = &iter->second;
//                        LOG_INFO << "New QOrder{timestamp: " << order->timestamp
//                                 << ", code: " << order->code
//                                 << ", order_no: " << order->order_no
//                                 << ", bs_flag: " << (int)order->bs_flag
//                                 << ", order_type: " << (int)order->order_type
//                                 << ", order_price: " << order->order_price
//                                 << ", order_volume: " << order->order_volume
//                                 << "}";
//                    }
//                }
                for (auto iter = right_order->begin(); iter != right_order->end(); ++iter) {
                    auto _it = new_order->find(iter->first);
                    if (_it == new_order->end()) {
                        shared_ptr<StatisticsData> statistics;
                        auto __it = statistics_data_.find(it->first);
                        if (__it == statistics_data_.end()) {
                            statistics = std::make_shared<StatisticsData>();
                            statistics_data_.insert(std::make_pair(it->first, statistics));
                        } else {
                            statistics = __it->second;
                        }
                        statistics->order_miss ++;
                        if (statistics->order_miss <= 10) {
                            MemQOrder& order = iter->second;
                            LOG_TRACE << "miss order, code: " << order.code << ", timestamp: " << order.timestamp << ", order_no: " << order.order_no;
                        }
                    } else {
                        MemQOrder& right_order = iter->second;
                        MemQOrder& new_order = _it->second;
                        CompareOrder(&right_order, &new_order);
                    }
                }
            }
            // 比较 Knock
            {
                for (auto iter = right_knock->begin(); iter != right_knock->end(); ++iter) {
                    auto _it = new_knock->find(iter->first);
                    if (_it == new_knock->end()) {
                        shared_ptr<StatisticsData> statistics;
                        auto __it = statistics_data_.find(it->first);
                        if (__it == statistics_data_.end()) {
                            statistics = std::make_shared<StatisticsData>();
                            statistics_data_.insert(std::make_pair(it->first, statistics));
                        } else {
                            statistics = __it->second;
                        }
                        statistics->knock_miss ++;
                        if (statistics->knock_miss <= 10) {
                            MemQKnock& knock = iter->second;
                            LOG_TRACE << "miss knock, code: " << knock.code << ", timestamp: " << knock.timestamp << ", match_no: " << knock.match_no;
                        }
                    } else {
                        MemQKnock& right_knock = iter->second;
                        MemQKnock& new_knock = _it->second;
                        CompareKnock(&right_knock, &new_knock);
                    }
                }
            }
        }
        for (auto it = statistics_data_.begin(); it != statistics_data_.end(); ++it) {
            LOG_INFO << "statistics, code: " << it->first
                    << ", right_tick_num: " << it->second->right_tick_num
                    << ", new_tick_num: " << it->second->new_tick_num
                    << ", tick_diff: " << it->second->tick_diff
                    << ", tick_miss: " << it->second->tick_miss
                    << ", order_diff: " << it->second->order_diff
                    << ", order_miss: " << it->second->order_miss
                    << ", knock_diff: " << it->second->knock_diff
                    << ", knock_miss: " << it->second->knock_miss;
        }

        stringstream ss;
        string pre_error = "miss code: ";
        ss << pre_error;
        for (auto& it : miss_code_) {
            ss << ", " << it;
        }
        if (ss.str().length() > pre_error.length()) {
            LOG_ERROR << ss.str();
        }
    }

    void CompareAllCode::CompareContract(MemQContract* right, MemQContract* data) {
        stringstream ss;
        ss << "Contract code: " << right->code;
        if (strcmp(right->underlying_code, data->underlying_code) != 0) {
            ss << ", underlying_code, right: " << right->underlying_code << ", new: " << data->underlying_code;
        }
        if (fabs(right->pre_close - data->pre_close) > WUCAI) {
            ss << ", pre_close, right: " << right->pre_close << ", new: " << data->pre_close;
        }
        if (fabs(right->upper_limit - data->upper_limit) > WUCAI) {
            ss << ", upper_limit, right: " << right->upper_limit << ", new: " << data->upper_limit;
        }
        if (fabs(right->lower_limit - data->lower_limit) > WUCAI) {
            ss << ", lower_limit, right: " << right->lower_limit << ", new: " << data->lower_limit;
        }
        if (fabs(right->pre_settle - data->pre_settle) > WUCAI) {
            ss << ", pre_settle, right: " << right->pre_settle << ", new: " << data->pre_settle;
        }
        if (right->pre_open_interest != data->pre_open_interest) {
            ss << ", pre_open_interest, right: " << right->pre_open_interest << ", new: " << data->pre_open_interest;
        }
        if (right->multiple != data->multiple) {
            ss << ", multiple, right: " << right->multiple << ", new: " << data->multiple;
        }
        if (fabs(right->price_step - data->price_step) > WUCAI) {
            ss << ", price_step, right: " << right->price_step << ", new: " << data->price_step;
        }
        if (right->market != data->market) {
            ss << ", market, right: " << right->market << ", new: " << data->market;
        }
        if (right->dtype != data->dtype) {
            ss << ", dtype, right: " << right->dtype << ", new: " << data->dtype;
        }
        if (right->cp_flag != data->cp_flag) {
            ss << ", cp_flag, right: " << right->cp_flag << ", new: " << data->cp_flag;
        }
        if (right->volume_unit != data->volume_unit) {
            ss << ", volume_unit, right: " << right->volume_unit << ", new: " << data->volume_unit;
        }
        string err_msg = ss.str();
        if (err_msg.find("right") != err_msg.npos) {
            LOG_ERROR << ss.str();
        }
    }

    void CompareAllCode::CompareTick(MemQTick* right, MemQTick* data) {
        stringstream ss;
        ss << "tick code: " << right->code << ", timestamp: " << right->timestamp;
        for (int i = 0; i < 10; i++) {
            if (fabs(right->bp[i] - data->bp[i]) > WUCAI) {
                ss << ", right bp[" << i << "]: " << right->bp[i] << ", new bp[" << i << "]: " << data->bp[i];
            }
            if (right->bv[i] != data->bv[i]) {
                ss << ", right bv[" << i << "]: " << right->bv[i] << ", new bv[" << i << "]: " << data->bv[i];
            }
            if (fabs(right->ap[i] - data->ap[i]) > WUCAI) {
                ss << ", right ap[" << i << "]: " << right->ap[i] << ", new ap[" << i << "]: " << data->ap[i];
            }
            if (right->av[i] != data->av[i]) {
                ss << ", right av[" << i << "]: " << right->av[i] << ", new av[" << i << "]: " << data->av[i];
            }
        }
        if (fabs(right->new_price - data->new_price) > WUCAI) {
            ss << ", right new_price: " << right->new_price << ", new new_price: " << data->new_price;
        }
        if (right->new_volume != data->new_volume) {
            ss << ", right new_volume: " << right->new_volume << ", new new_volume: " << data->new_volume;
        }
        if (fabs(right->new_amount - data->new_amount) > 10.0) {
            ss << ", right new_amount: " << right->new_amount << ", new new_amount: " << data->new_amount;
        }
        if (right->sum_volume != data->sum_volume) {
            ss << ", right sum_volume: " << right->sum_volume << ", new sum_volume: " << data->sum_volume;
        }
        if (fabs(right->sum_amount - data->sum_amount) > 10.0) {
            ss << ", right sum_amount: " << right->sum_amount << ", new sum_amount: " << data->sum_amount;
        }
        if (right->open_interest != data->open_interest) {
            ss << ", right open_interest: " << right->open_interest << ", new open_interest: " << data->open_interest;
        }
        if (fabs(right->open - data->open) > WUCAI) {
            ss << ", right open: " << right->open << ", new open: " << data->open;
        }
        if (fabs(right->close - data->close) > WUCAI) {
            ss << ", right close: " << right->close << ", new close: " << data->close;
        }
        if (fabs(right->settle - data->settle) > WUCAI) {
            ss << ", right settle: " << right->settle << ", new settle: " << data->settle;
        }
        if (right->state != data->state) {
            ss << ", right state: " << right->state << ", new state: " << data->state;
        }
        string err_msg = ss.str();
        if (err_msg.find("right") != err_msg.npos) {
            string code = right->code;
            shared_ptr<StatisticsData> statistics;
            auto it = statistics_data_.find(code);
            if (it == statistics_data_.end()) {
                statistics = std::make_shared<StatisticsData>();
                statistics_data_.insert(std::make_pair(code, statistics));
            } else {
                statistics = it->second;
            }
            if (statistics->tick_diff <= 10) {
                LOG_ERROR << ss.str();
            }
            statistics->tick_diff ++;
        }
    }

    void CompareAllCode::CompareOrder(MemQOrder* right, MemQOrder* data) {
        stringstream ss;
        ss << "order code: " << right->code << ", timestamp: " << right->timestamp;
        if (right->order_no != data->order_no) {
            ss << ", right order_no: " << right->order_no << ", new order_no: " << data->order_no;
        }

        if (right->order_type == 0 && right->order_price != data->order_price) {
            ss << ", right order_price: " << right->order_price << ", new order_price: " << data->order_price;
        }

        if (right->order_volume != data->order_volume) {
            ss << ", right order_volume: " << right->order_volume << ", new order_volume: " << data->order_volume;
        }

        if (right->bs_flag != data->bs_flag) {
            ss << ", right bs_flag: " << right->bs_flag << ", new bs_flag: " << data->bs_flag;
        }
//        if (strcmp(right->code, "600848.SH") == 0) {
//            LOG_INFO << "right QOrder{timestamp: " << right->timestamp
//                     << ", code: " << right->code
//                     << ", order_no: " << right->order_no
//                     << ", bs_flag: " << (int)right->bs_flag
//                     << ", order_type: " << (int)right->order_type
//                     << ", order_price: " << right->order_price
//                     << ", order_volume: " << right->order_volume
//                     << "}";
//        }
//        if (strcmp(data->code, "600848.SH") == 0) {
//            LOG_INFO << "new QOrder{timestamp: " << data->timestamp
//                     << ", code: " << data->code
//                     << ", order_no: " << data->order_no
//                     << ", bs_flag: " << (int)data->bs_flag
//                     << ", order_type: " << (int)data->order_type
//                     << ", order_price: " << data->order_price
//                     << ", order_volume: " << data->order_volume
//                     << "}";
//        }
        if (right->order_type != data->order_type) {
            ss << ", right order_type: " << right->order_type << ", new order_type: " << data->order_type;
        }

        string err_msg = ss.str();
        if (err_msg.find("right") != err_msg.npos) {
            string code = right->code;
            shared_ptr<StatisticsData> statistics;
            auto it = statistics_data_.find(code);
            if (it == statistics_data_.end()) {
                statistics = std::make_shared<StatisticsData>();
                statistics_data_.insert(std::make_pair(code, statistics));
            } else {
                statistics = it->second;
            }
            if (statistics->order_diff <= 10) {
                LOG_ERROR << ss.str();
            }
            statistics->order_diff ++;
        }
    }

    void CompareAllCode::CompareKnock(MemQKnock* right, MemQKnock* data) {
        stringstream ss;
        ss << "knock code: " << right->code << ", timestamp: " << right->timestamp << ", timestamp: " << data->timestamp;
        if (right->match_no != data->match_no) {
            ss << ", right match_no: " << right->match_no << ", new match_no: " << data->match_no;
        }

        if (right->bid_order_no != data->bid_order_no) {
            ss << ", right bid_order_no: " << right->bid_order_no << ", new bid_order_no: " << data->bid_order_no;
        }

        if (right->ask_order_no != data->ask_order_no) {
            ss << ", right ask_order_no: " << right->ask_order_no << ", new ask_order_no: " << data->ask_order_no;
        }

        if (right->match_price != data->match_price) {
            ss << ", right match_price: " << right->match_price << ", new match_price: " << data->match_price;
        }

        if (right->match_volume != data->match_volume) {
            ss << ", right match_volume: " << right->match_volume << ", new match_volume: " << data->match_volume;
        }
        string err_msg = ss.str();
        if (err_msg.find("right") != err_msg.npos) {
            string code = right->code;
            shared_ptr<StatisticsData> statistics;
            auto it = statistics_data_.find(code);
            if (it == statistics_data_.end()) {
                statistics = std::make_shared<StatisticsData>();
                statistics_data_.insert(std::make_pair(code, statistics));
            } else {
                statistics = it->second;
            }
            if (statistics->knock_diff <= 10) {
                LOG_ERROR << ss.str();
            }
            statistics->knock_diff ++;
        }
    }

    // 检查盘口
    void CompareAllCode::CheckSingleCodeData(MemQTick* tick, MemQContract* contract) {
//        // 测试用
//        if (tick->timestamp == 20240619094926000) {
//            tick->bp[0] = 9.61;
//            tick->bp[1] = 8.61;
//            tick->bv[3] = 0;
//            tick->ap[1] = 8.67;
//            tick->av[3] = 0;
//        }
        if (tick->ap[0] > 0 && tick->bp[0] > 0 && (tick->bp[0] - tick->ap[0]) > WUCAI) {
            LOG_ERROR << "检查盘口, code: " << tick->code << ", timestamp: " << tick->timestamp
                      << ", 买1价大于卖一价 "
                      << ", ap[0]: " << tick->ap[0]  << ", bp[0]: " << tick->bp[0];
        }
        for (int i = 0; i < 8; i++) {
            if (tick->ap[i] > 0 && (tick->ap[i] - contract->upper_limit) > WUCAI) {
                LOG_ERROR << "检查盘口, code: " << tick->code << ", timestamp: " << tick->timestamp
                                << ", 卖价大于涨停价，i: " << i
                                << ", ap[i]: " << tick->ap[i] << ", upper_limit: " << contract->upper_limit;
            }
            if (tick->av[i] < 0 || tick->av[i + 1] < 0 || tick->ap[i] < 0 || tick->ap[i + 1] < 0) {
                LOG_ERROR << "检查盘口, code: " << tick->code << ", timestamp: " << tick->timestamp
                          << ", 卖盘口数据不正常，i: " << i
                          << ", av[i]: " << tick->av[i] << ", av[i + 1]: " << tick->av[i + 1]
                          << ", ap[i]: " << tick->ap[i] << ", ap[i + 1]: " << tick->ap[i + 1];
            }
            if (i != 1) {
                if ((tick->av[i] > 0 && tick->ap[i] <= WUCAI) || (tick->av[i] == 0 && tick->ap[i] > 0) ){
                    LOG_ERROR << "检查盘口, code: " << tick->code << ", timestamp: " << tick->timestamp
                              << ", 卖盘口数据不正常，i: " << i
                              << ", av[i]: " << tick->av[i] << ", ap[i]: " << tick->ap[i];
                }
            }
            if (tick->av[i] > 0 && tick->av[i + 1] > 0) {
                if ((tick->ap[i] - tick->ap[i + 1] > WUCAI)  || fabs(tick->ap[i] - tick->ap[i + 1]) <= WUCAI) {  // ap变小
                    if (i != 0) {
                        LOG_ERROR << "检查盘口, code: " << tick->code << ", timestamp: " << tick->timestamp
                                << ", 卖盘口价格没变大, i: " << i
                                << ", ap[i]: " << tick->ap[i] << ", ap[i + 1]: " << tick->ap[i + 1];
                    }
                }
            }
        }

        for (int i = 0; i < 8; i++) {
            if (tick->bp[i] > 0 && (contract->lower_limit - tick->bp[i]) > WUCAI) {
                LOG_ERROR << "检查盘口, code: " << tick->code << ", timestamp: " << tick->timestamp
                          << ", 买价小于跌停价，i: " << i
                          << ", bp[i]: " << tick->bp[i] << ", lower_limit: " << contract->lower_limit;
            }
            if (tick->bv[i] < 0 || tick->bv[i + 1] < 0 || tick->bp[i] < 0 || tick->bp[i + 1] < 0) {
                LOG_ERROR << "检查盘口, code: " << tick->code << ", timestamp: " << tick->timestamp
                          << ", 买盘口数据不正常，i: " << i
                          << ", bv[i]: " << tick->bv[i] << ", bv[i + 1]: " << tick->bv[i + 1]
                          << ", bp[i]: " << tick->bp[i] << ", bp[i + 1]: " << tick->bp[i + 1];
            }
            if (i != 1) {
                if ((tick->bv[i] > 0 && tick->bp[i] <= WUCAI) || (tick->bv[i] == 0 && tick->bp[i] > 0) ){
                    LOG_ERROR << "检查盘口, code: " << tick->code << ", timestamp: " << tick->timestamp
                              << ", 买盘口数据不正常，i: " << i
                              << ", bv[i]: " << tick->bv[i] << ", bp[i]: " << tick->bp[i];
                }
            }
            if (tick->bv[i] > 0 && tick->bv[i + 1] > 0) {
                if ((tick->bp[i] - tick->bp[i + 1] < WUCAI)  || fabs(tick->bp[i] - tick->bp[i + 1]) <= WUCAI) {  // bp变大
                    if (i != 0) {
                        LOG_ERROR << "检查盘口, code: " << tick->code << ", timestamp: " << tick->timestamp
                                  << ", 买盘口价格没变小, i: " << i
                                  << ", bp[i]: " << tick->bp[i] << ", bp[i + 1]: " << tick->bp[i + 1];
                    }
                }
            }
        }
    }
}
