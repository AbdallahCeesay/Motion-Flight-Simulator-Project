#include "mbed.h"
#include <iostream>
#include <Eigen/Dense>

using namespace Eigen;
using namespace std;

int main()
{  
    Matrix <float, 3, 3> matrixA;
	matrixA.setRandom();

	cout << matrixA << endl << endl;

    // this is a dynamic matrix (resizeable). Set during run time and can be reset anytime during runtime 
    // dynamic matrix is stored on the heap
    MatrixXd d; 


    // this is a fixed size matrix. Set during compile time.
    // A fixed size matrix is stored on the stack
    Matrix4d matA;
    matA.setIdentity();

    cout << matA << endl << endl;

}

