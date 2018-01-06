#include <config.hpp>

#include <exceptions.hpp>

#include <cstring>
#include <istream>
#include <ostream>
#include <string>

namespace {
char const* g_configHeaderString = "DIR_SYNC_CFG";
}

Config::Config()
    :m_version(getCurrentVersion())
{
}

Config::Config(std::istream& is)
{
    char header_buffer[sizeof(g_configHeaderString)];
    is.read(header_buffer, sizeof(header_buffer));
    if(std::strncmp(header_buffer, g_configHeaderString, sizeof(header_buffer)) != 0) {
        GHULBUS_THROW(IOError(), "Invalid config file.");
    }

    is.read(reinterpret_cast<char*>(&m_version.major), sizeof(std::uint32_t));
    is.read(reinterpret_cast<char*>(&m_version.minor), sizeof(std::uint32_t));
    is.read(reinterpret_cast<char*>(&m_version.patch), sizeof(std::uint32_t));

    if((m_version.major == 1) && (m_version.minor == 0) && (m_version.patch == 0)) {
    } else {
        GHULBUS_THROW(IOError(), "Unknown version " + std::to_string(m_version.major) +
                                 "." + std::to_string(m_version.minor) +
                                 "." + std::to_string(m_version.patch));
    }

}

Config::Version Config::getCurrentVersion()
{
    return Version{ 1, 0, 0 };
}

void Config::serialize(std::ostream& os) const
{
    os.write(g_configHeaderString, sizeof(g_configHeaderString));

    os.write(reinterpret_cast<char const*>(&m_version.major), sizeof(std::uint32_t));
    os.write(reinterpret_cast<char const*>(&m_version.minor), sizeof(std::uint32_t));
    os.write(reinterpret_cast<char const*>(&m_version.patch), sizeof(std::uint32_t));
}

void Config::setConfigFilePath(boost::filesystem::path const& cfg_path)
{
    m_configFilePath = cfg_path;
}

boost::filesystem::path Config::getConfigFilePath() const
{
    return m_configFilePath;
}
