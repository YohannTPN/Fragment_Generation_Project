#pragma once

class Renderable {
public:
    virtual ~Renderable() = default;

    virtual void draw(float t = 0.0f) const = 0;


    virtual void setT(float value) { }
    virtual float getT() const { return 0.0f; }
};
