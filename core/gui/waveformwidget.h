#ifndef WAVEFORMWIDGET_H
#define WAVEFORMWIDGET_H

#include <QWidget>
#include <QAudioDecoder>

class Sound;

class WaveformWidget : public QWidget
{
    Q_OBJECT
public:
    explicit WaveformWidget(QWidget *parent = nullptr);
    virtual ~WaveformWidget();

    void setBackgroundColor(const QColor &);
    void setWaveformColor(const QColor &);
    void setCenterLineColor(const QColor &);
    void setPlayheadColor(const QColor &);
    void setSelectionColor(const QColor &);
    double xToTime(int x) const;
    double timeToX(double time) const;
    void setPlayhead(double time);
    void clearPlayhead();
    void setSelectionRange(double start, double end);
    void clearSelection();

public slots:
    void loadAudio(const QString &);
    void clearAudio();
    void frameAll();
    void frameTime(double start, double end);

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

    struct Range{
        double start = 0;
        double end = 0;
    };

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
    double m_hOffset = 0;
    double m_hZoomRatio = -1;
    bool m_showPlayhead = false;
    bool m_showSelection = false;
    bool m_redrawWaveform = true;

    int16_t *m_displayMins;    // An array of length m_width.
    int16_t *m_displayMaxes;   // An array of length m_width.

};

#endif // WAVEFORMWIDGET_H
