#include <QTimer>
#include <QSettings>
#include <QPainter>
#include <QMenu>
#include <QPaintEvent>
#include <math.h>
#include <stdlib.h>
#include <qmmp/buffer.h>
#include <qmmp/output.h>
#include <qmmp/soundcore.h>
#include "fft.h"
#include "inlines.h"
#include "fwave.h"

FWave::FWave (QWidget *parent) : Visual (parent)
{
    m_intern_vis_data = 0;
    m_peaks = 0;
    m_x_scale = 0;
    m_running = false;
    m_rows = 0;
    m_cols = 0;

    for(int i=0; i<50; ++i)
    {
        m_starPoints << StarPoint();
    }

    setWindowTitle (tr("FWave"));
    setMinimumSize(2*300-30, 105);
    m_timer = new QTimer (this);
    connect(m_timer, SIGNAL (timeout()), this, SLOT (timeout()));

    m_starTimer = new QTimer (this);
    connect(m_starTimer, SIGNAL (timeout()), this, SLOT (starTimeout()));

    m_starAction = new QAction(tr("Star"), this);
    m_starAction->setCheckable(true);
    connect(m_starAction, SIGNAL(triggered(bool)), this, SLOT(changeStarState(bool)));

    m_peaks_falloff = 0.2;
    m_analyzer_falloff = 1.2;
    m_timer->setInterval(10);
    m_starTimer->setInterval(1000);
    m_cell_size = QSize(3, 2);

    clear();
    readSettings();
}

FWave::~FWave()
{
    if(m_peaks)
        delete [] m_peaks;
    if(m_intern_vis_data)
        delete [] m_intern_vis_data;
    if(m_x_scale)
        delete [] m_x_scale;
}

void FWave::start()
{
    m_running = true;
    if(isVisible())
    {
        m_timer->start();
        m_starTimer->start();
    }
}

void FWave::stop()
{
    m_running = false;
    m_timer->stop();
    m_starTimer->stop();
    clear();
}

void FWave::clear()
{
    m_rows = 0;
    m_cols = 0;
    update();
}

void FWave::timeout()
{
    if(takeData(m_left_buffer, m_right_buffer))
    {
        process();
        update();
    }
}

void FWave::starTimeout()
{
    for(int i=0; i<m_starPoints.count(); ++i)
    {
        StarPoint *sp = &m_starPoints[i];
        sp->m_alpha = rand()%255;
        sp->m_pt = QPoint(rand()%width(), rand()%height());
    }
}

void FWave::readSettings()
{
    QSettings settings(Qmmp::configFile(), QSettings::IniFormat);
    settings.beginGroup("FWave");
    m_colors = ColorWidget::readColorConfig(settings.value("colors").toString());
    m_starAction->setChecked(settings.value("show_star", false).toBool());
    m_starColor = ColorWidget::readSingleColorConfig(settings.value("star_color").toString());
}

void FWave::writeSettings()
{
    QSettings settings(Qmmp::configFile(), QSettings::IniFormat);
    settings.beginGroup("FWave");
    settings.setValue("colors", ColorWidget::writeColorConfig(m_colors));
    settings.setValue("show_star", m_starAction->isChecked());
    settings.setValue("star_color", ColorWidget::writeSingleColorConfig(m_starColor));
    settings.endGroup();
}

void FWave::changeColor()
{
    ColorWidget c;
    c.setColors(m_colors);
    if(c.exec())
    {
        m_colors = c.getColors();
    }
}

void FWave::changeStarState(bool state)
{
    m_starAction->setChecked(state);
    update();
}

void FWave::changeStarColor()
{
    ColorWidget c;
    c.setColors(QList<QColor>() << m_starColor);
    if(c.exec())
    {
        QList<QColor> colors(c.getColors());
        if(!colors.isEmpty())
        {
            m_starColor = colors.first();
            update();
        }
    }
}

void FWave::hideEvent (QHideEvent *)
{
    m_timer->stop();
    m_starTimer->stop();
}

void FWave::showEvent (QShowEvent *)
{
    if(m_running)
    {
        m_timer->start();
        m_starTimer->start();
    }
}

void FWave::paintEvent (QPaintEvent * e)
{
    QPainter painter (this);
    painter.fillRect(e->rect(), Qt::black);
    draw(&painter);
}

void FWave::contextMenuEvent(QContextMenuEvent *)
{
    QMenu menu(this);
    connect(&menu, SIGNAL(triggered (QAction *)), SLOT(writeSettings()));
    connect(&menu, SIGNAL(triggered (QAction *)), SLOT(readSettings()));

    menu.addAction("Color", this, SLOT(changeColor()));
    menu.addAction(m_starAction);
    menu.addAction("StarColor", this, SLOT(changeStarColor()));

    menu.exec(QCursor::pos());
}

void FWave::process ()
{
    static fft_state *state = 0;
    if (!state)
        state = fft_init();

    int rows = (height() - 2) / m_cell_size.height();
    int cols = (width() - 2) / m_cell_size.width() / 2;

    if(m_rows != rows || m_cols != cols)
    {
        m_rows = rows;
        m_cols = cols;
        if(m_peaks)
            delete [] m_peaks;
        if(m_intern_vis_data)
            delete [] m_intern_vis_data;
        if(m_x_scale)
            delete [] m_x_scale;
        m_peaks = new double[m_cols * 2];
        m_intern_vis_data = new double[m_cols * 2];
        m_x_scale = new int[m_cols + 1];

        for(int i = 0; i < m_cols * 2; ++i)
        {
            m_peaks[i] = 0;
            m_intern_vis_data[i] = 0;
        }
        for(int i = 0; i < m_cols + 1; ++i)
            m_x_scale[i] = pow(pow(255.0, 1.0 / m_cols), i);
    }

    short dest_l[256];
    short dest_r[256];
    short yl, yr;
    int j, k, magnitude_l, magnitude_r;

    calc_freq (dest_l, m_left_buffer);
    calc_freq (dest_r, m_right_buffer);

    double y_scale = (double) 1.25 * m_rows / log(256);

    for (int i = 0; i < m_cols; i++)
    {
        j = m_cols * 2 - i - 1; //mirror index
        yl = yr = 0;
        magnitude_l = magnitude_r = 0;

        if(m_x_scale[i] == m_x_scale[i + 1])
        {
            yl = dest_l[i];
            yr = dest_r[i];
        }
        for (k = m_x_scale[i]; k < m_x_scale[i + 1]; k++)
        {
            yl = qMax(dest_l[k], yl);
            yr = qMax(dest_r[k], yr);
        }

        yl >>= 7; //256
        yr >>= 7;

        if (yl)
        {
            magnitude_l = int(log (yl) * y_scale);
            magnitude_l = qBound(0, magnitude_l, m_rows);
        }
        if (yr)
        {
            magnitude_r = int(log (yr) * y_scale);
            magnitude_r = qBound(0, magnitude_r, m_rows);
        }

        m_intern_vis_data[i] -= m_analyzer_falloff * m_rows / 15;
        m_intern_vis_data[i] = magnitude_l > m_intern_vis_data[i] ? magnitude_l : m_intern_vis_data[i];

        m_intern_vis_data[j] -= m_analyzer_falloff * m_rows / 15;
        m_intern_vis_data[j] = magnitude_r > m_intern_vis_data[j] ? magnitude_r : m_intern_vis_data[j];

        m_peaks[i] -= m_peaks_falloff * m_rows / 15;
        m_peaks[i] = magnitude_l > m_peaks[i] ? magnitude_l : m_peaks[i];

        m_peaks[j] -= m_peaks_falloff * m_rows / 15;
        m_peaks[j] = magnitude_r > m_peaks[j] ? magnitude_r : m_peaks[j];
    }
}

void FWave::draw (QPainter *p)
{
    if(m_starAction->isChecked())
    {
        for(int i=0; i<m_starPoints.count(); ++i)
        {
            StarPoint *sp = &m_starPoints[i];
            m_starColor.setAlpha(sp->m_alpha);
            p->setPen(QPen(m_starColor, 3));
            p->drawPoint(sp->m_pt);
        }
    }

    QLinearGradient line(0, 0, 0, height());
    for(int i=0; i<m_colors.count(); ++i)
    {
        line.setColorAt((i+1)*1.0/m_colors.count(), m_colors[i]);
    }
    p->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    p->setPen(QPen(line, 1));

    int x = 0;
    int rdx = qMax(0, width() - 2 * m_cell_size.width() * m_cols);

    float l = 1.0f;
    if(SoundCore::instance())
    {
        l = SoundCore::instance()->volume()*1.0/100;
    }

    for (int j = 0; j < m_cols * 2; ++j)
    {
        x = j * m_cell_size.width() + 1;
        if(j >= m_cols)
            x += rdx; //correct right part position

        int hh = m_intern_vis_data[j] * l * m_cell_size.height();
        if(abs(hh) > height()/2)
            hh = height()/2;

        p->drawLine(QPoint(x, height()/2 - hh), QPoint(x, height()/2 + hh));
    }
}
