#include "compare_one.h"
#include <regex>

#define WUCAI 0.0001

namespace co {
    CompareOneCode::CompareOneCode() {
    }

    CompareOneCode::~CompareOneCode() {
    }

    void CompareOneCode::Init() {
        string right_file = Config::Instance()->right_data();
        string new_file = Config::Instance()->new_data();
        ReadFile(right_file, right_data_);
        ReadFile(new_file, new_data_);
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
            CompareContract(&right_contract, &new_contract);
            // 比较 Tick
            {
                for (auto iter = right_tick->begin(); iter != right_tick->end(); ++iter) {
                    auto _it = new_tick->find(iter->first);
                    if (_it == new_tick->end()) {
                        shared_ptr<StatisticsData> statistics;
                        auto __it = statistics_data_.find(it->first);
                        if (__it == statistics_data_.end()) {
                            statistics = std::make_shared<StatisticsData>();
                            statistics_data_.insert(std::make_pair(it->first, statistics));
                        } else {
                            statistics = __it->second;
                        }
                        statistics->tick_miss ++;
                        if (statistics->order_miss <= 10) {
                            MemQTickBody& tick = iter->second;
                            LOG_INFO << "miss tick, code: " << tick.code << ", timestamp: " << tick.timestamp;
                        }
                    } else {
                        MemQTickBody& right_tick = iter->second;
                        MemQTickBody& new_tick = _it->second;
                        CompareTick(&right_tick, &new_tick);
                    }
                }
            }
            // 比较 Order
            {
//                for (auto iter = right_order->begin(); iter != right_order->end(); ++iter) {
//                    LOG_INFO << "right order, " << iter->first;
//                }
//                for (auto iter = new_order->begin(); iter != new_order->end(); ++iter) {
//                    LOG_INFO << "new order, " << iter->first;
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
                            LOG_INFO << "miss order, code: " << order.code << ", timestamp: " << order.timestamp << ", order_no: " << order.order_no;
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
                            LOG_INFO << "miss knock, code: " << knock.code << ", timestamp: " << knock.timestamp << ", match_no: " << knock.match_no;
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
            LOG_INFO << "statistics, code: " << it->first << ", tick_diff: " << it->second->tick_diff
                    << ", tick_miss: " << it->second->tick_miss
                    << ", order_diff: " << it->second->order_diff
                    << ", order_miss: " << it->second->order_miss
                    << ", knock_diff: " << it->second->knock_diff
                    << ", knock_miss: " << it->second->knock_miss;
        }
        for (auto& it : miss_code_) {
            LOG_ERROR << "miss code: " << it;
        }
    }

    void CompareOneCode::ParseQTickFromWal(const string& line, unordered_map<std::string, shared_ptr<FullDate>>& data) {
        std::regex reg("(.*)dtype: (.*), timestamp: (.*), code: (.*), name: (.*), market: (.*), pre_close: (.*), upper_limit: (.*), lower_limit: (.*), bp: (.*), bv: (.*), ap: (.*), av: (.*), status: (.*), new_price: (.*), new_volume: (.*), new_amount: (.*), sum_volume: (.*), sum_amount: (.*), open: (.*), high: (.*), low: (.*), avg_bid_price: (.*)");
        std::cmatch m;
        auto ret = std::regex_search(line.c_str(), m, reg);
        if (ret) {
            int64_t timestamp = atoll(m[3].str().c_str());
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
            string code = m[4].str();
            string std_code = code.substr(1, code.length() - 2);
            bool first_flag = false;
            shared_ptr<FullDate> full_data;
            auto it = data.find(std_code);
            if (it == data.end()) {
                full_data = std::make_shared<FullDate>();
                memset(&full_data->contract, 0, sizeof(MemQTickHead));
                strncpy(full_data->contract.code, std_code.c_str(), std_code.length());
                full_data->contract.timestamp = timestamp;
                full_data->contract.market = atoll(m[6].str().c_str());
                full_data->contract.dtype = atoi(m[2].str().c_str());
                full_data->contract.pre_close = atof(m[7].str().c_str());
                full_data->contract.upper_limit = atof(m[8].str().c_str());
                full_data->contract.lower_limit = atof(m[9].str().c_str());
                first_flag = true;
                full_data->mmap_tick = std::make_shared<map<int64_t, MemQTickBody>>();
                full_data->mmap_order = std::make_shared<map<int64_t, MemQOrder>>();
                full_data->mmap_knock = std::make_shared<map<int64_t, MemQKnock>>();
                data.insert(std::make_pair(std_code, full_data));
            } else {
                full_data = it->second;
            }
            MemQTickBody tick;
            memset(&tick, 0, sizeof(tick));
            strncpy(tick.code, std_code.c_str(), std_code.length());
            tick.timestamp = timestamp;
            {
                string tmp = m[10].str();
                tmp = tmp.substr(1, tmp.length() - 2);
                if (tmp.length() > 0) {
                    vector<string> bp;
                    x::Split(&bp, tmp, ",");
                    int index = 0;
                    for (auto & it : bp) {
                        tick.bp[index++] = atof(it.c_str());
                    }
                }
            }
            {
                string tmp = m[11].str();
                tmp = tmp.substr(1, tmp.length() - 2);
                if (tmp.length() > 0) {
                    vector<string> bv;
                    x::Split(&bv, tmp, ",");
                    int index = 0;
                    for (auto & it : bv) {
                        tick.bv[index++] = atoll(it.c_str());
                    }
                }
            }
            {
                string tmp = m[12].str();
                tmp = tmp.substr(1, tmp.length() - 2);
                if (tmp.length() > 0) {
                    vector<string> ap;
                    x::Split(&ap, tmp, ",");
                    int index = 0;
                    for (auto & it : ap) {
                        tick.ap[index++] = atof(it.c_str());
                    }
                }
            }
            {
                string tmp = m[13].str();
                tmp = tmp.substr(1, tmp.length() - 2);
                if (tmp.length() > 0) {
                    vector<string> av;
                    x::Split(&av, tmp, ",");
                    int index = 0;
                    for (auto & it : av) {
                        tick.av[index++] = atoll(it.c_str());
                    }
                }
            }
            tick.new_price = atof(m[15].str().c_str());
            tick.new_volume = atoll(m[16].str().c_str());
            tick.new_amount = atof(m[17].str().c_str());
            tick.sum_volume = atoll(m[18].str().c_str());
            tick.sum_amount = atof(m[19].str().c_str());
            tick.open = atof(m[20].str().c_str());
            tick.high = atof(m[21].str().c_str());
            tick.low = atof(m[22].str().c_str());
            string left = m[23].str();
            {
                std::regex reg("(.*)open_interest: (.*), pre_settle: (.*), pre_open_interest: (.*), close: (.*), settle: (.*), multiple: (.*), price_step: (.*), create_date: (.*)");
                std::cmatch m;
                auto ret = std::regex_search(left.c_str(), m, reg);
                if (ret) {
                    tick.open_interest = atoll(m[2].str().c_str());
                    tick.close = atof(m[5].str().c_str());
                    tick.settle = atof(m[6].str().c_str());
                    if (first_flag) {
                        full_data->contract.pre_settle = atof(m[3].str().c_str());
                        full_data->contract.pre_open_interest = atoll(m[4].str().c_str());
                        full_data->contract.multiple = atoll(m[7].str().c_str());
                        full_data->contract.price_step = atof(m[8].str().c_str());
                        string left = m[9].str();
                        {
                            std::regex reg("(.*)cp_flag: (.*), underlying_code: (.*), sum_bid_volume: (.*)");
                            std::cmatch m;
                            auto ret = std::regex_search(left.c_str(), m, reg);
                            if (ret) {
                                full_data->contract.cp_flag = atoi(m[2].str().c_str());
                                string underlying_code = m[3].str();
                                underlying_code = underlying_code.substr(1, underlying_code.length() - 2);
                                strncpy(full_data->contract.underlying_code, underlying_code.c_str(), underlying_code.length());
                            }
                        }
                    }
                }
            }
            if (valid_flag) {
                full_data->mmap_tick->insert(std::make_pair(timestamp, tick));
            }
        }
    }

    void CompareOneCode::ParseQOrderFromWal(const string& line, unordered_map<std::string, shared_ptr<FullDate>>& data) {
        if (auto it = line.find("order_no: 48911"); it != line.npos) {
            int a = 0;
        }
        std::regex reg("(.*)timestamp: (.*), code: (.*), order_no: (.*), bs_flag: (.*), order_type: (.*), order_price: (.*), order_volume: (.*), (.*)");
        std::cmatch m;
        auto ret = std::regex_search(line.c_str(), m, reg);
        if (ret) {
            int64_t order_no = atoll(m[4].str().c_str());
            string code = m[3].str();
            string std_code = code.substr(1, code.length() - 2);
            auto it = data.find(std_code);
            if (it != data.end()) {
                auto full_data = it->second;
                MemQOrder order;
                memset(&order, 0, sizeof(order));
                strncpy(order.code, std_code.c_str(), std_code.length());
                order.timestamp = atoll(m[2].str().c_str());
                order.order_no = order_no;
                order.bs_flag = atoi(m[5].str().c_str());
                order.order_type = atoi(m[6].str().c_str());
                order.order_price = atoll(m[7].str().c_str());
                order.order_volume = atoll(m[8].str().c_str());
                full_data->mmap_order->insert(std::make_pair(order_no, order));
            } else {
                LOG_ERROR << "order not find code: " << std_code;
                for (auto it = data.begin(); it != data.end(); ++it) {
                    LOG_INFO << "key : " << it->first;
                }
            }
        } else {
            LOG_ERROR << "order parse faild, line: " << line;
        }
    }
    void CompareOneCode::ParseQKnockFromWal(const string& line, unordered_map<std::string, shared_ptr<FullDate>>& data) {
        std::regex reg("(.*)timestamp: (.*), code: (.*), match_no: (.*), bs_flag: (.*), bid_order_no: (.*), ask_order_no: (.*), match_price: (.*), match_volume: (.*), match_amount: (.*), (.*)");
        std::cmatch m;
        auto ret = std::regex_search(line.c_str(), m, reg);
        if (ret) {
            int64_t match_no = atoll(m[4].str().c_str());
            string code = m[3].str();
            string std_code = code.substr(1, code.length() - 2);
            bool first_flag = false;
            auto it = data.find(std_code);
            if (it != data.end()) {
                auto full_data = it->second;
                MemQKnock knock;
                memset(&knock, 0, sizeof(knock));
                strncpy(knock.code, std_code.c_str(), std_code.length());
                knock.timestamp = atoll(m[2].str().c_str());
                knock.match_no = match_no;
                knock.bid_order_no = atoi(m[6].str().c_str());
                knock.ask_order_no = atoll(m[7].str().c_str());
                knock.match_price = atoll(m[8].str().c_str());
                knock.match_volume = atoll(m[9].str().c_str());
                full_data->mmap_knock->insert(std::make_pair(match_no, knock));
            }
        }
    }

    void CompareOneCode::ReadFile(const string& file, unordered_map<std::string, shared_ptr<FullDate>>& data) {
        std::fstream infile;
        infile.open(file, std::ios::in);   // 打开文件
        if (!infile.is_open()) {
            std::cout << "open file " << file << " failed! " << std::endl;
            return;
        }
        LOG_INFO << file;
        std::string line;
        shared_ptr<FullDate> full_data;
        while (std::getline(infile, line)) {     // 按行读取
            if (line.length() < 10) {
                continue;
            }
            char first_key = line.at(0);
            if (first_key == 'Q') {
                char secnod_key = line.at(1);
                if (secnod_key == 'T') {
                    ParseQTickFromWal(line, data);
                } else if (secnod_key == 'O') {
                    ParseQOrderFromWal(line, data);
                } else if (secnod_key == 'K') {
                    ParseQKnockFromWal(line, data);
                }
            } else if (first_key == 'M') {
                char secnod_key = line.at(4);
                if (secnod_key == 'C') {
                    ParseQContractFromMem(line, data);
                } else if (secnod_key == 'T') {
                    ParseQTickFromMem(line, data);
                } else if (secnod_key == 'O') {
                    ParseQOrderFromMem(line, data);
                } else if (secnod_key == 'K') {
                    ParseQKnockFromMem(line, data);
                }
            }
        }
        infile.close();
        for (auto it = data.begin(); it != data.end(); ++it) {
            LOG_INFO << "code: " << it->first << ", static pre_close: " << it->second->contract.pre_close
                            << ", upper_limit: " << it->second->contract.upper_limit
                            << ", lower_limit: " << it->second->contract.lower_limit
                            << ", tick num: " << it->second->mmap_tick->size()
                            << ", order num: " << it->second->mmap_order->size()
                            << ", knock num: " << it->second->mmap_knock->size();
        }
    }

    void CompareOneCode::CompareContract(MemQTickHead* right, MemQTickHead* data) {
        if (strcmp(right->underlying_code, data->underlying_code) != 0) {
            LOG_ERROR << "MemQTickHead diff, underlying_code, right: " << right->underlying_code << ", new: " << data->underlying_code;
        }
        if (fabs(right->pre_close - data->pre_close) > WUCAI) {
            LOG_ERROR << "MemQTickHead diff, pre_close, right: " << right->pre_close << ", new: " << data->pre_close;
        }
        if (fabs(right->upper_limit - data->upper_limit) > WUCAI) {
            LOG_ERROR << "MemQTickHead diff, upper_limit, right: " << right->upper_limit << ", new: " << data->upper_limit;
        }
        if (fabs(right->lower_limit - data->lower_limit) > WUCAI) {
            LOG_ERROR << "MemQTickHead diff, lower_limit, right: " << right->lower_limit << ", new: " << data->lower_limit;
        }
        if (fabs(right->pre_settle - data->pre_settle) > WUCAI) {
            LOG_ERROR << "MemQTickHead diff, pre_settle, right: " << right->pre_settle << ", new: " << data->pre_settle;
        }
        if (right->pre_open_interest != data->pre_open_interest) {
            LOG_ERROR << "MemQTickHead diff, pre_open_interest, right: " << right->pre_open_interest << ", new: " << data->pre_open_interest;
        }
        if (right->multiple != data->multiple) {
            LOG_ERROR << "MemQTickHead diff, multiple, right: " << right->multiple << ", new: " << data->multiple;
        }
        if (fabs(right->price_step - data->price_step) > WUCAI) {
            LOG_ERROR << "MemQTickHead diff, price_step, right: " << right->price_step << ", new: " << data->price_step;
        }
        if (right->market != data->market) {
            LOG_ERROR << "MemQTickHead diff, market, right: " << right->market << ", new: " << data->market;
        }
        if (right->dtype != data->dtype) {
            LOG_ERROR << "MemQTickHead diff, dtype, right: " << right->dtype << ", new: " << data->dtype;
        }
        if (right->cp_flag != data->cp_flag) {
            LOG_ERROR << "MemQTickHead diff, cp_flag, right: " << right->cp_flag << ", new: " << data->cp_flag;
        }
        if (right->volume_unit != data->volume_unit) {
            LOG_ERROR << "MemQTickHead diff, volume_unit, right: " << right->volume_unit << ", new: " << data->volume_unit;
        }
    }

    void CompareOneCode::CompareTick(MemQTickBody* right, MemQTickBody* data) {
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
        if (fabs(right->new_amount - data->new_amount) > WUCAI) {
            ss << ", right new_amount: " << right->new_amount << ", new new_amount: " << data->new_amount;
        }
        if (right->sum_volume != data->sum_volume) {
            ss << ", right sum_volume: " << right->sum_volume << ", new sum_volume: " << data->sum_volume;
        }
        if (fabs(right->sum_amount - data->sum_amount) > WUCAI) {
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

    void CompareOneCode::CompareOrder(MemQOrder* right, MemQOrder* data) {
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

    void CompareOneCode::CompareKnock(MemQKnock* right, MemQKnock* data) {
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

    void CompareOneCode::ParseQContractFromMem(const string& line, unordered_map<std::string, shared_ptr<FullDate>>& data) {
        std::regex reg("(.*)timestamp: (.*), code_index: (.*), code: (.*), name: (.*), dtype: (.*), market: (.*), pre_close: (.*), upper_limit: (.*), lower_limit: (.*), pre_settle: (.*), pre_open_interest: (.*), underlying_code: (.*), multiple: (.*), price_step: (.*), list_date: (.*), expire_date: (.*), exercise_price: (.*), cp_flag: (.*), volume_unit: (.*)}");
        std::cmatch m;
        auto ret = std::regex_search(line.c_str(), m, reg);
        if (ret) {
            int64_t timestamp = atoll(m[2].str().c_str());
            string code = m[4].str();
            string std_code = code.substr(1, code.length() - 2);
            shared_ptr<FullDate> full_data;
            auto it = data.find(std_code);
            if (it == data.end()) {
                full_data = std::make_shared<FullDate>();
                memset(&full_data->contract, 0, sizeof(MemQTickHead));
                strncpy(full_data->contract.code, std_code.c_str(), std_code.length());
                full_data->contract.timestamp = timestamp;
                full_data->contract.market = atoll(m[7].str().c_str());
                full_data->contract.dtype = atoi(m[6].str().c_str());
                full_data->contract.pre_close = atof(m[8].str().c_str());
                full_data->contract.upper_limit = atof(m[9].str().c_str());
                full_data->contract.lower_limit = atof(m[10].str().c_str());
                full_data->contract.pre_settle = atof(m[11].str().c_str());
                full_data->contract.pre_open_interest = atoll(m[12].str().c_str());
                string underlying_code = m[13].str();
                underlying_code = underlying_code.substr(1, underlying_code.length() - 2);
                strncpy(full_data->contract.underlying_code, underlying_code.c_str(), underlying_code.length());
                full_data->contract.multiple = atoll(m[14].str().c_str());
                full_data->contract.price_step = atof(m[15].str().c_str());
                full_data->contract.list_date = atol(m[16].str().c_str());
                full_data->contract.expire_date = atol(m[17].str().c_str());
                full_data->contract.exercise_price = atof(m[18].str().c_str());
                full_data->contract.cp_flag = atol(m[19].str().c_str());
                full_data->contract.volume_unit = atol(m[20].str().c_str());

                full_data->mmap_tick = std::make_shared<map<int64_t, MemQTickBody>>();
                full_data->mmap_order = std::make_shared<map<int64_t, MemQOrder>>();
                full_data->mmap_knock = std::make_shared<map<int64_t, MemQKnock>>();
                data.insert(std::make_pair(std_code, full_data));
            }
        }
    }

    void CompareOneCode::ParseQTickFromMem(const string& line, unordered_map<std::string, shared_ptr<FullDate>>& data) {
        std::regex reg("(.*)src: (.*), timestamp: (.*), code_index: (.*), code: (.*), new_price: (.*), new_volume: (.*), new_amount: (.*), sum_volume: (.*), sum_amount: (.*), new_bid_volume: (.*), new_bid_amount: (.*), new_ask_volume: (.*), new_ask_amount: (.*), bp: (.*), bv: (.*), ap: (.*), av: (.*), state: (.*), open: (.*), high: (.*), low: (.*), close: (.*), settle: (.*), open_interest: (.*), holo_no: (.*)");
        std::cmatch m;
        auto ret = std::regex_search(line.c_str(), m, reg);
        if (ret) {
            int64_t timestamp = atoll(m[3].str().c_str());
            int stamp = timestamp % 1000000000LL;
            if (stamp < 91500000) {
                return;
            }
            if (stamp > 1131000 && stamp < 1300000) {
                return;
            }
            if (stamp > 150000000) {
                return;
            }
            string code = m[5].str();
            string std_code = code.substr(1, code.length() - 2);
            bool first_flag = false;
            shared_ptr<FullDate> full_data;
            auto it = data.find(std_code);
            if (it == data.end()) {
                LOG_ERROR << std_code << " not find Contract";
                return;
            } else {
                full_data = it->second;
            }
            MemQTickBody tick;
            memset(&tick, 0, sizeof(tick));
            strncpy(tick.code, std_code.c_str(), std_code.length());
            tick.timestamp = timestamp;
            tick.new_price = atof(m[6].str().c_str());
            tick.new_volume = atoll(m[7].str().c_str());
            tick.new_amount = atof(m[8].str().c_str());
            tick.sum_volume = atoll(m[9].str().c_str());
            tick.sum_amount = atof(m[10].str().c_str());
            tick.new_bid_volume = atoll(m[11].str().c_str());
            tick.new_bid_amount = atof(m[12].str().c_str());
            tick.new_ask_volume = atoll(m[13].str().c_str());
            tick.new_ask_amount = atof(m[14].str().c_str());
            {
                string tmp = m[15].str();
                tmp = tmp.substr(1, tmp.length() - 2);
                if (tmp.length() > 0) {
                    vector<string> bp;
                    x::Split(&bp, tmp, ",");
                    int index = 0;
                    for (auto & it : bp) {
                        tick.bp[index++] = atof(it.c_str());
                    }
                }
            }
            {
                string tmp = m[16].str();
                tmp = tmp.substr(1, tmp.length() - 2);
                if (tmp.length() > 0) {
                    vector<string> bv;
                    x::Split(&bv, tmp, ",");
                    int index = 0;
                    for (auto & it : bv) {
                        tick.bv[index++] = atoll(it.c_str());
                    }
                }
            }
            {
                string tmp = m[17].str();
                tmp = tmp.substr(1, tmp.length() - 2);
                if (tmp.length() > 0) {
                    vector<string> ap;
                    x::Split(&ap, tmp, ",");
                    int index = 0;
                    for (auto & it : ap) {
                        tick.ap[index++] = atof(it.c_str());
                    }
                }
            }
            {
                string tmp = m[18].str();
                tmp = tmp.substr(1, tmp.length() - 2);
                if (tmp.length() > 0) {
                    vector<string> av;
                    x::Split(&av, tmp, ",");
                    int index = 0;
                    for (auto & it : av) {
                        tick.av[index++] = atoll(it.c_str());
                    }
                }
            }
            tick.state = atoi(m[19].str().c_str());
            tick.open = atof(m[20].str().c_str());
            tick.high = atof(m[21].str().c_str());
            tick.low = atof(m[22].str().c_str());
            tick.close = atof(m[23].str().c_str());
            tick.settle = atof(m[24].str().c_str());
            tick.open_interest = atoll(m[25].str().c_str());
            full_data->mmap_tick->insert(std::make_pair(timestamp, tick));
        }
    }

    void CompareOneCode::ParseQOrderFromMem(const string& line, unordered_map<std::string, shared_ptr<FullDate>>& data) {
        std::regex reg("(.*)timestamp: (.*), code_index: (.*), code: (.*), order_no: (.*), bs_flag: (.*), order_type: (.*), order_price: (.*), order_volume: (.*)}");
        std::cmatch m;
        auto ret = std::regex_search(line.c_str(), m, reg);
        if (ret) {
            int64_t order_no = atoll(m[5].str().c_str());
            string code = m[4].str();
            string std_code = code.substr(1, code.length() - 2);
            bool first_flag = false;
            auto it = data.find(std_code);
            if (it != data.end()) {
                auto full_data = it->second;
                MemQOrder order;
                memset(&order, 0, sizeof(order));
                strncpy(order.code, std_code.c_str(), std_code.length());
                order.timestamp = atoll(m[2].str().c_str());
                order.order_no = order_no;
                order.bs_flag = atoi(m[6].str().c_str());
                order.order_type = atoi(m[7].str().c_str());
                order.order_price = atoll(m[8].str().c_str());
                order.order_volume = atoll(m[9].str().c_str());
                full_data->mmap_order->insert(std::make_pair(order_no, order));
            }
        }
    }

    void CompareOneCode::ParseQKnockFromMem(const string& line, unordered_map<std::string, shared_ptr<FullDate>>& data) {
        std::regex reg("(.*)timestamp: (.*), code_index: (.*), code: (.*), match_no: (.*), bid_order_no: (.*), ask_order_no: (.*), match_price: (.*), match_volume: (.*)}");
        std::cmatch m;
        auto ret = std::regex_search(line.c_str(), m, reg);
        if (ret) {
            int64_t match_no = atoll(m[5].str().c_str());
            string code = m[4].str();
            string std_code = code.substr(1, code.length() - 2);
            bool first_flag = false;
            auto it = data.find(std_code);
            if (it != data.end()) {
                auto full_data = it->second;
                MemQKnock knock;
                memset(&knock, 0, sizeof(knock));
                strncpy(knock.code, std_code.c_str(), std_code.length());
                knock.timestamp = atoll(m[2].str().c_str());
                knock.match_no = match_no;
                knock.bid_order_no = atoi(m[6].str().c_str());
                knock.ask_order_no = atoll(m[7].str().c_str());
                knock.match_price = atoll(m[8].str().c_str());
                knock.match_volume = atoll(m[9].str().c_str());
                full_data->mmap_knock->insert(std::make_pair(match_no, knock));
            }
        }
    }
}
