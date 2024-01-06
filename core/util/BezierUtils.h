#ifndef BEZIERUTILS_H
#define BEZIERUTILS_H


#include "qmath.h"
#include "data/types.h"
#include "mathutils.h"

namespace photon {

namespace pomax {

  const double pi = M_PI;
  const double tau = 2 * pi;
  const double quart = pi / 2;
  // float precision significant decimal
  const double epsilon = 0.000001;
  // extremas used in bbox calculation and similar algorithms
  const int nMax = std::numeric_limits<int>::max();
  const int nMin = std::numeric_limits<int>::lowest();
  // a zero coordinate, which is surprisingly useful
  const point_d ZERO;

  const double Tvalues[] = {-0.0640568928626056260850430826247450385909,
    0.0640568928626056260850430826247450385909,
    -0.1911188674736163091586398207570696318404,
    0.1911188674736163091586398207570696318404,
    -0.3150426796961633743867932913198102407864,
    0.3150426796961633743867932913198102407864,
    -0.4337935076260451384870842319133497124524,
    0.4337935076260451384870842319133497124524,
    -0.5454214713888395356583756172183723700107,
    0.5454214713888395356583756172183723700107,
    -0.6480936519369755692524957869107476266696,
    0.6480936519369755692524957869107476266696,
    -0.7401241915785543642438281030999784255232,
    0.7401241915785543642438281030999784255232,
    -0.8200019859739029219539498726697452080761,
    0.8200019859739029219539498726697452080761,
    -0.8864155270044010342131543419821967550873,
    0.8864155270044010342131543419821967550873,
    -0.9382745520027327585236490017087214496548,
    0.9382745520027327585236490017087214496548,
    -0.9747285559713094981983919930081690617411,
    0.9747285559713094981983919930081690617411,
    -0.9951872199970213601799974097007368118745,
    0.9951872199970213601799974097007368118745,
  };

  const double Cvalues[] = {
      0.1279381953467521569740561652246953718517,
      0.1279381953467521569740561652246953718517,
      0.1258374563468282961213753825111836887264,
      0.1258374563468282961213753825111836887264,
      0.121670472927803391204463153476262425607,
      0.121670472927803391204463153476262425607,
      0.1155056680537256013533444839067835598622,
      0.1155056680537256013533444839067835598622,
      0.1074442701159656347825773424466062227946,
      0.1074442701159656347825773424466062227946,
      0.0976186521041138882698806644642471544279,
      0.0976186521041138882698806644642471544279,
      0.086190161531953275917185202983742667185,
      0.086190161531953275917185202983742667185,
      0.0733464814110803057340336152531165181193,
      0.0733464814110803057340336152531165181193,
      0.0592985849154367807463677585001085845412,
      0.0592985849154367807463677585001085845412,
      0.0442774388174198061686027482113382288593,
      0.0442774388174198061686027482113382288593,
      0.0285313886289336631813078159518782864491,
      0.0285313886289336631813078159518782864491,
      0.0123412297999871995468056670700372915759,
      0.0123412297999871995468056670700372915759,
  };

using PointVector = std::vector<point_d>;


namespace utils {
    static void align(PointVector &points, const line_d &line)
    {
        double tx = line.p1().x;
        double ty = line.p1().y;
        double a = -atan2(line.p2().y - ty, line.p2().x - tx);
        std::for_each(points.begin(), points.end(), [tx, ty, a](point_d &pt)
        {
              double x = pt.x;
              pt.x = (pt.x - tx) * cos(a) - (pt.y - ty) * sin(a);
              pt.y = (x - tx) * sin(a) + (pt.y - ty) * cos(a);
        });
    }

    static PointVector aligned(PointVector points, const line_d &line)
    {
        align(points, line);
        return points;
    }

    static std::vector<PointVector> derive(const PointVector t_points)
    {
        std::vector<PointVector> derivedPoints;
        PointVector p = t_points;
        int d = p.size();
        int c = d - 1;
        for(; d > 1; --d, --c)
        {
            PointVector list;
            for(int j = 0; j < c; ++j)
            {
                point_d pt = point_d{ c * (p[j + 1].x - p[j].x),
                        c * (p[j + 1].y - p[j].y)};
                list.push_back(pt);
            }
            derivedPoints.push_back(list);
            p = list;
        }


        return derivedPoints;
    }

    static double angle(const point_d &o, const point_d &v1, const point_d &v2)
    {
        double dx1 = v1.x - o.x,
          dy1 = v1.y - o.y,
          dx2 = v2.x - o.x,
          dy2 = v2.y - o.y,
          cross = dx1 * dy2 - dy1 * dx2,
          dot = dx1 * dx2 + dy1 * dy2;
        return atan2(cross, dot);
    }

    static double map(double v, double ds, double de, double ts, double te)
    {
        double d1 = de - ds,
          d2 = te - ts,
          v2 = v - ds,
          r = v2 / d1;
        return ts + d2 * r;
    }

    static point_d lerp(double r, const point_d &v1, const point_d &v2)
    {
        return point_d{v1.x + r * (v2.x - v1.x), v1.y + r * (v2.y - v1.y)};
    }

    class computed_pt
    {
    public:
        computed_pt(const point_d &pt):point(pt){}
        computed_pt(const point_d &t_pt, double t_t):point(t_pt),t(t_t){}

        operator point_d() const{return point;}
        point_d point;
        double t;
    };

    static computed_pt compute(double t_t, const PointVector &t_points)
    {
        if (t_t <= 0) {
          return computed_pt(t_points.front(), 0);
        }

        const uint order = t_points.size() - 1;

        if (t_t >= 1) {
          computed_pt(t_points.back(), 1);
        }

        const double mt = 1 - t_t;

        // constant?
        if (order == 0) {
          return computed_pt(t_points.front(), 0);
        }

        // linear?
        if (order == 1) {
          return computed_pt{point_d{
            mt * t_points[0].x + t_t * t_points[1].x,
            mt * t_points[0].y + t_t * t_points[1].y},
            t_t
          };
        }

        // quadratic/cubic curve?
        //if (order < 4) {
          double mt2 = mt * mt,
            t2 = t_t * t_t,
            a = 0,
            b = 0,
            c = 0,
            d = 0;
          if (order == 2) {
            a = mt2;
            b = mt * t_t * 2;
            c = t2;

            return computed_pt{point_d{
              a * t_points[0].x + b * t_points[1].x + c * t_points[2].x,
              a * t_points[0].y + b * t_points[1].y + c * t_points[2].y},
              t_t
            };

          } else if (order == 3) {
            a = mt2 * mt;
            b = mt2 * t_t * 3;
            c = mt * t2 * 3;
            d = t_t * t2;
          }
          return computed_pt{point_d{
            a * t_points[0].x + b * t_points[1].x + c * t_points[2].x + d * t_points[3].x,
            a * t_points[0].y + b * t_points[1].y + c * t_points[2].y + d * t_points[3].y},
            t_t
          };
       // }
          /*
          const dCpts = JSON.parse(JSON.stringify(points));
          while (dCpts.length > 1) {
            for (let i = 0; i < dCpts.length - 1; i++) {
              dCpts[i] = {
                x: dCpts[i].x + (dCpts[i + 1].x - dCpts[i].x) * t,
                y: dCpts[i].y + (dCpts[i + 1].y - dCpts[i].y) * t,
              };
              if (typeof dCpts[i].z !== "undefined") {
                dCpts[i] = dCpts[i].z + (dCpts[i + 1].z - dCpts[i].z) * t;
              }
            }
            dCpts.splice(dCpts.length - 1, 1);
          }
          dCpts[0].t = t;
          return dCpts[0];
          */
    }

    static void addRoot(std::vector<double> &roots, double t) {
        if(0.0 <= t && t <= 1.0)
        {
            roots.push_back(t);
        }
    }

    static void droots(const std::vector<double> &p, std::vector<double> &roots)
    {
        if (p.size() == 3) {
          double a = p[0],
            b = p[1],
            c = p[2],
            d = a - 2 * b + c;
          if (d != 0) {
            double m1 = -sqrt(b * b - a * c),
              m2 = -a + b,
              v1 = -(m1 + m2) / d,
              v2 = -(-m1 + m2) / d;
            addRoot(roots, v1);
            addRoot(roots, v2);
          } else if (b != c && d == 0) {
              addRoot(roots, (2 * b - c) / (2 * (b - c)));
          }
        }

        // linear roots are even easier
        if (p.size() == 2) {
          double a = p[0],
            b = p[1];
          if (a != b) {
              addRoot(roots, a / (a - b));
          }
        }
    }

    static void drootsX(const std::vector<point_d> &p, std::vector<double> &roots)
    {
        if (p.size() == 3) {
          double a = p[0].x,
            b = p[1].x,
            c = p[2].x,
            d = a - 2 * b + c;
          if (d != 0) {
            double m1 = -sqrt(b * b - a * c),
              m2 = -a + b,
              v1 = -(m1 + m2) / d,
              v2 = -(-m1 + m2) / d;
            addRoot(roots, v1);
            addRoot(roots, v2);
          } else if (b != c && d == 0) {
              addRoot(roots, (2 * b - c) / (2 * (b - c)));
          }
        }

        // linear roots are even easier
        if (p.size() == 2) {
          double a = p[0].x,
            b = p[1].x;
          if (a != b) {
              addRoot(roots, a / (a - b));
          }
        }
    }

    static point_d lli8(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4)
    {
        double nx =
            (x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4),
          ny = (x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4),
          d = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
        if(d == 0)
            return point_d{};
        return point_d{nx / d, ny / d};
    }

    static point_d lli4(const point_d &p1, const point_d & p2, const point_d & p3, const point_d & p4)
    {
        return lli8(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y);
    }

    static bool overlaps(const bounds_d &b1, const bounds_d &b2)
    {
        double l = (b1.x1 + b1.x2)/2.0;
        double t = (b2.x1 + b2.x2)/2.0;
        double d = (b1.width() + b2.width()) / 2.0;

        if(abs(l - t) >= d)
            return false;

        l = (b1.y1 + b1.y2)/2.0;
        t = (b2.y2 + b2.y2)/2.0;
        d = (b1.height() + b2.height()) / 2.0;

        if(abs(l - t) >= d)
            return false;
        return true;
    }

    static void drootsY(const std::vector<point_d> &p, std::vector<double> &roots)
    {
        if (p.size() == 3) {
          double a = p[0].y,
            b = p[1].y,
            c = p[2].y,
            d = a - 2 * b + c;
          if (d != 0) {
            double m1 = -sqrt(b * b - a * c),
              m2 = -a + b,
              v1 = -(m1 + m2) / d,
              v2 = -(-m1 + m2) / d;
            addRoot(roots, v1);
            addRoot(roots, v2);
          } else if (b != c && d == 0) {
              addRoot(roots, (2 * b - c) / (2 * (b - c)));
          }
        }

        // linear roots are even easier
        if (p.size() == 2) {
          double a = p[0].y,
            b = p[1].y;
          if (a != b) {
              addRoot(roots, a / (a - b));
          }
        }
    }



}










// A helper function to filter for values in the [0,1] interval:
static void addRoot(std::vector<double> &roots, double t) {
    if(0.0 <= t && t <= 1.0)
    {
        roots.push_back(t);
    }
}

static std::vector<PointVector> derive(const PointVector t_points)
{
    std::vector<PointVector> derivedPoints;
    PointVector p = t_points;
    int d = p.size();
    int c = d - 1;
    for(; d > 1; --d, --c)
    {
        PointVector list;
        for(int j = 0; j < c; ++j)
        {
            point_d pt = point_d{ c * (p[j + 1].x - p[j].x), c * (p[j + 1].y - p[j].y)};
            list.push_back(pt);
        }
        derivedPoints.push_back(list);
        p = list;
    }


    return derivedPoints;
}

static void droots(const std::vector<double> &p, std::vector<double> &roots)
{
    if (p.size() == 3) {
      double a = p[0],
        b = p[1],
        c = p[2],
        d = a - 2 * b + c;
      if (d != 0) {
        double m1 = -sqrt(b * b - a * c),
          m2 = -a + b,
          v1 = -(m1 + m2) / d,
          v2 = -(-m1 + m2) / d;
        addRoot(roots, v1);
        addRoot(roots, v2);
      } else if (b != c && d == 0) {
          addRoot(roots, (2 * b - c) / (2 * (b - c)));
      }
    }

    // linear roots are even easier
    if (p.size() == 2) {
      double a = p[0],
        b = p[1];
      if (a != b) {
          addRoot(roots, a / (a - b));
      }
    }
}

static void fastRoots(const PointVector &p, std::vector<double> &roots)
{
    auto derivedPoints = derive(p);
    for(auto derived : derivedPoints)
    {
        std::vector<double> xPoints;
        std::vector<double> yPoints;
        for(point_d pt : derived)
        {
            xPoints.push_back(pt.x);
            yPoints.push_back(pt.y);
        }
        droots(xPoints, roots);
        droots(yPoints, roots);
    }

}

static void alignForRoots(PointVector &points, const line_d &line)
{
    double tx = line.p1().x;
    double ty = line.p1().y;
    double a = -atan2(line.p2().y - ty, line.p2().x - tx);
    std::for_each(points.begin(), points.end(), [tx, ty, a](point_d &pt)
    {
          double x = pt.x;
          pt.x = (pt.x - tx) * cos(a) - (pt.y - ty) * sin(a);
          pt.y = (x - tx) * sin(a) + (pt.y - ty) * cos(a);
    });
}

// Now then: given cubic coordinates {pa, pb, pc, pd} find all roots.
static void roots(PointVector points, std::vector<double> &roots, line_d line = line_d(point_d{}, point_d{1,0}))
{
    if(points.size() < 4)
        return;
    alignForRoots(points, line);

    double pa = points[0].y;
    double pb = points[1].y;
    double pc = points[2].y;
    double pd = points[3].y;

    double a = (3.0*pa - 6.0*pb + 3.0*pc);
    double b = (-3.0*pa + 3.0*pb);
    double c = pa;
    double d = (-pa + 3.0*pb - 3.0*pc + pd);

  // do a check to see whether we even need cubic solving:
  if (abs(d - 0.0) < .00001) {
    // this is not a cubic curve.
    if (abs(a - 0.0) < .00001) {
      // in fact, this is not a quadratic curve either.
      if (abs(b - 0.0) < .00001) {
        // in fact in fact, there are no solutions.
        return;
      }
      // linear solution
      addRoot(roots, -c / b);
      return;
    }
    // quadratic solution
    double q = sqrt(b*b - 4.0*a*c);
    double a2 = 2.0*a;

    addRoot(roots, (q-b)/a2);
    addRoot(roots, (-b-q)/a2);
    return;
  }

  // at this point, we know we need a cubic solution.

  a /= d;
  b /= d;
  c /= d;

  double p = (3.0*b - a*a)/3.0;
    double p3 = p/3.0;
    double q = (2.0*a*a*a - 9.0*a*b + 27.0*c)/27.0;
    double q2 = q/2.0;
    double discriminant = q2*q2 + p3*p3*p3;

  // and some variables we're going to use later on:
  double u1, v1;


  // three possible real roots:
  if (discriminant < 0.0) {
    double mp3  = -p/3.0;
    double mp33 = mp3*mp3*mp3;
    double r    = sqrt( mp33 );
    double t    = -q / (2.0*r);
    double cosphi = t<-1.0 ? -1.0 : t>1.0 ? 1.0 : t;
    double phi  = acos(cosphi);
    double crtr = cbrt(r);
    double t1   = 2.0*crtr;

    addRoot(roots, t1 * cos(phi/3.0) - a/3.0);
    addRoot(roots, t1 * cos((phi+2*pi)/3.0) - a/3.0);
    addRoot(roots, t1 * cos((phi+4*pi)/3.0) - a/3.0);

    return;
  }

  // three real roots, but two of them are equal:
  if(discriminant == 0.0) {
    u1 = q2 < 0.0 ? cbrt(-q2) : -cbrt(q2);

    addRoot(roots, 2.0*u1 - a/3.0);
    addRoot(roots, -u1 - a/3.0);

    return;
  }

  // one real root, two complex roots
  double sd = sqrt(discriminant);
  u1 = cbrt(sd - q2);
  v1 = cbrt(sd + q2);
  addRoot(roots, u1 - v1 - a/3.0);
  return;
}

static double yForX(const point_d &a, const point_d &b, const point_d &c, const point_d &d, double x)
{
    PointVector points{4};
    points[0] = point_d{a.x, a.x - x};
    points[1] = point_d{b.x, b.x - x};
    points[2] = point_d{c.x, c.x - x};
    points[3] = point_d{d.x, d.x - x};

    std::vector<double> _roots;
    roots(points, _roots);
    if(_roots.size() > 0)
        return calc_bezier(a.y, b.y, c.y, d.y, _roots.front());

    return calc_bezier(a.y, b.y, c.y, d.y, 0);
}

} // pomax

} // deco

#endif // BEZIERUTILS_H
