#include "iostream"
#include "pipeline.h"

using namespace infodif;

void sampleFunction(std::shared_ptr<SharedData>){
    std::cout << "I am a sample function..." << std::endl;
};

int main(){
    Pipeline pipeline("videopath");
    

    pipeline.add_tmp_filter(sampleFunction);
    pipeline.add_tmp_filter(sampleFunction);
    pipeline.add_tmp_filter(sampleFunction);
    pipeline.add_tmp_filter(sampleFunction);

    std::cout << "Size of filter vector: " << pipeline.get_caller_size() << std::endl;

    pipeline.start();
    
    return 0;
}