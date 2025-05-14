#include "iostream"
#include "pipeline.h"

using namespace infodif;

void sampleFunction(std::shared_ptr<SharedData> data)
{
    // std::cout << "I am a sample function..." << std::endl;
    int val = std::any_cast<int>((*data)["level"]);
    (*data)["level"] = val + 1;
};

int main()
{
    Pipeline pipeline("videopath");

    pipeline.add_tmp_filter(sampleFunction);
    pipeline.add_tmp_filter(sampleFunction);
    pipeline.add_tmp_filter(sampleFunction);
    pipeline.add_tmp_filter(sampleFunction);

    std::cout << "Size of filter vector: " << pipeline.get_caller_size() << std::endl;

    pipeline.start();

    std::string state = pipeline.get_state();
    std::cout << "Before " << state << std::endl;

    for (size_t i = 0; i < 5; ++i)
    {
        auto data = std::make_shared<SharedData>();
        (*data)["level"] = static_cast<int>(i);
        pipeline.push_data(data);
        std::string state = pipeline.get_state();
        std::cout << state << std::endl;
    };

    state = pipeline.get_state();
    std::cout << "After " << state << std::endl;

    pipeline.stop();

    state = pipeline.get_state();
    std::cout << state << std::endl;

    auto final_queue = pipeline.get_outout_queue();
    while (true)
    {
        auto data = final_queue->pop();
        if (!data)
            break;

        int val = std::any_cast<int>((*data)["level"]);
        std::cout << "Processed data: level = " << val << std::endl;
    }

    return 0;
};
