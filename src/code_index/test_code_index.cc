#include <gtest/gtest.h>

#include "coral/coral.h"

TEST(CodeToIndex, InvalidCode) {
    // 非法数据测试
    ASSERT_EQ(co::CodeToIndex(""), 0);
    ASSERT_EQ(co::CodeToIndex("1"), 0);
    ASSERT_EQ(co::CodeToIndex("."), 0);
    ASSERT_EQ(co::CodeToIndex("1."), 0);
    ASSERT_EQ(co::CodeToIndex(".1"), 0);
    ASSERT_EQ(co::CodeToIndex(".SH"), 0);
    ASSERT_EQ(co::CodeToIndex("0.SH"), 0);
    ASSERT_EQ(co::CodeToIndex("00.SH"), 0);
    ASSERT_EQ(co::CodeToIndex("00.SH"), 0);
    ASSERT_EQ(co::CodeToIndex("000.SH"), 0);
    ASSERT_EQ(co::CodeToIndex("0000.SH"), 0);
    ASSERT_EQ(co::CodeToIndex("00000.SH"), 0);
    ASSERT_EQ(co::CodeToIndex("000000.SH"), 0);
    ASSERT_EQ(co::CodeToIndex("000001.SH"), 1);
    ASSERT_EQ(co::CodeToIndex("00-001.SH"), 0);
    ASSERT_EQ(co::CodeToIndex("0000001.SH"), 0);
    ASSERT_EQ(co::CodeToIndex("00000A.SH"), 0);
    ASSERT_EQ(co::CodeToIndex("000001.XX"), 0);
    ASSERT_EQ(co::CodeToIndex("A0000.CFFEX"), 0);
    ASSERT_EQ(co::CodeToIndex("AAA0001.CFFEX"), 0);
    ASSERT_EQ(co::CodeToIndex("AA0013.CFFEX"), 0);
    ASSERT_EQ(co::CodeToIndex("AU9999.SGE"), 0);
}

TEST(CodeToIndex, LowerUpper) {
    // 大小写支持测试
    ASSERT_TRUE(co::CodeToIndex("A0001.CFFEX") != 0);
    ASSERT_TRUE(co::CodeToIndex("a0001.CFFEX") == 0);
    ASSERT_TRUE(co::CodeToIndex("AA0001.CFFEX") != 0);
    ASSERT_TRUE(co::CodeToIndex("Aa0001.CFFEX") == 0);
    ASSERT_TRUE(co::CodeToIndex("aA0001.CFFEX") == 0);
    ASSERT_TRUE(co::CodeToIndex("aa0001.CFFEX") == 0);

    ASSERT_TRUE(co::CodeToIndex("A0001.SHFE") == 0);
    ASSERT_TRUE(co::CodeToIndex("a0001.SHFE") != 0);
    ASSERT_TRUE(co::CodeToIndex("AA0001.SHFE") == 0);
    ASSERT_TRUE(co::CodeToIndex("Aa0001.SHFE") == 0);
    ASSERT_TRUE(co::CodeToIndex("aA0001.SHFE") == 0);
    ASSERT_TRUE(co::CodeToIndex("aa0001.SHFE") != 0);

    ASSERT_TRUE(co::CodeToIndex("A0001.DCE") == 0);
    ASSERT_TRUE(co::CodeToIndex("a0001.DCE") != 0);
    ASSERT_TRUE(co::CodeToIndex("AA0001.DCE") == 0);
    ASSERT_TRUE(co::CodeToIndex("Aa0001.DCE") == 0);
    ASSERT_TRUE(co::CodeToIndex("aA0001.DCE") == 0);
    ASSERT_TRUE(co::CodeToIndex("aa0001.DCE") != 0);
    
    ASSERT_TRUE(co::CodeToIndex("A0001.CZCE") != 0);
    ASSERT_TRUE(co::CodeToIndex("a0001.CZCE") == 0);
    ASSERT_TRUE(co::CodeToIndex("AA0001.CZCE") != 0);
    ASSERT_TRUE(co::CodeToIndex("Aa0001.CZCE") == 0);
    ASSERT_TRUE(co::CodeToIndex("aA0001.CZCE") == 0);
    ASSERT_TRUE(co::CodeToIndex("aa0001.CZCE") == 0);

    ASSERT_TRUE(co::CodeToIndex("A0001.GFE") == 0);
    ASSERT_TRUE(co::CodeToIndex("a0001.GFE") != 0);
    ASSERT_TRUE(co::CodeToIndex("AA0001.GFE") == 0);
    ASSERT_TRUE(co::CodeToIndex("Aa0001.GFE") == 0);
    ASSERT_TRUE(co::CodeToIndex("aA0001.GFE") == 0);
    ASSERT_TRUE(co::CodeToIndex("aa0001.GFE") != 0);

    ASSERT_TRUE(co::CodeToIndex("A0001.INE") == 0);
    ASSERT_TRUE(co::CodeToIndex("a0001.INE") != 0);
    ASSERT_TRUE(co::CodeToIndex("AA0001.INE") == 0);
    ASSERT_TRUE(co::CodeToIndex("Aa0001.INE") == 0);
    ASSERT_TRUE(co::CodeToIndex("aA0001.INE") == 0);
    ASSERT_TRUE(co::CodeToIndex("aa0001.INE") != 0);

    ASSERT_TRUE(co::CodeToIndex("A0001.SGE") != 0);
    ASSERT_TRUE(co::CodeToIndex("a0001.SGE") == 0);
    ASSERT_TRUE(co::CodeToIndex("AA0001.SGE") != 0);
    ASSERT_TRUE(co::CodeToIndex("Aa0001.SGE") == 0);
    ASSERT_TRUE(co::CodeToIndex("aA0001.SGE") == 0);
    ASSERT_TRUE(co::CodeToIndex("aa0001.SGE") == 0);
}

TEST(CodeToIndex, Range) {
    // 边界测试
    // 0001表示2000年01月，是支持的最小合约月份
    // 8504表示2085年04月，是支持的最大合约月份。
    ASSERT_EQ(co::CodeToIndex("000000.SH"), co::kCodeIndexBeginSH);
    ASSERT_EQ(co::CodeToIndex("999999.SH"), co::kCodeIndexEndSH - 1);
    ASSERT_EQ(co::CodeToIndex("000000.SZ"), co::kCodeIndexBeginSZ);
    ASSERT_EQ(co::CodeToIndex("999999.SZ"), co::kCodeIndexEndSZ - 1);
    ASSERT_EQ(co::CodeToIndex("000000.BJ"), co::kCodeIndexBeginBJ);
    ASSERT_EQ(co::CodeToIndex("999999.BJ"), co::kCodeIndexEndBJ - 1);
    ASSERT_EQ(co::CodeToIndex("000000.CSI"), co::kCodeIndexBeginCSI);
    ASSERT_EQ(co::CodeToIndex("999999.CSI"), co::kCodeIndexEndCSI - 1);
    ASSERT_EQ(co::CodeToIndex("000000.CNI"), co::kCodeIndexBeginCNI);
    ASSERT_EQ(co::CodeToIndex("999999.CNI"), co::kCodeIndexEndCNI - 1);
    ASSERT_EQ(co::CodeToIndex("00000.HK"), co::kCodeIndexBeginHK);
    ASSERT_EQ(co::CodeToIndex("99999.HK"), co::kCodeIndexEndHK - 1);
    ASSERT_EQ(co::CodeToIndex("A0001.CFFEX"), co::kCodeIndexBeginCFFEX1);
    ASSERT_EQ(co::CodeToIndex("Z8504.CFFEX"), co::kCodeIndexEndCFFEX1 - 1);
    ASSERT_EQ(co::CodeToIndex("AA0001.CFFEX"), co::kCodeIndexBeginCFFEX2);
    ASSERT_EQ(co::CodeToIndex("ZZ8504.CFFEX"), co::kCodeIndexEndCFFEX2 - 1);
    ASSERT_EQ(co::CodeToIndex("a0001.SHFE"), co::kCodeIndexBeginSHFE1);
    ASSERT_EQ(co::CodeToIndex("z8504.SHFE"), co::kCodeIndexEndSHFE1 - 1);
    ASSERT_EQ(co::CodeToIndex("aa0001.SHFE"), co::kCodeIndexBeginSHFE2);
    ASSERT_EQ(co::CodeToIndex("zz8504.SHFE"), co::kCodeIndexEndSHFE2 - 1);
    ASSERT_EQ(co::CodeToIndex("a0001.DCE"), co::kCodeIndexBeginDCE1);
    ASSERT_EQ(co::CodeToIndex("z8504.DCE"), co::kCodeIndexEndDCE1 - 1);
    ASSERT_EQ(co::CodeToIndex("aa0001.DCE"), co::kCodeIndexBeginDCE2);
    ASSERT_EQ(co::CodeToIndex("zz8504.DCE"), co::kCodeIndexEndDCE2 - 1);
    ASSERT_EQ(co::CodeToIndex("A0001.CZCE"), co::kCodeIndexBeginCZCE1);
    ASSERT_EQ(co::CodeToIndex("Z8504.CZCE"), co::kCodeIndexEndCZCE1 - 1);
    ASSERT_EQ(co::CodeToIndex("AA0001.CZCE"), co::kCodeIndexBeginCZCE2);
    ASSERT_EQ(co::CodeToIndex("ZZ8504.CZCE"), co::kCodeIndexEndCZCE2 - 1);
    ASSERT_EQ(co::CodeToIndex("a0001.GFE"), co::kCodeIndexBeginGFE1);
    ASSERT_EQ(co::CodeToIndex("z8504.GFE"), co::kCodeIndexEndGFE1 - 1);
    ASSERT_EQ(co::CodeToIndex("aa0001.GFE"), co::kCodeIndexBeginGFE2);
    ASSERT_EQ(co::CodeToIndex("zz8504.GFE"), co::kCodeIndexEndGFE2 - 1);
    ASSERT_EQ(co::CodeToIndex("a0001.INE"), co::kCodeIndexBeginINE1);
    ASSERT_EQ(co::CodeToIndex("z8504.INE"), co::kCodeIndexEndINE1 - 1);
    ASSERT_EQ(co::CodeToIndex("aa0001.INE"), co::kCodeIndexBeginINE2);
    ASSERT_EQ(co::CodeToIndex("zz8504.INE"), co::kCodeIndexEndINE2 - 1);
    ASSERT_EQ(co::CodeToIndex("A0001.SGE"), co::kCodeIndexBeginSGE1);
    ASSERT_EQ(co::CodeToIndex("Z8504.SGE"), co::kCodeIndexEndSGE1 - 1);
    ASSERT_EQ(co::CodeToIndex("AA0001.SGE"), co::kCodeIndexBeginSGE2);
    ASSERT_EQ(co::CodeToIndex("ZZ8504.SGE"), co::kCodeIndexEndSGE2 - 1);
}

TEST(CodeToIndex, RealCode) {
    ASSERT_EQ(co::CodeToIndex("000001.SH"), co::kCodeIndexBeginSH + 1);
    ASSERT_EQ(co::IndexToCode(co::CodeToIndex("000001.SH")), "000001.SH");
    ASSERT_EQ(co::CodeToIndex("510300.SH"), co::kCodeIndexBeginSH + 510300);
    ASSERT_EQ(co::IndexToCode(co::CodeToIndex("510300.SH")), "510300.SH");

    ASSERT_EQ(co::CodeToIndex("000002.SZ"), co::kCodeIndexBeginSZ + 2);
    ASSERT_EQ(co::IndexToCode(co::CodeToIndex("000002.SZ")), "000002.SZ");
    ASSERT_EQ(co::CodeToIndex("159901.SZ"), co::kCodeIndexBeginSZ + 159901);
    ASSERT_EQ(co::IndexToCode(co::CodeToIndex("159901.SZ")), "159901.SZ");

    ASSERT_EQ(co::CodeToIndex("430017.BJ"), co::kCodeIndexBeginBJ + 430017);
    ASSERT_EQ(co::IndexToCode(co::CodeToIndex("430017.BJ")), "430017.BJ");

    ASSERT_EQ(co::CodeToIndex("000811.CSI"), co::kCodeIndexBeginCSI + 811);
    ASSERT_EQ(co::IndexToCode(co::CodeToIndex("000811.CSI")), "000811.CSI");

    ASSERT_EQ(co::CodeToIndex("980023.CNI"), co::kCodeIndexBeginCNI + 980023);
    ASSERT_EQ(co::IndexToCode(co::CodeToIndex("980023.CNI")), "980023.CNI");

    ASSERT_EQ(co::CodeToIndex("02966.HK"), co::kCodeIndexBeginHK + 2966);
    ASSERT_EQ(co::IndexToCode(co::CodeToIndex("02966.HK")), "02966.HK");

    ASSERT_EQ(co::IndexToCode(co::CodeToIndex("IF2504.CFFEX")), "IF2504.CFFEX");
    ASSERT_EQ(co::IndexToCode(co::CodeToIndex("au2505.SHFE")), "au2505.SHFE");
    ASSERT_EQ(co::IndexToCode(co::CodeToIndex("a2505.DCE")), "a2505.DCE");
    ASSERT_EQ(co::IndexToCode(co::CodeToIndex("AP2504.CZCE")), "AP2504.CZCE");
    ASSERT_EQ(co::IndexToCode(co::CodeToIndex("si2504.GFE")), "si2504.GFE");
    ASSERT_EQ(co::IndexToCode(co::CodeToIndex("sc2504.INE")), "sc2504.INE");
    ASSERT_EQ(co::IndexToCode(co::CodeToIndex("AU9999.SGE")), "");
}

TEST(CodeToIndex, CodeMap) {
    co::CodeMap<std::string_view, std::unique_ptr<co::MemQTickBody>> map;
    {
        auto q = std::make_unique<co::MemQTickBody>();
        strcpy(q->code, "510300.SH");
        q->new_volume = 100;
        map.Set(q->code, std::move(q));
    }
    {
        auto q = std::make_unique<co::MemQTickBody>();
        strcpy(q->code, "159901.SZ");
        q->new_volume = 200;
        map.Set(q->code, std::move(q));
    }
    {
        auto q = std::make_unique<co::MemQTickBody>();
        strcpy(q->code, "IF2504.CFFEX");
        q->new_volume = 300;
        map.Set(q->code, std::move(q));
    }
    {
        auto q = std::make_unique<co::MemQTickBody>();
        strcpy(q->code, "au2504.SHFE");
        q->new_volume = 400;
        map.Set(q->code, std::move(q));
    }
    ASSERT_EQ(map.Has("510300.SH", 0), true);
    ASSERT_EQ(map.Has("", co::CodeToIndex("510300.SH")), true);
    ASSERT_EQ(map.Get("510300.SH", 0) ? map.Get("510300.SH", 0)->new_volume : 0, 100);

    ASSERT_EQ(map.Has("159901.SZ", 0), true);
    ASSERT_EQ(map.Has("", co::CodeToIndex("159901.SZ")), true);
    ASSERT_EQ(map.Get("159901.SZ", 0) ? map.Get("159901.SZ", 0)->new_volume : 0, 200);

    ASSERT_EQ(map.Has("IF2504.CFFEX", 0), true);
    ASSERT_EQ(map.Has("", co::CodeToIndex("IF2504.CFFEX")), true);
    ASSERT_EQ(map.Get("IF2504.CFFEX", 0) ? map.Get("IF2504.CFFEX", 0)->new_volume : 0, 300);

    ASSERT_EQ(map.Has("au2504.SHFE", 0), true);
    ASSERT_EQ(map.Has("", co::CodeToIndex("au2504.SHFE")), true);
    ASSERT_EQ(map.Get("au2504.SHFE", 0) ? map.Get("au2504.SHFE", 0)->new_volume : 0, 400);
}


//int main(int argc, char** argv) {
//
////    std::cout << co::GenerateCodeIndexMagicSuffix() << std::endl;
////    std::cout << GenerateCodeIndexConstants() << std::endl;
//
//    ::testing::InitGoogleTest(&argc, argv);
//
//    int result = RUN_ALL_TESTS();
//
//    return result;
//}