#include "iostream"
#include "pipeline.h"
#include "threadsafequeue.h"

using namespace infodif;

Pipeline::Pipeline(const std::string &video_path)
{
    video_path_ = video_path;
    // set video cap object here
};

void Pipeline::start()
{
    std::vector<std::shared_ptr<ThreadSafeQueue<std::shared_ptr<SharedData>>>> queues;
    std::vector<std::thread> threads;

    std::cout << "Pipeline has been started" << std::endl;
    for (size_t i = 0; i <= this->m_callers.size(); ++i)
    {
        queues.push_back(std::make_shared<ThreadSafeQueue<std::shared_ptr<SharedData>>>());
    };

    for (size_t i = 0; i < this->m_callers.size(); ++i)
    {
        auto input_queue = queues[i];
        auto output_queue = queues[i + 1];
        auto &filter = this->m_callers[i];

        threads.emplace_back([input_queue, output_queue, &filter]()
                             {
                                 while (true)
                                 {
                                     auto data = input_queue->pop();
                                     if (!data) {
                                        output_queue->push(nullptr);
                                        break;
                                     };
                                     filter->process(data);
                                     output_queue->push(data);
                                 };
                             });
    };

    for (size_t i = 0; i < 5; ++i)
    {
        auto data = std::make_shared<SharedData>();
        (*data)["level"] = static_cast<int>(i);
        queues[0]->push(data);
    };

    queues[0]->push(nullptr);

    for (auto &t : threads)
    {
        std::cout << "Join..." << std::endl;
        t.join();
    };

    auto final_queue = queues.back();
    while (true) {
        auto data = final_queue->pop();
        if (!data) break;

        int val = std::any_cast<int>((*data)["level"]);
        std::cout << "Processed data: level = " << val << std::endl;
    }
};

int Pipeline::get_caller_size()
{
    return this->m_callers.size();
}

void Pipeline::add_filter(std::unique_ptr<Filter> filter)
{
    m_callers.push_back(std::move(filter));
};

void Pipeline::add_tmp_filter(std::function<void(std::shared_ptr<SharedData>)> tmp_filter)
{
    class TemporaryFilter : public Filter
    {
    public:
        explicit TemporaryFilter(std::function<void(std::shared_ptr<SharedData>)> tmp_filter)
            : m_tmp_filter(std::move(tmp_filter)) {}

        void process(std::shared_ptr<SharedData> data) override
        {
            m_tmp_filter(data);
        }

    private:
        std::function<void(std::shared_ptr<SharedData>)> m_tmp_filter;
    };

    std::unique_ptr<Filter> filter = std::make_unique<TemporaryFilter>(tmp_filter);

    m_callers.push_back(std::move(filter));
}