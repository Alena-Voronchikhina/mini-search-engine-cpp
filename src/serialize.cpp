#include "mse/serialize.hpp"

#include <fstream>
#include <cstdint>

namespace mse {
namespace {

constexpr char kMagic[4] = {'M', 'S', 'E', 'I'};
constexpr std::uint32_t kVersion = 1;

template <typename T>
bool write_pod(std::ostream& out, const T& v) {
    out.write(reinterpret_cast<const char*>(&v), sizeof(T));
    return static_cast<bool>(out);
}

template <typename T>
bool read_pod(std::istream& in, T& v) {
    in.read(reinterpret_cast<char*>(&v), sizeof(T));
    return static_cast<bool>(in);
}

bool write_string(std::ostream& out, const std::string& s) {
    const auto n = static_cast<std::uint32_t>(s.size());
    if (!write_pod(out, n))
        return false;
    out.write(s.data(), static_cast<std::streamsize>(s.size()));
    return static_cast<bool>(out);
}

bool read_string(std::istream& in, std::string& s) {
    std::uint32_t n = 0;
    if (!read_pod(in, n))
        return false;
    s.resize(n);
    if (n)
        in.read(s.data(), static_cast<std::streamsize>(n));
    return static_cast<bool>(in);
}

} // namespace

bool save_index(const Index& index, const std::string& path) {
    std::ofstream out(path, std::ios::binary);
    if (!out)
        return false;
    out.write(kMagic, 4);
    if (!write_pod(out, kVersion))
        return false;

    const auto ndocs = static_cast<std::uint32_t>(index.docs_.size());
    if (!write_pod(out, ndocs))
        return false;
    if (!write_pod(out, index.avgdl_))
        return false;

    for (const auto& d : index.docs_) {
        if (!write_string(out, d.path))
            return false;
        if (!write_pod(out, d.length))
            return false;
    }

    const auto nterms = static_cast<std::uint32_t>(index.inv_.size());
    if (!write_pod(out, nterms))
        return false;
    for (const auto& [term, plist] : index.inv_) {
        if (!write_string(out, term))
            return false;
        const auto np = static_cast<std::uint32_t>(plist.size());
        if (!write_pod(out, np))
            return false;
        for (const auto& p : plist) {
            if (!write_pod(out, p.doc_id))
                return false;
            const auto npos = static_cast<std::uint32_t>(p.positions.size());
            if (!write_pod(out, npos))
                return false;
            for (Pos pos : p.positions) {
                if (!write_pod(out, pos))
                    return false;
            }
        }
    }
    return static_cast<bool>(out);
}

bool load_index(Index& index, const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in)
        return false;
    char magic[4]{};
    in.read(magic, 4);
    if (!in || magic[0] != kMagic[0] || magic[1] != kMagic[1] || magic[2] != kMagic[2] ||
        magic[3] != kMagic[3])
        return false;
    std::uint32_t ver = 0;
    if (!read_pod(in, ver) || ver != kVersion)
        return false;

    Index tmp;
    std::uint32_t ndocs = 0;
    if (!read_pod(in, ndocs))
        return false;
    if (!read_pod(in, tmp.avgdl_))
        return false;
    tmp.docs_.resize(ndocs);
    for (auto& d : tmp.docs_) {
        if (!read_string(in, d.path))
            return false;
        if (!read_pod(in, d.length))
            return false;
    }

    std::uint32_t nterms = 0;
    if (!read_pod(in, nterms))
        return false;
    for (std::uint32_t t = 0; t < nterms; ++t) {
        std::string term;
        if (!read_string(in, term))
            return false;
        std::uint32_t np = 0;
        if (!read_pod(in, np))
            return false;
        std::vector<Posting> plist;
        plist.resize(np);
        for (auto& p : plist) {
            if (!read_pod(in, p.doc_id))
                return false;
            std::uint32_t npos = 0;
            if (!read_pod(in, npos))
                return false;
            p.positions.resize(npos);
            for (auto& pos : p.positions) {
                if (!read_pod(in, pos))
                    return false;
            }
        }
        tmp.inv_.emplace(std::move(term), std::move(plist));
    }
    tmp.finalized_ = true;
    index = std::move(tmp);
    return true;
}

} // namespace mse
