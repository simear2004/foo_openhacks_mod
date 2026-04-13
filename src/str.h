#pragma once

namespace Utility
{
string_t ToUTF8(wstring_view_t str);
wstring_t ToUTF16(string_view_t str);
} // namespace Utility