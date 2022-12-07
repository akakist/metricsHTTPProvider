#include "MetricsHTTPProvider.hpp"
#include <unistd.h>

int main(int argc, char* argv[])
{


//    auto l=userver::logging::DefaultLogger();
//    userver::logging::SetDefaultLoggerLevel(userver::logging::Level::kNone);

//    MetricsHTTPProvider *m=new
    MetricsHTTPProvider *m=new MetricsHTTPProvider(8081,"/sample/data");
    m->activate_object();
    for(int i=0;i<100;i++)
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
