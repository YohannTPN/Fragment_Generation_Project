#pragma once
#include <array>

// Classe pour une transformation IFS en coordonnées barycentriques
// Une matrice 5x5 où chaque colonne représente les nouvelles coordonnées 
// barycentriques des points de contrôle P0, P1, P2, P3, P4
class IFSTransform {
public:
    IFSTransform();
    
    // Constructeur avec les coefficients explicites
    // Format: matrix[ligne][colonne]
    // Chaque colonne = coordonnées barycentriques du point transformé
    IFSTransform(
        float c0_0, float c1_0, float c2_0, float c3_0, float c4_0,  // Colonne 0 (P0')
        float c0_1, float c1_1, float c2_1, float c3_1, float c4_1,  // Colonne 1 (P1')
        float c0_2, float c1_2, float c2_2, float c3_2, float c4_2,  // Colonne 2 (P2')
        float c0_3, float c1_3, float c2_3, float c3_3, float c4_3,  // Colonne 3 (P3')
        float c0_4, float c1_4, float c2_4, float c3_4, float c4_4   // Colonne 4 (P4')
    );
    
    // Applique la transformation à un point en coordonnées barycentriques
    // Input:  bary[5] = {α0, α1, α2, α3, α4} avec Σαi = 1
    // Output: result[5] = nouvelles coordonnées barycentriques
    void apply(const float bary[5], float result[5]) const;
    
    void print() const;
    float get(int row, int col) const { return matrix[row][col]; }
    
    // Créer la transformation T0 (segment gauche [P0 → P2])
    // P0 reste fixe, P4' devient P2
    static IFSTransform createT0(float a, float a1);
    
    // Créer la transformation T1 (segment droit [P2 → P4])
    // P0' devient P2, P4 reste fixe
    static IFSTransform createT1(float b, float b1);
    
private:
    float matrix[5][5];  
};