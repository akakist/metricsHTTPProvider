#include <iostream>
#include <fstream>

std::string config=R"x(
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
            fallback-path: /tmp/dynamic_config_fallback.json
        auth-checker-settings:
        # /// [Config service sample - handler static config]
        # yaml
        handler-config:
            path: ~uri~
            method: GET              # Only for HTTP POST requests. Other handlers may reuse the same URL but use different method.
            task_processor: main-task-processor
        # /// [Config service sample - handler static config]

)x";

std::string dynamic_config_fallback_json=
R"x(
{
  "HTTP_CLIENT_CONNECTION_POOL_SIZE": 1000,
  "HTTP_CLIENT_ENFORCE_TASK_DEADLINE": {
    "cancel-request": false,
    "update-timeout": false
  },
  "HTTP_CLIENT_CONNECT_THROTTLE": {
    "max-size": 100,
    "token-update-interval-ms": 0
  },
  "USERVER_CACHES": {},
  "USERVER_CANCEL_HANDLE_REQUEST_BY_DEADLINE": false,
  "USERVER_CHECK_AUTH_IN_HANDLERS": false,
  "USERVER_DUMPS": {},
  "USERVER_FILES_CONTENT_TYPE_MAP": {
    ".css": "text/css",
    ".gif": "image/gif",
    ".htm": "text/html",
    ".html": "text/html",
    ".jpeg": "image/jpeg",
    ".js": "application/javascript",
    ".json": "application/json",
    ".md": "text/markdown",
    ".png": "image/png",
    ".svg": "image/svg+xml",
    "__default__": "text/plain"
  },
  "USERVER_NO_LOG_SPANS": {
    "names": [],
    "prefixes": []
  },
  "USERVER_RPS_CCONTROL_ENABLED": true,
  "USERVER_RPS_CCONTROL": {
    "down-level": 1,
    "down-rate-percent": 2,
    "min-limit": 10,
    "no-limit-seconds": 1000,
    "overload-off-seconds": 3,
    "overload-on-seconds": 3,
    "up-level": 2,
    "up-rate-percent": 2
  },

  "USERVER_HTTP_PROXY": "",
  "USERVER_LOG_REQUEST": true,
  "USERVER_LOG_REQUEST_HEADERS": false,
  "USERVER_LRU_CACHES": {},
  "USERVER_RPS_CCONTROL_CUSTOM_STATUS": {},
  "USERVER_TASK_PROCESSOR_PROFILER_DEBUG": {},
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
  }
}
)x";

void copy_json_to_tmp()
{
    std::ofstream myfile;
    myfile.open ("/tmp/dynamic_config_fallback.json");
    myfile << dynamic_config_fallback_json;
    myfile.close();
}
