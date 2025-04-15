#include "DefaultConnectionPainter.hpp"

#include <QtGui/QIcon>

#include "AbstractGraphModel.hpp"
#include "ConnectionGraphicsObject.hpp"
#include "ConnectionState.hpp"
#include "Definitions.hpp"
#include "NodeData.hpp"
#include "StyleCollection.hpp"

namespace QtNodes {

//  This method returns a QPainterPath representing a smooth cubic Bézier curve between two endpoints.

QPainterPath DefaultConnectionPainter::cubicPath(ConnectionGraphicsObject const &connection) const
{
    //Gets the coordinates of the input and output ports.
    QPointF const &in = connection.endPoint(PortType::In);
    QPointF const &out = connection.endPoint(PortType::Out);

    //Retrieves the two control points for the Bezier curve.

    auto const c1c2 = connection.pointsC1C2();

   //Constructs a cubic Bézier curve from out to in with control points c1 and c2.
    // cubic spline
    QPainterPath cubic(out);

    cubic.cubicTo(c1c2.first, c1c2.second, in);

    return cubic;
}
// Draws a dashed line representing a temporary/dragged connection.
void DefaultConnectionPainter::drawSketchLine(QPainter *painter, ConnectionGraphicsObject const &cgo) const
{
    ConnectionState const &state = cgo.connectionState();

//   Only draw if the user hasn't finished connecting (i.e., one port is still undefined).

    if (state.requiresPort()) {
        auto const &connectionStyle = QtNodes::StyleCollection::connectionStyle();

        QPen pen;
        //   //   Sets up a dashed pen for sketch mode.
        pen.setWidth(static_cast<int>(connectionStyle.constructionLineWidth()));
        pen.setColor(connectionStyle.constructionColor());
        pen.setStyle(Qt::DashLine);
     
     //Draws the Bézier path with the dashed pen.

        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);

        auto cubic = cubicPath(cgo);

        // cubic spline
        painter->drawPath(cubic);
    }
}

void DefaultConnectionPainter::drawHoveredOrSelected(QPainter *painter, ConnectionGraphicsObject const &cgo) const
{
    // Checks if the connection is hovered or selected.

    bool const hovered = cgo.connectionState().hovered();
    bool const selected = cgo.isSelected();

    // drawn as a fat background
    //If either, draw a thicker line behind the normal line.
    if (hovered || selected) {
        auto const &connectionStyle = QtNodes::StyleCollection::connectionStyle();

        double const lineWidth = connectionStyle.lineWidth();
//   Style the pen differently for hover vs. selection.
        QPen pen;
        pen.setWidth(static_cast<int>(2 * lineWidth));
        pen.setColor(selected ? connectionStyle.selectedHaloColor()
                              : connectionStyle.hoveredColor());

        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);

        // cubic spline
        auto const cubic = cubicPath(cgo);
        painter->drawPath(cubic);
    }
}

void DefaultConnectionPainter::drawNormalLine(QPainter *painter, ConnectionGraphicsObject const &cgo) const
{
    
    ConnectionState const &state = cgo.connectionState();
// Don't draw a solid connection unless both ends are valid.
    if (state.requiresPort())
        return;

    // colors

    auto const &connectionStyle = QtNodes::StyleCollection::connectionStyle();
   //These colors depend on the data type at each port (if enabled).

    QColor normalColorOut = connectionStyle.normalColor();
    QColor normalColorIn = connectionStyle.normalColor();
    QColor selectedColor = connectionStyle.selectedColor();

    bool useGradientColor = false;

    AbstractGraphModel const &graphModel = cgo.graphModel();

    if (connectionStyle.useDataDefinedColors()) {
        using QtNodes::PortType;

        auto const cId = cgo.connectionId();

        auto dataTypeOut = graphModel
                               .portData(cId.outNodeId,
                                         PortType::Out,
                                         cId.outPortIndex,
                                         PortRole::DataType)
                               .value<NodeDataType>();

        auto dataTypeIn
            = graphModel.portData(cId.inNodeId, PortType::In, cId.inPortIndex, PortRole::DataType)
                  .value<NodeDataType>();

        useGradientColor = (dataTypeOut.id != dataTypeIn.id);

        normalColorOut = connectionStyle.normalColor(dataTypeOut.id);
        normalColorIn = connectionStyle.normalColor(dataTypeIn.id);
        selectedColor = normalColorOut.darker(200);
    }

    // geometry

    double const lineWidth = connectionStyle.lineWidth();

    // draw normal line
    QPen p;

    p.setWidth(lineWidth);

    bool const selected = cgo.isSelected();

    auto cubic = cubicPath(cgo);
    //If colors are different, draw a gradient.
    if (useGradientColor) {
        painter->setBrush(Qt::NoBrush);

        QColor cOut = normalColorOut;
        if (selected)
            cOut = cOut.darker(200);
        p.setColor(cOut);
        painter->setPen(p);

        unsigned int constexpr segments = 60;

//Draw 60 segments of the cubic curve manually with color transitioning halfway.
        for (unsigned int i = 0ul; i < segments; ++i) {
            double ratioPrev = double(i) / segments;
            double ratio = double(i + 1) / segments;

            if (i == segments / 2) {
                QColor cIn = normalColorIn;
                if (selected)
                    cIn = cIn.darker(200);

                p.setColor(cIn);
                painter->setPen(p);
            }
            painter->drawLine(cubic.pointAtPercent(ratioPrev), cubic.pointAtPercent(ratio));
        }

        {
            //Draws an icon (e.g., a conversion marker) at the midpoint of the connection.

            QIcon icon(":convert.png");

            QPixmap pixmap = icon.pixmap(QSize(22, 22));
            painter->drawPixmap(cubic.pointAtPercent(0.50)
                                    - QPoint(pixmap.width() / 2, pixmap.height() / 2),
                                pixmap);
        }
    }
    //If there's no gradient, draw the entire path with a single color.

     else {
        p.setColor(normalColorOut);

        if (selected) {
            p.setColor(selectedColor);
        }

        painter->setPen(p);
        painter->setBrush(Qt::NoBrush);

        painter->drawPath(cubic);
    }
}

//Main method that paints the connection object.

void DefaultConnectionPainter::paint(QPainter *painter, ConnectionGraphicsObject const &cgo) const
{
    drawHoveredOrSelected(painter, cgo);

    drawSketchLine(painter, cgo);

    drawNormalLine(painter, cgo);

    // Draws everything in layers: halo, sketch, then normal line.

#ifdef NODE_DEBUG_DRAWING
    debugDrawing(painter, cgo);
#endif

//Optional debug visuals if compiled with NODE_DEBUG_DRAWING.
    // draw end points
    auto const &connectionStyle = QtNodes::StyleCollection::connectionStyle();

    double const pointDiameter = connectionStyle.pointDiameter();

    painter->setPen(connectionStyle.constructionColor());
    painter->setBrush(connectionStyle.constructionColor());

    //Draws small circles at both ends of the connection.


    double const pointRadius = pointDiameter / 2.0;
    painter->drawEllipse(cgo.out(), pointRadius, pointRadius);
    painter->drawEllipse(cgo.in(), pointRadius, pointRadius);
}

//Creates a path used for interaction like hit testing.

QPainterPath DefaultConnectionPainter::getPainterStroke(ConnectionGraphicsObject const &connection) const
{
    auto cubic = cubicPath(connection);

    QPointF const &out = connection.endPoint(PortType::Out);
    QPainterPath result(out);

    unsigned int constexpr segments = 20;
////Approximates the curve using straight lines.
    for (auto i = 0ul; i < segments; ++i) {
        double ratio = double(i + 1) / segments;
        result.lineTo(cubic.pointAtPercent(ratio));
    }
//  Creates a wider area around the line for easier hit detection.
    QPainterPathStroker stroker;
    stroker.setWidth(10.0);

    return stroker.createStroke(result);
}

#ifdef NODE_DEBUG_DRAWING

//Debug function for drawing control points and bounding boxes.
void DefaultConnectionPainter::debugDrawing(QPainter *painter, ConnectionGraphicsObject const &cgo)
{
    Q_UNUSED(painter);

    {
        QPointF const &in = cgo.endPoint(PortType::In);
        QPointF const &out = cgo.endPoint(PortType::Out);

        auto const points = cgo.pointsC1C2();

        painter->setPen(Qt::red);
        painter->setBrush(Qt::red);

        painter->drawLine(QLineF(out, points.first));
        painter->drawLine(QLineF(points.first, points.second));
        painter->drawLine(QLineF(points.second, in));
        painter->drawEllipse(points.first, 3, 3);
        painter->drawEllipse(points.second, 3, 3);

        painter->setBrush(Qt::NoBrush);
        painter->drawPath(cubicPath(cgo));
    }

    {

        //Visualizes the control points, bounding box, and path of the connection for debugging purposes.
        painter->setPen(Qt::yellow);
        painter->drawRect(cgo.boundingRect());
    }
}
#endif

} // namespace QtNodes
