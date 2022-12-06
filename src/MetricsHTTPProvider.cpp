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
#include <regex>

struct Container
{
    std::mutex mutex;
    std::map<std::string, unsigned long> vals_ul;
    std::map<std::string,std::string> vals_string;

};
static Container container;
class ConfigDistributor final : public server::handlers::HttpHandlerJsonBase {
 public:
  static constexpr std::string_view kName = "handler-config";

  // Component is valid after construction and is able to accept requests
  ConfigDistributor(const components::ComponentConfig& config,
                    const components::ComponentContext& context);

  formats::json::Value HandleRequestJsonThrow(
      const server::http::HttpRequest&, const formats::json::Value& json,
      server::request::RequestContext&) const override;
};
extern std::string config;

struct st_active
{
    bool *_b;
    st_active(bool*b):_b(b){*_b=true;}
    ~st_active(){*_b=false;}
};
void copy_json_to_tmp();
class MetricsHTTPProviderImpl
{
public:
    unsigned int listen_port;
    std::string_view uri;
    std::thread thread_;
    bool stopped_=false;
    bool active_;
    MetricsHTTPProviderImpl(unsigned int _listen_port, std::string_view _uri): listen_port(_listen_port),uri(_uri)
    {
    }
    ~MetricsHTTPProviderImpl(){
        if(active_)
            LOG_ERROR() << "Try to destruct active object MetricsHTTPProviderImpl";

    }

    void
    set_value(std::string_view key, std::string_view value)
    {
        std::lock_guard<std::mutex> g(container.mutex);
        container.vals_string[std::string(key)]=value;
    }
    void
    add_value(std::string_view key, unsigned long value)
    {
        std::lock_guard<std::mutex> g(container.mutex);
        container.vals_ul[std::string(key)]=value;
    }
    static void* worker(MetricsHTTPProviderImpl* _this)
    {

        const components::ComponentList component_list = components::MinimalServerComponentList()
                                        .Append<ConfigDistributor>();

        crypto::impl::Openssl::Init();

        auto conf_replaced=std::regex_replace(config,std::regex("~port~"),std::to_string(_this->listen_port));
        conf_replaced=std::regex_replace(conf_replaced,std::regex("~uri~"),std::string(_this->uri));
        auto conf_prepared=std::make_unique<components::ManagerConfig>(components::ManagerConfig::FromString(conf_replaced,{},{}));
        std::optional<components::Manager> manager;
        try {
          manager.emplace(std::move(conf_prepared), component_list);
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
        copy_json_to_tmp();
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

void MetricsHTTPProvider::set_value(std::string_view key, std::string_view value)
{
    impl->set_value(key,value);
}

void MetricsHTTPProvider::add_value(std::string_view key, unsigned long value)
{
    impl->add_value(key,value);
}
void MetricsHTTPProvider::activate_object() {
    impl->activate_object();
}
void MetricsHTTPProvider::deactivate_object() {
    impl->deactivate_object();
}

void MetricsHTTPProvider::wait_object()
{
    impl->wait_object();
}
bool MetricsHTTPProvider::active()
{
    return impl->active_;
}

ConfigDistributor::ConfigDistributor(const components::ComponentConfig& config, const components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context)
{

}
formats::json::Value ConfigDistributor::HandleRequestJsonThrow( const server::http::HttpRequest&, const formats::json::Value& json,
    server::request::RequestContext&) const {

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
  return j.ExtractValue();
}
