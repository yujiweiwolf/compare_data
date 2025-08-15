#include <cstring>
#include <cstdio>
#include <array>
#include <algorithm>

#include "code_index.h"

#include "/home/work/sys/lib_22.04/libcoral-1.1.0/include/coral/define.h"

namespace co {

    //void Test() {
//    std::cout << GenerateCodeIndexMagicSuffix() << std::endl;
//    std::cout << GenerateCodeIndexConstants() << std::endl;
//}

    std::string GenerateCodeIndexMagicSuffix();
    std::string GenerateCodeIndexConstants();
    int64_t ParseCodePrefixDigit(const char* begin, const char* end, int64_t base);
    int64_t ParseCodePrefixFuture(const char* begin, const char* end, int64_t base1, int64_t base2, bool is_upper);
    int ParseCodeIndexDigit(char* out_data, int out_size, const char* format, int64_t code_index, int64_t base);
    int ParseCodeIndexFuture1(char* out_data, int out_size, const char* format, int64_t code_index, int64_t base, char alpha_a);
    int ParseCodeIndexFuture2(char* out_data, int out_size, const char* format, int64_t code_index, int64_t base, char alpha_a);
    int ParseCodeIndexFutureUpper1(char* out_data, int out_size, const char* format, int64_t code_index, int64_t base);
    int ParseCodeIndexFutureUpper2(char* out_data, int out_size, const char* format, int64_t code_index, int64_t base);
    int ParseCodeIndexFutureLower1(char* out_data, int out_size, const char* format, int64_t code_index, int64_t base);
    int ParseCodeIndexFutureLower2(char* out_data, int out_size, const char* format, int64_t code_index, int64_t base);

    struct CodeRange {
        int64_t begin;
        int64_t end;
        const char* format;
        int (*parser)(char*, int, const char*, int64_t, int64_t);
    };

    constexpr std::array<CodeRange, 20> kCodeRanges = {{
                                                               {kCodeIndexBeginSH,     kCodeIndexEndSH,     "%06lld.SH",    ParseCodeIndexDigit},
                                                               {kCodeIndexBeginSZ,     kCodeIndexEndSZ,     "%06lld.SZ",    ParseCodeIndexDigit},
                                                               {kCodeIndexBeginBJ,     kCodeIndexEndBJ,     "%06lld.BJ",    ParseCodeIndexDigit},
                                                               {kCodeIndexBeginCSI,    kCodeIndexEndCSI,    "%06lld.CSI",   ParseCodeIndexDigit},
                                                               {kCodeIndexBeginCNI,    kCodeIndexEndCNI,    "%06lld.CNI",   ParseCodeIndexDigit},
                                                               {kCodeIndexBeginHK,     kCodeIndexEndHK,     "%05lld.HK",    ParseCodeIndexDigit},
                                                               {kCodeIndexBeginCFFEX1, kCodeIndexEndCFFEX1, "%c%02d%02d.CFFEX", ParseCodeIndexFutureUpper1},
                                                               {kCodeIndexBeginCFFEX2, kCodeIndexEndCFFEX2, "%c%c%02d%02d.CFFEX", ParseCodeIndexFutureUpper2},
                                                               {kCodeIndexBeginSHFE1,  kCodeIndexEndSHFE1,  "%c%02d%02d.SHFE",  ParseCodeIndexFutureLower1},
                                                               {kCodeIndexBeginSHFE2,  kCodeIndexEndSHFE2,  "%c%c%02d%02d.SHFE",  ParseCodeIndexFutureLower2},
                                                               {kCodeIndexBeginDCE1,   kCodeIndexEndDCE1,   "%c%02d%02d.DCE",   ParseCodeIndexFutureLower1},
                                                               {kCodeIndexBeginDCE2,   kCodeIndexEndDCE2,   "%c%c%02d%02d.DCE",   ParseCodeIndexFutureLower2},
                                                               {kCodeIndexBeginCZCE1,  kCodeIndexEndCZCE1,  "%c%02d%02d.CZCE",  ParseCodeIndexFutureUpper1},
                                                               {kCodeIndexBeginCZCE2,  kCodeIndexEndCZCE2,  "%c%c%02d%02d.CZCE",  ParseCodeIndexFutureUpper2},
                                                               {kCodeIndexBeginGFE1,   kCodeIndexEndGFE1,   "%c%02d%02d.GFE",   ParseCodeIndexFutureLower1},
                                                               {kCodeIndexBeginGFE2,   kCodeIndexEndGFE2,   "%c%c%02d%02d.GFE",   ParseCodeIndexFutureLower2},
                                                               {kCodeIndexBeginINE1,   kCodeIndexEndINE1,   "%c%02d%02d.INE",   ParseCodeIndexFutureLower1},
                                                               {kCodeIndexBeginINE2,   kCodeIndexEndINE2,   "%c%c%02d%02d.INE",   ParseCodeIndexFutureLower2},
                                                               {kCodeIndexBeginSGE1,   kCodeIndexEndSGE1,   "%c%02d%02d.SGE",   ParseCodeIndexFutureUpper1},
                                                               {kCodeIndexBeginSGE2,   kCodeIndexEndSGE2,   "%c%c%02d%02d.SGE",   ParseCodeIndexFutureUpper2},
                                                               }};

    std::string GenerateCodeIndexMagicSuffix() {
        // 生成解析market的代码
        std::vector<std::tuple<std::string, std::string>> suffix_list = {
                {kSuffixSH, "kMarketSH"},
                {kSuffixSZ, "kMarketSZ"},
                {kSuffixBJ, "kMarketBJ"},
                {kSuffixCSI, "kMarketCSI"},
                {kSuffixCNI, "kMarketCNI"},
                {kSuffixHK, "kMarketHK"},
                {kSuffixCFFEX, "kMarketCFFEX"},
                {kSuffixSHFE, "kMarketSHFE"},
                {kSuffixDCE, "kMarketDCE"},
                {kSuffixCZCE, "kMarketCZCE"},
                {kSuffixGFE, "kMarketGFE"},
                {kSuffixINE, "kMarketINE"},
                {kSuffixSGE, "kMarketSGE"},
        };

        std::stringstream ss;
        ss << "// +---------- DO NOT EDIT BEGIN ----------+" << std::endl;
        ss << "// Generated by GenerateCodeIndexMagicSuffix().  DO NOT EDIT!" << std::endl;
        ss << "int64_t market = 0;" << std::endl;
        ss << "uint32_t bit_num = 0x01020304;" << std::endl;
        ss << "bool is_little_endian = *reinterpret_cast<uint8_t*>(&bit_num) == 0x04;" << std::endl;
        ss << "if (is_little_endian) {" << std::endl;
        ss << "    switch (suffix_binary) {" << std::endl;
        for (auto& [suffix, market]: suffix_list) {
            uint64_t magic_suffix = 0;
            if (suffix.size() > sizeof(magic_suffix)) {
                throw std::runtime_error("suffix is too long: " + suffix);
            }
            auto p_magic_suffix = reinterpret_cast<char*>(&magic_suffix);
            for (int i = 0; i < suffix.size(); ++i) {
                p_magic_suffix[i] = suffix[i];
            }
            ss << "        case 0x" << std::hex << magic_suffix << ": market = " << market << "; break;" << std::endl;
        }
        ss << "        default: break;" << std::endl;
        ss << "    }" << std::endl;
        ss << "} else {" << std::endl;
        ss << "    switch (suffix_binary) {" << std::endl;
        for (auto& [suffix, market]: suffix_list) {
            uint64_t magic_suffix = 0;
            if (suffix.size() > sizeof(magic_suffix)) {
                throw std::runtime_error("suffix is too long: " + suffix);
            }
            auto p_magic_suffix = reinterpret_cast<char*>(&magic_suffix);
            for (int i = 0; i < suffix.size(); ++i) {
                p_magic_suffix[sizeof(magic_suffix) - i - 1] = suffix[i];
            }
            ss << "        case 0x" << std::hex << magic_suffix << ": market = " << market << "; break;" << std::endl;
        }
        ss << "        default: break;" << std::endl;
        ss << "    }" << std::endl;
        ss << "}" << std::endl;
        ss << "// +---------- DO NOT EDIT END   ----------+" << std::endl;
        return ss.str();
    }

    std::string GenerateCodeIndexConstants() {
        // 生成常量定义代码
        // 修改后，需要手工更新kCodeRanges常量；
        //
        //【十进制编码法】直接转换为10进制数字，空间占用：1000000 * sizeof(int64_t) / (1<<20) = 7.63MB，示例："510300.SH" -> 510300
        // 6位数字：range: [000000, 999999], capacity: 1000000, bytes: 7.63MB
        // 特殊地：000000.SH是合法输入，但是因为前缀和后缀编码后都是0，导致结果也是0，被识别为无效值。
        //
        //【字母+月份编码法】将字母和月份分别编码后再拼接
        // 字母部分采样26进制，限制必须全部为小写或者大写，长度为1~3个字母；
        // 4位数的月份采用月份编码法，减去基准月份（2000年01月），得到差值；
        // 采用10个二进制位存储，能存储2**10=85年零4个月，最大值：2085年04月；
        // range: [AA-"2000-01", ZZ-"2085-04"], capacity=26**2 * 2**10=692224, bytes: 5.28MB
        // 字母部分：1~3个字母，最大值：26**3 - 1
        // 月份部分：4个数字，最大值：2**10 - 1，基准月份：2000年01
        // 0001表示2000年01月，是支持的最小合约月份
        // 8504表示2085年04月，是支持的最大合约月份。

        int64_t d5_size = 100000;  // 5位数字，capacity: 10**5 = 100000, bytes: 0.76MB
        int64_t d6_size = 1000000;  // 6位数字，capacity: 10**6 = 1000000, bytes: 7.63MB
        int64_t c1_size = 26624;  // 1位字母+月份，capacity: 26**1 * 2**10 = 26624, bytes: 0.20MB
        int64_t c2_size = 692224;  // 1位字母+月份，capacity: 26**2 * 2**10 = 692224, bytes: 5.28MB
        int64_t c3_size = 17997824;  // 1位字母+月份，capacity: 26**3 * 2**10 = 17997824, bytes: 137.31MB
        std::vector<std::tuple<std::string, int64_t>> range_list = {
                {"SH", d6_size},
                {"SZ", d6_size},
                {"BJ", d6_size},
                {"CSI", d6_size},
                {"CNI", d6_size},
                {"HK", d5_size},
                {"CFFEX1", c1_size},
                {"CFFEX2", c2_size},
                {"SHFE1", c1_size},
                {"SHFE2", c2_size},
                {"DCE1", c1_size},
                {"DCE2", c2_size},
                {"CZCE1", c1_size},
                {"CZCE2", c2_size},
                {"GFE1", c1_size},
                {"GFE2", c2_size},
                {"INE1", c1_size},
                {"INE2", c2_size},
                {"SGE1", c1_size},
                {"SGE2", c2_size},
        };
        std::stringstream ss;
        ss << "// +---------- DO NOT EDIT BEGIN ----------+" << std::endl;
        ss << "// Generated by GenerateCodeIndexConstants().  DO NOT EDIT!" << std::endl;
        int64_t sum_size = 0;
        for (auto& [type, size]: range_list) {
            ss << "constexpr int64_t kCodeIndexBegin" << type << " = " << sum_size << ";" << std::endl;
            sum_size += size;
            ss << "constexpr int64_t kCodeIndexEnd" << type << " = " << sum_size << ";" << std::endl;
        }
        ss << std::endl;
        ss << "constexpr int64_t kCodeIndexArraySize = " << sum_size << ";" << std::endl;
        ss << "constexpr int64_t kCodeIndexMin = 1;" << std::endl;
        ss << "constexpr int64_t kCodeIndexMax = " << (sum_size - 1) << ";" << std::endl;
        ss << "// +---------- DO NOT EDIT END   ----------+" << std::endl;
        return ss.str();
    }

    int64_t ParseCodePrefixDigit(const char* begin, const char* end, int64_t base) {
        int64_t value = 0;
        while (begin < end) {
            char c = *begin++;
            if (c < '0' || c > '9') {
                return 0;
            }
            value = value * 10 + (c - '0');
        }
        return base + value;
    }

    int64_t ParseCodePrefixFuture(const char* begin, const char* end, int64_t base1, int64_t base2, bool is_upper) {
        enum class State : uint8_t {
            kStart,
            kAlpha1,
            kAlpha2,
            kYear1,
            kYear2,
            kMonth1,
            kEnd,
            kError
        };

        char alpha_a = is_upper ? 'A' : 'a';
        char alpha_z = is_upper ? 'Z' : 'z';

        int alpha_count = 0; // 字母个数
        int64_t alpha_value = 0;  // 字母值
        int64_t year = 0;  // 年份值，4位数年份的后两位数字，比如2025年的year为25
        int64_t month = 0;  // 月份值
        State state = State::kStart;
        while (state != State::kError && begin < end) {
            char c = *begin++;
            switch (state) {
                case State::kStart:
                    if (c >= alpha_a && c <= alpha_z) {
                        ++alpha_count;
                        alpha_value = alpha_value * 26 + (int64_t)(c - alpha_a);
                        state = State::kAlpha1;
                    } else {
                        state = State::kError;
                    }
                    break;
                case State::kAlpha1:
                    if (c >= alpha_a && c <= alpha_z) {
                        ++alpha_count;
                        alpha_value = alpha_value * 26 + (int64_t)(c - alpha_a);
                        state = State::kAlpha2;
                    } else if (c >= '0' && c <= '9') {
                        year = year * 10 + (int64_t)(c - '0');
                        state = State::kYear1;
                    } else {
                        state = State::kError;
                    }
                    break;
                case State::kAlpha2:
                    if (c >= '0' && c <= '9') {
                        year = year * 10 + (int64_t)(c - '0');
                        state = State::kYear1;
                    } else {
                        state = State::kError;
                    }
                    break;
                case State::kYear1:
                    if (c >= '0' && c <= '9') {
                        year = year * 10 + (int64_t)(c - '0');
                        state = State::kYear2;
                    } else {
                        state = State::kError;
                    }
                    break;
                case State::kYear2:
                    if (c >= '0' && c <= '9') {
                        month = month * 10 + (int64_t)(c - '0');
                        state = State::kMonth1;
                    } else {
                        state = State::kError;
                    }
                    break;
                case State::kMonth1:
                    if (c >= '0' && c <= '9') {
                        month = month * 10 + (int64_t)(c - '0');
                        state = State::kEnd;
                    } else {
                        state = State::kError;
                    }
                    break;
                default:
                    state = State::kError;
                    break;
            }
        }
        int64_t ret = 0;
        if (state == State::kEnd) {
            int64_t magic_month = year * 12 + month - 1;  // 合约月份 - 2000年01月
            if (month >= 1 && month <= 12 &&
                magic_month >= 0 && magic_month <= kCodeIndexEncodedMonthMax) {
                ret = (alpha_value << kCodeIndexEncodedMonthBits) | magic_month;
                if (alpha_count == 1) {
                    ret += base1;
                } else {
                    ret += base2;
                }
            }
        }
        return ret;
    }

    int ParseCodeIndexDigit(char* out_data, int out_size, const char* format, int64_t code_index, int64_t base) {
        code_index -= base;
        int len = snprintf(out_data, out_size, format, code_index);
        if (len <= 0 || len >= out_size) {
            memset(out_data, 0, out_size);
        }
        return len;
    }

    int ParseCodeIndexFuture1(char* out_data, int out_size, const char* format, int64_t code_index, int64_t base, char alpha_a) {
        code_index -= base;
        int64_t n_month = code_index & kCodeIndexEncodedMonthMax;  // kCodeIndexEncodedMonthMax = 2**10 - 1 = 1023 = 0x3ff
        int64_t year = n_month / 12;
        int64_t month = 1 + n_month % 12;
        code_index = code_index >> kCodeIndexEncodedMonthBits;  // kCodeIndexEncodedMonthBits = 10
        char alpha = alpha_a + code_index;
        int len = snprintf(out_data, out_size, format, alpha, year, month);
        if (len <= 0 || len >= out_size) {
            memset(out_data, 0, out_size);
        }
        return len;
    }

    int ParseCodeIndexFuture2(char* out_data, int out_size, const char* format, int64_t code_index, int64_t base, char alpha_a) {
        code_index -= base;
        int64_t n_month = code_index & kCodeIndexEncodedMonthMax;  // kCodeIndexEncodedMonthMax = 2**10 - 1 = 1023 = 0x3ff
        int64_t year = n_month / 12;
        int64_t month = 1 + n_month % 12;
        code_index = code_index >> kCodeIndexEncodedMonthBits;  // kCodeIndexEncodedMonthBits = 10
        char alpha1 = alpha_a + code_index / 26;
        char alpha2 = alpha_a + code_index % 26;
        int len = snprintf(out_data, out_size, format, alpha1, alpha2, year, month);
        if (len <= 0 || len >= out_size) {
            memset(out_data, 0, out_size);
        }
        return len;
    }

    int ParseCodeIndexFutureUpper1(char* out_data, int out_size, const char* format, int64_t code_index, int64_t base) {
        return ParseCodeIndexFuture1(out_data, out_size, format, code_index, base, 'A');
    }

    int ParseCodeIndexFutureUpper2(char* out_data, int out_size, const char* format, int64_t code_index, int64_t base) {
        return ParseCodeIndexFuture2(out_data, out_size, format, code_index, base, 'A');
    }

    int ParseCodeIndexFutureLower1(char* out_data, int out_size, const char* format, int64_t code_index, int64_t base) {
        return ParseCodeIndexFuture1(out_data, out_size, format, code_index, base, 'a');
    }

    int ParseCodeIndexFutureLower2(char* out_data, int out_size, const char* format, int64_t code_index, int64_t base) {
        return ParseCodeIndexFuture2(out_data, out_size, format, code_index, base, 'a');
    }

    bool IsValidCodeIndex(int64_t code_index) {
        return code_index >= kCodeIndexMin && code_index <= kCodeIndexMax;
    }

    int64_t CodeToIndex(const char* code, int code_size) {
        // 将代码字符串转换为代码索引，可作为数组下标使用，编码方法以及限制详见GenerateCodeIndexConstants()中的注释；
        // 该函数耗时大概：50ns
        if (code_size < 0) {
            code_size = (int)strlen(code);
        }
        if (code_size < 3) {
            return 0;
        }
        // ----------------------------------------------------------------------------------------------
        // [第一步] 查找后缀
        // ----------------------------------------------------------------------------------------------
        uint64_t suffix_binary = 0;
        int suffix_len = 0;
        const char* p = code + code_size - 1;
        while (p >= code) {
            ++suffix_len;
            if (*p == '.') {
                memcpy(&suffix_binary, p, suffix_len);
                break;
            }
            --p;
        }
        if (suffix_len <= 1 || suffix_len >= code_size) {
            return 0;
        }
        int prefix_len = code_size - suffix_len;
        const char* prefix_end = p;
        // ----------------------------------------------------------------------------------------------
        // [第二步] 判断市场代码
        // ----------------------------------------------------------------------------------------------
        // +---------- DO NOT EDIT BEGIN ----------+
        // Generated by GenerateCodeIndexMagicSuffix().  DO NOT EDIT!
        int64_t market = 0;
        uint32_t bit_num = 0x01020304;
        bool is_little_endian = *reinterpret_cast<uint8_t*>(&bit_num) == 0x04;
        if (is_little_endian) {
            switch (suffix_binary) {
                case 0x48532e: market = kMarketSH; break;
                case 0x5a532e: market = kMarketSZ; break;
                case 0x4a422e: market = kMarketBJ; break;
                case 0x4953432e: market = kMarketCSI; break;
                case 0x494e432e: market = kMarketCNI; break;
                case 0x4b482e: market = kMarketHK; break;
                case 0x58454646432e: market = kMarketCFFEX; break;
                case 0x454648532e: market = kMarketSHFE; break;
                case 0x4543442e: market = kMarketDCE; break;
                case 0x45435a432e: market = kMarketCZCE; break;
                case 0x4546472e: market = kMarketGFE; break;
                case 0x454e492e: market = kMarketINE; break;
                case 0x4547532e: market = kMarketSGE; break;
                default: break;
            }
        } else {
            switch (suffix_binary) {
                case 0x2e53480000000000: market = kMarketSH; break;
                case 0x2e535a0000000000: market = kMarketSZ; break;
                case 0x2e424a0000000000: market = kMarketBJ; break;
                case 0x2e43534900000000: market = kMarketCSI; break;
                case 0x2e434e4900000000: market = kMarketCNI; break;
                case 0x2e484b0000000000: market = kMarketHK; break;
                case 0x2e43464645580000: market = kMarketCFFEX; break;
                case 0x2e53484645000000: market = kMarketSHFE; break;
                case 0x2e44434500000000: market = kMarketDCE; break;
                case 0x2e435a4345000000: market = kMarketCZCE; break;
                case 0x2e47464500000000: market = kMarketGFE; break;
                case 0x2e494e4500000000: market = kMarketINE; break;
                case 0x2e53474500000000: market = kMarketSGE; break;
                default: break;
            }
        }
        // +---------- DO NOT EDIT END   ----------+
        if (market <= 0) {
            return 0;
        }
        // ----------------------------------------------------------------------------------------------
        // [第三步] 解析前缀
        // ----------------------------------------------------------------------------------------------
        int64_t code_index = 0;
        switch (market) {
            case kMarketSH:
                code_index = prefix_len == 6 ? ParseCodePrefixDigit(code, prefix_end, kCodeIndexBeginSH) : 0;
                break;
            case kMarketSZ:
                code_index = prefix_len == 6 ? ParseCodePrefixDigit(code, prefix_end, kCodeIndexBeginSZ) : 0;
                break;
            case kMarketBJ:
                code_index = prefix_len == 6 ? ParseCodePrefixDigit(code, prefix_end, kCodeIndexBeginBJ) : 0;
                break;
            case kMarketCSI:
                code_index = prefix_len == 6 ? ParseCodePrefixDigit(code, prefix_end, kCodeIndexBeginCSI) : 0;
                break;
            case kMarketCNI:
                code_index = prefix_len == 6 ? ParseCodePrefixDigit(code, prefix_end, kCodeIndexBeginCNI) : 0;
                break;
            case kMarketHK:
                code_index = prefix_len == 5 ? ParseCodePrefixDigit(code, prefix_end, kCodeIndexBeginHK) : 0;
                break;
            case kMarketCFFEX:
                code_index = ParseCodePrefixFuture(code, prefix_end, kCodeIndexBeginCFFEX1, kCodeIndexBeginCFFEX2, true);
                break;
            case kMarketSHFE:
                code_index = ParseCodePrefixFuture(code, prefix_end, kCodeIndexBeginSHFE1, kCodeIndexBeginSHFE2, false);
                break;
            case kMarketDCE:
                code_index = ParseCodePrefixFuture(code, prefix_end, kCodeIndexBeginDCE1, kCodeIndexBeginDCE2, false);
                break;
            case kMarketCZCE:
                code_index = ParseCodePrefixFuture(code, prefix_end, kCodeIndexBeginCZCE1, kCodeIndexBeginCZCE2, true);
                break;
            case kMarketGFE:
                code_index = ParseCodePrefixFuture(code, prefix_end, kCodeIndexBeginGFE1, kCodeIndexBeginGFE2, false);
                break;
            case kMarketINE:
                code_index = ParseCodePrefixFuture(code, prefix_end, kCodeIndexBeginINE1, kCodeIndexBeginINE2, false);
                break;
            case kMarketSGE:
                code_index = ParseCodePrefixFuture(code, prefix_end, kCodeIndexBeginSGE1, kCodeIndexBeginSGE2, true);
                break;
            default:
                break;
        }
        return code_index;
    }

    int64_t CodeToIndex(const std::string_view& code) {
        return CodeToIndex(code.data(), code.size());
    }

    int64_t CodeToIndex(const std::string& code) {
        return CodeToIndex(code.data(), code.size());
    }

    bool IndexToCode(int64_t code_index, char* out_data, int out_size) {
        // 将代码索引转换为代码字符串，该函数耗时大概：170ns
        if (!IsValidCodeIndex(code_index)) {
            memset(out_data, 0, out_size);
            return false;
        }
        if (code_index < kCodeIndexEndSH) {  // 高频，优先判断
            ParseCodeIndexDigit(out_data, out_size, "%06lld.SH", code_index, kCodeIndexBeginSH);
        } else if (code_index < kCodeIndexEndSZ) { // 高频，优先判断
            ParseCodeIndexDigit(out_data, out_size, "%06lld.SZ", code_index, kCodeIndexBeginSZ);
        } else {  // 其他，进行二分查找
            auto itr = std::lower_bound(
                    kCodeRanges.begin(), kCodeRanges.end(), code_index,
                    [](const CodeRange& range, int64_t value) { return range.end <= value; }
            );
            if (itr != kCodeRanges.end() && code_index >= itr->begin && code_index < itr->end) {
                itr->parser(out_data, out_size, itr->format, code_index, itr->begin);
            } else {
                memset(out_data, 0, out_size);
                return false;
            }
        }
//        if (code_index < kCodeIndexEndSH) {
//            ParseCodeIndexDigit(&code, "%06lld.SH", code_index, kCodeIndexBeginSH);
//        } else if (code_index < kCodeIndexEndSZ) {
//            ParseCodeIndexDigit(&code, "%06lld.SZ", code_index, kCodeIndexBeginSZ);
//        } else if (code_index < kCodeIndexEndBJ) {
//            ParseCodeIndexDigit(&code, "%06lld.BJ", code_index, kCodeIndexBeginBJ);
//        } else if (code_index < kCodeIndexEndCSI) {
//            ParseCodeIndexDigit(&code, "%06lld.CSI", code_index, kCodeIndexBeginCSI);
//        } else if (code_index < kCodeIndexEndCNI) {
//            ParseCodeIndexDigit(&code, "%06lld.CNI", code_index, kCodeIndexBeginCNI);
//        } else if (code_index < kCodeIndexEndHK) {
//            ParseCodeIndexDigit(&code, "%05lld.HK", code_index, kCodeIndexBeginHK);
//        } else if (code_index < kCodeIndexEndCFFEX1) {
//            ParseCodeIndexFuture1(&code, "%c%02d%02d.CFFEX", code_index, kCodeIndexBeginCFFEX1, true);
//        } else if (code_index < kCodeIndexEndCFFEX2) {
//            ParseCodeIndexFuture2(&code, "%c%c%02d%02d.CFFEX", code_index, kCodeIndexBeginCFFEX2, true);
//        } else if (code_index < kCodeIndexEndSHFE1) {
//            ParseCodeIndexFuture1(&code, "%c%02d%02d.SHFE", code_index, kCodeIndexBeginSHFE1, false);
//        } else if (code_index < kCodeIndexEndSHFE2) {
//            ParseCodeIndexFuture2(&code, "%c%c%02d%02d.SHFE", code_index, kCodeIndexBeginSHFE2, false);
//        } else if (code_index < kCodeIndexEndDCE1) {
//            ParseCodeIndexFuture1(&code, "%c%02d%02d.DCE", code_index, kCodeIndexBeginDCE1, false);
//        } else if (code_index < kCodeIndexEndDCE2) {
//            ParseCodeIndexFuture2(&code, "%c%c%02d%02d.DCE", code_index, kCodeIndexBeginDCE2, false);
//        } else if (code_index < kCodeIndexEndCZCE1) {
//            ParseCodeIndexFuture1(&code, "%c%02d%02d.CZCE", code_index, kCodeIndexBeginCZCE1, true);
//        } else if (code_index < kCodeIndexEndCZCE2) {
//            ParseCodeIndexFuture2(&code, "%c%c%02d%02d.CZCE", code_index, kCodeIndexBeginCZCE2, true);
//        } else if (code_index < kCodeIndexEndGFE1) {
//            ParseCodeIndexFuture1(&code, "%c%02d%02d.GFE", code_index, kCodeIndexBeginGFE1, false);
//        } else if (code_index < kCodeIndexEndGFE2) {
//            ParseCodeIndexFuture2(&code, "%c%c%02d%02d.GFE", code_index, kCodeIndexBeginGFE2, false);
//        } else if (code_index < kCodeIndexEndSGE1) {
//            ParseCodeIndexFuture1(&code, "%c%02d%02d.SGE", code_index, kCodeIndexBeginSGE1, true);
//        } else if (code_index < kCodeIndexEndSGE2) {
//            ParseCodeIndexFuture2(&code, "%c%c%02d%02d.SGE", code_index, kCodeIndexBeginSGE2, true);
//        } else if (code_index < kCodeIndexEndINE1) {
//            ParseCodeIndexFuture1(&code, "%c%02d%02d.INE", code_index, kCodeIndexBeginINE1, false);
//        } else if (code_index < kCodeIndexEndINE2) {
//            ParseCodeIndexFuture2(&code, "%c%c%02d%02d.INE", code_index, kCodeIndexBeginINE2, false);
//        }
        return true;
    }

    std::string IndexToCode(int64_t code_index) {
        std::string code(32, '\0');
        bool ok = IndexToCode(code_index, code.data(), code.size());
        if (ok) {
            auto pos = code.find('\0');
            if (pos != std::string::npos) {
                code.erase(code.begin() + pos, code.end());
            }
        } else {
            code.clear();
        }
        return std::move(code);
    }

}