#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>
#include <cassert>

template <class T>
void print(const std::vector<T>& numbers);

template <class T>
void stlSort(std::vector<T>& numbers);

template <class T>
void singleThreadSort(std::vector<T>& numbers, int l, int r);

template <class T>
void multiThreadSort(std::vector<T>& numbers);

struct VectorIndex
{
        int left;
        int right;

        VectorIndex(int argLeft, int argRight) : left(argLeft), right(argRight) {}
};

std::queue<VectorIndex> g_portfolioTasks;
std::mutex g_mutex;
std::atomic<bool> g_bRun(false);
std::condition_variable g_condNotEmpty;

int main()
{
    srand(time(NULL));
    size_t vectorSize = 10'000'000;
    const unsigned int nThread = std::thread::hardware_concurrency();

    //STL SOTRING
    std::vector<int> vectorSTL;
    for (size_t i = 0; i < vectorSize; ++i)
        vectorSTL.push_back(rand() % 100);
    stlSort(vectorSTL);

    //SINGLE THREAD SORTING
    std::vector<int> vecSingleThreadSort;
    for (size_t i = 0; i < vectorSize; i++)
        vecSingleThreadSort.push_back(rand() % 100);
    auto start = std::chrono::high_resolution_clock::now();
    singleThreadSort(vecSingleThreadSort, 0, vectorSize - 1);
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Single thread algorithm time: " << std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>(end - start).count() << std::endl;

    //MULTI THREAD SORTING
    std::vector<int> vecMultiThreadSort;
    for (size_t i = 0; i < vectorSize; i++)
        vecMultiThreadSort.push_back(rand() % 100);
    start = std::chrono::high_resolution_clock::now();
    g_bRun = true;
    g_portfolioTasks.push(VectorIndex(0, vectorSize - 1));
    std::vector<std::thread> threads;
    for (size_t i = 0; i < nThread - 2; ++i)
    {
        threads.push_back(std::thread(multiThreadSort<int>, std::ref(vecMultiThreadSort)));
        threads.back().join();
    }
    end = std::chrono::high_resolution_clock::now();
    std::cout << "Multi thread algorithm time: " << std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>(end - start).count() << std::endl;

    return 0;
}

template <class T>
void print(const std::vector<T>& numbers)
{
    for (size_t i = 0; i < numbers.size(); i++)
        std::cout << numbers[i] << " ";
    std::cout << std::endl << std::endl;
}

template <class T>
void stlSort(std::vector<T>& numbers)
{
    auto start = std::chrono::high_resolution_clock::now();
    std::sort(numbers.begin(), numbers.end());
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "STL Time: " << std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>(end - start).count() << std::endl;
}

template<class T>
void singleThreadSort(std::vector<T>& numbers, int l, int r)
{
    if (l < r)
    {
        auto midElem = numbers[(l + r) / 2];

        int i = l;
        int j = r;
        while (i <= j)
        {
            while (numbers[i] < midElem)
                i++;
            while (numbers[j] > midElem)
                j--;
            if (i >= j)
                break;
            std::swap(numbers[i++], numbers[j--]);
        }
        singleThreadSort(numbers, l, j);
        singleThreadSort(numbers, j + 1, r);
    }
}

template <class T>
void multiThreadSort(std::vector<T>& numbers)
{
    while (true)
    {
        std::unique_lock<std::mutex> lock(g_mutex);
        g_condNotEmpty.wait(lock, [] { return !g_portfolioTasks.empty() || !g_bRun; });
        if (g_portfolioTasks.empty() && !g_bRun)
            break;

        VectorIndex elem = g_portfolioTasks.front();
        g_portfolioTasks.pop();

        lock.unlock();

        if (elem.left < elem.right)
        {
            auto midElem = numbers[(elem.left + elem.right) / 2];

            int i = elem.left;
            int j = elem.right;
            while (i <= j)
            {
                while (numbers[i] < midElem)
                    i++;
                while (numbers[j] > midElem)
                    j--;
                if (i >= j)
                    break;
                std::swap(numbers[i++], numbers[j--]);
            }

            g_portfolioTasks.push(VectorIndex(elem.left, j));
            g_condNotEmpty.notify_one();
            g_portfolioTasks.push(VectorIndex(j + 1, elem.right));
            g_condNotEmpty.notify_one();

        }

        if (g_portfolioTasks.empty())
            g_bRun = false;
    }
}
