#include <diff-match-patch.h>
#include <gtest/gtest.h>

TEST(DMPTest, basetest) {
    diff_match_patch<std::string> diff;
    std::string                   text1      = "Hello World";
    std::string                   text2      = "Hello!";
    auto                          patches    = diff.patch_make(text1, text2);
    auto                          patch_text = diff.patch_toText(patches);
    auto                          text3      = diff.patch_apply(patches, text1);
    EXPECT_EQ(text2, text3.first);
};