/*
 * eDSP, A cross-platform Digital Signal Processing library written in modern C++.
 * Copyright (C) 2018 Mohammed Boujemaoui Boulaghmoudi
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of  MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>
 *
 * File: benchmark_fft.cpp
 * Author: Mohammed Boujemaoui
 * Date: 10/10/2018
 */

#include <edsp/windowing.hpp>
#include <edsp/spectral/dft.hpp>
#include <edsp/spectral/internal/libfftw_impl.hpp>
#include <edsp/spectral/internal/libpffft_impl.hpp>
#include <edsp/converter/real2complex.hpp>
#include <benchmark/benchmark.h>

template <typename T>
void FFTWComputingRealFFT(benchmark::State& state) {
    const auto size     = state.range(0);
    const auto fft_size = edsp::spectral::make_fft_size(size);

    std::vector<T> input(size);
    std::vector<std::complex<T>> output(fft_size);
    edsp::windowing::hamming(std::begin(input), std::end(input));
    edsp::spectral::fftw_impl<T> impl(size);
    for (auto _ : state) {
        impl.dft(edsp::meta::data(input), edsp::meta::data(output));
    }
}

template <typename T>
void PFFFTComputingRealFFT(benchmark::State& state) {
    const auto size     = state.range(0);
    const auto fft_size = edsp::spectral::make_fft_size(size);

    std::vector<T> input(size);
    std::vector<std::complex<T>> output(fft_size);
    edsp::windowing::hamming(std::begin(input), std::end(input));
    edsp::spectral::pffft_impl<T> impl(size);
    for (auto _ : state) {
        impl.dft(edsp::meta::data(input), edsp::meta::data(output));
    }
}

template <typename T>
void FFTWComputingComplexFFT(benchmark::State& state) {
    const auto size = state.range(0);

    std::vector<T> window(size);
    std::vector<std::complex<T>> input(size), output(size);
    edsp::windowing::hamming(std::begin(window), std::end(window));
    edsp::converter::real2complex(std::cbegin(window), std::cend(window), std::begin(input));
    edsp::spectral::fftw_impl<T> impl(size);
    for (auto _ : state) {
        impl.dft(edsp::meta::data(input), edsp::meta::data(output));
    }
}

template <typename T>
void PFFFTComputingComplexFFT(benchmark::State& state) {
    const auto size = state.range(0);
    std::vector<T> window(size);
    std::vector<std::complex<T>> input(size), output(size);
    edsp::windowing::hamming(std::begin(window), std::end(window));
    edsp::converter::real2complex(std::cbegin(window), std::cend(window), std::begin(input));
    edsp::spectral::pffft_impl<T> impl(size);
    for (auto _ : state) {
        impl.dft(edsp::meta::data(input), edsp::meta::data(output));
    }
}

template <typename T>
void FFTWComputingComplexDCT(benchmark::State& state) {
    const auto size = state.range(0);
    std::vector<T> input(size), output(size);
    edsp::windowing::hamming(std::begin(input), std::end(input));
    edsp::spectral::fftw_impl<T> impl(size);
    for (auto _ : state) {
        impl.dct(edsp::meta::data(input), edsp::meta::data(output));
    }
}

template <typename T>
void PFFFTComputingComplexDCT(benchmark::State& state) {
    const auto size = state.range(0);
    std::vector<T> input(size), output(size);
    edsp::windowing::hamming(std::begin(input), std::end(input));
    edsp::spectral::pffft_impl<T> impl(size);
    for (auto _ : state) {
        impl.dct(edsp::meta::data(input), edsp::meta::data(output));
    }
}

template <typename T>
void FFTWComputingComplexDHT(benchmark::State& state) {
    const auto size = state.range(0);
    std::vector<T> input(size), output(size);
    edsp::windowing::hamming(std::begin(input), std::end(input));
    edsp::spectral::fftw_impl<T> impl(size);
    for (auto _ : state) {
        impl.dht(edsp::meta::data(input), edsp::meta::data(output));
    }
}

template <typename T>
void PFFFTComputingComplexDHT(benchmark::State& state) {
    const auto size = state.range(0);
    std::vector<T> input(size), output(size);
    edsp::windowing::hamming(std::begin(input), std::end(input));
    edsp::spectral::pffft_impl<T> impl(size);
    for (auto _ : state) {
        impl.dht(edsp::meta::data(input), edsp::meta::data(output));
    }
}


BENCHMARK_TEMPLATE(PFFFTComputingRealFFT, float)->Range(1 << 20, 1 << 20);
BENCHMARK_TEMPLATE(FFTWComputingRealFFT, float)->Range(1 << 20, 1 << 20);
BENCHMARK_TEMPLATE(PFFFTComputingComplexFFT, float)->Range(1 << 20, 1 << 20);
BENCHMARK_TEMPLATE(FFTWComputingComplexFFT, float)->Range(1 << 20, 1 << 20);
BENCHMARK_TEMPLATE(PFFFTComputingComplexDCT, float)->Range(1 << 10, 1 << 10);
BENCHMARK_TEMPLATE(FFTWComputingComplexDCT, float)->Range(1 << 10, 1 << 10);
BENCHMARK_TEMPLATE(PFFFTComputingComplexDHT, float)->Range(1 << 10, 1 << 10);
BENCHMARK_TEMPLATE(FFTWComputingComplexDHT, float)->Range(1 << 10, 1 << 10);
BENCHMARK_MAIN();
