#include <iostream>
#include <Eigen/Dense>

using namespace Eigen;
int main() {

    std::cout << "Hello World!" << std::endl;
    Matrix <float, 3, 3 > matrixA;

    matrixA.setZero();

    std::cout << matrixA << std::endl;


    return 0;
}