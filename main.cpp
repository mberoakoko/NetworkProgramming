#include "src/identity.hpp"
#include "src/socket_resource.hpp"

int main(){
    velocity_wave::usage::simple_resolve();
    std::cout << std::endl;
    // velocity_wave::usage::stress_test();
    velocity_wave::stress_test::stress_test();

    return 0;
}