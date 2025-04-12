#include "mbed.h"
#include <iostream>
#include <Eigen/Dense>


using namespace Eigen;
using namespace std;

int main()
{
    Matrix <float, 3, 3> matrixA;
	matrixA.setZero();

	cout << matrixA << endl;
}

