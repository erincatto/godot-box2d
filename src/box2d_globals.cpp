#include "box2d_globals.h"

void box2d_set_pixels_per_meter(float p_value) {
	// A non-positive length unit zeroes every tolerance Box2D derives from it, so bail before
	// the world is built rather than simulate with no slop and no speculative distance.
	ERR_FAIL_COND_MSG(p_value <= 0.0f, "Pixels per meter must be positive.");

	// Must run before any b2Default*Def call, those bake the length unit into their defaults.
	b2SetLengthUnitsPerMeter(p_value);
}

// TODO: revisit, consider implementing Godot-style cast function
float box2d_compute_safe_fraction(float p_unsafe_fraction, float p_total_distance, float p_amount) {
	if (p_amount <= 0.0f) {
		p_amount = 2.0f * B2_LINEAR_SLOP;
	}

	if (p_total_distance <= 0.0f) {
		return 0.0f;
	}

	if (p_unsafe_fraction >= 1.0f) {
		return 1.0f;
	}

	float distance = p_unsafe_fraction * p_total_distance;
	float adjusted_distance = Math::max(0.0f, distance - p_amount);

	return adjusted_distance / p_total_distance;
}

ShapeCollideResult box2d_collide_shapes(
		const Box2DShapePrimitive &p_shape_a,
		const b2Transform &xfa,
		const Box2DShapePrimitive &p_shape_b,
		const b2Transform &xfb,
		bool p_swapped) {
	b2ShapeType type_a = p_shape_a.type;
	b2ShapeType type_b = p_shape_b.type;

	// The collide functions work in frame A and want B relative to it.
	b2Transform xf = b2InvMulTransforms(xfa, xfb);

	b2LocalManifold manifold = { 0 };

	switch (type_a) {
		case b2ShapeType::b2_capsuleShape: {
			b2Capsule a = p_shape_a.capsule;
			switch (type_b) {
				case b2ShapeType::b2_capsuleShape: {
					manifold = b2CollideCapsules(&a, &p_shape_b.capsule, xf);
					break;
				}
				case b2ShapeType::b2_circleShape: {
					manifold = b2CollideCapsuleAndCircle(&a, &p_shape_b.circle, xf);
					break;
				}
				case b2ShapeType::b2_polygonShape:
				case b2ShapeType::b2_segmentShape:
				case b2ShapeType::b2_chainSegmentShape: {
					return box2d_collide_shapes(p_shape_b, xfb, p_shape_a, xfa, true);
				}
				default: {
					ERR_FAIL_V({});
				}
			}
			break;
		}
		case b2ShapeType::b2_circleShape: {
			b2Circle a = p_shape_a.circle;
			switch (type_b) {
				case b2ShapeType::b2_capsuleShape:
				case b2ShapeType::b2_polygonShape:
				case b2ShapeType::b2_segmentShape:
				case b2ShapeType::b2_chainSegmentShape: {
					return box2d_collide_shapes(p_shape_b, xfb, p_shape_a, xfa, true);
				}
				case b2ShapeType::b2_circleShape: {
					manifold = b2CollideCircles(&a, &p_shape_b.circle, xf);
					break;
				}
				default: {
					ERR_FAIL_V({});
				}
			}
			break;
		}
		case b2ShapeType::b2_polygonShape: {
			b2Polygon a = p_shape_a.polygon;
			switch (type_b) {
				case b2ShapeType::b2_capsuleShape: {
					manifold = b2CollidePolygonAndCapsule(&a, &p_shape_b.capsule, xf);
					break;
				}
				case b2ShapeType::b2_circleShape: {
					manifold = b2CollidePolygonAndCircle(&a, &p_shape_b.circle, xf);
					break;
				}
				case b2ShapeType::b2_polygonShape: {
					manifold = b2CollidePolygons(&a, &p_shape_b.polygon, xf);
					break;
				}
				case b2ShapeType::b2_segmentShape:
				case b2ShapeType::b2_chainSegmentShape: {
					return box2d_collide_shapes(p_shape_b, xfb, p_shape_a, xfa, true);
				}
				default: {
					ERR_FAIL_V({});
				}
			}
			break;
		}
		case b2ShapeType::b2_segmentShape: {
			b2Segment a = p_shape_a.segment;
			switch (type_b) {
				case b2ShapeType::b2_capsuleShape: {
					manifold = b2CollideSegmentAndCapsule(&a, &p_shape_b.capsule, xf);
					break;
				}
				case b2ShapeType::b2_circleShape: {
					manifold = b2CollideSegmentAndCircle(&a, &p_shape_b.circle, xf);
					break;
				}
				case b2ShapeType::b2_polygonShape: {
					manifold = b2CollideSegmentAndPolygon(&a, &p_shape_b.polygon, xf);
					break;
				}
				case b2ShapeType::b2_segmentShape:
				case b2ShapeType::b2_chainSegmentShape: {
					return {};
				}
				default: {
					ERR_FAIL_V({});
				}
			}
			break;
		}
		case b2ShapeType::b2_chainSegmentShape: {
			b2ChainSegment a = p_shape_a.chain_segment;
			switch (type_b) {
				case b2ShapeType::b2_capsuleShape: {
					b2SimplexCache cache{ 0 };
					manifold = b2CollideChainSegmentAndCapsule(&a, &p_shape_b.capsule, xf, &cache);
					break;
				}
				case b2ShapeType::b2_circleShape: {
					manifold = b2CollideChainSegmentAndCircle(&a, &p_shape_b.circle, xf);
					break;
				}
				case b2ShapeType::b2_polygonShape: {
					b2SimplexCache cache{ 0 };
					manifold = b2CollideChainSegmentAndPolygon(&a, &p_shape_b.polygon, xf, &cache);
					break;
				}
				case b2ShapeType::b2_segmentShape:
				case b2ShapeType::b2_chainSegmentShape: {
					return {};
				}
				default: {
					ERR_FAIL_V({});
				}
			}
			break;
		}
		default: {
			ERR_FAIL_V({});
		}
	}

	ShapeCollideResult result;

	result.point_count = manifold.pointCount;

	if (result.point_count == 0) {
		return result;
	}

	result.normal = -to_godot_normalized(b2RotateVector(xfa.q, manifold.normal));

	if (p_swapped) {
		result.normal *= -1.0f;
	}

	// Box2D puts the contact point midway between the surfaces. Godot wants it on one of them.
	for (int i = 0; i < manifold.pointCount; i++) {
		result.points[i].depth = -to_godot(manifold.points[i].separation);
		result.points[i].point = to_godot(b2TransformPoint(xfa, manifold.points[i].point)) + (0.5f * result.points[i].depth * result.normal);
	}

	return result;
}