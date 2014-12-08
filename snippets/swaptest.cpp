#include <chrono>
#include <iostream>

using namespace std;

void intswap(int *a, int *b) {
    *a ^= *b;
    *b ^= *a;
    *a ^= *b;
}

int main(int argc, char **argv) {


    int one = 1, two = 2;
    chrono::time_point<chrono::system_clock> start, end;
    chrono::duration<long double> elapsed;

    // use our swap
    start = chrono::system_clock::now();
    for (int i = 0; i < 1000000; i++) {
        one ^= two;
        two ^= one;
        one ^= two;
    }
    end = chrono::system_clock::now();
    elapsed = end - start;

    cout << "XOR swap took " << elapsed.count() << endl;

    start = chrono::system_clock::now();
    for (int i = 0; i < 1000000; i++) {
        std::swap(one,two);
    }
    end = chrono::system_clock::now();
    elapsed = end - start;
    cout << "std::swap took " << elapsed.count() << endl;

    return 0;
}
