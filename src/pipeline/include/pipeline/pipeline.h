#pragma once

#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <thread>
#include <unordered_map>
#include <vector>
#include <any>
#include "threadsafequeue.h"
#include <variant>

namespace infodif
{

    // Type alias for shared data between filters
    using SharedData = std::unordered_map<std::string, std::any>;

    /**
     * @brief Abstract base class representing a filter in the pipeline.
     */
    class Filter
    {
    public:
        virtual ~Filter() = default;

        /**
         * @brief Process the shared data.
         * @param data Shared data passed between filters.
         */
        virtual void process(std::shared_ptr<SharedData> data) = 0;

        void operator()(std::shared_ptr<SharedData> data)
        {
            process(data);
        };
    };

    /**
     * @brief Custom exception thrown when pipeline execution fails.
     */
    class PipelineExecutionException : public std::runtime_error
    {
    public:
        explicit PipelineExecutionException(const std::string &message) : std::runtime_error(message) {}
    };

    /**
     * @brief Pipeline class that manages and executes filters.
     */
    class Pipeline
    {
    public:
        enum class State
        {
            Idle,
            Paused,
            Running
        };

        Pipeline();

        /**
         * @brief Starts the pipeline execution.
         * @throws PipelineExecutionException if the pipeline fails to start.
         */
        void start();

        void stop();
        void push_data(std::shared_ptr<SharedData> data);

        int get_caller_size();
        const std::vector<std::variant<
            std::unique_ptr<Filter>,
            std::unique_ptr<std::function<void(std::shared_ptr<SharedData>)>>>> & get_m_callers_for_test() const;
        std::shared_ptr<ThreadSafeQueue<std::shared_ptr<SharedData>>> get_output_queue();

        /**
         * @brief Adds a filter to the end of the pipeline.
         * @param filter Unique pointer to the filter to be added.
         */
        void add_filter(std::unique_ptr<Filter> filter);

        /**
         * @brief Adds a temporary filter function to the pipeline.
         * @param tmp_filter Unique pointer to the function to be added.
         */
        void add_tmp_filter(std::unique_ptr<std::function<void(std::shared_ptr<SharedData>)>> tmp_filter);

        /**
         * @brief Retrieves the current state of the pipeline.
         * @return Current state of the pipeline.
         */
        State state() const;

    private:
        std::vector<std::variant<
            std::unique_ptr<Filter>,
            std::unique_ptr<std::function<void(std::shared_ptr<SharedData>)>>>>
            m_callers;
        State m_current_state{State::Paused};
        std::string video_path_;
        std::vector<std::shared_ptr<ThreadSafeQueue<std::shared_ptr<SharedData>>>> queues;
        std::vector<std::thread> threads;
        std::shared_ptr<ThreadSafeQueue<std::shared_ptr<SharedData>>> result_queue;
    };

}; // namespace infodif
