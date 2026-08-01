#ifndef WAVEFORMWIDGET_H
#define WAVEFORMWIDGET_H

#include <QWidget>
#include <QAudioDecoder>

class Sound;

class WaveformWidget : public QWidget
{
    Q_OBJECT
public:

    struct Range{
        double start = 0;
        double end = 0;

        bool contains(double t_value) const
        {
            return t_value >= start && t_value <= end;
        }
    };

    explicit WaveformWidget(QWidget *parent = nullptr);
    virtual ~WaveformWidget();

    void setBackgroundColor(const QColor &);
    void setWaveformColor(const QColor &);
    void setCenterLineColor(const QColor &);
    void setPlayheadColor(const QColor &);
    void setSelectionColor(const QColor &);
    void setShowWaveform(bool);   // suppress the raw min/max render, e.g. when a subclass draws its own visual
    double xToTime(int x) const;
    double timeToX(double time) const;
    void setPlayhead(double time);
    double playheadTime() const{return m_playheadTime;}
    void clearPlayhead();
    void setSelectionRange(double start, double end);
    void clearSelection();

    Range visibleRange(){return m_visibleRange;}

signals:
    // Emitted when the user pans/zooms the waveform directly, so a host can keep
    // other time-aligned views in sync. Carries the new visible time range.
    void visibleRangeChanged(double start, double end);

public slots:
    void loadAudio(const QString &);
    void clearAudio();
    void frameAll();
    void frameTime(double start, double end);
    // Time axis without decoded audio (e.g. a VirtualDJ-live-captured sequence with
    // no local file): lets the view frame/scroll/zoom over [0, seconds] using the
    // same coordinate math as real audio, just without a waveform image to draw.
    void setDuration(double seconds);

private slots:
    void bufferReady();
    void finished();

protected:
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void resizeEvent(QResizeEvent *) override;
    virtual void wheelEvent(QWheelEvent *) override;
    virtual void mousePressEvent(QMouseEvent *) override;
    virtual void mouseMoveEvent(QMouseEvent *) override;
    virtual void mouseReleaseEvent(QMouseEvent *) override;

private:


    double m_targetHZoomRatio;
    double m_targetHOffset;

    void RenderWaveform(QPainter *, double v_zoom_ratio);

    QAudioDecoder *m_decoder;

    QColor m_waveformColor;
    QColor m_backgroundColor;
    QColor m_centerLineColor;
    QColor m_playheadColor;
    QColor m_selectionColor;
    QImage m_cachedWaveform;

    Sound *m_sound = nullptr;

    QPoint m_lastPosition;
    bool m_isDragging = false;
    bool m_isRenderable = false;


    double m_playheadTime = 0;
    Range m_selectionRange;
    Range m_visibleRange;
    double m_hOffset = 0;
    double m_hZoomRatio = -1;
    // Samples-per-second-equivalent used for all pixel<->time math. Set to the real
    // decoded rate once audio loads; otherwise stays at 1.0, which makes m_hZoomRatio
    // plain seconds-per-pixel and m_hOffset plain seconds - a valid, self-contained
    // time axis with no audio behind it at all.
    double m_referenceRate = 1.0;
    double m_referenceDuration = 0.0;   // set via setDuration() when there's no audio to derive one from
    bool m_showPlayhead = false;
    bool m_showSelection = false;
    bool m_redrawWaveform = true;
    bool m_showWaveform = true;

    int16_t *m_displayMins;    // An array of length m_width.
    int16_t *m_displayMaxes;   // An array of length m_width.

};

#endif // WAVEFORMWIDGET_H
