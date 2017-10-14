//
// Created by Mohammed Boujemaoui on 12/10/2017.
//

#include "brownnoisegenerator.h"
#include "base/constants.h"
#include "base/types.h"

using namespace eDSP::generators;

template <typename T>
const std::vector<T> &BrownNoiseGenerator<T>::generate() {
    static T ns = static_cast<T>(0);
    for (size_t i = 0, size = Generator<T>::data.size(); i < size; ++i) {
        Generator<T>::data[i] = [&]() {
            bool compute = true;
            while (compute) {
                const T sample =  std::rand() / static_cast<T>(RAND_MAX);
                ns += sample;
                if (ns < -BrownInterval || ns > BrownInterval)
                    ns -= sample;
                else
                    compute = false;
            }
            return ns * BrownScale;
        }();
    }
    return Generator<T>::data;
}

template <typename T>
BrownNoiseGenerator<T>::BrownNoiseGenerator(size_t size) : Generator<T>(size) {}

EDSP_DCL_TEMPLATE(eDSP::generators::BrownNoiseGenerator, float);
EDSP_DCL_TEMPLATE(eDSP::generators::BrownNoiseGenerator, double);
