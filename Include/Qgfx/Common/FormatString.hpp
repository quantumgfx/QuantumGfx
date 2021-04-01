#pragma once

#include <sstream>
#include <iomanip>

namespace Qgfx
{

    template <typename SSType>
    void FormatStrSS(SSType& ss)
    {
    }

    template <typename SSType, typename ArgType>
    void FormatStrSS(SSType& ss, const ArgType& Arg)
    {
        ss << Arg;
    }

    template <typename SSType, typename FirstArgType, typename... RestArgsType>
    void FormatStrSS(SSType& ss, const FirstArgType& FirstArg, const RestArgsType&... RestArgs)
    {
        FormatStrSS(ss, FirstArg);
        FormatStrSS(ss, RestArgs...); // recursive call using pack expansion syntax
    }

    template <typename... RestArgsType>
    std::string FormatString(const RestArgsType&... Args)
    {
        std::stringstream ss;
        FormatStrSS(ss, Args...);
        return ss.str();
    }

    template <typename Type>
    struct MemorySizeFormatter
    {
        MemorySizeFormatter(Type _size, int _precision, Type _ref_size) :
            size{ _size },
            precision{ _precision },
            ref_size{ _ref_size }
        {}
        Type size = 0;
        int  precision = 0;
        Type ref_size = 0;
    };

    template <typename Type>
    MemorySizeFormatter<Type> FormatMemorySize(Type _size, int _precision = 0, Type _ref_size = 0)
    {
        return MemorySizeFormatter<Type>{_size, _precision, _ref_size};
    }

    template <typename SSType, typename Type>
    void FormatStrSS(SSType& ss, const MemorySizeFormatter<Type>& Arg)
    {
        auto ref_size = Arg.ref_size != 0 ? Arg.ref_size : Arg.size;
        if (ref_size >= (1 << 30))
        {
            ss << std::fixed << std::setprecision(Arg.precision) << static_cast<double>(Arg.size) / double{ 1 << 30 } << " GB";
        }
        else if (ref_size >= (1 << 20))
        {
            ss << std::fixed << std::setprecision(Arg.precision) << static_cast<double>(Arg.size) / double{ 1 << 20 } << " MB";
        }
        else if (ref_size >= (1 << 10))
        {
            ss << std::fixed << std::setprecision(Arg.precision) << static_cast<double>(Arg.size) / double{ 1 << 10 } << " KB";
        }
        else
        {
            ss << Arg.size << (((Arg.size & 0x01) == 0x01) ? " Byte" : " Bytes");
        }
    }

}