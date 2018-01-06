#ifndef DIR_SYNC_INCLUDE_GUARD_CONFIG_HPP
#define DIR_SYNC_INCLUDE_GUARD_CONFIG_HPP

#include <boost/filesystem/path.hpp>

#include <cstdint>
#include <iosfwd>

class Config {
public:
    struct Version {
        std::uint32_t major;
        std::uint32_t minor;
        std::uint32_t patch;
    };
private:
    Version m_version;

    boost::filesystem::path m_configFilePath;
public:
    Config();

    Config(std::istream& is);

    static Version getCurrentVersion();

    void setConfigFilePath(boost::filesystem::path const& cfg_path);

    boost::filesystem::path getConfigFilePath() const;

    void serialize(std::ostream& os) const;
};

#endif
