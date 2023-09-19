#include "config.h"
#include <tinyxml/tinyxml.h>
#include <stdexcept>
#include <memory>
#include <iostream>

// 未命名的空间，该空间中的函数和变量在文件外不可见
namespace
{

// 读取XML的一个节点
inline TiXmlElement *Read_XML_Node(TiXmlElement *node, const char *value)
{
    TiXmlElement *p_ele = node->FirstChildElement(value);
    if (!p_ele) {
        std::cerr << "Read XML node <" << value << "> failed" << std::endl;
        exit(-1);
    }
    return p_ele;
}

// 读取XML的一个节点（重载）
inline TiXmlElement *Read_XML_Node(TiXmlDocument *doc, const char *value)
{
    TiXmlElement *p_ele = doc->FirstChildElement(value);
    if (!p_ele) {
        std::cerr << "Read XML node <" << value << "> failed" << std::endl;
        exit(-1);
    }
    return p_ele;
}

// 读取XML节点中的文本
inline std::string Read_Str_From_XML_Node(TiXmlElement *node, const char *value)
{
    auto p_ele = Read_XML_Node(node, value);
    if (!p_ele->GetText()) {
        std::cerr << "Read XML node <" << value << "> text failed" << std::endl;
        exit(-1);
    }

    return std::string(p_ele->GetText());
}

} // namespace


namespace zest
{

Config *Config::g_config = nullptr;  // 类静态变量初始化

Config *Config::GetGlobalConfig()
{
    return g_config;
}

// 工厂函数
void Config::SetGlobalConfig(const char *xmlfile)
{
    if (g_config == nullptr) {
        if (xmlfile != nullptr)
            g_config = new Config(xmlfile);
        else
            g_config = new Config();
    }
    else
        throw std::runtime_error("Set global config more than once");

    ShowGlobalConfig();
}

// 默认构造函数，使用默认值
Config::Config():
    m_log_level("DEBUG"), m_log_file_name("test_rpc_server"),
    m_log_file_path("../log/"), m_log_max_file_size(10000),
    m_log_sync_interval(1000), m_server_port(12345),
    m_server_io_threads(4)
{
    /* do nothing */
}

// 构造函数，解析XML文件，获得配置参数
Config::Config(const char *xmlfile)
{
    std::shared_ptr<TiXmlDocument> xml_doc(new TiXmlDocument());
    if (xml_doc->LoadFile(xmlfile) == false) {
        std::cerr << "Read config file failed, no such XML file" << std::endl;
        exit(-1);
    }

    auto root = Read_XML_Node(xml_doc.get(), "root");
    auto log = Read_XML_Node(root, "log");
    auto server = Read_XML_Node(root, "server");

    m_log_level = Read_Str_From_XML_Node(log, "log_level");
    m_log_file_name = Read_Str_From_XML_Node(log, "log_file_name");
    m_log_file_path = Read_Str_From_XML_Node(log, "log_file_path");
    m_log_max_file_size = std::stoi(Read_Str_From_XML_Node(log, "log_max_file_size"));
    m_log_sync_interval = std::stoi(Read_Str_From_XML_Node(log, "log_sync_interval"));

    m_server_port = std::stoi(Read_Str_From_XML_Node(server, "port"));
    m_server_io_threads = std::stoi(Read_Str_From_XML_Node(server, "io_threads"));
}

Config::~Config()
{
    /* do nothing */
}

void Config::ShowGlobalConfig()
{
    if (!g_config) {
        std::cout << "Config has not been initialized!" << std::endl;
        return;
    }

    std::cout << "-------- ZEST RPC CONFIG ---------" << std::endl
              << "LOG_LEVEL: " << g_config->m_log_level << std::endl
              << "LOG_FILE_PATH: " << g_config->m_log_file_path << std::endl
              << "LOG_FILE_NAME: " << g_config->m_log_file_name << std::endl
              << "LOG_MAX_FILE_SIZE: " << g_config->m_log_max_file_size << std::endl
              << "LOG_SYNC_INTERVAL: " << g_config->m_log_sync_interval << std::endl 
              << std::endl
              << "PORT: " << g_config->m_server_port << std::endl
              << "IO_THREADS: " << g_config->m_server_io_threads << std::endl
              << "----------------------------------" << std::endl;
}

} // namespace zest
