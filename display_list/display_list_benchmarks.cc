// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/display_list_benchmarks.h"
#include "flutter/display_list/display_list_builder.h"

#include "third_party/skia/include/core/SkPoint.h"
#include "third_party/skia/include/core/SkTextBlob.h"

namespace flutter {
namespace testing {

// Constants chosen to produce benchmark results in the region of 1-50ms
constexpr size_t kLinesToDraw = 10000;
constexpr size_t kRectsToDraw = 5000;
constexpr size_t kOvalsToDraw = 1000;
constexpr size_t kCirclesToDraw = 5000;
constexpr size_t kRRectsToDraw = 5000;
constexpr size_t kArcSweepSetsToDraw = 1000;
constexpr size_t kImagesToDraw = 500;
constexpr size_t kFixedCanvasSize = 1024;

// Draw a series of diagonal lines across a square canvas of width/height of
// the length requested. The lines will start from the top left corner to the
// bottom right corner, and move from left to right (at the top) and from right
// to left (at the bottom) until 10,000 lines are drawn.
//
// The resulting image will be an hourglass shape.
void BM_DrawLine(benchmark::State& state,
                 std::unique_ptr<CanvasProvider> canvas_provider) {
  DisplayListBuilder builder;
  size_t length = state.range(0);

  canvas_provider->InitializeSurface(length, length);
  auto canvas = canvas_provider->GetSurface()->getCanvas();

  for (size_t i = 0; i < kLinesToDraw; i++) {
    builder.drawLine(SkPoint::Make(i % length, 0),
                     SkPoint::Make(length - i % length, length));
  }

  auto display_list = builder.Build();

  // We only want to time the actual rasterization.
  for (auto _ : state) {
    display_list->RenderTo(canvas);
    canvas_provider->GetSurface()->flushAndSubmit(true);
  }

  auto filename = canvas_provider->BackendName() + "-DrawLine-" +
                  std::to_string(state.range(0)) + ".png";
  canvas_provider->Snapshot(filename);
}

// Draws a series of square rects of the requested width across
// the canvas and repeats until `kRectsToDraw` rects have been drawn.
//
// Half the drawn rects will not have an integral offset.
void BM_DrawRect(benchmark::State& state,
                 std::unique_ptr<CanvasProvider> canvas_provider) {
  DisplayListBuilder builder;
  size_t length = state.range(0);
  size_t canvas_size = length * 2;
  canvas_provider->InitializeSurface(canvas_size, canvas_size);
  auto canvas = canvas_provider->GetSurface()->getCanvas();

  // As rects have SkScalar dimensions, we want to ensure that we also
  // draw rects with non-integer position and size
  const SkScalar offset = 0.5f;
  SkRect rect = SkRect::MakeLTRB(0, 0, length, length);

  for (size_t i = 0; i < kRectsToDraw; i++) {
    builder.drawRect(rect);
    rect.offset(offset, offset);
    if (rect.right() > canvas_size) {
      rect.offset(-canvas_size, 0);
    }
    if (rect.bottom() > canvas_size) {
      rect.offset(0, -canvas_size);
    }
  }

  auto display_list = builder.Build();

  // We only want to time the actual rasterization.
  for (auto _ : state) {
    display_list->RenderTo(canvas);
    canvas_provider->GetSurface()->flushAndSubmit(true);
  }

  auto filename = canvas_provider->BackendName() + "-DrawRect-" +
                  std::to_string(state.range(0)) + ".png";
  canvas_provider->Snapshot(filename);
}

// Draws a series of ovals of the requested height with aspect ratio 3:2 across
// the canvas and repeats until `kOvalsToDraw` ovals have been drawn.
//
// Half the drawn ovals will not have an integral offset.
void BM_DrawOval(benchmark::State& state,
                 std::unique_ptr<CanvasProvider> canvas_provider) {
  DisplayListBuilder builder;
  size_t length = state.range(0);
  size_t canvas_size = length * 2;
  canvas_provider->InitializeSurface(canvas_size, canvas_size);
  auto canvas = canvas_provider->GetSurface()->getCanvas();

  SkRect rect = SkRect::MakeXYWH(0, 0, length * 1.5f, length);
  const SkScalar offset = 0.5f;

  for (size_t i = 0; i < kOvalsToDraw; i++) {
    builder.drawOval(rect);
    rect.offset(offset, offset);
    if (rect.right() > canvas_size) {
      rect.offset(-canvas_size, 0);
    }
    if (rect.bottom() > canvas_size) {
      rect.offset(0, -canvas_size);
    }
  }
  auto display_list = builder.Build();

  // We only want to time the actual rasterization.
  for (auto _ : state) {
    display_list->RenderTo(canvas);
    canvas_provider->GetSurface()->flushAndSubmit(true);
  }

  auto filename = canvas_provider->BackendName() + "-DrawOval-" +
                  std::to_string(state.range(0)) + ".png";
  canvas_provider->Snapshot(filename);
}

// Draws a series of circles of the requested radius across
// the canvas and repeats until `kCirclesToDraw` circles have been drawn.
//
// Half the drawn circles will not have an integral center point.
void BM_DrawCircle(benchmark::State& state,
                   std::unique_ptr<CanvasProvider> canvas_provider) {
  DisplayListBuilder builder;
  size_t length = state.range(0);
  size_t canvas_size = length * 2;
  canvas_provider->InitializeSurface(canvas_size, canvas_size);
  auto canvas = canvas_provider->GetSurface()->getCanvas();

  SkScalar radius = length / 2.0f;
  const SkScalar offset = 0.5f;

  SkPoint center = SkPoint::Make(radius, radius);

  for (size_t i = 0; i < kCirclesToDraw; i++) {
    builder.drawCircle(center, radius);
    center.offset(offset, offset);
    if (center.x() + radius > canvas_size) {
      center.set(radius, center.y());
    }
    if (center.y() + radius > canvas_size) {
      center.set(center.x(), radius);
    }
  }
  auto display_list = builder.Build();

  // We only want to time the actual rasterization.
  for (auto _ : state) {
    display_list->RenderTo(canvas);
    canvas_provider->GetSurface()->flushAndSubmit(true);
  }

  auto filename = canvas_provider->BackendName() + "-DrawCircle-" +
                  std::to_string(state.range(0)) + ".png";
  canvas_provider->Snapshot(filename);
}

// Draws a series of rounded rects of the requested width across
// the canvas and repeats until `kRRectsToDraw` rects have been drawn.
//
// Half the drawn rounded rects will not have an integral offset.
void BM_DrawRRect(benchmark::State& state,
                  std::unique_ptr<CanvasProvider> canvas_provider,
                  SkRRect::Type type) {
  DisplayListBuilder builder;
  size_t length = state.range(0);
  size_t canvas_size = length * 2;
  canvas_provider->InitializeSurface(canvas_size, canvas_size);
  auto canvas = canvas_provider->GetSurface()->getCanvas();

  SkVector radii[4];
  switch (type) {
    case SkRRect::Type::kSimple_Type:
      radii[0] = SkVector::Make(5.0f, 5.0f);
      radii[1] = SkVector::Make(5.0f, 5.0f);
      radii[2] = SkVector::Make(5.0f, 5.0f);
      radii[3] = SkVector::Make(5.0f, 5.0f);
      break;
    case SkRRect::Type::kNinePatch_Type:
      radii[0] = SkVector::Make(5.0f, 2.0f);
      radii[1] = SkVector::Make(3.0f, 2.0f);
      radii[2] = SkVector::Make(3.0f, 4.0f);
      radii[3] = SkVector::Make(5.0f, 4.0f);
      break;
    case SkRRect::Type::kComplex_Type:
      radii[0] = SkVector::Make(5.0f, 4.0f);
      radii[1] = SkVector::Make(4.0f, 5.0f);
      radii[2] = SkVector::Make(3.0f, 6.0f);
      radii[3] = SkVector::Make(2.0f, 7.0f);
      break;
    default:
      break;
  }

  const SkScalar offset = 0.5f;
  const SkScalar multiplier = length / 16.0f;
  SkRRect rrect;

  SkVector set_radii[4];
  for (size_t i = 0; i < 4; i++) {
    set_radii[i] = radii[i] * multiplier;
  }
  rrect.setRectRadii(SkRect::MakeLTRB(0, 0, length, length), set_radii);

  for (size_t i = 0; i < kRRectsToDraw; i++) {
    builder.drawRRect(rrect);
    rrect.offset(offset, offset);
    if (rrect.rect().right() > canvas_size) {
      rrect.offset(-canvas_size, 0);
    }
    if (rrect.rect().bottom() > canvas_size) {
      rrect.offset(0, -canvas_size);
    }
  }
  auto display_list = builder.Build();

  // We only want to time the actual rasterization.
  for (auto _ : state) {
    display_list->RenderTo(canvas);
    canvas_provider->GetSurface()->flushAndSubmit(true);
  }

  auto filename = canvas_provider->BackendName() + "-DrawRRect-" +
                  std::to_string(state.range(0)) + ".png";
  canvas_provider->Snapshot(filename);
}

void BM_DrawArc(benchmark::State& state,
                std::unique_ptr<CanvasProvider> canvas_provider) {
  DisplayListBuilder builder;
  size_t length = state.range(0);
  size_t canvas_size = length * 2;
  canvas_provider->InitializeSurface(canvas_size, canvas_size);
  auto canvas = canvas_provider->GetSurface()->getCanvas();

  SkScalar starting_angle = 0.0f;
  SkScalar offset = 0.5f;

  // Just some random sweeps that will mostly circumnavigate the circle
  std::vector<SkScalar> segment_sweeps = {5.5f,  -10.0f, 42.0f, 71.7f, 90.0f,
                                          37.5f, 17.9f,  32.0f, 379.4f};

  SkRect bounds = SkRect::MakeLTRB(0, 0, length, length);

  for (size_t i = 0; i < kArcSweepSetsToDraw; i++) {
    for (SkScalar sweep : segment_sweeps) {
      builder.drawArc(bounds, starting_angle, sweep, false);
      starting_angle += sweep + 5.0f;
    }
    bounds.offset(offset, offset);
    if (bounds.right() > canvas_size) {
      bounds.offset(-canvas_size, 0);
    }
    if (bounds.bottom() > canvas_size) {
      bounds.offset(0, -canvas_size);
    }
  }

  auto display_list = builder.Build();

  // We only want to time the actual rasterization.
  for (auto _ : state) {
    display_list->RenderTo(canvas);
    canvas_provider->GetSurface()->flushAndSubmit(true);
  }

  auto filename = canvas_provider->BackendName() + "-DrawArc-" +
                  std::to_string(state.range(0)) + ".png";
  canvas_provider->Snapshot(filename);
}

// Returns a list of SkPoints that represent `n` points equally spaced out
// along the circumference of a circle with radius `r` and centered on `center`.
std::vector<SkPoint> GetPolygonPoints(size_t n, SkPoint center, SkScalar r) {
  std::vector<SkPoint> points;
  SkScalar x, y;
  float angle;
  float full_circle = 2.0f * M_PI;
  for (size_t i = 0; i < n; i++) {
    angle = (full_circle / (float)n) * (float)i;
    x = center.x() + r * std::cosf(angle);
    y = center.y() + r * std::sinf(angle);
    points.push_back(SkPoint::Make(x, y));
  }
  return points;
}

// Creates a path that represents a regular polygon with `sides` sides,
// centered on `center` with a radius of `radius`. The control points are
// equally spaced out along the circumference of the circle described by
// `radius` and `center`.
//
// The path segment connecting each control point is a line segment.
void GetLinesPath(SkPath& path, size_t sides, SkPoint center, float radius) {
  std::vector<SkPoint> points = GetPolygonPoints(sides, center, radius);
  path.moveTo(points[0]);
  for (size_t i = 1; i < sides; i++) {
    path.lineTo(points[i]);
  }
  path.lineTo(points[0]);
  path.close();
}

// Creates a path that represents a regular polygon with `sides` sides,
// centered on `center` with a radius of `radius`. The control points are
// equally spaced out along the circumference of the circle described by
// `radius` and `center`.
//
// The path segment connecting each control point is a quad bezier, with the
// bezier control point being on a circle with 80% of `radius` and with the
// control point angle half way between the start and end point angles for the
// polygon segment.
void GetQuadsPath(SkPath& path, size_t sides, SkPoint center, float radius) {
  std::vector<SkPoint> points = GetPolygonPoints(sides, center, radius);
  std::vector<SkPoint> control_points =
      GetPolygonPoints(sides * 2, center, radius * 0.8f);

  path.moveTo(points[0]);
  for (size_t i = 1; i < sides; i++) {
    path.quadTo(control_points[2 * i - 1], points[i]);
  }
  path.quadTo(control_points[2 * sides - 1], points[0]);
  path.close();
}

// Creates a path that represents a regular polygon with `sides` sides,
// centered on `center` with a radius of `radius`. The control points are
// equally spaced out along the circumference of the circle described by
// `radius` and `center`.
//
// The path segment connecting each control point is a conic, with the
// control point being on a circle with 80% of `radius` and with the
// control point angle half way between the start and end point angles for the
// polygon segment, and the conic weight set to 3.7f.
void GetConicsPath(SkPath& path, size_t sides, SkPoint center, float radius) {
  std::vector<SkPoint> points = GetPolygonPoints(sides, center, radius);
  std::vector<SkPoint> control_points =
      GetPolygonPoints(sides * 2, center, radius * 0.8f);

  path.moveTo(points[0]);
  for (size_t i = 1; i < sides; i++) {
    path.conicTo(control_points[2 * i - 1], points[i], 3.7f);
  }
  path.conicTo(control_points[2 * sides - 1], points[0], 3.7f);
  path.close();
}

// Creates a path that represents a regular polygon with `sides` sides,
// centered on `center` with a radius of `radius`. The control points are
// equally spaced out along the circumference of the circle described by
// `radius` and `center`.
//
// The path segment connecting each control point is a cubic, with the first
// control point being on a circle with 80% of `radius` and with the second
// control point being on a circle with 120% of `radius`. The first
// control point is 1/3, and the second control point is 2/3, of the angle
// between the start and end point angles for the polygon segment.
void GetCubicsPath(SkPath& path, size_t sides, SkPoint center, float radius) {
  std::vector<SkPoint> points = GetPolygonPoints(sides, center, radius);
  std::vector<SkPoint> inner_control_points =
      GetPolygonPoints(sides * 3, center, radius * 0.8f);
  std::vector<SkPoint> outer_control_points =
      GetPolygonPoints(sides * 3, center, radius * 1.2f);

  path.moveTo(points[0]);
  for (size_t i = 1; i < sides; i++) {
    path.cubicTo(inner_control_points[3 * i - 2],
                 outer_control_points[3 * i - 1], points[i]);
  }
  path.cubicTo(inner_control_points[3 * sides - 2],
               outer_control_points[3 * sides - 1], points[0]);
  path.close();
}

// Returns a path generated by one of the above path generators
// which is multiplied `number` times centered on each of the `number` control
// points along the circumference of a circle centered on `center` with radius
// `radius`.
//
// Each of the polygons will have `sides` sides, and the resulting path will be
// bounded by a circle with radius of 150% of `radius` (or another 20% on top of
// that for cubics)
void MultiplyPath(SkPath& path,
                  SkPath::Verb type,
                  SkPoint center,
                  size_t sides,
                  size_t number,
                  float radius) {
  std::vector<SkPoint> center_points =
      GetPolygonPoints(number, center, radius / 2.0f);

  for (SkPoint p : center_points) {
    switch (type) {
      case SkPath::Verb::kLine_Verb:
        GetLinesPath(path, sides, p, radius);
        break;
      case SkPath::Verb::kQuad_Verb:
        GetQuadsPath(path, sides, p, radius);
        break;
      case SkPath::Verb::kConic_Verb:
        GetConicsPath(path, sides, p, radius);
        break;
      case SkPath::Verb::kCubic_Verb:
        GetCubicsPath(path, sides, p, radius);
        break;
      default:
        break;
    }
  }
}

std::string VerbToString(SkPath::Verb type) {
  switch (type) {
    case SkPath::Verb::kLine_Verb:
      return "Lines";
    case SkPath::Verb::kQuad_Verb:
      return "Quads";
    case SkPath::Verb::kConic_Verb:
      return "Conics";
    case SkPath::Verb::kCubic_Verb:
      return "Cubics";
    default:
      return "Unknown";
  }
}

// Draws a series of overlapping 20-sided polygons where the path segment
// between each point is one of the verb types defined in SkPath.
//
// The number of polygons drawn will be varied to get an overall path
// with approximately 20*N verbs, so we can get an idea of the fixed
// cost of using drawPath as well as an idea of how the cost varies according
// to the verb count.
void BM_DrawPath(benchmark::State& state,
                 std::unique_ptr<CanvasProvider> canvas_provider,
                 SkPath::Verb type) {
  DisplayListBuilder builder;
  size_t length = kFixedCanvasSize;
  canvas_provider->InitializeSurface(length, length);
  auto canvas = canvas_provider->GetSurface()->getCanvas();

  SkPath path;

  std::string label = VerbToString(type);
  SkPoint center = SkPoint::Make(length / 2.0f, length / 2.0f);
  float radius = length * 0.25f;
  state.SetComplexityN(state.range(0));

  MultiplyPath(path, type, center, 20, state.range(0), radius);

  state.counters["VerbCount"] = path.countVerbs();

  builder.drawPath(path);
  auto display_list = builder.Build();

  // We only want to time the actual rasterization.
  for (auto _ : state) {
    display_list->RenderTo(canvas);
    canvas_provider->GetSurface()->flushAndSubmit(true);
  }

  auto filename = canvas_provider->BackendName() + "-DrawPath-" + label + "-" +
                  std::to_string(state.range(0)) + ".png";
  canvas_provider->Snapshot(filename);
}

// Returns a set of vertices that describe a circle that has a
// radius of `radius` and outer vertex count of approximately
// `vertex_count`. The final number of vertices will differ as we
// need to ensure the correct usage of vertices to ensure we do not
// request degenerate triangles be drawn. This final count is output
// through `final_vertex_count`.
//
// The resulting vertices will describe a disc consisting of a series
// of triangles with two vertices on the circumference of the disc,
// and the final vertex being the center point of the disc.
//
// Each vertex colour will alternate through Red, Green, Blue and Cyan.
sk_sp<SkVertices> GetTestVertices(SkPoint center,
                                  float radius,
                                  size_t vertex_count,
                                  SkVertices::VertexMode mode,
                                  size_t& final_vertex_count) {
  size_t outer_vertex_count = vertex_count / 2;
  std::vector<SkPoint> outer_points =
      GetPolygonPoints(outer_vertex_count, center, radius);

  std::vector<SkPoint> vertices;
  std::vector<SkColor> colors;

  switch (mode) {
    case SkVertices::VertexMode::kTriangleFan_VertexMode:
      // Calling the points on the outer circle O_0, O_1, O_2, ..., and
      // the center point C, this should create a triangle fan with vertices
      // C, O_0, O_1, O_2, O_3, ...
      vertices.push_back(center);
      colors.push_back(SK_ColorCYAN);
      for (size_t i = 0; i <= outer_points.size(); i++) {
        vertices.push_back(outer_points[i % outer_points.size()]);
        if (i % 3 == 0)
          colors.push_back(SK_ColorRED);
        else if (i % 3 == 1)
          colors.push_back(SK_ColorGREEN);
        else
          colors.push_back(SK_ColorBLUE);
      }
      break;
    case SkVertices::VertexMode::kTriangles_VertexMode:
      // Calling the points on the outer circle O_0, O_1, O_2, ..., and
      // the center point C, this should create a series of triangles with
      // vertices O_0, O_1, C, O_1, O_2, C, O_2, O_3, C, ...
      for (size_t i = 0; i < outer_vertex_count; i++) {
        vertices.push_back(outer_points[i % outer_points.size()]);
        colors.push_back(SK_ColorRED);
        vertices.push_back(outer_points[(i + 1) % outer_points.size()]);
        colors.push_back(SK_ColorGREEN);
        vertices.push_back(center);
        colors.push_back(SK_ColorBLUE);
      }
      break;
    case SkVertices::VertexMode::kTriangleStrip_VertexMode:
      // Calling the points on the outer circle O_0, O_1, O_2, ..., and
      // the center point C, this should create a strip with vertices
      // O_0, O_1, C, O_2, O_3, C, O_4, O_5, C, ...
      for (size_t i = 0; i <= outer_vertex_count; i++) {
        vertices.push_back(outer_points[i % outer_points.size()]);
        colors.push_back(i % 2 ? SK_ColorRED : SK_ColorGREEN);
        if (i % 2 == 1) {
          vertices.push_back(center);
          colors.push_back(SK_ColorBLUE);
        }
      }
      break;
    default:
      break;
  }

  final_vertex_count = vertices.size();
  return SkVertices::MakeCopy(mode, vertices.size(), vertices.data(), nullptr,
                              colors.data());
}

std::string VertexModeToString(SkVertices::VertexMode mode) {
  switch (mode) {
    case SkVertices::VertexMode::kTriangleStrip_VertexMode:
      return "TriangleStrip";
    case SkVertices::VertexMode::kTriangleFan_VertexMode:
      return "TriangleFan";
    case SkVertices::VertexMode::kTriangles_VertexMode:
      return "Triangles";
  }
  return "Unknown";
}

// Draws a series of discs generated by `GetTestVertices()` with
// 50 vertices in each disc. The number of discs drawn will vary according
// to the benchmark input, and the benchmark will automatically calculate
// the Big-O complexity of `DrawVertices` with N being the number of vertices
// being drawn.
//
// The discs drawn will be centered on points along a circle with radius of 25%
// of the canvas width/height, with each point being equally spaced out.
void BM_DrawVertices(benchmark::State& state,
                     std::unique_ptr<CanvasProvider> canvas_provider,
                     SkVertices::VertexMode mode) {
  DisplayListBuilder builder;
  size_t length = kFixedCanvasSize;
  canvas_provider->InitializeSurface(length, length);
  auto canvas = canvas_provider->GetSurface()->getCanvas();

  SkPoint center = SkPoint::Make(length / 2.0f, length / 2.0f);

  float radius = length / 4.0f;

  size_t vertex_count, total_vertex_count = 0;
  size_t disc_count = state.range(0);

  std::vector<SkPoint> center_points =
      GetPolygonPoints(disc_count, center, radius / 4.0f);

  for (SkPoint p : center_points) {
    sk_sp<SkVertices> vertices =
        GetTestVertices(p, radius, 50, mode, vertex_count);
    total_vertex_count += vertex_count;
    builder.drawVertices(vertices, SkBlendMode::kSrc);
  }

  state.counters["VertexCount"] = total_vertex_count;
  state.SetComplexityN(total_vertex_count);

  auto display_list = builder.Build();

  // We only want to time the actual rasterization.
  for (auto _ : state) {
    display_list->RenderTo(canvas);
    canvas_provider->GetSurface()->flushAndSubmit(true);
  }

  auto filename = canvas_provider->BackendName() + "-DrawVertices-" +
                  std::to_string(disc_count) + "-" + VertexModeToString(mode) +
                  ".png";
  canvas_provider->Snapshot(filename);
}

// Generate `count` test points.
//
// The points are distributed using some fixed constant offsets that were
// chosen to appear somewhat random.
//
// The points generated will wrap in x and y for the bounds of `canvas_size`.
std::vector<SkPoint> GetTestPoints(size_t count, SkISize canvas_size) {
  std::vector<SkPoint> points;

  // Some arbitrary offsets to use when building the list of points
  std::vector<SkScalar> delta_x = {10.0f, 6.3f, 15.0f, 3.5f, 22.6f, 4.7f};
  std::vector<SkScalar> delta_y = {9.3f, -5.4f, 8.5f, -12.0f, 19.2f, -19.6f};

  SkPoint current = SkPoint::Make(0.0f, 0.0f);
  for (size_t i = 0; i < count; i++) {
    points.push_back(current);
    current.offset(delta_x[i % delta_x.size()], delta_y[i % delta_y.size()]);
    if (current.x() > canvas_size.width()) {
      current.offset(-canvas_size.width(), 25.0f);
    }
    if (current.y() > canvas_size.height()) {
      current.offset(0.0f, -canvas_size.height());
    }
  }

  return points;
}

std::string PointModeToString(SkCanvas::PointMode mode) {
  switch (mode) {
    case SkCanvas::kLines_PointMode:
      return "Lines";
    case SkCanvas::kPolygon_PointMode:
      return "Polygon";
    case SkCanvas::kPoints_PointMode:
    default:
      return "Points";
  }
}

// Draws a series of points generated by `GetTestPoints()` above to
// a fixed-size canvas. The benchmark will vary the number of points drawn,
// and they can be drawn in one of three modes - Lines, Polygon or Points mode.
//
// This benchmark will automatically calculate the Big-O complexity of
// `DrawPoints` with N being the number of points being drawn.
void BM_DrawPoints(benchmark::State& state,
                   std::unique_ptr<CanvasProvider> canvas_provider,
                   SkCanvas::PointMode mode) {
  DisplayListBuilder builder;
  size_t length = kFixedCanvasSize;
  canvas_provider->InitializeSurface(length, length);
  auto canvas = canvas_provider->GetSurface()->getCanvas();

  size_t point_count = state.range(0);
  state.SetComplexityN(point_count);
  state.counters["PointCount"] = point_count;

  std::vector<SkPoint> points =
      GetTestPoints(point_count, SkISize::Make(length, length));
  builder.drawPoints(mode, points.size(), points.data());

  auto display_list = builder.Build();

  for (auto _ : state) {
    display_list->RenderTo(canvas);
    canvas_provider->GetSurface()->flushAndSubmit(true);
  }

  auto filename = canvas_provider->BackendName() + "-DrawPoints-" +
                  PointModeToString(mode) + "-" + std::to_string(point_count) +
                  ".png";
  canvas_provider->Snapshot(filename);
}

sk_sp<SkImage> ImageFromBitmapWithNewID(const SkBitmap& bitmap) {
  // If we create an SkPixmap with a ref to the SkBitmap's pixel data,
  // then create an SkImage from that, we always get a new generation ID,
  // so we will avoid hitting the cache.
  SkPixmap pixmap;
  bitmap.peekPixels(&pixmap);
  return SkImage::MakeFromRaster(pixmap, nullptr, nullptr);
}

// Draws `kImagesToDraw` bitmaps to a canvas, either with texture-backed
// bitmaps or bitmaps that need to be uploaded to the GPU first.
void BM_DrawImage(benchmark::State& state,
                  std::unique_ptr<CanvasProvider> canvas_provider,
                  const SkSamplingOptions& options,
                  bool upload_bitmap) {
  DisplayListBuilder builder;
  size_t bitmap_size = state.range(0);
  size_t canvas_size = 2 * bitmap_size;
  canvas_provider->InitializeSurface(canvas_size, canvas_size);
  auto canvas = canvas_provider->GetSurface()->getCanvas();

  sk_sp<SkImage> image;
  sk_sp<SkSurface> offscreen;
  SkBitmap bitmap;

  if (upload_bitmap) {
    SkImageInfo info = SkImageInfo::Make(bitmap_size, bitmap_size,
                                         SkColorType::kRGBA_8888_SkColorType,
                                         SkAlphaType::kPremul_SkAlphaType);
    bitmap.allocPixels(info, 0);
    bitmap.eraseColor(SK_ColorBLUE);
  } else {
    offscreen = canvas_provider->MakeOffscreenSurface(bitmap_size, bitmap_size);
    offscreen->getCanvas()->clear(SK_ColorRED);
  }

  SkScalar offset = 0.5f;
  SkPoint dst = SkPoint::Make(0, 0);

  for (size_t i = 0; i < kImagesToDraw; i++) {
    image = upload_bitmap ? ImageFromBitmapWithNewID(bitmap)
                          : offscreen->makeImageSnapshot();
    builder.drawImage(image, dst, options, true);

    dst.offset(offset, offset);
    if (dst.x() + bitmap_size > canvas_size) {
      dst.set(0, dst.y());
    }
    if (dst.y() + bitmap_size > canvas_size) {
      dst.set(dst.x(), 0);
    }
  }

  auto display_list = builder.Build();

  for (auto _ : state) {
    display_list->RenderTo(canvas);
    canvas_provider->GetSurface()->flushAndSubmit(true);
  }

  auto filename = canvas_provider->BackendName() + "-DrawImage-" +
                  (upload_bitmap ? "Upload-" : "Texture-") +
                  std::to_string(bitmap_size) + ".png";
  canvas_provider->Snapshot(filename);
}

std::string ConstraintToString(SkCanvas::SrcRectConstraint constraint) {
  switch (constraint) {
    case SkCanvas::SrcRectConstraint::kStrict_SrcRectConstraint:
      return "Strict";
    case SkCanvas::SrcRectConstraint::kFast_SrcRectConstraint:
      return "Fast";
    default:
      return "Unknown";
  }
}

// Draws `kImagesToDraw` bitmaps to a canvas, either with texture-backed
// bitmaps or bitmaps that need to be uploaded to the GPU first.
//
// The bitmaps are shrunk down to 75% of their size when rendered to the canvas.
void BM_DrawImageRect(benchmark::State& state,
                      std::unique_ptr<CanvasProvider> canvas_provider,
                      const SkSamplingOptions& options,
                      SkCanvas::SrcRectConstraint constraint,
                      bool upload_bitmap) {
  DisplayListBuilder builder;
  size_t bitmap_size = state.range(0);
  size_t canvas_size = 2 * bitmap_size;
  canvas_provider->InitializeSurface(canvas_size, canvas_size);
  auto canvas = canvas_provider->GetSurface()->getCanvas();

  sk_sp<SkImage> image;
  sk_sp<SkSurface> offscreen;
  SkBitmap bitmap;

  if (upload_bitmap) {
    SkImageInfo info = SkImageInfo::Make(bitmap_size, bitmap_size,
                                         SkColorType::kRGBA_8888_SkColorType,
                                         SkAlphaType::kPremul_SkAlphaType);
    bitmap.allocPixels(info, 0);
    bitmap.eraseColor(SK_ColorBLUE);
  } else {
    offscreen = canvas_provider->MakeOffscreenSurface(bitmap_size, bitmap_size);
    offscreen->getCanvas()->clear(SK_ColorRED);
  }

  SkScalar offset = 0.5f;
  SkRect src = SkRect::MakeXYWH(bitmap_size / 4.0f, bitmap_size / 4.0f,
                                bitmap_size / 2.0f, bitmap_size / 2.0f);
  SkRect dst =
      SkRect::MakeXYWH(0.0f, 0.0f, bitmap_size * 0.75f, bitmap_size * 0.75f);

  for (size_t i = 0; i < kImagesToDraw; i++) {
    image = upload_bitmap ? ImageFromBitmapWithNewID(bitmap)
                          : offscreen->makeImageSnapshot();
    builder.drawImageRect(image, src, dst, options, true, constraint);
    dst.offset(offset, offset);
    if (dst.right() > canvas_size) {
      dst.offsetTo(0, dst.y());
    }
    if (dst.bottom() > canvas_size) {
      dst.offsetTo(dst.x(), 0);
    }
  }

  auto display_list = builder.Build();

  for (auto _ : state) {
    display_list->RenderTo(canvas);
    canvas_provider->GetSurface()->flushAndSubmit(true);
  }

  auto filename = canvas_provider->BackendName() + "-DrawImageRect-" +
                  (upload_bitmap ? "Upload-" : "Texture-") +
                  ConstraintToString(constraint) + "-" +
                  std::to_string(bitmap_size) + ".png";
  canvas_provider->Snapshot(filename);
}

std::string FilterModeToString(const SkFilterMode mode) {
  switch (mode) {
    case SkFilterMode::kNearest:
      return "Nearest";
    case SkFilterMode::kLinear:
      return "Linear";
    default:
      return "Unknown";
  }
}

// Draws `kImagesToDraw` bitmaps to a canvas, either with texture-backed
// bitmaps or bitmaps that need to be uploaded to the GPU first.
//
// The image is split into 9 sub-rects and stretched proportionally for final
// rendering.
void BM_DrawImageNine(benchmark::State& state,
                      std::unique_ptr<CanvasProvider> canvas_provider,
                      const SkFilterMode filter,
                      bool upload_bitmap) {
  DisplayListBuilder builder;
  size_t bitmap_size = state.range(0);
  size_t canvas_size = 2 * bitmap_size;
  canvas_provider->InitializeSurface(canvas_size, canvas_size);
  auto canvas = canvas_provider->GetSurface()->getCanvas();

  SkIRect center = SkIRect::MakeXYWH(bitmap_size / 4, bitmap_size / 4,
                                     bitmap_size / 2, bitmap_size / 2);

  sk_sp<SkImage> image;
  sk_sp<SkSurface> offscreen;
  SkBitmap bitmap;

  if (upload_bitmap) {
    SkImageInfo info = SkImageInfo::Make(bitmap_size, bitmap_size,
                                         SkColorType::kRGBA_8888_SkColorType,
                                         SkAlphaType::kPremul_SkAlphaType);
    bitmap.allocPixels(info, 0);
    bitmap.eraseColor(SK_ColorBLUE);
  } else {
    offscreen = canvas_provider->MakeOffscreenSurface(bitmap_size, bitmap_size);
    offscreen->getCanvas()->clear(SK_ColorRED);
  }

  SkScalar offset = 0.5f;
  SkRect dst =
      SkRect::MakeXYWH(0.0f, 0.0f, bitmap_size * 0.75f, bitmap_size * 0.75f);

  for (size_t i = 0; i < kImagesToDraw; i++) {
    image = upload_bitmap ? ImageFromBitmapWithNewID(bitmap)
                          : offscreen->makeImageSnapshot();
    builder.drawImageNine(image, center, dst, filter, true);
    dst.offset(offset, offset);
    if (dst.right() > canvas_size) {
      dst.offsetTo(0, dst.y());
    }
    if (dst.bottom() > canvas_size) {
      dst.offsetTo(dst.x(), 0);
    }
  }

  auto display_list = builder.Build();

  for (auto _ : state) {
    display_list->RenderTo(canvas);
    canvas_provider->GetSurface()->flushAndSubmit(true);
  }

  auto filename = canvas_provider->BackendName() + "-DrawImageNine-" +
                  (upload_bitmap ? "Upload-" : "Texture-") +
                  FilterModeToString(filter) + "-" +
                  std::to_string(bitmap_size) + ".png";
  canvas_provider->Snapshot(filename);
}

// Draws a series of glyph runs with 32 glyphs in each run. The number of runs
// may vary according to the benchmark parameters. The text will start in the
// upper left corner of the canvas and advance from left to right and wrap at
// the canvas boundaries in both x and y.
//
// This benchmark will automatically calculate the Big-O complexity of
// `DrawTextBlob` with N being the number of glyphs being drawn.
void BM_DrawTextBlob(benchmark::State& state,
                     std::unique_ptr<CanvasProvider> canvas_provider) {
  DisplayListBuilder builder;
  size_t glyph_runs = state.range(0);
  size_t canvas_size = kFixedCanvasSize;
  canvas_provider->InitializeSurface(canvas_size, canvas_size);
  auto canvas = canvas_provider->GetSurface()->getCanvas();

  // We're just using plain Latin-1 where glyph count == character count
  const char* string_fragment = "This text has exactly 32 glyphs.";
  size_t fragment_length = strlen(string_fragment);
  state.SetComplexityN(glyph_runs * fragment_length);

  // TODO(gw280): different fonts
  SkFont font;

  auto blob_fragment = SkTextBlob::MakeFromString(string_fragment, font);
  auto bounds = blob_fragment->bounds();

  // Calculate the approximate number of these glyph runs we can fit on a single
  // canvas.
  size_t x_count_max = canvas_size / bounds.width();
  size_t y_count_max = canvas_size / bounds.height();
  size_t remaining_runs = glyph_runs;

  SkTextBlobBuilder blob_builder;
  size_t current_y = 0;
  while (remaining_runs > 0) {
    size_t runs_this_pass = std::min(x_count_max, remaining_runs);
    auto buffer = blob_builder.allocRun(
        font, runs_this_pass * fragment_length, 0,
        ((current_y % y_count_max) + 1) * bounds.height());
    for (size_t i = 0; i < runs_this_pass; i++) {
      font.textToGlyphs(string_fragment, fragment_length, SkTextEncoding::kUTF8,
                        buffer.glyphs + (i * fragment_length), fragment_length);
    }
    remaining_runs -= runs_this_pass;
    current_y++;
  }

  auto blob = blob_builder.make();

  builder.drawTextBlob(blob, 0.0f, 0.0f);

  auto display_list = builder.Build();

  for (auto _ : state) {
    display_list->RenderTo(canvas);
    canvas_provider->GetSurface()->flushAndSubmit(true);
  }

  auto filename = canvas_provider->BackendName() + "-DrawTextBlob-" +
                  std::to_string(glyph_runs * fragment_length) + ".png";
  canvas_provider->Snapshot(filename);
}

// Draw the shadow for a 10-sided regular polygon where the polygon's
// sides are denoted by one of a Line, Quad, Conic or Cubic path segment.
//
// The elevation of the light source will vary according to the benchmark
// paremeters.
//
// The benchmark can be run with either a transparent occluder or an opaque
// occluder.
void BM_DrawShadow(benchmark::State& state,
                   std::unique_ptr<CanvasProvider> canvas_provider,
                   bool transparent_occluder,
                   SkPath::Verb type) {
  DisplayListBuilder builder;
  size_t length = kFixedCanvasSize;
  canvas_provider->InitializeSurface(length, length);
  auto canvas = canvas_provider->GetSurface()->getCanvas();

  SkPath path;

  SkPoint center = SkPoint::Make(length / 2.0f, length / 2.0f);
  float radius = length * 0.25f;

  switch (type) {
    case SkPath::Verb::kLine_Verb:
      GetLinesPath(path, 10, center, radius);
      break;
    case SkPath::Verb::kQuad_Verb:
      GetQuadsPath(path, 10, center, radius);
      break;
    case SkPath::Verb::kConic_Verb:
      GetConicsPath(path, 10, center, radius);
      break;
    case SkPath::Verb::kCubic_Verb:
      GetCubicsPath(path, 10, center, radius);
      break;
    default:
      break;
  }

  float elevation = state.range(0);

  // We can hardcode dpr to 1.0f as we're varying elevation, and dpr is only
  // ever used in conjunction with elevation.
  builder.drawShadow(path, SK_ColorBLUE, elevation, transparent_occluder, 1.0f);
  auto display_list = builder.Build();

  // We only want to time the actual rasterization.
  for (auto _ : state) {
    display_list->RenderTo(canvas);
    canvas_provider->GetSurface()->flushAndSubmit(true);
  }

  auto filename = canvas_provider->BackendName() + "-DrawShadow-" +
                  VerbToString(type) + "-" +
                  (transparent_occluder ? "Transparent-" : "Opaque-") +
                  std::to_string(elevation) + "-" + ".png";
  canvas_provider->Snapshot(filename);
}

}  // namespace testing
}  // namespace flutter
