#include "AbstractGraphModel.hpp"

#include <QtNodes/ConnectionIdUtils>

namespace QtNodes {
/*
This function is called before ports are deleted from a node. 
It's used to safely remove or rewire connections that will be affected.
*/
void AbstractGraphModel::portsAboutToBeDeleted(NodeId const nodeId,
                                               PortType const portType,
                                               PortIndex const first,
                                               PortIndex const last)
{
    _shiftedByDynamicPortsConnections.clear(); //Clears any previously stored connections that were shifted due to port changes. Prepares to track new shifts

    auto portCountRole = portType == PortType::In ? NodeRole::InPortCount : NodeRole::OutPortCount;

    unsigned int portCount = nodeData(nodeId, portCountRole).toUInt();

    if (first > portCount - 1)
        return;

    if (last < first)
        return;
   //Sanity checks: If the range of ports is invalid, return early.

    auto clampedLast = std::min(last, portCount - 1);

    for (PortIndex portIndex = first; portIndex <= clampedLast; ++portIndex) {
        std::unordered_set<ConnectionId> conns = connections(nodeId, portType, portIndex);

        for (auto connectionId : conns) {
            deleteConnection(connectionId); //Deletes all connections associated with the soon-to-be-deleted ports.
        }
    }

    std::size_t const nRemovedPorts = clampedLast - first + 1;

    for (PortIndex portIndex = clampedLast + 1; portIndex < portCount; ++portIndex) {
        std::unordered_set<ConnectionId> conns = connections(nodeId, portType, portIndex);

        for (auto connectionId : conns) {
            // Erases the information about the port on one side;
            auto c = makeIncompleteConnectionId(connectionId, portType);

           // Shift the port index backward to reflect deletion.

            c = makeCompleteConnectionId(c, nodeId, portIndex - nRemovedPorts);

            //Store the shifted connection and delete the original.

            _shiftedByDynamicPortsConnections.push_back(c);
            
            deleteConnection(connectionId);
        }
    }
}

// Called after ports are deleted.
void AbstractGraphModel::portsDeleted()
{
    // Reconnects the stored connections to their new (shifted) port
    for (auto const connectionId : _shiftedByDynamicPortsConnections) {
        addConnection(connectionId);
    }


    _shiftedByDynamicPortsConnections.clear();
}


// Called before ports are inserted, to prepare shifting existing connections.
void AbstractGraphModel::portsAboutToBeInserted(NodeId const nodeId,
                                                PortType const portType,
                                                PortIndex const first,
                                                PortIndex const last)
{
    // Reset the list of shifted connections.ew
    _shiftedByDynamicPortsConnections.clear();

    auto portCountRole = portType == PortType::In ? NodeRole::InPortCount : NodeRole::OutPortCount;

    unsigned int portCount = nodeData(nodeId, portCountRole).toUInt();

    if (first > portCount)
        return;

    if (last < first)
        return;

    std::size_t const nNewPorts = last - first + 1;

    for (PortIndex portIndex = first; portIndex < portCount; ++portIndex) {
        std::unordered_set<ConnectionId> conns = connections(nodeId, portType, portIndex);

        for (auto connectionId : conns) {
            // Erases the information about the port on one side;
            auto c = makeIncompleteConnectionId(connectionId, portType);

            c = makeCompleteConnectionId(c, nodeId, portIndex + nNewPorts);

            _shiftedByDynamicPortsConnections.push_back(c);

            deleteConnection(connectionId);
        }
    }
}
// Called after ports are inserted.

void AbstractGraphModel::portsInserted()
{
    for (auto const connectionId : _shiftedByDynamicPortsConnections) {
        addConnection(connectionId);
    }

    _shiftedByDynamicPortsConnections.clear();
}

} // namespace QtNodes
