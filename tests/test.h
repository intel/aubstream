/*
 * Copyright (C) 2022-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "tests/test_defaults.h"
#include "gtest/gtest.h"

#define TEST_REQUIRES(expr) \
    {                       \
        if (!(expr)) {      \
            GTEST_SKIP();   \
            return;         \
        }                   \
    }

#define STR(x) #x

#define FAMILY_TEST_(test_case_name, test_name, parent_class, parent_id, matcher)              \
    class GTEST_TEST_CLASS_NAME_(test_case_name, test_name) : public parent_class {            \
      public:                                                                                  \
        GTEST_TEST_CLASS_NAME_(test_case_name, test_name)                                      \
        () {}                                                                                  \
                                                                                               \
      private:                                                                                 \
        void testBodyHw();                                                                     \
                                                                                               \
        void TestBody() override {                                                             \
            if (matcher(::gpu.get()))                                                          \
                testBodyHw();                                                                  \
        }                                                                                      \
        void SetUp() override {                                                                \
            if (matcher(::gpu.get()))                                                          \
                parent_class::SetUp();                                                         \
            else                                                                               \
                GTEST_SKIP();                                                                  \
        }                                                                                      \
        void TearDown() override {                                                             \
            if (matcher(::gpu.get()))                                                          \
                parent_class::TearDown();                                                      \
        }                                                                                      \
        static ::testing::TestInfo *const test_info_ GTEST_ATTRIBUTE_UNUSED_;                  \
        GTEST_DISALLOW_COPY_AND_ASSIGN_(                                                       \
            GTEST_TEST_CLASS_NAME_(test_case_name, test_name));                                \
    };                                                                                         \
                                                                                               \
    ::testing::TestInfo *const GTEST_TEST_CLASS_NAME_(test_case_name, test_name)::test_info_ = \
        ::testing::internal::MakeAndRegisterTestInfo(                                          \
            STR(test_case_name), #test_name, NULL, NULL,                                       \
            ::testing::internal::CodeLocation(__FILE__, __LINE__),                             \
            (parent_id),                                                                       \
            parent_class::SetUpTestCase,                                                       \
            parent_class::TearDownTestCase,                                                    \
            new ::testing::internal::TestFactoryImpl<GTEST_TEST_CLASS_NAME_(                   \
                test_case_name, test_name)>);                                                  \
    void GTEST_TEST_CLASS_NAME_(test_case_name, test_name)::testBodyHw()

#define HWTEST_F(test_fixture, test_name, matcher)               \
    FAMILY_TEST_(test_fixture, test_name, test_fixture,          \
                 ::testing::internal::GetTypeId<test_fixture>(), \
                 matcher)

#define HWTEST_P(test_suite_name, test_name, matcher)                                                             \
    class GTEST_TEST_CLASS_NAME_(test_suite_name, test_name) : public test_suite_name {                           \
      public:                                                                                                     \
        GTEST_TEST_CLASS_NAME_(test_suite_name, test_name)                                                        \
        () {}                                                                                                     \
                                                                                                                  \
      private:                                                                                                    \
        void matchBody();                                                                                         \
        void TestBody() override {                                                                                \
            if (matcher(::gpu.get()))                                                                             \
                matchBody();                                                                                      \
        }                                                                                                         \
        void SetUp() override {                                                                                   \
            if (matcher(::gpu.get()))                                                                             \
                test_suite_name::SetUp();                                                                         \
            else                                                                                                  \
                GTEST_SKIP();                                                                                     \
        }                                                                                                         \
        void TearDown() override {                                                                                \
            if (matcher(::gpu.get()))                                                                             \
                test_suite_name::TearDown();                                                                      \
        }                                                                                                         \
        static int AddToRegistry() {                                                                              \
            ::testing::UnitTest::GetInstance()                                                                    \
                ->parameterized_test_registry()                                                                   \
                .GetTestCasePatternHolder<test_suite_name>(#test_suite_name,                                      \
                                                           ::testing::internal::CodeLocation(__FILE__, __LINE__)) \
                ->AddTestPattern(#test_suite_name, #test_name,                                                    \
                                 new ::testing::internal::TestMetaFactory<GTEST_TEST_CLASS_NAME_(                 \
                                     test_suite_name, test_name)>(),                                              \
                                 ::testing::internal::CodeLocation(__FILE__, __LINE__));                          \
            return 0;                                                                                             \
        }                                                                                                         \
        static int gtest_registering_dummy_;                                                                      \
        GTEST_DISALLOW_COPY_AND_ASSIGN_(GTEST_TEST_CLASS_NAME_(test_suite_name, test_name));                      \
    };                                                                                                            \
                                                                                                                  \
    int GTEST_TEST_CLASS_NAME_(test_suite_name, test_name)::gtest_registering_dummy_ =                            \
        GTEST_TEST_CLASS_NAME_(test_suite_name, test_name)::AddToRegistry();                                      \
    void GTEST_TEST_CLASS_NAME_(test_suite_name, test_name)::matchBody()

struct MatchMultiDevice {
    static bool moreThanOne(const aub_stream::Gpu *gpu) {
        return gpu->deviceCount > 1;
    }
    static bool moreThanTwo(const aub_stream::Gpu *gpu) {
        return gpu->deviceCount > 2;
    }
    static bool moreThanThree(const aub_stream::Gpu *gpu) {
        return gpu->deviceCount > 3;
    }
};

struct MatchMemory {
    static bool hasBank0(const aub_stream::Gpu *gpu);
};

struct HwMatcher {
    static bool coreBelowGen12Core(const aub_stream::Gpu *gpu) {
        return gpu->gfxCoreFamily < aub_stream::CoreFamily::Gen12lp;
    }

    static bool coreBelowEqualGen12Core(const aub_stream::Gpu *gpu) {
        return gpu->gfxCoreFamily <= aub_stream::CoreFamily::Gen12lp;
    }

    static bool coreAboveEqualXeHp(const aub_stream::Gpu *gpu) {
        return gpu->gfxCoreFamily >= aub_stream::CoreFamily::XeHpCore;
    }

    static bool coreAboveXeHpc(const aub_stream::Gpu *gpu) {
        return gpu->gfxCoreFamily > aub_stream::CoreFamily::XeHpcCore;
    }

    static bool coreBelowEqualXeHpc(const aub_stream::Gpu *gpu) {
        return gpu->gfxCoreFamily <= aub_stream::CoreFamily::XeHpcCore;
    }

    template <bool f1(const aub_stream::Gpu *), bool f2(const aub_stream::Gpu *)>
    static bool And(const aub_stream::Gpu *gpu) {
        return f1(gpu) && f2(gpu);
    }
    template <bool f(const aub_stream::Gpu *)>
    static bool Not(const aub_stream::Gpu *gpu) {
        return !f(gpu);
    }
};
