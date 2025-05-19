#include "pipeline/pipeline.h"

#include "iostream"
#include "pipeline/threadsafequeue.h"

using namespace infodif;

Pipeline::Pipeline() {
    this->result_queue =
        std::make_shared<ThreadSafeQueue<std::shared_ptr<SharedData>>>();
};

void Pipeline::start() {
    if (this->get_caller_size() == 0) {
        throw PipelineExecutionException(
            "At least one filter need to run pipeline");
    }

    std::cout << "Pipeline has been started" << std::endl;

    this->queues[0]->set_wait_callback([this](bool waiting) {
        if (waiting) {
            this->m_current_state = State::Idle;
        } else {
            this->m_current_state = State::Running;
        };
    });

    for (size_t i = 0; i < this->get_caller_size(); ++i) {
        auto input_queue = this->queues[i];
        auto output_queue = (i + 1 != this->get_caller_size())
                                ? this->queues[i + 1]
                                : this->result_queue;
        auto &filter = this->m_callers[i];

        std::visit(
            [&](auto &visit_pointer) {
                auto *raw_pointer = visit_pointer.get();
                this->threads.emplace_back(
                    [input_queue, output_queue, raw_pointer]() {
                        while (true) {
                            auto data = input_queue->pop();
                            if (!data) {
                                output_queue->push(nullptr);
                                break;
                            };

                            (*raw_pointer)(data);
                            output_queue->push(data);
                        };
                    });
            },
            filter);
    };
};

void Pipeline::stop() {
    if (!queues.empty()) {
        this->queues[0]->push(nullptr);
    };

    if (!threads.empty()) {
        for (auto &t : threads) {
            if (t.joinable()) {
                std::cout << "Join..." << std::endl;
                t.join();
            };
        };
    };

    this->threads.clear();

    this->m_current_state = State::Paused;
};

int Pipeline::get_caller_size() { return this->m_callers.size(); };

void Pipeline::push_data(std::shared_ptr<SharedData> data) {
    if (this->get_caller_size() == 0) {
        throw PipelineExecutionException(
            "At least one filter need to run pipeline");
    }
    this->queues[0]->push(data);
};

std::shared_ptr<ThreadSafeQueue<std::shared_ptr<SharedData>>>
Pipeline::get_output_queue() {
    return this->result_queue;
};

const std::vector<std::variant<
    std::unique_ptr<Filter>,
    std::unique_ptr<std::function<void(std::shared_ptr<SharedData>)>>>> &
Pipeline::get_m_callers_for_test() const {
    return this->m_callers;
};

Pipeline::State Pipeline::state() const { return m_current_state; };

void Pipeline::add_filter(std::unique_ptr<Filter> filter) {
    this->m_callers.push_back(std::move(filter));
    this->queues.push_back(
        std::make_shared<ThreadSafeQueue<std::shared_ptr<SharedData>>>());
};

void Pipeline::add_tmp_filter(
    std::unique_ptr<std::function<void(std::shared_ptr<SharedData>)>>
        tmp_filter) {
    this->m_callers.push_back(std::move(tmp_filter));

    this->queues.push_back(
        std::make_shared<ThreadSafeQueue<std::shared_ptr<SharedData>>>());
};
