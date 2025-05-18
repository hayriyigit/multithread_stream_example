#pragma once

#include <gmock/gmock.h>
#include <pipeline/pipeline.h>


class MockFilter : public infodif::Filter{
    public:
        MOCK_METHOD(void, process, (std::shared_ptr<infodif::SharedData> data), (override));
};