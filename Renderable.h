#pragma once

class Renderable {
public:
    virtual ~Renderable() = default;

    // Dessiner l'objet
    virtual void draw(float t = 0.0f) const = 0;

    // Pour les objets qui utilisent un paramètre t (ex : Bézier)
    virtual void setT(float value) { }
    virtual float getT() const { return 0.0f; }
};
