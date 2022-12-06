#include "MetricsHTTPProvider.hpp"
#include <string>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/rcu/rcu.hpp>
#include <userver/server/handlers/http_handler_json_base.hpp>
#include <userver/utils/daemon_run.hpp>
#include <userver/utils/datetime.hpp>
#include <crypto/openssl.hpp>
#include <userver/components/run.hpp>
#include <utils/jemalloc.hpp>
#include <userver/formats/json.hpp>
#include <userver/components/manager.hpp>
#include <core/src/components/manager_config.hpp>
#include <userver/utest/using_namespace_userver.hpp>
#include <userver/logging/log.hpp>

struct ConfigDataWithTimestamp {
  std::chrono::system_clock::time_point updated_at;
  std::unordered_map<std::string, formats::json::Value> key_values;
};
struct Container
{
    std::mutex mutex;
    std::map<std::string_view, unsigned long> vals_ul;
    std::map<std::string_view,std::string_view> vals_string;

};
static Container container;
class ConfigDistributor final : public server::handlers::HttpHandlerJsonBase {
 public:
  static constexpr std::string_view kName = "handler-config";

  using KeyValues = std::unordered_map<std::string, formats::json::Value>;

  // Component is valid after construction and is able to accept requests
  ConfigDistributor(const components::ComponentConfig& config,
                    const components::ComponentContext& context);

  formats::json::Value HandleRequestJsonThrow(
      const server::http::HttpRequest&, const formats::json::Value& json,
      server::request::RequestContext&) const override;

  void SetNewValues(KeyValues&& key_values) {
    config_values_.Assign(ConfigDataWithTimestamp{
        /*.updated_at=*/utils::datetime::Now(),
        /*.key_values=*/std::move(key_values),
    });
  }

 private:
  rcu::Variable<ConfigDataWithTimestamp> config_values_;
};
extern std::string config;

struct st_active
{
    bool *_b;
    st_active(bool*b):_b(b){*_b=true;}
    ~st_active(){*_b=false;}
};
class MetricsHTTPProviderImpl
{
public:
    unsigned int listen_port;
    std::string uri;
    std::thread thread_;
    bool stopped_=false;
    bool active_;
    MetricsHTTPProviderImpl(unsigned int _listen_port, std::string _uri): listen_port(_listen_port),uri(_uri)
    {
    }

    void
    set_value(std::string_view key, std::string_view value)
    {
        std::lock_guard<std::mutex> g(container.mutex);
        container.vals_string[key]=value;
    }
    void
    add_value(std::string_view key, unsigned long value)
    {
        std::lock_guard<std::mutex> g(container.mutex);
        container.vals_ul[key]=value;
    }
    static void* worker(MetricsHTTPProviderImpl* _this)
    {

        const components::ComponentList component_list = components::MinimalServerComponentList()
                                        .Append<ConfigDistributor>();
      //  std::string init_log_path;
      //  std::string init_log_format = "tskv";

      //  DoRun1(components::InMemoryConfig(config),component_list,init_log_path,
      //         logging::FormatFromString(init_log_format));

        crypto::impl::Openssl::Init();

        auto conf=std::make_unique<components::ManagerConfig>(components::ManagerConfig::FromString(config,{},{}));
        std::optional<components::Manager> manager;
        try {
          manager.emplace(std::move(conf), component_list);
        } catch (const std::exception& ex) {
          LOG_ERROR() << "Loading failed: " << ex;
          throw;
        }
        st_active(&_this->active_);


        for (;;) {
            if(_this->stopped_)
                return NULL;
            sleep(1);
        }

        return NULL;

    }
    void
    activate_object()
    {
        thread_=std::thread(worker,this);
    }
    void
    deactivate_object()
    {
        stopped_=true;
        thread_.join();
    }
    void
    wait_object()
    {

    }
    bool
    active()
    {
        return active_;
    }


};

MetricsHTTPProvider::MetricsHTTPProvider(unsigned int _listen_port, std::string _uri): listen_port(_listen_port),uri(_uri)
{
    impl=new MetricsHTTPProviderImpl(_listen_port,_uri);
}
MetricsHTTPProvider::~MetricsHTTPProvider()
{
    delete impl;
}

void
MetricsHTTPProvider::set_value(std::string_view key, std::string_view value)
{
    impl->set_value(key,value);
}

void
MetricsHTTPProvider::add_value(std::string_view key, unsigned long value)
{
    impl->add_value(key,value);
}
void
MetricsHTTPProvider::activate_object() {
    impl->activate_object();
}
void
MetricsHTTPProvider::deactivate_object() {
    impl->deactivate_object();
}

void
MetricsHTTPProvider::wait_object()
{
    impl->wait_object();
}
bool
MetricsHTTPProvider::active()
{
    return impl->active_;
}

ConfigDistributor::ConfigDistributor(const components::ComponentConfig& config,
    const components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context) {

//  auto json = formats::json::FromString(kDynamicConfig);

//  KeyValues new_config;
//  for (auto [key, value] : Items(json)) {
//    new_config[std::move(key)] = value;
//  }

//  new_config["USERVER_LOG_REQUEST_HEADERS"] =
//      formats::json::ValueBuilder(true).ExtractValue();

//  SetNewValues(std::move(new_config));
}
formats::json::ValueBuilder MakeConfigs(
    const rcu::ReadablePtr<ConfigDataWithTimestamp>& config_values_ptr,
    const formats::json::Value& request) {
  formats::json::ValueBuilder configs(formats::common::Type::kObject);

  const auto updated_since = request["updated_since"].As<std::string>({});
  if (!updated_since.empty() && utils::datetime::Stringtime(updated_since) >=
                                    config_values_ptr->updated_at) {
    // Return empty JSON if "updated_since" is sent and no changes since then.
    return configs;
  }

  LOG_DEBUG() << "Sending dynamic config for service "
              << request["service"].As<std::string>("<unknown>");

  const auto& values = config_values_ptr->key_values;
  if (request["ids"].IsMissing()) {
    // Sending all the configs.
    for (const auto& [key, value] : values) {
      configs[key] = value;
    }

    return configs;
  }

  // Sending only the requested configs.
  for (const auto& id : request["ids"]) {
    const auto key = id.As<std::string>();

    const auto it = values.find(key);
    if (it != values.end()) {
      configs[key] = it->second;
    } else {
      LOG_ERROR() << "Failed to find config with name '" << key << "'";
    }
  }

  return configs;
}

formats::json::Value ConfigDistributor::HandleRequestJsonThrow(
    const server::http::HttpRequest&, const formats::json::Value& json,
    server::request::RequestContext&) const {
//  formats::json::ValueBuilder result;

  formats::json::ValueBuilder j;
  {
      std::lock_guard<std::mutex> lock(container.mutex);
      for(auto&[k,v]: container.vals_string)
      {
          j[std::string(k)]=v;
      }
      for(auto&[k,v]: container.vals_ul)
      {
          j[std::string(k)]=v;
      }

  }
//  j["11"]=11;
//  const auto config_values_ptr = config_values_.Read();
//  result["configs"] = MakeConfigs(config_values_ptr, json);

//  const auto updated_at = config_values_ptr->updated_at;
//  result["updated_at"] = utils::datetime::Timestring(updated_at);

  return j.ExtractValue();
}
