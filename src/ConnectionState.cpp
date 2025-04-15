#include "ConnectionState.hpp"

#include <QtCore/QDebug> //Used for logging/debugging 
#include <QtCore/QPointF> //  Defines 2D points with floating-point precision (not used here either—maybe leftover from edits).

#include "BasicGraphicsScene.hpp" // The scene that manages nodes and connections.
#include "ConnectionGraphicsObject.hpp" // The visual object representing a connection
#include "NodeGraphicsObject.hpp" //The visual object representing a node.

namespace QtNodes {

ConnectionState::~ConnectionState()
{
    //resetLastHoveredNode();
}

PortType ConnectionState::requiredPort() const
{
    PortType t = PortType::None; // Initializes t to None — the default return if no port is missing.

//If the connection has no source node, then we require an output port.
    if (_cgo.connectionId().outNodeId == InvalidNodeId) {
        t = PortType::Out;
    } else if (_cgo.connectionId().inNodeId == InvalidNodeId) {
        t = PortType::In; // Otherwise, if there's no destination node, we need an input port.
    }

    return t; // Returns which port is still needed to complete the connection.
}

bool ConnectionState::requiresPort() const
{
    const ConnectionId &id = _cgo.connectionId(); // Grabs the ConnectionId from the attached ConnectionGraphicsObject
    return id.outNodeId == InvalidNodeId || id.inNodeId == InvalidNodeId; // Returns true if either port is not yet assigned — i.e., the connection is incomplete.
} 

bool ConnectionState::hovered() const
{
    return _hovered; // Simple getter for whether this connection is currently hovered by the mouse.
}

void ConnectionState::setHovered(bool hovered)
{
    _hovered = hovered; // Sets the hover state to true or false.
}

void ConnectionState::setLastHoveredNode(NodeId const nodeId)
{
    _lastHoveredNode = nodeId; // Stores the ID of the last node the user hovered over.
}

NodeId ConnectionState::lastHoveredNode() const
{
    return _lastHoveredNode; // Returns the NodeId of the most recently hovered node.
}

void ConnectionState::resetLastHoveredNode() // Checks whether there is a valid last-hovered node to reset.
{
    if (_lastHoveredNode != InvalidNodeId) {
        auto ngo = _cgo.nodeScene()->nodeGraphicsObject(_lastHoveredNode); // Gets the NodeGraphicsObject for that last-hovered node.
        ngo->update();// repaint for the node's graphics (to remove hover highlight).

    }

    _lastHoveredNode = InvalidNodeId; // Resets _lastHoveredNode to InvalidNodeId to clear the state.
}

} // namespace QtNodes
