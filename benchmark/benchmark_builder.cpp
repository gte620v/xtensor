/***************************************************************************
* Copyright (c) 2016, Johan Mabille, Sylvain Corlay and Wolf Vollprecht    *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#include <benchmark/benchmark.h>

#include "xtensor/xnoalias.hpp"
#include "xtensor/xtensor.hpp"
#include "xtensor/xarray.hpp"

namespace xt
{
    template <class T>
    inline auto builder_xarange(benchmark::State& state)
    {
        for (auto _ : state)
        {
            T res = xt::arange(0, 10000);
            benchmark::DoNotOptimize(res.storage().data());
        }
    }

    template <class T>
    inline auto builder_arange_xsimd(benchmark::State& state)
    {
        for (auto _ : state)
        {
            auto expr = xt::arange<double>(0, 10000);
            xtensor<double, 1> res(expr.shape());
            using simd_type = xsimd::batch<double, 4>;
            for (std::size_t i = 0; i < res.size(); i += 4)
            {
                res.template store_simd<aligned_mode, simd_type>(i, expr.template step_simd<simd_type>(std::array<std::size_t, 1>{i})); 
            }
        }
    }

    template <class T>
    inline auto builder_arange_pure_xsimd(benchmark::State& state)
    {
        for (auto _ : state)
        {
            xtensor<double, 1> res(std::array<std::size_t, 1>{10000});
            using simd_type = xsimd::batch<double, 4>;
            simd_type x(0., 1., 2., 3.), y(4.);
            for (std::size_t i = 0; i < res.size(); i += 4)
            {
                res.template store_simd<unaligned_mode, simd_type>(i, x); 
                x = x + y;
            }
         }
    }

    template <class T>
    inline auto builder_arange_xsimd_stepper(benchmark::State& state)
    {
        for (auto _ : state)
        {
            auto expr = xt::arange<double>(0, 10000);
            xtensor<double, 1> res(expr.shape());
            using simd_type = xsimd::batch<double, 4>;

            auto expr_stepper = expr.stepper_begin(expr.shape());
            auto res_stepper = res.stepper_begin(expr.shape());
            for (std::size_t i = 0; i < res.size(); i += 4)
            {
                res_stepper.template store_simd<simd_type>(expr_stepper.template step_simd<simd_type>());
            }
        }
    }

    template <class T>
    inline auto builder_xarange_manual(benchmark::State& state)
    {
        for (auto _ : state)
        {
            T res = T::from_shape({10000});
            for (std::size_t i = 0; i < 10000; ++i)
            {
                res.storage()[i] = i;
            }
            benchmark::DoNotOptimize(res.data());
        }
    }

    inline auto builder_iota_vector(benchmark::State& state)
    {
        for (auto _ : state)
        {
            xt::uvector<double> a {};
            a.resize(10000);
            std::iota(a.begin(), a.end(), 0);
            benchmark::DoNotOptimize(a.data());
        }
    }
    
    template <class T>
    inline auto builder_arange_for_loop_assign(benchmark::State& state)
    {
        for (auto _ : state)
        { 
            auto expr = xt::arange(0, 10000);
            T res = T::from_shape({10000});
            for (std::size_t i = 0; i < 10000; ++i)
            {
                res(i) = expr(i);
            }
            benchmark::DoNotOptimize(res.data());
        }
    }

    template <class T>
    inline auto builder_arange_for_loop_iter_assign(benchmark::State& state)
    {
        for (auto _ : state)
        {
            auto expr = xt::arange<double>(0, 10000);
            T res = T::from_shape({10000});
            auto xend = expr.cend();
            auto reit = res.begin();
            for (auto it = expr.cbegin(); it != xend; ++it)
            {
                *reit++ = *it;
            }
            benchmark::DoNotOptimize(res.data());
        }
    }

    template <class T>
    inline auto builder_arange_for_loop_iter_assign_backward(benchmark::State& state)
    {
        for (auto _ : state)
        {
            auto expr = xt::arange<double>(0, 10000);
            T res = T::from_shape({10000});
            auto xend = expr.cend();
            auto reit = res.begin();
            auto it = expr.cbegin();
            for(ptrdiff_t n = 10000; n > 0; --n)
            {
                *reit = *it;
                ++it;
                ++reit;
            }
            benchmark::DoNotOptimize(res.data());
        }
    }

    template <class T>
    inline auto builder_arange_assign_iterator(benchmark::State& state)
    {
        for (auto _ : state)
        {
            auto xa = xt::arange(0, 10000);
            T res = T::from_shape({10000});
            std::copy(xa.cbegin(), xa.cend(), res.begin());
            benchmark::DoNotOptimize(res.data());
        }
    }

    template <class T>
    inline auto builder_std_iota(benchmark::State& state)
    {
        for (auto _ : state)
        {
            T res = T::from_shape({10000});
            std::iota(res.begin(), res.end(), 0);
            benchmark::DoNotOptimize(res.data());
        }
    }

    inline auto builder_ones(benchmark::State& state)
    {
        for (auto _ : state)
        { 
            xt::xarray<double> res = xt::ones<double>({200, 200});
            benchmark::DoNotOptimize(res.data());
        }
    }

    inline auto builder_ones_strided_assign(benchmark::State& state)
    {
        for (auto _ : state)
        {
            xt::xarray<double> res;
            res.resize({200, 200});
            strided_assign(res, xt::ones<double>({200, 200}), std::true_type{});
            benchmark::DoNotOptimize(res.data());
        }
    }

    inline auto builder_ones_assign_iterator(benchmark::State& state)
    {
        auto xo = xt::ones<double>({200, 200});
        for (auto _ : state)
        {
            xt::xarray<double> res(xt::dynamic_shape<size_t>{200, 200});
            auto xo = xt::ones<double>({200, 200});
            std::copy(xo.begin(), xo.end(), res.begin());
            benchmark::DoNotOptimize(res.storage().data());
        }
    }

    inline auto builder_ones_expr_for(benchmark::State& state)
    {
        auto xo = xt::ones<double>({200, 200});

        for (auto _ : state)
        {
            xt::xtensor<double, 2> res(xt::static_shape<size_t, 2>({200, 200}));
            auto xo = xt::ones<double>({200, 200}) * 0.15;
            for (std::size_t i = 0; i < xo.shape()[0]; ++i)
                for (std::size_t j = 0; j < xo.shape()[1]; ++j)
                    res(i, j) = xo(i, j);
            benchmark::DoNotOptimize(res.storage().data());
        }
    }

    inline auto builder_ones_expr(benchmark::State& state)
    {
        auto xo = xt::ones<double>({200, 200});

        for (auto _ : state)
        {
            xt::xtensor<double, 2> res = xt::ones<double>({200, 200}) * 0.15;
            benchmark::DoNotOptimize(res.storage().data());
        }
    }

    inline auto builder_ones_expr_fill(benchmark::State& state)
    {
        auto xo = xt::ones<double>({200, 200});

        for (auto _ : state)
        {
            xt::xtensor<double, 2> res = xt::xtensor<double, 2>::from_shape({200, 200});
            std::fill(res.begin(), res.end(), 0.15);
            benchmark::DoNotOptimize(res.storage().data());
        }
    }

    inline auto builder_std_fill(benchmark::State& state)
    {
        for (auto _ : state)
        {
            xt::xarray<double> res(xt::dynamic_shape<std::size_t>{200, 200});
            std::fill(res.storage().begin(), res.storage().end(), 1.0);
            benchmark::DoNotOptimize(res.storage().data());
        }
    }

    // BENCHMARK_TEMPLATE(builder_xarange, xarray<double>);
    // BENCHMARK_TEMPLATE(builder_xarange, xtensor<double, 1>);
    // BENCHMARK_TEMPLATE(builder_arange_pure_xsimd, xtensor<double, 1>);
    // BENCHMARK_TEMPLATE(builder_arange_xsimd, xtensor<double, 1>);
    // BENCHMARK_TEMPLATE(builder_arange_xsimd_stepper, xtensor<double, 1>);
    // BENCHMARK_TEMPLATE(builder_xarange_manual, xarray<double>);
    // BENCHMARK_TEMPLATE(builder_xarange_manual, xtensor<double, 1>);
    // BENCHMARK_TEMPLATE(builder_arange_for_loop_assign, xarray<double>);
    // BENCHMARK_TEMPLATE(builder_arange_for_loop_assign, xtensor<double, 1>);

    // BENCHMARK_TEMPLATE(builder_arange_assign_iterator, xarray<double>);
    // BENCHMARK_TEMPLATE(builder_arange_assign_iterator, xtensor<double, 1>);
    // BENCHMARK_TEMPLATE(builder_arange_for_loop_iter_assign, xarray<double>);
    // BENCHMARK_TEMPLATE(builder_arange_for_loop_iter_assign_backward, xarray<double>);
    // BENCHMARK_TEMPLATE(builder_arange_for_loop_iter_assign, xtensor<double, 1>);
    // BENCHMARK_TEMPLATE(builder_arange_for_loop_iter_assign_backward, xtensor<double, 1>);
    // BENCHMARK_TEMPLATE(builder_std_iota, xarray<double>);
    // BENCHMARK(builder_iota_vector);
    BENCHMARK(builder_ones);
    BENCHMARK(builder_ones_strided_assign);
    BENCHMARK(builder_ones_assign_iterator);
    BENCHMARK(builder_ones_expr);
    BENCHMARK(builder_ones_expr_fill);
    BENCHMARK(builder_ones_expr_for);
    BENCHMARK(builder_std_fill);
}