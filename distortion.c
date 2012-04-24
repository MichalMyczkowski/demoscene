#include <math.h>

#include "std/memory.h"
#include "distortion.h"

static void DeleteDistortionMap(DistortionMapT *map) {
  MemUnref(map->map);
}

DistortionMapT *NewDistortionMap(uint16_t width, uint16_t height) {
  DistortionMapT *map = NewRecordGC(DistortionMapT, (FreeFuncT)DeleteDistortionMap);

  map->width = width;
  map->height = height;
  map->map = NewTable(uint16_t, width * height);

  return map;
}

void GenerateTunnel(DistortionMapT *tunnel,
                    int16_t radius, int16_t centerX, int16_t centerY) {
  int minX = -centerX;
  int minY = -centerY;
  int maxX = minX + tunnel->width;
  int maxY = minY + tunnel->height;
  uint16_t *map = tunnel->map;

  int xi, yi;

  for (yi = minY; yi < maxY; yi++) {
    float y = (float)yi;

    for (xi = minX; xi < maxX; xi++) {
      float x = (float)xi;

      int offsetY = atan((float)x / (float)y) * 128.0f / M_PI;
      int offsetX = radius / sqrt(x * x + y * y);

      if (yi >= 0)
        offsetY += 128;

      *map++ = ((offsetY & 0xff) << 8) | (offsetX & 0xff);
    }
  }
}
