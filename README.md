<p align="center">
  <img src="logo.png" />
</p>

Godot Box2D v3 is a high-performance physics extension for Godot 4.3+ that implements Box2D v3. It can be considered a successor to the [Godot Box2D](https://github.com/appsinacup/godot-box2d) extension.

## Features

- Improved stability and performance, especially for large piles of bodies
- Faster, more accurate queries (e.g. `cast_motion`, `intersect_ray`)
- Continuous collision detection (CCD) enabled by default for dynamic vs static collisions
- Additional APIs to make the physics server easier and faster to use (docs coming soon)

## Unsupported Features / Limitations

Most of the Physics API has been implemented, but the following features do not have parity with Godot Physics:

- `WorldBoundaryShape2D` unsupported (planned for a future version of Box2D)
- `SeparationRayShape2D` unsupported
- Space parameters (`space_set_param`) unsupported because Box2D v3 uses a different solver
- "Cast Ray" CCD mode (`CCDMode::CCD_MODE_CAST_RAY`) unsupported
- Contacts are not visible when `Visible Collision Shapes` is enabled
- Convex polygons cannot have more than 8 vertices (does not affect `CollisionPolygon2D`)
- `CharacterBody2D`: unstable movement on moving platforms
- One-way collision is decided from the contact normal, ignoring the one-way
  collision margin. A rigid body that ends up overlapping a one-way platform from above is pushed
  back out rather than allowed to continue through. Character movement (`CharacterBody2D`) still
  accounts for penetration depth and margin.
- Collision exceptions place the two bodies in the same simulation island, so they sleep and wake
  together, and adding one at runtime wakes both.
- Removing a collision exception between two bodies that are already overlapping and at rest does
  not push them apart. Collision resumes once either body moves.

## Planned Features

- [Cross-platform determinism](https://box2d.org/posts/2024/08/determinism/)
- Serialization + Rollback

## Credits

Originally created by Andrew Song ([Pizzaandy](https://github.com/Pizzaandy)).
