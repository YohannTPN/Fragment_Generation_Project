# Archaeological Fragment Generator

A 3D procedural modeling tool that generates fragmented ceramic objects using parametric surfaces, Delaunay triangulation, and Iterated Function Systems (IFS) to simulate realistic fracture edges.

---

## Features

- **B-Spline revolution surface** as the base 3D object
- **Delaunay triangulation** in UV space to partition the surface into fragments
- **IFS fractal edges** — each fragment boundary is deformed using recursive barycentric transformations, producing irregular, self-similar fracture lines
- **Half-Edge mesh** with extrusion to give volume to each fragment
- **OBJ export** with fragments grouped by ID, ready for use in Blender or any 3D software

---

## Dependencies

- OpenGL / GLUT
- [Armadillo](https://arma.sourceforge.net/) (linear algebra)
- [CGAL](https://www.cgal.org/) (Delaunay triangulation)

---

## Building

```bash
mkdir build && cd build
cmake ..
make
```

---

## Controls

### 3D View

| Key | Action |
|-----|--------|
| `i` / `I` | Toggle IFS fractal mode on/off |
| `+` | Increase IFS recursion depth (regenerates mesh) |
| `-` | Decrease IFS recursion depth (regenerates mesh) |
| `e` / `E` | Toggle extrusion on/off |
| `w` | Toggle wireframe overlay |
| `x` / `X` | Toggle exploded view |
| `[` | Decrease explosion factor |
| `]` | Increase explosion factor |
| `r` / `R` | Regenerate Delaunay triangulation (new random seed) |
| `f` | Wireframe polygon mode |
| `p` | Fill polygon mode |
| `s` | Point polygon mode |
| `o` / `O` | Export current IFS mesh to `ifs_mesh.obj` (IFS must be active) |
| `q` | Quit |

### Mouse

| Action | Effect |
|--------|--------|
| Left click + drag | Rotate camera |
| Right click + drag | Zoom |

---

## Customization

### Changing the base generatrix curve

The 3D object is a B-Spline revolution surface. Its profile is defined by the control points matrix `Q` in `main.cpp`:

```cpp
mat Q = {
    {0.2, 0.0},   // {radius, height}
    {0.4, 0.5},
    {0.3, 1.0},
    {0.5, 1.5},
    {0.4, 2.0}
};
bspline = new BSplineCurve(Q, 3);  // degree 3
```

Each row is `{r, y}` — the radius and height of a control point. Add, remove, or modify rows to change the silhouette of the object. The second argument to `BSplineCurve` is the spline degree.

---

### Changing the number of fragments

The number of Delaunay points (and therefore approximate fragment count) is set here in `main.cpp`:

```cpp
triManager->generateDelaunayTriangulation(30, std::time(nullptr));
//                                         ^
//                                         number of seed points
```

Increase this value for more, smaller fragments.

---

### Changing IFS recursion depth

The default recursion depth is set at the top of `main.cpp`:

```cpp
int ifsDepth = 2;
```

It can also be changed at runtime with `+` and `-`. Higher values produce more detailed fractal edges but increase computation time significantly.

---

### Tuning the IFS transformation matrices

The fractal shape of each edge is controlled by two 5×5 transformation matrices `T0` (left sub-segment) and `T1` (right sub-segment), defined in `main.cpp`:

```cpp
// T0 parameters: controls the left half of each edge
float a  = -0.2f;  // curvature weight (negative = concave, positive = convex)
float a1 =  0.6f;  // complement weight (a + 2*a1 should equal 1.0)

// T1 parameters: controls the right half of each edge
float b  =  0.4f;
float b1 =  0.3f;

IFSTransform T0 = IFSTransform::createT0(a, a1);
IFSTransform T1 = IFSTransform::createT1(b, b1);
```

**Guidelines:**
- `a` and `b` control how much the mid-point deviates. Negative values pull inward, positive push outward.
- The constraint `a + 2 * a1 = 1.0` should be respected to keep the curve bounded.
- Symmetrical values (`a == b`, `a1 == b1`) produce more uniform edges; asymmetric values produce jagged, irregular fractures.

---

### Adding more control points to the IFS edge

Currently each edge uses 5 control points (`P0` to `P4`). To increase detail, you can modify `IFSEdge` to use 7 points by:

1. Changing the `ControlPoints` struct in `IFSEdge.h` to hold 7 points (`Vec2f p[7]`)
2. Updating `IFSTransform` to use 7×7 matrices
3. Adding a third transformation `T_mid` for the new middle sub-segment in `IFSEdge.cpp`
4. Updating `generateRecursive` to recurse into three sub-segments instead of two

---

### Adjusting edge curvature amplitude

The perpendicular displacement of IFS control points (which controls how "bumpy" the edges look) is set in `UVTriangleIFS.cpp` inside `getOrCreateEdgeIFS`:

```cpp
float amplitude = length * 0.15f;  // 15% of edge length
```

Increase this value for more dramatic fracture curves, decrease it for subtler deformation.

---

## OBJ Export and Blender workflow

Press `o` while IFS is active to export the mesh to `ifs_mesh.obj` in the working directory. Each fragment is written as a separate group with independent vertices.

In Blender:
1. **File → Import → Wavefront (.obj)**
2. Select the object, enter **Edit Mode** (`Tab`)
3. Press `A` to select all, then `P` → **By Loose Parts**
4. Return to **Object Mode** (`Tab`)
5. Each fragment is now a separate object and can be individually textured, animated, or physics-simulated
6. For smooth shading: right-click → **Shade Auto Smooth**

---

## Project Structure

```
├── main.cpp                  # Entry point, IFS parameters, keyboard controls
├── IFSEdge.cpp / .h          # Single fractal edge with 5 control points
├── IFSTransform.cpp / .h     # 5x5 barycentric transformation matrix
├── IFSManager.cpp / .h       # Manages and caches all IFS edges
├── UVTriangleIFS.cpp / .h    # Applies IFS to all Delaunay triangles, builds 3D mesh
├── UVTriangleManager.cpp / .h# Delaunay triangulation in UV space
├── HalfEdge.cpp / .h         # Half-Edge mesh, extrusion, normals, OBJ export
├── ParametricMapping.cpp / .h# UV → 3D mapping via revolution surface
├── BSplineCurve / Surface     # B-Spline generatrix and revolution surface
└── InputManager.cpp / .h     # Keyboard and mouse handling
```