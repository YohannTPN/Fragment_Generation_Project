#pragma once
#include <array>

// Classe pour une transformation IFS en coordonnées barycentriques
// Une matrice 4x4 où chaque colonne représente les nouvelles coordonnées 
// barycentriques des points de contrôle P0, P1, P2, P3
class IFSTransform {
public:
    IFSTransform();
    
    // Constructeur avec les coefficients explicites
    // Format: matrix[ligne][colonne]
    // Chaque colonne = coordonnées barycentriques du point transformé
    IFSTransform(
        float c0_0, float c1_0, float c2_0, float c3_0,  // Colonne 0 (P0')
        float c0_1, float c1_1, float c2_1, float c3_1,  // Colonne 1 (P1')
        float c0_2, float c1_2, float c2_2, float c3_2,  // Colonne 2 (P2')
        float c0_3, float c1_3, float c2_3, float c3_3   // Colonne 3 (P3')
    );
    
    // Applique la transformation à un point en coordonnées barycentriques
    // Input:  bary[4] = {α0, α1, α2, α3} avec Σαi = 1
    // Output: result[4] = nouvelles coordonnées barycentriques
    void apply(const float bary[4], float result[4]) const;
    
    void print() const;
    float get(int row, int col) const { return matrix[row][col]; }
    static IFSTransform createTi(
        float c2_0, float c2_1, float c2_2, float c2_3,
        float c3_0, float c3_1, float c3_2, float c3_3
    );
    
    static IFSTransform createTij(
        float c4_0, float c4_1, float c4_2, float c4_3,
        float c5_0, float c5_1, float c5_2, float c5_3
    );
    
    static IFSTransform createTj(
        float c6_0, float c6_1, float c6_2, float c6_3,
        float c7_0, float c7_1, float c7_2, float c7_3
    );
    
private:
    float matrix[4][4];  
};
