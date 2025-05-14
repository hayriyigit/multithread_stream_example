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
    std::cout << "Pipeline has been started" << std::endl;

    if(this->queues.size() == this->m_callers.size()){
        queues.push_back(std::make_shared<ThreadSafeQueue<std::shared_ptr<SharedData>>>());
    }

    queues[0]->set_wait_callback([this](bool waiting) {
        if (waiting) {
            this->m_current_state = State::Idle;
        } else {
            this->m_current_state = State::Running;
        }
    });

    for (size_t i = 0; i < get_caller_size(); ++i)
    {
        auto input_queue = queues[i];
        auto output_queue = queues[i + 1];
        auto &filter = m_callers[i];

        threads.emplace_back([i, input_queue, output_queue, &filter]()
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
                                 }; });
    };

};

void Pipeline::stop(){
    queues[0]->push(nullptr);

    for (auto &t : threads)
    {
        std::cout << "Join..." << std::endl;
        t.join();
    };
    m_current_state = State::Paused;
};

int Pipeline::get_caller_size()
{
    return m_callers.size();
};

std::string Pipeline::get_state(){
    switch (this->m_current_state)
    {
        case State::Idle:
            return "idle";
        case State::Paused:
            return "paused";
        case State::Running:
            return "running";
        default:
            return "statenotfound";
    }
};

void Pipeline::push_data(std::shared_ptr<SharedData> data){
    queues[0]->push(data);
};

std::shared_ptr<ThreadSafeQueue<std::shared_ptr<SharedData>>> Pipeline::get_outout_queue(){
    return queues.back();
};

void Pipeline::add_filter(std::unique_ptr<Filter> filter)
{
    m_callers.push_back(std::move(filter));
    queues.push_back(std::make_shared<ThreadSafeQueue<std::shared_ptr<SharedData>>>());
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

    queues.push_back(std::make_shared<ThreadSafeQueue<std::shared_ptr<SharedData>>>());
}