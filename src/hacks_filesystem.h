#pragma once
#include "filesystem.h"
#include "core_api.h"
#include <algorithm>
#include <string>

class filesystem_foobar2000 : public filesystem_v3
{
public:
    filesystem_foobar2000() {}

    bool is_our_path(const char* p_path) override;
    bool get_canonical_path(const char* p_path, pfc::string_base& p_out) override;
    bool get_display_path(const char* p_path, pfc::string_base& p_out) override;
    void open(service_ptr_t<file>& p_out, const char* p_path, t_open_mode p_mode, abort_callback& p_abort) override;
    void remove(const char* p_path, abort_callback& p_abort) override;
    void move(const char* p_src, const char* p_dst, abort_callback& p_abort) override;
    bool is_remote(const char* p_src) override;
    void get_stats(const char* p_path, t_filestats& p_stats, bool& p_is_writeable, abort_callback& p_abort) override;
    void create_directory(const char* p_path, abort_callback& p_abort) override;
    void list_directory(const char* p_path, directory_callback& p_out, abort_callback& p_abort) override;
    bool supports_content_types() override;
    char pathSeparator() override;

private:
    std::string ExpandFoobar2000Path(const std::string& fb2kPath) const;
    std::string GetFoobar2000Root() const;
    std::string GetProfilePath() const;
    bool StartsWith(const std::string& str, const std::string& prefix) const;
};

static service_factory_single_t<filesystem_foobar2000> g_filesystem_foobar2000_factory;