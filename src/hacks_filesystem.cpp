#include "pch.h"
#include "hacks_filesystem.h"

// fb2k:// protocol handler
//Supported path formats:
// fb2k://root/... -> foobar2000 installation root directory (the directory where foobar2000.exe is located)
// fb2k://profile/... -> foobar2000 profile directory (automatically adapts to standard/portable mode)

bool filesystem_foobar2000::is_our_path(const char* p_path)
{
    return foobar2000_io::matchProtocol(p_path, "fb2k");
}

bool filesystem_foobar2000::get_canonical_path(const char* p_path, pfc::string_base& p_out)
{
    if (!is_our_path(p_path))
        return false;

    std::string expandedPath = ExpandFoobar2000Path(p_path);
    if (expandedPath.empty())
        return false;

    p_out = expandedPath.c_str();
    return true;
}

bool filesystem_foobar2000::get_display_path(const char* p_path, pfc::string_base& p_out)
{
    return get_canonical_path(p_path, p_out);
}

void filesystem_foobar2000::open(service_ptr_t<file>& p_out, const char* p_path, t_open_mode p_mode, abort_callback& p_abort)
{
    std::string expandedPath = ExpandFoobar2000Path(p_path);
    if (expandedPath.empty())
        throw exception_io_invalid_path();

    filesystem::ptr localFS = filesystem::getLocalFS();
    localFS->open(p_out, expandedPath.c_str(), p_mode, p_abort);
}

void filesystem_foobar2000::remove(const char* p_path, abort_callback& p_abort)
{
    std::string expandedPath = ExpandFoobar2000Path(p_path);
    if (expandedPath.empty())
        throw exception_io_invalid_path();

    filesystem::ptr localFS = filesystem::getLocalFS();
    localFS->remove(expandedPath.c_str(), p_abort);
}

void filesystem_foobar2000::move(const char* p_src, const char* p_dst, abort_callback& p_abort)
{
    std::string expandedSrc = ExpandFoobar2000Path(p_src);
    std::string expandedDst = ExpandFoobar2000Path(p_dst);
    
    if (expandedSrc.empty() || expandedDst.empty())
        throw exception_io_invalid_path();

    filesystem::ptr localFS = filesystem::getLocalFS();
    localFS->move(expandedSrc.c_str(), expandedDst.c_str(), p_abort);
}

bool filesystem_foobar2000::is_remote(const char* p_src)
{
    return false;
}

void filesystem_foobar2000::get_stats(const char* p_path, t_filestats& p_stats, bool& p_is_writeable, abort_callback& p_abort)
{
    std::string expandedPath = ExpandFoobar2000Path(p_path);
    if (expandedPath.empty())
        throw exception_io_invalid_path();

    filesystem::ptr localFS = filesystem::getLocalFS();
    localFS->get_stats(expandedPath.c_str(), p_stats, p_is_writeable, p_abort);
}

void filesystem_foobar2000::create_directory(const char* p_path, abort_callback& p_abort)
{
    std::string expandedPath = ExpandFoobar2000Path(p_path);
    if (expandedPath.empty())
        throw exception_io_invalid_path();

    filesystem::ptr localFS = filesystem::getLocalFS();
    localFS->create_directory(expandedPath.c_str(), p_abort);
}

void filesystem_fo