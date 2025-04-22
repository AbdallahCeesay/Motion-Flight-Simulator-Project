#include "mbed.h"
#include <iostream>
#include <Eigen/Dense>
using namespace std;
using namespace Eigen;

int main()
{
    Matrix<float, 2, 4> matrixA;

    std::cout << matrixA.Random() << std::endl;
}

