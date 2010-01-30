#include "WorldView.h"

#include "MainWindow.h"

#include <QPainter>
#include <QDebug>
#include <QSettings>
#include <QListWidget>
#include <cmath>

#include "moc_WorldView.cxx"

const int WorldView::s_lineSelectRadius = 4;

WorldView::WorldView(MainWindow * window, QWidget * parent) :
    QWidget(parent),
    m_hsb(new QScrollBar(Qt::Horizontal, this)),
    m_vsb(new QScrollBar(Qt::Vertical, this)),
    m_window(window),
    m_world(NULL),
    m_zoom(1.0),
    m_offsetX(0),
    m_offsetY(0),
    m_mapCache(),
    m_selectedMap(NULL)
{
    m_hsb->show();
    m_vsb->show();

    connect(m_vsb, SIGNAL(valueChanged(int)), this, SLOT(verticalScroll(int)));
    connect(m_hsb, SIGNAL(valueChanged(int)), this, SLOT(horizontalScroll(int)));

    readSettings();
    updateViewCache();

    this->setMouseTracking(true);
}

WorldView::~WorldView()
{
    if( m_world )
        delete m_world;
}

void WorldView::readSettings()
{

    QSettings settings;
    m_grid = (GridRenderType)settings.value("editor/grid", Pretty).toInt();

    this->update();
}

void WorldView::resizeEvent(QResizeEvent * e)
{
    // move the scroll bars into position
    m_hsb->setGeometry(0, this->height()-m_hsb->height(), this->width()-m_vsb->width(), m_hsb->height());
    m_vsb->setGeometry(this->width()-m_vsb->width(), 0, m_vsb->width(), this->height()-m_hsb->height());

    updateViewCache();
    update();
}

void WorldView::updateViewCache()
{
    // when the scroll or zoom changes, we need to recalculate which maps
    // need to be drawn.
    m_mapCache.clear();
    if( m_world && m_world->isGood() ) {
        // select the maps that are in range
        std::vector<Map*> * maps = m_world->maps();
        double viewLeft = absoluteX(0);
        double viewTop = absoluteY(0);
        double viewRight = viewLeft + this->width() * m_zoom;
        double viewBottom = viewTop + this->height() * m_zoom;
        m_maxLayer = 0;
        for(unsigned int i=0; i < maps->size(); ++i) {
            // determine if the map is in range
            EditorMap * map = (EditorMap *) maps->at(i);

            if (!(map->left() > viewRight || map->top() > viewBottom ||
                  map->left() + map->width() < viewLeft || map->top() + map->height() < viewTop))
            {
                if (map->layerCount() > m_maxLayer)
                    m_maxLayer = map->layerCount();

                m_mapCache.append(map);
            }
        }
    }

    this->update();
}

void WorldView::paintEvent(QPaintEvent * e)
{
    QPainter p(this);
    p.setBackground(Qt::white);
    p.eraseRect(0, 0, this->width(), this->height());

    if( m_world ) {
        if( m_world->isGood() ) {
            for(int layer=0; layer<m_maxLayer; ++layer) {
                for(int i=0; i<m_mapCache.size(); ++i) {
                    EditorMap * map = m_mapCache[i];

                    // if the map is selected and this layer is unchecked, don't draw
                    if( map == m_selectedMap && m_window->layersList()->item(layer)->checkState() == Qt::Unchecked)
                        continue;

                    if( layer < map->layerCount() )
                        map->draw(&p, absoluteX(0), absoluteY(0),
                            (double)this->width(), (double)this->height(), layer);
                }
            }
            // draw a bold line around map borders
            QPen normalMapBorder(Qt::black, 2);
            QPen selectedMapBorder(Qt::blue, 2);
            p.setBrush(Qt::NoBrush);
            for(int i=0; i<m_mapCache.size(); ++i) {
                EditorMap * map = m_mapCache[i];

                if( m_selectedMap == map )
                    p.setPen(selectedMapBorder);
                else
                    p.setPen(normalMapBorder);

                p.drawRect(screenX(map->left()), screenY(map->top()),
                    map->width() * m_zoom, map->height() * m_zoom);
            }

            drawGrid(p);
        } else {
            p.drawText(0, 0, this->width(), this->height(), Qt::AlignCenter,
                tr("Error loading World."));
        }
    } else {
        p.drawText(0, 0, this->width(), this->height(), Qt::AlignCenter,
            tr("Double click a world to edit"));
    }
}

void WorldView::drawGrid(QPainter &p)
{
    if( m_grid != None ) {
        if( m_grid == Pretty )
            p.setPen(QColor(128, 128, 128, 64));
        else
            p.setPen(QColor(128, 128, 128));

        // vertical lines
        double gameLeft = absoluteX(0);
        double gameTop = absoluteY(0);
        double gameRight = absoluteX(this->width());
        double gameBottom = absoluteY(this->height());
        double gridX = gameLeft - std::fmod(gameLeft, Tile::size);
        double gridY = gameTop - std::fmod(gameTop, Tile::size);

        if( m_grid == Pretty ) {
            while(gridX < gameRight) {
                double drawX = screenX(gridX);
                p.drawLine((int)drawX, 0, (int)drawX, this->height());
                gridX += Tile::size;
            }

            while(gridY < gameBottom) {
                double drawY = screenY(gridY);
                p.drawLine(0, (int)drawY, this->width(), (int)drawY);
                gridY += Tile::size;
            }
        } else if( m_grid == Fast ) {
            for(double y = gridY; y < gameBottom; y+=Tile::size) {
                for(double x = gridX; x < gameRight; x+=Tile::size)
                    p.drawPoint((int)screenX(x), (int)screenY(y));
            }
        }
    }
}

double WorldView::screenX(double absoluteX)
{
    return (absoluteX - m_offsetX) * m_zoom;
}

double WorldView::screenY(double absoluteY)
{
    return (absoluteY - m_offsetY) * m_zoom;
}

double WorldView::absoluteX(double screenX)
{
    return (screenX / m_zoom) + m_offsetX;
}

double WorldView::absoluteY(double screenY)
{
    return (screenY / m_zoom) + m_offsetY;
}

bool WorldView::overMapLeft(int x, int y)
{
    if( ! m_selectedMap )
        return false;

    double absX = absoluteX(x);
    double absY = absoluteY(y);

    return  absX > m_selectedMap->left() - s_lineSelectRadius &&
            absX < m_selectedMap->left() + s_lineSelectRadius &&
            absY > m_selectedMap->top() && absY < m_selectedMap->top() + m_selectedMap->height();
}

bool WorldView::overMapTop(int x, int y)
{
    if( ! m_selectedMap )
        return false;

    double absX = absoluteX(x);
    double absY = absoluteY(y);

    return  absX > m_selectedMap->left() && absX < m_selectedMap->left() + m_selectedMap->width() &&
            absY > m_selectedMap->top() - s_lineSelectRadius &&
            absY < m_selectedMap->top() + s_lineSelectRadius;
}
bool WorldView::overMapRight(int x, int y)
{
    if( ! m_selectedMap )
        return false;

    double absX = absoluteX(x);
    double absY = absoluteY(y);

    return  absX > m_selectedMap->left() + m_selectedMap->width() - s_lineSelectRadius &&
            absX < m_selectedMap->left() + m_selectedMap->width() + s_lineSelectRadius &&
            absY > m_selectedMap->top() && absY < m_selectedMap->top() + m_selectedMap->height();

}
bool WorldView::overMapBottom(int x, int y)
{
    if( ! m_selectedMap )
        return false;

    double absX = absoluteX(x);
    double absY = absoluteY(y);

    return  absX > m_selectedMap->left() && absX < m_selectedMap->left() + m_selectedMap->width() &&
            absY > m_selectedMap->top() + m_selectedMap->height() - s_lineSelectRadius &&
            absY < m_selectedMap->top() + m_selectedMap->height() + s_lineSelectRadius;
}

void WorldView::mouseMoveEvent(QMouseEvent * e)
{
    // change mouse cursor to sizers if over map boundaries
    // if the user could use the arrow tool
    if( m_window->m_toolLeftClick == MainWindow::Arrow ||
        m_window->m_toolMiddleClick == MainWindow::Arrow ||
        m_window->m_toolRightClick == MainWindow::Arrow )
    {
        // left boundary
        if( overMapLeft(e->x(), e->y()) || overMapRight(e->x(),e->y()))
            this->setCursor(Qt::SizeHorCursor);
        else if( overMapTop(e->x(), e->y()) || overMapBottom(e->x(), e->y()))
            this->setCursor(Qt::SizeVerCursor);
        else
            this->setCursor(Qt::ArrowCursor);
    }
}

void WorldView::mousePressEvent(QMouseEvent * e)
{
    MainWindow::MouseTool tool = MainWindow::Nothing;

    if( e->button() == Qt::LeftButton )
        tool = m_window->m_toolLeftClick;
    else if( e->button() == Qt::MidButton )
        tool = m_window->m_toolMiddleClick;
    else if( e->button() == Qt::RightButton )
        tool = m_window->m_toolRightClick;

    switch( tool ){
        case MainWindow::Nothing:
            break;
        case MainWindow::Arrow:
            // if they clicked inside a map, select it
            selectMap(mapAt(e->x(), e->y()));
            break;
        case MainWindow::Eraser:

            break;
        case MainWindow::Pan:

            break;
        case MainWindow::Center:

            break;
        case MainWindow::Pencil:

            break;
        case MainWindow::Brush:

            break;
        case MainWindow::SetStartingPoint:

            break;
        default:
            qDebug() << "Invalid tool selected in mousePressEvent";
    }
}

EditorMap * WorldView::mapAt(int x, int y)
{
    double absX = absoluteX(x);
    double absY = absoluteY(y);
    for(int i=0; i<m_mapCache.size(); ++i) {
        EditorMap * map = m_mapCache[i];
        if( absX > map->left() && absX < map->left() + map->width() &&
            absY > map->top() && absY < map->top() + map->height() )
        {
            return map;
        }
    }
    return NULL;
}

void WorldView::setWorld(EditorWorld * world)
{
    m_world = world;
    selectMap(NULL);

    updateViewCache();
}

void WorldView::selectMap(EditorMap * map)
{
    m_selectedMap = map;
    QListWidget * list = m_window->layersList();
    list->clear();
    if( m_selectedMap ) {
        // add the layers from that map
        for(int i=0; i<m_selectedMap->layerCount(); ++i) {
            QListWidgetItem * newItem = new QListWidgetItem(tr("Layer %1").arg(i+1), list);
            newItem->setFlags(Qt::ItemIsUserCheckable|Qt::ItemIsSelectable|
                              Qt::ItemIsEditable|Qt::ItemIsEnabled);
            newItem->setCheckState(Qt::Checked);
            list->addItem(newItem);
        }

        if( m_selectedMap->layerCount() > 0 )
            list->item(0)->setSelected(true);
    } else {
        list->addItem(tr("Click a map to select it and view layers"));
    }

    this->update();
}

void WorldView::verticalScroll(int value)
{
    qDebug() << value ;
}
void WorldView::horizontalScroll(int value)
{
    qDebug() << value;
}

void WorldView::setSelectedLayer(int index)
{
    m_window->layersList()->setCurrentRow(index);
    m_selectedLayer = index;
}
