#pragma once

#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <thread>
#include <unordered_map>
#include <vector>
#include <any>

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

            Pipeline() = default;
            Pipeline(const std::string& video_path);
            /**
             * @brief Starts the pipeline execution.
             * @throws PipelineExecutionException if the pipeline fails to start.
             */
            void start();

            int get_caller_size();

            /**
             * @brief Adds a filter to the end of the pipeline.
             * @param filter Unique pointer to the filter to be added.
             */
            void add_filter(std::unique_ptr<Filter> filter);

            /**
             * @brief Adds a temporary filter function to the pipeline.
             * @param tmp_filter Function to be wrapped as a filter.
             */
            void add_tmp_filter(std::function<void(std::shared_ptr<SharedData>)> tmp_filter);

            /**
             * @brief Retrieves the current state of the pipeline.
             * @return Current state of the pipeline.
             */
            State state() const;

        private:
            std::vector<std::unique_ptr<Filter>> m_callers;
            State m_current_state{State::Idle};
            std::string video_path_;
    };

} // namespace infodif