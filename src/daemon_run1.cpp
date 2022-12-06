#include <userver/utils/daemon_run.hpp>

#include <iostream>

#include <boost/program_options.hpp>

#include <userver/components/run.hpp>
#include <userver/logging/log.hpp>

#include <boost/exception/diagnostic_information.hpp>
#include "MetricsHTTPProvider.hpp"

USERVER_NAMESPACE_BEGIN

namespace utils {

namespace {

std::optional<std::string> ToOptional(std::string&& s) {
  if (s.empty())
    return {};
  else
    return {std::move(s)};
}

}  // namespace

int DaemonMain1(const int argc, const char* const argv[],
               const components::ComponentList& components_list) 
{
  namespace po = boost::program_options;

  po::variables_map vm;
  po::options_description desc("Allowed options");
  std::string config_path = "config_dev.yaml";
  std::string config_vars_path;
  std::string config_vars_override_path;
  std::string init_log_path;
  std::string init_log_format = "tskv";

  // clang-format off
  desc.add_options()
    ("help,h", "produce this help message")
    ("config,c", po::value(&config_path)->default_value(config_path), "path to server config")
    ("config_vars", po::value(&config_vars_path), "path to config_vars.yaml; if set, config_vars in config.yaml are ignored")
    ("config_vars_override", po::value(&config_vars_override_path), "path to an additional config_vars.yaml, which overrides vars of config_vars.yaml")
    ("init-log,l", po::value(&init_log_path), "path to initialization log")
    ("init-log-format", po::value(&init_log_format), "format of the initialization log ('tskv', 'ltsv')")
  ;
  // clang-format on

  try {
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
  } catch (const std::exception& ex) {
    std::cerr << ex.what() << '\n';
    return 1;
  }

  if (vm.count("help")) {
    std::cerr << desc << '\n';
    return 0;
  }

  try {
    components::Run(config_path, ToOptional(std::move(config_vars_path)),
                    ToOptional(std::move(config_vars_override_path)),
                    components_list, init_log_path,
                    logging::FormatFromString(init_log_format));
    return 0;
  } catch (const std::exception& ex) {
//    auto msg =
//        fmt::format("Unhandled exception in components::Run: {}", ex.what());
//    LOG_ERROR() << msg;
//    std::cerr << msg << "\n";
//    return 1;
  } catch (...) {
//    auto msg = fmt::format("Non-standard exception in components::Run: {}",
//                           boost::current_exception_diagnostic_information());
//    LOG_ERROR() << msg;
//    std::cerr << msg << '\n';
//    return 1;
  }
  return 0;
}

}  // namespace utils



USERVER_NAMESPACE_END


int main(int argc, char* argv[])
{


    auto l=userver::logging::DefaultLogger();
    userver::logging::SetDefaultLoggerLevel(userver::logging::Level::kNone);

    MetricsHTTPProvider *m=new MetricsHTTPProvider(8081,"/sample/data");
    m->activate_object();
    for(int i=0;i<10;i++)
    {
        m->add_value("key-"+std::to_string(i),i);
        m->add_value("key2-"+std::to_string(i),i+100);
        sleep(1);
    }
    m->deactivate_object();
    m->wait_object();
    delete m;
    return 0;
}
