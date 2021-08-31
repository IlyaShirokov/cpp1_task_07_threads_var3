#include <iostream>
#include <atomic>
#include <thread>
#include <chrono>

using namespace std;

int main()
{
    long long i = 1;
    long long ii = 2;

    std::thread t1([&](){ for(int q = 0; q < 1000000; ++q){ i++; i--; ii++;}});
    std::thread t2([&](){ for(int q = 0; q < 1000000; ++q){ i++; i--; ii++;}});
    std::thread t3([&](){ for(int q = 0; q < 1000000; ++q){ i++; i--; ii++;}});
    std::thread t4([&](){ for(int q = 0; q < 1000000; ++q){ i++; i--; ii++;}});

    t1.join();
    t2.join();
    t3.join();
    t4.join();

    std::cout<<i<<" "<<ii << std::endl;
}
