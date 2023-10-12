#ifndef GILCONCURRENT_H
#define GILCONCURRENT_H


#include <QtConcurrent>
#include <boost/gil.hpp>
#include "data/types.h"

namespace photon {

template <typename View1>
QList<View1> subdivideViews(const View1 &view, int overlap = 0)
{
    QList<View1> views;

    int area = view.width() * view.height();

    if(area < 10000 || std::thread::hardware_concurrency() < 2)
    {
        views.append(view);
        return views;
    }

    int cellDivision = static_cast<int>(std::thread::hardware_concurrency()-2);

    if(cellDivision < 1)
        cellDivision = 1;

    int h = static_cast<int>(floor(view.height()/cellDivision));
    int remaining = view.height();

    for(int y = 0; y < cellDivision-1; ++y)
    {
        views.append(subimage_view(view,0,std::max(0,y * h - overlap),view.width(), std::min(h+overlap, static_cast<int>(view.height()))));
        remaining -= h;
    }
    views.append(subimage_view(view,0,view.height() - remaining - overlap,view.width(), remaining));

    /*
    int cellDivision = 4;
    int w = static_cast<int>(view.width()/cellDivision);
    int h = static_cast<int>(view.height()/cellDivision);

    for(int y = 0; y < cellDivision; ++y)
    {
        for(int x = 0; x < cellDivision; ++x)
        {
            views.append(subimage_view(view,x * w,y * h,w, h));
        }
    }
    */

    return views;
}


template <typename View1> BOOST_FORCEINLINE
void concurrentViewTransform(const View1 &view, const bounds_i &bounds, std::function<void(const View1 &view, const bounds_i &bounds)> func)
{
    struct Container
    {
        Container(const View1 &view, const bounds_i &bounds):view(view),bounds(bounds){}
        View1 view;
        bounds_i bounds;
    };


    QList<Container> containers;
    QList<View1> views = subdivideViews<View1>(view);

    int y = bounds.top();
    for(auto viewIt = views.cbegin(); viewIt != views.cend(); ++viewIt)
    {
        containers.append(Container{*viewIt, bounds_i{bounds.x1, y, bounds.x2, y + bounds.y2}});
        y += (*viewIt).height();
    }

    QtConcurrent::blockingMap(containers, [&func](const Container &container){func(container.view, container.bounds);});
}

template <typename View1>
void concurrentDemultiply(const View1 &view)
{
    QList<View1> views = subdivideViews<View1>(view);

    QtConcurrent::blockingMap(views, [](View1 &v)
    {
        demultiplyView(v);
    });
}

template <typename View1>
void concurrentPremultiply(const View1 &view)
{
    QList<View1> views = subdivideViews<View1>(view);

    QtConcurrent::blockingMap(views, [](View1 &v)
    {
        premultiplyView(v);
    });
}

template <typename View1> BOOST_FORCEINLINE
void concurrentViewTransform(const View1 &view, std::function<void(const View1 &view)> func)
{
    struct Container
    {
        Container(const View1 &view):view(view){}
        View1 view;
        bounds_i bounds;
    };

    QList<Container> containers;

    QList<View1> views = subdivideViews<View1>(view);

    for(auto viewIt = views.cbegin(); viewIt != views.cend(); ++viewIt)
    {
        containers.append(Container{*viewIt});
    }

    QtConcurrent::blockingMap(containers, [&func](const Container &container){func(container.view);});
}

template <typename View1, typename Mask1> BOOST_FORCEINLINE
void concurrentViewTransform(const View1 &view, const Mask1 &mask, std::function<void(const View1 &view, const Mask1 &mask)> func)
{
    struct Container
    {
        Container(const View1 &view, const Mask1 &maskView):view(view),maskView(maskView){}
        View1 view;
        Mask1 maskView;
        bounds_i bounds;
    };

    QList<Container> containers;

    QList<View1> views = subdivideViews<View1>(view);
    QList<Mask1> masks = subdivideViews<Mask1>(mask);

    auto maskIt = masks.cbegin();
    for(auto viewIt = views.cbegin(); viewIt != views.cend(); ++viewIt, ++maskIt)
    {
        containers.append(Container{*viewIt, *maskIt});
    }

    QtConcurrent::blockingMap(containers, [&func](const Container &container){func(container.view, container.maskView);});
}

template <typename View1, typename View2, typename Mask1> BOOST_FORCEINLINE
void concurrentViewTransform(const View1 &view, const View2 &view2, const Mask1 &mask, std::function<void(const View1 &view, const View2 &view2, const Mask1 &mask)> func)
{
    struct Container
    {
        Container(const View1 &view, const View2 &view2, const Mask1 &maskView):view(view),view2(view2),maskView(maskView){}
        View1 view;
        View2 view2;
        Mask1 maskView;

    };

    QList<Container> containers;

    QList<View1> views = subdivideViews<View1>(view);
    QList<View2> views2 = subdivideViews<View2>(view2);
    QList<Mask1> masks = subdivideViews<Mask1>(mask);

    auto maskIt = masks.cbegin();
    auto view2It = views2.cbegin();
    for(auto viewIt = views.cbegin(); viewIt != views.cend(); ++viewIt, ++view2It, ++maskIt)
    {
        containers.append(Container{*viewIt, *view2It, *maskIt});
    }

    QtConcurrent::blockingMap(containers, [&func](const Container &container){func(container.view, container.view2, container.maskView);});
}

template <typename View1, typename View2>
void concurrentConvolveSingleChannel3X3(View1 src, View2 dst, float kernel[][3])
{

    struct Container
    {
        Container(const View1 &v, const View2 &dst):view(v),destination(dst){}
        View1 view;
        View2 destination;

    };

    QList<Container> containers;

    QList<View1> views = subdivideViews<View1>(src,1);
    QList<View2> destinations = subdivideViews<View2>(dst,1);

    auto dstIt = destinations.cbegin();
    for(auto viewIt = views.cbegin(); viewIt != views.cend(); ++viewIt, ++dstIt)
    {
        containers.append(Container{*viewIt, *dstIt});
    }

    QtConcurrent::blockingMap(containers, [kernel](const Container &container){

        typename View1::xy_locator loc=container.view.xy_at(0,0);
        for (std::ptrdiff_t y=0; y<container.view.height(); ++y) {
            typename View2::x_iterator dIt=container.destination.row_begin(y);
            for (std::ptrdiff_t x=0; x<container.view.width(); ++x, ++loc.x())
            {
                float sum = 0.0;
                for(int k = -1; k <= 1;k++){
                    for(int j = -1; j <=1; j++){
                        if(j + x >= 0 && k + y >= 0 && j + x < container.view.width() && k + y < container.view.height())
                            sum = sum + kernel[j+1][k+1]* loc(j, k);

                    }
                }
                dIt[x] = sum;
            }
            loc.x()-=container.view.width(); ++loc.y();
        }

    });
}

template <typename View1, typename View2>
void concurrentConvolveThreeChannel3X3(View1 src, View2 dst, float kernel[][3])
{

    struct Container
    {
        Container(const View1 &v, const View2 &dst):view(v),destination(dst){}
        View1 view;
        View2 destination;

    };

    QList<Container> containers;

    QList<View1> views = subdivideViews<View1>(src,1);
    QList<View2> destinations = subdivideViews<View2>(dst,1);

    auto dstIt = destinations.cbegin();
    for(auto viewIt = views.cbegin(); viewIt != views.cend(); ++viewIt, ++dstIt)
    {
        containers.append(Container{*viewIt, *dstIt});
    }

    QtConcurrent::blockingMap(containers, [kernel](const Container &container){

        typename View1::xy_locator loc=container.view.xy_at(0,0);
        for (std::ptrdiff_t y=0; y<container.view.height(); ++y) {
            typename View2::x_iterator dIt=container.destination.row_begin(y);
            for (std::ptrdiff_t x=0; x<container.view.width(); ++x, ++loc.x())
            {
                float sum0 = 0.0;
                float sum1 = 0.0;
                float sum2 = 0.0;
                for(int k = -1; k <= 1;k++){
                    for(int j = -1; j <=1; j++){
                        if(j + x >= 0 && k + y >= 0 && j + x < container.view.width() && k + y < container.view.height())
                        {
                            sum0 = sum0 + kernel[j+1][k+1]* loc(j, k)[0];
                            sum1 = sum1 + kernel[j+1][k+1]* loc(j, k)[1];
                            sum2 = sum2 + kernel[j+1][k+1]* loc(j, k)[2];
                        }
                    }
                }
                dIt[x][0] = sum0;
                dIt[x][1] = sum1;
                dIt[x][2] = sum2;
            }
            loc.x()-=container.view.width(); ++loc.y();
        }

    });
}



/*
template <typename View1>
void concurrentViewTransform(const View1 &inView, const View1 &outView, std::function<void(const View1 &inView, const View1 &outView)> func)
{

    struct Container
    {
        Container(const View1 &inView, const View1 &outView):inView(inView),outView(outView){}
        View1 inView;
        View1 outView;

    };

    int cellDivision = 4;
    int w = static_cast<int>(inView.width()/cellDivision);
    int h = static_cast<int>(inView.height()/cellDivision);
    QList<Container> views;

    for(int y = 0; y < cellDivision; ++y)
    {
        for(int x = 0; x < cellDivision; ++x)
        {
            //qDebug() << x * w << "  " << y * w;
            views.append(Container{subimage_view(inView,x * w,y * h,w, h), subimage_view(outView,x * w,y * h,w, h)});
            //color_image_hist(, histogram, data.min, data.max, data.mid, data.valueMin, data.valueMax);
        }
    }


    QtConcurrent::blockingMap(views, [&func](const Container &container){func(container.inView, container.outView);});
}
*/

} // namespace deco


#endif // GILCONCURRENT_H
