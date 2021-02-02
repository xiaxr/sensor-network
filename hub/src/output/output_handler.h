#pragma once

#include "../config.h"
#include "../measurement.h"

namespace xiaxr{
    namespace output{
        class OutputHandler{
            public:
            OutputHandler(){}
            virtual ~OutputHandler(){}
            
            virtual bool begin(const Configuration &config) = 0;
            virtual void process(const Measurement &measurement)=0;            
        };
    }
}