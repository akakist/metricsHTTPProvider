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

namespace samples {

/// [Config service sample - component]
struct ConfigDataWithTimestamp {
  std::chrono::system_clock::time_point updated_at;
  std::unordered_map<std::string, formats::json::Value> key_values;
};

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
/// [Config service sample - component]

ConfigDistributor::ConfigDistributor(
    const components::ComponentConfig& config,
    const components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context) {
  constexpr std::string_view kDynamicConfig = R"~({
    "USERVER_TASK_PROCESSOR_PROFILER_DEBUG": {},
    "USERVER_LOG_REQUEST": true,
    "USERVER_LOG_REQUEST_HEADERS": false,
    "USERVER_CHECK_AUTH_IN_HANDLERS": false,
    "USERVER_CANCEL_HANDLE_REQUEST_BY_DEADLINE": false,
    "USERVER_RPS_CCONTROL_CUSTOM_STATUS": {},
    "USERVER_HTTP_PROXY": "",
    "USERVER_TASK_PROCESSOR_QOS": {
      "default-service": {
        "default-task-processor": {
          "wait_queue_overload": {
            "action": "ignore",
            "length_limit": 5000,
            "time_limit_us": 3000
          }
        }
      }
    },
    "USERVER_CACHES": {},
    "USERVER_LRU_CACHES": {},
    "USERVER_DUMPS": {}
  })~";

  auto json = formats::json::FromString(kDynamicConfig);

  KeyValues new_config;
  for (auto [key, value] : Items(json)) {
    new_config[std::move(key)] = value;
  }

  new_config["USERVER_LOG_REQUEST_HEADERS"] =
      formats::json::ValueBuilder(true).ExtractValue();

  SetNewValues(std::move(new_config));
}

/// [Config service sample - HandleRequestJsonThrow]
formats::json::ValueBuilder MakeConfigs(
    const rcu::ReadablePtr<ConfigDataWithTimestamp>& config_values_ptr,
    const formats::json::Value& request);

formats::json::Value ConfigDistributor::HandleRequestJsonThrow(
    const server::http::HttpRequest&, const formats::json::Value& json,
    server::request::RequestContext&) const {
  formats::json::ValueBuilder result;

  const auto config_values_ptr = config_values_.Read();
  result["configs"] = MakeConfigs(config_values_ptr, json);

  const auto updated_at = config_values_ptr->updated_at;
  result["updated_at"] = utils::datetime::Timestring(updated_at);

  return result.ExtractValue();
}
/// [Config service sample - HandleRequestJsonThrow]

/// [Config service sample - MakeConfigs]
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
/// [Config service sample - MakeConfigs]

}  // namespace samples

std::string config=R"(
components_manager:
    coro_pool:
        initial_size: 500             # Preallocate 500 coroutines at startup.
        max_size: 1000                # Do not keep more than 1000 preallocated coroutines.

    task_processors:                  # Task processor is an executor for coroutine tasks

        main-task-processor:          # Make a task processor for CPU-bound coroutine tasks.
            worker_threads: 4         # Process tasks in 4 threads.
            thread_name: main-worker  # OS will show the threads of this task processor with 'main-worker' prefix.

        fs-task-processor:            # Make a separate task processor for filesystem bound tasks.
            thread_name: fs-worker
            worker_threads: 4

    default_task_processor: main-task-processor
    components:                       # Configuring components that were registered via component_list

        server:
            listener:                 # configuring the main listening socket...
                port: ~port~            # ...to listen on this port and...
                task_processor: main-task-processor    # ...process incoming requests on this task processor.
        logging:
            fs-task-processor: fs-task-processor
            loggers:
                default:
                    file_path: '@stderr'
                    level: debug
                    overflow_behavior: discard  # Drop logs if the system is too busy to write them down.

        tracer:                           # Component that helps to trace execution times and requests in logs.
            service-name: config-service
        dynamic-config:                      # Dynamic config storage options, do nothing
            fs-cache-path: ''
        dynamic-config-fallbacks:            # Load options from file and push them into the dynamic config storage.
            fallback-path: dynamic_config_fallback.json
        auth-checker-settings:
        # /// [Config service sample - handler static config]
        # yaml
        handler-config:
            path: ~uri~
            method: POST              # Only for HTTP POST requests. Other handlers may reuse the same URL but use different method.
            task_processor: main-task-processor
        # /// [Config service sample - handler static config]

)";
//void DoRun1(const components::InMemoryConfig& config,
//           const components::ComponentList& component_list,
//           const std::string& init_log_path, logging::Format format
//           ) {
//}

/// [Config service sample - main]
/// [Config service sample - main]
