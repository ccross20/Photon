#ifndef GRAPHSORTER_H
#define GRAPHSORTER_H
#include <QSet>
#include "keira-global.h"

namespace keira {

class KEIRA_EXPORT GraphSorter
{
public:
    GraphSorter(NodeVector toSort);

    NodeVector sorted(){return m_sorted;}

private:
    void visit(Node *node);
    NodeVector m_toSort;
    NodeVector m_sorted;
    // Nodes currently on the DFS recursion stack - lets visit() recognize a
    // cycle (node revisited before it's finished) and stop instead of
    // recursing forever. Distinct from each Node's own "visited" flag, which
    // only means "fully processed", not "currently being processed".
    QSet<Node*> m_visiting;
};

} // namespace exo

#endif // GRAPHSORTER_H
