#include <asmjit/x86.h>
#include <stdio.h>
#include <unordered_map>
#include <gtest/gtest.h>
#include "arch_framework/dsl/feature/definition.h"

DEFINE_STRUCT(ValueTree, (int64_t)now,
              (int64_t)user,  // xx
              (int64_t)gid,
              (int64_t)time, 
              (int32_t)app_id, 
              (int64_t)play_count, 
              (std::vector<int64_t>)fids)

TEST(PARSETest, TestValueTree) {
    ModelFeature<InstanceProfile> mf;
    ASSERT_TRUE(mf.init("/home/wucunhua/repos/arch/arch_framework/conf/dsl/model_feature.conf"));
    InstanceProfile ins = {0, 0, 0, 0, 0, 0, {1, 2, 3, 4}};
    ASSERT_TRUE(mf.extract(ins));
    std::cout << dump_obj(ins) << "\n";
}
