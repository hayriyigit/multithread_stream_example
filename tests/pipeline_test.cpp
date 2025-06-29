#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <pipeline/pipeline.h>

#include <chrono>
#include <random>
#include <thread>

#include "filter_mock.h"

using namespace multimedia_pipeline;

using FunctionType = std::function<void(std::shared_ptr<SharedData>)>;

struct PipelineTest : testing::Test {
    Pipeline *p;

    PipelineTest() { p = new Pipeline; }

    ~PipelineTest() { delete p; }
};

TEST_F(PipelineTest, TestAddFilter_Class) {
    auto mf = std::make_unique<MockFilter>();
    auto *mf_ptr = mf.get();

    p->add_filter(std::move(mf));

    // Check pushed filter is Filter type
    {
        auto &m_callers = p->get_m_callers_for_test();
        const auto &uptr = std::get<std::unique_ptr<Filter>>(m_callers[0]);
        EXPECT_EQ(uptr.get(), mf_ptr);
    }

    auto expected = 1;
    auto actual = p->get_caller_size();

    ASSERT_EQ(expected, actual);
}

TEST_F(PipelineTest, TestAddFilter_Functional) {
    auto fn =
        std::make_unique<FunctionType>([&](std::shared_ptr<SharedData>) {});
    auto *fn_ptr = fn.get();

    p->add_tmp_filter(std::move(fn));

    // Check pushed filter is function type
    {
        auto &m_callers = p->get_m_callers_for_test();
        const auto &uptr =
            std::get<std::unique_ptr<FunctionType>>(m_callers[0]);
        EXPECT_EQ(uptr.get(), fn_ptr);
    }

    auto expected = 1;
    auto actual = p->get_caller_size();

    ASSERT_EQ(expected, actual);
}

TEST_F(PipelineTest, TestAddFilterOrder) {
    auto fn =
        std::make_unique<FunctionType>([&](std::shared_ptr<SharedData>) {});
    auto mf = std::make_unique<MockFilter>();

    auto *fn_ptr = fn.get();  // Pointer to the function type
    auto *mf_ptr = mf.get();  // Pointer to the Filter type

    p->add_tmp_filter(std::move(fn));  // Add the function first
    p->add_filter(std::move(mf));      // Filter class as the second

    auto &m_callers = p->get_m_callers_for_test();

    {
        const auto &uptr =
            std::get<std::unique_ptr<FunctionType>>(m_callers[0]);
        EXPECT_EQ(uptr.get(), fn_ptr);
    }

    {
        const auto &uptr = std::get<std::unique_ptr<Filter>>(m_callers[1]);
        EXPECT_EQ(uptr.get(), mf_ptr);
    }
}

TEST_F(PipelineTest, TestMCallerSize_Empty) {
    const int expected = 0;
    const int actual = p->get_caller_size();

    ASSERT_EQ(expected, actual);
}

TEST_F(PipelineTest, TestState_Initial) {
    auto expected = Pipeline::State::Paused;
    auto actual = p->state();

    EXPECT_EQ(expected, actual);
}

TEST_F(PipelineTest, TestState_Idle) {
    p->add_filter(std::make_unique<MockFilter>());

    p->start();

    sleep(1);  // Wait for the threads start

    auto expected = Pipeline::State::Idle;
    auto actual = p->state();

    EXPECT_EQ(expected, actual);

    p->stop();
}

TEST_F(PipelineTest, TestState_Running) {
    auto data = std::make_shared<SharedData>();
    auto fn = std::make_unique<FunctionType>([&](std::shared_ptr<SharedData>) {
        sleep(2);  // Something time consuming
    });
    p->add_tmp_filter(std::move(fn));

    p->push_data(data);

    p->start();

    sleep(1);  // Wait for the threads start

    auto expected = Pipeline::State::Running;
    auto actual = p->state();

    EXPECT_EQ(expected, actual);

    p->stop();
}

TEST_F(PipelineTest, TestPipelineCallOrder) {
    std::vector<int> order_log;
    auto data = std::make_shared<SharedData>();
    
    p->add_filter(std::make_unique<SpyFilter>(1, order_log));
    p->add_filter(std::make_unique<SpyFilter>(2, order_log));
    p->add_filter(std::make_unique<SpyFilter>(3, order_log));

    p->start();
    p->push_data(data);


    p->stop();

    EXPECT_EQ(order_log, std::vector<int>({1, 2, 3}));
}

TEST_F(PipelineTest, TestPiepelineBehavior) {
    auto data = std::make_shared<SharedData>();
    (*data)["debug"] = std::string{""};
    size_t filter_count(5);
    size_t data_count(50);

    for (size_t i = 0; i < filter_count; ++i) {
        auto fn = std::make_unique<FunctionType>(
            [i](std::shared_ptr<SharedData> data) {
                std::mt19937_64 eng{std::random_device{}()};
                std::uniform_int_distribution<> dist{10, 100};
                std::this_thread::sleep_for(
                    std::chrono::milliseconds{dist(eng)});
                std::string val = std::any_cast<std::string>((*data)["debug"]);
                (*data)["debug"] = val + std::to_string(i);
            });
        p->add_tmp_filter(std::move(fn));
    };

    p->start();

    std::vector<std::string> expected_vector;

    for (size_t i = 0; i < data_count; ++i) {
        auto data = std::make_shared<SharedData>();
        (*data)["debug"] = std::to_string(i);
        p->push_data(data);
        expected_vector.push_back(std::to_string(i) + "01234");
    };

    p->stop();

    auto result_queue = p->get_output_queue();
    for (const std::string &expected : expected_vector) {
        auto result = result_queue->pop();
        std::string actual = std::any_cast<std::string>((*result)["debug"]);
        EXPECT_EQ(expected, actual);
    };
}

TEST_F(PipelineTest, TestEmptyCallerThrow) {
    EXPECT_THROW(p->start();, PipelineExecutionException);
}

TEST_F(PipelineTest, TestEmptyThreadStop) { EXPECT_NO_THROW(p->stop()); }

int main(int argc, char **argv) {
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}