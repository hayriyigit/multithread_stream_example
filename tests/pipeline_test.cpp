#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <pipeline/pipeline.h>

#include "filter_mock.h"

using namespace infodif;

struct PipelineTest : testing::Test {
    Pipeline *p;

    PipelineTest() { p = new Pipeline; }

    ~PipelineTest() { delete p; }
};

TEST_F(PipelineTest, TestAddFilter_Class) {
    p->add_filter(std::make_unique<MockFilter>());

    auto expected = 1;
    auto actual = p->get_caller_size();

    ASSERT_EQ(expected, actual);
}

TEST_F(PipelineTest, TestAddFilter_Functional) {
    auto data = std::make_shared<SharedData>();

    auto fn =
        std::make_unique<std::function<void(std::shared_ptr<SharedData>)>>(
            [&](std::shared_ptr<SharedData>) {});

    p->add_tmp_filter(std::move(fn));

    auto expected = 1;
    auto actual = p->get_caller_size();

    ASSERT_EQ(expected, actual);
}

TEST_F(PipelineTest, TestAddFilterOrder) {
    auto data = std::make_shared<SharedData>();

    auto fn =
        std::make_unique<std::function<void(std::shared_ptr<SharedData>)>>(
            [&](std::shared_ptr<SharedData>) {});
    auto mf = std::make_unique<MockFilter>();

    auto *fn_ptr = fn.get();  // Pointer to the function type
    auto *mf_ptr = mf.get();  // Pointer to the Filter type

    p->add_tmp_filter(std::move(fn));  // Add the function first
    p->add_filter(std::move(mf));      // Filter class as the second

    auto &m_callers = p->get_m_callers_for_test();

    {
        const auto &uptr = std::get<
            std::unique_ptr<std::function<void(std::shared_ptr<SharedData>)>>>(
            m_callers[0]);
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
    auto fn =
        std::make_unique<std::function<void(std::shared_ptr<SharedData>)>>(
            [&](std::shared_ptr<SharedData>) {
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

TEST_F(PipelineTest, TestPiepelineBehavior) {
    auto data = std::make_shared<SharedData>();
    (*data)["debug"] = std::string{""};
    auto fn1 =
        std::make_unique<std::function<void(std::shared_ptr<SharedData>)>>(
            [&](std::shared_ptr<SharedData> data) {
                std::string val = std::any_cast<std::string>((*data)["debug"]);
                (*data)["debug"] = val + "1";
            });

    auto fn2 =
        std::make_unique<std::function<void(std::shared_ptr<SharedData>)>>(
            [&](std::shared_ptr<SharedData> data) {
                std::string val = std::any_cast<std::string>((*data)["debug"]);
                (*data)["debug"] = val + "2";
            });
    p->add_tmp_filter(std::move(fn1));
    p->add_tmp_filter(std::move(fn2));

    p->push_data(data);

    p->start();

    p->stop();

    auto result_queue = p->get_output_queue();
    auto result = result_queue->pop();

    std::string expected = "12";
    std::string actual = std::any_cast<std::string>((*result)["debug"]);

    EXPECT_EQ(expected, actual);
}

TEST_F(PipelineTest, TestEmptyCallerThrow) {
    EXPECT_THROW(p->start();, PipelineExecutionException);
}

TEST_F(PipelineTest, TestEmptyThreadStop) { EXPECT_NO_THROW(p->stop()); }

int main(int argc, char **argv) {
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}