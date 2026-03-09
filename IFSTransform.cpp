#include "IFSTransform.h"
#include <iostream>
#include <iomanip>

IFSTransform::IFSTransform() {
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            matrix[i][j] = (i == j) ? 1.0f : 0.0f;
        }
    }
}

IFSTransform::IFSTransform(
    float c0_0, float c1_0, float c2_0, float c3_0, float c4_0,
    float c0_1, float c1_1, float c2_1, float c3_1, float c4_1,
    float c0_2, float c1_2, float c2_2, float c3_2, float c4_2,
    float c0_3, float c1_3, float c2_3, float c3_3, float c4_3,
    float c0_4, float c1_4, float c2_4, float c3_4, float c4_4
) {
    // Remplir par colonne
    matrix[0][0] = c0_0; matrix[1][0] = c1_0; matrix[2][0] = c2_0; matrix[3][0] = c3_0; matrix[4][0] = c4_0;
    matrix[0][1] = c0_1; matrix[1][1] = c1_1; matrix[2][1] = c2_1; matrix[3][1] = c3_1; matrix[4][1] = c4_1;
    matrix[0][2] = c0_2; matrix[1][2] = c1_2; matrix[2][2] = c2_2; matrix[3][2] = c3_2; matrix[4][2] = c4_2;
    matrix[0][3] = c0_3; matrix[1][3] = c1_3; matrix[2][3] = c2_3; matrix[3][3] = c3_3; matrix[4][3] = c4_3;
    matrix[0][4] = c0_4; matrix[1][4] = c1_4; matrix[2][4] = c2_4; matrix[3][4] = c3_4; matrix[4][4] = c4_4;
}

void IFSTransform::apply(const float bary[5], float result[5]) const {
    // Multiplication matrice-vecteur: result = matrix * bary
    // result[i] = Σ(matrix[i][j] * bary[j])
    
    for (int i = 0; i < 5; ++i) {
        result[i] = 0.0f;
        for (int j = 0; j < 5; ++j) {
            result[i] += matrix[i][j] * bary[j];
        }
    }
}

void IFSTransform::print() const {
    std::cout << "IFSTransform matrix (5x5):" << std::endl;
    for (int i = 0; i < 5; ++i) {
        std::cout << "  [";
        for (int j = 0; j < 5; ++j) {
            std::cout << std::setw(8) << std::fixed << std::setprecision(3) 
                      << matrix[i][j];
            if (j < 4) std::cout << ", ";
        }
        std::cout << "]" << std::endl;
    }
}

IFSTransform IFSTransform::createT0(float a, float a1) {
    // T0 matrice (segment gauche [P0 → P2]):
    //      P0'  P1'  P2'  P3'  P4'
    // P0 [  1   0.5   a1   0    0  ]
    // P1 [  0   0.5   a   0.5   0  ]
    // P2 [  0    0    a1  0.5   1  ]
    // P3 [  0    0     0    0    0  ]
    // P4 [  0    0     0    0    0  ]
    
    return IFSTransform(
        1.0f, 0.0f, 0.0f, 0.0f, 0.0f,      // P0' = P0 (fixe)
        0.5f, 0.5f, 0.0f, 0.0f, 0.0f,      // P1' = 0.5·P0 + 0.5·P1
        a1,   a,    a1,   0.0f, 0.0f,      // P2' = a1·P0 + a·P1 + a1·P2
        0.0f, 0.5f, 0.5f, 0.0f, 0.0f,      // P3' = 0.5·P1 + 0.5·P2
        0.0f, 0.0f, 1.0f, 0.0f, 0.0f       // P4' = P2 (devient milieu)
    );
}

IFSTransform IFSTransform::createT1(float b, float b1) {
    // T1 matrice (segment droit [P2 → P4]):
    //      P0'  P1'  P2'  P3'  P4'
    // P0 [  0    0    0    0    0  ]
    // P1 [  0    0    0    0    0  ]
    // P2 [  1   0.5   b1   0    0  ]
    // P3 [  0   0.5   b   0.5   0  ]
    // P4 [  0    0    b1  0.5   1  ]
    
    return IFSTransform(
        0.0f, 0.0f, 1.0f, 0.0f, 0.0f,      // P0' = P2
        0.0f, 0.0f, 0.5f, 0.5f, 0.0f,      // P1' = 0.5·P2 + 0.5·P3
        0.0f, 0.0f, b1,   b,    b1,        // P2' = b1·P2 + b·P3 + b1·P4
        0.0f, 0.0f, 0.0f, 0.5f, 0.5f,      // P3' = 0.5·P3 + 0.5·P4
        0.0f, 0.0f, 0.0f, 0.0f, 1.0f       // P4' = P4
    );
}