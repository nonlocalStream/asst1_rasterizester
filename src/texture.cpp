#include "texture.h"
#include "CGL/color.h"
#define square(a) a*a

namespace CGL {

// Examines the enum parameters in sp and performs
// the appropriate sampling using the three helper functions below.
Color Texture::sample(const SampleParams &sp) {
  // Part 6: Fill in the functionality for sampling 
  //          nearest or bilinear in mipmap level 0, conditional on sp.psm
  // Part 7: Fill in full sampling (including trilinear), 
  //          conditional on sp.psm and sp.lsm
  int level;
  Color (Texture::*sample_method)(Vector2D, int);
  switch (sp.psm) {
      case P_NEAREST: 
          sample_method = &Texture::sample_nearest;
          break;
      case P_LINEAR:     
          sample_method = &Texture::sample_bilinear;
          break;
  }
  //return Color();
  switch (sp.lsm) {
      case L_ZERO:
          //cout << "l_zero" <<endl;
          return (*this.*sample_method)(sp.uv, 0);
      case L_NEAREST:
          level = round(get_level(sp));
          //cout << "l_nearest" <<endl;
          if (level < 0) {
              level = 0;
          } else {
              cout << level << endl;
          }
          return (*this.*sample_method)(sp.uv, level);
      case L_LINEAR:
          //cout << "l_linear" <<endl;
          float lev = get_level(sp);
          level = floor(lev);
          float dis = lev - level;
          Color s0 = (*this.*sample_method)(sp.uv, level);
          Color s1 = (*this.*sample_method)(sp.uv, level+1);
          return lerp(dis, s0, s1);
  }
}

// Given sp.du and sp.dv, returns the appropriate mipmap
// level to use for L_NEAREST or L_LINEAR filtering.
float Texture::get_level(const SampleParams &sp) {
  // Part 7: Fill this in.
  //
  float l1 = sqrt(square(sp.du[0]*width)+square(sp.dv[0]*height));
  float l2 = sqrt(square(sp.du[1]*width)+square(sp.dv[1]*height));
  float l = (l1 > l2)? l1 :l2;
  float level = log(l)/log(2);
  if (level >= mipmap.size())
      level = mipmap.size()-1;
  return level;
}

Color Texture::get_texel(int x, int y, int level) {
  unsigned char *p = &(mipmap[level].texels[0]) + 4 * (x + y * mipmap[level].width);
  float r = (float) p[0] / 255.;
  float g = (float) p[1] / 255.;
  float b = (float) p[2] / 255.;
  float a = (float) p[3] / 255.;
/*  cout << "r:" << (int) p[0]
       << " g:" << (int) p[1]
       << " b:" << (int) p[2] 
       << " a:" << (int) p[3] << endl;
       */
  return Color(r,g,b,a);

}
// Indexes into the level'th mipmap
// and returns the nearest pixel to (u,v)
Color Texture::sample_nearest(Vector2D uv, int level) {
  // Part 6: Fill this in.
  int w = mipmap[level].width;
  int h = mipmap[level].height;
  int x = (int) round(uv[0] * w);
  if (x < 0) x = 0;
  if (x >= w) x = w - 1;
  int y = (int) round(uv[1] * h);
  if (y < 0) y = 0;
  if (y >= h) y = h - 1;
  return get_texel(x, y, level);
}

Color Texture::lerp(float dis, Color c1, Color c2) {
    // dis is the ratio of distance to c1 / distance between c1 and c2
    return (1-dis)*c1 + dis*c2;
}
// Indexes into the level'th mipmap
// and returns a bilinearly weighted combination of
// the four pixels surrounding (u,v)
Color Texture::sample_bilinear(Vector2D uv, int level) {
  // Part 6: Fill this in
  float x = uv[0] * width;  
  float y = uv[1] * height;
  Color u00 = get_texel(floor(x), floor(y), level);
  Color u01 = get_texel(floor(x), ceil(y), level);
  Color u10 = get_texel(ceil(x), floor(y), level);
  Color u11 = get_texel(ceil(x), ceil(y), level);
  float s = x - floor(x);
  float t = y - floor(y);
  Color u0 = lerp(s, u00, u10);
  Color u1 = lerp(s, u01, u11);
  Color u = lerp(t, u0, u1);
  return u;
}



/****************************************************************************/



inline void uint8_to_float(float dst[4], unsigned char *src) {
  uint8_t *src_uint8 = (uint8_t *)src;
  dst[0] = src_uint8[0] / 255.f;
  dst[1] = src_uint8[1] / 255.f;
  dst[2] = src_uint8[2] / 255.f;
  dst[3] = src_uint8[3] / 255.f;
}

inline void float_to_uint8(unsigned char *dst, float src[4]) {
  uint8_t *dst_uint8 = (uint8_t *)dst;
  dst_uint8[0] = (uint8_t)(255.f * max(0.0f, min(1.0f, src[0])));
  dst_uint8[1] = (uint8_t)(255.f * max(0.0f, min(1.0f, src[1])));
  dst_uint8[2] = (uint8_t)(255.f * max(0.0f, min(1.0f, src[2])));
  dst_uint8[3] = (uint8_t)(255.f * max(0.0f, min(1.0f, src[3])));
}

void Texture::generate_mips(int startLevel) {

  // make sure there's a valid texture
  if (startLevel >= mipmap.size()) {
    std::cerr << "Invalid start level";
  }

  // allocate sublevels
  int baseWidth = mipmap[startLevel].width;
  int baseHeight = mipmap[startLevel].height;
  int numSubLevels = (int)(log2f((float)max(baseWidth, baseHeight)));

  numSubLevels = min(numSubLevels, kMaxMipLevels - startLevel - 1);
  mipmap.resize(startLevel + numSubLevels + 1);

  int width = baseWidth;
  int height = baseHeight;
  for (int i = 1; i <= numSubLevels; i++) {

    MipLevel &level = mipmap[startLevel + i];

    // handle odd size texture by rounding down
    width = max(1, width / 2);
    //assert (width > 0);
    height = max(1, height / 2);
    //assert (height > 0);

    level.width = width;
    level.height = height;
    level.texels = vector<unsigned char>(4 * width * height);
  }

  // create mips
  int subLevels = numSubLevels - (startLevel + 1);
  for (int mipLevel = startLevel + 1; mipLevel < startLevel + subLevels + 1;
       mipLevel++) {

    MipLevel &prevLevel = mipmap[mipLevel - 1];
    MipLevel &currLevel = mipmap[mipLevel];

    int prevLevelPitch = prevLevel.width * 4; // 32 bit RGBA
    int currLevelPitch = currLevel.width * 4; // 32 bit RGBA

    unsigned char *prevLevelMem;
    unsigned char *currLevelMem;

    currLevelMem = (unsigned char *)&currLevel.texels[0];
    prevLevelMem = (unsigned char *)&prevLevel.texels[0];

    float wDecimal, wNorm, wWeight[3];
    int wSupport;
    float hDecimal, hNorm, hWeight[3];
    int hSupport;

    float result[4];
    float input[4];

    // conditional differentiates no rounding case from round down case
    if (prevLevel.width & 1) {
      wSupport = 3;
      wDecimal = 1.0f / (float)currLevel.width;
    } else {
      wSupport = 2;
      wDecimal = 0.0f;
    }

    // conditional differentiates no rounding case from round down case
    if (prevLevel.height & 1) {
      hSupport = 3;
      hDecimal = 1.0f / (float)currLevel.height;
    } else {
      hSupport = 2;
      hDecimal = 0.0f;
    }

    wNorm = 1.0f / (2.0f + wDecimal);
    hNorm = 1.0f / (2.0f + hDecimal);

    // case 1: reduction only in horizontal size (vertical size is 1)
    if (currLevel.height == prevLevel.height) {
      //assert (currLevel.height == 1);

      for (int i = 0; i < currLevel.width; i++) {
        wWeight[0] = wNorm * (1.0f - wDecimal * i);
        wWeight[1] = wNorm * 1.0f;
        wWeight[2] = wNorm * wDecimal * (i + 1);

        result[0] = result[1] = result[2] = result[3] = 0.0f;

        for (int ii = 0; ii < wSupport; ii++) {
          uint8_to_float(input, prevLevelMem + 4 * (2 * i + ii));
          result[0] += wWeight[ii] * input[0];
          result[1] += wWeight[ii] * input[1];
          result[2] += wWeight[ii] * input[2];
          result[3] += wWeight[ii] * input[3];
        }

        // convert back to format of the texture
        float_to_uint8(currLevelMem + (4 * i), result);
      }

      // case 2: reduction only in vertical size (horizontal size is 1)
    } else if (currLevel.width == prevLevel.width) {
      //assert (currLevel.width == 1);

      for (int j = 0; j < currLevel.height; j++) {
        hWeight[0] = hNorm * (1.0f - hDecimal * j);
        hWeight[1] = hNorm;
        hWeight[2] = hNorm * hDecimal * (j + 1);

        result[0] = result[1] = result[2] = result[3] = 0.0f;
        for (int jj = 0; jj < hSupport; jj++) {
          uint8_to_float(input, prevLevelMem + prevLevelPitch * (2 * j + jj));
          result[0] += hWeight[jj] * input[0];
          result[1] += hWeight[jj] * input[1];
          result[2] += hWeight[jj] * input[2];
          result[3] += hWeight[jj] * input[3];
        }

        // convert back to format of the texture
        float_to_uint8(currLevelMem + (currLevelPitch * j), result);
      }

      // case 3: reduction in both horizontal and vertical size
    } else {

      for (int j = 0; j < currLevel.height; j++) {
        hWeight[0] = hNorm * (1.0f - hDecimal * j);
        hWeight[1] = hNorm;
        hWeight[2] = hNorm * hDecimal * (j + 1);

        for (int i = 0; i < currLevel.width; i++) {
          wWeight[0] = wNorm * (1.0f - wDecimal * i);
          wWeight[1] = wNorm * 1.0f;
          wWeight[2] = wNorm * wDecimal * (i + 1);

          result[0] = result[1] = result[2] = result[3] = 0.0f;

          // convolve source image with a trapezoidal filter.
          // in the case of no rounding this is just a box filter of width 2.
          // in the general case, the support region is 3x3.
          for (int jj = 0; jj < hSupport; jj++)
            for (int ii = 0; ii < wSupport; ii++) {
              float weight = hWeight[jj] * wWeight[ii];
              uint8_to_float(input, prevLevelMem +
                                        prevLevelPitch * (2 * j + jj) +
                                        4 * (2 * i + ii));
              result[0] += weight * input[0];
              result[1] += weight * input[1];
              result[2] += weight * input[2];
              result[3] += weight * input[3];
            }

          // convert back to format of the texture
          float_to_uint8(currLevelMem + currLevelPitch * j + 4 * i, result);
        }
      }
    }
  }
}

}
