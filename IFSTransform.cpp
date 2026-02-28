#include "IFSTransform.h"
#include <iostream>
#include <iomanip>

IFSTransform::IFSTransform() {
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            matrix[i][j] = (i == j) ? 1.0f : 0.0f;
        }
    }
}

IFSTransform::IFSTransform(
    float c0_0, float c1_0, float c2_0, float c3_0,
    float c0_1, float c1_1, float c2_1, float c3_1,
    float c0_2, float c1_2, float c2_2, float c3_2,
    float c0_3, float c1_3, float c2_3, float c3_3
) {
    // Remplir par colonne
    matrix[0][0] = c0_0; matrix[1][0] = c1_0; matrix[2][0] = c2_0; matrix[3][0] = c3_0;
    matrix[0][1] = c0_1; matrix[1][1] = c1_1; matrix[2][1] = c2_1; matrix[3][1] = c3_1;
    matrix[0][2] = c0_2; matrix[1][2] = c1_2; matrix[2][2] = c2_2; matrix[3][2] = c3_2;
    matrix[0][3] = c0_3; matrix[1][3] = c1_3; matrix[2][3] = c2_3; matrix[3][3] = c3_3;
}

void IFSTransform::apply(const float bary[4], float result[4]) const {
    // Multiplication matrice-vecteur: result = matrix * bary
    // result[i] = Σ(matrix[i][j] * bary[j])
    
    for (int i = 0; i < 4; ++i) {
        result[i] = 0.0f;
        for (int j = 0; j < 4; ++j) {
            result[i] += matrix[i][j] * bary[j];
        }
    }
}

void IFSTransform::print() const {
    std::cout << "IFSTransform matrix:" << std::endl;
    for (int i = 0; i < 4; ++i) {
        std::cout << "  [";
        for (int j = 0; j < 4; ++j) {
            std::cout << std::setw(8) << std::fixed << std::setprecision(3) 
                      << matrix[i][j];
            if (j < 3) std::cout << ", ";
        }
        std::cout << "]" << std::endl;
    }
}

IFSTransform IFSTransform::createTi(
    float c2_0, float c2_1, float c2_2, float c2_3,
    float c3_0, float c3_1, float c3_2, float c3_3
) {
    // Ti matrice:
    // [1    c2_0  c3_0  0]
    // [0    c2_1  c3_1  1]
    // [0    c2_2  c3_2  0]
    // [0    c2_3  c3_3  0]
    
    return IFSTransform(
        1.0f, 0.0f, 0.0f, 0.0f,      // P0' = P0
        c2_0, c2_1, c2_2, c2_3,      // P1' = combinaison
        c3_0, c3_1, c3_2, c3_3,      // P2' = combinaison
        0.0f, 1.0f, 0.0f, 0.0f       // P3' = P1
    );
}

IFSTransform IFSTransform::createTij(
    float c4_0, float c4_1, float c4_2, float c4_3,
    float c5_0, float c5_1, float c5_2, float c5_3
) {
    // Tij matrice:
    // [0    c4_0  c5_0  0]
    // [1    c4_1  c5_1  0]
    // [0    c4_2  c5_2  1]
    // [0    c4_3  c5_3  0]
    
    return IFSTransform(
        0.0f, 1.0f, 0.0f, 0.0f,      // P0' = P1
        c4_0, c4_1, c4_2, c4_3,      // P1' = combinaison
        c5_0, c5_1, c5_2, c5_3,      // P2' = combinaison
        0.0f, 0.0f, 1.0f, 0.0f       // P3' = P2
    );
}

IFSTransform IFSTransform::createTj(
    float c6_0, float c6_1, float c6_2, float c6_3,
    float c7_0, float c7_1, float c7_2, float c7_3
) {
    // Tj matrice:
    // [0    c6_0  c7_0  0]
    // [0    c6_1  c7_1  0]
    // [1    c6_2  c7_2  0]
    // [0    c6_3  c7_3  1]
    
    return IFSTransform(
        0.0f, 0.0f, 1.0f, 0.0f,      // P0' = P2
        c6_0, c6_1, c6_2, c6_3,      // P1' = combinaison
        c7_0, c7_1, c7_2, c7_3,      // P2' = combinaison
        0.0f, 0.0f, 0.0f, 1.0f       // P3' = P3
    );
}
