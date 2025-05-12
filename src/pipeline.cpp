#include "iostream"
#include "pipeline.h"

using namespace infodif;

Pipeline::Pipeline(const std::string& video_path)
{
    video_path_ = video_path;
    // set video cap object here
};

void Pipeline::start()
{
    auto data = std::make_shared<SharedData>();
    std::cout << "Pipeline has been started" << std::endl;
    for(size_t i = 0; i < this->m_callers.size(); ++i){
        auto& filter = this->m_callers[i];
        filter->process(data);
    }
};

int Pipeline::get_caller_size(){
    return this->m_callers.size();
}

void Pipeline::add_filter(std::unique_ptr<Filter> filter)
{
    m_callers.push_back(std::move(filter));
};

void Pipeline::add_tmp_filter(std::function<void(std::shared_ptr<SharedData>)> tmp_filter)
{
    class TemporaryFilter : public Filter {
        public:
            explicit TemporaryFilter(std::function<void(std::shared_ptr<SharedData>)> tmp_filter)
                : m_tmp_filter(std::move(tmp_filter)) {}

            void process(std::shared_ptr<SharedData> data) override {
                m_tmp_filter(data);
            }

        private:
            std::function<void(std::shared_ptr<SharedData>)> m_tmp_filter;
        };

    std::unique_ptr<Filter> filter = std::make_unique<TemporaryFilter>(tmp_filter);

    m_callers.push_back(std::move(filter));
}