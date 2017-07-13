#ifndef COLOUR_HH
#define COLOUR_HH

#include <algorithm> // min, max
#include <math.h>
#include <vector>

using namespace std;

// from http://axonflux.com/handy-rgb-to-hsl-and-rgb-to-hsv-color-model-c

/**
 * Converts an RGB color value to HSL. Conversion formula
 * adapted from http://en.wikipedia.org/wiki/HSL_color_space.
 * Assumes r, g, and b are contained in the set [0, 255] and
 * returns h, s, and l in the set [0, 1].
 *
 * @param   Number  r       The red color value
 * @param   Number  g       The green color value
 * @param   Number  b       The blue color value
 * @return  Array           The HSL representation
function rgbToHsl(r, g, b){
    r /= 255, g /= 255, b /= 255;
    var max = Math.max(r, g, b), min = Math.min(r, g, b);
    var h, s, l = (max + min) / 2;

    if(max == min){
        h = s = 0; // achromatic
    }else{
        var d = max - min;
        s = l > 0.5 ? d / (2 - max - min) : d / (max + min);
        switch(max){
            case r: h = (g - b) / d + (g < b ? 6 : 0); break;
            case g: h = (b - r) / d + 2; break;
            case b: h = (r - g) / d + 4; break;
        }
        h /= 6;
    }

    return [h, s, l];
}
 */

vector<double> rgb2hsl(const int r, const int g, const int b){
  const int max_i = max({r,g,b}), min_i = min({r,g,b});
  double r_d = r/255.0, g_d = g/255.0, b_d = b/255.0;
  const double max_d = max({r_d,g_d,b_d}), min_d = min({r_d,g_d,b_d});
  double h,s,l = (max_d+min_d)/2.0;
  if(max_i==min_i){ // shades of grey
    h = 0, s = 0;
  }
  else{
    const double d = max_d-min_d;
    s = (l > 0.5) ? (d / (2 - max_d - min_d)) : (d / (max_d + min_d));
    if(r==max_i){
      h = (g_d - b_d) / d + (g < b ? 6 : 0);
    }
    else if(g==max_i){
      h = (b_d - r_d) / d + 2;
    }
    else{
      h = (r_d - g_d) / d + 4;
    }
    h /= 6.0;
  }
  return vector<double>({h,s,l});
}

/**
 * Converts an HSL color value to RGB. Conversion formula
 * adapted from http://en.wikipedia.org/wiki/HSL_color_space.
 * Assumes h, s, and l are contained in the set [0, 1] and
 * returns r, g, and b in the set [0, 255].
 *
 * @param   Number  h       The hue
 * @param   Number  s       The saturation
 * @param   Number  l       The lightness
 * @return  Array           The RGB representation
function hslToRgb(h, s, l){
    var r, g, b;

    if(s == 0){
        r = g = b = l; // achromatic
    }else{
        function hue2rgb(p, q, t){
            if(t < 0) t += 1;
            if(t > 1) t -= 1;
            if(t < 1/6) return p + (q - p) * 6 * t;
            if(t < 1/2) return q;
            if(t < 2/3) return p + (q - p) * (2/3 - t) * 6;
            return p;
        }

        var q = l < 0.5 ? l * (1 + s) : l + s - l * s;
        var p = 2 * l - q;
        r = hue2rgb(p, q, h + 1/3);
        g = hue2rgb(p, q, h);
        b = hue2rgb(p, q, h - 1/3);
    }

    return [r * 255, g * 255, b * 255];
}
 */

/**
 * Converts an RGB color value to HSV. Conversion formula
 * adapted from http://en.wikipedia.org/wiki/HSV_color_space.
 * Assumes r, g, and b are contained in the set [0, 255] and
 * returns h, s, and v in the set [0, 1].
 *
 * @param   Number  r       The red color value
 * @param   Number  g       The green color value
 * @param   Number  b       The blue color value
 * @return  Array           The HSV representation
function rgbToHsv(r, g, b){
    r = r/255, g = g/255, b = b/255;
    var max = Math.max(r, g, b), min = Math.min(r, g, b);
    var h, s, v = max;

    var d = max - min;
    s = max == 0 ? 0 : d / max;

    if(max == min){
        h = 0; // achromatic
    }else{
        switch(max){
            case r: h = (g - b) / d + (g < b ? 6 : 0); break;
            case g: h = (b - r) / d + 2; break;
            case b: h = (r - g) / d + 4; break;
        }
        h /= 6;
    }

    return [h, s, v];
}
 */

// [0..255] --> [0,1]
// aka HSB
vector<double> rgb2hsv(const int r, const int g, const int b){
  const int max_i = max({r,g,b}), min_i = min({r,g,b});
  double r_d = r/255.0, g_d = g/255.0, b_d = b/255.0;
  const double max_d = max({r_d,g_d,b_d}), min_d = min({r_d,g_d,b_d});
  double h,s,v = max_d;
  const double d = max_d-min_d;
  s = (max_i == 0) ? 0 : (d / max_d);
  if(max_i==min_i){ // shades of grey
    h = 0;
  }
  else{
    if(r==max_i){
      h = (g_d - b_d) / d + (g < b ? 6 : 0);
    }
    else if(g==max_i){
      h = (b_d - r_d) / d + 2;
    }
    else{
      h = (r_d - g_d) / d + 4;
    }
    h /= 6.0;
  }
  return vector<double>({h,s,v});
}

/**
 * Converts an HSV color value to RGB. Conversion formula
 * adapted from http://en.wikipedia.org/wiki/HSV_color_space.
 * Assumes h, s, and v are contained in the set [0, 1] and
 * returns r, g, and b in the set [0, 255].
 *
 * @param   Number  h       The hue
 * @param   Number  s       The saturation
 * @param   Number  v       The value
 * @return  Array           The RGB representation
function hsvToRgb(h, s, v){
    var r, g, b;

    var i = Math.floor(h * 6);
    var f = h * 6 - i;
    var p = v * (1 - s);
    var q = v * (1 - f * s);
    var t = v * (1 - (1 - f) * s);

    switch(i % 6){
        case 0: r = v, g = t, b = p; break;
        case 1: r = q, g = v, b = p; break;
        case 2: r = p, g = v, b = t; break;
        case 3: r = p, g = q, b = v; break;
        case 4: r = t, g = p, b = v; break;
        case 5: r = v, g = p, b = q; break;
    }

    return [r * 255, g * 255, b * 255];
}
 */

// [0,1] --> [0..255]
// aka HSB
vector<int> hsv2rgb(const double h, const double s, const double v){
  const int i = floor(6*h);
  const double f = h * 6 - i;
  const double p = v * (1 - s);
  const double q = v * (1 - f * s);
  const double t = v * (1 - (1 - f) * s);
  double r, g, b;
  switch(i){
    case 0: r=v, g=t, b=p; break;
    case 1: r=q, g=v, b=p; break;
    case 2: r=p, g=v, b=t; break;
    case 3: r=p, g=q, b=v; break;
    case 4: r=t, g=p, b=v; break;
    case 5: r=v, g=p, b=q; break;
  }
  const int r_d = r*255, g_d = g*255, b_d = b*255;
  return vector<int>({r_d,g_d,b_d});
}

#endif
