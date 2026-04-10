#pragma once
#include "pch.h"
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

    void move_overwrite(const char* src, const char* dst, abort_callback& abort) override;
    void make_directory(const char* path, abort_callback& abort, bool* didCreate = nullptr) override;
    bool directory_exists(const char* path, abort_callback& abort) override;
    bool file_exists(const char* path, abort_callback& abort) override;
    char pathSeparator() override;
    void list_directory_ex(const char* p_path, directory_callback& p_out, unsigned listMode, abort_callback& p_abort) override;

    t_filestats2 get_stats2(const char* p_path, uint32_t s2flags, abort_callback& p_abort) override;
    void list_directory_v3(const char* path, directory_callback_v3& callback, unsigned listMode, abort_callback& p_abort) override;

private:
    std::string ExpandFoobar2000Path(const std::string& fb2kPath) const;
    std::string GetFoobar2000Root() const;
    std::string GetProfilePath() const;
    bool StartsWith(const std::string& str, const std::string& prefix) const;
    
    filesystem_v3::ptr getLocalFSv3();
};

static service_factory_single_t<filesystem_foobar2000> g_filesystem_foobar2000_factory;
