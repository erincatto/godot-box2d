#include "box2d_world_boundary_shape_2d.h"
#include "../bodies/box2d_collision_object_2d.h"

void Box2DWorldBoundaryShape2D::add_to_body(Box2DShapeInstance *p_instance) const {
	b2Polygon shape;
	if (!make_boundary(p_instance->get_global_transform(), data, shape)) {
		return;
	}
	b2ShapeDef shape_def = p_instance->get_shape_def();
	b2ShapeId id = b2CreatePolygonShape(p_instance->get_collision_object()->get_body_id(), &shape_def, &shape);
	p_instance->add_shape_id(id);
}

int Box2DWorldBoundaryShape2D::cast(const CastQuery &p_query, const Transform2D &p_transform, LocalVector<CastHit> &p_results) const {
	b2Polygon shape;
	if (!make_boundary(p_transform, data, shape)) {
		return 0;
	}
	return box2d_cast_shape(shape, p_query, p_results);
}

int Box2DWorldBoundaryShape2D::overlap(const OverlapQuery &p_query, const Transform2D &p_transform, LocalVector<ShapeOverlap> &p_results) const {
	b2Polygon shape;
	if (!make_boundary(p_transform, data, shape)) {
		return 0;
	}
	return box2d_overlap_shape(shape, p_query, p_results);
}

bool Box2DWorldBoundaryShape2D::make_boundary(const Transform2D &p_transform, const Variant &p_data, b2Polygon &p_polygon) {
	ERR_FAIL_COND_V(p_data.get_type() != Variant::ARRAY, false);
	Array arr = p_data;
	ERR_FAIL_COND_V(arr.size() != 2, false);

	Vector2 normal = arr[0];
	real_t distance = arr[1];
	ERR_FAIL_COND_V(normal.is_zero_approx(), false);
	normal = normal.normalized();

	// Polygon contacts are computed relative to a corner of one polygon, so float round off grows
	// with the extent. Two kilometers keeps it to a few hundredths of the linear slop.
	const real_t extent = 2000.0f * b2GetLengthUnitsPerMeter();

	Vector2 tangent = normal.orthogonal() * extent;
	Vector2 surface = normal * distance;
	Vector2 depth = normal * extent;

	b2Vec2 points[4] = {
		to_box2d(p_transform.xform(surface + tangent)),
		to_box2d(p_transform.xform(surface - tangent)),
		to_box2d(p_transform.xform(surface - tangent - depth)),
		to_box2d(p_transform.xform(surface + tangent - depth))
	};

	b2Hull hull = b2ComputeHull(points, 4);

	// A zero scale collapses the box
	ERR_FAIL_COND_V(hull.count != 4, false);

	p_polygon = b2MakePolygon(&hull, 0.0f);

	return true;
}
