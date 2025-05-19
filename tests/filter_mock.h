#pragma once

#include <gmock/gmock.h>
#include <pipeline/pipeline.h>

#include <memory>
#include <vector>

class MockFilter : public infodif::Filter {
   public:
    MOCK_METHOD(void, process, (std::shared_ptr<infodif::SharedData> data),
                (override));
};

struct SpyFilter : public infodif::Filter {
    SpyFilter(int id, std::vector<int>& log) : _id(id), _log(log) {}
    void process(std::shared_ptr<infodif::SharedData>) { _log.push_back(_id); }

   private:
    int _id;
    std::vector<int>& _log;
};