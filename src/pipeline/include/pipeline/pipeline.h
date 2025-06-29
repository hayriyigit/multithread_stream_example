#pragma once

#include <any>
#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <thread>
#include <unordered_map>
#include <variant>
#include <vector>

#include "threadsafequeue.h"

namespace multimedia_pipeline {

/**
 * @brief Type alias for shared data between filters
 */
using SharedData = std::unordered_map<std::string, std::any>;

/**
 * @brief Abstract base class representing a filter in the pipeline.
 */
class Filter {
   public:
    virtual ~Filter() = default;

    /**
     * @brief Process the shared data.
     *
     * @param data Shared data passed between filters.
     */
    virtual void process(std::shared_ptr<SharedData> data) = 0;

    /**
     * @brief Invoke the filter by calling the process method
     *
     * The operator allows a Filter instance to be used like a callable
     *
     * @param data A shared pointer to a SharedData containing the payload to be
     * processed
     */
    void operator()(std::shared_ptr<SharedData> data) { process(data); };
};

/**
 * @brief Custom exception thrown when pipeline execution fails.
 */
class PipelineExecutionException : public std::runtime_error {
   public:
    explicit PipelineExecutionException(const std::string &message)
        : std::runtime_error(message) {}
};

/**
 * @brief Pipeline class that manages and executes filters.
 */
class Pipeline {
   public:
    enum class State { Idle, Paused, Running };

    Pipeline();
    ~Pipeline();

    /**
     * @brief Starts the pipeline execution.
     *
     * @throws PipelineExecutionException if the pipeline fails to start.
     */
    void start();

    /**
     * @brief Stops the pipeline's processing threads.
     *
     * This method must be called
     */
    void stop();

    /**
     * @brief Enqueue a new item for processing
     *
     * @param data A shared pointer to a SharedData containing the payload to be
     * processed
     */
    void push_data(std::shared_ptr<SharedData> data);

    /**
     * @brief Retrieves the size of the filter vector
     *
     * @return Int size of m_callers the filter vector
     */
    int get_caller_size();

    /**
     * @brief  Retrives the filter vector for testing
     *
     * @return A const reference of m_callers the filter vector
     */
    const std::vector<std::variant<
        std::unique_ptr<Filter>,
        std::unique_ptr<std::function<void(std::shared_ptr<SharedData>)>>>> &
    get_m_callers_for_test() const;

    /**
     * @brief Retrieves the result queue
     *
     * @return A shared pointer to the ThreadSafeQueue instance
     */

    std::shared_ptr<ThreadSafeQueue<std::shared_ptr<SharedData>>>
    get_output_queue();

    /**
     * @brief Adds a filter to the end of the pipeline.
     *
     * @param filter Unique pointer to the filter to be added.
     */
    void add_filter(std::unique_ptr<Filter> filter);

    /**
     * @brief Adds a temporary filter function to the pipeline.
     *
     * @param tmp_filter Unique pointer to the function to be added.
     */
    void add_tmp_filter(
        std::unique_ptr<std::function<void(std::shared_ptr<SharedData>)>>
            tmp_filter);

    /**
     * @brief Retrieves the current state of the pipeline.
     *
     * @return Current state of the pipeline.
     */
    State state() const;

   private:
    /**
     * @brief Vector of filter variants
     *
     * A filter may be:
     *      - A unique pointer to a filter instance:    std::unique_ptr<Filter>
     *      - A unique pointer to a callback function:
     * std::unique_ptr<std::function<void(std::shared_ptr<SharedData>)>>>>
     */
    std::vector<std::variant<
        std::unique_ptr<Filter>,
        std::unique_ptr<std::function<void(std::shared_ptr<SharedData>)>>>>
        m_callers;

    /**
     * @brief Indicator of the pipelines operational state
     *
     * Indicates whether the pipeline is Idle, Running or Paused
     */
    State m_current_state{State::Paused};

    /**
     * @brief Path of video file to be processed
     */
    std::string video_path_;

    /**
     * @brief Vector of shared pointer to a ThreadSafeQueue instance.
     *
     *  Each queue buffers SharedData pointers between two consecutive filters:
     *   - queues[0] is the input queue for the first filter,
     *   - queues[i] feeds the (i+1)-th filter,
     *
     */
    std::vector<std::shared_ptr<ThreadSafeQueue<std::shared_ptr<SharedData>>>>
        queues;

    /**
     * @brief A holder vector of threads when the pipeline starts.
     */
    std::vector<std::thread> threads;

    /**
     * @brief A shared pointer to a ThreadSafeQueue instance.
     *
     * The queue holding the final processed data items. The consumer should
     * pop from this queue to retrieve the processed results.
     */
    std::shared_ptr<ThreadSafeQueue<std::shared_ptr<SharedData>>> result_queue;

};

};  // namespace multimedia_pipeline
