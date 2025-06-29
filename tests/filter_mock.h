#pragma once

#include <gmock/gmock.h>
#include <pipeline/pipeline.h>

#include <memory>
#include <vector>

class MockFilter : public multimedia_pipeline::Filter {
   public:
    MOCK_METHOD(void, process, (std::shared_ptr<multimedia_pipeline::SharedData> data),
                (override));
};

struct SpyFilter : public multimedia_pipeline::Filter {
    SpyFilter(int id, std::vector<int>& log) : _id(id), _log(log) {}
    void process(std::shared_ptr<multimedia_pipeline::SharedData>) { _log.push_back(_id); }

   private:
    int _id;
    std::vector<int>& _log;
};