#include <algorithm>
#include <autodiff/reverse/var.hpp>
#include <gcem/gcem.hpp>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <span>
#include <sstream>
#include <unordered_map>
using namespace autodiff;

template<typename Type>
concept numeric_type = std::is_arithmetic_v<Type>;

constexpr std::size_t No = 3;
constexpr std::size_t HIndex = 0;
//compile time cache size
constexpr std::size_t CTCR = 10;

constexpr std::array<std::size_t, 4> precisions = {6, 5, 7, 10};
constexpr std::array<const char*, No> elements = {"Hydrogen", "Nitrogen", "Oxygen"};
using decimal_type = double;
constexpr decimal_type None(std::numeric_limits<double>::quiet_NaN());
constexpr decimal_type c1(0.48508), c2(1.55174), c3(0.1561), c1H(1.202), c2H(-0.30228);
constexpr std::array<decimal_type, No> omegas = {decimal_type(None), decimal_type(0.04), decimal_type(0.022)};
constexpr std::array<decimal_type, No> tcs = {decimal_type(33.2), decimal_type(126.2), decimal_type(154.6)};

// container for runtime cache
auto rtcrs = std::unordered_map<decimal_type, std::array<decimal_type, tcs.size()>>{};

template<typename Ttype>
constexpr auto alpha_H(const Ttype T, const std::size_t H)
{
    if constexpr(numeric_type<decltype(T)>)
        if(std::is_constant_evaluated())
            return c1H * gcem::exp(c2H * (T / tcs[H]));
    return c1H * exp(c2H * (T / tcs[H]));
}

//typedef dual (*rt_ftype)(dual, const decimal_type, const decimal_type);
template<typename Ttype, typename TcType, typename OmegaType>
constexpr auto alpha_elem(const Ttype T, const TcType Tc, const OmegaType omega) -> std::decay_t<decltype(T)>
{
    if constexpr(numeric_type<decltype(T)>)
        if(std::is_constant_evaluated()) {
            decltype(T) r = 1 + (c1 + c2 * omega - c3 * omega * omega) * (1 - gcem::sqrt(T / Tc));
            return r * r;
        }
    decltype(T) r = 1 + (c1 + c2 * omega - c3 * omega * omega) * (1 - sqrt(T / Tc));
    return r * r;
}

constexpr auto calculateAlphas(const numeric_type auto T, const std::size_t H)
{
    std::array<decimal_type, No> arr;
    auto omega = omegas.begin();
    auto tc = tcs.begin();
    for(auto index(0U); index < arr.size(); ++index) {
        if(index == H)
            arr[index] = alpha_H(T, H);
        else
            arr[index] = alpha_elem(T, *tc, *omega);
        ++tc;
        ++omega;
    }
    return arr;
}
//global CompileTimeCalculatedResults stored in constexpr container, ctcrs, e.g. precalculated cache
constexpr std::array<std::pair<decimal_type, std::array<decimal_type, No>>, CTCR> ctcrs =
    {std::pair{decimal_type(300), calculateAlphas(decimal_type(300), HIndex)},
     {None, {}}};

auto calculateVars(var T, std::span<const decimal_type> Tc, std::span<const decimal_type> Os, const std::size_t H)
{
    std::array<var, No> arr;
    auto omega = Os.begin();
    auto tc = Tc.begin();
    for(auto index(0U); index < arr.size(); ++index) {
        if(index == H)
            arr[index] = alpha_H(T, H);
        else
            arr[index] = alpha_elem(T, *tc, *omega);
        ++tc;
        ++omega;
    }
    return arr;
}

template<typename C, typename Ttype>
void printData(const C& c, const Ttype T, const std::size_t order)
{
    auto tstr = (std::ostringstream() << std::fixed << std::setprecision(2) << T).str();
    if(!order) {
        std::cout << std::setw(32) << std::left << "Task 1 and 2" << std::setw(1) << '|' << std::setw(15);
        std::ranges::for_each(elements, [](const auto& el) { std::cout << std::right << std::setw(15) << el << '|'; });
        std::cout << '\n';
    }
    auto p{precisions[order]};
    auto message = order ? std::string("d").append(std::to_string(order)).append("\\alpha/").append("dT") : std::string("alpha values");
    std::cout << std::setw(32) << std::fixed << std::setprecision(2) << std::left << std::string("T(").append(tstr).append(") ").append(message);
    auto out = std::ostream_iterator<typename C::value_type>(std::cout, "|");
    std::cout << std::setw(1) << std::left << std::fixed << '|';
    if(p < 8)
        std::cout << std::setprecision(p);
    else
        std::cout << std::scientific;
    std::ranges::for_each(c, [&out](const auto& el) {std::cout<<std::setw(15)<<std::right;*out=el; });
    std::cout << '\n';
}

using ResultSet = std::array<decimal_type, No>;
using ResultDataSet = std::span<ResultSet>;
void alpha(decimal_type T, std::span<const decimal_type> Tc,
           std::span<const decimal_type> omega, const std::size_t H,
           ResultDataSet result)
{
    const auto resultDataSetSize = result.size();
    if(resultDataSetSize == 0)
        return;
    auto resultSetIter = result.begin();
    // first, search precalculated cache ctcrs for Temperature
    bool found(false);
    for(auto ctcrs_iter = ctcrs.begin(); ctcrs_iter != ctcrs.end() && ctcrs_iter->first != None; ++ctcrs_iter) {
        if(const auto& [temp, alphas] = *ctcrs_iter; temp == T) {
            //populate result
            std::ranges::copy(alphas, resultSetIter->begin());
            auto tstr = (std::ostringstream() << std::fixed << std::setprecision(2) << T).str();
            std::cout << "results for values for T(" << tstr << ") found in CompileTime cache\n";
            found = true;
        }
    }
    if(false == found) {
        // now, search runtime cache  rtcrs for Temperature
        // finally calculate coeficients, if we had no cached data
        // and populate runtime cache
        auto ValPairIter = rtcrs.try_emplace(T, calculateAlphas(T, H));
        if(!ValPairIter.second)
            std::cout << "results found in RunTimeCache\n";
        else
            std::cout << "new calculation of result set\n";
        //populate result
        std::ranges::copy(ValPairIter.first->second, resultSetIter->begin());
    }
    if(resultDataSetSize == 1)
        return;
    ++resultSetIter;
    var Tt = T;
    auto rtelem = calculateVars(Tt, Tc, omega, H);
    auto rtelem1 = decltype(rtelem){};
    auto resultSetIndex(0U);
    auto resultSetElemIter = resultSetIter->begin();
    for(auto& elem : rtelem) {
        rtelem1[resultSetIndex] = derivativesx(elem, wrt(Tt))[0];
        *(resultSetElemIter++) = val(rtelem1[resultSetIndex++]);
    }
    if(resultDataSetSize == 2)
        return;
    ++resultSetIter;
    auto rtelem2 = decltype(rtelem1){};
    resultSetElemIter = resultSetIter->begin();
    resultSetIndex = 0U;
    for(auto& elem : rtelem1) {
        rtelem2[resultSetIndex] = derivativesx(elem, wrt(Tt))[0];
        *(resultSetElemIter++) = val(rtelem2[resultSetIndex++]);
    }
    if(resultDataSetSize == 3)
        return;
    ++resultSetIter;
    auto rtelem3 = decltype(rtelem2){};
    resultSetElemIter = resultSetIter->begin();
    resultSetIndex = 0U;
    for(auto& elem : rtelem2) {
        rtelem3[resultSetIndex] = derivativesx(elem, wrt(Tt))[0];
        *(resultSetElemIter++) = val(rtelem3[resultSetIndex++]);
    }
    if(resultDataSetSize == 4)
        return;
    ++resultSetIter;
    resultSetElemIter = resultSetIter->begin();
    for(auto& elem : rtelem3) {
        *(resultSetElemIter++) = val(derivativesx(elem, wrt(Tt))[0]);
    }
}

auto test()
{
    //-------------------------------
    std::cout << ("*** simple T E S T ***\n\n");
    std::cout << "content of compile time calculated cache:\n";
    for(const auto& [T, alphas] : ctcrs) {
        if(std::isnan(T))
            break;
        printData(alphas, T, 0);
    }

    std::cout << "test compile time calculation\n";
    constexpr double cT = 300;
    constexpr double ctN = alpha_elem(cT, tcs[1], omegas[1]);
    constexpr double ctH = alpha_H(cT, 0);
    std::cout << "ctH = " << ctH << '\n';
    std::cout << "ctN = " << ctN << '\n';
    auto rtH = alpha_H(cT, 0);
    var T = 300;
    var rtN = alpha_elem(T, tcs[1], omegas[1]);
    //auto alpha_elemptr = alpha_elem<var, decimal_type, decimal_type>;
    //double dudT = derivative(alpha_elemptr, wrt(T), at(T, tcs[1], omegas[1]));
    //auto dudT = derivativesx(alpha_elemptr, wrt(T));
    auto [dudT] = derivativesx(rtN, wrt(T));

    std::cout << "test runtime time calculation\n";
    std::cout << "rtH = " << rtH << '\n';
    std::cout << "rtN = " << rtN << '\n';
    std::cout << "(T=300,N) d\\alpha/dT = " << dudT << '\n';
    //-----------------------------------
    std::cout << ("*** END T E S T ***\n\n");
}
auto main(int argc, char** argv) -> int
{
    if(argc == 2 && std::string(argv[1]) == "test") {
        test();
        return 0;
    }
    auto result = std::array<ResultSet, 4>{};
    auto T = decimal_type(300);
    alpha(T, tcs, omegas, 0, result);
    auto order = 0U;
    for(const auto& resultSet : result) {
        printData(resultSet, T, order++);
    }
    std::cout << '\n';

    return 0;
}
