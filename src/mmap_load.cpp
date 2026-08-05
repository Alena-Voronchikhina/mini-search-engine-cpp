#include "mse/mmap_load.hpp"

#include "mse/serialize.hpp"

#include <cstdint>
#include <string>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace mse {

bool load_index_mmap(Index& index, const std::string& path) {
#if defined(_WIN32)
    HANDLE file = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
        return load_index(index, path);
    LARGE_INTEGER sz{};
    if (!GetFileSizeEx(file, &sz) || sz.QuadPart <= 0) {
        CloseHandle(file);
        return false;
    }
    HANDLE mapping = CreateFileMappingA(file, nullptr, PAGE_READONLY, 0, 0, nullptr);
    if (!mapping) {
        CloseHandle(file);
        return load_index(index, path);
    }
    void* view = MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, 0);
    if (!view) {
        CloseHandle(mapping);
        CloseHandle(file);
        return load_index(index, path);
    }
    const bool ok = load_index_from_memory(index, static_cast<const std::uint8_t*>(view),
                                           static_cast<std::size_t>(sz.QuadPart));
    UnmapViewOfFile(view);
    CloseHandle(mapping);
    CloseHandle(file);
    return ok;
#else
    const int fd = ::open(path.c_str(), O_RDONLY);
    if (fd < 0)
        return load_index(index, path);
    struct stat st {};
    if (fstat(fd, &st) != 0 || st.st_size <= 0) {
        ::close(fd);
        return false;
    }
    void* mapped =
        ::mmap(nullptr, static_cast<std::size_t>(st.st_size), PROT_READ, MAP_PRIVATE, fd, 0);
    ::close(fd);
    if (mapped == MAP_FAILED)
        return load_index(index, path);
    const bool ok = load_index_from_memory(index, static_cast<const std::uint8_t*>(mapped),
                                           static_cast<std::size_t>(st.st_size));
    ::munmap(mapped, static_cast<std::size_t>(st.st_size));
    return ok;
#endif
}

} // namespace mse
