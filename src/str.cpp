#include "pch.h"
#include "str.h"

namespace Utility
{
template <typename C>
inline void RemoveNullTerminator(C& str)
{
    if (str.back() == (typename C::value_type)'\0')
        str.pop_back();
}

static wstring_t CPToUTF16Internal(UINT cp, DWORD flags, string_view_t str)
{
    int requiredLength = ::MultiByteToWideChar(cp, flags, str.data(), static_cast<int>(str.size()), nullptr, 0);
    if (requiredLength <= 0)
        return {};

    wstring_t result;
    result.resize(requiredLength);
    if (requiredLength != ::MultiByteToWideChar(cp, flags, str.data(), static_cast<int>(str.size()), std::data(result), requiredLength))
        return {};

    RemoveNullTerminator(result);
    return result;
}

static string_t UTF16ToCPInternal(UINT cp, DWORD flags, wstring_view_t str)
{
    int requiredLength = ::WideCharToMultiByte(cp, flags, str.data(), static_cast<int>(str.size()), nullptr, 0, nullptr, nullptr);
    if (requiredLength <= 0)
        return {};

    string_t result;
    result.resize(requiredLength);
    if (requiredLength != ::WideCharToMultiByte(cp, flags, str.data(), static_cast<int>(str.size()), std::data(result), requiredLength, nullptr, nullptr))
        return {};

    RemoveNullTerminator(result);
    return result;
}

string_t ToUTF8(wstring_view_t str)
{
    return str.empty() ? string_t{} : UTF16ToCPInternal(CP_UTF8, 0, str);
}

wstring_t ToUTF16(string_view_t str)
{
    return str.empty() ? wstring_t{} : CPToUTF16Internal(CP_UTF8, 0, str);
}

} // namespace Utility
