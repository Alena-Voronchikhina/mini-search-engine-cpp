#include "mse/serialize.hpp"

#include "mse/compress.hpp"

#include <cstdint>
#include <cstring>
#include <fstream>
#include <vector>

namespace mse {
namespace detail {

constexpr char kMagic[4] = {'M', 'S', 'E', 'I'};
constexpr std::uint32_t kVersionUncompressed = 1;
constexpr std::uint32_t kVersionCompressed = 2;

struct Cursor {
    const std::uint8_t *cur{};
    const std::uint8_t *end{};

    template <typename T> bool read_pod(T &v) {
        if (static_cast<std::size_t>(end - cur) < sizeof(T))
            return false;
        std::memcpy(&v, cur, sizeof(T));
        cur += sizeof(T);
        return true;
    }

    bool read_string(std::string &s) {
        std::uint32_t n = 0;
        if (!read_pod(n) || static_cast<std::size_t>(end - cur) < n)
            return false;
        s.assign(reinterpret_cast<const char *>(cur), n);
        cur += n;
        return true;
    }

    bool read_bytes(std::vector<std::uint8_t> &bytes) {
        std::uint32_t n = 0;
        if (!read_pod(n) || static_cast<std::size_t>(end - cur) < n)
            return false;
        bytes.assign(cur, cur + n);
        cur += n;
        return true;
    }
};

template <typename T> bool write_pod(std::ostream &out, const T &v) {
    out.write(reinterpret_cast<const char *>(&v), sizeof(T));
    return static_cast<bool>(out);
}

bool write_string(std::ostream &out, const std::string &s) {
    const auto n = static_cast<std::uint32_t>(s.size());
    if (!write_pod(out, n))
        return false;
    out.write(s.data(), static_cast<std::streamsize>(s.size()));
    return static_cast<bool>(out);
}

bool write_bytes(std::ostream &out, const std::vector<std::uint8_t> &bytes) {
    const auto n = static_cast<std::uint32_t>(bytes.size());
    if (!write_pod(out, n))
        return false;
    if (n)
        out.write(reinterpret_cast<const char *>(bytes.data()), static_cast<std::streamsize>(n));
    return static_cast<bool>(out);
}

bool write_postings_v2(std::ostream &out, const std::vector<Posting> &plist) {
    std::vector<std::uint32_t> doc_ids;
    doc_ids.reserve(plist.size());
    for (const auto &p : plist)
        doc_ids.push_back(p.doc_id);
    if (!write_bytes(out, compress_sorted(doc_ids)))
        return false;
    const auto np = static_cast<std::uint32_t>(plist.size());
    if (!write_pod(out, np))
        return false;
    for (const auto &p : plist) {
        std::vector<std::uint32_t> positions(p.positions.begin(), p.positions.end());
        if (!write_bytes(out, compress_sorted(positions)))
            return false;
    }
    return true;
}

bool read_postings_v1(Cursor &in, std::vector<Posting> &plist) {
    std::uint32_t np = 0;
    if (!in.read_pod(np))
        return false;
    plist.resize(np);
    for (auto &p : plist) {
        if (!in.read_pod(p.doc_id))
            return false;
        std::uint32_t npos = 0;
        if (!in.read_pod(npos))
            return false;
        p.positions.resize(npos);
        for (auto &pos : p.positions) {
            if (!in.read_pod(pos))
                return false;
        }
    }
    return true;
}

bool read_postings_v2(Cursor &in, std::vector<Posting> &plist) {
    std::vector<std::uint8_t> docs_bytes;
    if (!in.read_bytes(docs_bytes))
        return false;
    std::vector<std::uint32_t> doc_ids;
    if (!decompress_sorted(docs_bytes, doc_ids))
        return false;
    std::uint32_t np = 0;
    if (!in.read_pod(np) || np != doc_ids.size())
        return false;
    plist.resize(np);
    for (std::uint32_t i = 0; i < np; ++i) {
        plist[i].doc_id = doc_ids[i];
        std::vector<std::uint8_t> pos_bytes;
        if (!in.read_bytes(pos_bytes))
            return false;
        std::vector<std::uint32_t> positions;
        if (!decompress_sorted(pos_bytes, positions))
            return false;
        plist[i].positions.assign(positions.begin(), positions.end());
    }
    return true;
}

} // namespace detail

struct IndexSerializer {
    static bool parse(detail::Cursor &in, Index &index) {
        char magic[4]{};
        if (static_cast<std::size_t>(in.end - in.cur) < 4)
            return false;
        std::memcpy(magic, in.cur, 4);
        in.cur += 4;
        if (magic[0] != detail::kMagic[0] || magic[1] != detail::kMagic[1] ||
            magic[2] != detail::kMagic[2] || magic[3] != detail::kMagic[3])
            return false;

        std::uint32_t ver = 0;
        if (!in.read_pod(ver))
            return false;
        if (ver != detail::kVersionUncompressed && ver != detail::kVersionCompressed)
            return false;

        Index tmp;
        std::uint32_t ndocs = 0;
        if (!in.read_pod(ndocs) || !in.read_pod(tmp.avgdl_))
            return false;
        tmp.docs_.resize(ndocs);
        for (auto &d : tmp.docs_) {
            if (!in.read_string(d.path) || !in.read_pod(d.length))
                return false;
        }

        std::uint32_t nterms = 0;
        if (!in.read_pod(nterms))
            return false;
        for (std::uint32_t t = 0; t < nterms; ++t) {
            std::string term;
            if (!in.read_string(term))
                return false;
            std::vector<Posting> plist;
            const bool ok = (ver == detail::kVersionCompressed)
                                ? detail::read_postings_v2(in, plist)
                                : detail::read_postings_v1(in, plist);
            if (!ok)
                return false;
            tmp.inv_.emplace(std::move(term), std::move(plist));
        }
        tmp.finalized_ = true;
        index = std::move(tmp);
        return true;
    }

    static bool save(const Index &index, std::ostream &out) {
        out.write(detail::kMagic, 4);
        if (!detail::write_pod(out, detail::kVersionCompressed))
            return false;
        const auto ndocs = static_cast<std::uint32_t>(index.docs_.size());
        if (!detail::write_pod(out, ndocs) || !detail::write_pod(out, index.avgdl_))
            return false;
        for (const auto &d : index.docs_) {
            if (!detail::write_string(out, d.path) || !detail::write_pod(out, d.length))
                return false;
        }
        const auto nterms = static_cast<std::uint32_t>(index.inv_.size());
        if (!detail::write_pod(out, nterms))
            return false;
        for (const auto &[term, plist] : index.inv_) {
            if (!detail::write_string(out, term) || !detail::write_postings_v2(out, plist))
                return false;
        }
        return static_cast<bool>(out);
    }
};

bool save_index(const Index &index, const std::string &path) {
    std::ofstream out(path, std::ios::binary);
    if (!out)
        return false;
    return IndexSerializer::save(index, out);
}

bool load_index_from_memory(Index &index, const std::uint8_t *data, std::size_t size) {
    if (!data || size == 0)
        return false;
    detail::Cursor cur{data, data + size};
    return IndexSerializer::parse(cur, index);
}

bool load_index(Index &index, const std::string &path) {
    std::ifstream in(path, std::ios::binary);
    if (!in)
        return false;
    in.seekg(0, std::ios::end);
    const auto sz = in.tellg();
    if (sz <= 0)
        return false;
    in.seekg(0, std::ios::beg);
    std::vector<std::uint8_t> buf(static_cast<std::size_t>(sz));
    in.read(reinterpret_cast<char *>(buf.data()), sz);
    if (!in)
        return false;
    return load_index_from_memory(index, buf.data(), buf.size());
}

} // namespace mse
