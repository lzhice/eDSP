/*
 * EasyDSP, A cross-platform Digital Signal Processing library written in modern C++.
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
 * You should have received a copy of the GNU General Public License along withº
 * this program.  If not, see <http://www.gnu.org/licenses/>
 *
 * Filename: chebyshev_i.hpp
 * Author: Mohammed Boujemaoui
 * Date: 30/8/2018
 */
#ifndef EASYDSP_CHEBYSHEV_I_HPP
#define EASYDSP_CHEBYSHEV_I_HPP

#include <easy/dsp/utilities/db2mag.hpp>
#include <easy/meta/expects.hpp>
#include <easy/dsp/math/complex.hpp>
#include <easy/dsp/math/constant.hpp>
#include <easy/dsp/filter/filter_types.hpp>
#include <easy/dsp/filter/iir/designer/bilinear/bandpass_transformer.hpp>
#include <easy/dsp/filter/iir/designer/bilinear/bandstop_transformer.hpp>
#include <easy/dsp/filter/iir/designer/bilinear/lowpass_transformer.hpp>
#include <easy/dsp/filter/iir/designer/bilinear/highpass_transformer.hpp>
#include <easy/dsp/filter/iir/designer/abstract_designer.hpp>

namespace easy { namespace dsp { namespace filter {

    namespace chebyshevI {
        struct LowPassAnalogDesigner {
            template <typename T, std::size_t MaxSize>
            void design(LayoutBase<T, MaxSize>& analog, std::size_t num_poles, T ripple_db) const {
                meta::expects(num_poles < MaxSize, "Index out of bounds");
                analog.reset();

                const auto eps     = std::sqrt(math::inv(std::exp(-ripple_db * 0.1 * constants<T>::ln_ten) - 1));
                const auto v0      = std::asinh(math::inv(eps)) / num_poles;
                const auto sinh_v0 = -std::sinh(v0);
                const auto cosh_v0 = std::cosh(v0);

                const auto size  = static_cast<T>(num_poles * 2);
                const auto pairs = num_poles / 2;
                for (auto i = 0ul; i < pairs; ++i) {
                    const auto k = 2 * i + 1 - num_poles;
                    const auto a = sinh_v0 * std::cos(k * constants<T>::pi / size);
                    const auto b = cosh_v0 * std::sin(k * constants<T>::pi / size);
                    analog.insert_conjugate(std::complex<T>(a, b), math::infinity<T>());
                }

                if (math::is_odd(num_poles)) {
                    analog.insert(std::complex<T>(sinh_v0, 0), math::infinity<T>());
                    analog.setNormalW(0);
                    analog.setNormalGain(1);
                } else {
                    analog.setNormalW(0);
                    analog.setNormalGain(dsp::db2mag(-ripple_db));
                }
            }
        };

        struct LowShelfAnalogDesigner {
            template <typename T, std::size_t MaxSize>
            void design(LayoutBase<T, MaxSize>& analog, std::size_t num_poles, T gain_db, T ripple_db) const {
                meta::expects(num_poles < MaxSize, "Index out of bounds");

                analog.setNormalW(constants<T>::pi);
                analog.setNormalGain(1);
                analog.reset();

                gain_db = -gain_db;
                if (ripple_db >= std::fabs(gain_db))
                    ripple_db = std::fabs(gain_db);
                if (gain_db < 0)
                    ripple_db = -ripple_db;

                const auto G  = dsp::db2mag(gain_db);
                const auto Gb = dsp::db2mag(gain_db - ripple_db);
                const auto G0 = 1;
                const auto g0 = std::pow(G0, math::inv(num_poles));
                const auto eps =
                    (Gb != G0) ? std::sqrt((math::square(G) - math::square(Gb)) / (math::square(Gb) - math::square(G0)))
                               : G - 1;
                const auto b =
                    std::pow(G / eps + Gb * std::sqrt(1 + math::inv(math::square(eps))), math::inv(num_poles));
                const auto u      = log(b / g0);
                const auto v      = log(pow(1. / eps + sqrt(1 + 1 / (eps * eps)), 1. / num_poles));
                const auto sinh_u = std::sinh(u);
                const auto sinh_v = std::sinh(v);
                const auto cosh_u = std::cosh(u);
                const auto cosh_v = std::cosh(v);
                const auto size   = static_cast<T>(2 * num_poles);
                const auto pairs  = num_poles / 2;
                for (auto i = 0ul; i < pairs; ++i) {
                    const auto a  = constants<T>::pi * (2 * i - 1) / size;
                    const auto sn = std::sin(a);
                    const auto cs = std::cos(a);
                    analog.insert_conjugate(std::complex<T>(-sn * sinh_u, cs * cosh_u),
                                            std::complex<T>(-sn * sinh_v, cs * cosh_v));
                }

                if (math::is_odd(num_poles)) {
                    analog.insert(std::complex<T>(-sinh_u), std::complex<T>(-sinh_v));
                }
            }
        };

        template <typename T, std::size_t MaxSize>
        class LowPass : public AbstractDesigner<T, LowPass<T, MaxSize>, MaxSize> {
            friend class AbstractDesigner<T, LowPass, MaxSize>;
            void operator()(std::size_t order, T sample_rate, T cuttoff_frequency, double ripple_db) {
                const auto normalized_frequency = cuttoff_frequency / sample_rate;
                LowPassAnalogDesigner{}.design(this->analog_, order, ripple_db);
                LowPassTransformer<T>{normalized_frequency}(this->analog_, this->digital_);
            }
        };

        template <typename T, std::size_t MaxSize>
        class HighPass : public AbstractDesigner<T, HighPass<T, MaxSize>, MaxSize> {
            friend class AbstractDesigner<T, HighPass, MaxSize>;
            void operator()(std::size_t order, T sample_rate, T cuttoff_frequency, double ripple_db) {
                const auto normalized_frequency = cuttoff_frequency / sample_rate;
                LowPassAnalogDesigner{}.design(this->analog_, order, ripple_db);
                HighPassTransformer<T>{normalized_frequency}(this->analog_, this->digital_);
            }
        };

        template <typename T, std::size_t MaxSize>
        class BandPass : public AbstractDesigner<T, BandPass<T, MaxSize>, MaxSize, 2 * MaxSize> {
            friend class AbstractDesigner<T, BandPass, MaxSize>;
            void operator()(std::size_t order, T sample_rate, T center_frequency, T bandwidth_frequency,
                            double ripple_db) {
                const auto normalized_center    = center_frequency / sample_rate;
                const auto normalized_bandwidth = bandwidth_frequency / sample_rate;
                LowPassAnalogDesigner{}.design(this->analog_, order, ripple_db);
                BandPassTransformer<T>{normalized_center, normalized_bandwidth}(this->analog_, this->digital_);
            }
        };

        template <typename T, std::size_t MaxSize>
        class BandStopPass : public AbstractDesigner<T, BandStopPass<T, MaxSize>, MaxSize, 2 * MaxSize> {
            friend class AbstractDesigner<T, BandStopPass, MaxSize>;
            void operator()(std::size_t order, T sample_rate, T center_frequency, T bandwidth_frequency,
                            double ripple_db) {
                const auto normalized_center    = center_frequency / sample_rate;
                const auto normalized_bandwidth = bandwidth_frequency / sample_rate;
                LowPassAnalogDesigner{}.design(this->analog_, order, ripple_db);
                BandStopTransformer<T>{normalized_center, normalized_bandwidth}(this->analog_, this->digital_);
            }
        };

        template <typename T, std::size_t MaxSize>
        class LowShelfPass : public AbstractDesigner<T, LowShelfPass<T, MaxSize>, MaxSize> {
            friend class AbstractDesigner<T, LowShelfPass, MaxSize>;
            void operator()(std::size_t order, T sample_rate, T cuttoff_frequency, T gain_db, double ripple_db) {
                const auto normalized_frequency = cuttoff_frequency / sample_rate;
                LowShelfAnalogDesigner{}.design(this->analog_, order, gain_db, ripple_db);
                LowPassTransformer<T>{normalized_frequency}(this->analog_, this->digital_);
            }
        };

        template <typename T, std::size_t MaxSize>
        class HighShelfPass : public AbstractDesigner<T, HighShelfPass<T, MaxSize>, MaxSize> {
            friend class AbstractDesigner<T, HighShelfPass, MaxSize>;
            void operator()(std::size_t order, T sample_rate, T cuttoff_frequency, T gain_db, double ripple_db) {
                const auto normalized_frequency = cuttoff_frequency / sample_rate;
                LowShelfAnalogDesigner{}.design(this->analog_, order, gain_db, ripple_db);
                HighPassTransformer<T>{normalized_frequency}(this->analog_, this->digital_);
            }
        };

        template <typename T, std::size_t MaxSize>
        class BandShelfPass : public AbstractDesigner<T, BandShelfPass<T, MaxSize>, MaxSize, 2 * MaxSize> {
            friend class AbstractDesigner<T, BandShelfPass, MaxSize, 2 * MaxSize>;
            void operator()(std::size_t order, T sample_rate, T center_frequency, T bandwidth_frequency, T gain_db,
                            double ripple_db) {
                const auto normalized_center    = center_frequency / sample_rate;
                const auto normalized_bandwidth = bandwidth_frequency / sample_rate;
                LowShelfAnalogDesigner{}.design(this->analog_, order, gain_db, ripple_db);
                BandPassTransformer<T>{normalized_center, normalized_bandwidth}(this->analog_, this->digital_);

                // HACK!
                this->digital_.setNormalW(normalized_center < 0.25 ? constants<T>::pi : 0);
                this->digital_.setNormalGain(1);
            }
        };
    } // namespace chebyshevI

    template <typename T, FilterType Type, std::size_t MaxOrder>
    struct ChebyshevIDesigner {};

    template <typename T, std::size_t MaxOrder>
    struct ChebyshevIDesigner<T, FilterType::LowPass, MaxOrder> {
        template <typename... Arg>
        constexpr auto operator()(Arg... arg)
            -> decltype(chebyshevI::LowPass<T, MaxOrder>{}.design(std::declval<Arg&&>()...)) {
            return chebyshevI::LowPass<T, MaxOrder>{}.design(arg...);
        }
    };

    template <typename T, std::size_t MaxOrder>
    struct ChebyshevIDesigner<T, FilterType::HighPass, MaxOrder> {
        template <typename... Arg>
        constexpr auto operator()(Arg... arg)
            -> decltype(chebyshevI::HighPass<T, MaxOrder>{}.design(std::declval<Arg&&>()...)) {
            return chebyshevI::HighPass<T, MaxOrder>{}.design(arg...);
        }
    };

    template <typename T, std::size_t MaxOrder>
    struct ChebyshevIDesigner<T, FilterType::BandPass, MaxOrder> {
        template <typename... Arg>
        constexpr auto operator()(Arg... arg)
            -> decltype(chebyshevI::BandPass<T, MaxOrder>{}.design(std::declval<Arg&&>()...)) {
            return chebyshevI::BandPass<T, MaxOrder>{}.design(arg...);
        }
    };

    template <typename T, std::size_t MaxOrder>
    struct ChebyshevIDesigner<T, FilterType::BandStop, MaxOrder> {
        template <typename... Arg>
        constexpr auto operator()(Arg... arg)
            -> decltype(chebyshevI::BandStopPass<T, MaxOrder>{}.design(std::declval<Arg&&>()...)) {
            return chebyshevI::BandStopPass<T, MaxOrder>{}.design(arg...);
        }
    };

    template <typename T, std::size_t MaxOrder>
    struct ChebyshevIDesigner<T, FilterType::LowShelf, MaxOrder> {
        template <typename... Arg>
        constexpr auto operator()(Arg... arg)
            -> decltype(chebyshevI::LowShelfPass<T, MaxOrder>{}.design(std::declval<Arg&&>()...)) {
            return chebyshevI::LowShelfPass<T, MaxOrder>{}.design(arg...);
        }
    };

    template <typename T, std::size_t MaxOrder>
    struct ChebyshevIDesigner<T, FilterType::HighShelf, MaxOrder> {
        template <typename... Arg>
        constexpr auto operator()(Arg... arg)
            -> decltype(chebyshevI::HighShelfPass<T, MaxOrder>{}.design(std::declval<Arg&&>()...)) {
            return chebyshevI::HighShelfPass<T, MaxOrder>{}.design(arg...);
        }
    };

    template <typename T, std::size_t MaxOrder>
    struct ChebyshevIDesigner<T, FilterType::BandShelf, MaxOrder> {
        template <typename... Arg>
        constexpr auto operator()(Arg... arg)
            -> decltype(chebyshevI::BandShelfPass<T, MaxOrder>{}.design(std::declval<Arg&&>()...)) {
            return chebyshevI::BandShelfPass<T, MaxOrder>{}.design(arg...);
        }
    };

}}} // namespace easy::dsp::filter

#endif // EASYDSP_CHEBYSHEV_I_HPP
